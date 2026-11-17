/*
  DMX Total Control
  safetymanager.cpp

  Copyright (c) DMX Total Control contributors

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QCoreApplication>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QFileInfo>
#include <QThread>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QSet>
#include <QUrl>
#include <QDebug>

#include "safetymanager.h"
#include "patchmanager.h"
#include "appsettings.h"
#include "inputoutputmap.h"
#include "channelmodifier.h"
#include "outputlayer.h"
#include "qlcchannel.h"
#include "universe.h"
#include "fixture.h"
#include "doc.h"

/* The time of a line of the log */
static QString logTime(const QDateTime &when)
{
    return when.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
}

SafetyManager::SafetyManager(Doc *doc, PatchManager *patch, AppSettings *settings, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_patch(patch)
    , m_settings(settings)
    , m_idleTimer(new QTimer(this))
    , m_sessionStart(QDateTime::currentDateTime())
    , m_holdTimer(new QTimer(this))
    , m_keyTimer(new QTimer(this))
    , m_rampTimer(new QTimer(this))
{
    // the parks follow their fixtures (before the new layer, which has
    // them as they are then)
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotFixtureRemoved(quint32)));
    connect(m_doc, SIGNAL(fixtureChanged(quint32)), this, SLOT(slotFixtureChanged(quint32)));
    // the patch: each change gives the universes a new layer at once (a
    // fixture moved is protected at its new address before any frame)
    connect(m_doc, SIGNAL(fixtureAdded(quint32)), this, SLOT(slotPatchChanged()));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotPatchChanged()));
    connect(m_doc, SIGNAL(fixtureChanged(quint32)), this, SLOT(slotPatchChanged()));
    connect(m_doc, SIGNAL(loaded()), this, SLOT(slotPatchChanged()));
    connect(m_doc, SIGNAL(cleared()), this, SLOT(slotDocCleared()));
    // (the groups the show stop lights)
    connect(m_doc, SIGNAL(fixtureGroupRemoved(quint32)), this, SLOT(slotGroupRemoved(quint32)));
    connect(m_doc, SIGNAL(fixtureGroupChanged(quint32)), this, SLOT(slotGroupChanged(quint32)));
    connect(m_patch, &PatchManager::protectionChanged, this, &SafetyManager::slotProtectionChanged);
    // (the icon of a fixture says whether the show stop lights a generic
    // dimmer: its light follows it while it is on)
    connect(m_patch, &PatchManager::fixturesChanged, this, [this]() {
        if (m_showStop && m_patch->loading() == false && m_doc->isClearingOrLoading() == false)
            push();
    });
    // (while applyPatch holds the universes: nothing is told then, Doc
    // tells the change right after)
    PatchManager::setPatchHook(m_doc, [this]() { pushLayers(false); });

    InputOutputMap *map = m_doc->inputOutputMap();
    connect(map, SIGNAL(universeAdded(quint32)), this, SLOT(slotPatchChanged()));
    // every Blackout disarms: the button, the shortcut, the SHOW, a mapping
    connect(map, SIGNAL(blackoutChanged(bool)), this, SLOT(slotBlackoutChanged(bool)));
    // the fires, their ends and the refusals, from the threads of the universes
    connect(map, SIGNAL(effectEvent(quint32,quint32,int,int,uint)),
            this, SLOT(slotEffectEvent(quint32,quint32,int,int,uint)));

    // the countdown of the auto-disarm
    connect(m_idleTimer, &QTimer::timeout, this, &SafetyManager::slotIdleTick);
    // the show stop: put or taken away once held for a second
    m_holdTimer->setSingleShot(true);
    m_holdTimer->setInterval(holdMs());
    connect(m_holdTimer, &QTimer::timeout, this, &SafetyManager::slotHoldDone);
    // the Pause key: its second press is waited for a moment, then forgotten
    m_keyTimer->setSingleShot(true);
    m_keyTimer->setInterval(keyWindowMs());
    connect(m_keyTimer, &QTimer::timeout, this, &SafetyManager::keyPendingChanged);
    // the fade to black: Blackout at its end (not a moment before it is dark)
    m_rampTimer->setSingleShot(true);
    m_rampTimer->setTimerType(Qt::PreciseTimer);
    connect(m_rampTimer, &QTimer::timeout, this, &SafetyManager::slotRampDone);
    if (m_settings != nullptr)
        connect(m_settings, &AppSettings::liveSafetyChanged, this, &SafetyManager::slotIdleTick);

    // never armed when it starts: the universes get the layers disarmed
    push();
}

SafetyManager::~SafetyManager()
{
    PatchManager::setPatchHook(m_doc, {});
    m_idleTimer->stop();
    m_holdTimer->stop();
    m_keyTimer->stop();
    m_rampTimer->stop();
    // never left armed, parked nor in a show stop: the universes outlive it.
    // Disarmed and out on every interface before it goes (one that repeats
    // the last frame it got must not keep a fire, nor a park)
    if (m_armed || m_parks.isEmpty() == false || m_showStop || m_rampId != 0)
        disarmAndFlush(QStringLiteral("exit"));
}

/*********************************************************************
 * The layers of the output
 *********************************************************************/

QVector<QSharedPointer<const OutputLayer>> SafetyManager::buildLayers(Doc *doc, PatchManager *patch,
                                                                      quint32 serial, quint32 armToken,
                                                                      const QList<ParkEntry> &parks,
                                                                      const QMap<quint32, StopValue> *showStop,
                                                                      quint32 rampId, quint32 rampMs)
{
    const int count = int(doc->inputOutputMap()->universesCount());
    QVector<OutputLayer *> layers(count);
    for (int u = 0; u < count; u++)
    {
        layers[u] = new OutputLayer();
        layers[u]->serial = serial;
        layers[u]->armToken = armToken;
        // (the fade to black: the same on every universe)
        layers[u]->rampId = rampId;
        layers[u]->rampMs = rampId != 0 ? rampMs : 0;
    }

    // a protected special effect with any park is parked off: it never fires
    QSet<quint32> parked;
    for (const ParkEntry &park : parks)
        parked.insert(park.fixture);
    // (the fixtures the gate has: their channels are never parked)
    QSet<quint32> units;

    for (Fixture *fxi : doc->fixtures())
    {
        if (patch->isProtected(fxi->id()) == false)
            continue;
        // (its times and its safety channel; its protect is isProtected's)
        const PatchManager::EffectRule rule = patch->effectRule(fxi->id());
        units.insert(fxi->id());
        const int channels = int(fxi->channels());

        // what fires: every channel but the safety one and pan and tilt
        // (fine bytes too), whatever its group says (a valve that is no
        // intensity has the time limit too). Pan and tilt pass while armed,
        // without a time (a jet that only aims never fires); a fixture with
        // nothing else: all of them but the safety one fire
        QVector<bool> fire(channels, false);
        bool anyFire = false;
        for (int c = 0; c < channels; c++)
        {
            if (c == rule.safety)
                continue;
            const QLCChannel *ch = fxi->channel(quint32(c));
            // (a channel the definition doesn't describe: as a fire one)
            const bool motion = ch != nullptr && (ch->group() == QLCChannel::Pan || ch->group() == QLCChannel::Tilt);
            fire[c] = motion == false;
            anyFire = anyFire || fire.at(c);
        }
        if (anyFire == false)
        {
            for (int c = 0; c < channels; c++)
                fire[c] = c != rule.safety;
        }

        // a unit for each universe it is in (as the scenes write it: its
        // universe address); across two universes, or parked, it never fires
        const bool parkedOff = parked.contains(fxi->id());
        const bool canFire = PatchManager::crossesUniverses(fxi) == false && parkedOff == false;
        QMap<int, OutputLayer::Unit> parts;
        for (int c = 0; c < channels; c++)
        {
            const quint32 ua = fxi->universeAddress() + quint32(c);
            const int u = int(ua / UNIVERSE_SIZE);
            const quint16 address = quint16(ua % UNIVERSE_SIZE);
            if (u >= count)
                continue;
            if (parts.contains(u) == false)
            {
                OutputLayer::Unit unit;
                unit.fixture = fxi->id();
                unit.maxFireMs = rule.maxFireMs;
                unit.minWaitMs = rule.minWaitMs;
                unit.safetyFrom = rule.from;
                unit.safetyTo = rule.to;
                unit.canFire = canFire;
                unit.parked = parkedOff;
                parts.insert(u, unit);
            }
            OutputLayer::Unit &unit = parts[u];
            if (c == rule.safety)
                unit.safety = address;
            else if (fire.at(c))
                unit.fire.append(address);
            else
                unit.aux.append(address);
        }
        for (auto it = parts.cbegin(); it != parts.cend(); ++it)
            layers[it.key()]->addUnit(it.value());
    }

    // the parks: the byte that goes out on each channel, in the universe of
    // that channel. Not on a protected special effect (the gate gives its
    // channels 0, parked off), nor a park of another mode than the
    // fixture's now (its channels are others)
    for (const ParkEntry &park : parks)
    {
        const Fixture *fxi = doc->fixture(park.fixture);
        if (fxi == nullptr || units.contains(park.fixture) || park.channel >= fxi->channels() ||
            park.mode != modeOf(fxi))
            continue;
        const quint32 ua = fxi->universeAddress() + park.channel;
        const int u = int(ua / UNIVERSE_SIZE);
        if (u < count)
            layers[u]->addPark(int(ua % UNIVERSE_SIZE), park.value, park.inBlackout);
    }

    // the show stop: above the parks and Blackout, never on a protected
    // channel (the gate's: 0). A value it sets goes out through the limits
    // of its channel (an inverted dimmer, a maximum), as the scenes do
    if (showStop != nullptr)
    {
        const QList<Universe *> universes = doc->inputOutputMap()->universes();
        for (OutputLayer *layer : std::as_const(layers))
            layer->showStop = true;
        for (auto it = showStop->cbegin(); it != showStop->cend(); ++it)
        {
            const int u = int(it.key() / UNIVERSE_SIZE);
            const int address = int(it.key() % UNIVERSE_SIZE);
            if (u >= count || layers[u]->role(address) != OutputLayer::Free)
                continue;
            uchar value = it.value().value;
            if (it.value().hold == false && u < universes.count())
            {
                ChannelModifier *modifier = universes.at(u)->channelModifier(ushort(address));
                if (modifier != nullptr)
                    value = modifier->getValue(value);
            }
            layers[u]->addForced(address, value, it.value().hold);
        }
    }

    QVector<QSharedPointer<const OutputLayer>> out;
    out.reserve(count);
    for (OutputLayer *layer : std::as_const(layers))
        out.append(QSharedPointer<const OutputLayer>(layer));
    return out;
}

void SafetyManager::push()
{
    pushLayers(true);
}

void SafetyManager::pushLayers(bool notify)
{
    const QList<Universe *> universes = m_doc->inputOutputMap()->universes();
    // (the program closes: the parks and the show stop let go with the rest,
    // the stage at rest)
    QMap<quint32, StopValue> stop;
    const bool stopping = m_showStop && m_closing == false;
    if (stopping)
        stop = showStopValues(m_doc, m_patch, m_showStopLevel, m_showStopGroups);
    const QVector<QSharedPointer<const OutputLayer>> layers = buildLayers(m_doc, m_patch, ++m_serial, armToken(),
                                                                          m_closing ? QList<ParkEntry>() : m_parks.values(),
                                                                          stopping ? &stop : nullptr,
                                                                          m_closing ? 0 : m_rampId, m_rampMs);
    // (only a leaf lock of each universe: their next frame takes it)
    for (int i = 0; i < universes.count() && i < layers.count(); i++)
        universes.at(i)->setOutputLayer(layers.at(i));
    m_revision++;
    if (notify)
        emit revisionChanged();
}

bool SafetyManager::flush(int maxMs)
{
    const QList<Universe *> universes = m_doc->inputOutputMap()->universes();
    // a frame now on each one, even if nothing changed (an interface that
    // copies only changed frames gets it): from its thread when it runs,
    // else here
    for (Universe *universe : universes)
        universe->requestFrame();

    QElapsedTimer waited;
    waited.start();
    while (true)
    {
        bool sent = true;
        for (Universe *universe : universes)
        {
            if (universe->isRunning() && universe->sentLayerSerial() != m_serial)
                sent = false;
        }
        if (sent)
            return true;
        if (waited.elapsed() >= maxMs)
        {
            qWarning() << "[SafetyManager] the universes didn't send the last layer in" << maxMs << "ms";
            return false;
        }
        QThread::msleep(5);
    }
}

void SafetyManager::disarmAndFlush(const QString &reason)
{
    // (the end of the program: never armed again)
    const bool exiting = reason == QLatin1String("exit");
    // (a fade to black: no Blackout comes at its end, the stage goes to rest)
    const bool ramping = exiting && m_rampId != 0;
    if (exiting)
    {
        m_closing = true;
        endRamp(false);
    }
    const bool wasArmed = m_armed;
    disarm(reason);
    // the end of the program lets the parks and the show stop go too: the
    // stage goes to rest (an interface that repeats the last frame keeps no
    // hazer on). The parks stay in the show
    const bool parked = exiting && (m_parks.isEmpty() == false || m_showStop || ramping);
    if (parked && wasArmed == false)
        push();
    if (wasArmed == false && m_patch->hasProtected() == false && parked == false)
        return;
    flush();
    // the ends of the fires it cut, told by the universes (before they go)
    QCoreApplication::sendPostedEvents(m_doc->inputOutputMap(), QEvent::MetaCall);
    // a fire whose end didn't come (no frame): in the log all the same
    if (m_firing.isEmpty() == false)
    {
        const QDateTime now = QDateTime::currentDateTime();
        for (auto it = m_firing.cbegin(); it != m_firing.cend(); ++it)
            logFire(it.key(), it.value(), uint(qMax(qint64(0), it->start.msecsTo(now))), OutputLayer::Disarmed);
        m_firing.clear();
        m_revision++;
        emit firingChanged();
        emit revisionChanged();
    }
}

void SafetyManager::slotPatchChanged()
{
    // a show loading: DtcCore gives the layers before its output starts
    // (Doc and the patch hear of each fixture before it is all there)
    if (m_patch->loading() || m_doc->isClearingOrLoading())
        return;
    push();
}

void SafetyManager::slotProtectionChanged()
{
    // a protected special effect can only be parked off: one parked before
    // it was protected (a PAR called CO2 now) is parked off, all of it
    QList<quint32> parkedOn;
    for (const ParkEntry &park : std::as_const(m_parks))
    {
        if (parkedOn.contains(park.fixture) == false && protectedNow(park.fixture))
            parkedOn.append(park.fixture);
    }
    bool changed = false;
    for (quint32 id : std::as_const(parkedOn))
    {
        const Fixture *fxi = m_doc->fixture(id);
        bool off = fxi != nullptr;
        for (quint32 c = 0; off && c < fxi->channels(); c++)
        {
            const auto it = m_parks.constFind(qMakePair(id, c));
            off = it != m_parks.constEnd() && it->value == 0 && it->mode == modeOf(fxi);
        }
        if (off == false)
        {
            parkAllOff(id, keepsBlackout(id));
            changed = true;
        }
    }
    if (changed)
    {
        m_doc->setModified();
        emit parksChanged();
    }

    emit protectionChanged();
    // nothing to fire any more: not armed
    if (m_armed && hasProtected() == false)
        disarm(QStringLiteral("none"));
    slotPatchChanged();
}

void SafetyManager::slotDocCleared()
{
    // (the show stop of the show that went: off, its settings as a new one has them)
    clearShowStop();
    // (a fade to black of the show that went: the layers below have none)
    endRamp(false);
    // (the parks of the show that went)
    if (m_parks.isEmpty() == false)
    {
        m_parks.clear();
        emit parksChanged();
    }
    // (the fires of the show that went: its universes said nothing more)
    if (m_firing.isEmpty() == false)
    {
        m_firing.clear();
        emit firingChanged();
    }
    if (m_waiting.isEmpty() == false)
    {
        m_waiting.clear();
        emit waitingChanged();
    }
    slotPatchChanged();
}

/*********************************************************************
 * Arming
 *********************************************************************/

bool SafetyManager::hasProtected() const
{
    return m_patch->hasProtected();
}

bool SafetyManager::armed() const
{
    return m_armed;
}

bool SafetyManager::arm()
{
    if (m_armed)
        return true;
    // the program closes: nothing arms them again (the phone, a controller,
    // while the outputs send the stage at rest)
    if (m_closing)
        return false;
    if (hasProtected() == false)
    {
        emit message(tr("No fixture is a protected special effect: there is nothing to arm"), true);
        return false;
    }
    if (m_doc->inputOutputMap()->blackout())
    {
        emit message(tr("Blackout is on: turn it off to arm the special effects"), true);
        return false;
    }
    // (the show stop keeps them disarmed while it lasts)
    if (m_showStop)
    {
        emit message(tr("The show stop is on: take it away to arm the special effects"), true);
        return false;
    }

    // a new arming each time (0: disarmed): what held an effect before
    // has to let it go first, also disarmed and armed between two frames
    m_token = ++m_tokens;
    if (m_token == 0)
        m_token = ++m_tokens;
    m_armed = true;
    push();
    writeLog(logTime(QDateTime::currentDateTime()) + QStringLiteral("\t\t\t\t") + tr("Special effects armed"));
    restartIdle();
    m_idleTimer->start();
    slotIdleTick();
    emit armedChanged();
    return true;
}

void SafetyManager::toggleArmed()
{
    if (m_armed)
        disarm(QStringLiteral("hand"));
    else
        arm();
}

void SafetyManager::disarm(const QString &reason)
{
    if (m_armed == false)
        return;
    m_armed = false;
    push();
    m_idleTimer->stop();
    setDisarmIn(-1);
    writeLog(logTime(QDateTime::currentDateTime()) + QStringLiteral("\t\t\t\t") +
             tr("Special effects disarmed: %1").arg(reasonText(reason)));
    emit armedChanged();
    emit disarmed(reason);
}

QString SafetyManager::reasonText(const QString &reason) const
{
    if (reason == QLatin1String("stopAll"))
        return tr("Stop all");
    if (reason == QLatin1String("blackout"))
        return tr("Blackout");
    if (reason == QLatin1String("show"))
        return tr("another show");
    if (reason == QLatin1String("idle"))
        return tr("%n minute(s) without a fire", "", m_settings != nullptr ? m_settings->autoDisarmMinutes() : 0);
    if (reason == QLatin1String("showStop"))
        return tr("show stop");
    if (reason == QLatin1String("fadeToBlack"))
        return tr("fade to black");
    if (reason == QLatin1String("recall"))
        return tr("Bring back what was playing");
    if (reason == QLatin1String("exit"))
        return tr("the program closed");
    if (reason == QLatin1String("none"))
        return tr("no fixture is protected any more");
    return tr("by hand");
}

void SafetyManager::slotBlackoutChanged(bool on)
{
    if (on == false)
        return;
    disarm(QStringLiteral("blackout"));
    // (a fade to black ends: Blackout is there; the light full under it)
    endRamp();
}

/*********************************************************************
 * Auto-disarm
 *********************************************************************/

int SafetyManager::disarmIn() const
{
    return m_disarmIn;
}

void SafetyManager::setAutoDisarmMsForTest(int ms)
{
    m_testAutoDisarmMs = ms;
    if (m_armed)
        slotIdleTick();
}

int SafetyManager::autoDisarmMs() const
{
    if (m_testAutoDisarmMs >= 0)
        return m_testAutoDisarmMs;
    return m_settings != nullptr ? m_settings->autoDisarmMinutes() * 60000 : 0;
}

void SafetyManager::restartIdle()
{
    m_idle.start();
}

void SafetyManager::setDisarmIn(int seconds)
{
    if (seconds == m_disarmIn)
        return;
    m_disarmIn = seconds;
    emit disarmInChanged();
}

void SafetyManager::slotIdleTick()
{
    if (m_armed == false)
    {
        m_idleTimer->stop();
        setDisarmIn(-1);
        return;
    }
    const int limit = autoDisarmMs();
    // (a second for the countdown; finer for a short limit)
    const int interval = limit > 0 ? qBound(20, limit / 10, 1000) : 1000;
    if (m_idleTimer->interval() != interval)
        m_idleTimer->setInterval(interval);
    if (limit <= 0)
    {
        setDisarmIn(-1);
        return;
    }
    // never while one fires: the time runs from its end
    if (m_firing.isEmpty() == false)
        m_idle.start();
    const qint64 left = qint64(limit) - m_idle.elapsed();
    if (left <= 0)
    {
        disarm(QStringLiteral("idle"));
        return;
    }
    setDisarmIn(int((left + 999) / 1000));
}

/*********************************************************************
 * Fires
 *********************************************************************/

void SafetyManager::slotEffectEvent(quint32 universe, quint32 fixture, int kind, int reason, uint durationMs)
{
    Q_UNUSED(universe)
    // (what changed is told, nothing else: a chaser on a CO2 disarmed gives
    // a refusal and a release at each step, and nothing fires)
    bool firing = false;
    bool waiting = false;
    auto wait = [this, fixture, &waiting](const QString &why)
    {
        if (m_waiting.value(fixture) != why)
        {
            m_waiting.insert(fixture, why);
            waiting = true;
        }
    };
    switch (kind)
    {
        case OutputLayer::FireStarted:
            if (m_firing.contains(fixture) == false)
            {
                m_firing.insert(fixture, { QDateTime::currentDateTime(), fixtureName(fixture), fixtureNumber(fixture) });
                firing = true;
            }
            waiting = m_waiting.remove(fixture) > 0;
            restartIdle();
        break;
        case OutputLayer::FireEnded:
        {
            restartIdle();
            const auto it = m_firing.constFind(fixture);
            // (logged already: disarmAndFlush)
            if (it == m_firing.constEnd())
                break;
            const Firing fire = it.value();
            m_firing.remove(fixture);
            firing = true;
            logFire(fixture, fire, durationMs, reason);
            // cut or disarmed: its source has to let it go
            if (reason != OutputLayer::Released && reason != OutputLayer::Removed)
                wait(QStringLiteral("release"));
        }
        break;
        case OutputLayer::FireRefused:
            wait(reason == OutputLayer::Waiting ? QStringLiteral("wait") : QStringLiteral("release"));
        break;
        case OutputLayer::Unlatched:
            waiting = m_waiting.remove(fixture) > 0;
        break;
        default:
            return;
    }
    if (firing == false && waiting == false)
        return;
    m_revision++;
    if (firing)
        emit firingChanged();
    if (waiting)
        emit waitingChanged();
    emit revisionChanged();
}

QString SafetyManager::fixtureName(quint32 id) const
{
    const Fixture *fxi = m_doc->fixture(id);
    return fxi != nullptr ? fxi->name() : QString();
}

int SafetyManager::fixtureNumber(quint32 id) const
{
    return m_patch->fixtureNumber(id);
}

QVariantList SafetyManager::firing() const
{
    QVariantList list;
    for (auto it = m_firing.cbegin(); it != m_firing.cend(); ++it)
    {
        // (deleted while it fires: as it was, until its end comes)
        const bool here = m_doc->fixture(it.key()) != nullptr;
        list.append(QVariantMap({ { "id", it.key() }, { "name", here ? fixtureName(it.key()) : it->name },
                                  { "number", here ? fixtureNumber(it.key()) : it->number } }));
    }
    return list;
}

QVariantList SafetyManager::waiting() const
{
    QVariantList list;
    for (auto it = m_waiting.cbegin(); it != m_waiting.cend(); ++it)
    {
        if (m_doc->fixture(it.key()) == nullptr)
            continue;
        list.append(QVariantMap({ { "id", it.key() }, { "name", fixtureName(it.key()) },
                                  { "number", fixtureNumber(it.key()) }, { "reason", it.value() } }));
    }
    return list;
}

QString SafetyManager::reasonKey(int reason)
{
    switch (reason)
    {
        case OutputLayer::Released: return QStringLiteral("released");
        case OutputLayer::TimeLimit: return QStringLiteral("time");
        case OutputLayer::ShowStop: return QStringLiteral("showstop");
        case OutputLayer::NotAllowed: return QStringLiteral("notallowed");
        case OutputLayer::Removed: return QStringLiteral("removed");
        case OutputLayer::Parked: return QStringLiteral("parked");
        default: return QStringLiteral("disarmed");
    }
}

QString SafetyManager::fireReasonText(int reason) const
{
    switch (reason)
    {
        case OutputLayer::Released: return tr("released");
        case OutputLayer::TimeLimit: return tr("cut at its maximum time");
        case OutputLayer::ShowStop: return tr("show stop");
        case OutputLayer::NotAllowed: return tr("it can't fire");
        case OutputLayer::Removed: return tr("removed");
        case OutputLayer::Parked: return tr("parked");
        default: return tr("disarmed");
    }
}

QVariantList SafetyManager::recentFires() const
{
    QVariantList list;
    for (const Fire &fire : m_recent)
        list.append(QVariantMap({ { "id", fire.fixture }, { "name", fire.name }, { "number", fire.number },
                                  { "time", fire.start.toString(QStringLiteral("HH:mm:ss")) },
                                  { "ms", int(fire.ms) }, { "reason", reasonKey(fire.reason) },
                                  { "reasonText", fireReasonText(fire.reason) } }));
    return list;
}

void SafetyManager::logFire(quint32 fixture, const Firing &firing, uint ms, int reason)
{
    // (its name now if it was renamed while it fired; as it was if it is gone)
    const bool here = m_doc->fixture(fixture) != nullptr;
    const QDateTime &start = firing.start;
    Fire fire { fixture, here ? fixtureName(fixture) : firing.name, here ? fixtureNumber(fixture) : firing.number,
                start, ms, reason };
    m_recent.prepend(fire);
    while (m_recent.count() > 10)
        m_recent.removeLast();
    writeLog(logTime(start) + QLatin1Char('\t') + (fire.number > 0 ? QString::number(fire.number) : QString()) +
             QLatin1Char('\t') + fire.name + QLatin1Char('\t') + QStringLiteral("%1 ms").arg(ms) +
             QLatin1Char('\t') + fireReasonText(reason));
    emit recentFiresChanged();
}

int SafetyManager::revision() const
{
    return m_revision;
}

/*********************************************************************
 * The log of the session
 *********************************************************************/

static QString logFolder()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QStringLiteral("/effects-log");
}

QString SafetyManager::logFile() const
{
    return m_logFile;
}

void SafetyManager::writeLog(const QString &line)
{
    const QString path = logFolder() + QLatin1Char('/') +
                         m_sessionStart.toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss")) + QStringLiteral(".txt");
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text) == false ||
        file.write((line + QLatin1Char('\n')).toUtf8()) < 0)
    {
        // said once: the show goes on
        if (m_logFailed == false)
        {
            m_logFailed = true;
            emit message(tr("The log of the special effects can't be written (%1)").arg(QDir::toNativeSeparators(path)), true);
        }
        return;
    }
    if (m_logFile != path)
    {
        m_logFile = path;
        emit recentFiresChanged();
    }
}

void SafetyManager::openLog()
{
    const QString folder = logFolder();
    QDir().mkpath(folder);
    QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
}
