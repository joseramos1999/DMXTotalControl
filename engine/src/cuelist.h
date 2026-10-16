/*
  DMX Total Control
  cuelist.h

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

#ifndef CUELIST_H
#define CUELIST_H

#include <functional>
#include <QStringList>
#include <QMutex>
#include <QList>
#include <QHash>
#include <QPair>
#include <QSet>

#include "function.h"
#include "fxscene.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class GenericFader;
class Universe;
class Fixture;
class Doc;

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLDTCCuelist          QStringLiteral("Cuelist")
#define KXMLDTCCue              QStringLiteral("Cue")
#define KXMLDTCCueNote          QStringLiteral("Note")
#define KXMLDTCCueValue         QStringLiteral("V")
#define KXMLDTCCueRelease       QStringLiteral("R")
#define KXMLDTCCueFxRelease     QStringLiteral("FxR")

/**
 * A cuelist: numbered cues that hold only what they
 * change. What the stage shows on a cue is the fold of the cues before it
 * (tracking): a value carries on until another cue changes it or releases it,
 * and a released channel goes back to what's below. A cue can also start,
 * change or stop FX layers, tracked per (axis, fixture).
 *
 * The list plays with one persistent fader per universe and, on each GO,
 * changes only the channels whose tracked target changed: no dips, and the
 * tracked channels are not taken back from the other playbacks. The tracked
 * FX layers play in a private FXScene, with the static values of their
 * channels as bases: the faders of the list never hold an FX channel.
 */
class Cuelist final : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(Cuelist)

public:
    /** (fixture, channel) */
    typedef QPair<quint32, quint32> Key;
    /** (axis, fixture): "pan"/"tilt" for position FX, else the param of the layer */
    typedef QPair<QString, quint32> FxKey;

    /** What starts a cue: GO, the end of the previous cue (+ time), or a time after its GO */
    enum Trigger { TriggerGo = 0, TriggerFollow, TriggerWait };
    /** GO on the last cue: back to the first (default), release the list, or nothing.
     *  DMX Total Control: a Follow or Wait on the first cue goes on from the last cue by
     *  itself: Loop wraps to the first cue, Release releases the list (Stay ignores it) */
    enum AtEnd { AtEndLoop = 0, AtEndRelease, AtEndStay };

    struct Cue
    {
        /** stable inside the list, never reused */
        quint32 uid = 0;
        /** thousandths: "2.5" -> 2500; unique; the list is kept sorted */
        int number = 0;
        QString name;
        QString note;
        Trigger trigger = TriggerGo;
        /** ms. Follow: after the previous cue completes. Wait: after the previous cue's GO */
        uint triggerTime = 0;
        /** ms: intensities up, colour mix, every LTP move */
        uint fade = 1000;
        /** ms: dimmers going down; -1 = same as fade */
        int downFade = -1;
        /** ms before this cue's fades start */
        uint delay = 0;
        /** cue only: its data doesn't track on to the next cues */
        bool skip = false;
        /** hard values: Record and Update keep values equal to the tracked ones */
        bool block = false;
        /** only what this cue changes */
        QHash<Key, uchar> values;
        /** the DTC palette of some of the values (opaque to the engine) */
        QHash<Key, int> palettes;
        /** released: the list stops controlling them, back to what's below */
        QSet<Key> releases;
        /** the FX it starts or changes, with explicit fixtures */
        QList<FXScene::Layer> fx;
        /** the FX it stops */
        QSet<FxKey> fxReleases;

        uint effectiveDown() const { return downFade < 0 ? fade : uint(downFade); }
        uint length() const { return delay + qMax(fade, effectiveDown()); }
        bool operator==(const Cue &other) const;
        bool operator!=(const Cue &other) const { return !(*this == other); }
    };

    /** What the stage shows on a cue */
    struct State
    {
        QHash<Key, uchar> values;
        /** the uid of the cue that set each value */
        QHash<Key, quint32> owner;
        QHash<Key, int> palettes;
        /** the tracked FX layers, each one on the fixtures it still owns */
        QList<FXScene::Layer> fx;
        /** the uid of the cue that holds each FX slot */
        QHash<FxKey, quint32> fxOwner;

        bool operator==(const State &other) const;
        bool operator!=(const State &other) const { return !(*this == other); }
    };

    /** The playback state, for polling */
    struct Status
    {
        int current = -1;
        quint32 currentUid = 0;
        int next = -1;
        /** of the fades the current cue runs with (its own times, a BACK
         *  time, the fade-in of a group or a block) */
        qreal progress = 1.0;
        bool fading = false;
        /** a Follow or Wait (or, past the end, the release) will go on by itself */
        bool waiting = false;
    };

public:
    Cuelist(Doc *doc);
    ~Cuelist();

    QIcon getIcon() const override;
    quint32 totalDuration() override;

    Function *createCopy(Doc *doc, bool addToDoc = true) override;
    bool copyFrom(const Function *function) override;

    /*********************************************************************
     * Cue numbers
     *********************************************************************/
public:
    /** "3", "2.5" or "2,5" in thousandths (3 decimals at most, 0.001 to
     *  99999.999), or -1 when it is not a cue number */
    static int parseNumber(const QString &text);
    /** 2500 -> "2.5", 3000 -> "3", 1250 -> "1.25" (always with '.') */
    static QString numberToString(int number);
    static int maxNumber() { return 99999999; }

    /** The slots of an FX parameter: "pan" and "tilt" for "pantilt", else the param */
    static QStringList fxAxes(const QString &param);
    /** Every (axis, fixture) slot a layer covers */
    static QSet<FxKey> fxKeysOf(const FXScene::Layer &layer);

    /*********************************************************************
     * Contents. Any thread; every edit emits changed() and, while the list
     * runs, makes it play the new data at once
     *********************************************************************/
public:
    int cueCount() const;
    QList<Cue> cues() const;
    /** A default Cue (uid 0) when $index is out of range */
    Cue cueAt(int index) const;
    int indexOfUid(quint32 uid) const;
    int indexOfNumber(int number) const;
    /** Where a cue with $number goes (the index of that number when it exists) */
    int insertIndexFor(int number) const;

    /** Adds a cue in its place: -1 when its number is taken or not valid.
     *  A cue with uid 0 (or a uid of the list) gets a new one. Returns the index */
    int addCue(const Cue &cue);
    /** Replaces the cue with the same uid; its number may change (it moves,
     *  and must stay unique) */
    bool replaceCue(const Cue &cue);
    bool removeCue(quint32 uid);
    /** Replaces every cue (import, remap): sorted, duplicate numbers dropped */
    void setCues(const QList<Cue> &cues);

    /** What cue $index is applied on: the tracked cues before it */
    State stateBefore(int index) const;
    /** What the stage shows on cue $index (empty for -1 or out of range) */
    State trackedState(int index) const;

    /** A palette changed: every value that refers to $paletteId takes
     *  valueOf(fixture, channel); -1 drops the reference and keeps the value */
    void applyPalette(int paletteId, const std::function<int(quint32, quint32)> &valueOf);
    /** A palette was deleted: its values stay, without the reference */
    void forgetPalette(int paletteId);

private:
    int indexOfUidLocked(quint32 uid) const;
    int indexOfNumberLocked(int number) const;
    int insertIndexLocked(int number) const;
    State foldLocked(int index, bool withCue) const;
    /** Removes the keys and FX of the fixtures $remove() says, from every cue */
    bool removeFixturesLocked(const std::function<bool(quint32)> &remove);
    /** Sorts the cues, drops invalid and duplicate numbers, gives new uids
     *  to cues with none or a repeated one (m_nextUid ends past every uid) */
    void normalizeLocked(QList<Cue> &cues);
    /** Keeps a cue consistent: values win over releases, FX over FX releases,
     *  palettes only on values, layers with fixtures */
    static void sanitize(Cue &cue);

    /*********************************************************************
     * Options (saved)
     *********************************************************************/
public:
    /** Off: each cue alone, everything else released. Default on */
    bool tracking() const;
    void setTracking(bool on);

    AtEnd atEnd() const;
    void setAtEnd(AtEnd mode);

    /** A new start begins at the first cue (default), or at the cue after
     *  the one it was released on */
    bool resetOnRelease() const;
    void setResetOnRelease(bool on);

    /** ms for BACK; -1 (default): the times of the cue BACK goes to */
    int backTime() const;
    void setBackTime(int ms);

    /** A GO on another playback releases this one (applied by the playbacks) */
    bool releaseOnNextGo() const;
    void setReleaseOnNextGo(bool on);

    /* Reused Function fields: fadeInSpeed() = the fade of new cues,
     * fadeOutSpeed() = the release time, priority, drive mode (BeatGO and
     * Pulse give a GO per division) */

    /*********************************************************************
     * Transport. Any thread: queued, and applied by write() in the timer thread.
     * GO, back and goToCue do nothing on a stopped list; right after start()
     * they wait for its first frame, after the start cue
     *********************************************************************/
public:
    /** Paused: only resumes (GO resumes a paused fade) */
    void go();
    /** Paused: resumes and goes back */
    void back();
    /** Paused: resumes and goes to the cue. $snap: no fade */
    void goToCue(int index, bool snap = false);
    /** Pre-selects the cue of the next GO; -1 clears it */
    void setNextCue(int index);
    /** Where the next start() begins; survives a release, used by the next start */
    void setStartCue(int index);
    /** The published state: written by every frame of a running list (and by
     *  setNextCue / setStartCue), so a paused or stopped list keeps its last one */
    Status status() const;
    /** What the next GO (or start) plays, from the current data: the start
     *  cue of a stopped list (also right after stop(), before its postRun),
     *  the pre-selected one of a paused list, and right after edits; -1 when
     *  nothing */
    int nextIndex() const;
    /** The channels changed by GOs since the last call (programmer live edit) */
    QSet<Key> takeChangedKeys();

    /*********************************************************************
     * Fixtures
     *********************************************************************/
public slots:
    void slotFixtureRemoved(quint32 fxi_id) override;

    /*********************************************************************
     * Save & Load
     *********************************************************************/
public:
    bool saveXML(QXmlStreamWriter *doc) const override;
    bool loadXML(QXmlStreamReader &root) override;
    /** Drops the keys of fixtures that are not in the show */
    void postLoad() override;

    static QString triggerToString(Trigger trigger);
    static Trigger stringToTrigger(const QString &str);
    static QString atEndToString(AtEnd mode);
    static AtEnd stringToAtEnd(const QString &str);

private:
    static void saveCueXML(QXmlStreamWriter *doc, const Cue &cue);
    static bool loadCueXML(QXmlStreamReader &root, Cue &cue);

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    void preRun(MasterTimer *timer) override;
    void write(MasterTimer *timer, QList<Universe *> universes) override;
    void postRun(MasterTimer *timer, QList<Universe *> universes) override;

protected:
    /** The faders, and the FX player through its parent priority */
    void updateFadersPriority() override;

private:
    enum ActionType { ActionGo = 0, ActionBack, ActionGoTo };
    struct Action
    {
        ActionType type = ActionGo;
        quint32 uid = 0;
        bool snap = false;
    };
    /** Normal: a GO, with the cue's times. LiveReapply: the data changed, snap to it */
    enum ApplyMode { ApplyNormal = 0, ApplyLiveReapply };

    /* the runtime helpers: timer thread, with m_mutex locked */
    QSharedPointer<GenericFader> getFader(Universe *universe);
    /** Where a run begins: the start cue, else the cue after the one a
     *  release left (without resetOnRelease), else the first; -1 when empty */
    int startIndexLocked() const;
    /** The same, resuming after $lastUid instead of m_lastUid */
    int startIndexLocked(quint32 lastUid) const;
    /** What the next GO (or Follow/Wait) plays: the pre-selected cue, else
     *  the next one (past the end: the first with Loop, else -1) */
    int naturalNextLocked() const;
    /** The cue whose Follow or Wait takes the list on by itself from the
     *  current cue, $next being naturalNextLocked(): that cue, or past the
     *  end of a Release list the first cue (its trigger releases the list);
     *  nullptr when only a GO goes on */
    const Cue *autoTriggerLocked(int next) const;
    /** The cues changed: the current cue keeps its index right (a paused
     *  list has no frame to do it), and the status shows it. A deleted
     *  current cue gives way to the one before it, as reapplyLocked plays */
    void followEditsLocked();
    /** The cue that plays $offset virtual ms after the start, following the
     *  Follow/Wait triggers, and how far into it. $moved: the chain went past
     *  the first cue (a trigger, or a whole loop cycle, came before $offset) */
    int seekLocked(quint64 offset, quint64 *remainder, bool *moved) const;
    /** One GO, BACK or jump; true when it applied a cue */
    bool processActionLocked(const Action &action, const QList<Universe *> &universes);
    /** The cues changed while playing: the current cue again, at once */
    void reapplyLocked(const QList<Universe *> &universes);
    /** Makes the faders show trackedState($index) ($index -1: nothing):
     *  only the channels whose target changed are touched */
    void applyCueLocked(const QList<Universe *> &universes, int index,
                        uint fade, uint down, uint delay, ApplyMode mode);
    /** The channels FX $layers move, each with the layer that moves it: the
     *  channel FXScene finds for each axis and its fine channel, and -1 for
     *  the other byte of a coarse/fine pair that the FX doesn't name */
    QHash<Key, int> fxChannelsOf(const QList<FXScene::Layer> &layers) const;
    /** Makes the FX player play $layers around $bases: started with $fadeMs,
     *  updated at once while it runs, stopped with $fadeMs when none are left */
    void playFxLocked(const QList<Universe *> &universes, const QList<FXScene::Layer> &layers,
                      const QHash<Key, uchar> &bases, const QHash<Key, int> &channels,
                      uint fadeMs, qreal level, ApplyMode mode);
    /** Publishes m_status (also from the setters, in any thread: it only reads
     *  members guarded by m_mutex) */
    void updateStatusLocked();
    /** FadeChannels run in real ms: a cue time at the live speed */
    uint scaledTime(uint ms) const;
    /** The coarse or fine channel GenericFader keeps with $channel in one
     *  FadeChannel, or QLCChannel::invalid() */
    static quint32 pairedChannel(const Fixture *fixture, quint32 channel);
    /** A dimmer (group Intensity without a colour): down fade goes for it */
    static bool isDimmer(const Fixture *fixture, quint32 channel);

    /* the data, guarded by m_mutex */
    mutable QMutex m_mutex;
    QList<Cue> m_cues;
    quint32 m_nextUid;
    bool m_tracking;
    AtEnd m_atEnd;
    bool m_resetOnRelease;
    int m_backTime;
    bool m_releaseOnNextGo;

    /* the playback, guarded by m_mutex */
    QList<Action> m_actions;
    quint32 m_preselectUid;
    /** survives postRun: where the next start begins */
    quint32 m_startUid;
    /** the cue a release left, to resume from */
    quint32 m_lastUid;
    quint32 m_currentUid;
    int m_current;
    /** the static targets our faders hold, before intensity */
    QHash<Key, uchar> m_playing;
    /** drained by takeChangedKeys() */
    QSet<Key> m_changed;
    /** virtual ms (frameTick() includes the live speed) */
    uint m_cueElapsed;
    /** the cue's own length: Follow counts from it, as totalDuration() and a seek */
    uint m_cueLength;
    /** when the fades the cue runs with end (in m_cueElapsed time): the status */
    uint m_fadeLength;
    /** a super-scene seek: how far into its cue the first cue starts */
    uint m_seekRemainder;
    bool m_followArmed;
    bool m_firstOfRun;
    /** cues edited, a palette changed, a fixture removed: play the new data */
    bool m_dataDirty;
    bool m_releaseRequested;
    /** uids to announce (MasterTimer::cueStarted) once unlocked */
    QList<quint32> m_notify;

    /* timer thread only */
    bool m_fadersPaused;
    qreal m_appliedIntensity;

    /** plays the tracked FX layers; the static values of FX channels are its
     *  bases. It is not in the Doc nor in the MasterTimer: it runs inside our
     *  own preRun/write/postRun (timer thread), so our release stops it in
     *  the same frame and nothing else ever holds it. Our live speed and
     *  pause reach it there too */
    FXScene *m_fxPlayer;
    /* what the player plays, guarded by m_mutex (it is empty when it doesn't run) */
    QList<FXScene::Layer> m_fxPlaying;
    QHash<Key, uchar> m_fxBases;
    /** the channels the player sends, and their layer in m_fxPlaying (see fxChannelsOf) */
    QHash<Key, int> m_fxChannels;

    Status m_status;
    mutable QMutex m_statusMutex;
};

/** @} */

#endif
