/*
  DMX Total Control
  liverecall.cpp

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

#include <QDateTime>
#include <QPointer>
#include <QTimer>
#include <QSet>

#include "liverecall.h"
#include "playbackmanager.h"
#include "safetymanager.h"
#include "controlmapper.h"
#include "zonemanager.h"
#include "showsurface.h"
#include "dtccore.h"
#include "zone.h"

#include "functionparent.h"
#include "mastertimer.h"
#include "function.h"
#include "cuelist.h"
#include "doc.h"

/*********************************************************************
 * The snapshot
 *********************************************************************/

void LiveRecall::Snapshot::merge(const Snapshot &later)
{
    // (each function once: the one taken first, but a cuelist on the cue it
    // played last)
    QSet<quint32> scenes;
    for (const ZoneScene &scene : std::as_const(zoneScenes))
        scenes.insert(scene.fid);
    for (const Run &run : std::as_const(functions))
        scenes.insert(run.fid);
    for (const ZoneScene &scene : later.zoneScenes)
    {
        if (scenes.contains(scene.fid) == false)
        {
            zoneScenes.append(scene);
            scenes.insert(scene.fid);
        }
    }
    for (const Run &run : later.functions)
    {
        if (scenes.contains(run.fid) == false)
        {
            functions.append(run);
            scenes.insert(run.fid);
        }
    }
    for (const List &list : later.cuelists)
    {
        bool found = false;
        for (List &mine : cuelists)
        {
            if (mine.fid == list.fid)
            {
                mine = list;
                found = true;
            }
        }
        if (found == false)
            cuelists.append(list);
    }
    takenAt = later.takenAt;
    reason = later.reason;
}

QVariantMap LiveRecall::Snapshot::toVariant() const
{
    QVariantList scenes, runs, lists;
    for (const ZoneScene &scene : zoneScenes)
        scenes.append(QVariantMap({ { "zone", scene.zone }, { "fid", scene.fid } }));
    for (const Run &run : functions)
        runs.append(QVariantMap({ { "fid", run.fid }, { "playback", run.playback }, { "level", run.level } }));
    for (const List &list : cuelists)
        lists.append(QVariantMap({ { "fid", list.fid }, { "cueUid", list.cueUid }, { "paused", list.paused },
                                   { "zone", list.zone }, { "playback", list.playback }, { "level", list.level } }));
    return QVariantMap({ { "takenAt", takenAt }, { "reason", reason }, { "zoneScenes", scenes },
                         { "functions", runs }, { "cuelists", lists } });
}

/*********************************************************************
 * Taking it, bringing it back
 *********************************************************************/

LiveRecall::LiveRecall(DtcCore *core, QObject *parent)
    : QObject(parent)
    , m_core(core)
    , m_window(new QTimer(this))
{
    m_window->setSingleShot(true);
    connect(m_window, &QTimer::timeout, this, &LiveRecall::clear);
    // (the functions of the show that went are gone)
    connect(m_core->doc(), SIGNAL(cleared()), this, SLOT(slotDocCleared()));
}

LiveRecall::~LiveRecall()
{
    m_window->stop();
}

int LiveRecall::msLeft() const
{
    if (m_available == false)
        return 0;
    return int(qMax(qint64(0), qint64(m_windowMs) - m_since.elapsed()));
}

QString LiveRecall::describe(int scenes, int cuelists)
{
    const QString s = tr("%n scene(s)", "", scenes);
    const QString c = tr("%n cuelist(s)", "", cuelists);
    if (scenes > 0 && cuelists > 0)
        return tr("%1 and %2").arg(s, c);
    return cuelists > 0 ? c : s;
}

QString LiveRecall::summary() const
{
    if (m_available == false)
        return QString();
    return describe(int(m_snapshot.zoneScenes.count() + m_snapshot.functions.count()),
                    int(m_snapshot.cuelists.count()));
}

void LiveRecall::take(const QString &reason, bool zonesOnly)
{
    Doc *doc = m_core->doc();
    ZoneManager *zones = m_core->zones();
    PlaybackManager *playbacks = m_core->playbacks();

    // what a flash holds plays only while it is held (the SHOW, the phone,
    // a pad, Live); what a fader of the SHOW holds up is never stopped
    QSet<quint32> flashes = m_core->surface()->flashScenes();
    flashes.unite(m_core->mapping()->flashScenes());
    for (int i = 0; i < zones->count(); i++)
        flashes.unite(zones->zone(i)->pressedFlashes());
    const QSet<quint32> held = m_core->surface()->heldScenes();

    Snapshot shot;
    shot.takenAt = QDateTime::currentMSecsSinceEpoch();
    shot.reason = reason;
    QSet<quint32> seen;

    // the groups: what each one plays
    for (int i = 0; i < zones->count(); i++)
    {
        Zone *zone = zones->zone(i);
        for (quint32 fid : zone->activeFunctions())
        {
            Function *f = doc->function(fid);
            if (f == nullptr || seen.contains(fid) || held.contains(fid) || zone->isHeld(fid) || flashes.contains(fid))
                continue;
            if (Cuelist *cl = qobject_cast<Cuelist *>(f))
                shot.cuelists.append({ fid, cl->status().currentUid, cl->isPaused(), int(zone->id()), false, 1.0 });
            else
                shot.zoneScenes.append({ zone->id(), fid });
            seen.insert(fid);
        }
    }

    // the rest that plays on its own: a button, a playback, the editors (not
    // what plays inside another one: its steps, its tracks come with it)
    if (zonesOnly == false)
    {
        for (Function *f : doc->functions())
        {
            const quint32 fid = f->id();
            if (seen.contains(fid) || f->stopped() || f->isVisible() == false)
                continue;
            const bool playback = f->isStartedBy(PlaybackManager::playbackParent());
            // (what Stop all stops: the same rule)
            const bool own = DtcCore::playsOnItsOwn(f);
            if (own == false || held.contains(fid) || flashes.contains(fid) || playbacks->startedByFlash(fid))
                continue;
            const qreal level = playback ? playbacks->functionLevel(fid) : 1.0;
            if (Cuelist *cl = qobject_cast<Cuelist *>(f))
                shot.cuelists.append({ fid, cl->status().currentUid, cl->isPaused(), -1, playback, level });
            else
                shot.functions.append({ fid, playback, level });
            seen.insert(fid);
        }
    }

    // nothing played: what was taken before stays (in its window)
    if (shot.isEmpty())
        return;
    if (m_available)
        m_snapshot.merge(shot);
    else
        m_snapshot = shot;
    m_available = true;
    m_serial++;
    m_since.start();
    m_window->start(m_windowMs);
    emit taken();
    emit availableChanged();
}

bool LiveRecall::recall()
{
    if (m_available == false)
        return false;
    // once
    const Snapshot shot = m_snapshot;
    clear();

    // what comes back never fires a protected special effect: armed («Release
    // groups» doesn't disarm; armed again after Stop all), they are disarmed
    // first, and out on every universe before anything starts again. A scene
    // that comes back with a CO2 then waits for a release (its effect stays
    // at 0), also once they are armed again
    if (SafetyManager *safety = m_core->safety())
    {
        if (safety->armed())
        {
            safety->disarm(QStringLiteral("recall"));
            safety->flush();
        }
    }

    Doc *doc = m_core->doc();
    ZoneManager *zones = m_core->zones();
    PlaybackManager *playbacks = m_core->playbacks();
    MasterTimer *timer = doc->masterTimer();
    // (a function in beats would read the time as beats: its own fade)
    auto fadeOf = [](const Function *f) {
        return f->tempoType() == Function::Beats ? Function::defaultSpeed() : kRecallFadeMs;
    };
    int scenes = 0;
    int lists = 0;

    // (what plays again by now, or is gone, is left as it is)
    for (const Snapshot::ZoneScene &scene : shot.zoneScenes)
    {
        Zone *zone = zones->zoneById(scene.zone);
        Function *f = doc->function(scene.fid);
        if (zone == nullptr || f == nullptr || zone->isActive(scene.fid) || zone->functionIDs().contains(scene.fid) == false)
            continue;
        zone->recall(scene.fid, kRecallFadeMs);
        scenes++;
    }
    for (const Snapshot::Run &run : shot.functions)
    {
        Function *f = doc->function(run.fid);
        if (f == nullptr || f->stopped() == false)
            continue;
        if (run.playback)
        {
            if (playbacks->recall(run.fid, -1, kRecallFadeMs))
                scenes++;
            continue;
        }
        f->start(timer, FunctionParent::master(), 0, fadeOf(f));
        scenes++;
    }
    for (const Snapshot::List &list : shot.cuelists)
    {
        Cuelist *cl = qobject_cast<Cuelist *>(doc->function(list.fid));
        if (cl == nullptr || cl->stopped() == false)
            continue;
        // on the cue it played (one deleted since: where it starts)
        const int index = cl->indexOfUid(list.cueUid);
        if (list.zone >= 0)
        {
            Zone *zone = zones->zoneById(quint32(list.zone));
            if (zone == nullptr || zone->functionIDs().contains(list.fid) == false)
                continue;
            if (index >= 0)
                cl->setStartCue(index);
            zone->recall(list.fid, kRecallFadeMs);
        }
        else if (list.playback)
        {
            if (playbacks->recall(list.fid, index, kRecallFadeMs) == false)
                continue;
        }
        else
        {
            if (index >= 0)
                cl->setStartCue(index);
            cl->start(timer, FunctionParent::master(), 0, fadeOf(cl));
        }
        lists++;
        // paused, it is paused again once it has faded in (paused at once,
        // it would stay dark)
        if (list.paused)
        {
            QPointer<Cuelist> paused(cl);
            QTimer::singleShot(int(kRecallFadeMs + 2 * MasterTimer::tick()), this, [paused]() {
                if (paused != nullptr && paused->isRunning() && paused->stopped() == false && paused->isPaused() == false)
                    paused->setPause(true);
            });
        }
    }

    if (scenes + lists > 0)
        emit message(tr("Brought back: %1").arg(describe(scenes, lists)));
    emit recalled();
    return true;
}

void LiveRecall::clear()
{
    m_window->stop();
    if (m_available == false && m_snapshot.isEmpty())
        return;
    m_snapshot = Snapshot();
    const bool was = m_available;
    m_available = false;
    if (was)
        emit availableChanged();
}

void LiveRecall::slotDocCleared()
{
    clear();
}
