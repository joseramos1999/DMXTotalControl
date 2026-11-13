/*
  DMX Total Control
  playbackmanager.h

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

#ifndef PLAYBACKMANAGER_H
#define PLAYBACKMANAGER_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QHash>
#include <QList>
#include <QMap>
#include <QSet>
#include <QElapsedTimer>

#include <tuple>
#include <functional>

#include "functionparent.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class CuelistEditor;
class AppSettings;
class Cuelist;
class Programmer;
class Function;
class QTimer;
class Doc;

#define KXMLDTCPlaybacks QStringLiteral("Playbacks")

/**
 * The playbacks: pages of slots, each one holding a
 * cuelist or any other scene, with GO, Pause/Back, Flash, Release, Select
 * and a fader that is its intensity master. Main GO acts on the selected
 * playback. The Playbacks page, the SHOW, the phone remote and the mappings
 * are thin adapters over this class.
 *
 * Everything takes (page, slot), with page -1 for the current page. What
 * plays is per function: the same function on two slots has
 * one state and one level. Playbacks start functions with their own parent
 * (ManualVCWidget), so a release from a playback really releases the
 * function, even when a group started it too, and the other way round.
 *
 * Pages, slots and levels are part of the show (DMX Total Control section);
 * flashes and the selection are not.
 */
class PlaybackManager : public QObject
{
    Q_OBJECT

    /** Bumped on every discrete change (state, cue, level, assignment, page) */
    Q_PROPERTY(int revision READ revision NOTIFY stateChanged)
    /** Bumped when the progress of a running cue moves (by 5 %) */
    Q_PROPERTY(int progressRevision READ progressRevision NOTIFY progressChanged)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY pageChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pagesChanged)
    Q_PROPERTY(QStringList pageNames READ pageNames NOTIFY pagesChanged)
    /** How many slots a page shows, 4 - 20 (10 by default) */
    Q_PROPERTY(int slotsPerPage READ slotsPerPage WRITE setSlotsPerPage NOTIFY pagesChanged)
    /** The selected playback: { page, slot, functionId }, or {} */
    Q_PROPERTY(QVariantMap selected READ selected NOTIFY stateChanged)
    /** Whether any slot of any page holds a function */
    Q_PROPERTY(bool hasAssigned READ hasAssigned NOTIFY assignmentsChanged)
    /** The speed of every cuelist: its fades (also the ones that run), delays,
     *  Follow and Wait. x0.1 - x10, x1 by default; not saved (a show opens at x1) */
    Q_PROPERTY(qreal cueSpeed READ cueSpeed WRITE setCueSpeed NOTIFY cueSpeedChanged)

public:
    PlaybackManager(Doc *doc, Programmer *programmer, CuelistEditor *cuelists, AppSettings *settings,
                    QObject *parent = nullptr);
    ~PlaybackManager();

    /** A function a fader of the SHOW holds up: a playback releasing it
     *  lets it go, but it plays on (only that fader stops it) */
    void setHeldByFader(const std::function<bool(quint32)> &held);

    /** The parent of the functions the playbacks start (not parent(): QObject has one) */
    static FunctionParent playbackParent() { return FunctionParent(FunctionParent::ManualVCWidget, 0x50420000); }
    /** The most slots a page can have */
    static int maxSlots() { return 20; }
    /** The longest flash attack or decay, in ms */
    static int maxFlashTime() { return 10000; }

    int revision() const;
    int progressRevision() const;

    /*********************************************************************
     * Pages (there is always one at least)
     *********************************************************************/
    int currentPage() const;
    void setCurrentPage(int page);
    int pageCount() const;
    QStringList pageNames() const;
    int slotsPerPage() const;
    void setSlotsPerPage(int count);

    /** @return the index of the new page */
    Q_INVOKABLE int addPage(const QString &name = QString());
    Q_INVOKABLE void renamePage(int page, const QString &name);
    /** What its slots play keeps playing */
    Q_INVOKABLE void removePage(int page);
    Q_INVOKABLE void movePage(int from, int to);
    Q_INVOKABLE void nextPage();
    Q_INVOKABLE void previousPage();
    /** -1 is the current page. @return the page, or -1 when there is no such page */
    Q_INVOKABLE int resolvePage(int page) const;

    /*********************************************************************
     * Slots (page -1 = the current page everywhere)
     *********************************************************************/
    /** Put a function on a slot; -1 clears it */
    Q_INVOKABLE void assign(int page, int slot, int functionId);
    Q_INVOKABLE void clearSlot(int page, int slot);
    /** Swap two slots */
    Q_INVOKABLE void moveSlot(int fromPage, int fromSlot, int toPage, int toSlot);
    /** The function of a slot, or -1 */
    Q_INVOKABLE int functionAt(int page, int slot) const;
    /** The first empty slot of a page, or -1 when it is full */
    Q_INVOKABLE int firstFreeSlot(int page) const;
    /** label, color, button1, button2, button3 (an action: none, go, back,
     *  pauseBack, pause, release, flash, flashGo, toggle, select), faderStarts
     *  (the fader up from 0 starts it), faderReleases (the fader down to 0
     *  releases it: -1 default for the type, 0 or 1), flashAttack and
     *  flashDecay (ms, 0 - maxFlashTime(); 0: at once), faderMode ("level":
     *  the intensity, or "crossfade": each travel of the fader is a GO
     *  faded by hand, Normal cuelists only) */
    Q_INVOKABLE void setSlotOption(int page, int slot, const QString &key, const QVariant &value);
    /** { page, slot, empty, functionId, name, label, type, color, isCuelist,
     *  running, paused, state, level, flashing (held, or its attack or
     *  decay runs), flashHeld (a press holds it), selected, cue, cueNumber,
     *  cueName, next, nextNumber, nextName, cueCount, progress, button1,
     *  button2, button3, faderStarts, faderReleases (resolved),
     *  faderReleasesMode (as set: -1 default, 0, 1), flashAttack, flashDecay,
     *  fading (a cue fades), fadeTotal and fadeElapsed (ms: the time of its
     *  delay and fade from its GO, with Live Time, and how far it is),
     *  liveScale (what Live Time did: 1 none), ownTimes (the cue runs with
     *  its own times, not a BACK time or a fade-in: Live Time can be
     *  stored), manual (an A/B fader
     *  crossfades it by hand), fadeBase (the time it started with, ms), mode (of a cuelist: "normal", "chase", "timecode",
     *  "submaster", "override"; "" for a scene), ruledByFader (a submaster
     *  its fader starts: GO on it stopped only chooses its cue), faderMode
     *  ("crossfade" when its fader is an A/B crossfade, else "level"),
     *  xfadePos (where an A/B fader is, 0.0 - 1.0), clock (the clock of a
     *  Timecode list that plays, ms; -1 for none) } */
    Q_INVOKABLE QVariantMap slotInfo(int page, int slot) const;
    /** The same as slotInfo, for a function (without page and slot) */
    Q_INVOKABLE QVariantMap functionStatus(int functionId) const;

    /*********************************************************************
     * Transport on a slot
     *********************************************************************/
    /** Cuelists: the next cue (starts it at full when its fader is down).
     *  Steps: the next step. Others: start (or resume) them. A submaster
     *  that is stopped only takes the next cue as the one its fader starts
     *  on (ruledByFader) */
    Q_INVOKABLE void go(int page, int slot);
    /** A stopped submaster: the cue before, for its fader to start on */
    Q_INVOKABLE void back(int page, int slot);
    /** Pause/Back: a fading cue pauses, a paused one goes back, else back
     *  (a stopped submaster: Back) */
    Q_INVOKABLE void pauseBack(int page, int slot);
    Q_INVOKABLE void pause(int page, int slot);
    Q_INVOKABLE void release(int page, int slot);
    /** Release it when it plays, else GO (a stopped submaster: nothing, its
     *  fader starts it) */
    Q_INVOKABLE void toggle(int page, int slot);
    /** The playback Main GO and Record act on (only one at a time) */
    Q_INVOKABLE void select(int page, int slot);
    /** A stopped submaster: only the cue its fader starts on */
    Q_INVOKABLE void goTo(int page, int slot, int cueIndex);
    /** Pre-select: the cue the next GO plays (-1 clears it) */
    Q_INVOKABLE void setNextCue(int page, int slot, int cueIndex);
    /** At full while pressed; a flash that started it releases it. The
     *  release lets go of what the press held, even when the page or the
     *  slot changed meanwhile. The flash attack and decay of the slot make
     *  it go up to full and back to its level in that time */
    Q_INVOKABLE void flash(int page, int slot, bool down);
    /** One of the three buttons of the slot (1 - 3), pressed or released.
     *  flashGo on a stopped playback: the flash start is its GO, and it
     *  plays on when the button comes up */
    Q_INVOKABLE void pressButton(int page, int slot, int button, bool down);
    /** The fader, 0.0 - 1.0. An A/B fader (faderMode "crossfade") is no
     *  intensity: each travel from one end to the other is a GO whose fade
     *  follows it (back to where it started cancels it); the first value
     *  only tells where it is. level() is where it is then */
    Q_INVOKABLE void setLevel(int page, int slot, qreal level);
    Q_INVOKABLE qreal level(int page, int slot) const;
    /** False for an A/B fader no value told where it is yet (a controller
     *  fader drives it at once: nothing to pick up) */
    bool faderKnown(int page, int slot) const;
    /** Its fader is an A/B crossfade (faderMode "crossfade"; page -1: the current one) */
    bool crossfadesAt(int page, int slot) const;
    /** GO and BACK of the selected playback (else the one that got the last GO) */
    Q_INVOKABLE void mainGo();
    Q_INVOKABLE void mainBack();
    /** Every playback, and every cuelist that plays; $except plays on, with
     *  its flash (a cue of it releases all of them) */
    Q_INVOKABLE void releaseAll(int except = -1);

    /*********************************************************************
     * Cue speed and Live Time
     *********************************************************************/
    qreal cueSpeed() const;
    void setCueSpeed(qreal speed);
    /** Live Time of the cue a cuelist fades (else nothing): its fades go on
     *  from where they are and end $ms after its GO (fadeTotal); 0 ends them
     *  now. For this run only */
    Q_INVOKABLE void setLiveTime(int page, int slot, int ms);
    /** The same, its time by $factor (0.5: half the time) */
    Q_INVOKABLE void scaleLiveTime(int page, int slot, qreal factor);
    /** The time Live Time gave the cue goes into the cue (its attribute and
     *  fixture times too): an edit of the cuelist, undone as the others.
     *  Not when the cue runs with other times (ownTimes false) */
    Q_INVOKABLE bool storeLiveTime(int page, int slot);
    /** The same on a cuelist (the cuelist editor) */
    Q_INVOKABLE void setLiveTimeFunction(int functionId, int ms);
    Q_INVOKABLE void scaleLiveTimeFunction(int functionId, qreal factor);
    Q_INVOKABLE bool storeLiveTimeFunction(int functionId);

    /*********************************************************************
     * The same on a function (group tiles, SHOW buttons, mappings with a
     * function, the editors)
     *********************************************************************/
    Q_INVOKABLE void goFunction(int functionId);
    Q_INVOKABLE void backFunction(int functionId);
    Q_INVOKABLE void pauseBackFunction(int functionId);
    Q_INVOKABLE void releaseFunction(int functionId);
    Q_INVOKABLE void goToFunction(int functionId, int cueIndex);
    /** With the flash times of the first slot that holds it (page order,
     *  then slot order); in no slot, at once */
    Q_INVOKABLE void flashFunction(int functionId, bool down);

    /*********************************************************************
     * Record to a playback
     *********************************************************************/
    /** An empty slot gets a new cuelist with the programmer as cue 1; a
     *  cuelist gets the programmer as its next cue. @return the index of the
     *  cue, or -1 (message()) */
    Q_INVOKABLE int recordToSlot(int page, int slot);
    /** Update the cue loaded from the selected cuelist, else the cue it plays */
    Q_INVOKABLE bool updateSelected(bool cueOnly = false);
    /** Clear the programmer after recording, when the settings ask for it
     *  (also after RECORD and UPDATE on the command line) */
    void afterRecord();

    /*********************************************************************
     * Readers (mappings, remote)
     *********************************************************************/
    QVariantMap selected() const;
    bool hasAssigned() const;
    /** -1 empty, 0 loaded, 1 on, 2 fading or waiting (cuelists), 3 paused */
    int state(int page, int slot) const;
    /** The same for a function (-1 when it doesn't exist) */
    int functionState(quint32 functionId) const;
    /** Held by a flash, or its flash attack or decay runs (the Flash button is lit) */
    bool isFlashing(int page, int slot) const;
    bool isSelected(int page, int slot) const;
    /** Cuelists: the cue it plays (-1 none), the one the next GO plays, and how many */
    int currentCue(int page, int slot) const;
    int nextCue(int page, int slot) const;
    int cueCount(int page, int slot) const;

    /** Stop all (panic): flashes are let go, the levels stay */
    void resetLive();
    /** A new show: one empty page */
    void clear();

    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc) const;

signals:
    void stateChanged();
    void progressChanged();
    void pageChanged();
    void pagesChanged();
    /** Pages moved, or one was deleted (before pagesChanged): $newIndexes is
     *  the index each page had before -> its index now (-1: deleted), and
     *  $removedSlots what the slots of a deleted page held (slot -> function
     *  ID, still playing). The page indexes kept elsewhere follow their
     *  pages: SHOW sections and controls, mappings */
    void pagesRenumbered(const QList<int> &newIndexes, const QHash<int, quint32> &removedSlots);
    void assignmentsChanged();
    /** A result or a problem that QML shows to the user */
    void message(const QString &text, bool error);
    void cueSpeedChanged();

private slots:
    void slotFunctionRunning(quint32 id);
    void slotFunctionStopped(quint32 id);
    void slotFunctionRemoved(quint32 id);
    void slotDocCleared();
    void slotFunctionReplaced(quint32 id);
    /** A cuelist edited (its cues): the playbacks show it */
    void slotFunctionChanged(quint32 id);
    void slotFunctionListChanged();
    void slotCueStarted(quint32 functionId, quint32 cueUid);
    void slotPoll();
    /** The flash attacks and decays move on (and end) */
    void slotFlashRamps();

private:
    struct Slot
    {
        quint32 functionId = 0;
        QString label;
        QString color;
        QString button1 = QStringLiteral("go");
        QString button2 = QStringLiteral("pauseBack");
        QString button3 = QStringLiteral("flash");
        bool faderStarts = true;
        /** -1: the default for the type (off for cuelists, on for submasters
         *  and the rest) */
        int faderReleases = -1;
        /** a flash goes up to full in flashAttack ms, and back down in
         *  flashDecay ms (0: at once) */
        int flashAttack = 0;
        int flashDecay = 0;
        /** the fader is an A/B crossfade (on a Normal cuelist only) */
        bool crossfade = false;
    };
    /** An A/B fader, per function: the end it left (-1: not known yet),
     *  whether a travel runs (a GO faded by hand), where it is */
    struct Crossfade
    {
        int end = -1;
        bool active = false;
        qreal pos = 0.0;
    };
    /** A flash on its way up to full (attack) or back down (decay): its
     *  value comes from the time since it started, so a late tick doesn't
     *  make it longer */
    struct FlashRamp
    {
        qreal from = 0.0;
        qreal to = 1.0;
        /** on m_rampClock, ms */
        qint64 start = 0;
        int duration = 0;
        /** a decay of what the flash started: released at the end */
        bool releaseAtEnd = false;
    };
    struct Page
    {
        QString name;
        /** by slot index; slots past slotsPerPage are kept, hidden */
        QMap<int, Slot> slotMap;
    };

    /** The slot at (page, slot), or nullptr (page -1: the current page) */
    const Slot *slotAt(int page, int slot) const;
    Function *function(quint32 functionId) const;
    static bool isAction(const QString &action);
    /** Whether a slot releases its function when its fader goes to 0 */
    static bool faderReleases(const Slot &slot, const Function *f);
    /** Whether the fader of a slot is an A/B crossfade: set so, on a Normal cuelist */
    static bool crossfades(const Slot &slot, const Function *f);
    /** An A/B fader moved to $pos: a travel starts, goes on, completes or
     *  is cancelled */
    void crossfadeTo(Cuelist *cuelist, qreal pos);
    /** The A/B fader of a function stops driving it (another mode, the
     *  slot cleared): a travel that runs goes on by itself, and where the
     *  fader is will be learnt again */
    void dropCrossfade(quint32 functionId);
    /** A submaster on a playback whose fader starts it: the fader starts
     *  and releases it, and GO or Back on it stopped only choose its cue.
     *  Off the playbacks (or with no fader to start it) it plays as a
     *  Normal cuelist */
    bool ruledByFader(const Function *f) const;
    /** The cue a stopped cuelist starts on: $step from the one it starts on
     *  now. Past the last cue the first (Loop, Release), or the last (Stay);
     *  never before the first, as BACK */
    void chooseStartCue(Cuelist *cuelist, int step);
    /** Whether it plays: started and not stopped (it may still be fading out) */
    static bool isPlaying(const Function *f);
    /** A flash press holds it */
    bool isFlashing(quint32 functionId) const;
    /** Held, or its flash attack or decay runs (the Flash button is lit) */
    bool isFlashLit(quint32 functionId) const;
    /** The cue a cuelist plays (-1: none) and the one its next GO plays */
    static void cueIndexes(Cuelist *cuelist, int *current, int *next);

    /** Starts a function with the playbacks as parent, at its level; a
     *  cuelist from $cueIndex (-1: from where it starts) */
    void startFunction(Function *f, int cueIndex = -1);
    void ensureConnected(Function *f);
    /** The level it plays at: where its flash attack or decay is, else full
     *  while flashing, else its fader */
    qreal effectiveLevel(quint32 functionId) const;
    /** The level of the function on its intensity (effectiveLevel()); a
     *  super scene on each track and on its static values */
    void applyLevel(Function *f);
    /** GO on a stopped playback with its fader down plays it at full */
    void levelUpForGo(quint32 functionId);
    /** A GO on a cuelist releases the cuelists that ask for it */
    void releaseOthersOnGo(quint32 functionId);
    void releaseIt(Function *f);
    void pauseFunction(Function *f);
    /** A flash press of a slot ($button 0: flash()) holds the function of
     *  the slot; its release lets go of that same function */
    void flashPress(int page, int slot, int button, bool down);
    /** A flash press: up to full in $attack ms (0: at once). The last release
     *  brings it back in the $decay of the last press */
    void flashDown(Function *f, int attack, int decay);
    void flashUp(Function *f);
    /** The flash times of the first slot that holds the function (page
     *  order, then slot order); 0 when no slot holds it */
    void flashTimesOf(quint32 functionId, int *attack, int *decay) const;
    /** A GO keeps a playback a flash started: a decay that would release it
     *  ends at its level instead */
    void keepFlashStart(quint32 functionId);
    /** Where a ramp is now */
    qreal rampValue(const FlashRamp &ramp) const;
    void startRamp(quint32 functionId, qreal from, qreal to, int duration, bool releaseAtEnd);
    /** A ramp ends at $to instead, from where it is, in the time it has left */
    void retargetRamp(FlashRamp &ramp, qreal to);
    /** The flash holds of a function are gone, and the presses that held it */
    void dropFlashHolds(quint32 functionId);
    /** It stopped, or it is not the same function any more */
    void forgetRuntime(quint32 functionId);

    /** The keys of slotInfo() about the function (and its level) */
    void fillStatus(QVariantMap &info, Function *f) const;
    static void fillOptions(QVariantMap &info, const Slot &slot, const Function *f);
    /** The pages or slots changed: the selection and the last GO can't point
     *  to a slot that isn't there any more */
    void fixSelection();
    /** Index of a page after another page moved from $from to $to */
    static int movedIndex(int index, int from, int to);
    /** The flash presses follow their pages, and pagesRenumbered() */
    void renumberPages(const QList<int> &newIndexes, const QHash<int, quint32> &removedSlots);
    void bump();
    void ensurePolling();
    /** Reads what the UI shows of every playback: bumps the revisions when
     *  it changed ($force: always) and keeps polling while something plays */
    void poll(bool force);
    Page defaultPage(int number) const;

private:
    Doc *m_doc;
    Programmer *m_programmer;
    CuelistEditor *m_cuelists;
    AppSettings *m_settings;
    std::function<bool(quint32)> m_heldByFader;

    QList<Page> m_pages;
    int m_currentPage;
    int m_slotsPerPage;
    /** the selected slot (not saved), and the one that got the last GO */
    int m_selectedPage;
    int m_selectedSlot;
    int m_lastGoPage;
    int m_lastGoSlot;

    /* the runtime, per function (clones share it) */
    QHash<quint32, qreal> m_levels;
    QHash<quint32, int> m_flashHolds;
    /** the decay of the last press that holds each function, ms */
    QHash<quint32, int> m_flashDecays;
    /** the flash attacks and decays that run, on m_rampClock (m_rampTimer
     *  ticks while there is one) */
    QHash<quint32, FlashRamp> m_flashRamps;
    QElapsedTimer m_rampClock;
    QTimer *m_rampTimer;
    QSet<quint32> m_startedByFlash;
    /** the flash presses not released yet: (page as given, -1 for the
     *  current one; slot; button, 0 for flash()) -> the functions they hold */
    QMap<std::tuple<int, int, int>, QList<quint32>> m_flashPresses;
    /** the functions the playbacks started and didn't see stop */
    QSet<quint32> m_held;
    /** super scenes: an intensity override per track (by track index) */
    QHash<quint32, QList<int>> m_showOverrides;
    /** the A/B faders */
    QHash<quint32, Crossfade> m_crossfades;

    int m_revision;
    int m_progressRevision;
    /** what the UI shows of each function, to bump only on changes */
    QHash<quint32, QList<int>> m_signatures;
    /** the progress of each one, in steps of 5 %, and (DMX Total Control) the
     *  clock of a Timecode list in tenths of a second (-1: none) */
    QHash<quint32, QPair<int, qint64>> m_progress;
    QTimer *m_pollTimer;
};

#endif
