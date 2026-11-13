/*
  DMX Total Control
  palettemanager.h

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

#ifndef PALETTEMANAGER_H
#define PALETTEMANAGER_H

#include <QObject>
#include <QColor>
#include <QHash>
#include <QMap>
#include <QPair>
#include <QSet>
#include <QVariant>

class QXmlStreamReader;
class QXmlStreamWriter;
class PatchManager;
class Programmer;
class QLCChannel;
class Fixture;
class Doc;

#define KXMLDTCPalettes QStringLiteral("Palettes")

/**
 * Palettes: intensity, position, colour
 * and beam values recorded from the programmer, for each fixture, or all of
 * them at once ("all": every value but the maintenance channels).
 *
 * They go by reference: the scenes, cues and steps recorded with a palette
 * remember it, so correcting the palette (refocusing "Singer") changes every
 * one that uses it. On a fixture the palette was not recorded for, it uses
 * the values of a fixture of the same type, else the same attributes (and,
 * for colours, the mix of its colour).
 *
 * A palette recorded on several fixtures is a fan, in the order they were
 * selected: the fixtures it was not recorded on share it out in the order
 * of the selection (or of the group), and each one keeps its place in it.
 *
 * Palettes are part of the show (DMX Total Control section): the scenes keep
 * their plain values too, so QLC+ still plays them.
 */
class PaletteManager : public QObject
{
    Q_OBJECT

    /** [{ id, type, number, name, code, color, fixtures, spread, scenes }] by
     *  type and number (code: "COL 2", as the command line says it; spread:
     *  the fixtures it was not recorded on that have a place in its fan;
     *  scenes: the scenes, the cuelists and the steps that use it) */
    Q_PROPERTY(QVariantList palettes READ palettes NOTIFY palettesChanged)
    /** How each palette is on the selection: { "<id>": 0 none, 1 some, 2 all } */
    Q_PROPERTY(QVariantMap states READ states NOTIFY statesChanged)
    /** How many programmer values of the selection each type would record: { type: n } */
    Q_PROPERTY(QVariantMap recordable READ recordable NOTIFY statesChanged)

public:
    PaletteManager(Doc *doc, Programmer *programmer, PatchManager *patch, QObject *parent = nullptr);

    /** "intensity", "position", "colour", "beam", "all" */
    Q_INVOKABLE static QStringList types();
    /** The type of palette that records a channel, or "" (maintenance, nothing) */
    static QString typeOf(const QLCChannel *channel);
    /** Whether a palette of $type records a channel: the ones of its type, or
     *  for "all" every channel but the maintenance ones (lamp, reset) */
    static bool records(const QString &type, const QLCChannel *channel);
    /** The command word of a type: INT, POS, COL, BEAM, PAL (not translated) */
    static QString word(const QString &type);

    QVariantList palettes() const;
    QVariantMap states() const;
    QVariantMap recordable() const;
    /** One palette, like in palettes(), or an empty map */
    Q_INVOKABLE QVariantMap palette(int id) const;
    /** The palette $number of $type, or -1 */
    Q_INVOKABLE int paletteByNumber(const QString &type, int number) const;
    /** The names of the palettes $ids, in the order of the palettes (by type
     *  and number); IDs of no palette are left out */
    QStringList namesOf(const QSet<int> &ids) const;

    /** Record the programmer values of the selection of $type as a new palette.
     *  Those values then come from it. @return its id, or -1 if there is nothing to record */
    Q_INVOKABLE int record(const QString &type, const QString &name = QString(), int number = -1);
    /** Update: the programmer values of the selection go into the palette,
     *  the other fixtures keep theirs. The scenes that use it follow */
    Q_INVOKABLE bool update(int id);
    /** Record over: the palette gets exactly the values of the selection */
    Q_INVOKABLE bool replace(int id);
    Q_INVOKABLE void rename(int id, const QString &name);
    /** Give a palette another number in its type. @return an error, empty when done */
    Q_INVOKABLE QString setNumber(int id, int number);
    /** Delete a palette: the scenes keep its values */
    Q_INVOKABLE void remove(int id);

    /** The palette on the selection of the programmer (the fixtures it was not
     *  recorded on keep a place in its fan). @return how many fixtures took it */
    Q_INVOKABLE int apply(int id);
    /** The selected fixtures it was not recorded on get a new place in its
     *  fan, in the order of the selection, and take it; what uses them
     *  follows. @return how many got a place */
    Q_INVOKABLE int spreadAgain(int id);
    /** Select the fixtures of the palette and apply it, to change it and update it */
    Q_INVOKABLE bool load(int id);

    /** Live (SHOW, mappings): the palette as a live edit of the programmer on a
     *  fixture group (its fan in the order of the group), or its own fixtures
     *  with $groupId -1 */
    void setLive(int id, int groupId, bool on);
    bool isLive(int id, int groupId) const;

    /** The values of a palette on a fixture: channel -> value (empty: it doesn't apply) */
    QMap<quint32, uchar> valuesFor(int id, quint32 fixtureId) const;

    /*********************************************************************
     * The scenes that use palettes
     *********************************************************************/
    /** The palette of each value of a scene (static or FX) that comes from one */
    QHash<QPair<quint32, quint32>, int> sceneRefs(quint32 functionId) const;
    void setSceneRefs(quint32 functionId, const QHash<QPair<quint32, quint32>, int> &refs);
    /** Some values of a scene were written ($written): theirs are now $refs */
    void mergeSceneRefs(quint32 functionId, const QHash<QPair<quint32, quint32>, int> &refs,
                        const QList<QPair<quint32, quint32>> &written);
    /** The palettes a scene uses: [{ id, type, number, name, color, channels }] */
    Q_INVOKABLE QVariantList scenePalettes(quint32 functionId) const;

    void clear();
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc) const;
    /** Once a show is loaded: the cues and steps that name a palette that isn't
     *  there (a damaged or edited file) keep their values as plain values, like
     *  when it is deleted; and no palette made later gets an ID up to
     *  $usedId (what the SHOW and the mappings still name) */
    void afterLoad(int usedId);

signals:
    void palettesChanged();
    void statesChanged();

private slots:
    void slotFunctionRemoved(quint32 id);
    /** A cuelist or steps were added or changed: the palettes their cues or
     *  steps use (counted in "scenes") may have changed */
    void slotFunctionChanged(quint32 id);
    void slotFixtureRemoved(quint32 id);
    /** The programmer or the palettes changed: the states are made again when read */
    void slotStatesChanged();

private:
    struct Value
    {
        quint32 channel;
        /** The attribute of the channel (programmer key) and the name of its
         *  capability at that value, to use it on other fixture types */
        QString key;
        QString capability;
        uchar value;
    };
    struct Entry
    {
        quint32 fixture;
        QString manufacturer;
        QString model;
        QString mode;
        QList<Value> values;
    };
    struct Palette
    {
        int id;
        QString type;
        int number;
        QString name;
        QColor color;
        /** In the order the fixtures were selected: the fan */
        QList<Entry> entries;
        /** The fixtures it was not recorded on, and their place in its fan
         *  (0 the first entry .. 1 the last), given the first time they take it */
        QHash<quint32, qreal> places;
    };

    Palette *find(int id);
    const Palette *find(int id) const;
    QVariantMap toMap(const Palette &palette) const;
    /** The programmer values of $type on the selection, as entries */
    QList<Entry> entriesFromProgrammer(const QString &type) const;
    bool sameType(const Entry &entry, const Fixture *fixture) const;
    /** A fan: recorded on two fixtures of the same type at least (one
     *  fixture of each type gives what it gives on its own) */
    static bool isFan(const Palette &palette);
    /** $fixture, not recorded in the fan $palette, follows its place in it:
     *  its type has no recorded fixture or several (one alone gives its values) */
    bool takesPlace(const Palette &palette, const Fixture *fixture) const;
    /** The values of an entry: channel -> value */
    static QMap<quint32, uchar> valuesOf(const Entry &entry);
    /** A palette on a fixture it was not recorded for, nor its type; with a
     *  $place in its fan, the colour there and the attributes of the nearest
     *  recorded fixture */
    QMap<quint32, uchar> genericValues(const Palette &palette, const Fixture *fixture, qreal place = -1) const;
    /** The values at $place (0..1) of a fan of entries of the type of $fixture:
     *  pan, dimmer, RGB... between its two neighbours (16 bit with their
     *  fine channels), wheels and other stepped channels from the nearest one */
    QMap<quint32, uchar> fanValues(const QList<const Entry *> &fan, const Fixture *fixture, qreal place) const;
    /** The colour of a palette at $place in its fan: the blend of the colours
     *  of the two recorded fixtures around it */
    QColor fanColour(const Palette &palette, qreal place) const;
    /** The fixtures of $ordered it was not recorded on get their place in
     *  the fan of $palette (with $again, the ones that had one too). What
     *  uses them follows. @return how many got a place */
    int placeFixtures(Palette &palette, const QList<quint32> &ordered, bool again);
    QColor colourOf(const Entry &entry) const;
    /** The colour a palette of $type keeps (colour and "all" palettes): the
     *  one of the first of $entries that has one */
    QColor colourOf(const QString &type, const QList<Entry> &entries) const;
    /** The fixtures a live palette goes on */
    QList<quint32> liveFixtures(const Palette &palette, int groupId) const;
    /** The palette changed: the programmer and the scenes that use it follow */
    void propagate(int id);
    /** The programmer values of the selection now come from the palette */
    void linkProgrammer(int id);
    int nextNumber(const QString &type) const;
    void sort();

private:
    Doc *m_doc;
    Programmer *m_programmer;
    PatchManager *m_patch;
    QList<Palette> m_palettes;
    int m_lastId;
    /** functionId -> (fixture, channel) -> palette */
    QHash<quint32, QHash<QPair<quint32, quint32>, int>> m_sceneRefs;
    /** cuelists and steps: ID -> the palettes they refer to (their references
     *  live in their cues and steps) */
    QHash<quint32, QSet<int>> m_listRefs;
    /** states() and recordable(), made once after each change */
    mutable bool m_statesDirty;
    mutable QVariantMap m_states;
    mutable QVariantMap m_recordable;
};

#endif
