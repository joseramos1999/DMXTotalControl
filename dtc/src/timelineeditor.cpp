/*
  DMX Total Control
  timelineeditor.cpp

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

#include <QFileInfo>
#include <QThread>
#include <QPointer>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QDebug>
#include <qmath.h>

#include "timelineeditor.h"
#include "programmer.h"

#include "audioplugincache.h"
#include "audioparameters.h"
#include "audiodecoder.h"
#include "showfunction.h"
#include "mastertimer.h"
#include "track.h"
#include "scene.h"
#include "audio.h"
#include "show.h"
#include "doc.h"

/* A block of a static scene, when added */
#define DEFAULT_BLOCK_MS 10000
/* The steps of the history of a super scene */
#define HISTORY_STEPS 100
/* Changes of one field closer than this are one step (ms) */
#define MERGE_MS 1000

TimelineEditor::TimelineEditor(Doc *doc, Programmer *programmer, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_programmer(programmer)
    , m_revision(0)
    , m_playingId(-1)
    , m_position(0)
    , m_historyRevision(0)
    , m_editDepth(0)
    , m_recording(false)
    , m_editShow(Function::invalidId())
    , m_pendingChanged(false)
{
    m_clock.start();
    // (a show cleared: once, when it is, not for each of its functions)
    connect(m_doc, SIGNAL(functionRemoved(quint32)), this, SLOT(slotFunctionRemoved(quint32)));
    connect(m_doc, SIGNAL(cleared()), this, SLOT(slotCleared()));
}

TimelineEditor::~TimelineEditor()
{
}

void TimelineEditor::setHeldByFader(const std::function<bool(quint32)> &held)
{
    m_heldByFader = held;
}

void TimelineEditor::slotFunctionRemoved(quint32 fid)
{
    if (m_doc->isClearing())
        return;

    // a super scene deleted: its history goes. Any other function: the
    // blocks of it in the histories, which DtcCore::deleteFunction took out
    // of the super scenes (its id may be given to a new function)
    const bool forgotten = m_history.remove(fid) || forgetFunction(fid);
    forgetClips(fid);
    if (m_editDepth > 0)
        m_pendingChanged = true;
    else
        emit changed();
    if (forgotten)
    {
        m_historyRevision++;
        emit historyChanged();
    }
}

void TimelineEditor::slotCleared()
{
    // another show, or a new one: the histories and the clipboard go
    m_history.clear();
    m_historyRevision++;
    emit changed();
    emit historyChanged();
    if (m_clipboard.isEmpty() == false)
    {
        m_clipboard.clear();
        emit clipboardChanged();
    }
}

int TimelineEditor::revision() const
{
    return m_revision;
}

int TimelineEditor::historyRevision() const
{
    return m_historyRevision;
}

int TimelineEditor::playingId() const
{
    return m_playingId;
}

int TimelineEditor::position() const
{
    return m_position;
}

int TimelineEditor::clipboardCount() const
{
    return int(m_clipboard.count());
}

Show *TimelineEditor::showOf(quint32 id) const
{
    return qobject_cast<Show *>(m_doc->function(id));
}

void TimelineEditor::touch(Show *show)
{
    m_revision++;
    m_doc->setModified();
    // in an edit: once, at its end (a group of blocks rebuilds the view once)
    if (m_editDepth > 0)
    {
        m_pendingShows.insert(show->id());
        return;
    }
    emit show->changed(show->id());
    emit changed();
}

quint32 TimelineEditor::cycleOf(quint32 functionId) const
{
    Function *f = m_doc->function(functionId);
    if (f == nullptr || f->type() == Function::SceneType)
        return 0;
    quint32 cycle = f->totalDuration();
    return cycle == Function::infiniteSpeed() ? 0 : cycle;
}

/*********************************************************************
 * Reading
 *********************************************************************/

static QVariantList pointsToList(const QList<Track::AutomationPoint> &points)
{
    QVariantList list;
    for (const Track::AutomationPoint &p : points)
        list.append(QVariantMap({ { "time", p.time }, { "value", p.value }, { "smooth", p.smooth } }));
    return list;
}

QVariantMap TimelineEditor::show(quint32 id) const
{
    QVariantMap info;
    Show *show = showOf(id);
    if (show == nullptr)
        return info;

    QVariantList tracks;
    quint32 length = 0;
    for (Track *track : show->tracks())
    {
        QVariantList blocks;
        for (ShowFunction *sf : track->showFunctions())
        {
            Function *f = m_doc->function(sf->functionID());
            QVariantMap block;
            block.insert("id", sf->id());
            block.insert("functionId", sf->functionID());
            block.insert("name", f != nullptr ? f->name() : tr("(missing)"));
            block.insert("type", f != nullptr ? Function::typeToString(f->type()) : QString());
            block.insert("start", sf->startTime());
            block.insert("duration", sf->duration(m_doc));
            block.insert("fadeIn", sf->fadeIn());
            block.insert("fadeOut", sf->fadeOut());
            block.insert("stretch", sf->stretch());
            block.insert("locked", sf->isLocked());
            block.insert("color", sf->color().isValid() ? sf->color().name() : QString());
            block.insert("cycle", cycleOf(sf->functionID()));
            blocks.append(block);
            length = qMax(length, sf->startTime() + sf->duration(m_doc));
        }

        QVariantMap t;
        t.insert("id", track->id());
        t.insert("name", track->name());
        t.insert("mute", track->isMute());
        t.insert("locked", track->isLocked());
        t.insert("hidden", track->isHidden());
        t.insert("dimmer", pointsToList(track->automation("dimmer")));
        t.insert("phase", pointsToList(track->automation("phase")));
        t.insert("blocks", blocks);
        tracks.append(t);
    }

    Scene *values = qobject_cast<Scene *>(m_doc->function(show->staticScene()));

    info.insert("id", id);
    info.insert("name", show->name());
    info.insert("length", show->endTime() > 0 ? show->endTime() : length);
    info.insert("contentLength", length);
    info.insert("endTime", show->endTime());
    info.insert("loop", show->loop());
    info.insert("grid", show->timeDivisionType() == Show::Time ? "time" : "bars");
    info.insert("bpm", show->timeDivisionBPM());
    info.insert("beatsPerBar", show->timeDivisionType() == Show::Time ? 4 : show->beatsDivision());
    info.insert("gridOffset", show->gridOffset());
    info.insert("staticValues", values != nullptr ? values->values().count() : 0);
    info.insert("tracks", tracks);
    return info;
}

/*********************************************************************
 * Tracks
 *********************************************************************/

int TimelineEditor::addTrack(quint32 showId, const QString &name)
{
    Show *show = showOf(showId);
    if (show == nullptr)
        return -1;

    Edit edit(this, show, tr("Add a track"));
    Track *track = appendTrack(show, name);
    touch(show);
    return int(track->id());
}

Track *TimelineEditor::appendTrack(Show *show, const QString &name)
{
    Track *track = new Track(Function::invalidId(), show);
    track->setName(name.trimmed().isEmpty() ? tr("Track %1").arg(show->tracks().count() + 1) : name.trimmed());
    show->addTrack(track);
    return track;
}

void TimelineEditor::removeTrack(quint32 showId, int trackId)
{
    Show *show = showOf(showId);
    if (show == nullptr || show->track(quint32(trackId)) == nullptr)
        return;

    // a playing show (whoever started it) forgets the track and its blocks
    // first, and plays on (Show::removeTrack)
    Edit edit(this, show, tr("Delete a track"));
    show->removeTrack(quint32(trackId));
    touch(show);
}

void TimelineEditor::setTrack(quint32 showId, int trackId, const QString &key, const QVariant &value)
{
    Show *show = showOf(showId);
    Track *track = show != nullptr ? show->track(quint32(trackId)) : nullptr;
    if (track == nullptr)
        return;

    // folding a track is not a step of the history
    if (key == "hidden")
    {
        track->setHidden(value.toBool());
        touch(show);
        return;
    }

    Edit edit(this, show, tr("Change a track"));
    if (key == "name" && value.toString().trimmed().isEmpty() == false)
        track->setName(value.toString().trimmed());
    else if (key == "mute")
        track->setMute(value.toBool());
    else if (key == "locked")
        track->setLocked(value.toBool());
    else
        return;
    touch(show);
}

void TimelineEditor::moveTrack(quint32 showId, int trackId, int direction)
{
    Show *show = showOf(showId);
    Track *track = show != nullptr ? show->track(quint32(trackId)) : nullptr;
    if (track == nullptr)
        return;
    Edit edit(this, show, tr("Move a track"));
    show->moveTrack(track, direction < 0 ? -1 : 1);
    touch(show);
}

/*********************************************************************
 * Blocks
 *********************************************************************/

int TimelineEditor::addBlock(quint32 showId, int trackId, quint32 functionId, int start)
{
    Show *show = showOf(showId);
    Track *track = show != nullptr ? show->track(quint32(trackId)) : nullptr;
    Function *f = m_doc->function(functionId);
    if (track == nullptr || track->isLocked() || canHold(show, f, true) == false)
        return -1;

    Edit edit(this, show, tr("Add a block"));
    ShowFunction *sf = track->createShowFunction(functionId);
    sf->setStartTime(quint32(qMax(0, start)));
    quint32 cycle = f->type() == Function::AudioType ? f->totalDuration() : cycleOf(functionId);
    sf->setDuration(cycle > 0 ? cycle : DEFAULT_BLOCK_MS);
    touch(show);
    return int(sf->id());
}

bool TimelineEditor::canHold(Show *show, Function *f, bool tell)
{
    if (f == nullptr || f->id() == show->id() || f->type() == Function::ShowType)
        return false;
    // a rack (or steps) that plays this super scene: it would play itself
    if (f->contains(show->id()))
    {
        if (tell)
            emit message(tr("“%1” plays this super scene: it can't go into it").arg(f->name()));
        return false;
    }
    return true;
}

int TimelineEditor::addAudio(quint32 showId, int trackId, const QUrl &file, int start)
{
    Show *show = showOf(showId);
    if (show == nullptr)
        return -1;

    // a track to put it on first: the audio is a function of the show, it
    // would stay there without its block
    Track *track = show->track(quint32(trackId));
    if (track == nullptr || track->isLocked())
    {
        emit message(track == nullptr ? tr("Add a track for the audio first")
                                      : tr("The track is locked: unlock it to add the audio"));
        return -1;
    }

    // the audio and its block: one step
    Edit edit(this, show, tr("Add audio"));
    QString path = file.isLocalFile() ? file.toLocalFile() : file.toString();
    Audio *audio = createOwnedAudio(path, QFileInfo(path).completeBaseName(), Function::invalidId(), true);
    if (audio == nullptr)
    {
        emit message(tr("Unable to read the audio of %1").arg(QFileInfo(path).fileName()));
        return -1;
    }
    const int block = addBlock(showId, trackId, audio->id(), start);
    if (block < 0)
        m_doc->deleteFunction(audio->id());
    return block;
}

void TimelineEditor::moveBlock(quint32 showId, int blockId, int trackId, int start, bool merge)
{
    Show *show = showOf(showId);
    ShowFunction *sf = show != nullptr ? show->showFunction(quint32(blockId)) : nullptr;
    Track *from = show != nullptr ? show->getTrackFromShowFunctionID(quint32(blockId)) : nullptr;
    Track *to = show != nullptr ? show->track(quint32(trackId)) : nullptr;
    if (sf == nullptr || from == nullptr || sf->isLocked() || from->isLocked())
        return;

    Edit edit(this, show, tr("Move %n block(s)", nullptr, 1), merge ? QString("start:%1").arg(blockId) : QString());
    moveOne(sf, from, to, quint32(qMax(0, start)));
    touch(show);
}

void TimelineEditor::moveOne(ShowFunction *sf, Track *from, Track *to, quint32 start)
{
    sf->setStartTime(start);
    if (to != nullptr && to != from && to->isLocked() == false)
    {
        from->removeShowFunction(sf, false);
        to->addShowFunction(sf);
    }
}

void TimelineEditor::resizeBlock(quint32 showId, int blockId, int start, int duration, bool merge)
{
    Show *show = showOf(showId);
    ShowFunction *sf = show != nullptr ? show->showFunction(quint32(blockId)) : nullptr;
    Track *track = show != nullptr ? show->getTrackFromShowFunctionID(quint32(blockId)) : nullptr;
    if (sf == nullptr || sf->isLocked() || (track != nullptr && track->isLocked()))
        return;

    Edit edit(this, show, tr("Resize a block"), merge ? QString("length:%1").arg(blockId) : QString());
    sf->setStartTime(quint32(qMax(0, start)));
    sf->setDuration(quint32(qMax(100, duration)));
    // the fades fit in the block
    sf->setFadeIn(qMin(sf->fadeIn(), sf->duration()));
    sf->setFadeOut(qMin(sf->fadeOut(), sf->duration() - sf->fadeIn()));
    touch(show);
}

void TimelineEditor::setBlock(quint32 showId, int blockId, const QString &key, const QVariant &value, bool merge)
{
    Show *show = showOf(showId);
    ShowFunction *sf = show != nullptr ? show->showFunction(quint32(blockId)) : nullptr;
    if (sf == nullptr)
        return;

    Edit edit(this, show, tr("Change %n block(s)", nullptr, 1), merge ? QString("%1:%2").arg(key).arg(blockId) : QString());
    if (setOne(sf, key, value))
        touch(show);
}

bool TimelineEditor::setOne(ShowFunction *sf, const QString &key, const QVariant &value)
{
    const quint32 duration = sf->duration(m_doc);
    if (key == "fadeIn")
        sf->setFadeIn(quint32(qBound(0, value.toInt(), int(duration))));
    else if (key == "fadeOut")
        sf->setFadeOut(quint32(qBound(0, value.toInt(), int(duration))));
    else if (key == "stretch")
        sf->setStretch(value.toBool());
    else if (key == "locked")
        sf->setLocked(value.toBool());
    else if (key == "color")
        sf->setColor(QColor(value.toString()));
    else
        return false;
    return true;
}

void TimelineEditor::removeBlock(quint32 showId, int blockId)
{
    removeBlocks(showId, { blockId });
}

int TimelineEditor::duplicateBlock(quint32 showId, int blockId)
{
    const QVariantList copies = duplicateBlocks(showId, { blockId });
    return copies.isEmpty() ? -1 : copies.first().toInt();
}

/*********************************************************************
 * Several blocks
 *********************************************************************/

QList<ShowFunction *> TimelineEditor::blocksOf(Show *show, const QVariantList &ids) const
{
    QList<ShowFunction *> blocks;
    for (const QVariant &v : ids)
    {
        bool ok = false;
        const int id = v.toInt(&ok);
        ShowFunction *sf = ok && id >= 0 ? show->showFunction(quint32(id)) : nullptr;
        if (sf != nullptr && blocks.contains(sf) == false)
            blocks.append(sf);
    }
    return blocks;
}

void TimelineEditor::moveBlocks(quint32 showId, const QVariantList &blockIds, int deltaMs, int trackOffset)
{
    Show *show = showOf(showId);
    if (show == nullptr)
        return;

    // the blocks that move, and the row of their tracks (as shown)
    const QList<Track *> order = show->tracks();
    QList<QPair<ShowFunction *, int>> moves;
    quint32 earliest = UINT_MAX;
    for (ShowFunction *sf : blocksOf(show, blockIds))
    {
        Track *track = show->getTrackFromShowFunctionID(sf->id());
        if (track == nullptr || track->isLocked() || sf->isLocked())
            continue;
        moves.append({ sf, int(order.indexOf(track)) });
        earliest = qMin(earliest, sf->startTime());
    }
    if (moves.isEmpty())
        return;

    // the earliest one stops at 0. All of them onto other tracks, or none
    const int delta = qMax(deltaMs, -int(earliest));
    bool across = trackOffset != 0;
    for (const auto &move : std::as_const(moves))
    {
        const int row = move.second + trackOffset;
        if (row < 0 || row >= order.count() || order.at(row)->isLocked())
            across = false;
    }
    const int offset = across ? trackOffset : 0;
    if (delta == 0 && offset == 0)
        return;

    Edit edit(this, show, tr("Move %n block(s)", nullptr, int(moves.count())));
    {
        // a running show sees the whole group moved, never half of it
        QMutexLocker locker(show->runnerMutex());
        for (const auto &move : std::as_const(moves))
            moveOne(move.first, order.at(move.second), order.at(move.second + offset),
                    quint32(int(move.first->startTime()) + delta));
    }
    touch(show);
}

void TimelineEditor::removeBlocks(quint32 showId, const QVariantList &blockIds)
{
    Show *show = showOf(showId);
    if (show == nullptr)
        return;

    QList<QPair<Track *, ShowFunction *>> gone;
    for (ShowFunction *sf : blocksOf(show, blockIds))
    {
        Track *track = show->getTrackFromShowFunctionID(sf->id());
        if (track != nullptr && track->isLocked() == false)
            gone.append({ track, sf });
    }
    if (gone.isEmpty())
        return;

    // a playing show (whoever started it) forgets the blocks first, all at
    // once, and plays on (Track::removeShowFunction)
    Edit edit(this, show, tr("Delete %n block(s)", nullptr, int(gone.count())));
    QList<quint32> functions;
    {
        QMutexLocker locker(show->runnerMutex());
        for (const auto &block : std::as_const(gone))
        {
            if (functions.contains(block.second->functionID()) == false)
                functions.append(block.second->functionID());
            block.first->removeShowFunction(block.second, true);
        }
    }
    // the audio of the super scene goes with its last block: Doc::
    // deleteFunction stops it first (never with the lock held: the Doc waits
    // for the timer to let a function go, and the timer takes it)
    for (quint32 fid : std::as_const(functions))
    {
        if (show->contains(fid) == false && ownedAudio(fid, show))
            m_doc->deleteFunction(fid);
    }
    touch(show);
}

QVariantList TimelineEditor::duplicateBlocks(quint32 showId, const QVariantList &blockIds)
{
    QVariantList copies;
    Show *show = showOf(showId);
    if (show == nullptr)
        return copies;

    // the blocks on tracks that aren't locked, and where the group begins
    // and ends
    QList<QPair<Track *, ShowFunction *>> blocks;
    quint32 first = UINT_MAX;
    quint32 last = 0;
    for (ShowFunction *sf : blocksOf(show, blockIds))
    {
        Track *track = show->getTrackFromShowFunctionID(sf->id());
        if (track == nullptr || track->isLocked())
            continue;
        blocks.append({ track, sf });
        first = qMin(first, sf->startTime());
        last = qMax(last, sf->startTime() + sf->duration(m_doc));
    }
    if (blocks.isEmpty())
        return copies;

    // the group again, right after itself: each copy on the track of its
    // block, as far after it as the group is long. They share the function
    Edit edit(this, show, tr("Duplicate %n block(s)", nullptr, int(blocks.count())));
    const quint32 span = last - first;
    for (const auto &block : std::as_const(blocks))
    {
        ShowFunction *sf = block.second;
        ShowFunction *copy = block.first->createShowFunction(sf->functionID());
        copy->setStartTime(sf->startTime() + span);
        copy->setDuration(sf->duration(m_doc));
        copy->setColor(sf->color());
        copy->setFadeIn(sf->fadeIn());
        copy->setFadeOut(sf->fadeOut());
        copy->setStretch(sf->stretch());
        copies.append(int(copy->id()));
    }
    touch(show);
    return copies;
}

void TimelineEditor::setBlocks(quint32 showId, const QVariantList &blockIds, const QString &key, const QVariant &value)
{
    Show *show = showOf(showId);
    if (show == nullptr)
        return;
    const QList<ShowFunction *> blocks = blocksOf(show, blockIds);
    if (blocks.isEmpty())
        return;

    Edit edit(this, show, tr("Change %n block(s)", nullptr, int(blocks.count())));
    bool known = false;
    for (ShowFunction *sf : blocks)
        known = setOne(sf, key, value) || known;
    if (known)
        touch(show);
}

/*********************************************************************
 * Copy and paste
 *********************************************************************/

int TimelineEditor::copyBlocks(quint32 showId, const QVariantList &blockIds)
{
    Show *show = showOf(showId);
    if (show == nullptr)
        return 0;

    // each block with the row of its track (as shown), from the topmost one,
    // and its start, from the earliest one
    const QList<Track *> order = show->tracks();
    QList<ClipBlock> clips;
    int top = INT_MAX;
    quint32 earliest = UINT_MAX;
    for (ShowFunction *sf : blocksOf(show, blockIds))
    {
        Track *track = show->getTrackFromShowFunctionID(sf->id());
        if (track == nullptr)
            continue;
        ClipBlock clip;
        clip.row = int(order.indexOf(track));
        clip.offset = 0;
        clip.block = blockState(sf);
        clip.block.duration = sf->duration(m_doc);
        clips.append(clip);
        top = qMin(top, clip.row);
        earliest = qMin(earliest, sf->startTime());
    }
    if (clips.isEmpty())
        return 0;

    for (ClipBlock &clip : clips)
    {
        clip.row -= top;
        clip.offset = clip.block.start - earliest;
    }
    m_clipboard = clips;
    emit clipboardChanged();
    return int(clips.count());
}

int TimelineEditor::cutBlocks(quint32 showId, const QVariantList &blockIds)
{
    Show *show = showOf(showId);
    const int copied = copyBlocks(showId, blockIds);
    if (copied == 0)
        return 0;

    // (the blocks on locked tracks are only copied)
    int gone = 0;
    for (ShowFunction *sf : blocksOf(show, blockIds))
    {
        Track *track = show->getTrackFromShowFunctionID(sf->id());
        if (track != nullptr && track->isLocked() == false)
            gone++;
    }
    if (gone > 0)
    {
        Edit edit(this, show, tr("Cut %n block(s)", nullptr, gone));
        removeBlocks(showId, blockIds);
    }
    return copied;
}

QVariantList TimelineEditor::pasteBlocks(quint32 showId, int trackId, int start)
{
    QVariantList pasted;
    Show *show = showOf(showId);
    if (show == nullptr || m_clipboard.isEmpty())
        return pasted;

    // the topmost one on the track given (or the first one), the earliest one
    // at $start; the others as far from them as they were
    const int base = qMax(0, int(show->tracks().indexOf(show->track(quint32(trackId)))));
    const quint32 from = quint32(qMax(0, start));
    QHash<quint32, quint32> made;
    QSet<quint32> told;
    int locked = 0;

    Edit edit(this, show, QString());
    for (const ClipBlock &clip : std::as_const(m_clipboard))
    {
        const int row = base + clip.row;
        if (row < show->tracks().count() && show->tracks().at(row)->isLocked())
        {
            locked++;
            continue;
        }
        const quint32 fid = pastedFunction(show, clip.block, made, told);
        if (fid == Function::invalidId())
            continue;

        // the tracks missing, at the end
        while (row >= show->tracks().count())
            appendTrack(show, QString());
        Track *track = show->tracks().at(row);
        if (track->isHidden())
            track->setHidden(false);

        // as Duplicate: its settings, not locked
        ShowFunction *sf = track->createShowFunction(fid);
        sf->setStartTime(from + clip.offset);
        sf->setDuration(clip.block.duration);
        sf->setFadeIn(clip.block.fadeIn);
        sf->setFadeOut(clip.block.fadeOut);
        sf->setStretch(clip.block.stretch);
        sf->setColor(clip.block.color);
        pasted.append(int(sf->id()));
    }
    edit.setText(tr("Paste %n block(s)", nullptr, int(pasted.count())));

    if (locked > 0)
        emit message(tr("%n block(s) not pasted: their track is locked", nullptr, locked));
    if (pasted.isEmpty() == false)
        touch(show);
    return pasted;
}

quint32 TimelineEditor::pastedFunction(Show *show, const BlockState &block, QHash<quint32, quint32> &made,
                                       QSet<quint32> &told)
{
    // a scene (or a rack, an audio of the list...): the same one
    if (block.audioFile.isEmpty())
    {
        Function *f = m_doc->function(block.functionId);
        const bool tell = told.contains(block.functionId) == false;
        told.insert(block.functionId);
        return canHold(show, f, tell) ? block.functionId : Function::invalidId();
    }

    // the audio of a super scene: the same one in it (or one of the same file
    // it has, when that one is gone: cut), another one from its file
    // anywhere else, so that deleting it there never takes the first one.
    // Once for all its blocks
    if (made.contains(block.functionId))
        return made.value(block.functionId);
    auto ownAudio = [this, show, &block](quint32 fid) {
        return ownedAudio(fid, show, block.audioFile) && show->contains(fid);
    };
    quint32 fid = ownAudio(block.functionId) ? block.functionId : Function::invalidId();
    const QList<Function *> audios = m_doc->functionsByType(Function::AudioType);
    for (int i = 0; i < audios.count() && fid == Function::invalidId(); i++)
    {
        if (ownAudio(audios.at(i)->id()))
            fid = audios.at(i)->id();
    }
    if (fid == Function::invalidId())
    {
        Audio *copy = createOwnedAudio(block.audioFile, block.audioName, Function::invalidId(), true);
        if (copy != nullptr)
            fid = copy->id();
        else
            emit message(tr("Unable to read the audio of %1").arg(QFileInfo(block.audioFile).fileName()));
    }
    made.insert(block.functionId, fid);
    return fid;
}

void TimelineEditor::forgetClips(quint32 fid)
{
    // (its id may be given to a new function)
    bool forgotten = false;
    for (int i = int(m_clipboard.count()) - 1; i >= 0; i--)
    {
        if (m_clipboard.at(i).block.functionId == fid && m_clipboard.at(i).block.audioFile.isEmpty())
        {
            m_clipboard.removeAt(i);
            forgotten = true;
        }
    }
    if (forgotten == false)
        return;

    // the ones left: still from the topmost and the earliest of them
    int top = INT_MAX;
    quint32 earliest = UINT_MAX;
    for (const ClipBlock &clip : std::as_const(m_clipboard))
    {
        top = qMin(top, clip.row);
        earliest = qMin(earliest, clip.offset);
    }
    for (ClipBlock &clip : m_clipboard)
    {
        clip.row -= top;
        clip.offset -= earliest;
    }
    emit clipboardChanged();
}

/*********************************************************************
 * Automation and settings
 *********************************************************************/

void TimelineEditor::setAutomation(quint32 showId, int trackId, const QString &lane, const QVariantList &points)
{
    Show *show = showOf(showId);
    Track *track = show != nullptr ? show->track(quint32(trackId)) : nullptr;
    if (track == nullptr || (lane != "dimmer" && lane != "phase"))
        return;

    QList<Track::AutomationPoint> list;
    for (const QVariant &v : points)
    {
        QVariantMap p = v.toMap();
        list.append({ quint32(qMax(0, p.value("time").toInt())), qBound(0.0, p.value("value").toDouble(), 1.0),
                      p.value("smooth").toBool() });
    }
    Edit edit(this, show, tr("Change the automation"));
    track->setAutomation(lane, list);
    touch(show);
}

void TimelineEditor::setSettings(quint32 showId, const QVariantMap &settings, bool merge)
{
    Show *show = showOf(showId);
    if (show == nullptr)
        return;

    Edit edit(this, show, tr("Change the timeline"), merge ? QStringList(settings.keys()).join(",") : QString());
    if (settings.contains("endTime"))
        show->setEndTime(quint32(qMax(0, settings.value("endTime").toInt())));
    if (settings.contains("loop"))
        show->setLoop(settings.value("loop").toBool());
    if (settings.contains("gridOffset"))
        show->setGridOffset(quint32(qMax(0, settings.value("gridOffset").toInt())));

    QString grid = settings.value("grid", show->timeDivisionType() == Show::Time ? "time" : "bars").toString();
    int bpm = qBound(20, settings.value("bpm", show->timeDivisionBPM()).toInt(), 300);
    int beats = settings.value("beatsPerBar", show->timeDivisionType() == Show::Time ? 4 : show->beatsDivision()).toInt();
    Show::TimeDivision division = grid == "time" ? Show::Time
                                : beats == 3 ? Show::BPM_3_4 : beats == 2 ? Show::BPM_2_4 : Show::BPM_4_4;
    show->setTimeDivision(division, bpm);
    touch(show);
}

int TimelineEditor::captureStatic(quint32 showId)
{
    Show *show = showOf(showId);
    if (show == nullptr)
        return 0;

    QList<SceneValue> values = m_programmer->sceneValues();
    if (values.isEmpty())
    {
        emit message(tr("Set the static values in the programmer first"));
        return 0;
    }

    Edit edit(this, show, tr("Take the static values"));
    if (setStaticValues(show, values) == false)
        return 0;
    touch(show);
    return values.count();
}

Scene *TimelineEditor::staticScene(Show *show, bool create, quint32 preferredId)
{
    Scene *scene = qobject_cast<Scene *>(m_doc->function(show->staticScene()));
    if (scene != nullptr || create == false)
        return scene;

    // a hidden scene of the super scene holds them
    scene = new Scene(m_doc);
    scene->setVisible(false);
    const bool free = preferredId != Function::invalidId() && m_doc->function(preferredId) == nullptr;
    if (m_doc->addFunction(scene, free ? preferredId : Function::invalidId()) == false)
    {
        delete scene;
        return nullptr;
    }
    show->setStaticScene(scene->id());
    return scene;
}

bool TimelineEditor::setStaticValues(Show *show, const QList<SceneValue> &values, quint32 preferredId)
{
    Scene *scene = staticScene(show, true, preferredId);
    if (scene == nullptr)
        return false;
    scene->setName(tr("%1 (static values)").arg(show->name()));
    scene->clear();
    for (const SceneValue &scv : values)
    {
        if (scene->fixtures().contains(scv.fxi) == false)
            scene->addFixture(scv.fxi);
        scene->setValue(scv);
    }
    return true;
}

void TimelineEditor::clearStatic(quint32 showId)
{
    Show *show = showOf(showId);
    if (show == nullptr || show->staticScene() == Function::invalidId())
        return;

    // (playing with the show, Doc::deleteFunction stops it first)
    Edit edit(this, show, tr("Clear the static values"));
    quint32 scene = show->staticScene();
    show->setStaticScene(Function::invalidId());
    m_doc->deleteFunction(scene);
    touch(show);
}

/*********************************************************************
 * Undo
 *********************************************************************/

TimelineEditor::Edit::Edit(TimelineEditor *editor, Show *show, const QString &text, const QString &key, bool record)
    : m_editor(editor)
    , m_outermost(editor->m_editDepth == 0)
{
    // the outermost edit is the step: the edits it calls are part of it
    if (m_editor->m_editDepth++ > 0)
        return;
    m_editor->m_recording = record;
    m_editor->m_editShow = show->id();
    m_editor->m_editText = text;
    m_editor->m_editKey = key;
    if (record)
        m_editor->m_before = m_editor->capture(show);
}

TimelineEditor::Edit::~Edit()
{
    if (--m_editor->m_editDepth > 0)
        return;

    bool stepped = false;
    Show *show = m_editor->showOf(m_editor->m_editShow);
    if (m_editor->m_recording && show != nullptr)
        stepped = m_editor->record(show);
    m_editor->m_recording = false;
    m_editor->m_before = ShowState();
    // (what QML does when told may be an edit of its own)
    m_editor->notify();
    if (stepped)
    {
        m_editor->m_historyRevision++;
        emit m_editor->historyChanged();
    }
}

void TimelineEditor::Edit::setText(const QString &text)
{
    if (m_outermost)
        m_editor->m_editText = text;
}

TimelineEditor::ShowState TimelineEditor::capture(const Show *show) const
{
    ShowState state;
    for (Track *track : show->tracks())
    {
        TrackState t;
        t.id = track->id();
        t.sceneId = track->getSceneID();
        t.name = track->name();
        t.mute = track->isMute();
        t.locked = track->isLocked();
        t.hidden = track->isHidden();
        t.dimmer = track->automation("dimmer");
        t.phase = track->automation("phase");
        for (ShowFunction *sf : track->showFunctions())
            t.blocks.append(blockState(sf));
        state.tracks.append(t);
    }

    state.endTime = show->endTime();
    state.gridOffset = show->gridOffset();
    state.loop = show->loop();
    state.division = int(show->timeDivisionType());
    state.bpm = show->timeDivisionBPM();
    if (Scene *values = qobject_cast<Scene *>(m_doc->function(show->staticScene())))
    {
        state.hasStatic = true;
        state.staticId = values->id();
        state.staticValues = values->values();
    }
    return state;
}

TimelineEditor::BlockState TimelineEditor::blockState(const ShowFunction *sf) const
{
    BlockState b;
    b.id = sf->id();
    b.functionId = sf->functionID();
    b.start = sf->startTime();
    b.duration = sf->duration();
    b.fadeIn = sf->fadeIn();
    b.fadeOut = sf->fadeOut();
    b.stretch = sf->stretch();
    b.locked = sf->isLocked();
    b.color = sf->color();
    // the audio of the super scene: made again from its file
    Audio *audio = qobject_cast<Audio *>(m_doc->function(sf->functionID()));
    if (audio != nullptr && audio->isVisible() == false)
    {
        b.audioFile = audio->getSourceFileName();
        b.audioName = audio->name();
    }
    return b;
}

static bool samePoints(const QList<Track::AutomationPoint> &a, const QList<Track::AutomationPoint> &b)
{
    if (a.count() != b.count())
        return false;
    for (int i = 0; i < a.count(); i++)
    {
        if (a.at(i).time != b.at(i).time || a.at(i).value != b.at(i).value || a.at(i).smooth != b.at(i).smooth)
            return false;
    }
    return true;
}

static bool sameValues(const QList<SceneValue> &a, const QList<SceneValue> &b)
{
    // (SceneValue::operator== leaves the value out)
    if (a.count() != b.count())
        return false;
    for (int i = 0; i < a.count(); i++)
    {
        if (a.at(i).fxi != b.at(i).fxi || a.at(i).channel != b.at(i).channel || a.at(i).value != b.at(i).value)
            return false;
    }
    return true;
}

bool TimelineEditor::same(const ShowState &a, const ShowState &b)
{
    if (a.endTime != b.endTime || a.gridOffset != b.gridOffset || a.loop != b.loop || a.division != b.division ||
        a.bpm != b.bpm || a.hasStatic != b.hasStatic || sameValues(a.staticValues, b.staticValues) == false ||
        a.tracks.count() != b.tracks.count())
        return false;

    for (int i = 0; i < a.tracks.count(); i++)
    {
        const TrackState &ta = a.tracks.at(i);
        const TrackState &tb = b.tracks.at(i);
        // (folding a track is not a change)
        if (ta.id != tb.id || ta.sceneId != tb.sceneId || ta.name != tb.name || ta.mute != tb.mute ||
            ta.locked != tb.locked || samePoints(ta.dimmer, tb.dimmer) == false ||
            samePoints(ta.phase, tb.phase) == false || ta.blocks.count() != tb.blocks.count())
            return false;
        for (int j = 0; j < ta.blocks.count(); j++)
        {
            const BlockState &ba = ta.blocks.at(j);
            const BlockState &bb = tb.blocks.at(j);
            if (ba.id != bb.id || ba.functionId != bb.functionId || ba.start != bb.start ||
                ba.duration != bb.duration || ba.fadeIn != bb.fadeIn || ba.fadeOut != bb.fadeOut ||
                ba.stretch != bb.stretch || ba.locked != bb.locked || ba.color != bb.color ||
                ba.audioFile != bb.audioFile || ba.audioName != bb.audioName)
                return false;
        }
    }
    return true;
}

bool TimelineEditor::record(Show *show)
{
    const ShowState after = capture(show);
    // (a locked block, the same name...: no step)
    if (same(m_before, after))
        return false;

    History &history = m_history[show->id()];
    const qint64 now = m_clock.elapsed();
    if (m_editKey.isEmpty() == false && history.redo.isEmpty() && history.undo.isEmpty() == false &&
        history.undo.last().key == m_editKey && now - history.undo.last().at < MERGE_MS)
    {
        // the same field again (a spin box held down): part of the step it
        // began, which has the state before it all. Back where it began,
        // there is no step at all
        history.undo.last().at = now;
        if (same(history.undo.last().state, after))
            history.undo.removeLast();
    }
    else
    {
        history.undo.append({ m_before, m_editText, m_editKey, now });
    }
    history.redo.clear();
    while (history.undo.count() > HISTORY_STEPS)
        history.undo.removeFirst();
    return true;
}

QVariantMap TimelineEditor::history(quint32 showId) const
{
    const History history = m_history.value(showId);
    return { { "canUndo", history.undo.isEmpty() == false },
             { "canRedo", history.redo.isEmpty() == false },
             { "undoText", history.undo.isEmpty() ? QString() : history.undo.last().text },
             { "redoText", history.redo.isEmpty() ? QString() : history.redo.last().text } };
}

bool TimelineEditor::undo(quint32 showId)
{
    return travel(showId, true);
}

bool TimelineEditor::redo(quint32 showId)
{
    return travel(showId, false);
}

bool TimelineEditor::travel(quint32 showId, bool back)
{
    // (not from inside an edit)
    Show *show = showOf(showId);
    if (show == nullptr || m_editDepth > 0 || m_history.contains(showId) == false)
        return false;
    History &history = m_history[showId];
    QList<Step> &from = back ? history.undo : history.redo;
    if (from.isEmpty())
        return false;

    // the other way: the super scene as it is now (never merged)
    const Step step = from.takeLast();
    (back ? history.redo : history.undo).append({ capture(show), step.text, QString(), 0 });
    apply(show, step.state);
    m_historyRevision++;
    emit historyChanged();
    return true;
}

void TimelineEditor::apply(Show *show, ShowState state)
{
    // silent: no step, and the show is told once, at the end
    Edit edit(this, show, QString(), QString(), false);

    // the audio of the super scene now, deleted at the end if it is not
    // used any more
    QSet<quint32> audioBefore;
    for (Track *track : show->tracks())
    {
        for (ShowFunction *sf : track->showFunctions())
            if (ownedAudio(sf->functionID(), show))
                audioBefore.insert(sf->functionID());
    }

    // (a) the audio of the blocks, made again from its file where it is
    // gone: before the lock (the Doc tells everyone). Its id may be taken:
    // the steps of the history get the new one
    struct Remap { quint32 from; quint32 to; QString file; };
    QList<Remap> remaps;
    QSet<quint32> audioAfter;
    for (const TrackState &t : std::as_const(state.tracks))
    {
        for (const BlockState &b : t.blocks)
        {
            if (b.audioFile.isEmpty() || audioAfter.contains(b.functionId))
                continue;
            audioAfter.insert(b.functionId);
            if (ownedAudio(b.functionId, show, b.audioFile))
                continue;
            Audio *audio = createOwnedAudio(b.audioFile, b.audioName, b.functionId, false);
            if (audio == nullptr || audio->id() != b.functionId)
                remaps.append({ b.functionId, audio != nullptr ? audio->id() : Function::invalidId(), b.audioFile });
        }
    }
    auto remap = [&remaps](ShowState &s, bool lost)
    {
        for (TrackState &t : s.tracks)
        {
            for (BlockState &b : t.blocks)
            {
                for (const Remap &r : std::as_const(remaps))
                {
                    if (b.functionId == r.from && b.audioFile == r.file && (lost || r.to != Function::invalidId()))
                    {
                        b.functionId = r.to;
                        break;
                    }
                }
            }
        }
    };
    if (remaps.isEmpty() == false)
    {
        // (an audio that could not be made at all: its block stays out)
        remap(state, true);
        History &history = m_history[show->id()];
        for (Step &step : history.undo)
            remap(step.state, false);
        for (Step &step : history.redo)
            remap(step.state, false);
        for (const Remap &r : std::as_const(remaps))
        {
            audioAfter.remove(r.from);
            audioAfter.insert(r.to);
        }
    }

    // the timeline settings
    if (show->endTime() != state.endTime)
        show->setEndTime(state.endTime);
    if (show->loop() != state.loop)
        show->setLoop(state.loop);
    if (show->gridOffset() != state.gridOffset)
        show->setGridOffset(state.gridOffset);
    show->setTimeDivision(Show::TimeDivision(state.division), state.bpm);

    // folding is not undone: a track keeps it, also where undo takes it (a
    // track moved up or down: known by its name)
    QHash<QString, bool> folded;
    QSet<QString> twice;
    for (Track *track : show->tracks())
    {
        if (folded.contains(track->name()))
            twice.insert(track->name());
        folded.insert(track->name(), track->isHidden());
    }

    // (b) the tracks and the blocks, by id. A running show reads them: all
    // of it at once
    {
        QMutexLocker locker(show->runnerMutex());

        QHash<quint32, quint32> wanted;
        QSet<quint32> tracks;
        for (const TrackState &t : std::as_const(state.tracks))
        {
            tracks.insert(t.id);
            for (const BlockState &b : t.blocks)
                wanted.insert(b.id, b.functionId);
        }

        // the blocks off their tracks: the ones it had wait, the others go (a
        // running show forgets them first: what they play stops)
        QHash<quint32, ShowFunction *> kept;
        for (Track *track : show->tracks())
        {
            for (ShowFunction *sf : track->showFunctions())
            {
                const bool keep = wanted.value(sf->id(), Function::invalidId()) == sf->functionID();
                track->removeShowFunction(sf, keep == false);
                if (keep)
                    kept.insert(sf->id(), sf);
            }
        }

        // the tracks it had, with their ids
        for (const TrackState &t : std::as_const(state.tracks))
        {
            if (show->track(t.id) != nullptr)
                continue;
            Track *track = new Track(t.sceneId, show);
            track->setName(t.name);
            track->setHidden(t.hidden);
            show->addTrack(track, t.id);
        }

        // their blocks, in their order (a block brought back plays from the
        // next Play, as one added while it plays)
        for (const TrackState &t : std::as_const(state.tracks))
        {
            Track *track = show->track(t.id);
            for (const BlockState &b : t.blocks)
            {
                ShowFunction *sf = kept.take(b.id);
                if (sf == nullptr)
                {
                    sf = new ShowFunction(b.id);
                    sf->setFunctionID(b.functionId);
                }
                sf->setStartTime(b.start);
                sf->setDuration(b.duration);
                sf->setFadeIn(b.fadeIn);
                sf->setFadeOut(b.fadeOut);
                sf->setStretch(b.stretch);
                sf->setLocked(b.locked);
                sf->setColor(b.color);
                if (track->addShowFunction(sf) == false)
                {
                    show->forgetBlock(sf);
                    delete sf;
                }
            }
        }
        // (a block twice in the state: never)
        for (ShowFunction *sf : std::as_const(kept))
        {
            show->forgetBlock(sf);
            delete sf;
        }

        // the tracks it didn't have, empty by now
        for (Track *track : show->tracks())
        {
            if (tracks.contains(track->id()) == false)
                show->removeTrack(track->id());
        }
    }

    for (const TrackState &t : std::as_const(state.tracks))
    {
        Track *track = show->track(t.id);
        if (track->name() != t.name)
        {
            if (folded.contains(t.name) && twice.contains(t.name) == false)
                track->setHidden(folded.value(t.name));
            track->setName(t.name);
        }
        track->setSceneID(t.sceneId);
        track->setMute(t.mute);
        track->setLocked(t.locked);
        if (samePoints(track->automation("dimmer"), t.dimmer) == false)
            track->setAutomation("dimmer", t.dimmer);
        if (samePoints(track->automation("phase"), t.phase) == false)
            track->setAutomation("phase", t.phase);
    }

    // (c) the static values (after the lock: the Doc deletes the scene)
    Scene *values = staticScene(show, false);
    quint32 valuesGone = Function::invalidId();
    if (state.hasStatic)
    {
        if (values == nullptr || sameValues(values->values(), state.staticValues) == false)
            setStaticValues(show, state.staticValues, state.staticId);
    }
    else if (values != nullptr)
    {
        valuesGone = values->id();
        show->setStaticScene(Function::invalidId());
    }

    // (d) the audio no block uses any more. Never with the lock held: the
    // Doc waits for the timer to let a function go, and the timer takes it
    for (quint32 fid : std::as_const(audioBefore))
    {
        if (audioAfter.contains(fid))
            continue;
        bool used = false;
        for (Function *f : m_doc->functionsByType(Function::ShowType))
            used = used || f->contains(fid);
        if (used == false)
            m_doc->deleteFunction(fid);
    }
    if (valuesGone != Function::invalidId())
        m_doc->deleteFunction(valuesGone);

    // (e) told when the silent edit ends
    touch(show);
}

void TimelineEditor::notify()
{
    const QSet<quint32> shows = m_pendingShows;
    const bool any = m_pendingChanged || shows.isEmpty() == false;
    m_pendingShows.clear();
    m_pendingChanged = false;
    for (quint32 id : shows)
    {
        if (Show *show = showOf(id))
            emit show->changed(id);
    }
    if (any)
        emit changed();
}

bool TimelineEditor::forgetFunction(quint32 fid)
{
    // (the audio of a super scene stays: it is made again from its file)
    auto strip = [fid](ShowState &state)
    {
        bool stripped = false;
        for (TrackState &t : state.tracks)
        {
            for (int i = t.blocks.count() - 1; i >= 0; i--)
            {
                if (t.blocks.at(i).functionId == fid && t.blocks.at(i).audioFile.isEmpty())
                {
                    t.blocks.removeAt(i);
                    stripped = true;
                }
            }
        }
        return stripped;
    };

    if (m_editDepth > 0 && m_recording)
        strip(m_before);
    bool forgotten = false;
    for (auto it = m_history.begin(); it != m_history.end(); ++it)
    {
        bool here = false;
        for (Step &step : it->undo)
            here = strip(step.state) || here;
        for (Step &step : it->redo)
            here = strip(step.state) || here;
        // (not in the middle of an edit: the super scene is not as it ends)
        if (here && m_editDepth == 0)
            prune(it.key());
        forgotten = forgotten || here;
    }
    return forgotten;
}

void TimelineEditor::prune(quint32 showId)
{
    Show *show = showOf(showId);
    if (show == nullptr || m_history.contains(showId) == false)
        return;
    History &history = m_history[showId];

    // an undo step goes from its state to the next one (the last one to the
    // super scene as it is), a redo step the other way
    const ShowState now = capture(show);
    ShowState next = now;
    for (int i = history.undo.count() - 1; i >= 0; i--)
    {
        if (same(history.undo.at(i).state, next))
            history.undo.removeAt(i);
        else
            next = history.undo.at(i).state;
    }
    ShowState previous = now;
    for (int i = history.redo.count() - 1; i >= 0; i--)
    {
        if (same(history.redo.at(i).state, previous))
            history.redo.removeAt(i);
        else
            previous = history.redo.at(i).state;
    }
}

Audio *TimelineEditor::createOwnedAudio(const QString &path, const QString &name, quint32 preferredId, bool strict)
{
    Audio *audio = new Audio(m_doc);
    // (a file that is gone: the Doc's error log says so)
    const bool read = audio->setSourceFileName(path);
    if (strict && (read == false || audio->totalDuration() == 0))
    {
        delete audio;
        return nullptr;
    }
    audio->setName(name.isEmpty() ? QFileInfo(path).completeBaseName() : name);
    // the audio belongs to the super scene: not in the lists of scenes
    audio->setVisible(false);
    const bool free = preferredId != Function::invalidId() && m_doc->function(preferredId) == nullptr;
    if (m_doc->addFunction(audio, free ? preferredId : Function::invalidId()) == false)
    {
        delete audio;
        return nullptr;
    }
    return audio;
}

bool TimelineEditor::ownedAudio(quint32 fid, const Show *show, const QString &file) const
{
    Audio *audio = qobject_cast<Audio *>(m_doc->function(fid));
    if (audio == nullptr || audio->isVisible() || (file.isEmpty() == false && audio->getSourceFileName() != file))
        return false;
    for (Function *f : m_doc->functionsByType(Function::ShowType))
    {
        if (f != show && f->contains(fid))
            return false;
    }
    return true;
}

/*********************************************************************
 * Playback
 *********************************************************************/

void TimelineEditor::play(quint32 showId, int from)
{
    Show *show = showOf(showId);
    if (show == nullptr)
        return;

    stop();
    connect(show, SIGNAL(timeChanged(quint32)), this, SLOT(slotTimeChanged(quint32)), Qt::UniqueConnection);
    connect(show, SIGNAL(stopped(quint32)), this, SLOT(slotShowStopped(quint32)), Qt::UniqueConnection);
    m_position = qMax(0, from);
    m_playingId = int(showId);
    show->start(m_doc->masterTimer(), FunctionParent::master(), quint32(m_position));
    emit playingChanged();
    emit positionChanged();
}

void TimelineEditor::stop()
{
    if (m_playingId < 0)
        return;
    if (Show *show = showOf(quint32(m_playingId)))
    {
        // (one a fader of the SHOW holds up: only that fader stops it)
        if (m_heldByFader == nullptr || m_heldByFader(show->id()) == false)
            show->stop(FunctionParent::master());
        // (played elsewhere later, it doesn't move the playhead)
        disconnect(show, SIGNAL(timeChanged(quint32)), this, SLOT(slotTimeChanged(quint32)));
    }
    m_playingId = -1;
    emit playingChanged();
}

void TimelineEditor::slotTimeChanged(quint32 time)
{
    // only the super scene playing from here: another one played from a
    // group, a playback... (or the times still queued of one stopped) don't
    // move the playhead
    Show *show = qobject_cast<Show *>(sender());
    if (show == nullptr || int(show->id()) != m_playingId)
        return;
    m_position = int(time);
    emit positionChanged();
}

void TimelineEditor::slotShowStopped(quint32 id)
{
    if (int(id) != m_playingId)
        return;
    // the end of the run before, after it started again (Play again, To the
    // start): the new run plays on
    Show *show = showOf(id);
    if (show != nullptr && show->stopped() == false)
        return;
    if (show != nullptr)
        disconnect(show, SIGNAL(timeChanged(quint32)), this, SLOT(slotTimeChanged(quint32)));
    m_playingId = -1;
    emit playingChanged();
}

/*********************************************************************
 * Audio
 *********************************************************************/

QVariantList TimelineEditor::waveform(quint32 audioId)
{
    Audio *audio = qobject_cast<Audio *>(m_doc->function(audioId));
    if (audio == nullptr)
        return QVariantList();

    QString path = audio->getSourceFileName();
    if (m_waveforms.contains(path))
        return m_waveforms.value(path);
    if (m_computing.value(path))
        return QVariantList();

    // the decoder is made here (its plugin loader lives in this thread),
    // the file is read in a thread
    AudioDecoder *decoder = m_doc->audioPluginCache()->getDecoderForFile(path);
    if (decoder == nullptr)
        return QVariantList();
    m_computing.insert(path, true);

    QPointer<TimelineEditor> self(this);
    QThread *thread = QThread::create([self, decoder, path, audioId]()
    {
        AudioParameters ap = decoder->audioParameters();
        int channels = qMax(1, ap.channels());
        // the sndfile and mad plugins give 16 bit samples
        int sampleSize = ap.sampleSize() == 1 ? 1 : 2;
        qint64 frame = qint64(channels) * sampleSize;
        qint64 bucket = qMax<qint64>(1, qint64(ap.sampleRate()) * waveformStep() / 1000) * frame;

        QVariantList peaks;
        QByteArray buffer(int(bucket), 0);
        decoder->seek(0);
        while (true)
        {
            qint64 got = 0;
            while (got < bucket)
            {
                qint64 n = decoder->read(buffer.data() + got, bucket - got);
                if (n <= 0)
                    break;
                got += n;
            }
            if (got <= 0)
                break;

            int peak = 0;
            if (sampleSize == 1)
            {
                for (qint64 i = 0; i < got; i++)
                    peak = qMax(peak, qAbs(int(qint8(buffer.at(int(i))))) * 256);
            }
            else
            {
                const qint16 *samples = reinterpret_cast<const qint16 *>(buffer.constData());
                for (qint64 i = 0; i < got / 2; i++)
                    peak = qMax(peak, qAbs(int(samples[i])));
            }
            peaks.append(qMin(1.0, peak / 32768.0));
            if (got < bucket)
                break;
        }

        QMetaObject::invokeMethod(qApp, [self, decoder, path, audioId, peaks]() {
            delete decoder;
            if (self.isNull())
                return;
            self->m_waveforms.insert(path, peaks);
            self->m_computing.remove(path);
            emit self->waveformReady(audioId);
        }, Qt::QueuedConnection);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start(QThread::LowPriority);
    return QVariantList();
}
