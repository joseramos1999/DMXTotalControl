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
 * and beam values recorded from the programmer, for each fixture.
 *
 * They go by reference: the scenes recorded with a palette remember it, so
 * correcting the palette (refocusing "Singer") changes every scene that
 * uses it. On a fixture the palette was not recorded for, it uses the values
 * of a fixture of the same type, else the same attributes (and, for colours,
 * the mix of its colour).
 *
 * Palettes are part of the show (DMX Total Control section): the scenes keep
 * their plain values too, so QLC+ still plays them.
 */
class PaletteManager : public QObject
{
    Q_OBJECT

    /** [{ id, type, number, name, color, fixtures, scenes }] by type and number
     *  (scenes: the scenes and the cuelists that use it) */
    Q_PROPERTY(QVariantList palettes READ palettes NOTIFY palettesChanged)
    /** How each palette is on the selection: { "<id>": 0 none, 1 some, 2 all } */
    Q_PROPERTY(QVariantMap states READ states NOTIFY statesChanged)
    /** How many programmer values of the selection each type would record: { type: n } */
    Q_PROPERTY(QVariantMap recordable READ recordable NOTIFY statesChanged)

public:
    PaletteManager(Doc *doc, Programmer *programmer, PatchManager *patch, QObject *parent = nullptr);

    /** "intensity", "position", "colour", "beam" */
    Q_INVOKABLE static QStringList types();
    /** The type of palette that records a channel, or "" (maintenance, nothing) */
    static QString typeOf(const QLCChannel *channel);

    QVariantList palettes() const;
    QVariantMap states() const;
    QVariantMap recordable() const;
    /** One palette, like in palettes(), or an empty map */
    Q_INVOKABLE QVariantMap palette(int id) const;
    /** The palette $number of $type, or -1 */
    Q_INVOKABLE int paletteByNumber(const QString &type, int number) const;

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

    /** The palette on the selection of the programmer. @return how many fixtures took it */
    Q_INVOKABLE int apply(int id);
    /** Select the fixtures of the palette and apply it, to change it and update it */
    Q_INVOKABLE bool load(int id);

    /** Live (SHOW, mappings): the palette as a live edit of the programmer on a
     *  fixture group, or its own fixtures with $groupId -1 */
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
    /** Once a show is loaded: the cues that name a palette that isn't there
     *  (a damaged or edited file) keep their values as plain values, like
     *  when it is deleted; and no palette made later gets an ID up to
     *  $usedId (what the SHOW and the mappings still name) */
    void afterLoad(int usedId);

signals:
    void palettesChanged();
    void statesChanged();

private slots:
    void slotFunctionRemoved(quint32 id);
    /** A cuelist was added or changed: the palettes its cues use (counted
     *  in "scenes") may have changed */
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
        QList<Entry> entries;
    };

    Palette *find(int id);
    const Palette *find(int id) const;
    QVariantMap toMap(const Palette &palette) const;
    /** The programmer values of $type on the selection, as entries */
    QList<Entry> entriesFromProgrammer(const QString &type) const;
    bool sameType(const Entry &entry, const Fixture *fixture) const;
    /** A palette on a fixture it was not recorded for, nor its type */
    QMap<quint32, uchar> genericValues(const Palette &palette, const Fixture *fixture) const;
    QColor colourOf(const Entry &entry) const;
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
    /** cuelist ID -> the palettes its cues refer to (the refs live in the cues) */
    QHash<quint32, QSet<int>> m_cuelistRefs;
    /** states() and recordable(), made once after each change */
    mutable bool m_statesDirty;
    mutable QVariantMap m_states;
    mutable QVariantMap m_recordable;
};

#endif
