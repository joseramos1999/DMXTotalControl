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
#include <QSet>
#include <QUrl>
#include <QColor>
#include <QElapsedTimer>
#include <functional>

#include "scenevalue.h"
#include "track.h"

class ShowFunction;
class Programmer;
class Audio;
class Scene;
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
 *
 * Each super scene has its own undo history (in memory, up to 100 steps):
 * every edit below is one step, a copy of the super scene as it was before.
 * Folding a track (hidden) is not a step.
 *
 * Blocks copied wait in a clipboard of their own (in memory, while the show
 * is open) to be pasted in this super scene or another one.
 */
class TimelineEditor : public QObject
{
    Q_OBJECT

    /** Changes on every edit: QML reads show() again */
    Q_PROPERTY(int revision READ revision NOTIFY changed)
    /** Changes when a history changes: QML reads history() again */
    Q_PROPERTY(int historyRevision READ historyRevision NOTIFY historyChanged)
    /** The super scene playing from the editor, -1 for none */
    Q_PROPERTY(int playingId READ playingId NOTIFY playingChanged)
    /** Where the playing super scene is (ms) */
    Q_PROPERTY(int position READ position NOTIFY positionChanged)
    /** The blocks copied (or cut), waiting to be pasted */
    Q_PROPERTY(int clipboardCount READ clipboardCount NOTIFY clipboardChanged)

public:
    TimelineEditor(Doc *doc, Programmer *programmer, QObject *parent = nullptr);
    ~TimelineEditor();

    /** A super scene a fader of the SHOW holds up: stop() here doesn't stop
     *  it (only that fader stops it) */
    void setHeldByFader(const std::function<bool(quint32)> &held);

    int revision() const;
    int historyRevision() const;
    int playingId() const;
    int position() const;
    int clipboardCount() const;

    /*********************************************************************
     * Undo
     *********************************************************************/
    /** { canUndo, canRedo, undoText, redoText (what the step did) } */
    Q_INVOKABLE QVariantMap history(quint32 showId) const;
    /** @return false with nothing to undo (or redo) */
    Q_INVOKABLE bool undo(quint32 showId);
    Q_INVOKABLE bool redo(quint32 showId);

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
    /** $merge: the changes of one field within a second are one undo step
     *  (a spin box held down) */
    Q_INVOKABLE void moveBlock(quint32 showId, int blockId, int trackId, int start, bool merge = false);
    Q_INVOKABLE void resizeBlock(quint32 showId, int blockId, int start, int duration, bool merge = false);
    /** $key: "fadeIn", "fadeOut" (ms), "stretch", "locked", "color" */
    Q_INVOKABLE void setBlock(quint32 showId, int blockId, const QString &key, const QVariant &value,
                              bool merge = false);
    /** The audio of the super scene goes with its last block */
    Q_INVOKABLE void removeBlock(quint32 showId, int blockId);
    /** A copy right after it. @return its id */
    Q_INVOKABLE int duplicateBlock(quint32 showId, int blockId);

    /*********************************************************************
     * Several blocks at once (one undo step each)
     *********************************************************************/
    /** By $deltaMs in time and $trackOffset tracks down (up when negative),
     *  all by the same. Locked blocks and blocks on locked tracks stay. The
     *  group never goes before 0, and it moves only in time when a block
     *  would land outside the tracks or on a locked one */
    Q_INVOKABLE void moveBlocks(quint32 showId, const QVariantList &blockIds, int deltaMs, int trackOffset);
    /** The blocks on locked tracks stay */
    Q_INVOKABLE void removeBlocks(quint32 showId, const QVariantList &blockIds);
    /** Copies right after the group, each on the track of its block (not a
     *  locked one). @return their ids */
    Q_INVOKABLE QVariantList duplicateBlocks(quint32 showId, const QVariantList &blockIds);
    /** setBlock() on each of them */
    Q_INVOKABLE void setBlocks(quint32 showId, const QVariantList &blockIds, const QString &key, const QVariant &value);

    /*********************************************************************
     * Copy and paste
     *********************************************************************/
    /** The blocks to the clipboard, with their places (the locked ones too).
     *  @return how many */
    Q_INVOKABLE int copyBlocks(quint32 showId, const QVariantList &blockIds);
    /** Copied, then deleted (one undo step). The blocks on locked tracks
     *  are copied, and stay. @return how many were copied */
    Q_INVOKABLE int cutBlocks(quint32 showId, const QVariantList &blockIds);
    /** The blocks of the clipboard: the earliest at $start, the topmost on
     *  $trackId (the first track when it is none), the others as far from
     *  them as they were. Missing tracks are added at the end, a folded one
     *  unfolds; none goes on a locked track. Scenes are shared; the audio of
     *  another super scene is made again from its file. One undo step.
     *  @return the ids of the new blocks */
    Q_INVOKABLE QVariantList pasteBlocks(quint32 showId, int trackId, int start);

    /*********************************************************************
     * Automation and settings
     *********************************************************************/
    /** $lane "dimmer" or "phase": [{ time, value (0-1), smooth }] */
    Q_INVOKABLE void setAutomation(quint32 showId, int trackId, const QString &lane, const QVariantList &points);
    /** Some of: endTime, loop, grid ("time"/"bars"), bpm, beatsPerBar (2-4), gridOffset */
    Q_INVOKABLE void setSettings(quint32 showId, const QVariantMap &settings, bool merge = false);

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
    void historyChanged();
    void playingChanged();
    void positionChanged();
    void waveformReady(quint32 audioId);
    void message(const QString &text);
    void clipboardChanged();

private slots:
    void slotFunctionRemoved(quint32 fid);
    void slotCleared();
    void slotTimeChanged(quint32 time);
    void slotShowStopped(quint32 id);

private:
    Show *showOf(quint32 id) const;
    /** The show changed: told at once, or at the end of the edit */
    void touch(Show *show);
    /** The length of one cycle of a function, 0 for a static scene */
    quint32 cycleOf(quint32 functionId) const;

    /** The blocks of $ids that are in $show, each once, in the order given */
    QList<ShowFunction *> blocksOf(Show *show, const QVariantList &ids) const;
    /** $sf to $start, and onto $to when it is another track, not locked */
    void moveOne(ShowFunction *sf, Track *from, Track *to, quint32 start);
    /** @return false for a $key it doesn't know */
    bool setOne(ShowFunction *sf, const QString &key, const QVariant &value);
    /** A new track at the end, "Track N" without a $name */
    Track *appendTrack(Show *show, const QString &name);
    /** $f can be a block of $show: not a super scene, nor a rack that plays
     *  $show (said, if $tell) */
    bool canHold(Show *show, Function *f, bool tell);

    /*********************************************************************
     * Undo: a super scene as it is
     *********************************************************************/
private:
    struct BlockState
    {
        quint32 id;
        quint32 functionId;
        quint32 start;
        /** ShowFunction::duration() as it is (0: the function's own) */
        quint32 duration;
        quint32 fadeIn;
        quint32 fadeOut;
        bool stretch;
        bool locked;
        QColor color;
        /** The file and the name of an audio of the super scene (hidden,
         *  made with its block): empty for anything else */
        QString audioFile;
        QString audioName;
    };

    struct TrackState
    {
        quint32 id;
        quint32 sceneId;
        QString name;
        bool mute;
        bool locked;
        /** Folded: not compared, set again only on a track brought back */
        bool hidden;
        QList<Track::AutomationPoint> dimmer;
        QList<Track::AutomationPoint> phase;
        /** In the order of the track */
        QList<BlockState> blocks;
    };

    struct ShowState
    {
        /** In the order of Show::tracks() */
        QList<TrackState> tracks;
        quint32 endTime = 0;
        quint32 gridOffset = 0;
        bool loop = false;
        int division = 0;
        int bpm = 120;
        bool hasStatic = false;
        /** The id of the static scene (not compared: it may come back with
         *  another one) */
        quint32 staticId = UINT_MAX;
        QList<SceneValue> staticValues;
    };

    struct Step
    {
        /** The super scene before the step (undo) or after it (redo) */
        ShowState state;
        QString text;
        /** Steps of the same key within a second are merged */
        QString key;
        qint64 at = 0;
    };

    struct History
    {
        QList<Step> undo;
        QList<Step> redo;
    };

    /** One step of the history per call of an edit (the outermost one when
     *  they nest: addAudio's addBlock is part of "Add audio"). The show is
     *  told once, when it ends. A silent edit (undo, redo) makes no step */
    class Edit
    {
    public:
        Edit(TimelineEditor *editor, Show *show, const QString &text, const QString &key = QString(),
             bool record = true);
        ~Edit();
        /** What the step did, once it is known (when it is the step) */
        void setText(const QString &text);

    private:
        Q_DISABLE_COPY(Edit)
        TimelineEditor *m_editor;
        bool m_outermost;
    };

    ShowState capture(const Show *show) const;
    /** A block as it is */
    BlockState blockState(const ShowFunction *sf) const;
    static bool same(const ShowState &a, const ShowState &b);
    /** The state after an edit: a step of the history, if it changed */
    bool record(Show *show);
    /** Undo or redo */
    bool travel(quint32 showId, bool back);
    /** Back to $state, by ids (the blocks keep theirs). The audio of the
     *  super scene is made again from its file */
    void apply(Show *show, ShowState state);
    /** The changes waiting for the end of the edit, told */
    void notify();
    /** The blocks of a function deleted (a scene of the list) never come back */
    bool forgetFunction(quint32 fid);
    /** Steps that change nothing any more go */
    void prune(quint32 showId);

    /** An audio of a super scene (hidden), with $preferredId when it is free.
     *  $strict: a file that can't be read is refused (else it is made all
     *  the same, as a show opened without its audio) */
    Audio *createOwnedAudio(const QString &path, const QString &name, quint32 preferredId, bool strict);
    /** $fid is a hidden audio no other super scene plays (of $file, if given) */
    bool ownedAudio(quint32 fid, const Show *show, const QString &file = QString()) const;
    /** The hidden scene of the static values of $show, made if $create */
    Scene *staticScene(Show *show, bool create, quint32 preferredId = UINT_MAX);
    bool setStaticValues(Show *show, const QList<SceneValue> &values, quint32 preferredId = UINT_MAX);

    /*********************************************************************
     * The clipboard
     *********************************************************************/
private:
    struct ClipBlock
    {
        /** Its track, counted from the topmost one copied */
        int row;
        /** After the earliest block copied (ms) */
        quint32 offset;
        /** Its duration as it plays (never 0) */
        BlockState block;
    };

    /** What a block of the clipboard plays in $show, invalid when it can't
     *  go there. $made: the audio of another super scene copied here (by
     *  the id of the one copied, invalid for a file that can't be read) */
    quint32 pastedFunction(Show *show, const BlockState &block, QHash<quint32, quint32> &made,
                           QSet<quint32> &told);
    /** The blocks of $fid leave the clipboard (not the audio of a super
     *  scene: it is made again from its file) */
    void forgetClips(quint32 fid);

private:
    Doc *m_doc;
    Programmer *m_programmer;
    std::function<bool(quint32)> m_heldByFader;
    int m_revision;
    int m_playingId;
    int m_position;
    QHash<QString, QVariantList> m_waveforms;
    QHash<QString, bool> m_computing;

    // the histories, by super scene
    QHash<quint32, History> m_history;
    int m_historyRevision;
    QElapsedTimer m_clock;
    // the edit in progress: how deep, what it was before, what to tell
    int m_editDepth;
    bool m_recording;
    quint32 m_editShow;
    ShowState m_before;
    QString m_editText;
    QString m_editKey;
    QSet<quint32> m_pendingShows;
    bool m_pendingChanged;

    // the blocks copied, while the show is open
    QList<ClipBlock> m_clipboard;
};

#endif
