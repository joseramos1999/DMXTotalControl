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

#include "cuelist.h"

class PaletteManager;
class ZoneManager;
class Programmer;
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
     *  triggerTime, fade, downFade (-1: same as fade), delay, skip, block,
     *  changes, releases, fx, fxReleases, palettes (names) }] */
    Q_INVOKABLE QVariantList cues(quint32 id) const;
    /** { tracking, atEnd ("loop", "release", "stay"), resetOnRelease,
     *  releaseTime, defaultFade, backTime, releaseOnNextGo, priority } */
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
    /** "go", "follow" or "wait" ("" unchanged), and its time ($time -1 unchanged) */
    Q_INVOKABLE void setCueTrigger(quint32 id, const QVariantList &indexes, const QString &trigger, int time);
    Q_INVOKABLE void setCueName(quint32 id, int index, const QString &name);
    Q_INVOKABLE void setCueNote(quint32 id, int index, const QString &note);
    /** Skip (cue only) and block: 0 or 1, -1 unchanged */
    Q_INVOKABLE void setCueFlags(quint32 id, const QVariantList &indexes, int skip, int block);

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

    /** Steps become a cuelist with the same ID (its groups, SHOW buttons and
     *  mappings follow): each step holds what changes from the step before.
     *  @return the ID, or -1 */
    Q_INVOKABLE int convertFromSteps(quint32 sequenceId);

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
};

#endif
