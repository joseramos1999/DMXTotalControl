/*
  DMX Total Control
  cuelist.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMutexLocker>
#include <QDebug>
#include <QIcon>
#include <algorithm>
#include <climits>
#include <cmath>

#include "qlcfixturemode.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "inputoutputmap.h"
#include "mastertimer.h"
#include "qlcchannel.h"
#include "universe.h"
#include "cuelist.h"
#include "fixture.h"
#include "doc.h"

// the curves of the cues are the ones of the FadeChannels
static_assert(int(FadeChannel::CurveLinear) == 0 && int(FadeChannel::CurveSmooth) == 1 &&
              int(FadeChannel::CurveSlowStart) == 2 && int(FadeChannel::CurveFastStart) == 3,
              "Cuelist curves are FadeChannel::Curve values");

Cuelist::Cuelist(Doc *doc)
    : Function(doc, Function::CuelistType)
    , m_nextUid(1)
    , m_tracking(true)
    , m_atEnd(AtEndLoop)
    , m_resetOnRelease(true)
    , m_backTime(-1)
    , m_releaseOnNextGo(false)
    , m_moveInBlack(false)
    , m_mibDelay(0)
    , m_mibFade(defaultMibFade())
    , m_mode(ModeNormal)
    , m_chaseBpm(defaultChaseBpm())
    , m_chaseShowTempo(false)
    , m_chaseMultiplier(1.0)
    , m_chaseXfade(0)
    , m_chaseDirection(ChaseForward)
    , m_tcSource(TcInternal)
    , m_tcOffset(0)
    , m_runMode(ModeNormal)
    , m_preselectUid(0)
    , m_startUid(0)
    , m_lastUid(0)
    , m_currentUid(0)
    , m_current(-1)
    , m_cueElapsed(0)
    , m_cueLength(0)
    , m_fadeLength(0)
    , m_seekRemainder(0)
    , m_followArmed(false)
    , m_firstOfRun(true)
    , m_dataDirty(false)
    , m_releaseRequested(false)
    , m_liveScale(1.0)
    , m_ownTimes(true)
    , m_mibUid(0)
    , m_chaseElapsed(0)
    , m_bounceDir(1)
    , m_randomUid(0)
    , m_random(QRandomGenerator::global()->generate())
    , m_manual(false)
    , m_startManual(false)
    , m_manualLength(0)
    , m_manualPrevUid(0)
    , m_manualPreselect(0)
    , m_manualPosition(0.0)
    , m_tcClock(0)
    , m_tcLastClock(0)
    , m_tcUid(0)
    , m_tcLive(false)
    , m_fadersPaused(false)
    , m_appliedIntensity(-1)
    , m_appliedFactor(0)
    , m_fxPlayer(nullptr)
{
    setName(tr("New Cuelist"));
    // the fade of new cues, and the release time
    setFadeInSpeed(1000);
    setFadeOutSpeed(1000);

    // the FX of the cues play in a scene of its own, not in the show
    m_fxPlayer = new FXScene(doc);
    m_fxPlayer->setName(name() + QStringLiteral(" (FX)"));
}

Cuelist::~Cuelist()
{
    // the player runs only inside our own frames, and our postRun stops it
    // in the same frame: once the list has stopped (stopAndWait before a
    // delete, MasterTimer::stop before a clear) nothing holds it, and there
    // is nothing to wait for. A list deleted without a release (driven by
    // no timer thread) leaves no FX faders behind either. DMX Total Control:
    // nor FX it was fading out, held by a pause
    m_fxPlayer->setFadersPaused(false);
    if (m_fxPlayer->isRunning())
    {
        m_fxPlayer->setFadeOutSpeed(0);
        m_fxPlayer->postRun(doc() != NULL ? doc()->masterTimer() : NULL, QList<Universe *>());
    }
    delete m_fxPlayer;
}

QIcon Cuelist::getIcon() const
{
    return QIcon(":/sequence.png");
}

quint32 Cuelist::totalDuration()
{
    QMutexLocker locker(&m_mutex);
    if (m_cues.isEmpty())
        return 0;

    // DMX Total Control: a Chase goes round by itself: a lap of its steps
    if (m_mode == ModeChase)
    {
        const quint64 lap = quint64(chaseLapLocked()) * chaseBeatLocked();
        return lap >= quint64(infiniteSpeed()) ? infiniteSpeed() : quint32(lap);
    }
    // DMX Total Control: a Timecode list ends with the last cue the clock
    // plays (the last one in order with a time): its time and its length
    if (m_mode == ModeTimecode)
    {
        for (int i = int(m_cues.count()) - 1; i >= 0; i--)
        {
            const Cue &cue = m_cues.at(i);
            if (cue.time >= 0)
            {
                const quint64 end = quint64(cue.time) + cue.length();
                return end >= quint64(infiniteSpeed()) ? infiniteSpeed() : quint32(end);
            }
        }
        return 0;
    }

    // DMX Total Control: the chain of triggers from the first cue, through
    // the links of the cues: one cycle back to the first cue, or up to the
    // end. A cue that waits for a GO, or a link that loops for ever, has no end
    quint64 total = 0;
    int last = 0;
    switch (walkLocked(&total, &last))
    {
        case ChainCycle:
            break;
        case ChainLast:
        {
            // a Release list that releases itself after the last cue: with
            // the first cue's trigger; else the last cue plays out
            quint64 ms = 0;
            if (m_atEnd == AtEndRelease && gapLocked(last, 0, &ms))
                total += ms;
            else
                total += m_cues.at(last).length();
            break;
        }
        default:
            return infiniteSpeed();
    }
    return total >= quint64(infiniteSpeed()) ? infiniteSpeed() : quint32(total);
}

namespace
{
/** The steps a walk of the chain of a list takes at most */
const int kChainSteps = 100000;
/** DMX Total Control: the time a hand crossfade gives a cue with none, ms */
const uint kManualTime = 1000;
}

bool Cuelist::gapLocked(int from, int to, quint64 *ms) const
{
    const Cue &next = m_cues.at(to);
    *ms = next.triggerTime;
    if (next.trigger == TriggerFollow)
        *ms += m_cues.at(from).length();
    return next.trigger != TriggerGo;
}

Cuelist::ChainEnd Cuelist::walkLocked(quint64 *ms, int *last) const
{
    QHash<quint32, int> taken;
    // where it took a link that loops without a count, with the counts it
    // had: there again with the same counts, it goes round for ever
    QHash<int, QHash<quint32, int>> loops;
    int current = 0;
    for (int step = 0; step < kChainSteps; step++)
    {
        bool viaLink = false;
        const int next = linkNextLocked(current, taken, &viaLink);
        if (next < 0)
        {
            if (last != nullptr)
                *last = current;
            return ChainLast;
        }
        if (viaLink && m_cues.at(current).linkCount == 0)
        {
            auto seen = loops.constFind(current);
            if (seen != loops.cend() && seen.value() == taken)
                return ChainEndless;
            loops.insert(current, taken);
        }
        quint64 gap = 0;
        if (gapLocked(current, next, &gap) == false)
            return ChainGo;
        *ms += gap;
        countLink(m_cues.at(current), viaLink, taken);
        current = next;
        if (current == 0 && taken.isEmpty())
            return ChainCycle;
    }
    // (an endless loop of links with counts)
    return ChainEndless;
}

bool Cuelist::Cue::operator==(const Cue &o) const
{
    return uid == o.uid && number == o.number && name == o.name && note == o.note &&
           trigger == o.trigger && triggerTime == o.triggerTime && fade == o.fade &&
           downFade == o.downFade && delay == o.delay && curve == o.curve && skip == o.skip && block == o.block &&
           values == o.values && palettes == o.palettes && releases == o.releases &&
           fx == o.fx && fxReleases == o.fxReleases && groupTimes == o.groupTimes &&
           fixtureTimes == o.fixtureTimes && link == o.link && linkCount == o.linkCount && actions == o.actions &&
           noMib == o.noMib && time == o.time;
}

namespace
{
/** The times of a channel with the fixture part $f and the attribute part
 *  $g of $cue (see Cue::timesFor) */
Cuelist::Times resolveTimes(const Cuelist::Cue &cue, const Cuelist::PartTime &f, const Cuelist::PartTime &g)
{
    Cuelist::Times t;
    t.fade = f.fade >= 0 ? uint(f.fade) : g.fade >= 0 ? uint(g.fade) : cue.fade;
    t.down = f.fade >= 0 ? uint(f.fade) : cue.downFade >= 0 ? uint(cue.downFade) : (g.fade >= 0 ? uint(g.fade) : cue.fade);
    t.delay = f.delay >= 0 ? uint(f.delay) : g.delay >= 0 ? uint(g.delay) : cue.delay;
    t.curve = f.curve >= 0 ? f.curve : g.curve >= 0 ? g.curve : cue.curve;
    return t;
}
}

Cuelist::Times Cuelist::Cue::timesFor(quint32 fixture, int group) const
{
    return resolveTimes(*this, fixtureTimes.value(fixture), groupTimes.value(group));
}

uint Cuelist::Cue::length() const
{
    // every fixture part (and none) with every attribute part (and none)
    QList<PartTime> fixtureParts = fixtureTimes.values();
    fixtureParts.prepend(PartTime());
    QList<PartTime> groupParts = groupTimes.values();
    groupParts.prepend(PartTime());

    uint longest = 0;
    for (const PartTime &f : std::as_const(fixtureParts))
    {
        for (const PartTime &g : std::as_const(groupParts))
        {
            const Times t = resolveTimes(*this, f, g);
            longest = qMax(longest, t.delay + qMax(t.fade, t.down));
        }
    }
    return longest;
}

bool Cuelist::State::operator==(const State &o) const
{
    return values == o.values && owner == o.owner && palettes == o.palettes &&
           fx == o.fx && fxOwner == o.fxOwner;
}

/*********************************************************************
 * Copying
 *********************************************************************/

Function *Cuelist::createCopy(Doc *doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function *copy = new Cuelist(doc);
    if (copy->copyFrom(this) == false)
    {
        delete copy;
        copy = NULL;
    }
    if (addToDoc == true && copy != NULL && doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }
    return copy;
}

bool Cuelist::copyFrom(const Function *function)
{
    const Cuelist *cl = qobject_cast<const Cuelist *>(function);
    if (cl == NULL)
        return false;

    if (cl != this)
    {
        QMutexLocker locker(&m_mutex);
        QMutexLocker otherLocker(&cl->m_mutex);
        // the uids and the palette references too
        m_cues = cl->m_cues;
        m_nextUid = cl->m_nextUid;
        m_tracking = cl->m_tracking;
        m_atEnd = cl->m_atEnd;
        m_resetOnRelease = cl->m_resetOnRelease;
        m_backTime = cl->m_backTime;
        m_releaseOnNextGo = cl->m_releaseOnNextGo;
        m_moveInBlack = cl->m_moveInBlack;
        m_mibDelay = cl->m_mibDelay;
        m_mibFade = cl->m_mibFade;
        m_mode = cl->m_mode;
        m_chaseBpm = cl->m_chaseBpm;
        m_chaseShowTempo = cl->m_chaseShowTempo;
        m_chaseMultiplier = cl->m_chaseMultiplier;
        m_chaseXfade = cl->m_chaseXfade;
        m_chaseDirection = cl->m_chaseDirection;
        m_tcSource = cl->m_tcSource;
        m_tcOffset = cl->m_tcOffset;
        pruneLinks(m_cues);
        m_dataDirty = true;
        followEditsLocked();
    }

    return Function::copyFrom(function);
}

/*********************************************************************
 * Cue numbers
 *********************************************************************/

int Cuelist::parseNumber(const QString &text)
{
    const QString trimmed = text.trimmed();
    int whole = 0;
    int fraction = 0;
    int digits = 0;
    int decimals = 0;
    bool point = false;

    for (const QChar ch : trimmed)
    {
        const char16_t c = ch.unicode();
        if (c == u'.' || c == u',')
        {
            if (point)
                return -1;
            point = true;
            continue;
        }
        if (c < u'0' || c > u'9')
            return -1;

        const int digit = int(c - u'0');
        if (point)
        {
            if (++decimals > 3)
                return -1;
            fraction = fraction * 10 + digit;
        }
        else
        {
            whole = whole * 10 + digit;
            if (whole > maxNumber() / 1000)
                return -1;
            digits++;
        }
    }
    if (digits + decimals == 0)
        return -1;

    for (int d = decimals; d < 3; d++)
        fraction *= 10;
    const int number = whole * 1000 + fraction;
    return number >= 1 && number <= maxNumber() ? number : -1;
}

QString Cuelist::numberToString(int number)
{
    if (number < 0)
        return QString();

    QString text = QString::number(number / 1000);
    const int fraction = number % 1000;
    if (fraction != 0)
    {
        QString decimals = QString::number(fraction).rightJustified(3, QLatin1Char('0'));
        while (decimals.endsWith(QLatin1Char('0')))
            decimals.chop(1);
        text += QLatin1Char('.') + decimals;
    }
    return text;
}

QStringList Cuelist::fxAxes(const QString &param)
{
    if (param == QLatin1String("pantilt"))
        return { QStringLiteral("pan"), QStringLiteral("tilt") };
    return { param };
}

QSet<Cuelist::FxKey> Cuelist::fxKeysOf(const FXScene::Layer &layer)
{
    QSet<FxKey> keys;
    const QStringList axes = fxAxes(layer.param);
    for (quint32 fixture : layer.fixtures)
        for (const QString &axis : axes)
            keys.insert(FxKey(axis, fixture));
    return keys;
}

int Cuelist::timeGroupOf(const QLCChannel *ch)
{
    if (ch == NULL)
        return -1;

    // the families of the palettes (PaletteManager::typeOf in the app), but
    // every channel of the pan and tilt moves with the position, rotations
    // included: a palette doesn't record them, a cue times them
    const QString attribute = FXScene::attributeOf(ch);
    switch (ch->group())
    {
        case QLCChannel::Intensity:
            if (ch->colour() == QLCChannel::NoColour && attribute != QLatin1String("hue") &&
                attribute != QLatin1String("saturation") && attribute != QLatin1String("lightness"))
                return TimeIntensity;
            return TimeColour;
        // the strobe goes with the intensity, the iris with the beam
        case QLCChannel::Shutter:
            return attribute == QLatin1String("iris") ? TimeBeam : TimeIntensity;
        case QLCChannel::Pan:
        case QLCChannel::Tilt:
            return TimePosition;
        case QLCChannel::Colour:
            return TimeColour;
        case QLCChannel::Speed:
            return attribute == QLatin1String("ptspeed") ? TimePosition : TimeBeam;
        case QLCChannel::Gobo:
        case QLCChannel::Prism:
        case QLCChannel::Beam:
        case QLCChannel::Effect:
            return TimeBeam;
        default:
            return -1;
    }
}

QString Cuelist::timeGroupToString(int group)
{
    switch (group)
    {
        case TimeIntensity: return QStringLiteral("intensity");
        case TimeColour: return QStringLiteral("colour");
        case TimePosition: return QStringLiteral("position");
        case TimeBeam: return QStringLiteral("beam");
        default: return QString();
    }
}

int Cuelist::stringToTimeGroup(const QString &str)
{
    for (int group = 0; group < TimeGroupCount; group++)
    {
        if (str.compare(timeGroupToString(group), Qt::CaseInsensitive) == 0)
            return group;
    }
    return -1;
}

QString Cuelist::curveToString(int curve)
{
    switch (curve)
    {
        case FadeChannel::CurveSmooth: return QStringLiteral("smooth");
        case FadeChannel::CurveSlowStart: return QStringLiteral("slowstart");
        case FadeChannel::CurveFastStart: return QStringLiteral("faststart");
        default: return QStringLiteral("linear");
    }
}

int Cuelist::stringToCurve(const QString &str)
{
    for (int curve = FadeChannel::CurveSmooth; curve <= FadeChannel::CurveFastStart; curve++)
    {
        if (str.compare(curveToString(curve), Qt::CaseInsensitive) == 0)
            return curve;
    }
    return FadeChannel::CurveLinear;
}

/*********************************************************************
 * Contents
 *********************************************************************/

int Cuelist::cueCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_cues.count();
}

QList<Cuelist::Cue> Cuelist::cues() const
{
    QMutexLocker locker(&m_mutex);
    return m_cues;
}

Cuelist::Cue Cuelist::cueAt(int index) const
{
    QMutexLocker locker(&m_mutex);
    return index >= 0 && index < m_cues.count() ? m_cues.at(index) : Cue();
}

int Cuelist::indexOfUid(quint32 uid) const
{
    QMutexLocker locker(&m_mutex);
    return indexOfUidLocked(uid);
}

int Cuelist::indexOfNumber(int number) const
{
    QMutexLocker locker(&m_mutex);
    return indexOfNumberLocked(number);
}

int Cuelist::insertIndexFor(int number) const
{
    QMutexLocker locker(&m_mutex);
    return insertIndexLocked(number);
}

int Cuelist::indexOfUidLocked(quint32 uid) const
{
    if (uid == 0)
        return -1;
    for (int i = 0; i < m_cues.count(); i++)
    {
        if (m_cues.at(i).uid == uid)
            return i;
    }
    return -1;
}

int Cuelist::indexOfNumberLocked(int number) const
{
    const int i = insertIndexLocked(number);
    return i < m_cues.count() && m_cues.at(i).number == number ? i : -1;
}

int Cuelist::insertIndexLocked(int number) const
{
    // the first cue that doesn't come before it (the list is sorted)
    auto it = std::lower_bound(m_cues.cbegin(), m_cues.cend(), number,
                               [](const Cue &cue, int n) { return cue.number < n; });
    return int(it - m_cues.cbegin());
}

int Cuelist::addCue(const Cue &cue)
{
    if (cue.number < 1 || cue.number > maxNumber())
        return -1;

    int index = -1;
    {
        QMutexLocker locker(&m_mutex);
        if (indexOfNumberLocked(cue.number) >= 0)
            return -1;

        Cue added = cue;
        sanitize(added);
        if (added.uid == 0 || indexOfUidLocked(added.uid) >= 0)
            added.uid = m_nextUid++;
        else
            m_nextUid = qMax(m_nextUid, added.uid + 1);

        index = insertIndexLocked(added.number);
        m_cues.insert(index, added);
        // (its link to a cue the list doesn't have)
        pruneLinks(m_cues);
        m_dataDirty = true;
        followEditsLocked();
    }
    emit changed(id());
    return index;
}

bool Cuelist::replaceCue(const Cue &cue)
{
    if (cue.number < 1 || cue.number > maxNumber())
        return false;

    {
        QMutexLocker locker(&m_mutex);
        const int from = indexOfUidLocked(cue.uid);
        if (from < 0)
            return false;
        const int sameNumber = indexOfNumberLocked(cue.number);
        if (sameNumber >= 0 && sameNumber != from)
            return false;

        Cue replaced = cue;
        sanitize(replaced);
        // (a link to a cue the list doesn't have goes)
        if (replaced.link != 0 && indexOfUidLocked(replaced.link) < 0)
        {
            replaced.link = 0;
            replaced.linkCount = 0;
        }
        if (replaced == m_cues.at(from))
            return true;

        // a new number moves it
        m_cues.removeAt(from);
        m_cues.insert(insertIndexLocked(replaced.number), replaced);
        m_dataDirty = true;
        followEditsLocked();
    }
    emit changed(id());
    return true;
}

bool Cuelist::removeCue(quint32 uid)
{
    {
        QMutexLocker locker(&m_mutex);
        const int index = indexOfUidLocked(uid);
        if (index < 0)
            return false;
        m_cues.removeAt(index);
        // the links to it go
        pruneLinks(m_cues);
        m_dataDirty = true;
        followEditsLocked();
    }
    emit changed(id());
    return true;
}

void Cuelist::setCues(const QList<Cue> &cues)
{
    {
        QMutexLocker locker(&m_mutex);
        QList<Cue> list = cues;
        normalizeLocked(list);
        if (list == m_cues)
            return;
        m_cues = list;
        m_dataDirty = true;
        followEditsLocked();
    }
    emit changed(id());
}

void Cuelist::normalizeLocked(QList<Cue> &cues)
{
    // sorted by number; of two cues with the same number the first one stays
    std::stable_sort(cues.begin(), cues.end(), [](const Cue &a, const Cue &b) { return a.number < b.number; });

    QList<Cue> kept;
    for (const Cue &cue : std::as_const(cues))
    {
        if (cue.number < 1 || cue.number > maxNumber())
        {
            qWarning() << "Cuelist" << name() << ": cue number" << cue.number << "is not valid, cue dropped";
            continue;
        }
        if (kept.isEmpty() == false && kept.last().number == cue.number)
        {
            qWarning() << "Cuelist" << name() << ": cue" << numberToString(cue.number) << "twice, the second one dropped";
            continue;
        }
        kept.append(cue);
    }

    // uids are unique: a cue without one, or with the uid of a cue before
    // it, gets a new one past every uid of the list
    QSet<quint32> used;
    QList<int> fresh;
    for (int i = 0; i < kept.count(); i++)
    {
        Cue &cue = kept[i];
        sanitize(cue);
        if (cue.uid == 0 || used.contains(cue.uid))
        {
            fresh.append(i);
            continue;
        }
        used.insert(cue.uid);
        m_nextUid = qMax(m_nextUid, cue.uid + 1);
    }
    for (int i : std::as_const(fresh))
        kept[i].uid = m_nextUid++;

    // links only to cues of the list
    pruneLinks(kept);
    cues = kept;
}

bool Cuelist::pruneLinks(QList<Cue> &cues)
{
    QSet<quint32> uids;
    for (const Cue &cue : std::as_const(cues))
        uids.insert(cue.uid);

    bool pruned = false;
    for (Cue &cue : cues)
    {
        if (cue.link != 0 && uids.contains(cue.link) == false)
        {
            cue.link = 0;
            cue.linkCount = 0;
            pruned = true;
        }
    }
    return pruned;
}

void Cuelist::sanitize(Cue &cue)
{
    // a value wins over a release of the same channel
    for (auto it = cue.values.cbegin(); it != cue.values.cend(); ++it)
        cue.releases.remove(it.key());

    // palettes only refer to values
    for (auto it = cue.palettes.begin(); it != cue.palettes.end();)
    {
        if (cue.values.contains(it.key()))
            ++it;
        else
            it = cue.palettes.erase(it);
    }

    // FX layers have a parameter and their own fixtures, once each
    for (int l = cue.fx.count() - 1; l >= 0; l--)
    {
        FXScene::Layer &layer = cue.fx[l];
        QList<quint32> unique;
        for (quint32 fixture : std::as_const(layer.fixtures))
            if (unique.contains(fixture) == false)
                unique.append(fixture);
        if (unique.count() != layer.fixtures.count())
            layer.fixtures = unique;
        if (layer.param.isEmpty() || layer.fixtures.isEmpty())
            cue.fx.removeAt(l);
    }

    // an FX wins over a release of the same slot
    for (const FXScene::Layer &layer : std::as_const(cue.fx))
        for (const FxKey &key : fxKeysOf(layer))
            cue.fxReleases.remove(key);

    if (cue.downFade < -1)
        cue.downFade = -1;
    cue.curve = qBound(int(FadeChannel::CurveLinear), cue.curve, int(FadeChannel::CurveFastStart));

    // part times: -1 (the cue's) up to an hour, the four attributes, and
    // only parts with a time (or a curve)
    auto tidy = [](PartTime &part) {
        part.fade = qBound(-1, part.fade, 3600000);
        part.delay = qBound(-1, part.delay, 3600000);
        part.curve = qBound(-1, part.curve, int(FadeChannel::CurveFastStart));
        return part.isEmpty();
    };
    cue.groupTimes.removeIf([&](QMap<int, PartTime>::iterator it) {
        return it.key() < 0 || it.key() >= TimeGroupCount || tidy(it.value());
    });
    cue.fixtureTimes.removeIf([&](QMap<quint32, PartTime>::iterator it) { return tidy(it.value()); });

    // DMX Total Control: a link is taken up to 999 times (0: always); no
    // link, no count. An action names what it does
    cue.linkCount = cue.link == 0 ? 0 : qBound(0, cue.linkCount, 999);
    cue.actions.removeIf([](const CueAction &action) { return action.value(QStringLiteral("target")).isEmpty(); });
    // DMX Total Control: a time on the clock, or none (-1)
    cue.time = qBound(-1, cue.time, maxCueTime());
}

bool Cuelist::removeFixturesLocked(const std::function<bool(quint32)> &remove)
{
    bool removed = false;
    for (Cue &cue : m_cues)
    {
        for (auto it = cue.values.begin(); it != cue.values.end();)
        {
            if (remove(it.key().first))
            {
                it = cue.values.erase(it);
                removed = true;
            }
            else
            {
                ++it;
            }
        }
        for (auto it = cue.palettes.begin(); it != cue.palettes.end();)
        {
            if (remove(it.key().first))
                it = cue.palettes.erase(it);
            else
                ++it;
        }
        for (auto it = cue.releases.begin(); it != cue.releases.end();)
        {
            if (remove(it->first))
            {
                it = cue.releases.erase(it);
                removed = true;
            }
            else
            {
                ++it;
            }
        }
        for (int l = cue.fx.count() - 1; l >= 0; l--)
        {
            FXScene::Layer &layer = cue.fx[l];
            if (layer.fixtures.removeIf(remove) > 0)
            {
                removed = true;
                layer.own = layer.tidyOwn();
            }
            if (layer.fixtures.isEmpty())
                cue.fx.removeAt(l);
        }
        for (auto it = cue.fxReleases.begin(); it != cue.fxReleases.end();)
        {
            if (remove(it->second))
            {
                it = cue.fxReleases.erase(it);
                removed = true;
            }
            else
            {
                ++it;
            }
        }
        if (cue.fixtureTimes.removeIf([&](QMap<quint32, PartTime>::iterator it) { return remove(it.key()); }) > 0)
            removed = true;
    }
    return removed;
}

/*********************************************************************
 * Tracking
 *********************************************************************/

namespace
{
/** An FX layer of a cue: (cue uid, layer index) */
typedef QPair<quint32, int> FxOrigin;

/** A layer, and the fixtures it still holds */
struct FxHeld
{
    FXScene::Layer layer;
    QSet<quint32> owned;
};

/** The fold of the cues: the state, and which layer holds each FX slot */
struct Fold
{
    Cuelist::State state;
    QList<FxOrigin> order;
    QHash<FxOrigin, FxHeld> layers;
    QHash<Cuelist::FxKey, FxOrigin> holders;

    /** A layer that loses one slot of a fixture loses the whole fixture: a layer can't be split */
    void loseFixture(const FxOrigin &origin, quint32 fixture)
    {
        auto it = layers.find(origin);
        if (it == layers.end())
            return;
        it->owned.remove(fixture);
        for (const QString &axis : Cuelist::fxAxes(it->layer.param))
        {
            auto held = holders.find(Cuelist::FxKey(axis, fixture));
            if (held != holders.end() && held.value() == origin)
                holders.erase(held);
        }
    }

    void apply(const Cuelist::Cue &cue)
    {
        for (const Cuelist::Key &key : cue.releases)
        {
            state.values.remove(key);
            state.owner.remove(key);
            state.palettes.remove(key);
        }
        for (auto it = cue.values.cbegin(); it != cue.values.cend(); ++it)
        {
            state.values.insert(it.key(), it.value());
            state.owner.insert(it.key(), cue.uid);
            auto palette = cue.palettes.constFind(it.key());
            if (palette != cue.palettes.cend())
                state.palettes.insert(it.key(), palette.value());
            else
                state.palettes.remove(it.key());
        }

        for (const Cuelist::FxKey &key : cue.fxReleases)
        {
            auto held = holders.constFind(key);
            if (held != holders.cend())
                loseFixture(FxOrigin(held.value()), key.second);
        }
        for (int l = 0; l < cue.fx.count(); l++)
        {
            const FXScene::Layer &layer = cue.fx.at(l);
            const FxOrigin origin(cue.uid, l);
            const QStringList axes = Cuelist::fxAxes(layer.param);
            FxHeld held;
            held.layer = layer;
            for (quint32 fixture : layer.fixtures)
            {
                for (const QString &axis : axes)
                {
                    const Cuelist::FxKey key(axis, fixture);
                    auto other = holders.constFind(key);
                    if (other != holders.cend() && other.value() != origin)
                        loseFixture(FxOrigin(other.value()), fixture);
                    holders.insert(key, origin);
                }
                held.owned.insert(fixture);
            }
            layers.insert(origin, held);
            order.append(origin);
        }
    }

    /** The state, with each tracked layer on the fixtures it still holds (in its order) */
    Cuelist::State finish()
    {
        for (const FxOrigin &origin : std::as_const(order))
        {
            const FxHeld held = layers.value(origin);
            if (held.owned.isEmpty())
                continue;
            FXScene::Layer layer = held.layer;
            layer.fixtures.clear();
            for (quint32 fixture : held.layer.fixtures)
                if (held.owned.contains(fixture))
                    layer.fixtures.append(fixture);
            layer.own = layer.tidyOwn();
            state.fx.append(layer);
        }
        for (auto it = holders.cbegin(); it != holders.cend(); ++it)
            state.fxOwner.insert(it.key(), it.value().first);
        return state;
    }
};
}

Cuelist::State Cuelist::foldLocked(int index, bool withCue) const
{
    Fold fold;
    const int count = m_cues.count();
    // a skip cue (cue only) doesn't track on
    if (m_tracking)
    {
        for (int i = 0; i < qMin(index, count); i++)
        {
            if (m_cues.at(i).skip == false)
                fold.apply(m_cues.at(i));
        }
    }
    // a cue always shows itself, skip included
    if (withCue && index >= 0 && index < count)
        fold.apply(m_cues.at(index));
    return fold.finish();
}

Cuelist::State Cuelist::stateBefore(int index) const
{
    QMutexLocker locker(&m_mutex);
    return foldLocked(index, false);
}

Cuelist::State Cuelist::trackedState(int index) const
{
    QMutexLocker locker(&m_mutex);
    if (index < 0 || index >= m_cues.count())
        return State();
    return foldLocked(index, true);
}

QList<Cuelist::State> Cuelist::trackedStates(QList<Cue> *cues) const
{
    QMutexLocker locker(&m_mutex);
    if (cues != nullptr)
        *cues = m_cues;
    // as foldLocked(), the cues before each one folded once (a fold is cheap
    // to copy: what it holds is shared until it changes)
    QList<State> states;
    states.reserve(m_cues.count());
    Fold tracked;
    for (const Cue &cue : m_cues)
    {
        // a skip cue (cue only) doesn't track on, but shows itself
        if (m_tracking && cue.skip == false)
        {
            tracked.apply(cue);
            Fold fold = tracked;
            states.append(fold.finish());
        }
        else
        {
            Fold fold = tracked;
            fold.apply(cue);
            states.append(fold.finish());
        }
    }
    return states;
}

/*********************************************************************
 * Palettes
 *********************************************************************/

void Cuelist::applyPalette(int paletteId, const std::function<int(quint32, quint32)> &valueOf)
{
    if (!valueOf)
        return;

    // the channels that use it, then their values outside the lock (the
    // caller reads its own palettes)
    QSet<Key> keys;
    {
        QMutexLocker locker(&m_mutex);
        for (const Cue &cue : std::as_const(m_cues))
            for (auto it = cue.palettes.cbegin(); it != cue.palettes.cend(); ++it)
                if (it.value() == paletteId)
                    keys.insert(it.key());
    }
    if (keys.isEmpty())
        return;

    QHash<Key, int> newValues;
    for (const Key &key : std::as_const(keys))
        newValues.insert(key, valueOf(key.first, key.second));

    bool modified = false;
    {
        QMutexLocker locker(&m_mutex);
        for (Cue &cue : m_cues)
        {
            for (auto it = cue.palettes.begin(); it != cue.palettes.end();)
            {
                auto value = newValues.constFind(it.key());
                if (it.value() != paletteId || value == newValues.cend())
                {
                    ++it;
                    continue;
                }
                // the palette doesn't have this channel any more: a plain value
                if (value.value() < 0)
                {
                    it = cue.palettes.erase(it);
                    modified = true;
                    continue;
                }
                const uchar v = uchar(qBound(0, value.value(), 255));
                auto current = cue.values.find(it.key());
                if (current != cue.values.end() && current.value() != v)
                {
                    current.value() = v;
                    modified = true;
                }
                ++it;
            }
        }
        if (modified)
            m_dataDirty = true;
    }
    if (modified)
        emit changed(id());
}

void Cuelist::forgetPalette(int paletteId)
{
    bool modified = false;
    {
        QMutexLocker locker(&m_mutex);
        for (Cue &cue : m_cues)
        {
            for (auto it = cue.palettes.begin(); it != cue.palettes.end();)
            {
                if (it.value() == paletteId)
                {
                    it = cue.palettes.erase(it);
                    modified = true;
                }
                else
                {
                    ++it;
                }
            }
        }
        if (modified)
            m_dataDirty = true;
    }
    if (modified)
        emit changed(id());
}

/*********************************************************************
 * Options
 *********************************************************************/

bool Cuelist::tracking() const
{
    QMutexLocker locker(&m_mutex);
    return m_tracking;
}

void Cuelist::setTracking(bool on)
{
    {
        QMutexLocker locker(&m_mutex);
        if (on == m_tracking)
            return;
        m_tracking = on;
        // what each cue shows changes
        m_dataDirty = true;
    }
    emit changed(id());
}

Cuelist::Mode Cuelist::mode() const
{
    QMutexLocker locker(&m_mutex);
    return m_mode;
}

void Cuelist::setMode(Mode mode)
{
    {
        QMutexLocker locker(&m_mutex);
        if (mode == m_mode)
            return;
        m_mode = mode;
        // (a Chase counts its step from now)
        m_chaseElapsed = 0;
        // DMX Total Control: a list that plays goes on from its cue as a
        // Timecode list: the clock runs from the time of that cue
        if (mode == ModeTimecode)
        {
            m_tcClock = m_current >= 0 ? tcTimeLocked(m_current) : 0;
            m_tcLastClock = m_tcClock;
            const int pick = tcPickLocked(m_tcClock);
            m_tcUid = pick >= 0 ? m_cues.at(pick).uid : 0;
        }
        updateStatusLocked();
    }
    emit changed(id());
}

Cuelist::AtEnd Cuelist::atEnd() const
{
    QMutexLocker locker(&m_mutex);
    return m_atEnd;
}

void Cuelist::setAtEnd(AtEnd mode)
{
    {
        QMutexLocker locker(&m_mutex);
        if (mode == m_atEnd)
            return;
        m_atEnd = mode;
    }
    emit changed(id());
}

bool Cuelist::resetOnRelease() const
{
    QMutexLocker locker(&m_mutex);
    return m_resetOnRelease;
}

void Cuelist::setResetOnRelease(bool on)
{
    {
        QMutexLocker locker(&m_mutex);
        if (on == m_resetOnRelease)
            return;
        m_resetOnRelease = on;
    }
    emit changed(id());
}

int Cuelist::backTime() const
{
    QMutexLocker locker(&m_mutex);
    return m_backTime;
}

void Cuelist::setBackTime(int ms)
{
    ms = qMax(-1, ms);
    {
        QMutexLocker locker(&m_mutex);
        if (ms == m_backTime)
            return;
        m_backTime = ms;
    }
    emit changed(id());
}

bool Cuelist::releaseOnNextGo() const
{
    QMutexLocker locker(&m_mutex);
    return m_releaseOnNextGo;
}

void Cuelist::setReleaseOnNextGo(bool on)
{
    {
        QMutexLocker locker(&m_mutex);
        if (on == m_releaseOnNextGo)
            return;
        m_releaseOnNextGo = on;
    }
    emit changed(id());
}

bool Cuelist::moveInBlack() const
{
    QMutexLocker locker(&m_mutex);
    return m_moveInBlack;
}

void Cuelist::setMoveInBlack(bool on)
{
    {
        QMutexLocker locker(&m_mutex);
        if (on == m_moveInBlack)
            return;
        m_moveInBlack = on;
        // switched on: the next cue is prepared now. Off: what was prepared
        // stays, dark, until the next GO
        m_mibUid = 0;
    }
    emit changed(id());
}

uint Cuelist::mibDelay() const
{
    QMutexLocker locker(&m_mutex);
    return m_mibDelay;
}

void Cuelist::setMibDelay(uint ms)
{
    ms = qMin(ms, 3600000u);
    {
        QMutexLocker locker(&m_mutex);
        if (ms == m_mibDelay)
            return;
        m_mibDelay = ms;
        m_mibUid = 0;
    }
    emit changed(id());
}

uint Cuelist::mibFade() const
{
    QMutexLocker locker(&m_mutex);
    return m_mibFade;
}

void Cuelist::setMibFade(uint ms)
{
    ms = qMin(ms, 3600000u);
    {
        QMutexLocker locker(&m_mutex);
        if (ms == m_mibFade)
            return;
        m_mibFade = ms;
        m_mibUid = 0;
    }
    emit changed(id());
}

int Cuelist::chaseBpm() const
{
    QMutexLocker locker(&m_mutex);
    return m_chaseBpm;
}

void Cuelist::setChaseBpm(int bpm)
{
    bpm = qBound(10, bpm, 600);
    {
        QMutexLocker locker(&m_mutex);
        if (bpm == m_chaseBpm)
            return;
        m_chaseBpm = bpm;
    }
    emit changed(id());
}

bool Cuelist::chaseShowTempo() const
{
    QMutexLocker locker(&m_mutex);
    return m_chaseShowTempo;
}

void Cuelist::setChaseShowTempo(bool on)
{
    {
        QMutexLocker locker(&m_mutex);
        if (on == m_chaseShowTempo)
            return;
        m_chaseShowTempo = on;
    }
    emit changed(id());
}

qreal Cuelist::chaseMultiplier() const
{
    QMutexLocker locker(&m_mutex);
    return m_chaseMultiplier;
}

void Cuelist::setChaseMultiplier(qreal factor)
{
    factor = chaseMultiplierOf(factor);
    {
        QMutexLocker locker(&m_mutex);
        if (factor == m_chaseMultiplier)
            return;
        m_chaseMultiplier = factor;
    }
    emit changed(id());
}

int Cuelist::chaseXfade() const
{
    QMutexLocker locker(&m_mutex);
    return m_chaseXfade;
}

void Cuelist::setChaseXfade(int percent)
{
    percent = qBound(-1, percent, 100);
    {
        QMutexLocker locker(&m_mutex);
        if (percent == m_chaseXfade)
            return;
        m_chaseXfade = percent;
    }
    emit changed(id());
}

Cuelist::ChaseDirection Cuelist::chaseDirection() const
{
    QMutexLocker locker(&m_mutex);
    return m_chaseDirection;
}

void Cuelist::setChaseDirection(ChaseDirection direction)
{
    {
        QMutexLocker locker(&m_mutex);
        if (direction == m_chaseDirection)
            return;
        m_chaseDirection = direction;
        // (a Bounce starts on its way up; Random draws its next step again)
        m_bounceDir = 1;
        m_randomUid = 0;
        updateStatusLocked();
    }
    emit changed(id());
}

qreal Cuelist::effectiveChaseBpm() const
{
    QMutexLocker locker(&m_mutex);
    if (m_mode != ModeChase)
        return 0;
    const uint tap = tapPeriod();
    if (tap > 0)
        return 60000.0 / qreal(tap);
    qreal bpm = m_chaseBpm;
    const MasterTimer *timer = doc() != NULL ? doc()->masterTimer() : NULL;
    if (m_chaseShowTempo && timer != NULL && timer->bpmNumber() > 0)
        bpm = timer->bpmNumber();
    return bpm * m_chaseMultiplier;
}

Cuelist::TcSource Cuelist::tcSource() const
{
    QMutexLocker locker(&m_mutex);
    return m_tcSource;
}

void Cuelist::setTcSource(TcSource source)
{
    {
        QMutexLocker locker(&m_mutex);
        if (source == m_tcSource)
            return;
        m_tcSource = source;
    }
    emit changed(id());
}

uint Cuelist::tcOffset() const
{
    QMutexLocker locker(&m_mutex);
    return m_tcOffset;
}

void Cuelist::setTcOffset(uint ms)
{
    ms = qMin(ms, uint(maxCueTime()));
    {
        QMutexLocker locker(&m_mutex);
        if (ms == m_tcOffset)
            return;
        m_tcOffset = ms;
    }
    emit changed(id());
}

/*********************************************************************
 * Transport
 *********************************************************************/

void Cuelist::go()
{
    // GO on a paused list resumes its fades, it doesn't go on
    if (isRunning() && isPaused())
    {
        setPause(false);
        return;
    }

    // a stopped list drops it: nothing waits for the next run. Started and
    // not run yet, it is kept: preRun plays the start before it
    QMutexLocker locker(&m_mutex);
    if (stopped())
        return;
    m_actions.append(Action());
}

void Cuelist::back()
{
    // a paused list goes back too: write() waits while paused
    if (isRunning() && isPaused())
        setPause(false);

    Action action;
    action.type = ActionBack;
    // (the cue it goes back to doesn't do its actions again)
    action.actions = false;
    QMutexLocker locker(&m_mutex);
    if (stopped())
        return;
    m_actions.append(action);
}

void Cuelist::goToCue(int index, bool snap)
{
    QMutexLocker locker(&m_mutex);
    if (stopped() || index < 0 || index >= m_cues.count())
        return;
    if (isRunning() && isPaused())
        setPause(false);
    Action action;
    action.type = ActionGoTo;
    action.uid = m_cues.at(index).uid;
    action.snap = snap;
    m_actions.append(action);
}

void Cuelist::setNextCue(int index)
{
    QMutexLocker locker(&m_mutex);
    m_preselectUid = index >= 0 && index < m_cues.count() ? m_cues.at(index).uid : 0;
    // published at once: a paused or stopped list has no write() to do it
    updateStatusLocked();
}

void Cuelist::setStartCue(int index)
{
    QMutexLocker locker(&m_mutex);
    m_startUid = index >= 0 && index < m_cues.count() ? m_cues.at(index).uid : 0;
    // a stopped list shows the new start as its next cue
    updateStatusLocked();
}

int Cuelist::nextIndex() const
{
    QMutexLocker locker(&m_mutex);
    // stopped: where the next start begins. Right after stop() the run is
    // still on its cue until postRun (the next frame): the one to resume from
    if (stopped())
        return startIndexLocked(m_current >= 0 ? m_currentUid : m_lastUid);
    return nextLocked(nullptr);
}

Cuelist::Status Cuelist::status() const
{
    QMutexLocker locker(&m_statusMutex);
    return m_status;
}

QSet<Cuelist::Key> Cuelist::takeChangedKeys()
{
    QSet<Key> keys;
    QMutexLocker locker(&m_mutex);
    keys.swap(m_changed);
    return keys;
}

QList<FXScene::Layer> Cuelist::playingFx() const
{
    QMutexLocker locker(&m_mutex);
    return m_fxPlaying;
}

void Cuelist::setLiveTime(uint ms)
{
    // (as a GO: write() does it, in the timer thread; a paused list when it goes on)
    Action action;
    action.type = ActionLiveTime;
    action.ms = ms;
    QMutexLocker locker(&m_mutex);
    if (stopped())
        return;
    m_actions.append(action);
}

void Cuelist::scaleLiveTime(qreal factor)
{
    if (factor <= 0)
        return;
    Action action;
    action.type = ActionLiveScale;
    action.scale = factor;
    QMutexLocker locker(&m_mutex);
    if (stopped())
        return;
    m_actions.append(action);
}

void Cuelist::liveTimeStored()
{
    QMutexLocker locker(&m_mutex);
    m_liveScale = 1.0;
    updateStatusLocked();
}

qreal Cuelist::timeFactor() const
{
    const MasterTimer *timer = doc() != NULL ? doc()->masterTimer() : NULL;
    return speedFactor() * (timer != NULL ? timer->cueSpeed() : 1.0);
}

void Cuelist::manualGo()
{
    // (a paused list goes on: write() waits while paused)
    if (isRunning() && isPaused())
        setPause(false);

    Action action;
    action.type = ActionManualGo;
    QMutexLocker locker(&m_mutex);
    if (stopped())
        return;
    // the travel starts at its end: an older position doesn't count
    m_manualPosition.store(0.0);
    m_actions.append(action);
}

void Cuelist::setStartManual(bool on)
{
    QMutexLocker locker(&m_mutex);
    m_startManual = on;
    if (on)
        m_manualPosition.store(0.0);
}

void Cuelist::setManualPosition(qreal position)
{
    m_manualPosition.store(qBound(0.0, position, 1.0));
}

void Cuelist::endManual(ManualEnd how)
{
    // (as a GO: write() does it, in the order they came)
    Action action;
    action.type = ActionManualEnd;
    action.end = how;
    QMutexLocker locker(&m_mutex);
    if (stopped())
        return;
    m_actions.append(action);
}

/*********************************************************************
 * Fixtures
 *********************************************************************/

void Cuelist::slotFixtureRemoved(quint32 fxi_id)
{
    {
        QMutexLocker locker(&m_mutex);
        if (removeFixturesLocked([fxi_id](quint32 fixture) { return fixture == fxi_id; }) == false)
            return;
        m_dataDirty = true;
    }
    emit changed(id());
}

/*********************************************************************
 * Save & Load
 *********************************************************************/

QString Cuelist::triggerToString(Trigger trigger)
{
    switch (trigger)
    {
        case TriggerFollow: return QStringLiteral("Follow");
        case TriggerWait: return QStringLiteral("Wait");
        default: return QStringLiteral("Go");
    }
}

Cuelist::Trigger Cuelist::stringToTrigger(const QString &str)
{
    if (str.compare(QLatin1String("Follow"), Qt::CaseInsensitive) == 0)
        return TriggerFollow;
    if (str.compare(QLatin1String("Wait"), Qt::CaseInsensitive) == 0)
        return TriggerWait;
    return TriggerGo;
}

QString Cuelist::atEndToString(AtEnd mode)
{
    switch (mode)
    {
        case AtEndRelease: return QStringLiteral("Release");
        case AtEndStay: return QStringLiteral("Stay");
        default: return QStringLiteral("Loop");
    }
}

Cuelist::AtEnd Cuelist::stringToAtEnd(const QString &str)
{
    if (str.compare(QLatin1String("Release"), Qt::CaseInsensitive) == 0)
        return AtEndRelease;
    if (str.compare(QLatin1String("Stay"), Qt::CaseInsensitive) == 0)
        return AtEndStay;
    return AtEndLoop;
}

QString Cuelist::modeToString(Mode mode)
{
    switch (mode)
    {
        case ModeChase: return QStringLiteral("Chase");
        case ModeTimecode: return QStringLiteral("Timecode");
        case ModeSubmaster: return QStringLiteral("Submaster");
        case ModeOverride: return QStringLiteral("Override");
        default: return QStringLiteral("Normal");
    }
}

Cuelist::Mode Cuelist::stringToMode(const QString &str)
{
    for (Mode mode : { ModeChase, ModeTimecode, ModeSubmaster, ModeOverride })
    {
        if (str.compare(modeToString(mode), Qt::CaseInsensitive) == 0)
            return mode;
    }
    return ModeNormal;
}

QString Cuelist::chaseDirectionToString(ChaseDirection direction)
{
    switch (direction)
    {
        case ChaseBackward: return QStringLiteral("Backward");
        case ChaseBounce: return QStringLiteral("Bounce");
        case ChaseRandom: return QStringLiteral("Random");
        default: return QStringLiteral("Forward");
    }
}

Cuelist::ChaseDirection Cuelist::stringToChaseDirection(const QString &str)
{
    for (ChaseDirection direction : { ChaseBackward, ChaseBounce, ChaseRandom })
    {
        if (str.compare(chaseDirectionToString(direction), Qt::CaseInsensitive) == 0)
            return direction;
    }
    return ChaseForward;
}

qreal Cuelist::chaseMultiplierOf(qreal factor)
{
    // (NaN, 0 and below: x1)
    if (!(factor > 0) || std::isinf(factor))
        return 1.0;
    // the nearest on a log scale: x3 is x4, x1.4 is x1
    qreal best = 1.0;
    for (qreal multiplier : { 0.25, 0.5, 2.0, 4.0 })
    {
        if (std::abs(std::log2(factor / multiplier)) < std::abs(std::log2(factor / best)))
            best = multiplier;
    }
    return best;
}

QString Cuelist::tcSourceToString(TcSource source)
{
    return source == TcMtc ? QStringLiteral("Mtc") : QStringLiteral("Internal");
}

Cuelist::TcSource Cuelist::stringToTcSource(const QString &str)
{
    return str.compare(QLatin1String("Mtc"), Qt::CaseInsensitive) == 0 ? TcMtc : TcInternal;
}

void Cuelist::saveCueXML(QXmlStreamWriter *doc, const Cue &cue)
{
    doc->writeStartElement(KXMLDTCCue);
    doc->writeAttribute("Uid", QString::number(cue.uid));
    // thousandths: the locale never matters
    doc->writeAttribute("Number", QString::number(cue.number));
    if (cue.name.isEmpty() == false)
        doc->writeAttribute("Name", cue.name);
    if (cue.trigger != TriggerGo)
        doc->writeAttribute("Trigger", triggerToString(cue.trigger));
    if (cue.triggerTime != 0)
        doc->writeAttribute("TriggerTime", QString::number(cue.triggerTime));
    doc->writeAttribute("Fade", QString::number(cue.fade));
    if (cue.downFade >= 0)
        doc->writeAttribute("DownFade", QString::number(cue.downFade));
    if (cue.delay != 0)
        doc->writeAttribute("Delay", QString::number(cue.delay));
    if (cue.curve != FadeChannel::CurveLinear)
        doc->writeAttribute("Curve", curveToString(cue.curve));
    if (cue.skip)
        doc->writeAttribute("Skip", "1");
    if (cue.block)
        doc->writeAttribute("Block", "1");
    if (cue.link != 0)
    {
        doc->writeAttribute("Link", QString::number(cue.link));
        if (cue.linkCount != 0)
            doc->writeAttribute("LinkCount", QString::number(cue.linkCount));
    }
    if (cue.noMib)
        doc->writeAttribute("NoMib", "1");
    // DMX Total Control: its time on the clock of a Timecode list
    if (cue.time >= 0)
        doc->writeAttribute("Time", QString::number(cue.time));

    if (cue.note.isEmpty() == false)
        doc->writeTextElement(KXMLDTCCueNote, cue.note);

    // sorted, so the file doesn't change for nothing
    QList<Key> keys = cue.values.keys();
    std::sort(keys.begin(), keys.end());
    for (const Key &key : std::as_const(keys))
    {
        doc->writeStartElement(KXMLDTCCueValue);
        doc->writeAttribute("F", QString::number(key.first));
        doc->writeAttribute("C", QString::number(key.second));
        auto palette = cue.palettes.constFind(key);
        if (palette != cue.palettes.cend())
            doc->writeAttribute("P", QString::number(palette.value()));
        doc->writeCharacters(QString::number(cue.values.value(key)));
        doc->writeEndElement();
    }

    QList<Key> released = cue.releases.values();
    std::sort(released.begin(), released.end());
    for (const Key &key : std::as_const(released))
    {
        doc->writeEmptyElement(KXMLDTCCueRelease);
        doc->writeAttribute("F", QString::number(key.first));
        doc->writeAttribute("C", QString::number(key.second));
    }

    for (const FXScene::Layer &layer : cue.fx)
        FXScene::saveLayerXML(doc, layer);

    QList<FxKey> fxReleased = cue.fxReleases.values();
    std::sort(fxReleased.begin(), fxReleased.end());
    for (const FxKey &key : std::as_const(fxReleased))
    {
        doc->writeEmptyElement(KXMLDTCCueFxRelease);
        doc->writeAttribute("Param", key.first);
        doc->writeAttribute("F", QString::number(key.second));
    }

    // the times of the attributes, in their order, then of the fixtures, by ID
    auto writeTimes = [doc](const PartTime &part) {
        if (part.fade >= 0)
            doc->writeAttribute("Fade", QString::number(part.fade));
        if (part.delay >= 0)
            doc->writeAttribute("Delay", QString::number(part.delay));
        if (part.curve >= 0)
            doc->writeAttribute("Curve", curveToString(part.curve));
    };
    for (auto it = cue.groupTimes.cbegin(); it != cue.groupTimes.cend(); ++it)
    {
        doc->writeEmptyElement(KXMLDTCCueTime);
        doc->writeAttribute("Attr", timeGroupToString(it.key()));
        writeTimes(it.value());
    }
    for (auto it = cue.fixtureTimes.cbegin(); it != cue.fixtureTimes.cend(); ++it)
    {
        doc->writeEmptyElement(KXMLDTCCueTime);
        doc->writeAttribute("F", QString::number(it.key()));
        writeTimes(it.value());
    }

    // what it does when it starts, in order (the attributes sorted)
    for (const CueAction &action : cue.actions)
    {
        doc->writeEmptyElement(KXMLDTCCueAction);
        for (auto it = action.cbegin(); it != action.cend(); ++it)
            doc->writeAttribute(it.key(), it.value());
    }

    doc->writeEndElement();
}

void Cuelist::loadCueTimeXML(const QXmlStreamAttributes &attrs, Cue &cue)
{
    // (what isn't a time, in a damaged file, is the cue's)
    PartTime part;
    bool ok = false;
    const int fade = attrs.value("Fade").toInt(&ok);
    if (ok && fade >= 0)
        part.fade = fade;
    const int delay = attrs.value("Delay").toInt(&ok);
    if (ok && delay >= 0)
        part.delay = delay;
    // (a curve it doesn't know is linear)
    if (attrs.hasAttribute("Curve"))
        part.curve = stringToCurve(attrs.value("Curve").toString());
    if (part.isEmpty())
        return;

    if (attrs.hasAttribute("Attr"))
    {
        const int group = stringToTimeGroup(attrs.value("Attr").toString());
        if (group >= 0)
            cue.groupTimes.insert(group, part);
    }
    else if (attrs.hasAttribute("F"))
    {
        const quint32 fixture = attrs.value("F").toUInt(&ok);
        if (ok)
            cue.fixtureTimes.insert(fixture, part);
    }
}

bool Cuelist::loadCueXML(QXmlStreamReader &root, Cue &cue)
{
    const QXmlStreamAttributes attrs = root.attributes();
    bool numberOk = false;
    cue.uid = attrs.value("Uid").toUInt();
    cue.number = attrs.value("Number").toInt(&numberOk);
    cue.name = attrs.value("Name").toString();
    cue.trigger = stringToTrigger(attrs.value("Trigger").toString());
    // (a time that isn't a number, in a damaged file, keeps the default)
    cue.triggerTime = attrs.value("TriggerTime").toUInt();
    bool fadeOk = false, downOk = false;
    const uint fade = attrs.value("Fade").toUInt(&fadeOk);
    if (fadeOk)
        cue.fade = fade;
    const int downFade = attrs.value("DownFade").toInt(&downOk);
    cue.downFade = downOk ? qMax(-1, downFade) : -1;
    cue.delay = attrs.value("Delay").toUInt();
    // (a curve it doesn't know is linear)
    cue.curve = stringToCurve(attrs.value("Curve").toString());
    cue.skip = attrs.value("Skip") == QLatin1String("1");
    cue.block = attrs.value("Block") == QLatin1String("1");
    // (a link that isn't a uid of the list, in a damaged file, is none; the
    // count is bound to 0..999)
    cue.link = attrs.value("Link").toUInt();
    bool countOk = false;
    const int linkCount = attrs.value("LinkCount").toInt(&countOk);
    cue.linkCount = countOk ? linkCount : 0;
    cue.noMib = attrs.value("NoMib") == QLatin1String("1");
    // (a time that isn't a number is none; sanitize keeps it in range)
    bool timeOk = false;
    const int time = attrs.value("Time").toInt(&timeOk);
    cue.time = timeOk ? qMax(-1, time) : -1;

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLDTCCueNote)
        {
            cue.note = root.readElementText(QXmlStreamReader::SkipChildElements);
        }
        else if (root.name() == KXMLDTCCueValue)
        {
            const QXmlStreamAttributes va = root.attributes();
            bool fixtureOk = false, channelOk = false, paletteOk = false, valueOk = false;
            const Key key(va.value("F").toUInt(&fixtureOk), va.value("C").toUInt(&channelOk));
            const int palette = va.value("P").toInt(&paletteOk);
            const int value = root.readElementText(QXmlStreamReader::SkipChildElements).trimmed().toInt(&valueOk);
            if (fixtureOk && channelOk && valueOk)
            {
                cue.values.insert(key, uchar(qBound(0, value, 255)));
                if (paletteOk)
                    cue.palettes.insert(key, palette);
            }
        }
        else if (root.name() == KXMLDTCCueRelease)
        {
            const QXmlStreamAttributes ra = root.attributes();
            bool fixtureOk = false, channelOk = false;
            const Key key(ra.value("F").toUInt(&fixtureOk), ra.value("C").toUInt(&channelOk));
            if (fixtureOk && channelOk)
                cue.releases.insert(key);
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLDTCFXLayer)
        {
            FXScene::Layer layer = FXScene::loadLayerXML(root);
            if (layer.param.isEmpty() == false)
                cue.fx.append(layer);
        }
        else if (root.name() == KXMLDTCCueFxRelease)
        {
            const QXmlStreamAttributes fa = root.attributes();
            bool fixtureOk = false;
            const QString param = fa.value("Param").toString().toLower();
            const quint32 fixture = fa.value("F").toUInt(&fixtureOk);
            if (fixtureOk && param.isEmpty() == false)
                cue.fxReleases.insert(FxKey(param, fixture));
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLDTCCueTime)
        {
            loadCueTimeXML(root.attributes(), cue);
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLDTCCueAction)
        {
            // as it was saved (one without a target goes)
            CueAction action;
            const QXmlStreamAttributes actionAttrs = root.attributes();
            for (const QXmlStreamAttribute &attr : actionAttrs)
                action.insert(attr.name().toString(), attr.value().toString());
            cue.actions.append(action);
            root.skipCurrentElement();
        }
        else
        {
            // a newer version: what we don't know is skipped
            root.skipCurrentElement();
        }
    }

    return numberOk;
}

bool Cuelist::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCFunction);
    saveXMLCommon(doc);
    // the fade of new cues and the release time
    saveXMLSpeed(doc);

    QMutexLocker locker(&m_mutex);
    doc->writeStartElement(KXMLDTCCuelist);
    doc->writeAttribute("Tracking", m_tracking ? "1" : "0");
    doc->writeAttribute("AtEnd", atEndToString(m_atEnd));
    doc->writeAttribute("ResetOnRelease", m_resetOnRelease ? "1" : "0");
    doc->writeAttribute("BackTime", QString::number(m_backTime));
    doc->writeAttribute("ReleaseOnNextGo", m_releaseOnNextGo ? "1" : "0");
    doc->writeAttribute("NextUid", QString::number(m_nextUid));
    // DMX Total Control: Move in black, only when it is not as by default
    if (m_moveInBlack)
        doc->writeAttribute("MoveInBlack", "1");
    if (m_mibDelay != 0)
        doc->writeAttribute("MibDelay", QString::number(m_mibDelay));
    if (m_mibFade != defaultMibFade())
        doc->writeAttribute("MibFade", QString::number(m_mibFade));
    // and the type of list, only when it is not Normal
    if (m_mode != ModeNormal)
        doc->writeAttribute("Mode", modeToString(m_mode));
    // a Chase: its speed, its change and its direction, when not as by default
    if (m_mode == ModeChase)
    {
        if (m_chaseBpm != defaultChaseBpm())
            doc->writeAttribute("ChaseBpm", QString::number(m_chaseBpm));
        if (m_chaseShowTempo)
            doc->writeAttribute("ChaseShowBpm", "1");
        if (m_chaseMultiplier != 1.0)
            doc->writeAttribute("ChaseMultiplier", QString::number(m_chaseMultiplier));
        if (m_chaseXfade != 0)
            doc->writeAttribute("ChaseXfade", QString::number(m_chaseXfade));
        if (m_chaseDirection != ChaseForward)
            doc->writeAttribute("ChaseDirection", chaseDirectionToString(m_chaseDirection));
    }
    // a Timecode list: its clock and its offset, when not as by default
    if (m_mode == ModeTimecode)
    {
        if (m_tcSource != TcInternal)
            doc->writeAttribute("TcSource", tcSourceToString(m_tcSource));
        if (m_tcOffset != 0)
            doc->writeAttribute("TcOffset", QString::number(m_tcOffset));
    }
    doc->writeEndElement();

    for (const Cue &cue : m_cues)
        saveCueXML(doc, cue);

    doc->writeEndElement();
    return true;
}

bool Cuelist::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << "Function node not found!";
        return false;
    }

    if (root.attributes().value(KXMLQLCFunctionType).toString() != typeToString(Function::CuelistType))
    {
        qWarning() << "Function is not a cuelist!";
        return false;
    }

    QList<Cue> cues;
    bool tracking = true;
    AtEnd atEnd = AtEndLoop;
    bool resetOnRelease = true;
    int backTime = -1;
    bool releaseOnNextGo = false;
    quint32 nextUid = 1;
    bool moveInBlack = false;
    uint mibDelay = 0;
    uint mibFade = defaultMibFade();
    Mode mode = ModeNormal;
    int chaseBpm = defaultChaseBpm();
    bool chaseShowTempo = false;
    qreal chaseMultiplier = 1.0;
    int chaseXfade = 0;
    ChaseDirection chaseDirection = ChaseForward;
    TcSource tcSource = TcInternal;
    uint tcOffset = 0;

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(root);
        }
        else if (root.name() == KXMLDTCCuelist)
        {
            const QXmlStreamAttributes attrs = root.attributes();
            tracking = attrs.value("Tracking") != QLatin1String("0");
            atEnd = stringToAtEnd(attrs.value("AtEnd").toString());
            resetOnRelease = attrs.value("ResetOnRelease") != QLatin1String("0");
            bool backOk = false;
            const int back = attrs.value("BackTime").toInt(&backOk);
            if (backOk)
                backTime = qMax(-1, back);
            releaseOnNextGo = attrs.value("ReleaseOnNextGo") == QLatin1String("1");
            nextUid = qMax(1u, attrs.value("NextUid").toUInt());
            // (a time that isn't a number, in a damaged file, keeps the default)
            moveInBlack = attrs.value("MoveInBlack") == QLatin1String("1");
            bool timeOk = false;
            const uint delay = attrs.value("MibDelay").toUInt(&timeOk);
            if (timeOk)
                mibDelay = qMin(delay, 3600000u);
            const uint fade = attrs.value("MibFade").toUInt(&timeOk);
            if (timeOk)
                mibFade = qMin(fade, 3600000u);
            mode = stringToMode(attrs.value("Mode").toString());
            // (in range; what isn't a number keeps the default)
            bool chaseOk = false;
            const int bpm = attrs.value("ChaseBpm").toInt(&chaseOk);
            if (chaseOk)
                chaseBpm = qBound(10, bpm, 600);
            chaseShowTempo = attrs.value("ChaseShowBpm") == QLatin1String("1");
            const qreal multiplier = attrs.value("ChaseMultiplier").toDouble(&chaseOk);
            if (chaseOk)
                chaseMultiplier = chaseMultiplierOf(multiplier);
            const int xfade = attrs.value("ChaseXfade").toInt(&chaseOk);
            if (chaseOk)
                chaseXfade = qBound(-1, xfade, 100);
            chaseDirection = stringToChaseDirection(attrs.value("ChaseDirection").toString());
            tcSource = stringToTcSource(attrs.value("TcSource").toString());
            bool offsetOk = false;
            const uint offset = attrs.value("TcOffset").toUInt(&offsetOk);
            if (offsetOk)
                tcOffset = qMin(offset, uint(maxCueTime()));
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLDTCCue)
        {
            Cue cue;
            if (loadCueXML(root, cue))
                cues.append(cue);
            else
                qWarning() << "Cuelist" << name() << ": a cue without a number, dropped";
        }
        else
        {
            qWarning() << "Unknown cuelist tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    QMutexLocker locker(&m_mutex);
    m_tracking = tracking;
    m_atEnd = atEnd;
    m_resetOnRelease = resetOnRelease;
    m_backTime = backTime;
    m_releaseOnNextGo = releaseOnNextGo;
    m_moveInBlack = moveInBlack;
    m_mibDelay = mibDelay;
    m_mibFade = mibFade;
    m_mode = mode;
    m_chaseBpm = chaseBpm;
    m_chaseShowTempo = chaseShowTempo;
    m_chaseMultiplier = chaseMultiplier;
    m_chaseXfade = chaseXfade;
    m_chaseDirection = chaseDirection;
    m_tcSource = tcSource;
    m_tcOffset = tcOffset;
    // sorted, one cue per number, unique uids, and uids are never reused
    m_nextUid = nextUid;
    normalizeLocked(cues);
    m_cues = cues;
    m_dataDirty = true;
    followEditsLocked();

    return true;
}

void Cuelist::postLoad()
{
    Doc *show = doc();
    if (show == NULL)
        return;

    // the channels of fixtures that are not in the show
    QMutexLocker locker(&m_mutex);
    if (removeFixturesLocked([show](quint32 fixture) { return show->fixture(fixture) == NULL; }))
        m_dataDirty = true;

    // and, like the scenes, the channels their fixtures don't have (a
    // damaged or edited file)
    auto missing = [show](const Key &key) {
        Fixture *fxi = show->fixture(key.first);
        return fxi == NULL || key.second >= fxi->channels();
    };
    for (Cue &cue : m_cues)
    {
        for (auto it = cue.values.begin(); it != cue.values.end();)
        {
            if (missing(it.key()))
            {
                cue.palettes.remove(it.key());
                it = cue.values.erase(it);
                m_dataDirty = true;
            }
            else
            {
                ++it;
            }
        }
        for (auto it = cue.releases.begin(); it != cue.releases.end();)
        {
            if (missing(*it))
            {
                it = cue.releases.erase(it);
                m_dataDirty = true;
            }
            else
            {
                ++it;
            }
        }
    }
}

/*********************************************************************
 * Running
 *********************************************************************/

void Cuelist::preRun(MasterTimer *timer)
{
    {
        QMutexLocker locker(&m_mutex);
        // DMX Total Control: a new type plays from this start on
        m_runMode = m_mode;
        m_playing.clear();
        // (the player is stopped: our last postRun stopped it)
        m_fxPlaying.clear();
        m_fxBases.clear();
        m_fxChannels.clear();
        // (m_actions is kept: the GOs and jumps that came after start(), a
        // second press in the same tick. postRun dropped the older ones)
        m_preselectUid = 0;
        m_changed.clear();
        m_notify.clear();
        m_actionNotify.clear();
        // a new start counts the links again
        m_linkTaken.clear();
        m_current = -1;
        m_currentUid = 0;
        m_firstOfRun = true;
        m_followArmed = false;
        m_releaseRequested = false;
        m_dataDirty = false;
        m_fadersPaused = false;
        m_appliedIntensity = -1;
        m_cueElapsed = 0;
        m_cueLength = 0;
        m_fadeLength = 0;
        m_seekRemainder = 0;
        m_liveScale = 1.0;
        m_ownTimes = true;
        m_mibUid = 0;
        // DMX Total Control: a Chase starts its step, a Bounce on its way up
        m_chaseElapsed = 0;
        m_bounceDir = 1;
        m_randomUid = 0;
        m_manual = false;
        // DMX Total Control: the fades start at the speed of now (a block
        // stretched before its start is no change of speed)
        m_appliedFactor = timeFactor();

        // the cue set with setStartCue(), else a GO. DMX Total Control: a start
        // cue deleted since is a GO, which starts where the status said
        // (startIndexLocked skips it too), not a jump to nothing
        Action first;
        if (m_startUid != 0 && indexOfUidLocked(m_startUid) >= 0)
        {
            first.type = ActionGoTo;
            first.uid = m_startUid;
        }
        // DMX Total Control: the A/B fader: that cue fades in by hand
        if (m_startManual)
        {
            first.type = ActionManualGo;
            m_startManual = false;
        }

        // a super scene that starts the block after its beginning (start()
        // set the elapsed time, in real ms; the block may stretch the list,
        // DMX Total Control: and the cue speed makes it go faster or slower)
        const quint64 offset = quint64(qMax(qint64(0), qRound64(qreal(elapsed()) * m_appliedFactor)));
        // DMX Total Control, Timecode: the clock starts at the time the
        // super-scene block starts in (the timeline is the clock), else at
        // 00:00, or at the time of the cue the run starts on (a start cue,
        // the cue after a release). Its first frame plays the cue of the clock
        if (m_mode == ModeTimecode)
        {
            first = Action();
            first.type = ActionTcSync;
            const int start = startIndexLocked();
            m_tcLive = false;
            if (m_tcSource == TcMtc)
            {
                // a MIDI timecode: the cue of its time, which the first frame
                // reads (not the block, nor a start cue). The run waits for
                // it, its clock before 00:00
                first.snap = true;
                m_tcClock = -1;
            }
            else if (offset > 0)
            {
                m_tcClock = qint64(offset);
                first.snap = true;
            }
            else if (start > 0 || (start == 0 && indexOfUidLocked(m_startUid) >= 0))
            {
                m_tcClock = tcTimeLocked(start);
                first.uid = m_cues.at(start).uid;
            }
            else
            {
                m_tcClock = 0;
            }
            m_tcLastClock = m_tcClock;
            m_tcUid = 0;
        }
        else if (offset > 0 && m_cues.isEmpty() == false)
        {
            quint64 remainder = 0;
            bool moved = false;
            // DMX Total Control: a Chase by its steps (the remainder counts
            // in its first step)
            const int index = m_mode == ModeChase ? chaseSeekLocked(offset, &remainder, &moved, &m_bounceDir)
                                                  : seekLocked(offset, &remainder, &moved, &m_linkTaken);
            // past a trigger: the cue the chain is on at that time, snapped
            // (its actions are not done: they were, in the part not played).
            // Still on the first cue (the show runs in ticks, so most blocks
            // start a few ms in): the run starts as usual, with the block
            // fade-in and the cue fade, and only its timers count from there
            if (moved)
            {
                first.type = ActionGoTo;
                first.uid = m_cues.at(index).uid;
                first.snap = true;
                first.actions = false;
            }
            m_seekRemainder = uint(qMin(remainder, quint64(UINT_MAX)));
        }
        // before them: they go on from the cue the run starts on
        m_actions.prepend(first);
        m_startUid = 0;
    }
    {
        QMutexLocker locker(&m_statusMutex);
        m_status = Status();
    }

    Function::preRun(timer);
}

int Cuelist::seekLocked(quint64 offset, quint64 *remainder, bool *moved, QHash<quint32, int> *taken) const
{
    *moved = false;
    taken->clear();

    // a chain with no GO in it that comes back to the first cue (a loop, or
    // a link to it) plays the same cues in every cycle
    quint64 cycle = 0;
    if (walkLocked(&cycle, nullptr) == ChainCycle && cycle > 0)
    {
        // whole cycles already played
        if (offset >= cycle)
            *moved = true;
        offset %= cycle;
    }

    // DMX Total Control: through the links, counting their jumps. A link that
    // loops without a count, there again with the same counts, goes round the
    // same way: whole laps are skipped
    QHash<int, QPair<QHash<quint32, int>, quint64>> loops;
    int current = 0;
    quint64 start = 0;
    // (a guard against loops of cues without times)
    for (int guard = 0; guard < kChainSteps; guard++)
    {
        bool viaLink = false;
        const int next = linkNextLocked(current, *taken, &viaLink);
        if (next < 0)
            break;
        if (viaLink && m_cues.at(current).linkCount == 0)
        {
            auto seen = loops.constFind(current);
            if (seen != loops.cend() && seen->first == *taken && start > seen->second)
            {
                const quint64 lap = start - seen->second;
                start += (offset - start) / lap * lap;
            }
            loops.insert(current, qMakePair(*taken, start));
        }
        quint64 ms = 0;
        if (gapLocked(current, next, &ms) == false || start + ms > offset)
            break;
        countLink(m_cues.at(current), viaLink, *taken);
        current = next;
        start += ms;
        *moved = true;
    }

    *remainder = offset - start;
    return current;
}

QSharedPointer<GenericFader> Cuelist::getFader(Universe *universe)
{
    if (universe == NULL)
        return QSharedPointer<GenericFader>();

    QSharedPointer<GenericFader> fader = m_fadersMap.value(universe->id(), QSharedPointer<GenericFader>());
    if (fader.isNull())
    {
        fader = universe->requestFader(Universe::Auto, playbackPriority());
        fader->adjustIntensity(getAttributeValue(Intensity));
        fader->setBlendMode(blendMode());
        fader->setName(name());
        fader->setParentFunctionID(id());
        fader->setHandleSecondary(true);
        m_fadersMap[universe->id()] = fader;
    }
    return fader;
}

QSharedPointer<GenericFader> Cuelist::topFaderLocked(Universe *universe)
{
    if (universe == NULL)
        return QSharedPointer<GenericFader>();

    // (our own FX play in channels that are never in our faders)
    const QSharedPointer<GenericFader> newest = m_fadersMap.value(universe->id());
    if (newest.isNull() == false && universe->isTopOfPriority(newest, m_fxPlayer->faders()) == false)
    {
        m_olderFaders.append(qMakePair(universe->id(), newest));
        m_fadersMap.remove(universe->id());
    }
    return getFader(universe);
}

bool Cuelist::takeChannelLocked(Universe *universe, const QSharedPointer<GenericFader> &top,
                                const Fixture *fixture, const Key &key)
{
    const QSharedPointer<GenericFader> holder = m_holder.value(key);
    if (holder.isNull() || holder == top)
        return false;

    // a coarse/fine pair is one FadeChannel, by its coarse channel
    const QLCFixtureMode *mode = fixture->fixtureMode();
    const quint32 primary = mode != NULL ? mode->primaryChannel(key.second) : QLCChannel::invalid();
    const quint32 channel = primary != QLCChannel::invalid() ? primary : key.second;
    FadeChannel fc;
    if (holder->channelCopy(key.first, channel, &fc) == false)
        return false;

    // an LTP channel goes on from the value of the playback that had it;
    // HTP values merge, so an intensity keeps the fade it runs
    const quint32 address = fc.addressInUniverse();
    const bool ltp = address != QLCChannel::invalid() &&
                     (universe->channelCapabilities(ushort(address)) & Universe::HTP) == 0;
    if (ltp)
    {
        for (int i = 0; i < fc.channelCount(); i++)
            fc.setCurrent(universe->preGMValue(int(address) + i), i);
    }
    // in the new fader before it leaves the old one: no frame without it
    top->replace(fc);
    holder->removeChannel(key.first, channel);
    return ltp;
}

QList<QSharedPointer<GenericFader>> Cuelist::allFaders() const
{
    QList<QSharedPointer<GenericFader>> faders;
    for (const auto &older : std::as_const(m_olderFaders))
        faders.append(older.second);
    for (const QSharedPointer<GenericFader> &fader : std::as_const(m_fadersMap))
        faders.append(fader);
    return faders;
}

void Cuelist::dropEmptyFaders()
{
    for (auto it = m_olderFaders.begin(); it != m_olderFaders.end();)
    {
        const QSharedPointer<GenericFader> fader = it->second;
        if (fader.isNull() == false && fader->channelsCount() > 0)
        {
            ++it;
            continue;
        }
        if (fader.isNull() == false)
            fader->requestDelete();
        m_holder.removeIf([&fader](const QHash<Key, QSharedPointer<GenericFader>>::iterator &holder) {
            return holder.value() == fader;
        });
        it = m_olderFaders.erase(it);
    }
}

int Cuelist::startIndexLocked() const
{
    return startIndexLocked(m_lastUid);
}

int Cuelist::startIndexLocked(quint32 lastUid) const
{
    const int count = m_cues.count();
    if (count == 0)
        return -1;

    const int start = indexOfUidLocked(m_startUid);
    if (start >= 0)
        return start;

    // resume after the cue the last release left
    if (m_resetOnRelease == false)
    {
        const int last = indexOfUidLocked(lastUid);
        if (last >= 0)
        {
            if (last + 1 < count)
                return last + 1;
            // (DMX Total Control: a Chase always goes round)
            return m_atEnd == AtEndStay && m_mode != ModeChase ? last : 0;
        }
    }
    return 0;
}

int Cuelist::nextLocked(bool *viaLink) const
{
    if (viaLink != nullptr)
        *viaLink = false;
    if (m_cues.isEmpty())
        return -1;

    const int preselected = indexOfUidLocked(m_preselectUid);
    if (preselected >= 0)
        return preselected;
    if (m_current < 0)
        return startIndexLocked();
    // DMX Total Control: a Chase goes on by its direction, without links
    if (m_mode == ModeChase)
    {
        int bounce = m_bounceDir;
        return chaseNextLocked(m_current, 1, &bounce);
    }
    // DMX Total Control: a Timecode list goes on in order, without links
    // (its clock does too); past the end as by AtEnd
    if (m_mode == ModeTimecode)
        return m_current + 1 < m_cues.count() ? m_current + 1 : (m_atEnd == AtEndLoop ? 0 : -1);
    bool link = false;
    const int next = linkNextLocked(m_current, m_linkTaken, &link);
    if (viaLink != nullptr)
        *viaLink = link;
    return next;
}

int Cuelist::linkNextLocked(int from, const QHash<quint32, int> &taken, bool *viaLink) const
{
    *viaLink = false;
    const int count = m_cues.count();
    // DMX Total Control: its link, while it has jumps left
    if (from >= 0 && from < count)
    {
        const Cue &cue = m_cues.at(from);
        const int target = cue.link != 0 ? indexOfUidLocked(cue.link) : -1;
        if (target >= 0 && (cue.linkCount == 0 || taken.value(cue.uid) < cue.linkCount))
        {
            *viaLink = true;
            return target;
        }
    }
    if (from + 1 < count)
        return from + 1;
    return m_atEnd == AtEndLoop && count > 0 ? 0 : -1;
}

void Cuelist::countLink(const Cue &from, bool viaLink, QHash<quint32, int> &taken)
{
    // (a link without a count is always taken: nothing to count)
    if (viaLink)
    {
        if (from.linkCount > 0)
            taken[from.uid]++;
    }
    else if (from.link != 0)
    {
        // it goes on past its link: the count starts again the next time
        taken.remove(from.uid);
    }
}

void Cuelist::takeLinkLocked(int from, bool viaLink)
{
    if (from >= 0 && from < m_cues.count())
        countLink(m_cues.at(from), viaLink, m_linkTaken);
}

const Cuelist::Cue *Cuelist::autoTriggerLocked(int next) const
{
    // (DMX Total Control: the triggers of a Chase don't count, its speed
    // steps it; nor the ones of a Timecode list, its clock does)
    if (m_current < 0 || m_cues.isEmpty() || m_mode == ModeChase || m_mode == ModeTimecode)
        return nullptr;
    if (next >= 0 && next < m_cues.count())
        return m_cues.at(next).trigger != TriggerGo ? &m_cues.at(next) : nullptr;

    // DMX Total Control: past the end, as Loop wraps with it, Release goes
    // with the trigger of the first cue: the list releases itself (a
    // Sequence played once, converted, ends when its last step does)
    if (m_atEnd == AtEndRelease && m_current + 1 >= m_cues.count() && m_cues.first().trigger != TriggerGo)
        return &m_cues.first();
    return nullptr;
}

void Cuelist::followEditsLocked()
{
    // DMX Total Control: write() re-applies the current cue by its uid, but
    // not while the list is paused: until then the index and the status
    // would point at the cue that took its place
    if (m_current >= 0)
    {
        int index = indexOfUidLocked(m_currentUid);
        // deleted: the one before it, the cue reapplyLocked would play
        if (index < 0 && m_cues.isEmpty() == false)
            index = qBound(0, m_current - 1, int(m_cues.count()) - 1);
        m_current = index;
        m_currentUid = index >= 0 ? m_cues.at(index).uid : 0;
    }
    updateStatusLocked();
}

uint Cuelist::scaledTime(uint ms) const
{
    // DMX Total Control: the speed of this frame (write() took it), the one
    // the fades that run are at
    const qreal factor = m_appliedFactor > 0 ? m_appliedFactor : timeFactor();
    if (ms == 0 || factor <= 0)
        return ms;
    return uint(qMin(qRound64(qreal(ms) / factor), qint64(UINT_MAX)));
}

quint32 Cuelist::pairedChannel(const Fixture *fixture, quint32 channel)
{
    const QLCFixtureMode *mode = fixture->fixtureMode();
    if (mode == NULL)
        return QLCChannel::invalid();

    // a fine channel: its coarse one
    const quint32 primary = mode->primaryChannel(channel);
    if (primary != QLCChannel::invalid())
        return primary;

    // a coarse channel: the fine channel that names it
    for (quint32 c = 0; c < fixture->channels(); c++)
    {
        if (c != channel && mode->primaryChannel(c) == channel)
            return c;
    }
    return QLCChannel::invalid();
}

bool Cuelist::isDimmer(const Fixture *fixture, quint32 channel)
{
    // RGB and CMY are intensities in QLC+ too: they take the fade, as the moves
    const QLCChannel *ch = fixture->channel(channel);
    return ch != NULL && ch->group() == QLCChannel::Intensity && ch->colour() == QLCChannel::NoColour;
}

void Cuelist::write(MasterTimer *timer, QList<Universe *> universes)
{
    // pause and level reach the faders here only, in the timer thread: the
    // other threads never touch them. DMX Total Control: the older faders
    // too, and the ones that hold nothing any more go
    dropEmptyFaders();
    const bool paused = isPaused();
    if (paused != m_fadersPaused)
    {
        for (const QSharedPointer<GenericFader> &fader : allFaders())
        {
            if (!fader.isNull())
                fader->setPaused(paused);
        }
        m_fxPlayer->setPause(paused);
        // DMX Total Control: its faders hold too (a base fading in with the
        // cue), and the ones of FX it is still fading out: we don't write a
        // paused player, so it can't do it itself
        m_fxPlayer->setFadersPaused(paused);
        m_fadersPaused = paused;
    }
    const qreal level = getAttributeValue(Intensity);
    if (level != m_appliedIntensity)
    {
        for (const QSharedPointer<GenericFader> &fader : allFaders())
        {
            if (!fader.isNull())
                fader->adjustIntensity(level);
        }
        // the base level of the FX player: no override ids to keep
        m_fxPlayer->adjustAttribute(level, Function::Intensity);
        m_appliedIntensity = level;
    }

    // paused: the faders hold their values and the cue timers stop. BeatGO
    // divisions are dropped, not saved up for a burst of GOs on resume
    if (paused)
    {
        takeDivisionJumps();
        return;
    }

    advanceFrameTick();

    QList<quint32> started;
    QList<quint32> withActions;
    bool release = false;
    {
        QMutexLocker locker(&m_mutex);

        // DMX Total Control: a new speed (the cue speed, a group's dial)
        // reaches the fades that run, from where they are: they end sooner
        // or later. The cue timers count in cue ms, at the speed of frameTick
        // (DMX Total Control: not the ones of a hand crossfade: the hand
        // runs them)
        const qreal factor = timeFactor();
        if (m_appliedFactor > 0 && factor != m_appliedFactor)
            scaleFadesLocked(m_appliedFactor / factor, false);
        m_appliedFactor = factor;

        // BeatGO and pulses (a GO per division) and its own rhythm (a GO a beat)
        for (int jumps = takeRhythmSteps(); jumps > 0; jumps--)
            m_actions.append(Action());

        // the cues changed while playing: the new data shows at once, and the
        // next GO still goes on from this cue (e.g. after an Update)
        if (m_dataDirty)
        {
            m_dataDirty = false;
            // DMX Total Control: what Move in black prepared goes back to
            // the cue, and the new data is prepared again below. A hand
            // crossfade goes on by itself
            m_mibUid = 0;
            if (m_manual)
                endManualLocked(ManualOnClock, universes);
            reapplyLocked(universes);
        }

        // DMX Total Control, Timecode: a MIDI timecode is the clock, less the
        // offset (before it, the clock is before 00:00). One that stopped
        // holds the clock, and so the cue
        if (m_mode == ModeTimecode && m_tcSource == TcMtc)
        {
            const qint64 timecode = timer != NULL ? timer->timecode() : -1;
            m_tcLive = timecode >= 0;
            if (m_tcLive)
                m_tcClock = timecode - qint64(m_tcOffset);
        }

        // in the order they came: the last one wins on the channels they share
        bool applied = false;
        while (m_actions.isEmpty() == false)
        {
            if (processActionLocked(m_actions.takeFirst(), universes))
                applied = true;
        }
        // only the first cue of a run starts in the middle
        m_seekRemainder = 0;

        // DMX Total Control, the A/B fader: the fades of its cue where the
        // hand has them (a list that is not Normal any more: by themselves)
        if (m_manual && m_mode != ModeNormal)
            endManualLocked(ManualOnClock, universes);
        if (m_manual)
        {
            const qreal position = qBound(0.0, m_manualPosition.load(), 1.0);
            const uint ms = uint(qRound64(position * qreal(m_manualLength)));
            manualTimeLocked(ms);
            // the status shows where it is (m_fadeLength is its length)
            m_cueElapsed = ms;
        }

        // Follow and Wait: at most one automatic step per frame (with a rhythm
        // of its own, its beats move the cues instead). DMX Total Control: a
        // Chase steps by its own speed instead
        if (m_mode == ModeChase)
        {
            if (applied == false && chaseTickLocked(universes))
                applied = true;
        }
        else if (m_mode == ModeTimecode)
        {
            // DMX Total Control: a Timecode list plays the cue whose time has
            // come instead (its triggers don't count)
            if (applied == false && tcTickLocked(universes))
                applied = true;
        }
        else if (applied == false && m_followArmed && m_current >= 0 && tapPeriod() == 0)
        {
            bool viaLink = false;
            const bool preselected = indexOfUidLocked(m_preselectUid) >= 0;
            const int next = nextLocked(&viaLink);
            const Cue *trigger = autoTriggerLocked(next);
            if (trigger != nullptr)
            {
                // Follow: after the current cue completes; Wait: after its GO
                quint64 due = trigger->triggerTime;
                if (trigger->trigger == TriggerFollow)
                    due += m_cueLength;
                if (m_cueElapsed >= due)
                {
                    if (next >= 0)
                    {
                        // DMX Total Control: as a GO, the link counts its
                        // jump (a pre-selected cue is no step of the link)
                        if (preselected == false)
                            takeLinkLocked(m_current, viaLink);
                        const Cue &cue = m_cues.at(next);
                        const uint fade = cue.fade;
                        const uint down = cue.effectiveDown();
                        const uint delay = cue.delay;
                        m_preselectUid = 0;
                        applyCueLocked(universes, next, fade, down, delay, ApplyNormal, &cue, true);
                        applied = true;
                    }
                    else
                    {
                        // DMX Total Control: past the end of a Release list,
                        // as a GO on the last cue would
                        m_releaseRequested = true;
                    }
                }
            }
        }

        // DMX Total Control: Move in black, in a frame with no GO (nor
        // during a hand crossfade)
        if (applied == false && m_releaseRequested == false && m_manual == false)
            moveInBlackLocked(universes);

        // (DMX Total Control: a hand crossfade is where the hand has it)
        const uint tick = frameTick();
        if (m_manual == false)
            m_cueElapsed = m_cueElapsed < UINT_MAX - tick ? m_cueElapsed + tick : UINT_MAX;
        // DMX Total Control: the clock of a Timecode list goes on with the
        // cue timers (a pause holds it: no frame comes here); a MIDI
        // timecode moves it instead
        if (m_mode == ModeTimecode && m_tcSource != TcMtc)
            m_tcClock += tick;
        updateStatusLocked();

        started.swap(m_notify);
        withActions.swap(m_actionNotify);
        release = m_releaseRequested;
        m_releaseRequested = false;
    }

    // the FX of the cues play in this frame, after our own values. The
    // MasterTimer doesn't know the player: a new priority (ours, passed on by
    // updateFadersPriority) reaches its faders here. DMX Total Control: and
    // our live speed (a group's dial, a stretched block) its waves, as the
    // cue fades and the Follow/Wait timers (FXScene fades in real ms: the
    // fade it got is already at our speed)
    if (timer != NULL && m_fxPlayer->isRunning())
    {
        m_fxPlayer->setSpeedFactor(speedFactor());
        m_fxPlayer->setMotionSpeed(motionSpeed());
        m_fxPlayer->applyPriorityChange();
        m_fxPlayer->write(timer, universes);
    }

    // the programmer releases what a GO changed (live edit)
    if (timer != NULL)
    {
        for (quint32 uid : std::as_const(started))
            timer->notifyCueStarted(id(), uid);
        // DMX Total Control: and the app does the actions of the cues that started
        for (quint32 uid : std::as_const(withActions))
            timer->notifyCueActions(id(), uid);
    }

    incrementElapsed();

    // AtEndRelease: GO on the last cue (or the trigger of the first cue after
    // it) releases the list, as a chaser stops itself
    if (release)
        stop(FunctionParent::master());
}

bool Cuelist::processActionLocked(const Action &action, const QList<Universe *> &universes)
{
    const int count = m_cues.count();
    int target = -1;

    // DMX Total Control: Live Time, on the cue that fades only (not one the
    // hand crossfades)
    if (action.type == ActionLiveTime || action.type == ActionLiveScale)
    {
        if (m_current < 0 || m_cueElapsed >= m_fadeLength || m_manual)
            return false;
        liveTimeLocked(action.type == ActionLiveScale ? action.scale : qreal(action.ms) / qreal(m_fadeLength));
        return false;
    }

    // DMX Total Control, the A/B fader: the end of a hand crossfade. Any
    // other action ends it first: a GO lets it go on by itself (no new cue),
    // a new travel completes it
    if (action.type == ActionManualEnd)
        return m_manual && endManualLocked(action.end, universes);
    if (m_manual)
    {
        endManualLocked(action.type == ActionManualGo ? ManualComplete : ManualOnClock, universes);
        if (action.type == ActionGo)
            return false;
    }
    // DMX Total Control: a Timecode list: its clock goes with them
    if (m_mode == ModeTimecode)
        return tcActionLocked(action, universes);
    // (a Chase runs on its own clock: a plain GO)
    const bool manual = action.type == ActionManualGo && m_mode != ModeChase;
    // what a cancel brings back
    const quint32 prevUid = m_current >= 0 ? m_currentUid : 0;
    const QHash<quint32, int> prevLinks = m_linkTaken;
    const quint32 prevPreselect = m_preselectUid;

    if (action.type == ActionGo || action.type == ActionManualGo)
    {
        m_followArmed = true;
        const int preselected = indexOfUidLocked(m_preselectUid);
        // (DMX Total Control: a hand crossfade that starts the list, on its start cue)
        const int chosen = action.uid != 0 ? indexOfUidLocked(action.uid) : -1;
        m_preselectUid = 0;
        if (chosen >= 0)
        {
            target = chosen;
        }
        else if (preselected >= 0)
        {
            target = preselected;
        }
        else if (m_current < 0)
        {
            target = startIndexLocked();
        }
        else if (m_mode == ModeChase)
        {
            // DMX Total Control: the next step of the chase now, always round
            target = takeChaseStepLocked(1);
        }
        else
        {
            // DMX Total Control: the link of the cue while it has jumps left,
            // else the next cue
            bool viaLink = false;
            target = nextLocked(&viaLink);
            if (target >= 0)
                takeLinkLocked(m_current, viaLink);
            else if (m_atEnd == AtEndRelease)
                m_releaseRequested = true;
            // AtEndStay: it stays on the last cue
        }
    }
    else if (action.type == ActionBack && m_mode == ModeChase)
    {
        // DMX Total Control: a Chase goes a step the other way now, with the
        // times of its steps, and runs on from there
        if (m_current < 0)
            return false;
        target = takeChaseStepLocked(-1);
    }
    else if (action.type == ActionBack)
    {
        // nothing before the first cue
        if (m_current <= 0)
            return false;
        target = m_current - 1;
        // the list waits on that look: the next GO or jump arms it again
        m_followArmed = false;
        // a BACK time is one time for everything
        if (m_backTime >= 0)
        {
            applyCueLocked(universes, target, uint(m_backTime), uint(m_backTime), 0, ApplyNormal, nullptr, false);
            return true;
        }
    }
    else
    {
        target = indexOfUidLocked(action.uid);
        if (target < 0)
            return false;
        m_followArmed = true;
        if (action.snap)
        {
            applyCueLocked(universes, target, 0, 0, 0, ApplyNormal, nullptr, action.actions);
            return true;
        }
    }

    if (target < 0 || target >= count)
        return false;

    // the cue's own times (a jump uses the times of the cue it goes to),
    // its attribute and fixture times too
    const Cue &cue = m_cues.at(target);
    uint fade = cue.fade;
    uint down = cue.effectiveDown();
    uint delay = cue.delay;
    const Cue *own = &cue;
    // DMX Total Control, the A/B fader: the times of the cue (no group
    // fade-in) at the place the hand sets. A cue with no time crossfades
    // linearly over the travel. Follow waits for its end
    if (manual)
    {
        m_manualLength = cue.length();
        if (m_manualLength == 0)
        {
            fade = down = m_manualLength = kManualTime;
            own = nullptr;
        }
        m_manualPrevUid = prevUid;
        m_manualLinks = prevLinks;
        m_manualPreselect = prevPreselect;
        m_followArmed = false;
        applyCueLocked(universes, target, fade, down, delay, ApplyManual, own, action.actions);
        m_manual = true;
        return true;
    }
    // DMX Total Control: a Chase with a change of its own: a part of its
    // step for everything, linear and with no delay
    if (m_mode == ModeChase && m_chaseXfade >= 0)
    {
        fade = down = uint(quint64(chaseBeatLocked()) * uint(m_chaseXfade) / 100);
        delay = 0;
        own = nullptr;
    }
    // the first cue of a run fades in with the group or the super-scene
    // block: one time for everything
    if (m_firstOfRun && overrideFadeInSpeed() != defaultSpeed())
    {
        fade = down = overrideFadeInSpeed();
        own = nullptr;
    }
    applyCueLocked(universes, target, fade, down, delay, ApplyNormal, own, action.actions);
    return true;
}

uint Cuelist::chasePeriodLocked() const
{
    // (the show tempo is a plain int of the timer, as the chasers read it)
    qreal beat = 60000.0 / qreal(m_chaseBpm);
    const MasterTimer *timer = doc() != NULL ? doc()->masterTimer() : NULL;
    if (m_chaseShowTempo && timer != NULL && timer->beatTimeDuration() > 0)
        beat = timer->beatTimeDuration();
    return uint(qMax(20.0, std::round(beat / m_chaseMultiplier)));
}

uint Cuelist::chaseBeatLocked() const
{
    const uint tap = tapPeriod();
    return tap > 0 ? tap : chasePeriodLocked();
}

int Cuelist::chaseLapLocked() const
{
    const int count = m_cues.count();
    return m_chaseDirection == ChaseBounce && count > 1 ? 2 * (count - 1) : count;
}

int Cuelist::chaseNextLocked(int from, int way, int *bounce) const
{
    const int count = m_cues.count();
    if (count == 0)
        return -1;
    if (from < 0 || from >= count)
        return 0;

    switch (m_chaseDirection)
    {
        case ChaseBackward:
            return (from - way + count) % count;
        case ChaseBounce:
        {
            if (count == 1)
                return 0;
            // past an end: it turns
            if (from + way * *bounce < 0 || from + way * *bounce >= count)
                *bounce = -*bounce;
            return from + way * *bounce;
        }
        case ChaseRandom:
        {
            const int drawn = indexOfUidLocked(m_randomUid);
            return drawn != from || count == 1 ? drawn : -1;
        }
        default:
            return (from + way + count) % count;
    }
}

int Cuelist::takeChaseStepLocked(int way)
{
    const int next = chaseNextLocked(m_current, way, &m_bounceDir);
    // (Random: the cues changed since it drew one)
    if (next < 0 && m_chaseDirection == ChaseRandom)
        return drawChaseStepLocked(m_current);
    return next;
}

int Cuelist::drawChaseStepLocked(int from)
{
    const int count = m_cues.count();
    if (count <= 1 || from < 0 || from >= count)
        return count > 0 ? int(m_random.bounded(count)) : -1;
    // one of the others: never the same step twice in a row
    const int step = int(m_random.bounded(count - 1));
    return step >= from ? step + 1 : step;
}

bool Cuelist::chaseTickLocked(const QList<Universe *> &universes)
{
    // a tapped rhythm, BeatGO and Pulse give its steps as GOs (takeRhythmSteps)
    if (m_current < 0 || tapPeriod() != 0 || driveMode() != DriveOff)
        return false;
    // at the cue speed and its live speed, as its fades (frameTick). What is
    // left over a step counts in the next one: no burst of steps when the
    // step gets shorter or the speed goes up
    const uint period = chasePeriodLocked();
    m_chaseElapsed += frameTick();
    if (m_chaseElapsed < period)
        return false;
    const uint rest = m_chaseElapsed % period;
    const bool applied = processActionLocked(Action(), universes);
    m_chaseElapsed = rest;
    return applied;
}

int Cuelist::chaseSeekLocked(quint64 offset, quint64 *remainder, bool *moved, int *bounce) const
{
    const int count = m_cues.count();
    const quint64 beat = chaseBeatLocked();
    const quint64 steps = offset / beat;
    *remainder = offset % beat;
    *moved = steps > 0;
    *bounce = 1;

    switch (m_chaseDirection)
    {
        case ChaseBackward:
            return int((quint64(count) - steps % quint64(count)) % quint64(count));
        case ChaseBounce:
        {
            if (count == 1)
                return 0;
            // up to the last cue, then down: on the way down from the last one
            const int at = int(steps % quint64(chaseLapLocked()));
            if (at < count - 1)
                return at;
            *bounce = -1;
            return chaseLapLocked() - at;
        }
        default:
            // (Random: as Forward, any step will do)
            return int(steps % quint64(count));
    }
}

int Cuelist::tcPickLocked(qint64 clock) const
{
    // (cues are in order and their times should go up: a time before the
    // one of the cue before is shown as a warning, and this rule still holds)
    for (int i = int(m_cues.count()) - 1; i >= 0; i--)
    {
        const int time = m_cues.at(i).time;
        if (time >= 0 && time <= clock)
            return i;
    }
    return -1;
}

int Cuelist::tcFirstLocked() const
{
    int earliest = -1;
    for (const Cue &cue : m_cues)
    {
        if (cue.time >= 0 && (earliest < 0 || cue.time < earliest))
            earliest = cue.time;
    }
    return earliest >= 0 ? tcPickLocked(earliest) : -1;
}

qint64 Cuelist::tcTimeLocked(int index) const
{
    for (int i = qMin(index, int(m_cues.count()) - 1); i >= 0; i--)
    {
        if (m_cues.at(i).time >= 0)
            return m_cues.at(i).time;
    }
    return 0;
}

bool Cuelist::tcActionLocked(const Action &action, const QList<Universe *> &universes)
{
    // a MIDI timecode drives the list: GO, BACK and jumps do nothing, and
    // the start waits for the timecode (the clock plays its cue then)
    if (m_tcSource == TcMtc && (action.type != ActionTcSync || m_tcLive == false))
        return false;

    int target = -1;
    bool snap = false;
    bool withActions = action.actions;
    switch (action.type)
    {
        case ActionTcSync:
            // the start: the cue it starts on, with its times, else the cue
            // of the clock. A super-scene block that starts after the first
            // cue the clock plays snaps (its actions were in the part not
            // played), as a seek; one that starts on it fades it in
            target = action.uid != 0 ? indexOfUidLocked(action.uid) : tcPickLocked(m_tcClock);
            snap = action.snap && target != tcFirstLocked();
            withActions = snap == false;
            break;
        case ActionBack:
            // nothing before the first cue (nor before the clock reaches it)
            if (m_current <= 0)
                return false;
            target = m_current - 1;
            snap = true;
            withActions = false;
            break;
        case ActionGoTo:
            target = indexOfUidLocked(action.uid);
            snap = true;
            break;
        default:
        {
            // GO (and the A/B fader's: a plain GO): the pre-selected cue,
            // else the next one in order; past the end as by AtEnd
            const int preselected = indexOfUidLocked(m_preselectUid);
            m_preselectUid = 0;
            target = preselected >= 0 ? preselected : nextLocked(nullptr);
            if (target < 0 && m_current >= 0 && m_atEnd == AtEndRelease)
                m_releaseRequested = true;
            break;
        }
    }
    if (target < 0 || target >= m_cues.count())
        return false;

    // the clock goes to the time of the cue (a cue without one leaves it
    // going; the start keeps it: it is where the run is), and the clock
    // goes on from there
    const Cue &cue = m_cues.at(target);
    if (action.type != ActionTcSync && cue.time >= 0)
        m_tcClock = cue.time;
    const int pick = tcPickLocked(m_tcClock);
    m_tcUid = pick >= 0 ? m_cues.at(pick).uid : 0;
    m_tcLastClock = m_tcClock;
    // (a start after the time of its cue counts from there: the status, Move in black)
    m_seekRemainder = cue.time >= 0 && m_tcClock > cue.time ? uint(qMin(m_tcClock - cue.time, qint64(UINT_MAX))) : 0;

    if (action.type == ActionBack && m_backTime >= 0)
        applyCueLocked(universes, target, uint(m_backTime), uint(m_backTime), 0, ApplyNormal, nullptr, false);
    else if (snap)
        applyCueLocked(universes, target, 0, 0, 0, ApplyNormal, nullptr, withActions);
    else
        playCueLocked(universes, target, withActions);
    return true;
}

bool Cuelist::tcTickLocked(const QList<Universe *> &universes)
{
    const qint64 moved = m_tcClock - m_tcLastClock;
    m_tcLastClock = m_tcClock;
    const int pick = tcPickLocked(m_tcClock);
    const quint32 uid = pick >= 0 ? m_cues.at(pick).uid : 0;
    if (uid == m_tcUid)
        return false;
    m_tcUid = uid;
    // on it already (a GO to a cue without a time, an edit)
    if (pick == m_current)
        return false;

    // before the first cue: the list holds nothing (a clock that went back)
    if (pick < 0)
    {
        applyCueLocked(universes, -1, 0, 0, 0, ApplyNormal, nullptr, false);
        m_current = -1;
        m_currentUid = 0;
        m_cueElapsed = 0;
        m_cueLength = 0;
        m_fadeLength = 0;
        return true;
    }

    // one step on (the next cue with a time, the clock going on: less than
    // a second, or a frame of a fast one): with its times. Anything else
    // (back, past several cues, a jump of the clock) snaps to the look of
    // the cue of that time
    bool step = pick > m_current && moved >= 0 && (moved < 1000 || moved <= qint64(frameTick()));
    for (int i = m_current + 1; step && i < pick; i++)
    {
        if (m_cues.at(i).time >= 0)
            step = false;
    }
    if (step)
    {
        playCueLocked(universes, pick, true);
        return true;
    }
    const qint64 into = m_tcClock - m_cues.at(pick).time;
    m_seekRemainder = uint(qBound(qint64(0), into, qint64(UINT_MAX)));
    applyCueLocked(universes, pick, 0, 0, 0, ApplyNormal, nullptr, false);
    return true;
}

void Cuelist::playCueLocked(const QList<Universe *> &universes, int index, bool withActions)
{
    const Cue &cue = m_cues.at(index);
    // the first cue of a run fades in with the group or the super-scene
    // block: one time for everything
    if (m_firstOfRun && overrideFadeInSpeed() != defaultSpeed())
    {
        applyCueLocked(universes, index, overrideFadeInSpeed(), overrideFadeInSpeed(), cue.delay,
                       ApplyNormal, nullptr, withActions);
        return;
    }
    applyCueLocked(universes, index, cue.fade, cue.effectiveDown(), cue.delay, ApplyNormal, &cue, withActions);
}

void Cuelist::liveTimeLocked(qreal factor)
{
    // DMX Total Control: 0 ends them now. Each fade that runs gets to its
    // target at once: a release to what's below too, and it leaves the fader
    // there, as at the end of its fade
    if (factor <= 0)
    {
        for (const QSharedPointer<GenericFader> &fader : allFaders())
        {
            if (fader.isNull())
                continue;
            fader->forEachChannel([](FadeChannel &fc) {
                if ((fc.flags() & FadeChannel::Modulated) == 0 && fc.isReady() == false)
                    fc.setReady(true);
            });
        }
        m_cueElapsed = m_fadeLength;
        return;
    }

    // the fades from where they are, and the clock of the cue with them: the
    // status goes on from the same place, and a Follow waits for the new end
    // (a Wait keeps its time on that clock)
    scaleFadesLocked(factor, false);
    auto scaled = [factor](uint ms) {
        return uint(qBound(qint64(0), qRound64(qreal(ms) * factor), qint64(UINT_MAX)));
    };
    m_cueElapsed = scaled(m_cueElapsed);
    m_fadeLength = scaled(m_fadeLength);
    m_cueLength = scaled(m_cueLength);
    m_liveScale *= factor;
}

void Cuelist::scaleFadesLocked(qreal factor, bool manual)
{
    auto scaled = [factor](quint64 ms) {
        return uint(qBound(qint64(0), qRound64(qreal(ms) * factor), qint64(UINT_MAX)));
    };
    // (DMX Total Control: the older faders too)
    for (const QSharedPointer<GenericFader> &fader : allFaders())
    {
        if (fader.isNull())
            continue;
        fader->forEachChannel([&](FadeChannel &fc) {
            // (the FX player has its own faders: FX channels are never ours)
            const quint64 delay = fc.delay();
            const quint64 fade = fc.fadeTime();
            const quint64 elapsed = fc.elapsed();
            if ((fc.flags() & FadeChannel::Modulated) || fc.isReady() || elapsed >= delay + fade)
                return;
            // DMX Total Control: the ones of a hand crossfade, or the others
            if (((fc.flags() & FadeChannel::Manual) != 0) != manual)
                return;
            const uint newDelay = scaled(delay);
            const uint newFade = scaled(fade);
            quint64 newElapsed;
            if (elapsed < delay)
            {
                newElapsed = qMin(quint64(scaled(elapsed)), quint64(newDelay));
            }
            else
            {
                // in its fade: as far on its way, rounded up (a value never
                // goes back)
                const qreal into = fade > 0 ? std::ceil(qreal(elapsed - delay) * qreal(newFade) / qreal(fade)) : 0.0;
                newElapsed = quint64(newDelay) + qMin(quint64(into), quint64(newFade));
            }
            fc.setDelay(newDelay);
            fc.setFadeTime(newFade);
            fc.setElapsed(uint(qMin(newElapsed, quint64(UINT_MAX))));
        });
    }
}

void Cuelist::manualTimeLocked(uint ms)
{
    // (a channel at its end is ready: the hand can bring it back)
    for (const QSharedPointer<GenericFader> &fader : allFaders())
    {
        if (fader.isNull())
            continue;
        fader->forEachChannel([ms](FadeChannel &fc) {
            if ((fc.flags() & FadeChannel::Manual) == 0)
                return;
            fc.setReady(false);
            fc.setElapsed(ms);
            fc.nextStep(0);
        });
    }
}

void Cuelist::manualToClockLocked()
{
    // their times are cue ms: on the clock they go at the cue speed
    const qreal factor = m_appliedFactor > 0 ? m_appliedFactor : timeFactor();
    if (factor > 0 && factor != 1.0)
        scaleFadesLocked(1.0 / factor, true);
    for (const QSharedPointer<GenericFader> &fader : allFaders())
    {
        if (fader.isNull() == false)
            fader->forEachChannel([](FadeChannel &fc) { fc.removeFlag(FadeChannel::Manual); });
    }
}

bool Cuelist::endManualLocked(ManualEnd how, const QList<Universe *> &universes)
{
    m_manual = false;
    if (how != ManualCancel)
    {
        // complete: every fade at its end; else on from where it is (a
        // Follow after it counts from there)
        if (how == ManualComplete)
        {
            manualTimeLocked(m_manualLength);
            m_cueElapsed = m_fadeLength;
        }
        manualToClockLocked();
        m_followArmed = true;
        return false;
    }

    // back where it started: the channels at their start values (what the
    // stage showed), then the cue before snapped, as it was
    manualTimeLocked(0);
    manualToClockLocked();
    m_linkTaken = m_manualLinks;
    m_preselectUid = m_manualPreselect;
    int prev = indexOfUidLocked(m_manualPrevUid);
    if (m_manualPrevUid == 0)
    {
        // the travel started the list: it is released, and its next start
        // begins on that cue again
        m_startUid = m_currentUid;
        m_current = -1;
        m_currentUid = 0;
        m_releaseRequested = true;
        return false;
    }
    // (deleted meanwhile: the one before the cue of the travel)
    if (prev < 0)
        prev = qBound(0, m_current - 1, int(m_cues.count()) - 1);
    applyCueLocked(universes, prev, 0, 0, 0, ApplyNormal, nullptr, false);
    // the list waits on that look, as after BACK
    m_followArmed = false;
    return true;
}

void Cuelist::reapplyLocked(const QList<Universe *> &universes)
{
    // nothing plays yet (an empty list, or no GO yet)
    if (m_current < 0 && m_playing.isEmpty() && m_fxPlaying.isEmpty())
        return;

    int index = indexOfUidLocked(m_currentUid);
    // the current cue was deleted: the one before it plays
    if (index < 0 && m_current >= 0 && m_cues.isEmpty() == false)
        index = qBound(0, m_current - 1, int(m_cues.count()) - 1);

    m_current = index;
    m_currentUid = index >= 0 ? m_cues.at(index).uid : 0;
    m_cueLength = index >= 0 ? m_cues.at(index).length() : 0;
    // snapped, and the cue timers go on (its actions were done)
    applyCueLocked(universes, index, 0, 0, 0, ApplyLiveReapply, nullptr, false);
}

void Cuelist::applyCueLocked(const QList<Universe *> &universes, int index,
                             uint fade, uint down, uint delay, ApplyMode mode, const Cue *own,
                             bool withActions)
{
    const bool live = mode == ApplyLiveReapply;
    const State state = index >= 0 && index < m_cues.count() ? foldLocked(index, true) : State();

    // the channels the FX layers move belong to the FX player, with their
    // static values as its bases: our faders never hold them (an FX that
    // modulates what's below would find our fader above it)
    const QHash<Key, int> fxChannels = fxChannelsOf(state.fx);
    QHash<Key, uchar> values = state.values;
    QHash<Key, uchar> bases;
    for (auto it = fxChannels.cbegin(); it != fxChannels.cend(); ++it)
    {
        auto value = values.find(it.key());
        if (value == values.end())
            continue;
        // (the other byte of a pair that the FX doesn't name is no base: it
        // goes back to what's below)
        if (it.value() >= 0)
            bases.insert(it.key(), value.value());
        values.erase(value);
    }

    // FadeChannels count real ms, the cue timers virtual ms (the live speed).
    // DMX Total Control: a hand crossfade counts cue ms: the hand sets them
    const bool manual = mode == ApplyManual;
    auto real = [&](uint ms) { return manual ? ms : scaledTime(ms); };
    const uint fadeMs = live ? 0 : real(fade);
    const uint downMs = live ? 0 : real(down);
    const uint delayMs = live ? 0 : real(delay);
    // what our faders scale the intensities by
    const qreal level = m_appliedIntensity >= 0 ? m_appliedIntensity : getAttributeValue(Intensity);

    // the times of a channel: the same for all of them, or the attribute and
    // fixture times of $own. Both bytes of a coarse/fine pair take the times
    // of the coarse one: they share a FadeChannel. DMX Total Control: the
    // curve of $own too (one time for everything is linear)
    Times uniform;
    uniform.fade = fadeMs;
    uniform.down = downMs;
    uniform.delay = delayMs;
    uniform.curve = live == false && own != nullptr ? own->curve : int(FadeChannel::CurveLinear);
    const bool parts = live == false && own != nullptr && own->hasOwnTimes();
    QHash<const QLCChannel *, int> groups;
    auto timesOf = [&](const Fixture *fxi, quint32 channel) {
        if (parts == false)
            return uniform;
        const QLCFixtureMode *fixtureMode = fxi->fixtureMode();
        const quint32 primary = fixtureMode != NULL ? fixtureMode->primaryChannel(channel) : QLCChannel::invalid();
        const QLCChannel *ch = fxi->channel(primary != QLCChannel::invalid() ? primary : channel);
        auto group = groups.constFind(ch);
        if (group == groups.cend())
            group = groups.insert(ch, timeGroupOf(ch));
        Times t = own->timesFor(fxi->id(), group.value());
        t.fade = real(t.fade);
        t.down = real(t.down);
        t.delay = real(t.delay);
        return t;
    };

    // DMX Total Control: a GO, a jump or a BACK: Move in black prepares the
    // cue after this one again
    if (live == false)
        m_mibUid = 0;

    // new and changed targets
    setTargetsLocked(universes, values, nullptr, timesOf, live, manual);

    // what we hold and the cue doesn't (released, not tracked, Tracking off,
    // moved into an FX): it glides to what's below (or to its rest value),
    // then leaves the fader
    QSet<quint32> removedFixtures;
    for (auto it = m_playing.begin(); it != m_playing.end();)
    {
        if (values.contains(it.key()))
        {
            ++it;
            continue;
        }
        const Key key = it.key();
        it = m_playing.erase(it);
        if (live == false)
            m_changed.insert(key);

        Fixture *fxi = doc()->fixture(key.first);
        if (fxi == NULL)
        {
            removedFixtures.insert(key.first);
            continue;
        }
        if (key.second >= fxi->channels())
            continue;
        // it shares a FadeChannel with a channel that stays
        const quint32 paired = pairedChannel(fxi, key.second);
        if (paired != QLCChannel::invalid() && values.contains(Key(key.first, paired)))
            continue;
        const int universeIndex = int((fxi->universeAddress() + key.second) / UNIVERSE_SIZE);
        if (universeIndex >= universes.count())
            continue;
        Universe *universe = universes.at(universeIndex);
        // DMX Total Control: in the fader of ours that holds it
        QSharedPointer<GenericFader> fader = m_holder.value(key, m_fadersMap.value(universe->id()));
        if (fader.isNull())
            continue;
        const bool dimmer = isDimmer(fxi, key.second);
        const Times times = timesOf(fxi, key.second);
        // its value became the base of an FX (both bytes of a pair go
        // together): the player takes it over from what the stage shows (an
        // intensity from stage / level, FXScene::write), so it holds that
        // one more frame and leaves. Going to what's below first would flash
        // the HTP values of the lower playbacks
        const bool handover = bases.contains(key) ||
                              (paired != QLCChannel::invalid() && bases.contains(Key(key.first, paired)));

        fader->updateChannel(doc(), universe, key.first, key.second, [&](FadeChannel &fc)
        {
            // already on its way out
            if (fc.flags() & (FadeChannel::SetTarget | FadeChannel::AutoRemove))
                return;
            if (handover)
            {
                fc.setStart(fc.current());
                fc.setTarget(fc.current());
                fc.addFlag(FadeChannel::AutoRemove);
                fc.setElapsed(0);
                fc.setReady(false);
                fc.setFadeTime(0);
                fc.setDelay(0);
                fc.setCurve(FadeChannel::CurveLinear);
                return;
            }
            // from what it sends now: the fader scales intensities by the level.
            // DMX Total Control: one an Override list mixes keeps its own
            // value: the fader mixes it with what's below as it goes there
            const bool scaled = fc.canFade() && (fc.flags() & FadeChannel::Intensity) &&
                                (fc.flags() & (FadeChannel::CrossFade | FadeChannel::MixBelow)) == 0;
            const quint32 out = scaled ? fc.current(level) : fc.current();
            fc.setStart(out);
            fc.setCurrent(out);
            // GenericFader::write aims it at what's below every frame, and
            // drops it once it is there
            fc.addFlag(FadeChannel::SetTarget);
            fc.addFlag(FadeChannel::AutoRemove);
            fc.setTarget(quint32(0));
            fc.setElapsed(0);
            fc.setReady(false);
            fc.setFadeTime(fc.canFade() == false ? 0 : (dimmer ? times.down : times.fade));
            fc.setDelay(times.delay);
            fc.setCurve(times.curve);
            // DMX Total Control: a hand crossfade releases it by hand too
            if (manual)
                fc.addFlag(FadeChannel::Manual);
        });
    }

    // the channels of deleted fixtures leave the faders at once
    if (removedFixtures.isEmpty() == false)
    {
        // (DMX Total Control: the older faders too)
        for (const QSharedPointer<GenericFader> &fader : allFaders())
        {
            if (fader.isNull())
                continue;
            const QHash<quint32, FadeChannel> channels = fader->channels();
            for (const FadeChannel &fc : channels)
            {
                if (removedFixtures.contains(fc.fixture()))
                    fader->remove(&fc);
            }
        }
    }

    // the tracked FX layers, each on the fixtures it still holds (DMX Total
    // Control: a hand crossfade starts them in their own time, at the cue speed)
    const uint fxFadeMs = manual ? (own != nullptr ? scaledTime(own->fade) : 0) : fadeMs;
    playFxLocked(universes, state.fx, bases, fxChannels, fxFadeMs, level, mode);

    if (live == false && index >= 0 && index < m_cues.count())
    {
        const Cue &cue = m_cues.at(index);
        m_current = index;
        m_currentUid = cue.uid;
        // a super-scene seek starts the first cue in the middle
        m_cueElapsed = m_seekRemainder;
        m_seekRemainder = 0;
        // DMX Total Control: a Chase counts its step from here (a GO, a
        // jump), and Random draws the step after it
        m_chaseElapsed = m_cueElapsed;
        if (m_mode == ModeChase && m_chaseDirection == ChaseRandom)
            m_randomUid = m_cues.at(drawChaseStepLocked(index)).uid;
        m_cueLength = cue.length();
        // DMX Total Control: the fades as they run, which a BACK time or the
        // fade-in of a group or a block change: the status shows them (and
        // Pause/Back pauses a fade that runs), Follow keeps the cue's length.
        // With its own times: up to the longest of them
        const quint64 fadeEnd = quint64(m_cueElapsed) + (own != nullptr ? own->length() : delay + qMax(fade, down));
        m_fadeLength = uint(qMin(fadeEnd, quint64(UINT_MAX)));
        // DMX Total Control: its own times again (Live Time is for one run),
        // unless a BACK time, a fade-in or a Chase change replace them
        m_liveScale = 1.0;
        m_ownTimes = own != nullptr;
        m_notify.append(cue.uid);
        // DMX Total Control: what it does when it starts
        if (withActions && cue.actions.isEmpty() == false)
            m_actionNotify.append(cue.uid);
        m_firstOfRun = false;
    }
}

void Cuelist::setTargetsLocked(const QList<Universe *> &universes, const QHash<Key, uchar> &values,
                               const QSet<Key> *only, const std::function<Times(const Fixture *, quint32)> &timesOf,
                               bool live, bool manual)
{
    // what our faders scale the intensities by
    const qreal level = m_appliedIntensity >= 0 ? m_appliedIntensity : getAttributeValue(Intensity);
    // the coarse/fine channels updated by this apply (they share a FadeChannel)
    QSet<Key> pairsTouched;
    // DMX Total Control: the fader of each universe the changes go into
    QHash<quint32, QSharedPointer<GenericFader>> tops;
    // DMX Total Control: an Override list mixes its values with what's below
    const bool mix = m_runMode == ModeOverride;

    for (auto it = values.cbegin(); it != values.cend(); ++it)
    {
        const Key key = it.key();
        const uchar value = it.value();
        if (only != nullptr && only->contains(key) == false)
            continue;
        auto playing = m_playing.constFind(key);
        if (playing != m_playing.cend() && playing.value() == value)
            continue;

        Fixture *fxi = doc()->fixture(key.first);
        if (fxi == NULL || key.second >= fxi->channels())
            continue;
        // the universe of the channel, as Scene finds it
        const int universeIndex = int((fxi->universeAddress() + key.second) / UNIVERSE_SIZE);
        if (universeIndex >= universes.count())
            continue;
        Universe *universe = universes.at(universeIndex);
        if (universe == NULL)
            continue;
        // DMX Total Control: the last change wins: above the playbacks of
        // our priority that came after our fader, and a channel an older
        // fader of ours holds comes up into it
        auto top = tops.constFind(universe->id());
        if (top == tops.cend())
            top = tops.insert(universe->id(), topFaderLocked(universe));
        const QSharedPointer<GenericFader> fader = top.value();
        if (fader.isNull())
            continue;
        const bool moved = takeChannelLocked(universe, fader, fxi, key);

        // new to us, or on its way back: the channel holds what the stage
        // shows. DMX Total Control: an LTP channel that moved up too
        const bool retake = playing == m_playing.cend();
        const bool dimmer = isDimmer(fxi, key.second);
        const Times times = timesOf(fxi, key.second);
        const quint32 paired = pairedChannel(fxi, key.second);
        const Key pairKey(key.first, paired);
        const bool pairHeld = paired != QLCChannel::invalid() && values.contains(pairKey);
        // the other byte stays ours and keeps its target: the fade it runs
        // must not end sooner because this byte takes a shorter one
        bool pairKept = false;
        if (pairHeld)
        {
            auto pairPlaying = m_playing.constFind(pairKey);
            pairKept = pairsTouched.contains(pairKey) == false && pairPlaying != m_playing.cend() &&
                       pairPlaying.value() == values.value(pairKey);
            pairsTouched.insert(key);
        }

        fader->updateChannel(doc(), universe, key.first, key.second, [&](FadeChannel &fc)
        {
            const int i = fc.channelIndex(key.second);
            // what is left of the fade the channel runs (its delay included)
            const quint64 runs = quint64(fc.fadeTime()) + fc.delay();
            const uint left = fc.isReady() || fc.elapsed() >= runs ? 0 : uint(qMin(runs - fc.elapsed(), quint64(UINT_MAX)));
            // handed over to the FX player in this frame (AutoRemove alone):
            // it still holds our value, before the level (DMX Total Control:
            // unless it moved up: it holds what the stage shows)
            const bool handedOver = moved == false && (fc.flags() & FadeChannel::AutoRemove) &&
                                    (fc.flags() & FadeChannel::SetTarget) == 0;
            if (fc.flags() & FadeChannel::SetTarget)
            {
                // it was going back to what's below: ours again
                fc.removeFlag(FadeChannel::SetTarget);
            }
            fc.removeFlag(FadeChannel::AutoRemove);
            // DMX Total Control: an Override list. The fader mixes the whole
            // value with what's below (not a relative one: an offset)
            if (mix && (fc.flags() & FadeChannel::Relative) == 0)
                fc.addFlag(FadeChannel::MixBelow);
            int from = fc.current(i);
            // the stage shows it at our level: an intensity starts from
            // there, without a dip or a jump (DMX Total Control: mixed, the
            // fader doesn't scale it)
            if ((retake || moved) && handedOver == false && fc.canFade() && (fc.flags() & FadeChannel::Intensity) &&
                (fc.flags() & FadeChannel::MixBelow) == 0)
                from = level > 0.004 ? qMin(255, qRound(qreal(from) / level)) : value;
            fc.setStart(uchar(from), i);
            fc.setCurrent(uchar(from), i);
            fc.setTarget(value, i);
            // the other byte of a coarse/fine pair: the fade starts again, so
            // it goes on from where it is (no jump back to where its own fade
            // began), to the target it has when it is ours too, else it stays
            for (int j = 0; j < fc.channelCount(); j++)
            {
                if (j == i)
                    continue;
                fc.setStart(fc.current(j), j);
                if (pairHeld == false)
                    fc.setTarget(fc.current(j), j);
            }
            fc.setElapsed(0);
            fc.setReady(false);
            // the down fade is for dimmers going down; colours and moves take the fade
            uint time = dimmer && fc.target() < fc.start() ? times.down : times.fade;
            // one fade for both bytes: the one still on its way doesn't jump ahead
            if (pairKept)
                time = qMax(time, left);
            fc.setFadeTime(fc.canFade() == false ? 0 : time);
            fc.setDelay(times.delay);
            fc.setCurve(times.curve);
            // DMX Total Control: a hand crossfade (the A/B fader) sets its time
            if (manual)
                fc.addFlag(FadeChannel::Manual);
            else
                fc.removeFlag(FadeChannel::Manual);
        });

        // DMX Total Control: where it is now, both bytes of a pair
        m_holder.insert(key, fader);
        if (paired != QLCChannel::invalid())
            m_holder.insert(pairKey, fader);
        m_playing.insert(key, value);
        if (live == false)
            m_changed.insert(key);
    }
}

void Cuelist::moveInBlackLocked(const QList<Universe *> &universes)
{
    // (not in a Chase nor in an Override list: the type of an Override list
    // counts from its start, as its mix)
    if (m_moveInBlack == false || m_current < 0 || m_mode == ModeChase || m_runMode == ModeOverride)
        return;
    // the fades of the cue as they run (Live Time, a BACK time...), then the delay
    if (quint64(m_cueElapsed) < quint64(m_fadeLength) + m_mibDelay)
        return;

    // what a GO plays now: a cue pre-selected later is prepared instead
    const int next = nextLocked(nullptr);
    if (next < 0 || next == m_current)
        return;
    const Cue &cue = m_cues.at(next);
    if (cue.uid == m_mibUid || cue.noMib)
        return;
    m_mibUid = cue.uid;
    presetLocked(universes, next);
}

void Cuelist::presetLocked(const QList<Universe *> &universes, int next)
{
    const State state = foldLocked(next, true);
    // the FX of that cue and of this one: their channels start at the GO
    const QHash<Key, int> nextFx = fxChannelsOf(state.fx);
    auto isFx = [&](const Key &key) { return nextFx.contains(key) || m_fxChannels.contains(key); };

    // the channels of each fixture that cue holds
    QHash<quint32, QList<Key>> byFixture;
    for (auto it = state.values.cbegin(); it != state.values.cend(); ++it)
        byFixture[it.key().first].append(it.key());

    QSet<Key> prepared;
    for (auto it = byFixture.cbegin(); it != byFixture.cend(); ++it)
    {
        const Fixture *fxi = doc()->fixture(it.key());
        if (fxi == NULL)
            continue;
        const QList<quint32> light = lightChannels(fxi);
        if (light.isEmpty())
            continue;

        // lit by that cue: a value above 0, or an FX on its light
        bool lit = false;
        for (quint32 channel : light)
        {
            const Key key(fxi->id(), channel);
            if (state.values.value(key) > 0 || nextFx.contains(key))
                lit = true;
        }
        if (lit == false || isDarkLocked(universes, fxi, light) == false)
            continue;

        // its other attributes that the cue changes (RGB too, with a dimmer)
        for (const Key &key : it.value())
        {
            if (light.contains(key.second) || isFx(key))
                continue;
            auto playing = m_playing.constFind(key);
            if (playing == m_playing.cend() || playing.value() != state.values.value(key))
                prepared.insert(key);
        }
    }
    if (prepared.isEmpty())
        return;

    // in its own time, linear, with no delay. The programmer releases them
    // at the next GO, as if that GO had changed them
    Times times;
    times.fade = scaledTime(m_mibFade);
    times.down = times.fade;
    setTargetsLocked(universes, state.values, &prepared,
                     [&times](const Fixture *, quint32) { return times; }, false, false);
}

bool Cuelist::isDarkLocked(const QList<Universe *> &universes, const Fixture *fixture,
                           const QList<quint32> &light) const
{
    for (quint32 channel : light)
    {
        const Key key(fixture->id(), channel);
        // this cue lights it (its output can be 0 with the fader of the
        // list down), or an FX of it may: not dark
        if (m_playing.value(key) > 0 || m_fxChannels.contains(key))
            return false;
        const quint32 address = fixture->universeAddress() + channel;
        const int universeIndex = int(address / UNIVERSE_SIZE);
        if (universeIndex >= universes.count() || universes.at(universeIndex) == NULL)
            return false;
        // what the stage shows (another playback can light it)
        if (universes.at(universeIndex)->preGMValue(int(address % UNIVERSE_SIZE)) > 0)
            return false;
    }
    return true;
}

QList<quint32> Cuelist::lightChannels(const Fixture *fixture)
{
    QList<quint32> dimmers;
    QList<quint32> intensities;
    for (quint32 c = 0; c < fixture->channels(); c++)
    {
        if (isDimmer(fixture, c))
            dimmers.append(c);
        else if (fixture->channel(c) != NULL && fixture->channel(c)->group() == QLCChannel::Intensity)
            intensities.append(c);
    }
    return dimmers.isEmpty() ? intensities : dimmers;
}

QHash<Cuelist::Key, int> Cuelist::fxChannelsOf(const QList<FXScene::Layer> &layers) const
{
    QHash<Key, int> channels;
    for (int l = 0; l < layers.count(); l++)
    {
        const FXScene::Layer &layer = layers.at(l);
        const QStringList axes = fxAxes(layer.param);
        for (quint32 fid : layer.fixtures)
        {
            const Fixture *fxi = doc()->fixture(fid);
            if (fxi == NULL)
                continue;
            for (const QString &axis : axes)
            {
                // the channels FXScene finds for the axis; the first layer
                // that finds a channel moves it, as in FXScene
                quint32 fine = QLCChannel::invalid();
                const quint32 channel = FXScene::channelFor(fxi, axis, &fine);
                if (channel == QLCChannel::invalid() || channels.value(Key(fid, channel), -1) >= 0)
                    continue;
                channels.insert(Key(fid, channel), l);
                if (fine != QLCChannel::invalid())
                    channels.insert(Key(fid, fine), l);
                // GenericFader keeps a coarse channel and the fine one after
                // it in one FadeChannel: that byte can't stay in our faders
                const quint32 paired = pairedChannel(fxi, channel);
                if (paired != QLCChannel::invalid() && channels.contains(Key(fid, paired)) == false)
                    channels.insert(Key(fid, paired), -1);
            }
        }
    }

    // DMX Total Control: the emitters of the fixtures without a dimmer that a
    // dimmer layer dims (a virtual dimmer), once every layer has its
    // channels, as in FXScene: their values are bases of the player too
    for (int l = 0; l < layers.count(); l++)
    {
        if (layers.at(l).param != QLatin1String("dimmer"))
            continue;
        for (quint32 fid : layers.at(l).fixtures)
        {
            const Fixture *fxi = doc()->fixture(fid);
            const QList<QPair<quint32, quint32>> emitters = FXScene::virtualDimmerChannels(fxi);
            for (const QPair<quint32, quint32> &emitter : emitters)
            {
                if (channels.value(Key(fid, emitter.first), -1) >= 0)
                    continue;
                channels.insert(Key(fid, emitter.first), l);
                if (emitter.second != QLCChannel::invalid())
                    channels.insert(Key(fid, emitter.second), l);
                const quint32 paired = pairedChannel(fxi, emitter.first);
                if (paired != QLCChannel::invalid() && channels.contains(Key(fid, paired)) == false)
                    channels.insert(Key(fid, paired), -1);
            }
        }
    }
    return channels;
}

void Cuelist::playFxLocked(const QList<Universe *> &universes, const QList<FXScene::Layer> &layers,
                           const QHash<Key, uchar> &bases, const QHash<Key, int> &channels,
                           uint fadeMs, qreal level, ApplyMode mode)
{
    // a GO changes the FX channels whose layer or base changes, and the ones
    // an FX starts or stops on: the programmer releases them too (live edit)
    bool fxChanged = false;
    if (mode != ApplyLiveReapply)
    {
        auto differs = [&](const Key &key)
        {
            // its layer: an index, -1 (the other byte of a pair), -2 (not an FX channel)
            const int before = m_fxChannels.value(key, -2);
            const int now = channels.value(key, -2);
            if (before < 0 || now < 0)
                return before != now;
            if (m_fxPlaying.at(before) != layers.at(now) || bases.contains(key) != m_fxBases.contains(key))
                return true;
            return bases.contains(key) && bases.value(key) != m_fxBases.value(key);
        };
        for (auto it = channels.cbegin(); it != channels.cend(); ++it)
        {
            if (differs(it.key()))
            {
                m_changed.insert(it.key());
                fxChanged = true;
            }
        }
        for (auto it = m_fxChannels.cbegin(); it != m_fxChannels.cend(); ++it)
        {
            if (channels.contains(it.key()) == false)
                m_changed.insert(it.key());
        }
    }

    if (layers.isEmpty())
    {
        // the FX fade out to what's below them: our values, or the playbacks
        // under the list
        if (m_fxPlayer->isRunning())
        {
            m_fxPlayer->setFadeOutSpeed(fadeMs);
            m_fxPlayer->postRun(doc()->masterTimer(), universes);
        }
    }
    else if (m_fxPlayer->isRunning() == false || layers != m_fxPlaying || bases != m_fxBases)
    {
        QList<quint32> fixtures;
        QSet<quint32> seen;
        for (const FXScene::Layer &layer : layers)
        {
            for (quint32 fid : layer.fixtures)
            {
                if (seen.contains(fid) == false)
                {
                    seen.insert(fid);
                    fixtures.append(fid);
                }
            }
        }
        // running, it takes new layers and bases at once (FXScene snaps live
        // edits), and each layer goes on from where its wave is
        m_fxPlayer->setFixtures(fixtures);
        m_fxPlayer->setValues(bases);
        m_fxPlayer->setLayers(layers);
        if (m_fxPlayer->isRunning() == false)
        {
            // it fades in with the cue, at our priority and level. write()
            // runs it from the next frame on (this one included)
            m_fxPlayer->setName(name() + QStringLiteral(" (FX)"));
            m_fxPlayer->setFadeInSpeed(fadeMs);
            m_fxPlayer->setFadeOutSpeed(fadeMs);
            m_fxPlayer->setParentPriority(playbackPriority());
            m_fxPlayer->adjustAttribute(level, Function::Intensity);
            m_fxPlayer->preRun(doc()->masterTimer());
        }
    }

    // DMX Total Control: the last change wins: FX this GO changes go above
    // the playbacks of our priority (a new player requests new faders, on top)
    if (fxChanged && layers.isEmpty() == false && m_fxPlayer->isRunning())
        m_fxPlayer->raiseFaders();

    m_fxPlaying = layers;
    m_fxBases = bases;
    m_fxChannels = channels;
}

void Cuelist::updateStatusLocked()
{
    Status status;
    status.current = m_current;
    status.currentUid = m_currentUid;
    bool viaLink = false;
    status.next = nextLocked(&viaLink);
    // DMX Total Control: the next cue is a jump of the link of this one
    if (viaLink)
    {
        const Cue &cue = m_cues.at(m_current);
        status.linkJump = m_linkTaken.value(cue.uid) + 1;
        status.linkCount = cue.linkCount;
    }
    if (m_current >= 0)
    {
        // DMX Total Control: of the fades as they run (m_fadeLength), not the cue's own times
        status.progress = m_fadeLength > 0 ? qMin(1.0, qreal(m_cueElapsed) / qreal(m_fadeLength)) : 1.0;
        status.fading = m_cueElapsed < m_fadeLength;
        status.waiting = m_followArmed && autoTriggerLocked(status.next) != nullptr;
        // DMX Total Control: Live Time shows and changes them
        status.fadeTotal = m_fadeLength;
        status.fadeElapsed = qMin(m_cueElapsed, m_fadeLength);
        status.liveScale = m_liveScale;
        status.ownTimes = m_ownTimes;
        // DMX Total Control: the A/B fader has it
        status.manual = m_manual;
    }
    // DMX Total Control: the clock of a Timecode list, also before its first
    // cue (a MIDI timecode before the offset: 00:00)
    if (m_mode == ModeTimecode && isRunning())
        status.clock = qMax(qint64(0), m_tcClock);

    QMutexLocker locker(&m_statusMutex);
    m_status = status;
}

void Cuelist::postRun(MasterTimer *timer, QList<Universe *> universes)
{
    // the release time, or the fade out of the group or the super-scene block
    const uint release = overrideFadeOutSpeed() != defaultSpeed() ? overrideFadeOutSpeed() : fadeOutSpeed();

    // a paused list that is released would freeze: its faders go on first.
    // DMX Total Control: the player's too, and the FX it is still fading out
    // (when it doesn't run, its postRun below doesn't do it). DMX Total
    // Control: the older faders go with the newest ones
    const QList<QSharedPointer<GenericFader>> faders = allFaders();
    for (const QSharedPointer<GenericFader> &fader : faders)
    {
        if (!fader.isNull())
            fader->setPaused(false);
    }
    m_fxPlayer->setFadersPaused(false);
    m_fadersPaused = false;

    if (release == 0)
    {
        for (const auto &older : std::as_const(m_olderFaders))
        {
            if (!older.second.isNull())
                older.second->requestDelete();
        }
        dismissAllFaders();
    }
    else
    {
        // everything crossfades to what's below; the universe drops the faders
        for (const QSharedPointer<GenericFader> &fader : faders)
        {
            if (!fader.isNull())
                fader->setFadeOut(true, release);
        }
        m_fadersMap.clear();
    }
    m_olderFaders.clear();
    m_holder.clear();

    // the FX fade out with the same time, in this frame: when the list has
    // stopped (stopAndWait) the player has too, and nothing runs it any more
    if (m_fxPlayer->isRunning())
    {
        m_fxPlayer->setFadeOutSpeed(release);
        m_fxPlayer->postRun(timer, universes);
    }

    {
        QMutexLocker locker(&m_mutex);
        // to resume from; m_startUid is kept (a stop and a start in the same tick)
        m_lastUid = m_currentUid;
        m_playing.clear();
        m_fxPlaying.clear();
        m_fxBases.clear();
        m_fxChannels.clear();
        m_actions.clear();
        m_notify.clear();
        m_actionNotify.clear();
        // a release counts the links again
        m_linkTaken.clear();
        m_preselectUid = 0;
        m_current = -1;
        m_currentUid = 0;
        m_followArmed = false;
        m_releaseRequested = false;
        m_cueElapsed = 0;
        m_cueLength = 0;
        m_fadeLength = 0;
        m_liveScale = 1.0;
        m_ownTimes = true;
        m_mibUid = 0;
        m_manual = false;

        Status status;
        status.next = startIndexLocked();
        QMutexLocker statusLocker(&m_statusMutex);
        m_status = status;
    }

    Function::postRun(timer, universes);
}

void Cuelist::updateFadersPriority()
{
    // DMX Total Control: the older faders first, so each one stays below
    // the ones that came after it
    InputOutputMap *io = doc()->inputOutputMap();
    for (const auto &older : std::as_const(m_olderFaders))
    {
        Universe *universe = io->universe(older.first);
        if (universe != nullptr && older.second.isNull() == false)
            universe->setFaderFunctionPriority(older.second, playbackPriority());
    }
    Function::updateFadersPriority();
    m_fxPlayer->setParentPriority(playbackPriority());
}

qreal Cuelist::frameMotion() const
{
    // DMX Total Control: not the motion speed (its FX take it), the cue speed
    const MasterTimer *timer = doc() != NULL ? doc()->masterTimer() : NULL;
    return timer != NULL ? timer->cueSpeed() : 1.0;
}
