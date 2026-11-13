/*
  DMX Total Control
  cuelisteditor.h

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

#ifndef CUELISTEDITOR_H
#define CUELISTEDITOR_H

#include <QObject>
#include <QVariant>
#include <QElapsedTimer>

#include "cuelist.h"

class PaletteManager;
class ZoneManager;
class Programmer;
class Chaser;
class Doc;

/**
 * Recording and editing of cuelists, on top of the engine Cuelist:
 * cues hold only what they change, and the stage shows the tracked state.
 *
 * Record stores the difference between the programmer and what the cue
 * would track from the cues before it; Update merges the programmer into a
 * cue (values equal to the tracked ones go, so they track again); Load puts
 * a cue, tracked or on its own, in the programmer to edit it. Cue numbers
 * are text ("2.5") in QML: the engine keeps thousandths.
 */
class CuelistEditor : public QObject
{
    Q_OBJECT

    /** The cuelist the playbacks and the editors work on; -1 for none */
    Q_PROPERTY(int selectedId READ selectedId WRITE setSelectedId NOTIFY selectedIdChanged)
    /** The cue loaded in the programmer: { id, uid, index, number, name, tracked }, or {} */
    Q_PROPERTY(QVariantMap editing READ editing NOTIFY editingChanged)

public:
    CuelistEditor(Doc *doc, Programmer *programmer, ZoneManager *zones, PaletteManager *palettes,
                  QObject *parent = nullptr);

    int selectedId() const;
    void setSelectedId(int id);
    QVariantMap editing() const;

    /*********************************************************************
     * Cuelists
     *********************************************************************/
    /** A new cuelist with the programmer as cue 1. $priority -1: the default
     *  of the functions. @return its ID, or -1 (empty programmer) */
    Q_INVOKABLE int create(const QString &name, int zoneIndex = -1, int priority = -1);
    /** [{ uid, index, number, name, note, trigger ("go", "follow", "wait"),
     *  triggerTime, fade, downFade (-1: same as fade), delay, curve
     *  ("linear", "smooth", "slowstart", "faststart"), skip, block, noMib
     *  (Move in black leaves it alone), changes, releases, fx, fxReleases, palettes (names), ownTimes (how
     *  many attributes and fixtures have times of their own), timesText
     *  (their summary: the curve of the cue when it isn't linear, the delay
     *  and the fade of each attribute (and its curve) and how many fixtures:
     *  "Position 0.5 s + 2 s, 3 fixtures" with middle dots), link (the uid
     *  of the cue after it, 0: the next one), linkNumber (its number, ""),
     *  linkCount (0: always), actions (how many), actionsText (them, with
     *  commas), time (on the clock of a Timecode list, ms; -1: none),
     *  timeWarning (its time is before the one of the cue before it) }] */
    Q_INVOKABLE QVariantList cues(quint32 id) const;
    /** { mode ("normal", "chase", "timecode", "submaster", "override"),
     *  tracking, atEnd ("loop", "release", "stay"), resetOnRelease,
     *  releaseTime, defaultFade, backTime, releaseOnNextGo, priority,
     *  moveInBlack, mibDelay, mibFade (ms), chaseBpm, chaseShowTempo,
     *  chaseMultiplier, chaseXfade, chaseDirection, tcSource ("internal",
     *  "mtc"), tcOffset (ms) } */
    Q_INVOKABLE QVariantMap options(quint32 id) const;
    /** One of the keys of options() */
    Q_INVOKABLE void setOption(quint32 id, const QString &key, const QVariant &value);

    /*********************************************************************
     * Cue numbers
     *********************************************************************/
    /** The number for a new cue after $afterIndex: the next whole number
     *  after the last cue (-1, or the last index), else the roundest number
     *  between that cue and the next one (2.5 between 2 and 3, 2.2 between
     *  2 and 2.5); before the first cue for -2. "" when none fits */
    Q_INVOKABLE QString nextNumber(quint32 id, int afterIndex = -1) const;
    Q_INVOKABLE bool numberExists(quint32 id, const QString &number) const;

    /*********************************************************************
     * Record, update, load
     *********************************************************************/
    /**
     * Record the programmer as cue $number ("" = the next whole number): the
     * values and FX that differ from what the cue tracks from the cues before.
     * $opts: name, cueOnly (the cue doesn't track on: Skip), block (keep the
     * values equal to the tracked ones too), existing: what to do when the
     * number exists: "ask" (default: return -2, nothing changes), "merge"
     * (updateCue), "replace" (the cue holds the programmer only, its name and
     * times stay) or "insertAfter" (a point cue after it).
     * @return the index of the cue, -2 (ask) or -1 on error (message())
     */
    Q_INVOKABLE int recordCue(quint32 id, const QString &number = QString(), const QVariantMap &opts = QVariantMap());
    /**
     * Merge the programmer into a cue: its values and FX
     * go into the cue, but the ones equal to what it tracks are taken out of
     * it (unless it is a block cue), so they track again. What the programmer
     * doesn't hold is untouched. The change tracks on to the next cues;
     * $opts.cueOnly: the next cue keeps what it showed before (values)
     */
    Q_INVOKABLE bool updateCue(quint32 id, int index, const QVariantMap &opts = QVariantMap());
    /** Put a cue in the programmer to edit it: what the stage shows on it
     *  ($tracked), or only what the cue holds */
    Q_INVOKABLE bool loadCue(quint32 id, int index, bool tracked = true);
    /** Remove values from a cue: the channels and FX of the
     *  programmer leave the cue (they track from before again) */
    Q_INVOKABLE bool removeFromCue(quint32 id, int index);
    /** The channels and FX of the programmer become [RELEASE] in the cue */
    Q_INVOKABLE bool releaseInCue(quint32 id, int index);
    /** The same with keys of the tracking sheet ("fixture:channel", "fixture:axis") */
    Q_INVOKABLE bool removeKeysFromCue(quint32 id, int index, const QVariantList &keys);
    Q_INVOKABLE bool releaseKeysInCue(quint32 id, int index, const QVariantList &keys);

    /*********************************************************************
     * Cues
     *********************************************************************/
    /** Delete cues. $keepLook: what a cue changes moves into the next cue
     *  (where that one doesn't change it), so the next cues look the same */
    Q_INVOKABLE bool removeCues(quint32 id, const QVariantList &indexes, bool keepLook = true);
    /** Copy a cue as cue $number ("" = the next whole number): what it holds,
     *  or ($tracked) everything the stage shows on it, as a block cue.
     *  @return the index of the copy, or -1 */
    Q_INVOKABLE int copyCue(quint32 id, int index, const QString &number, bool tracked = false);
    /** Give a cue another number. @return "" when done, else the error */
    Q_INVOKABLE QString renumber(quint32 id, int index, const QString &number);
    /** Times of several cues, in ms; -1 leaves them unchanged, and $downFade -2
     *  makes it the same as the fade */
    Q_INVOKABLE void setCueTimes(quint32 id, const QVariantList &indexes, int fade, int downFade, int delay);
    /** Every time of a cue by $factor: its fade, down fade and delay, and
     *  those of its attributes and fixtures (not its trigger time). Live
     *  Time keeps the time of a run this way */
    void scaleCueTimes(quint32 id, int index, qreal factor);
    /** "go", "follow" or "wait" ("" unchanged), and its time ($time -1 unchanged) */
    Q_INVOKABLE void setCueTrigger(quint32 id, const QVariantList &indexes, const QString &trigger, int time);
    Q_INVOKABLE void setCueName(quint32 id, int index, const QString &name);
    Q_INVOKABLE void setCueNote(quint32 id, int index, const QString &note);
    /** Skip (cue only) and block: 0 or 1, -1 unchanged */
    Q_INVOKABLE void setCueFlags(quint32 id, const QVariantList &indexes, int skip, int block);
    /** Move in black prepares these cues ($on, the default) or not (noMib) */
    Q_INVOKABLE void setCueMib(quint32 id, const QVariantList &indexes, bool on);
    /** The time of a cue on the clock of a Timecode list, in ms (up to
     *  Cuelist::maxCueTime()); -1: none */
    Q_INVOKABLE void setCueTime(quint32 id, int index, int ms);
    /** A cue gets the time of the clock of its list now (it runs).
     *  @return false when the list has no clock running */
    Q_INVOKABLE bool stampCueTime(quint32 id, int index);

    /*********************************************************************
     * Times of the attributes and fixtures of a cue (they win over the
     * cue's own; a fixture's over its attribute's). In ms, -1 is the cue's
     *********************************************************************/
    /** { fade, downFade, delay, curve (the cue's, as in cues()), attributes:
     *  [{ attribute ("intensity", "colour", "position", "beam"), fade,
     *  delay, curve ("": the cue's) }] (all four), fixtures: [{ fixture,
     *  name, fade, delay }] (by ID), length (the longest time of the cue) },
     *  or {} */
    Q_INVOKABLE QVariantMap cueTimes(quint32 id, int index) const;
    /** The times of an attribute in several cues: -1 leaves a time
     *  unchanged, -2 makes it the cue's again. $curve: "" unchanged, "cue"
     *  the cue's, else the name of a curve */
    Q_INVOKABLE void setAttributeTime(quint32 id, const QVariantList &indexes, const QString &attribute,
                                      int fade, int delay, const QString &curve = QString());
    /** The curve of the fades of several cues ("linear", "smooth",
     *  "slowstart", "faststart"; anything else is linear) */
    Q_INVOKABLE void setCueCurve(quint32 id, const QVariantList &indexes, const QString &curve);
    /** Fixtures get times of their own in several cues, starting with the
     *  fade and delay of each cue; the ones that have them keep them */
    Q_INVOKABLE void addFixtureTimes(quint32 id, const QVariantList &indexes, const QVariantList &fixtures);
    /** The times of fixtures in several cues, as setAttributeTime */
    Q_INVOKABLE void setFixtureTimes(quint32 id, const QVariantList &indexes, const QVariantList &fixtures,
                                     int fade, int delay);
    /** The fixtures lose their own times; an empty $fixtures: all of them */
    Q_INVOKABLE void clearFixtureTimes(quint32 id, const QVariantList &indexes, const QVariantList &fixtures);
    /** A time fan: each of $fixtures (in their order) gets its own delay
     *  ($what "delay") or fade ("fade") between $fromMs and $toMs in several
     *  cues; its other time stays. $shape: "line" (one after the other),
     *  "middle" (from the middle) or "ends" (from the ends). Fixed times,
     *  each one can be edited after */
    Q_INVOKABLE void fanFixtureTimes(quint32 id, const QVariantList &indexes, const QVariantList &fixtures,
                                     const QString &what, int fromMs, int toMs, const QString &shape);
    /** The IDs of the fixtures a cue changes (values, releases, FX and FX
     *  releases), sorted */
    Q_INVOKABLE QVariantList cueFixtures(quint32 id, int index) const;

    /*********************************************************************
     * Link and actions of a cue
     *********************************************************************/
    /** After cue $index the list goes to the cue $linkUid of the list (0:
     *  the next cue, as without a link), $count times (0..999, 0: always)
     *  before it goes on past it. A uid the list doesn't have changes nothing */
    Q_INVOKABLE void setCueLink(quint32 id, int index, quint32 linkUid, int count);
    /** What a cue does when it starts, in order: [{ target, functionId,
     *  cue, cueUid, zoneId, slot, mode, value (as the action has them),
     *  text }] (text: "GO of Song 2"; "(deleted)" for a function that is
     *  gone) */
    Q_INVOKABLE QVariantList cueActions(quint32 id, int index) const;
    /** Replaces them. The ones it doesn't know are left out (actionFrom) */
    Q_INVOKABLE void setCueActions(quint32 id, int index, const QVariantList &actions);
    /** An action as QML and the mappings see it: functionId, cue, cueUid,
     *  zoneId, slot and value are numbers */
    static QVariantMap actionToMap(const Cuelist::CueAction &action);
    /** An action from QML: playbackGo, playbackGoTo (cue, cueUid) and
     *  playbackRelease on a functionId (slot -1: wherever it is), scene
     *  (functionId, mode "on" or "off"), groupRelease (zoneId; -1: all the
     *  groups), playbackReleaseAll and bpm (value 20-300). Empty for any
     *  other target */
    static Cuelist::CueAction actionFrom(const QVariantMap &map);
    /** A show was loaded: the actions on functions it doesn't have do nothing */
    void afterLoad();

    /*********************************************************************
     * Views
     *********************************************************************/
    /**
     * What the stage shows on a cue, fixture by fixture (sorted by fixture,
     * then channel): [{ fixture, fixtureName, channels: [{ key ("f:c"),
     * channel, name, value, state, from, palette, paletteName }], fx: [{ key
     * ("f:axis"), axis, param, state, from }] }]. state: "changed" (the cue
     * sets it), "blocked" (the cue sets it to what it tracked), "tracked"
     * (from: the number of the cue that sets it) or "released" (by the cue)
     */
    Q_INVOKABLE QVariantList trackingSheet(quint32 id, int index) const;
    /** The palettes the values of a cue come from: [{ id, name, type, number, color, channels }] */
    Q_INVOKABLE QVariantList cuePalettes(quint32 id, int index) const;

    /** Steps (a Sequence) or a Chaser become a cuelist with the same ID (its
     *  groups, SHOW buttons and mappings follow): each step holds what changes
     *  from the step before; a chaser keeps its scenes. What doesn't play the
     *  same goes to message() (see QlcExchange::stepsToCues).
     *  @return the ID, or -1 */
    Q_INVOKABLE int convertFromSteps(quint32 stepsId);
    /** $cl (not in the show) takes the place of the steps $chaser, with its
     *  ID: what plays them plays it. The hidden channels of a Sequence go
     *  with it, the scenes of a Chaser stay.
     *  @return false when it can't: $cl is deleted */
    bool replaceSteps(Chaser *chaser, Cuelist *cl);

    /*********************************************************************
     * Undo. Each cuelist has its own history of its cue edits and options
     * (not of creating it or converting steps). It isn't saved
     *********************************************************************/
    /** { canUndo, canRedo, undo, redo (the names of the steps) } */
    Q_INVOKABLE QVariantMap undoState(quint32 id) const;
    /** Puts back the cuelist before its last edit (after the last one
     *  undone). @return the index of the first cue that changed, or -1 */
    Q_INVOKABLE int undo(quint32 id);
    Q_INVOKABLE int redo(quint32 id);

    /** The programmer doesn't hold a cue any more */
    Q_INVOKABLE void stopEditing();
    /** A new show */
    void clear();

signals:
    /** A cuelist was changed by the editor; QML refreshes its view */
    void edited(quint32 id);
    /** A problem (or a warning) that QML shows to the user */
    void message(const QString &text);
    void selectedIdChanged();
    /** The cue in the programmer changed: a load, a stop, and also every edit
     *  of its cuelist (its index or number may have moved) */
    void editingChanged();
    /** A cue was put in the programmer (Load), from anywhere: the views select it */
    void cueLoaded(quint32 id, int index);
    /** The undo history of a cuelist changed; Function::invalidId(): of all of them */
    void undoChanged(quint32 id);

private slots:
    void slotFunctionRemoved(quint32 id);
    void slotFunctionReplaced(quint32 id);

private:
    Cuelist *cuelist(quint32 id) const;
    void touch(quint32 id);
    /** The programmer: values, palette references and FX */
    QHash<Cuelist::Key, uchar> programmerValues() const;
    QHash<Cuelist::Key, int> programmerRefs() const;
    /** A new cue at $number with the programmer, against what it tracks */
    int recordNew(Cuelist *cl, int number, const QVariantMap &opts);
    /** The time a new cue at $index of a Timecode list gets: the clock while
     *  it runs, else 5 s after the cue before it with a time (the first: 0) */
    static int newCueTime(const Cuelist *cl, int index);
    /** The programmer values and FX that differ from $before ($block: all of them) */
    void fillDelta(Cuelist::Cue &cue, const Cuelist::State &before, bool block) const;
    /** Takes keys and FX slots out of a cue; $release: they become releases */
    static bool takeOut(Cuelist::Cue &cue, const QSet<Cuelist::Key> &keys,
                        const QSet<Cuelist::FxKey> &slotKeys, bool release);
    /** The programmer keys and FX slots */
    QSet<Cuelist::Key> programmerKeys() const;
    QSet<Cuelist::FxKey> programmerSlots() const;
    /** "f:c" and "f:axis" keys of the tracking sheet */
    static void parseKeys(const QVariantList &keys, QSet<Cuelist::Key> *channels, QSet<Cuelist::FxKey> *slotKeys);
    QString paletteName(int paletteId) const;
    /** The summary of the attribute and fixture times of a cue ("" without them) */
    static QString timesText(const Cuelist::Cue &cue);
    /** The name of a FadeChannel::Curve, translated */
    static QString curveName(int curve);
    /** The fixtures of the show in $fixtures, once each, sorted */
    QList<quint32> fixtureIds(const QVariantList &fixtures) const;
    /** One of the keys of options(), without an undo step. @return false for an unknown key */
    static bool applyOption(Cuelist *cl, const QString &key, const QVariant &value);
    /** What an action does, in words */
    QString describeAction(const Cuelist::CueAction &action) const;
    /** The actions on the functions $gone() says get functionId -1, in every
     *  cuelist (without an undo step) and in their histories */
    void forgetActionsOn(const std::function<bool(quint32)> &gone);

    /*********************************************************************
     * Undo
     *********************************************************************/
    /** What an undo puts back: the cues and the options of a cuelist */
    struct UndoState
    {
        QList<Cuelist::Cue> cues;
        QVariantMap options;
        bool operator==(const UndoState &other) const { return cues == other.cues && options == other.options; }
        bool operator!=(const UndoState &other) const { return !(*this == other); }
    };
    struct UndoEntry
    {
        QString label;
        /** edits with the same key, close in time, are one step */
        QString mergeKey;
        qint64 at = 0;
        UndoState before;
        UndoState after;
    };
    /** Makes one undo step of an edit, from its first line to its end. A
     *  nested edit (Record that merges into a cue) is part of the outer one */
    class EditScope
    {
        Q_DISABLE_COPY(EditScope)
    public:
        EditScope(CuelistEditor *editor, quint32 id, const QString &label, const QString &mergeKey = QString());
        ~EditScope();
        /** The name of the step, once the edit knows it (the number of a new cue) */
        void setLabel(const QString &label);
    private:
        CuelistEditor *m_editor;
        bool m_outer;
    };

    UndoState undoStateOf(quint32 id) const;
    /** The step of the edit that ends, unless nothing changed */
    void commitUndo();
    /** Undoes ($redo: redoes) the last step of a cuelist: the step goes to
     *  the other history. @return as undo() */
    int step(quint32 id, bool redo);
    /** The cuelist as $entry has it before ($redo: after) */
    int restore(quint32 id, const UndoEntry &entry, bool redo);
    /** Drops what refers to fixtures (and channels) that are not in the
     *  show; the actions on functions that are not in it do nothing */
    void pruneMissing(QList<Cuelist::Cue> &cues) const;
    /** The values that come from a palette take its values now; the
     *  palettes that are gone leave plain values */
    void refreshPalettes(QList<Cuelist::Cue> &cues) const;
    /** Drops the history of a cuelist */
    void dropUndo(quint32 id);

private:
    Doc *m_doc;
    Programmer *m_programmer;
    ZoneManager *m_zones;
    PaletteManager *m_palettes;
    int m_selectedId;
    /** The cue in the programmer: the cuelist and the uid of the cue */
    quint32 m_editId;
    quint32 m_editUid;
    bool m_editTracked;

    /** The undo history of each cuelist: the last step at the end */
    QHash<quint32, QList<UndoEntry>> m_undo;
    QHash<quint32, QList<UndoEntry>> m_redo;
    QElapsedTimer m_undoClock;
    /** The edit in progress (EditScope) */
    int m_scopeDepth;
    quint32 m_scopeId;
    QString m_scopeLabel;
    QString m_scopeMerge;
    UndoState m_scopeBefore;
};

#endif
