/*
  DMX Total Control
  zone.cpp

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
#include <QDebug>

#include "zone.h"
#include "scenetaps.h"
#include "zonemanager.h"
#include "doc.h"
#include "function.h"
#include "functionparent.h"
#include "curvefx.h"
#include "fxscene.h"
#include "show.h"
#include "efx.h"

#define KXMLDTCZoneID        QStringLiteral("ID")
#define KXMLDTCZoneName      QStringLiteral("Name")
#define KXMLDTCZoneColor     QStringLiteral("Color")
#define KXMLDTCZoneExclusive QStringLiteral("Exclusive")
#define KXMLDTCZoneFade      QStringLiteral("Fade")
#define KXMLDTCZoneMaster    QStringLiteral("Master")
#define KXMLDTCZoneFunction  QStringLiteral("Function")
#define KXMLDTCZoneRelease   QStringLiteral("Release")
#define KXMLDTCZoneProtect   QStringLiteral("Protect")
#define KXMLDTCZoneFlash     QStringLiteral("Flash")
#define KXMLDTCZoneDimmer    QStringLiteral("Dimmer")
#define KXMLDTCZoneSpeed     QStringLiteral("Speed")
#define KXMLDTCZonePhase     QStringLiteral("Phase")
#define KXMLDTCZoneSize      QStringLiteral("Size")
#define KXMLDTCZoneFadeSpeed QStringLiteral("FadeSpeed")

Zone::Zone(Doc *doc, quint32 id, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_id(id)
    , m_color(QColor("#3d8bfd"))
    , m_exclusive(true)
    , m_fadeTime(-1)
    , m_master(1.0)
    , m_lastStarted(Function::invalidId())
{
    Q_ASSERT(m_doc != nullptr);
    connect(m_doc, SIGNAL(functionRemoved(quint32)), this, SLOT(slotFunctionRemoved(quint32)));
    connect(m_doc, SIGNAL(functionReplaced(quint32)), this, SLOT(slotFunctionReplaced(quint32)));
}

Zone::~Zone()
{
    stopAll();
}

quint32 Zone::id() const
{
    return m_id;
}

QString Zone::name() const
{
    return m_name;
}

void Zone::setName(const QString &name)
{
    if (name == m_name)
        return;
    m_name = name;
    m_doc->setModified();
    emit nameChanged();
}

QColor Zone::color() const
{
    return m_color;
}

void Zone::setColor(const QColor &color)
{
    if (color == m_color)
        return;
    m_color = color;
    m_doc->setModified();
    emit colorChanged();
}

bool Zone::exclusive() const
{
    return m_exclusive;
}

void Zone::setExclusive(bool exclusive)
{
    if (exclusive == m_exclusive)
        return;
    m_exclusive = exclusive;
    m_doc->setModified();
    emit exclusiveChanged();
}

int Zone::fadeTime() const
{
    return m_fadeTime;
}

void Zone::setFadeTime(int ms)
{
    if (ms < 0)
        ms = -1;
    if (ms == m_fadeTime)
        return;
    m_fadeTime = ms;
    m_doc->setModified();
    emit fadeTimeChanged();
}

qreal Zone::master() const
{
    return m_master;
}

void Zone::setMaster(qreal value)
{
    value = qBound(0.0, value, 1.0);
    if (qFuzzyCompare(value + 1.0, m_master + 1.0))
        return;
    m_master = value;

    for (quint32 fid : m_intensityOverrides.keys())
        applyIntensity(fid);

    // saved with the show, like the dimmers of the scenes
    m_doc->setModified();
    emit masterChanged();
}

void Zone::applyIntensity(quint32 fid)
{
    Function *f = m_doc->function(fid);
    if (f == nullptr)
        return;
    const qreal level = m_master * sceneDimmer(fid) * sceneLevel(fid);
    m_intensityOverrides.insert(fid, applyLevel(m_doc, f, level, m_intensityOverrides.value(fid)));
}

QList<int> Zone::applyLevel(Doc *doc, Function *f, qreal level, const QList<int> &ids)
{
    if (f == nullptr)
        return QList<int>();

    // a super scene has no intensity of its own: a level per track (a track
    // removed while it plays moves the overrides after it down a place)
    if (Show *show = qobject_cast<Show *>(f))
    {
        const int count = int(show->attributes().count());
        QList<int> kept(count, Function::invalidAttributeId());
        for (int id : ids)
        {
            const int index = show->adjustAttribute(level, id);
            if (index >= 0 && index < count && kept.at(index) == Function::invalidAttributeId())
                kept[index] = id;
        }
        for (int i = 0; i < count; i++)
        {
            if (kept.at(i) == Function::invalidAttributeId())
                kept[i] = show->requestAttributeOverride(i, level);
        }

        // the static values play with it, as a scene of its own
        if (Function *values = doc->function(show->staticScene()))
            values->requestAttributeOverride(Function::Intensity, level);
        return kept;
    }

    // (an override of a previous run is gone: the function dropped it when
    // it stopped, and its number may go to another one)
    if (ids.isEmpty() == false && f->adjustAttribute(level, ids.first()) >= 0)
        return ids;
    const int overrideId = f->requestAttributeOverride(Function::Intensity, level);
    if (overrideId == Function::invalidAttributeId())
        return QList<int>();
    return QList<int>({ overrideId });
}

/*********************************************************************
 * Functions
 *********************************************************************/

QList<quint32> Zone::functionIDs() const
{
    return m_functions;
}

QVariantList Zone::functionsInfo() const
{
    QVariantList list;
    for (quint32 fid : m_functions)
    {
        Function *f = m_doc->function(fid);
        if (f == nullptr)
            continue;

        QVariantMap info;
        info.insert("id", fid);
        info.insert("name", f->name());
        info.insert("type", Function::typeToString(f->type()));
        info.insert("active", m_active.contains(fid));
        info.insert("priority", f->priority());
        // its own rhythm, tapped on its button (ms a step, 0: none)
        info.insert("tap", f->tapPeriod());
        Options opt = m_options.value(fid);
        info.insert("release", opt.release);
        info.insert("protection", opt.protection);
        info.insert("flash", opt.flash);
        info.insert("dimmer", opt.dimmer);
        list.append(info);
    }
    return list;
}

int Zone::activeCount() const
{
    return m_active.count();
}

void Zone::addFunction(quint32 fid)
{
    if (m_functions.contains(fid) || m_doc->function(fid) == nullptr)
        return;

    m_functions.append(fid);
    connectFunction(fid);
    m_doc->setModified();
    emit functionsChanged();
}

void Zone::removeFunction(quint32 fid)
{
    if (m_functions.contains(fid) == false)
        return;

    stop(fid);
    disconnectFunction(fid);
    m_functions.removeAll(fid);
    m_options.remove(fid);
    m_doc->setModified();
    emit functionsChanged();
}

void Zone::moveFunction(int from, int to)
{
    if (from < 0 || from >= m_functions.count() || to < 0 || to >= m_functions.count() || from == to)
        return;

    m_functions.move(from, to);
    m_doc->setModified();
    emit functionsChanged();
}

void Zone::connectFunction(quint32 fid)
{
    Function *f = m_doc->function(fid);
    if (f == nullptr)
        return;

    connect(f, SIGNAL(running(quint32)), this, SLOT(slotFunctionRunning(quint32)), Qt::UniqueConnection);
    connect(f, SIGNAL(stopped(quint32)), this, SLOT(slotFunctionStopped(quint32)), Qt::UniqueConnection);
    connect(f, SIGNAL(nameChanged(quint32)), this, SIGNAL(functionsChanged()), Qt::UniqueConnection);
    connect(f, SIGNAL(changed(quint32)), this, SIGNAL(functionsChanged()), Qt::UniqueConnection);
    connect(f, SIGNAL(tapChanged(quint32)), this, SIGNAL(functionsChanged()), Qt::UniqueConnection);
    connect(f, SIGNAL(loopsFinished(quint32)), this, SLOT(slotLoopsFinished(quint32)), Qt::UniqueConnection);
}

void Zone::disconnectFunction(quint32 fid)
{
    Function *f = m_doc->function(fid);
    if (f != nullptr)
        disconnect(f, nullptr, this, nullptr);
}

/*********************************************************************
 * Playback
 *********************************************************************/

uint Zone::fadeIn() const
{
    return m_fadeTime < 0 ? Function::defaultSpeed() : uint(m_fadeTime);
}

uint Zone::fadeOut() const
{
    return m_fadeTime < 0 ? Function::defaultSpeed() : uint(m_fadeTime);
}

void Zone::trigger(quint32 fid)
{
    const bool active = m_active.contains(fid);
    // a scene started paused plays at the second press (with Stop, it stops)
    if (active && SceneTaps::mode() != SceneTaps::Stop && m_waiting.remove(fid))
    {
        if (Function *f = m_doc->function(fid))
        {
            f->setPause(false);
            // (its rhythm counts from this press, as from a start)
            SceneTaps::press(f, false);
        }
        emit functionsChanged();
        return;
    }

    // what its button does in the mode of the buttons (GO, Stop or as
    // always): start it, stop it, or a beat of its own rhythm (a new rhythm
    // changes the function, and so the list)
    switch (SceneTaps::press(m_doc->function(fid), active))
    {
        case SceneTaps::StartIt:
            start(fid);
            break;
        case SceneTaps::StopIt:
            stop(fid);
            break;
        case SceneTaps::ToggleIt:
            if (active)
                stop(fid);
            else
                start(fid);
            break;
        case SceneTaps::Keep:
            break;
    }
}

void Zone::start(quint32 fid)
{
    Function *f = m_doc->function(fid);
    if (f == nullptr || m_functions.contains(fid) == false)
        return;

    // release the scenes this one replaces. Releases across groups need
    // the manager; a standalone zone only handles its own scenes
    ZoneManager *manager = qobject_cast<ZoneManager *>(parent());
    if (manager != nullptr)
    {
        manager->releaseFor(this, fid);
    }
    else if (effectiveRelease(fid) == ReleaseGroup)
    {
        const QList<quint32> running = m_active.values();
        for (quint32 other : running)
        {
            if (other != fid)
                stop(other);
        }
    }

    // The crossfade is the new function fading in while the previous one
    // fades out. Both are run time overrides: the function itself is untouched.
    // The live fade speed stretches or shortens them
    const Options opt = m_options.value(fid);
    uint in = fadeIn(), out = fadeOut();
    if (opt.fadeSpeed != 1.0)
    {
        in = uint(qreal(in == Function::defaultSpeed() ? f->fadeInSpeed() : in) / opt.fadeSpeed);
        out = uint(qreal(out == Function::defaultSpeed() ? f->fadeOutSpeed() : out) / opt.fadeSpeed);
    }

    f->setSpeedFactor(opt.speed);
    if (f->startMode() == Function::StartPaused)
        m_waiting.insert(fid);

    // the master first, so its first frame isn't at full (it is given again
    // when it runs: a previous run still stopping drops the overrides)
    applyIntensity(fid);
    f->start(m_doc->masterTimer(), FunctionParent(FunctionParent::ManualVCWidget, m_id), 0, in, out);

    m_active.insert(fid);
    m_lastStarted = fid;
    emit activeChanged();
    emit functionsChanged();
}

void Zone::stop(quint32 fid)
{
    if (m_active.contains(fid) == false)
        return;

    Function *f = m_doc->function(fid);
    if (f != nullptr)
        f->stop(FunctionParent(FunctionParent::ManualVCWidget, m_id));

    m_waiting.remove(fid);
    m_liveOverrides.remove(fid);
    m_active.remove(fid);
    m_intensityOverrides.remove(fid);
    m_sceneLevels.remove(fid);
    emit activeChanged();
    emit functionsChanged();
}

void Zone::stopAll()
{
    const QList<quint32> running = m_active.values();
    for (quint32 fid : running)
        stop(fid);
}

bool Zone::isActive(quint32 fid) const
{
    return m_active.contains(fid);
}

QList<quint32> Zone::activeFunctions() const
{
    return m_active.values();
}

void Zone::press(quint32 fid)
{
    if (isFlash(fid))
        start(fid);
    else
        trigger(fid);
}

void Zone::unpress(quint32 fid)
{
    if (isFlash(fid))
        stop(fid);
}

void Zone::step(int direction)
{
    if (m_functions.isEmpty())
        return;

    int current = m_functions.indexOf(m_lastStarted);
    if (current < 0 || m_active.contains(m_lastStarted) == false)
        current = direction > 0 ? -1 : 0;

    int count = int(m_functions.count());
    quint32 target = m_functions.at(((current + direction) % count + count) % count);

    if (m_active.contains(m_lastStarted) && m_lastStarted != target)
        stop(m_lastStarted);
    start(target);
}

void Zone::next()
{
    step(1);
}

void Zone::previous()
{
    step(-1);
}

QVariantMap Zone::options(quint32 fid) const
{
    Options opt = m_options.value(fid);
    QVariantMap map;
    map.insert("release", opt.release);
    map.insert("protection", opt.protection);
    map.insert("flash", opt.flash);
    map.insert("dimmer", opt.dimmer);
    map.insert("speed", opt.speed);
    map.insert("phase", opt.phase);
    map.insert("size", opt.size);
    map.insert("fadeSpeed", opt.fadeSpeed);
    map.insert("waiting", m_waiting.contains(fid));
    return map;
}

void Zone::setOption(quint32 fid, const QString &key, const QVariant &value)
{
    if (m_functions.contains(fid) == false)
        return;

    Options &opt = m_options[fid];
    if (key == "release")
        opt.release = qBound(int(ReleaseGroupDefault), value.toInt(), int(ReleaseOthers));
    else if (key == "protection")
        opt.protection = qBound(int(ProtectNone), value.toInt(), int(ProtectOthers));
    else if (key == "flash")
        opt.flash = value.toBool();
    else if (key == "dimmer")
    {
        opt.dimmer = qBound(0.0, value.toDouble(), 1.0);
        // live control: apply it right away to the running scene
        if (m_intensityOverrides.contains(fid))
            applyIntensity(fid);
    }
    else if (key == "speed")
    {
        opt.speed = qBound(0.1, value.toDouble(), 10.0);
        Function *f = m_doc->function(fid);
        if (f != nullptr && m_active.contains(fid))
            f->setSpeedFactor(opt.speed);
    }
    else if (key == "phase")
    {
        opt.phase = ((value.toInt() % 360) + 360) % 360;
        applyLiveControls(fid);
    }
    else if (key == "size")
    {
        opt.size = qBound(0.0, value.toDouble(), 2.0);
        applyLiveControls(fid);
    }
    else if (key == "fadeSpeed")
    {
        opt.fadeSpeed = qBound(0.1, value.toDouble(), 10.0);
    }
    else
        return;

    m_doc->setModified();
    emit functionsChanged();
}

Zone::ReleaseMode Zone::effectiveRelease(quint32 fid) const
{
    int release = m_options.value(fid).release;
    if (release == ReleaseGroupDefault)
        return m_exclusive ? ReleaseGroup : ReleaseNone;
    return ReleaseMode(release);
}

Zone::Protection Zone::protection(quint32 fid) const
{
    return Protection(m_options.value(fid).protection);
}

bool Zone::isFlash(quint32 fid) const
{
    return m_options.value(fid).flash;
}

qreal Zone::sceneDimmer(quint32 fid) const
{
    return m_options.value(fid).dimmer;
}

void Zone::setSceneLevel(quint32 fid, qreal level)
{
    // (a scene this group doesn't play: its next run starts at full)
    if (m_active.contains(fid) == false)
        return;
    level = qBound(0.0, level, 1.0);
    if (m_sceneLevels.contains(fid) && qFuzzyCompare(level + 1.0, sceneLevel(fid) + 1.0))
        return;
    m_sceneLevels.insert(fid, level);
    applyIntensity(fid);
}

qreal Zone::sceneLevel(quint32 fid) const
{
    return m_sceneLevels.value(fid, 1.0);
}

void Zone::slotFunctionRunning(quint32 fid)
{
    if (m_active.contains(fid) == false)
        return;

    // Overrides are reset when a function stops: the level again every time
    // the function actually starts running (the overrides asked at the start
    // stay, those of a previous run are asked again)
    Function *f = m_doc->function(fid);
    if (f == nullptr)
        return;

    applyIntensity(fid);

    // the live controls, and the pause of a scene that starts paused
    m_liveOverrides.remove(fid);
    f->setSpeedFactor(m_options.value(fid).speed);
    applyLiveControls(fid);
    if (m_waiting.contains(fid))
    {
        f->setPause(true);
        emit functionsChanged();
    }
}

void Zone::slotFunctionStopped(quint32 fid)
{
    Function *f = m_doc->function(fid);

    // the signal of a previous run, arriving after the scene was started
    // again: the new run is not concerned when this group plays in it (a
    // quick second press). Started again by someone else only (a playback
    // released it and played it again in the same frame), the run of this
    // group is over: it doesn't play it any more, nor gives it its master
    const bool restarted = f != nullptr && f->stopped() == false;
    if (restarted && f->isStartedBy(FunctionParent(FunctionParent::ManualVCWidget, m_id)))
        return;

    m_intensityOverrides.remove(fid);
    m_liveOverrides.remove(fid);
    m_waiting.remove(fid);

    // the live speed belongs to this playback only
    if (f != nullptr && (f->isRunning() == false || (restarted && m_active.contains(fid))))
        f->setSpeedFactor(1.0);

    // A function stopping on its own (e.g. a single shot chaser) or because
    // someone else stopped it is no longer active in this zone
    if (m_active.contains(fid) && (f == nullptr || f->isRunning() == false || restarted))
    {
        m_active.remove(fid);
        m_sceneLevels.remove(fid);
        emit activeChanged();
        emit functionsChanged();
    }
}

void Zone::slotFunctionReplaced(quint32 fid)
{
    // the same ID, a new function (an effect converted to FX): listen to it
    if (m_functions.contains(fid))
    {
        m_active.remove(fid);
        m_intensityOverrides.remove(fid);
        m_sceneLevels.remove(fid);
        connectFunction(fid);
        emit activeChanged();
        emit functionsChanged();
    }
}

void Zone::slotFunctionRemoved(quint32 fid)
{
    if (m_functions.removeAll(fid) > 0)
    {
        m_active.remove(fid);
        m_intensityOverrides.remove(fid);
        m_sceneLevels.remove(fid);
        emit activeChanged();
        emit functionsChanged();
    }
}

/*********************************************************************
 * Load & save
 *********************************************************************/

bool Zone::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLDTCZone)
    {
        qWarning() << Q_FUNC_INFO << "Zone node not found";
        return false;
    }

    // (what a damaged file has wrong keeps the default)
    QXmlStreamAttributes attrs = root.attributes();
    m_name = attrs.value(KXMLDTCZoneName).toString();
    const QColor color(attrs.value(KXMLDTCZoneColor).toString());
    if (color.isValid())
        m_color = color;
    bool exclusiveOk = false;
    const int exclusive = attrs.value(KXMLDTCZoneExclusive).toInt(&exclusiveOk);
    if (exclusiveOk)
        m_exclusive = exclusive != 0;
    bool fadeOk = false;
    const int fade = attrs.value(KXMLDTCZoneFade).toInt(&fadeOk);
    if (fadeOk)
        m_fadeTime = qMax(-1, fade);
    bool masterOk = false;
    const qreal master = attrs.value(KXMLDTCZoneMaster).toDouble(&masterOk);
    if (masterOk && qIsFinite(master))
        m_master = qBound(0.0, master, 1.0);

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLDTCZoneFunction)
        {
            bool idOk = false;
            quint32 fid = root.attributes().value(KXMLDTCZoneID).toUInt(&idOk);
            if (idOk && m_doc->function(fid) != nullptr && m_functions.contains(fid) == false)
            {
                m_functions.append(fid);
                connectFunction(fid);

                QXmlStreamAttributes fattrs = root.attributes();
                Options opt;
                // in the ranges setOption() keeps them in; a value that
                // isn't a number keeps the default
                auto integer = [&fattrs](const QString &key, int fallback) {
                    bool ok = false;
                    const int value = fattrs.value(key).toInt(&ok);
                    return ok ? value : fallback;
                };
                auto real = [&fattrs](const QString &key, qreal min, qreal max, qreal fallback) {
                    bool ok = false;
                    const qreal value = fattrs.value(key).toDouble(&ok);
                    return ok && qIsFinite(value) ? qBound(min, value, max) : fallback;
                };
                opt.release = qBound(int(ReleaseGroupDefault), integer(KXMLDTCZoneRelease, opt.release), int(ReleaseOthers));
                opt.protection = qBound(int(ProtectNone), integer(KXMLDTCZoneProtect, opt.protection), int(ProtectOthers));
                opt.flash = fattrs.value(KXMLDTCZoneFlash).toInt() != 0;
                opt.dimmer = real(KXMLDTCZoneDimmer, 0.0, 1.0, opt.dimmer);
                opt.speed = real(KXMLDTCZoneSpeed, 0.1, 10.0, opt.speed);
                opt.phase = ((integer(KXMLDTCZonePhase, opt.phase) % 360) + 360) % 360;
                opt.size = real(KXMLDTCZoneSize, 0.0, 2.0, opt.size);
                opt.fadeSpeed = real(KXMLDTCZoneFadeSpeed, 0.1, 10.0, opt.fadeSpeed);
                m_options.insert(fid, opt);
            }
        }
        root.skipCurrentElement();
    }

    emit nameChanged();
    emit colorChanged();
    emit exclusiveChanged();
    emit fadeTimeChanged();
    emit masterChanged();
    emit functionsChanged();

    return true;
}

bool Zone::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != nullptr);

    doc->writeStartElement(KXMLDTCZone);
    doc->writeAttribute(KXMLDTCZoneID, QString::number(m_id));
    doc->writeAttribute(KXMLDTCZoneName, m_name);
    doc->writeAttribute(KXMLDTCZoneColor, m_color.name());
    doc->writeAttribute(KXMLDTCZoneExclusive, m_exclusive ? "1" : "0");
    doc->writeAttribute(KXMLDTCZoneFade, QString::number(m_fadeTime));
    doc->writeAttribute(KXMLDTCZoneMaster, QString::number(m_master));

    for (quint32 fid : m_functions)
    {
        doc->writeStartElement(KXMLDTCZoneFunction);
        doc->writeAttribute(KXMLDTCZoneID, QString::number(fid));
        Options opt = m_options.value(fid);
        if (opt.release != ReleaseGroupDefault)
            doc->writeAttribute(KXMLDTCZoneRelease, QString::number(opt.release));
        if (opt.protection != ProtectNone)
            doc->writeAttribute(KXMLDTCZoneProtect, QString::number(opt.protection));
        if (opt.flash)
            doc->writeAttribute(KXMLDTCZoneFlash, "1");
        if (opt.dimmer < 1.0)
            doc->writeAttribute(KXMLDTCZoneDimmer, QString::number(opt.dimmer));
        if (opt.speed != 1.0)
            doc->writeAttribute(KXMLDTCZoneSpeed, QString::number(opt.speed));
        if (opt.phase != 0)
            doc->writeAttribute(KXMLDTCZonePhase, QString::number(opt.phase));
        if (opt.size != 1.0)
            doc->writeAttribute(KXMLDTCZoneSize, QString::number(opt.size));
        if (opt.fadeSpeed != 1.0)
            doc->writeAttribute(KXMLDTCZoneFadeSpeed, QString::number(opt.fadeSpeed));
        doc->writeEndElement();
    }

    doc->writeEndElement();
    return true;
}

/*********************************************************************
 * Live controls, start mode and loops
 *********************************************************************/

bool Zone::isWaiting(quint32 fid) const
{
    return m_waiting.contains(fid);
}

void Zone::applyLiveControls(quint32 fid)
{
    Function *f = m_doc->function(fid);
    if (f == nullptr || m_active.contains(fid) == false || f->isRunning() == false)
        return;

    const Options opt = m_options.value(fid);
    QHash<int, int> &overrides = m_liveOverrides[fid];

    // request an override the first time a control moves from its default
    auto set = [&](int attribute, qreal value, bool isDefault)
    {
        if (overrides.contains(attribute))
        {
            f->adjustAttribute(value, overrides.value(attribute));
        }
        else if (isDefault == false)
        {
            int id = f->requestAttributeOverride(attribute, value);
            if (id != Function::invalidAttributeId())
                overrides.insert(attribute, id);
        }
    };

    // Size scales the movement of a Move FX, Phase shifts it
    if (EFX *efx = qobject_cast<EFX *>(f))
    {
        set(EFX::Width, qBound(0.0, efx->width() * opt.size, 127.0), opt.size == 1.0);
        set(EFX::Height, qBound(0.0, efx->height() * opt.size, 127.0), opt.size == 1.0);
        set(EFX::StartOffset, (efx->startOffset() + opt.phase) % 360, opt.phase == 0);
    }
    else if (qobject_cast<CurveFX *>(f) != nullptr)
    {
        set(CurveFX::Size, opt.size, opt.size == 1.0);
        set(CurveFX::Phase, opt.phase, opt.phase == 0);
    }
    else if (qobject_cast<FXScene *>(f) != nullptr)
    {
        set(FXScene::Size, opt.size, opt.size == 1.0);
        set(FXScene::Phase, opt.phase, opt.phase == 0);
    }
}

void Zone::slotLoopsFinished(quint32 fid)
{
    Function *f = m_doc->function(fid);
    if (f == nullptr || m_active.contains(fid) == false)
        return;

    int jump = f->jumpTo();
    if (jump == Function::jumpNone())
        return;

    // next or previous scene of this group
    if (jump == Function::jumpNext() || jump == Function::jumpPrevious())
    {
        int index = m_functions.indexOf(fid);
        int count = int(m_functions.count());
        if (index < 0 || count < 2)
            return;
        int direction = jump == Function::jumpNext() ? 1 : -1;
        start(m_functions.at(((index + direction) % count + count) % count));
        return;
    }

    // any scene: in its group if it has one
    quint32 target = quint32(jump);
    if (m_functions.contains(target))
    {
        start(target);
        return;
    }
    if (ZoneManager *manager = qobject_cast<ZoneManager *>(parent()))
    {
        for (int i = 0; i < manager->count(); i++)
        {
            Zone *zone = manager->zone(i);
            if (zone->functionIDs().contains(target))
            {
                zone->start(target);
                return;
            }
        }
    }
    if (Function *other = m_doc->function(target))
        other->start(m_doc->masterTimer(), FunctionParent::master());
}
