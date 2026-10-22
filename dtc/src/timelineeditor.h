/*
  DMX Total Control
  timelineeditor.h

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

#ifndef TIMELINEEDITOR_H
#define TIMELINEEDITOR_H

#include <QObject>
#include <QVariant>
#include <QHash>
#include <QUrl>

class ShowFunction;
class Programmer;
class Track;
class Show;
class Doc;

/**
 * Super scenes: a timeline of tracks with blocks of scenes and of
 * audio [the engine Show]. Blocks have fades and loop or stretch their scene;
 * tracks have automation lanes (dimmer, which is the volume of audio, and
 * the phase of effects), can be locked and hidden. The show has a grid in
 * time or in bars, an end marker, a loop and static values of its own.
 *
 * Times are in milliseconds. Blocks and tracks are known by their ids in
 * the show.
 */
class TimelineEditor : public QObject
{
    Q_OBJECT

    /** Changes on every edit: QML reads show() again */
    Q_PROPERTY(int revision READ revision NOTIFY changed)
    /** The super scene playing from the editor, -1 for none */
    Q_PROPERTY(int playingId READ playingId NOTIFY playingChanged)
    /** Where the playing super scene is (ms) */
    Q_PROPERTY(int position READ position NOTIFY positionChanged)

public:
    TimelineEditor(Doc *doc, Programmer *programmer, QObject *parent = nullptr);
    ~TimelineEditor();

    int revision() const;
    int playingId() const;
    int position() const;

    /**
     * { id, name, length (ms), endTime, loop, grid ("time"/"bars"), bpm,
     *   beatsPerBar, gridOffset, staticValues (channels),
     *   tracks: [{ id, name, mute, locked, hidden,
     *              dimmer: [{ time, value, smooth }], phase: [...],
     *              blocks: [{ id, functionId, name, type, start, duration,
     *                         fadeIn, fadeOut, stretch, color, cycle, locked }] }] }
     */
    Q_INVOKABLE QVariantMap show(quint32 id) const;

    /*********************************************************************
     * Tracks
     *********************************************************************/
    Q_INVOKABLE int addTrack(quint32 showId, const QString &name = QString());
    Q_INVOKABLE void removeTrack(quint32 showId, int trackId);
    /** $key: "name", "mute", "locked", "hidden" */
    Q_INVOKABLE void setTrack(quint32 showId, int trackId, const QString &key, const QVariant &value);
    Q_INVOKABLE void moveTrack(quint32 showId, int trackId, int direction);

    /*********************************************************************
     * Blocks
     *********************************************************************/
    /** A block of a scene at $start, as long as one cycle of it (10 s for a
     *  static scene). @return its id, -1 on error */
    Q_INVOKABLE int addBlock(quint32 showId, int trackId, quint32 functionId, int start);
    /** A block of audio from a file. @return its id, -1 on error */
    Q_INVOKABLE int addAudio(quint32 showId, int trackId, const QUrl &file, int start);
    Q_INVOKABLE void moveBlock(quint32 showId, int blockId, int trackId, int start);
    Q_INVOKABLE void resizeBlock(quint32 showId, int blockId, int start, int duration);
    /** $key: "fadeIn", "fadeOut" (ms), "stretch", "locked", "color" */
    Q_INVOKABLE void setBlock(quint32 showId, int blockId, const QString &key, const QVariant &value);
    Q_INVOKABLE void removeBlock(quint32 showId, int blockId);
    /** A copy right after it. @return its id */
    Q_INVOKABLE int duplicateBlock(quint32 showId, int blockId);

    /*********************************************************************
     * Automation and settings
     *********************************************************************/
    /** $lane "dimmer" or "phase": [{ time, value (0-1), smooth }] */
    Q_INVOKABLE void setAutomation(quint32 showId, int trackId, const QString &lane, const QVariantList &points);
    /** Some of: endTime, loop, grid ("time"/"bars"), bpm, beatsPerBar (2-4), gridOffset */
    Q_INVOKABLE void setSettings(quint32 showId, const QVariantMap &settings);

    /** The static values of the super scene: the values of the programmer.
     *  @return the number of channels */
    Q_INVOKABLE int captureStatic(quint32 showId);
    Q_INVOKABLE void clearStatic(quint32 showId);

    /*********************************************************************
     * Playback
     *********************************************************************/
    Q_INVOKABLE void play(quint32 showId, int from = 0);
    Q_INVOKABLE void stop();

    /*********************************************************************
     * Audio
     *********************************************************************/
    /** The peaks of an audio function, 0-1, one every waveformStep() ms. The
     *  first call computes them in the background (waveformReady) */
    Q_INVOKABLE QVariantList waveform(quint32 audioId);
    Q_INVOKABLE static int waveformStep() { return 25; }

signals:
    void changed();
    void playingChanged();
    void positionChanged();
    void waveformReady(quint32 audioId);
    void message(const QString &text);

private slots:
    void slotFunctionRemoved();
    void slotTimeChanged(quint32 time);
    void slotShowStopped(quint32 id);

private:
    Show *showOf(quint32 id) const;
    void touch(Show *show);
    /** The length of one cycle of a function, 0 for a static scene */
    quint32 cycleOf(quint32 functionId) const;

private:
    Doc *m_doc;
    Programmer *m_programmer;
    int m_revision;
    int m_playingId;
    int m_position;
    QHash<QString, QVariantList> m_waveforms;
    QHash<QString, bool> m_computing;
};

#endif
