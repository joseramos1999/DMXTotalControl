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

TimelineEditor::TimelineEditor(Doc *doc, Programmer *programmer, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_programmer(programmer)
    , m_revision(0)
    , m_playingId(-1)
    , m_position(0)
{
    // (a show cleared: once, when it is, not for each of its functions)
    connect(m_doc, SIGNAL(functionRemoved(quint32)), this, SLOT(slotFunctionRemoved()));
    connect(m_doc, SIGNAL(cleared()), this, SIGNAL(changed()));
}

TimelineEditor::~TimelineEditor()
{
}

void TimelineEditor::slotFunctionRemoved()
{
    if (m_doc->isClearing() == false)
        emit changed();
}

int TimelineEditor::revision() const
{
    return m_revision;
}

int TimelineEditor::playingId() const
{
    return m_playingId;
}

int TimelineEditor::position() const
{
    return m_position;
}

Show *TimelineEditor::showOf(quint32 id) const
{
    return qobject_cast<Show *>(m_doc->function(id));
}

void TimelineEditor::touch(Show *show)
{
    m_revision++;
    m_doc->setModified();
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

    Track *track = new Track(Function::invalidId(), show);
    track->setName(name.trimmed().isEmpty() ? tr("Track %1").arg(show->tracks().count() + 1) : name.trimmed());
    show->addTrack(track);
    touch(show);
    return int(track->id());
}

void TimelineEditor::removeTrack(quint32 showId, int trackId)
{
    Show *show = showOf(showId);
    if (show == nullptr || show->track(quint32(trackId)) == nullptr)
        return;

    // a playing show (whoever started it) forgets the track and its blocks
    // first, and plays on (Show::removeTrack)
    show->removeTrack(quint32(trackId));
    touch(show);
}

void TimelineEditor::setTrack(quint32 showId, int trackId, const QString &key, const QVariant &value)
{
    Show *show = showOf(showId);
    Track *track = show != nullptr ? show->track(quint32(trackId)) : nullptr;
    if (track == nullptr)
        return;

    if (key == "name" && value.toString().trimmed().isEmpty() == false)
        track->setName(value.toString().trimmed());
    else if (key == "mute")
        track->setMute(value.toBool());
    else if (key == "locked")
        track->setLocked(value.toBool());
    else if (key == "hidden")
        track->setHidden(value.toBool());
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
    if (track == nullptr || f == nullptr || track->isLocked() || functionId == showId ||
        (f->type() == Function::ShowType))
        return -1;
    // a rack (or steps) that plays this super scene: it would play itself
    if (f->contains(showId))
    {
        emit message(tr("“%1” plays this super scene: it can't go into it").arg(f->name()));
        return -1;
    }

    ShowFunction *sf = track->createShowFunction(functionId);
    sf->setStartTime(quint32(qMax(0, start)));
    quint32 cycle = f->type() == Function::AudioType ? f->totalDuration() : cycleOf(functionId);
    sf->setDuration(cycle > 0 ? cycle : DEFAULT_BLOCK_MS);
    touch(show);
    return int(sf->id());
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

    QString path = file.isLocalFile() ? file.toLocalFile() : file.toString();
    Audio *audio = new Audio(m_doc);
    if (audio->setSourceFileName(path) == false || audio->totalDuration() == 0)
    {
        delete audio;
        emit message(tr("Unable to read the audio of %1").arg(QFileInfo(path).fileName()));
        return -1;
    }
    audio->setName(QFileInfo(path).completeBaseName());
    // the audio belongs to the super scene: not in the lists of scenes
    audio->setVisible(false);
    if (m_doc->addFunction(audio) == false)
    {
        delete audio;
        return -1;
    }
    const int block = addBlock(showId, trackId, audio->id(), start);
    if (block < 0)
        m_doc->deleteFunction(audio->id());
    return block;
}

void TimelineEditor::moveBlock(quint32 showId, int blockId, int trackId, int start)
{
    Show *show = showOf(showId);
    ShowFunction *sf = show != nullptr ? show->showFunction(quint32(blockId)) : nullptr;
    Track *from = show != nullptr ? show->getTrackFromShowFunctionID(quint32(blockId)) : nullptr;
    Track *to = show != nullptr ? show->track(quint32(trackId)) : nullptr;
    if (sf == nullptr || from == nullptr || sf->isLocked() || from->isLocked())
        return;

    sf->setStartTime(quint32(qMax(0, start)));
    if (to != nullptr && to != from && to->isLocked() == false)
    {
        from->removeShowFunction(sf, false);
        to->addShowFunction(sf);
    }
    touch(show);
}

void TimelineEditor::resizeBlock(quint32 showId, int blockId, int start, int duration)
{
    Show *show = showOf(showId);
    ShowFunction *sf = show != nullptr ? show->showFunction(quint32(blockId)) : nullptr;
    Track *track = show != nullptr ? show->getTrackFromShowFunctionID(quint32(blockId)) : nullptr;
    if (sf == nullptr || sf->isLocked() || (track != nullptr && track->isLocked()))
        return;

    sf->setStartTime(quint32(qMax(0, start)));
    sf->setDuration(quint32(qMax(100, duration)));
    // the fades fit in the block
    sf->setFadeIn(qMin(sf->fadeIn(), sf->duration()));
    sf->setFadeOut(qMin(sf->fadeOut(), sf->duration() - sf->fadeIn()));
    touch(show);
}

void TimelineEditor::setBlock(quint32 showId, int blockId, const QString &key, const QVariant &value)
{
    Show *show = showOf(showId);
    ShowFunction *sf = show != nullptr ? show->showFunction(quint32(blockId)) : nullptr;
    if (sf == nullptr)
        return;

    quint32 duration = sf->duration(m_doc);
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
        return;
    touch(show);
}

void TimelineEditor::removeBlock(quint32 showId, int blockId)
{
    Show *show = showOf(showId);
    ShowFunction *sf = show != nullptr ? show->showFunction(quint32(blockId)) : nullptr;
    Track *track = show != nullptr ? show->getTrackFromShowFunctionID(quint32(blockId)) : nullptr;
    if (sf == nullptr || track == nullptr || track->isLocked())
        return;

    // a playing show (whoever started it) forgets the block first, and plays
    // on (Track::removeShowFunction). The audio of the block goes with it,
    // when nothing else plays it: Doc::deleteFunction stops it first
    quint32 fid = sf->functionID();
    track->removeShowFunction(sf, true);
    Function *f = m_doc->function(fid);
    if (f != nullptr && f->type() == Function::AudioType && show->contains(fid) == false)
        m_doc->deleteFunction(fid);
    touch(show);
}

int TimelineEditor::duplicateBlock(quint32 showId, int blockId)
{
    Show *show = showOf(showId);
    ShowFunction *sf = show != nullptr ? show->showFunction(quint32(blockId)) : nullptr;
    Track *track = show != nullptr ? show->getTrackFromShowFunctionID(quint32(blockId)) : nullptr;
    if (sf == nullptr || track == nullptr || track->isLocked())
        return -1;

    ShowFunction *copy = track->createShowFunction(sf->functionID());
    copy->setStartTime(sf->startTime() + sf->duration(m_doc));
    copy->setDuration(sf->duration(m_doc));
    copy->setColor(sf->color());
    copy->setFadeIn(sf->fadeIn());
    copy->setFadeOut(sf->fadeOut());
    copy->setStretch(sf->stretch());
    touch(show);
    return int(copy->id());
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
    track->setAutomation(lane, list);
    touch(show);
}

void TimelineEditor::setSettings(quint32 showId, const QVariantMap &settings)
{
    Show *show = showOf(showId);
    if (show == nullptr)
        return;

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

    // a hidden scene of the super scene holds them
    Scene *scene = qobject_cast<Scene *>(m_doc->function(show->staticScene()));
    if (scene == nullptr)
    {
        scene = new Scene(m_doc);
        scene->setVisible(false);
        if (m_doc->addFunction(scene) == false)
        {
            delete scene;
            return 0;
        }
        show->setStaticScene(scene->id());
    }
    scene->setName(tr("%1 (static values)").arg(show->name()));
    scene->clear();
    for (const SceneValue &scv : values)
    {
        if (scene->fixtures().contains(scv.fxi) == false)
            scene->addFixture(scv.fxi);
        scene->setValue(scv);
    }
    touch(show);
    return values.count();
}

void TimelineEditor::clearStatic(quint32 showId)
{
    Show *show = showOf(showId);
    if (show == nullptr || show->staticScene() == Function::invalidId())
        return;

    // (playing with the show, Doc::deleteFunction stops it first)
    quint32 scene = show->staticScene();
    show->setStaticScene(Function::invalidId());
    m_doc->deleteFunction(scene);
    touch(show);
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
