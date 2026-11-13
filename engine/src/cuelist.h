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
#include <atomic>
#include <QRandomGenerator>
#include <QStringList>
#include <QMutex>
#include <QList>
#include <QHash>
#include <QMap>
#include <QPair>
#include <QSet>

#include "function.h"
#include "fxscene.h"

class QXmlStreamAttributes;
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
#define KXMLDTCCueTime          QStringLiteral("Time")
#define KXMLDTCCueAction        QStringLiteral("Action")

class QLCChannel;

/**
 * A cuelist: numbered cues that hold only what they
 * change. What the stage shows on a cue is the fold of the cues before it
 * (tracking): a value carries on until another cue changes it or releases it,
 * and a released channel goes back to what's below. A cue can also start,
 * change or stop FX layers, tracked per (axis, fixture). DMX Total Control:
 * a cue can link to another cue, the one a GO plays after it (a number of
 * times, or always), and hold actions the app does when it starts.
 *
 * The list plays with one persistent fader per universe and, on each GO,
 * changes only the channels whose tracked target changed: no dips, and the
 * tracked channels are not taken back from the other playbacks. The tracked
 * FX layers play in a private FXScene, with the static values of their
 * channels as bases: the faders of the list never hold an FX channel.
 *
 * DMX Total Control: the last change wins. When another playback of the same
 * priority wrote after our fader (it started, or its GO changed something),
 * the channels a GO changes go into a new fader above it, and the older one
 * keeps the channels the list only tracks: an LTP channel follows the last
 * GO that changed it. Each channel is in one fader of the list at a time.
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
    /** How the list plays. Normal: GO plays the next cue. Chase: it plays its
     *  cues by itself, round and round, a step a beat of its speed, in its
     *  direction (its triggers, its links and AtEnd don't count; GO and BACK
     *  take a step now). Submaster: the fader of its playback rules it
     *  (applied by the playbacks: up from zero starts it, down to zero
     *  releases it, and GO on it stopped only chooses the cue it starts on);
     *  in a group or on a SHOW button it plays as a Normal list. Override:
     *  its level mixes everything it holds with what's below (0: what's
     *  below, full: its values replace it), from its next start. Timecode:
     *  each cue plays at its time on a clock (the last cue in order whose
     *  time has come; its triggers and its links don't count); a run starts
     *  the clock (a super-scene block at the time it starts in: the
     *  timeline is the clock), and GO, BACK and a jump set it to the time
     *  of their cue */
    enum Mode { ModeNormal = 0, ModeChase, ModeTimecode, ModeSubmaster, ModeOverride };
    /** DMX Total Control, Timecode: the clock of the list. Internal: its own
     *  (frames at its speeds). Mtc: a MIDI timecode (MasterTimer::timecode),
     *  less the offset, also in a super-scene block: GO, BACK and jumps do
     *  nothing, a run waits for the timecode, and a timecode that stops
     *  holds the cue */
    enum TcSource { TcInternal = 0, TcMtc };
    /** The order a Chase plays its cues in. Bounce turns at the ends; Random
     *  never plays the same step twice in a row */
    enum ChaseDirection { ChaseForward = 0, ChaseBackward, ChaseBounce, ChaseRandom };

    /** The attributes a cue can give times of their own, as the palettes */
    enum TimeGroup { TimeIntensity = 0, TimeColour, TimePosition, TimeBeam, TimeGroupCount };

    /** The times of an attribute or a fixture in a cue, in ms; -1: the cue's */
    struct PartTime
    {
        int fade = -1;
        int delay = -1;
        /** a FadeChannel::Curve; -1: the cue's */
        int curve = -1;

        bool isEmpty() const { return fade < 0 && delay < 0 && curve < 0; }
        bool operator==(const PartTime &other) const
        {
            return fade == other.fade && delay == other.delay && curve == other.curve;
        }
        bool operator!=(const PartTime &other) const { return !(*this == other); }
    };

    /** What a cue does when it starts (GO of another cuelist, a scene, the
     *  BPM...): the app knows them, the engine only keeps them, as the
     *  palettes. Attributes of their own, "target" names what it does */
    typedef QMap<QString, QString> CueAction;

    /** What a channel takes on a GO, in ms */
    struct Times
    {
        uint fade = 0;
        /** dimmers going down */
        uint down = 0;
        uint delay = 0;
        /** a FadeChannel::Curve */
        int curve = 0;
    };

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
        /** how its fades move: a FadeChannel::Curve (its releases too) */
        int curve = 0;
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
        /** times of their own of the attributes (TimeGroup) */
        QMap<int, PartTime> groupTimes;
        /** times of their own of fixtures: they win over the attributes */
        QMap<quint32, PartTime> fixtureTimes;
        /** the uid of the cue a GO (or its trigger) plays after this one;
         *  0: the next cue. A uid the list doesn't have is dropped */
        quint32 link = 0;
        /** how many times the link is taken before the list goes on past
         *  this cue (0..999); 0: always */
        int linkCount = 0;
        /** what it does when it starts, in order (MasterTimer::cueActions) */
        QList<CueAction> actions;
        /** DMX Total Control: Move in black doesn't prepare it: its moves
         *  are seen at its GO */
        bool noMib = false;
        /** DMX Total Control, Timecode: when it plays on the clock (ms, up
         *  to maxCueTime()); -1: never by itself */
        int time = -1;

        uint effectiveDown() const { return downFade < 0 ? fade : uint(downFade); }
        bool hasOwnTimes() const { return groupTimes.isEmpty() == false || fixtureTimes.isEmpty() == false; }
        /** The times of the channels of $fixture in $group (-1: no group).
         *  Fade, delay and curve: the fixture's, else the attribute's, else
         *  the cue's. Down: the fixture's fade, else the cue's down, else
         *  the fade of the attribute or of the cue */
        Times timesFor(quint32 fixture, int group) const;
        /** Its longest time: the delay and fade of every attribute and fixture
         *  part counts, also of an attribute the cue doesn't change */
        uint length() const;
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
        /** next is the link of the current cue: the jump it makes (from 1),
         *  0 when it isn't */
        int linkJump = 0;
        /** the jumps that link makes (0: always) */
        int linkCount = 0;
        /** the time of the fades the current cue runs with, from its GO (its
         *  delay and fade, with a Live Time change), and how far they are: in
         *  cue ms (at x1; the speeds make them pass faster or slower) */
        uint fadeTotal = 0;
        uint fadeElapsed = 0;
        /** what Live Time did to them in this run (1: the times they started with) */
        qreal liveScale = 1.0;
        /** DMX Total Control: the fades run with the cue's own times (not a
         *  BACK time, a group or block fade-in or a Chase change): Live Time
         *  can be stored in it */
        bool ownTimes = true;
        /** a hand crossfade runs (the A/B fader): progress is where the
         *  hand has its fades */
        bool manual = false;
        /** DMX Total Control, Timecode: the time of its clock (virtual ms);
         *  -1 when it doesn't run, and for the other types */
        qint64 clock = -1;
    };

    /** How a hand crossfade ends: at its cue, back on the cue before, or
     *  going on by itself from where it is (in its own time) */
    enum ManualEnd { ManualComplete = 0, ManualCancel, ManualOnClock };

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
    /** DMX Total Control: the latest time of a cue, under 100 hours (ms) */
    static int maxCueTime() { return 359999999; }

    /** The slots of an FX parameter: "pan" and "tilt" for "pantilt", else the param */
    static QStringList fxAxes(const QString &param);
    /** Every (axis, fixture) slot a layer covers */
    static QSet<FxKey> fxKeysOf(const FXScene::Layer &layer);

    /** The TimeGroup of a channel, or -1 (it takes the cue's times) */
    static int timeGroupOf(const QLCChannel *channel);
    /** "intensity", "colour", "position", "beam" (as the palettes); "" for -1 */
    static QString timeGroupToString(int group);
    /** -1 when it is none of them */
    static int stringToTimeGroup(const QString &str);

    /** "linear", "smooth", "slowstart", "faststart" (a FadeChannel::Curve) */
    static QString curveToString(int curve);
    /** Anything it doesn't know is linear */
    static int stringToCurve(const QString &str);

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
    /** trackedState() of every cue, in one pass; $cues gets the cues they
     *  are of (the list may change after the call) */
    QList<State> trackedStates(QList<Cue> *cues = nullptr) const;

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
     *  palettes only on values, layers with fixtures, link counts in range,
     *  actions with a target */
    static void sanitize(Cue &cue);
    /** The links to uids that are not in $cues go. @return whether one went */
    static bool pruneLinks(QList<Cue> &cues);

    /*********************************************************************
     * Options (saved)
     *********************************************************************/
public:
    /** Default Normal */
    Mode mode() const;
    void setMode(Mode mode);

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

    /** Move in black (default off): $mibDelay ms after the fades of a cue
     *  end, the fixtures that are dark and that the next cue lights up take
     *  its other attributes in $mibFade ms (default 2 s), so at its GO they
     *  only fade up. A cue with noMib is not prepared */
    bool moveInBlack() const;
    void setMoveInBlack(bool on);
    uint mibDelay() const;
    void setMibDelay(uint ms);
    uint mibFade() const;
    void setMibFade(uint ms);
    static uint defaultMibFade() { return 2000; }

    /** DMX Total Control, Chase: a step a beat of its own BPM (10..600,
     *  default 120) or of the show tempo, by the multiplier (0.25, 0.5, 1, 2
     *  or 4; another one takes the nearest). A tapped rhythm wins over them;
     *  BeatGO and Pulse step it on their divisions instead */
    int chaseBpm() const;
    void setChaseBpm(int bpm);
    static int defaultChaseBpm() { return 120; }
    bool chaseShowTempo() const;
    void setChaseShowTempo(bool on);
    qreal chaseMultiplier() const;
    void setChaseMultiplier(qreal factor);
    /** The change of a step: 0..100 % of the step for everything, with no
     *  delay (default 0: a cut), or -1: the times of each cue */
    int chaseXfade() const;
    void setChaseXfade(int percent);
    ChaseDirection chaseDirection() const;
    void setChaseDirection(ChaseDirection direction);
    /** The steps a minute a Chase plays at (at x1: the speeds make it go
     *  faster or slower): the tapped rhythm, else its BPM or the show tempo
     *  by the multiplier. 0 for the other modes */
    qreal effectiveChaseBpm() const;

    /** DMX Total Control, Timecode: its clock (default its own) and, for
     *  a MIDI timecode, the time of it where the list's 00:00 is (ms) */
    TcSource tcSource() const;
    void setTcSource(TcSource source);
    uint tcOffset() const;
    void setTcOffset(uint ms);

    /* Reused Function fields: fadeInSpeed() = the fade of new cues,
     * fadeOutSpeed() = the release time, priority, drive mode (BeatGO and
     * Pulse give a GO per division) */

    /*********************************************************************
     * Transport. Any thread: queued, and applied by write() in the timer thread.
     * GO, back and goToCue do nothing on a stopped list; right after start()
     * they wait for its first frame, after the start cue. DMX Total
     * Control, Timecode: the clock goes to the time of their cue (a cue
     * without one leaves it going); GO plays it with its times, back and
     * goToCue snap (back with a BACK time takes it)
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
    /** DMX Total Control: the FX layers the list plays now (empty when none);
     *  the programmer reads which of them dim fixtures without a dimmer */
    QList<FXScene::Layer> playingFx() const;

    /** Live Time, while a cue fades (else nothing): its fades go on from
     *  where they are and end $ms after its GO (its delay and fade, as
     *  Status::fadeTotal); a Follow after it waits for that end. 0 ends them
     *  now. For this run only: the next cue has its own times again */
    void setLiveTime(uint ms);
    /** The same, the time of the fades by $factor (0.5: they end in half the time) */
    void scaleLiveTime(qreal factor);
    /** The times of the cue were made the ones it runs with: Status::liveScale is 1 */
    void liveTimeStored();

    /** The speed its cue times go at: its live speed and the cue speed of
     *  the MasterTimer (2: twice as fast) */
    qreal timeFactor() const;

    /** DMX Total Control, the A/B fader: a GO whose fade the hand makes.
     *  The next cue starts at once, and its fades (delays and down fades
     *  too, in proportion; a cue with no time over 1 s) are where
     *  setManualPosition() puts them, until endManual(). Its FX start in
     *  their own time. A GO meanwhile lets it go on by itself, and Follow
     *  waits for its end. Nothing on a stopped list (setStartManual); a
     *  Chase plays a plain GO */
    void manualGo();
    /** The next start() begins with a manualGo() on the cue it starts on */
    void setStartManual(bool on);
    /** How far the hand crossfade is: 0 (where it started) to 1 (the cue) */
    void setManualPosition(qreal position);
    /** The hand crossfade ends. Cancelled, the cue before is back (a list
     *  the crossfade started is released, and its next start begins on that
     *  cue again) */
    void endManual(ManualEnd how);

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
    /** "Normal", "Chase", "Timecode", "Submaster", "Override" */
    static QString modeToString(Mode mode);
    /** Anything it doesn't know is Normal */
    static Mode stringToMode(const QString &str);
    /** "Forward", "Backward", "Bounce", "Random" */
    static QString chaseDirectionToString(ChaseDirection direction);
    /** Anything it doesn't know is Forward */
    static ChaseDirection stringToChaseDirection(const QString &str);
    /** ×¼ ×½ ×1 ×2 ×4: the nearest one; what isn't a factor is ×1 */
    static qreal chaseMultiplierOf(qreal factor);
    /** "Internal", "Mtc"; anything it doesn't know is Internal */
    static QString tcSourceToString(TcSource source);
    static TcSource stringToTcSource(const QString &str);

private:
    static void saveCueXML(QXmlStreamWriter *doc, const Cue &cue);
    static bool loadCueXML(QXmlStreamReader &root, Cue &cue);
    /** The attributes of a <Time>: of an attribute (Attr) or a fixture (F) */
    static void loadCueTimeXML(const QXmlStreamAttributes &attrs, Cue &cue);

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
    /** The motion speed goes to its FX only, not to the times of its cues.
     *  DMX Total Control: the cue speed of the MasterTimer goes to its times
     *  (frameTick: the cue timers, its own rhythm), not to its FX */
    qreal frameMotion() const override;

private:
    enum ActionType { ActionGo = 0, ActionBack, ActionGoTo, ActionLiveTime, ActionLiveScale,
                      ActionManualGo, ActionManualEnd, ActionTcSync };
    struct Action
    {
        ActionType type = ActionGo;
        /** GoTo: the cue. ManualGo: the start cue of a run (0: as a GO).
         *  TcSync: the cue the run starts on (0: the one of the clock) */
        quint32 uid = 0;
        /** GoTo: no fade. TcSync: a super-scene seek */
        bool snap = false;
        /** the cue it plays runs its actions (not a BACK, nor a seek) */
        bool actions = true;
        /** Live Time: the new time (ms), or the factor of the time */
        uint ms = 0;
        qreal scale = 1.0;
        /** ManualEnd: how */
        ManualEnd end = ManualComplete;
    };
    /** Normal: a GO, with the cue's times. LiveReapply: the data changed,
     *  snap to it. Manual: a GO whose fades the hand sets (the A/B fader) */
    enum ApplyMode { ApplyNormal = 0, ApplyLiveReapply, ApplyManual };

    /* the runtime helpers: timer thread, with m_mutex locked */
    QSharedPointer<GenericFader> getFader(Universe *universe);
    /** DMX Total Control: the fader the channels a GO changes go into: the
     *  newest one of $universe, or a new one above it when another playback
     *  of our priority came after it (the old one joins m_olderFaders) */
    QSharedPointer<GenericFader> topFaderLocked(Universe *universe);
    /** DMX Total Control: the FadeChannel of $key in an older fader of ours
     *  moves into $top (both bytes of a coarse/fine pair). An HTP channel
     *  keeps its running fade; an LTP one starts from what the stage shows,
     *  and then true is returned */
    bool takeChannelLocked(Universe *universe, const QSharedPointer<GenericFader> &top,
                           const Fixture *fixture, const Key &key);
    /** DMX Total Control: every fader of ours, the older ones first */
    QList<QSharedPointer<GenericFader>> allFaders() const;
    /** DMX Total Control: the older faders that hold no channel any more go */
    void dropEmptyFaders();
    /** Where a run begins: the start cue, else the cue after the one a
     *  release left (without resetOnRelease), else the first; -1 when empty */
    int startIndexLocked() const;
    /** The same, resuming after $lastUid instead of m_lastUid */
    int startIndexLocked(quint32 lastUid) const;
    /** What the next GO (or Follow/Wait) plays: the pre-selected cue, else
     *  the link of the current cue while it has jumps left ($viaLink), else
     *  the next one (past the end: the first with Loop, else -1) */
    int nextLocked(bool *viaLink) const;
    /** Where the list goes from cue $from with the link counts $taken: its
     *  link while it has jumps left ($viaLink), else the next cue (past the
     *  end: the first with Loop, else -1) */
    int linkNextLocked(int from, const QHash<quint32, int> &taken, bool *viaLink) const;
    /** The step from cue $from in $taken: a jump of its link counts, going
     *  on past its link starts its count again */
    static void countLink(const Cue &from, bool viaLink, QHash<quint32, int> &taken);
    /** The same on the counts of the run */
    void takeLinkLocked(int from, bool viaLink);
    /** The cue whose Follow or Wait takes the list on by itself from the
     *  current cue, $next being nextLocked(): that cue, or past the
     *  end of a Release list the first cue (its trigger releases the list);
     *  nullptr when only a GO goes on */
    const Cue *autoTriggerLocked(int next) const;
    /** The time from cue $from to cue $to, by the trigger of $to; false when
     *  $to waits for a GO */
    bool gapLocked(int from, int to, quint64 *ms) const;
    /** How the chain of Follow and Wait triggers from the first cue ends */
    enum ChainEnd { ChainGo = 0, ChainLast, ChainCycle, ChainEndless };
    /** Follows the chain from the first cue, its links (and their counts)
     *  too, adding the times between the cues to $ms: up to a cue that waits
     *  for a GO, past the end ($last: the cue it ended on), back on the first
     *  cue with no counts pending, or a link that loops for ever */
    ChainEnd walkLocked(quint64 *ms, int *last) const;
    /** The cues changed: the current cue keeps its index right (a paused
     *  list has no frame to do it), and the status shows it. A deleted
     *  current cue gives way to the one before it, as reapplyLocked plays */
    void followEditsLocked();
    /** The cue that plays $offset virtual ms after the start, following the
     *  Follow/Wait triggers and the links, and how far into it. $moved: the
     *  chain went past the first cue (a trigger, or a whole loop cycle, came
     *  before $offset). $taken: the link counts there */
    int seekLocked(quint64 offset, quint64 *remainder, bool *moved, QHash<quint32, int> *taken) const;
    /** One GO, BACK or jump; true when it applied a cue */
    bool processActionLocked(const Action &action, const QList<Universe *> &universes);
    /** DMX Total Control, Chase: the ms of a step of its own speed (cue ms):
     *  of its BPM or the show tempo, by the multiplier; 20 at least */
    uint chasePeriodLocked() const;
    /** The same, or the tapped rhythm when it has one: the step that plays */
    uint chaseBeatLocked() const;
    /** The steps of a lap: the cues, there and back for a Bounce */
    int chaseLapLocked() const;
    /** The step $way (1: the next one, -1: back) from cue $from, by the
     *  direction. Bounce: $bounce is the way it goes (1 or -1), turned at the
     *  ends. Random: the step drawn for after the current one, -1 when there
     *  is none */
    int chaseNextLocked(int from, int way, int *bounce) const;
    /** The step a GO ($way 1) or a BACK (-1) plays now: a Bounce turns,
     *  Random draws one when none was drawn */
    int takeChaseStepLocked(int way);
    /** Random: a step other than $from (the only one, with one cue) */
    int drawChaseStepLocked(int from);
    /** Its own speed: a step once its time has gone by (not with a tapped
     *  rhythm nor BeatGO/Pulse: their beats are GOs); true when it applied one */
    bool chaseTickLocked(const QList<Universe *> &universes);
    /** The step that plays $offset virtual ms after the start, from the first
     *  cue, how far into it ($remainder), and the way a Bounce goes there.
     *  $moved: a step came before $offset */
    int chaseSeekLocked(quint64 offset, quint64 *remainder, bool *moved, int *bounce) const;
    /** DMX Total Control, Timecode: the cue the clock plays at $clock: the
     *  last one in order with a time, and its time not after $clock; -1
     *  before them */
    int tcPickLocked(qint64 clock) const;
    /** The first cue the clock plays (at the earliest time); -1 when no cue has a time */
    int tcFirstLocked() const;
    /** The time of cue $index, else of the last cue before it with one, else 0 */
    qint64 tcTimeLocked(int index) const;
    /** GO, BACK, a jump or the start of a run on a Timecode list: the clock
     *  goes to the time of its cue (the start keeps it); true when it
     *  applied a cue */
    bool tcActionLocked(const Action &action, const QList<Universe *> &universes);
    /** The clock went on: the cue whose time has come, when it is another
     *  one. One step on with its times, else snapped; true when it applied one */
    bool tcTickLocked(const QList<Universe *> &universes);
    /** Cue $index with its times (a group or a block fade-in for the first cue of a run) */
    void playCueLocked(const QList<Universe *> &universes, int index, bool withActions);
    /** Live Time on the cue that fades: its time by $factor; 0 ends it now */
    void liveTimeLocked(qreal factor);
    /** The fades that run in our faders take $factor times as long from
     *  where they are (the same place on their way, never back): the delay,
     *  the fade and the time they ran, as if they had started with them.
     *  DMX Total Control: $manual: only the ones of a hand crossfade, else
     *  only the others */
    void scaleFadesLocked(qreal factor, bool manual);
    /** DMX Total Control, the A/B fader: the channels of the hand crossfade
     *  at $ms of their cue (where the position puts them) */
    void manualTimeLocked(uint ms);
    /** They go on by themselves from where they are, at the cue speed */
    void manualToClockLocked();
    /** The hand crossfade ends ($how); true when it applied a cue (cancelled) */
    bool endManualLocked(ManualEnd how, const QList<Universe *> &universes);
    /** The cues changed while playing: the current cue again, at once */
    void reapplyLocked(const QList<Universe *> &universes);
    /** Makes the faders show trackedState($index) ($index -1: nothing):
     *  only the channels whose target changed are touched. $own: the cue
     *  whose times each channel takes (its attribute and fixture times);
     *  nullptr: $fade, $down and $delay for all of them. $withActions: the
     *  cue's actions are announced (MasterTimer::cueActions) */
    void applyCueLocked(const QList<Universe *> &universes, int index,
                        uint fade, uint down, uint delay, ApplyMode mode, const Cue *own,
                        bool withActions);
    /** The new and changed targets of $values (of the keys in $only, when it
     *  isn't nullptr) go into our faders, each channel with the times
     *  $timesOf gives it (in real ms). A channel whose target we already
     *  hold is not touched: a fade that runs goes on, and nothing is taken
     *  back from the other playbacks (no dips). $live: the data changed,
     *  snapped, and the programmer is not told (m_changed). DMX Total
     *  Control, $manual: the hand crossfade sets their time (Manual) */
    void setTargetsLocked(const QList<Universe *> &universes, const QHash<Key, uchar> &values,
                          const QSet<Key> *only, const std::function<Times(const Fixture *, quint32)> &timesOf,
                          bool live, bool manual);
    /** DMX Total Control, Move in black: the fades of the current cue have
     *  ended (and the delay): the cue a GO plays next is prepared, once */
    void moveInBlackLocked(const QList<Universe *> &universes);
    /** The fixtures of cue $next that are dark now and that it lights up
     *  take its other attributes (not the channels of an FX, of this cue or
     *  of that one: the FX start at its GO) */
    void presetLocked(const QList<Universe *> &universes, int next);
    /** Whether $fixture's $light channels are dark: this cue doesn't light
     *  them (nor an FX of it, even at the bottom of its wave) and the stage
     *  shows them at 0 (before the grand master: another playback can light it) */
    bool isDarkLocked(const QList<Universe *> &universes, const Fixture *fixture,
                      const QList<quint32> &light) const;
    /** The light of a fixture: its dimmers, or its intensities (RGB...) when it has none */
    static QList<quint32> lightChannels(const Fixture *fixture);
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
    /** FadeChannels run in real ms: a cue time at the live speed (and,
     *  DMX Total Control, the cue speed: timeFactor) */
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
    bool m_moveInBlack;
    uint m_mibDelay;
    uint m_mibFade;
    Mode m_mode;
    int m_chaseBpm;
    bool m_chaseShowTempo;
    qreal m_chaseMultiplier;
    int m_chaseXfade;
    ChaseDirection m_chaseDirection;
    TcSource m_tcSource;
    uint m_tcOffset;

    /* the playback, guarded by m_mutex */
    /** the type the run plays as (preRun takes it: a new type plays from
     *  the next start). Override: its values mix with what's below */
    Mode m_runMode;
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
    /** virtual ms (frameTick() includes the live speed and the cue speed) */
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
    /** the cues whose actions are announced (MasterTimer::cueActions) once unlocked */
    QList<quint32> m_actionNotify;
    /** the jumps each link took in this pass (by the uid of its cue) */
    QHash<quint32, int> m_linkTaken;
    /** what Live Time did to the times of the current cue (Status::liveScale) */
    qreal m_liveScale;
    /** the current cue runs with its own times (Status::ownTimes) */
    bool m_ownTimes;
    /** Move in black: the uid of the cue it prepared since the last GO (0: none) */
    quint32 m_mibUid;
    /** Chase: virtual ms since its step began (its own speed counts them),
     *  the way a Bounce goes, and the uid of the step Random drew for next */
    uint m_chaseElapsed;
    int m_bounceDir;
    quint32 m_randomUid;
    QRandomGenerator m_random;
    /** DMX Total Control, the A/B fader: a hand crossfade runs; the next
     *  start begins with one; its length (cue ms); what a cancel brings back
     *  (the cue before, 0: none; the link counts; the cue chosen for the GO) */
    bool m_manual;
    bool m_startManual;
    uint m_manualLength;
    quint32 m_manualPrevUid;
    QHash<quint32, int> m_manualLinks;
    quint32 m_manualPreselect;
    /** where the hand has it, 0..1: set from any thread, read by write() */
    std::atomic<double> m_manualPosition;
    /** DMX Total Control, Timecode: the clock (virtual ms: frameTick()
     *  includes the live speed and the cue speed), where it was when the cue
     *  of the clock was last looked at, and the uid of that cue (0: none) */
    qint64 m_tcClock;
    qint64 m_tcLastClock;
    quint32 m_tcUid;
    /** a MIDI timecode came in for this frame (the clock follows it) */
    bool m_tcLive;

    /* timer thread only */
    bool m_fadersPaused;
    qreal m_appliedIntensity;
    /** the timeFactor() the fades in our faders run at: a new one rescales them */
    qreal m_appliedFactor;
    /** DMX Total Control: our faders that another playback of our priority
     *  passed, oldest first, with the id of their universe (m_fadersMap has
     *  the newest one of each universe) */
    QList<QPair<quint32, QSharedPointer<GenericFader>>> m_olderFaders;
    /** DMX Total Control: the fader that holds each channel (both bytes of
     *  a pair); none: the newest one */
    QHash<Key, QSharedPointer<GenericFader>> m_holder;

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
