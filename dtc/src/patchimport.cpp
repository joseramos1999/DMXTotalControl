/*
  DMX Total Control
  patchimport.cpp

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

/* PatchManager: the patch import from the fixture table (CSV) a visualizer
 * exports. PatchTable reads the table and finds the fixtures of the library;
 * here the rows become fixtures of the show (new ones, or the ones imported
 * earlier, updated), with their numbers, groups and place in the plan */

#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QVector3D>
#include <QSet>
#include <algorithm>
#include <cmath>

#include "patchmanager.h"
#include "patchtable.h"
#include "qlcfixturedefcache.h"
#include "monitorproperties.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "inputoutputmap.h"
#include "fixturegroup.h"
#include "qlcchannel.h"
#include "universe.h"
#include "fixture.h"
#include "programmer.h"
#include "doc.h"

/* The fixture chosen by hand for each type of a table (the next import of
 * the same type takes it), and the view of the plan used last */
#define KSettingImportChoices QStringLiteral("patchImport/choices")
#define KSettingImportView QStringLiteral("patchImport/view")

/* The room around the rig when the plan grows to hold it, mm */
static const qreal kPlanMargin = 1000.0;
/* The largest stage plan there is (setStageSize), mm */
static const qreal kMaxStage = 200000.0;
/* How far aside a fixture goes from another one on the same spot of the plan, mm */
static const qreal kStackStep = 50.0;
/* The highest universe a table may patch on: a fixture keeps its universe
 * in 7 bits (Fixture::setUniverse) */
static const int kMaxTableUniverse = 128;
/* The highest fixture number (the patch editor's too) */
static const int kMaxImportNumber = 99999;
/* The largest file read as a fixture table (a big rig is a few hundred KB) */
static const qint64 kMaxTableBytes = 16 * 1024 * 1024;

struct PatchManager::TablePlan
{
    /** A fixture type of the table, and what it patches */
    struct Type
    {
        QString key;
        QString fixture;
        QString mode;
        int channels = 0;
        int count = 0;
        /** exact, similar, channels, remembered, manual, generic, skip */
        QString status;
        bool generic = false;
        bool skip = false;
        QLCFixtureDef *def = nullptr;
        QLCFixtureMode *fxMode = nullptr;
        /** Of what is patched */
        int dtcChannels = 1;
        QString note;
        /** Not found: where to look for it by hand */
        QString foundManufacturer;
        QString foundModel;
        /** A generic dimmer: the icon its fixtures get on the plan, the
         *  kind the table's name says ("Acme CO2 Jet": co2); empty: none */
        QString icon;
        /** The glyph its fixtures show on the plan (empty: not imported) */
        QString kind;
    };
    /** A row of the table that is patched */
    struct Item
    {
        int row = -1;
        int type = -1;
        /** The fixture it updates; invalid: a new one */
        quint32 existing = Fixture::invalidId();
        QString name;
        QString label;
        quint32 universe = 0;
        quint32 address = 0;
        int channels = 1;
        int number = 0;
        bool hasPlan = false;
        QPointF plan;
        /** Aim setup height, mm; -1: not set */
        int height = -1;
        /** Aim setup mount: 1 standing, 0 hung, -1 not given */
        int standing = -1;
        QString identifier;
    };

    PatchTable::Table table;
    QString error;
    QList<Type> types;
    /** By universe and address */
    QList<Item> items;
    QStringList problems;
    /** Of the problems, the ones of rows that are imported all the same
     *  (a second DMX break, a fixture number, an inversion): for the message */
    QStringList notes;
    int skippedRows = 0;
    int notImported = 0;
    QList<quint32> removed;
    int removeCount = 0;
    /** Of the fixtures the remove option takes, in the order of their ID */
    QStringList removeNames;
    int universeFrom = 0;
    int universeTo = 0;
    int newUniverses = 0;
    bool place = true;
    QString view;
    QString suggestedView;
    bool hasPositions = false;
    bool groupsByType = true;
    bool groupsByLayer = false;
    bool groupsFromTable = false;
    bool removeMissing = false;
    QSizeF stage;
    QSizeF stageNow;
    /** The choices made by hand, to remember when importing */
    QVariantMap choices;
};

/* A moving head: what Follow aims (the aim setup is kept for these) */
static bool aimableMode(const QLCFixtureMode *mode)
{
    if (mode == nullptr)
        return false;
    for (const QLCChannel *ch : mode->channels())
    {
        if (Programmer::isPositionChannel(ch))
            return true;
    }
    return false;
}

static bool isGenericDef(const QLCFixtureDef *def)
{
    return def == nullptr || (def->manufacturer() == KXMLFixtureGeneric && def->model() == KXMLFixtureGeneric);
}

static QByteArray readFile(const QUrl &file, QString *error)
{
    const QString path = file.isLocalFile() ? file.toLocalFile() : file.toString();
    // (another kind of file chosen with "All files": not read whole)
    if (QFileInfo(path).size() > kMaxTableBytes)
    {
        *error = PatchManager::tr("%1 is too big for a fixture table").arg(QFileInfo(path).fileName());
        return QByteArray();
    }
    QFile f(path);
    if (f.open(QIODevice::ReadOnly) == false)
    {
        *error = PatchManager::tr("Unable to read %1").arg(QFileInfo(path).fileName());
        return QByteArray();
    }
    return f.readAll();
}

QString PatchManager::importIdentifier(quint32 id) const
{
    return m_importIds.value(id);
}

void PatchManager::planTable(const QByteArray &data, const QVariantMap &options, TablePlan *plan) const
{
    plan->table = PatchTable::read(data);
    const PatchTable::Table &table = plan->table;
    if (table.error == PatchTable::Empty)
    {
        plan->error = tr("The file is empty");
        return;
    }
    if (table.error == PatchTable::MissingColumns)
    {
        plan->error = tr("This isn't a fixture table (CSV). Missing: %1")
                          .arg(QStringLiteral("“") + table.missingColumns.join(QStringLiteral("”, “")) + QStringLiteral("”"));
        return;
    }
    if (table.rows.isEmpty())
    {
        plan->error = tr("The table has no fixtures");
        return;
    }

    QLCFixtureDefCache *cache = m_doc->fixtureDefCache();
    const PatchTable::Library library = PatchTable::library(cache);
    QSettings settings;
    const QVariantMap remembered = settings.value(KSettingImportChoices).toMap();
    const QVariantMap given = options.value(QStringLiteral("choices")).toMap();

    plan->place = options.value(QStringLiteral("place"), true).toBool();
    plan->groupsByType = options.value(QStringLiteral("groupsByType"), true).toBool();
    plan->groupsByLayer = options.value(QStringLiteral("groupsByLayer"), table.layers.count() > 1).toBool();
    plan->groupsFromTable = options.value(QStringLiteral("groupsFromTable"), table.hasGroups).toBool();
    plan->removeMissing = options.value(QStringLiteral("removeMissing"), false).toBool();

    /*
     * The types: the fixture chosen by hand, remembered, or found in the
     * library; else a generic dimmer with its channels (on the right
     * addresses all the same)
     */
    auto channelNote = [&](TablePlan::Type &type) {
        if (type.channels > 0 && type.dtcChannels != type.channels)
            type.note = tr("%1 channels in the library, %2 in the table").arg(type.dtcChannels).arg(type.channels);
    };
    auto resolve = [&](TablePlan::Type &type) {
        QVariantMap choice;
        bool byHand = false;
        if (given.contains(type.key))
        {
            choice = given.value(type.key).toMap();
            plan->choices.insert(type.key, choice);
            byHand = true;
        }
        else
        {
            choice = remembered.value(type.key).toMap();
        }
        const QString chosen = byHand ? QStringLiteral("manual") : QStringLiteral("remembered");

        if (choice.value(QStringLiteral("skip")).toBool())
        {
            type.status = QStringLiteral("skip");
            type.skip = true;
            return;
        }
        if (choice.value(QStringLiteral("generic")).toBool())
        {
            type.status = chosen;
            type.generic = true;
            type.dtcChannels = qBound(1, type.channels, UNIVERSE_SIZE);
            return;
        }
        if (choice.contains(QStringLiteral("model")))
        {
            QLCFixtureDef *def = cache->fixtureDef(choice.value(QStringLiteral("manufacturer")).toString(),
                                                   choice.value(QStringLiteral("model")).toString());
            QLCFixtureMode *mode = def != nullptr ? def->mode(choice.value(QStringLiteral("mode")).toString()) : nullptr;
            // (a fixture remembered that the library no longer has: found again)
            if (def != nullptr && mode != nullptr)
            {
                type.status = chosen;
                type.def = def;
                type.fxMode = mode;
                type.dtcChannels = int(mode->channels().count());
                channelNote(type);
                return;
            }
        }

        const PatchTable::Match match = PatchTable::match(library, type.fixture, type.mode, type.channels);
        QLCFixtureDef *def = match.how != PatchTable::NotFound ? cache->fixtureDef(match.manufacturer, match.model) : nullptr;
        QLCFixtureMode *mode = def != nullptr ? def->mode(match.mode) : nullptr;
        if (def != nullptr && mode != nullptr)
        {
            type.status = match.how == PatchTable::Exact ? QStringLiteral("exact")
                        : match.how == PatchTable::ByChannels ? QStringLiteral("channels") : QStringLiteral("similar");
            type.def = def;
            type.fxMode = mode;
            type.dtcChannels = int(mode->channels().count());
            channelNote(type);
            return;
        }
        type.status = QStringLiteral("generic");
        type.generic = true;
        type.dtcChannels = qBound(1, type.channels, UNIVERSE_SIZE);
        type.foundManufacturer = match.foundManufacturer;
        type.foundModel = match.foundModel;
        if (match.foundModel.isEmpty() == false)
            type.note = tr("%1 %2 is in the library, with no mode like this one")
                            .arg(match.foundManufacturer, match.foundModel);
        if (type.channels <= 0)
            type.note = tr("The table doesn't say how many channels it has: 1");
    };

    QHash<QString, int> typeIndex;
    QList<int> rowType;
    for (const PatchTable::Row &row : table.rows)
    {
        const QString key = PatchTable::typeKey(row.fixture, row.mode, row.channels);
        int t = typeIndex.value(key, -1);
        if (t < 0)
        {
            TablePlan::Type type;
            type.key = key;
            type.fixture = row.fixture;
            type.mode = row.mode;
            type.channels = row.channels;
            resolve(type);
            // what the plan shows of them: a generic dimmer standing in for a
            // fixture the library doesn't have gets the kind the table's name
            // says (a CO2 jet, a flame); else the one of its definition. A
            // name that only says "dimmer" chooses nothing: each fixture's
            // own name says what it is ("CO2 1" of a "Generic Dimmer")
            if (type.generic)
            {
                type.icon = kindFromWords(type.fixture);
                if (type.icon == QLatin1String("dimmer"))
                    type.icon.clear();
                type.kind = fixtureKind(KXMLFixtureGeneric, KXMLFixtureGeneric, KXMLFixtureDimmer, row.unit, type.icon);
            }
            else if (type.def != nullptr)
            {
                type.kind = fixtureKind(type.def->manufacturer(), type.def->model(),
                                        QLCFixtureDef::typeToString(type.def->type()), row.unit);
            }
            t = int(plan->types.count());
            plan->types.append(type);
            typeIndex.insert(key, t);
        }
        plan->types[t].count++;
        rowType.append(t);

        if (row.universe > 0)
        {
            plan->universeFrom = plan->universeFrom == 0 ? row.universe : qMin(plan->universeFrom, row.universe);
            plan->universeTo = qMax(plan->universeTo, row.universe);
        }
    }

    /*
     * The rows: where they go, and the fixture of the show each one updates
     * (imported earlier from this table: its identifier; else the same name
     * and fixture type)
     */
    auto sameType = [](const Fixture *fxi, const TablePlan::Type &type) {
        const QLCFixtureDef *def = fxi->fixtureDef();
        if (type.generic)
            return isGenericDef(def);
        return isGenericDef(def) == false && def->manufacturer() == type.def->manufacturer() &&
               def->model() == type.def->model();
    };
    QHash<QString, quint32> byIdentifier;
    for (auto it = m_importIds.cbegin(); it != m_importIds.cend(); ++it)
    {
        if (m_doc->fixture(it.key()) != nullptr)
            byIdentifier.insert(it.value(), it.key());
    }
    QHash<QString, QList<quint32>> byName;
    {
        QList<Fixture *> all = m_doc->fixtures();
        std::sort(all.begin(), all.end(), [](Fixture *a, Fixture *b) { return a->id() < b->id(); });
        for (Fixture *fxi : std::as_const(all))
            byName[fxi->name()].append(fxi->id());
    }

    QList<TablePlan::Item> candidates;
    QSet<quint32> claimed;
    QSet<QString> tableIds;
    for (const PatchTable::Row &row : table.rows)
    {
        if (row.identifier.isEmpty() == false)
            tableIds.insert(row.identifier);
    }
    // the fixture of the show a row is: the one imported from it (its
    // identifier), else one with its name and fixture type that the table
    // has no row for by its identifier. A type not imported has no fixture
    // to compare: its name alone, of the fixtures imported earlier
    auto existingFor = [&](const PatchTable::Row &row, const TablePlan::Type &type, const QString &name) {
        const quint32 known = row.identifier.isEmpty() ? Fixture::invalidId()
                                                       : byIdentifier.value(row.identifier, Fixture::invalidId());
        if (known != Fixture::invalidId() && claimed.contains(known) == false)
            return known;
        const bool byNameOnly = type.def == nullptr && type.generic == false;
        for (quint32 id : byName.value(name))
        {
            if (claimed.contains(id) || tableIds.contains(m_importIds.value(id)))
                continue;
            if (byNameOnly ? m_importIds.contains(id) : sameType(m_doc->fixture(id), type))
                return id;
        }
        return Fixture::invalidId();
    };
    QHash<int, int> perType;
    for (int r = 0; r < table.rows.count(); r++)
    {
        const PatchTable::Row &row = table.rows.at(r);
        const int t = rowType.at(r);
        const TablePlan::Type &type = plan->types.at(t);
        const int n = ++perType[t];
        const QString label = row.unit.isEmpty() ? tr("Line %1").arg(row.line) : row.unit;
        const QString name = row.unit.isEmpty() ? QStringLiteral("%1 %2").arg(row.fixture.isEmpty() ? tr("Fixture") : row.fixture).arg(n)
                                                : row.unit;

        // a row not imported is still in the table: the fixture it is stays
        // (the remove option doesn't take it)
        auto keep = [&]() {
            const quint32 id = existingFor(row, type, name);
            if (id != Fixture::invalidId())
                claimed.insert(id);
        };
        if (type.skip)
        {
            keep();
            plan->notImported++;
            continue;
        }
        if (row.universe <= 0 || row.address <= 0)
        {
            plan->problems.append(tr("%1: no universe or address (not imported)").arg(label));
            plan->skippedRows++;
            keep();
            continue;
        }
        if (row.universe > kMaxTableUniverse)
        {
            plan->problems.append(tr("%1: universe %2 is too high (not imported)").arg(label).arg(row.universe));
            plan->skippedRows++;
            keep();
            continue;
        }
        if (row.address + type.dtcChannels - 1 > UNIVERSE_SIZE)
        {
            plan->problems.append(tr("%1: addresses %2-%3 go past the end of universe %4 (not imported)")
                                      .arg(label).arg(row.address).arg(row.address + type.dtcChannels - 1).arg(row.universe));
            plan->skippedRows++;
            keep();
            continue;
        }

        TablePlan::Item item;
        item.row = r;
        item.type = t;
        item.universe = quint32(row.universe - 1);
        item.address = quint32(row.address - 1);
        item.channels = type.dtcChannels;
        item.label = label;
        item.name = name;
        item.identifier = row.identifier;
        item.existing = existingFor(row, type, name);
        if (item.existing != Fixture::invalidId())
            claimed.insert(item.existing);

        // how high a moving head hangs (its aim setup), placed on the plan or not
        if (row.hasPosition && aimableMode(type.fxMode))
            item.height = qBound(0, qRound(row.position.y()), 100000);
        // and whether it stands on its base: turned upside down in the table
        if (row.hasRotation && aimableMode(type.fxMode))
            item.standing = PatchTable::upsideDown(row.rotation) ? 1 : 0;
        candidates.append(item);
    }

    // what was imported earlier and the table no longer has
    QList<quint32> missing;
    for (auto it = m_importIds.cbegin(); it != m_importIds.cend(); ++it)
    {
        if (m_doc->fixture(it.key()) != nullptr && claimed.contains(it.key()) == false && tableIds.contains(it.value()) == false)
            missing.append(it.key());
    }
    std::sort(missing.begin(), missing.end());
    plan->removeCount = int(missing.count());
    for (quint32 id : std::as_const(missing))
        plan->removeNames.append(m_doc->fixture(id)->name());
    if (plan->removeMissing)
        plan->removed = missing;
    const QSet<quint32> removed(plan->removed.cbegin(), plan->removed.cend());

    /*
     * Overlaps, with the fixtures that stay and between rows: the row that
     * comes first in the table wins. A fixture a row should have updated
     * stays where it is then, so it is looked at again
     */
    QSet<quint32> staying;
    QStringList overlapProblems;
    int overlapSkipped = 0;
    for (bool again = true; again;)
    {
        again = false;
        plan->items.clear();
        overlapProblems.clear();
        overlapSkipped = 0;
        QSet<quint32> updating;
        for (const TablePlan::Item &item : std::as_const(candidates))
        {
            if (item.existing != Fixture::invalidId() && staying.contains(item.existing) == false)
                updating.insert(item.existing);
        }
        for (const TablePlan::Item &item : std::as_const(candidates))
        {
            const quint32 from = item.address;
            const quint32 to = item.address + quint32(item.channels) - 1;
            QString clash;
            for (Fixture *fxi : m_doc->fixtures())
            {
                // (never the fixture the row updates: staying where it is, it
                // would hide what the row overlaps)
                if (fxi->id() == item.existing || updating.contains(fxi->id()) || removed.contains(fxi->id()) ||
                    fxi->universe() != item.universe || fxi->channels() == 0)
                    continue;
                const quint32 start = fxi->address();
                const quint32 end = start + fxi->channels() - 1;
                if (from <= end && start <= to)
                {
                    clash = tr("%1: addresses %2-%3 of universe %4 are used by %5 (not imported)")
                                .arg(item.label).arg(from + 1).arg(to + 1).arg(item.universe + 1).arg(fxi->name());
                    break;
                }
            }
            for (int i = 0; clash.isEmpty() && i < plan->items.count(); i++)
            {
                const TablePlan::Item &other = plan->items.at(i);
                if (other.universe == item.universe && from <= other.address + quint32(other.channels) - 1 &&
                    other.address <= to)
                    clash = tr("%1 overlaps %2 in universe %3 (not imported)")
                                .arg(item.label, other.label).arg(item.universe + 1);
            }
            if (clash.isEmpty() == false)
            {
                overlapProblems.append(clash);
                overlapSkipped++;
                if (item.existing != Fixture::invalidId() && staying.contains(item.existing) == false)
                {
                    staying.insert(item.existing);
                    again = true;
                }
                continue;
            }
            plan->items.append(item);
        }
    }
    plan->problems.append(overlapProblems);
    plan->skippedRows += overlapSkipped;

    // what the rows imported leave out
    auto note = [plan](const QString &text) {
        plan->problems.append(text);
        plan->notes.append(text);
    };
    for (const TablePlan::Item &item : std::as_const(plan->items))
    {
        const PatchTable::Row &row = table.rows.at(item.row);
        if (row.secondBreak)
        {
            const QString where = row.secondUniverse > 0 && row.secondAddress > 0
                                ? QStringLiteral("%1.%2").arg(row.secondUniverse).arg(row.secondAddress)
                                : QStringLiteral("?");
            note(tr("%1: its second DMX break (%2) is not imported").arg(item.label, where));
        }
        if (aimableMode(plan->types.at(item.type).fxMode))
        {
            if (row.invertPanText.isEmpty() == false)
                note(tr("%1: Invert Pan is “%2”, neither yes nor no").arg(item.label, row.invertPanText));
            if (row.invertTiltText.isEmpty() == false)
                note(tr("%1: Invert Tilt is “%2”, neither yes nor no").arg(item.label, row.invertTiltText));
        }
    }

    /*
     * Fixture numbers: the table's when they are free; the ones a fixture
     * updated has; the next ones after them (in patch order)
     */
    QSet<quint32> updating;
    for (const TablePlan::Item &item : std::as_const(plan->items))
    {
        if (item.existing != Fixture::invalidId())
            updating.insert(item.existing);
    }
    QHash<int, QString> owners;
    for (Fixture *fxi : m_doc->fixtures())
    {
        const int number = m_numbers.value(fxi->id(), 0);
        if (number > 0 && updating.contains(fxi->id()) == false && removed.contains(fxi->id()) == false)
            owners.insert(number, fxi->name());
    }
    for (TablePlan::Item &item : plan->items)
    {
        const int number = table.rows.at(item.row).number;
        if (number <= 0)
            continue;
        if (number > kMaxImportNumber)
        {
            note(tr("%1: number %2 is too high: it gets another one").arg(item.label).arg(number));
            continue;
        }
        if (owners.contains(number))
        {
            note(tr("%1: number %2 is taken by %3: it gets another one").arg(item.label).arg(number).arg(owners.value(number)));
            continue;
        }
        item.number = number;
        owners.insert(number, item.name);
    }
    std::stable_sort(plan->items.begin(), plan->items.end(), [](const TablePlan::Item &a, const TablePlan::Item &b) {
        return a.universe != b.universe ? a.universe < b.universe : a.address < b.address;
    });
    for (TablePlan::Item &item : plan->items)
    {
        const int current = item.existing != Fixture::invalidId() ? m_numbers.value(item.existing, 0) : 0;
        if (item.number == 0 && current > 0 && owners.contains(current) == false)
        {
            item.number = current;
            owners.insert(current, item.name);
        }
    }
    int next = 1;
    for (auto it = owners.cbegin(); it != owners.cend(); ++it)
        next = qMax(next, it.key() + 1);
    for (TablePlan::Item &item : plan->items)
    {
        if (item.number == 0)
            item.number = next++;
    }

    /*
     * The plan: from above (x across, z towards the audience: the bottom of
     * the plan) or from the front (x across, the highest at the top). The
     * plan grows to hold the rig, which goes in its middle
     */
    plan->stageNow = stageSize();
    plan->stage = plan->stageNow;
    float minY = 0, maxY = 0, minZ = 0, maxZ = 0;
    for (const TablePlan::Item &item : std::as_const(plan->items))
    {
        const PatchTable::Row &row = table.rows.at(item.row);
        if (row.hasPosition == false)
            continue;
        if (plan->hasPositions == false)
        {
            minY = maxY = row.position.y();
            minZ = maxZ = row.position.z();
        }
        plan->hasPositions = true;
        minY = qMin(minY, row.position.y());
        maxY = qMax(maxY, row.position.y());
        minZ = qMin(minZ, row.position.z());
        maxZ = qMax(maxZ, row.position.z());
    }
    // hung at different heights more than spread front to back: seen from the front
    plan->suggestedView = plan->hasPositions && maxY - minY > maxZ - minZ ? QStringLiteral("front") : QStringLiteral("top");
    plan->view = options.value(QStringLiteral("view")).toString();
    if (plan->view != QLatin1String("top") && plan->view != QLatin1String("front"))
        plan->view = settings.value(KSettingImportView).toString();
    if (plan->view != QLatin1String("top") && plan->view != QLatin1String("front"))
        plan->view = plan->suggestedView;

    if (plan->place && plan->hasPositions)
    {
        const bool front = plan->view == QLatin1String("front");
        auto across = [](const PatchTable::Row &row) { return qreal(row.position.x()); };
        auto down = [front](const PatchTable::Row &row) {
            return front ? -qreal(row.position.y()) : qreal(row.position.z());
        };
        qreal minU = 0, maxU = 0, minV = 0, maxV = 0;
        bool first = true;
        for (const TablePlan::Item &item : std::as_const(plan->items))
        {
            const PatchTable::Row &row = table.rows.at(item.row);
            if (row.hasPosition == false)
                continue;
            const qreal u = across(row), v = down(row);
            minU = first ? u : qMin(minU, u);
            maxU = first ? u : qMax(maxU, u);
            minV = first ? v : qMin(minV, v);
            maxV = first ? v : qMax(maxV, v);
            first = false;
        }
        // in whole metres, never smaller than it is
        auto fit = [](qreal span, qreal now) {
            const qreal needed = std::ceil((span + 2 * kPlanMargin) / 1000.0) * 1000.0;
            return qMin(kMaxStage, qMax(now, needed));
        };
        plan->stage = QSizeF(fit(maxU - minU, plan->stageNow.width()), fit(maxV - minV, plan->stageNow.height()));
        const qreal cu = (minU + maxU) / 2, cv = (minV + maxV) / 2;
        for (TablePlan::Item &item : plan->items)
        {
            const PatchTable::Row &row = table.rows.at(item.row);
            if (row.hasPosition == false)
                continue;
            item.hasPlan = true;
            item.plan = QPointF(qBound(0.0, plan->stage.width() / 2 + across(row) - cu, plan->stage.width()),
                                qBound(0.0, plan->stage.height() / 2 + down(row) - cv, plan->stage.height()));
        }

        // fixtures one over the other (from above) on one spot: each one
        // a few cm aside, so it keeps a spot of its own (the plan places
        // again the fixtures that share one, see autoLayout)
        QSet<QPair<int, int>> spots;
        MonitorProperties *monitor = m_doc->monitorProperties();
        for (Fixture *fxi : m_doc->fixtures())
        {
            if (updating.contains(fxi->id()) || removed.contains(fxi->id()) || monitor->containsFixture(fxi->id()) == false)
                continue;
            const QVector3D p = monitor->fixturePosition(fxi->id(), 0, 0);
            spots.insert(qMakePair(qRound(p.x()), qRound(p.y())));
        }
        for (TablePlan::Item &item : plan->items)
        {
            if (item.hasPlan == false)
                continue;
            const QPointF spot = item.plan;
            for (int k = 1; spots.contains(qMakePair(qRound(item.plan.x()), qRound(item.plan.y()))) && k < 1000; k++)
            {
                const qreal dx = (k % 2 == 1 ? 1 : -1) * ((k + 1) / 2) * kStackStep;
                item.plan = QPointF(qBound(0.0, spot.x() + dx, plan->stage.width()), spot.y());
            }
            spots.insert(qMakePair(qRound(item.plan.x()), qRound(item.plan.y())));
        }
    }

    // the universes the show doesn't have yet
    quint32 needed = 0;
    for (const TablePlan::Item &item : std::as_const(plan->items))
        needed = qMax(needed, item.universe + 1);
    const quint32 count = m_doc->inputOutputMap()->universesCount();
    plan->newUniverses = needed > count ? int(needed - count) : 0;
    if (plan->newUniverses > 0)
        plan->problems.prepend(tr("%n universe(s) will be added to the show", "", plan->newUniverses));
}

QVariantMap PatchManager::previewMap(const TablePlan &plan) const
{
    QVariantMap map;
    map.insert("ok", plan.error.isEmpty());
    map.insert("error", plan.error);
    if (plan.error.isEmpty() == false)
        return map;

    map.insert("rows", plan.table.rows.count());
    map.insert("universeFrom", plan.universeFrom);
    map.insert("universeTo", plan.universeTo);
    QVariantList types;
    for (const TablePlan::Type &type : plan.types)
    {
        QVariantMap t;
        t.insert("key", type.key);
        t.insert("fixture", type.fixture);
        t.insert("mode", type.mode);
        t.insert("channels", type.channels);
        t.insert("count", type.count);
        t.insert("status", type.status);
        t.insert("generic", type.generic);
        t.insert("manufacturer", type.def != nullptr ? type.def->manufacturer() : QString());
        t.insert("model", type.def != nullptr ? type.def->model() : QString());
        t.insert("modeName", type.fxMode != nullptr ? type.fxMode->name() : QString());
        t.insert("modeChannels", type.skip ? 0 : type.dtcChannels);
        t.insert("note", type.note);
        t.insert("foundManufacturer", type.foundManufacturer);
        t.insert("foundModel", type.foundModel);
        t.insert("icon", type.icon);
        t.insert("kind", type.kind);
        types.append(t);
    }
    map.insert("types", types);
    int updates = 0;
    for (const TablePlan::Item &item : plan.items)
        updates += item.existing != Fixture::invalidId() ? 1 : 0;
    map.insert("newCount", plan.items.count() - updates);
    map.insert("updateCount", updates);
    map.insert("skippedRows", plan.skippedRows);
    map.insert("notImported", plan.notImported);
    map.insert("removeCount", plan.removeCount);
    map.insert("removeNames", plan.removeNames);
    map.insert("problems", plan.problems);
    map.insert("layers", plan.table.layers);
    map.insert("hasGroups", plan.table.hasGroups);
    map.insert("hasPositions", plan.hasPositions);
    map.insert("view", plan.view);
    map.insert("suggestedView", plan.suggestedView);
    map.insert("stage", QVariantMap({ { "width", plan.stage.width() }, { "height", plan.stage.height() } }));
    map.insert("stageNow", QVariantMap({ { "width", plan.stageNow.width() }, { "height", plan.stageNow.height() } }));
    map.insert("newUniverses", plan.newUniverses);
    map.insert("options", QVariantMap({
        { "place", plan.place },
        { "view", plan.view },
        { "groupsByType", plan.groupsByType },
        { "groupsByLayer", plan.groupsByLayer },
        { "groupsFromTable", plan.groupsFromTable },
        { "removeMissing", plan.removeMissing },
    }));
    return map;
}

QVariantMap PatchManager::tablePreview(const QUrl &file, const QVariantMap &options)
{
    QString error;
    const QByteArray data = readFile(file, &error);
    if (error.isEmpty() == false)
        return QVariantMap({ { "ok", false }, { "error", error } });
    QVariantMap map = tablePreviewData(data, options);
    map.insert("fileName", QFileInfo(file.isLocalFile() ? file.toLocalFile() : file.toString()).fileName());
    return map;
}

QVariantMap PatchManager::tablePreviewData(const QByteArray &data, const QVariantMap &options)
{
    TablePlan plan;
    planTable(data, options, &plan);
    return previewMap(plan);
}

QString PatchManager::importTable(const QUrl &file, const QVariantMap &options)
{
    QString error;
    const QByteArray data = readFile(file, &error);
    if (error.isEmpty() == false)
    {
        m_lastError = error;
        return QString();
    }
    return importTableData(data, options);
}

QString PatchManager::importTableData(const QByteArray &data, const QVariantMap &options)
{
    m_lastError.clear();
    TablePlan plan;
    planTable(data, options, &plan);
    if (plan.error.isEmpty() == false)
    {
        m_lastError = plan.error;
        return QString();
    }
    if (plan.items.isEmpty() && plan.removed.isEmpty())
    {
        m_lastError = plan.problems.isEmpty() ? tr("Nothing to import") : tr("Nothing to import: %1").arg(plan.problems.last());
        return QString();
    }

    InputOutputMap *ioMap = m_doc->inputOutputMap();
    MonitorProperties *monitor = m_doc->monitorProperties();

    // what the table no longer has
    int removed = 0;
    for (quint32 id : std::as_const(plan.removed))
        removed += m_doc->deleteFixture(id) ? 1 : 0;

    quint32 needed = 0;
    for (const TablePlan::Item &item : std::as_const(plan.items))
        needed = qMax(needed, item.universe + 1);
    while (needed > ioMap->universesCount())
    {
        ioMap->addUniverse();
        ioMap->startUniverses();
    }

    // the fixtures updated first: the places they leave may be the ones of new fixtures
    QList<quint32> ids;
    int updated = 0, added = 0, failed = 0;
    // the ones that become another fixture (a generic dimmer, or another of
    // the library): the icon chosen by hand for what they were goes
    QSet<quint32> otherFixture;
    for (const TablePlan::Item &item : std::as_const(plan.items))
    {
        Fixture *fxi = item.existing != Fixture::invalidId() ? m_doc->fixture(item.existing) : nullptr;
        if (fxi == nullptr)
            continue;
        const TablePlan::Type &type = plan.types.at(item.type);
        const QLCFixtureDef *before = fxi->fixtureDef();
        if (isGenericDef(before) != type.generic ||
            (type.generic == false && (before->manufacturer() != type.def->manufacturer() ||
                                       before->model() != type.def->model())))
            otherFixture.insert(fxi->id());
        QLCFixtureDef *def = type.def;
        QLCFixtureMode *mode = type.fxMode;
        if (type.generic)
        {
            def = nullptr;
            mode = nullptr;
            if (isGenericDef(fxi->fixtureDef()) == false || int(fxi->channels()) != item.channels)
            {
                def = fxi->genericDimmerDef(item.channels);
                mode = fxi->genericDimmerMode(def, item.channels);
            }
        }
        fxi->setName(item.name);
        applyPatch(m_doc, fxi, def, mode, item.universe, item.address);
        updated++;
    }
    for (const TablePlan::Item &item : std::as_const(plan.items))
    {
        if (item.existing != Fixture::invalidId() && m_doc->fixture(item.existing) != nullptr)
        {
            ids.append(item.existing);
            continue;
        }
        const TablePlan::Type &type = plan.types.at(item.type);
        Fixture *fxi = new Fixture(m_doc);
        fxi->setUniverse(item.universe);
        fxi->setAddress(item.address);
        if (type.generic)
        {
            QLCFixtureDef *def = fxi->genericDimmerDef(item.channels);
            fxi->setFixtureDefinition(def, fxi->genericDimmerMode(def, item.channels));
        }
        else
        {
            fxi->setFixtureDefinition(type.def, type.fxMode);
        }
        fxi->setName(item.name);
        if (m_doc->addFixture(fxi) == false)
        {
            // (checked when planning: Doc refuses it all the same)
            delete fxi;
            ids.append(Fixture::invalidId());
            failed++;
            continue;
        }
        ids.append(fxi->id());
        added++;
    }

    // what each one keeps: the row it came from, its number, its place and how it hangs
    QSet<QString> identifiers;
    if (plan.place && plan.hasPositions &&
        (plan.stage.width() > plan.stageNow.width() || plan.stage.height() > plan.stageNow.height()))
        setStageSize(plan.stage.width(), plan.stage.height());
    for (int i = 0; i < plan.items.count(); i++)
    {
        const quint32 id = ids.at(i);
        if (id == Fixture::invalidId())
            continue;
        const TablePlan::Item &item = plan.items.at(i);
        const PatchTable::Row &row = plan.table.rows.at(item.row);
        if (item.identifier.isEmpty() == false && identifiers.contains(item.identifier) == false)
        {
            // (the same identifier twice in a table: the first row keeps it)
            identifiers.insert(item.identifier);
            m_importIds.insert(id, item.identifier);
        }
        m_numbers.insert(id, item.number);
        if (item.hasPlan)
            monitor->setFixturePosition(id, 0, 0, QVector3D(float(item.plan.x()), float(item.plan.y()), 0));

        // its icon: a generic dimmer, the kind the table's name says. One
        // chosen by hand since the last import stays, unless the type's
        // match changed (another icon, another fixture)
        const TablePlan::Type &type = plan.types.at(item.type);
        const QString icon = type.generic ? type.icon : QString();
        if (otherFixture.contains(id) || m_importIcons.value(id) != icon)
        {
            if (icon.isEmpty())
                m_icons.remove(id);
            else
                m_icons.insert(id, icon);
        }
        if (icon.isEmpty())
            m_importIcons.remove(id);
        else
            m_importIcons.insert(id, icon);

        if (aimableMode(plan.types.at(item.type).fxMode))
        {
            AimSetup setup = m_aims.value(id);
            if (item.height >= 0)
                setup.height = item.height;
            if (item.standing >= 0)
                setup.standing = item.standing == 1;
            if (row.invertPanGiven)
                setup.panInverted = row.invertPan;
            if (row.invertTiltGiven)
                setup.tiltInverted = row.invertTilt;
            if (setup.isDefault())
                m_aims.remove(id);
            else
                m_aims.insert(id, setup);
        }
    }

    // the groups: per fixture type, per layer, the table's own; a group of
    // that name already there gets the ones it doesn't have
    QList<QPair<QString, QList<quint32>>> groups;
    auto addTo = [&groups](const QString &name, quint32 id) {
        const QString trimmed = name.trimmed();
        if (trimmed.isEmpty() || id == Fixture::invalidId())
            return;
        for (auto &group : groups)
        {
            if (group.first == trimmed)
            {
                group.second.append(id);
                return;
            }
        }
        groups.append(qMakePair(trimmed, QList<quint32>({ id })));
    };
    for (int i = 0; i < plan.items.count(); i++)
    {
        const TablePlan::Item &item = plan.items.at(i);
        const PatchTable::Row &row = plan.table.rows.at(item.row);
        if (plan.groupsByType)
            addTo(row.fixture, ids.at(i));
        if (plan.groupsByLayer)
            addTo(row.layer, ids.at(i));
        if (plan.groupsFromTable)
        {
            for (const QString &name : row.groups)
                addTo(name, ids.at(i));
        }
    }
    for (const auto &group : std::as_const(groups))
    {
        FixtureGroup *existing = nullptr;
        for (FixtureGroup *grp : m_doc->fixtureGroups())
        {
            if (grp->name() == group.first)
            {
                existing = grp;
                break;
            }
        }
        QVariantList members;
        if (existing != nullptr)
        {
            for (quint32 fid : existing->fixtureList())
                members.append(fid);
        }
        for (quint32 fid : group.second)
        {
            if (members.contains(fid) == false)
                members.append(fid);
        }
        if (existing != nullptr)
            setGroupFixtures(existing->id(), members);
        else
            createGroup(group.first, members);
    }

    // the new ones the table gives no place (or not placed): in the free slots of the plan
    autoLayout(false);

    // a generic dimmer given the icon of a CO2 jet (the table's name) is a
    // protected special effect (as one patched with its name: not told)
    QList<quint32> imported;
    for (quint32 id : std::as_const(ids))
        if (id != Fixture::invalidId())
            imported.append(id);
    recheckKinds(imported, false);

    // remembered for the next import: the fixtures chosen by hand, the view
    QSettings settings;
    if (plan.choices.isEmpty() == false)
    {
        QVariantMap remembered = settings.value(KSettingImportChoices).toMap();
        for (auto it = plan.choices.cbegin(); it != plan.choices.cend(); ++it)
        {
            if (it.value().toMap().value(QStringLiteral("auto")).toBool())
                remembered.remove(it.key());
            else
                remembered.insert(it.key(), it.value());
        }
        settings.setValue(KSettingImportChoices, remembered);
    }
    if (plan.place && plan.hasPositions)
        settings.setValue(KSettingImportView, plan.view);

    m_doc->setModified();
    m_doc->refreshChannelCapabilities();
    notifyFixtures();
    emit groupsChanged();

    // (each count with its own plural)
    QString done = tr("Imported %n fixture(s): %1, %2", "", added + updated)
                       .arg(tr("%n new", "", added), tr("%n updated", "", updated));
    if (removed > 0)
        done += QStringLiteral(", ") + tr("%n removed", "", removed);
    const int notImported = plan.skippedRows + failed;
    if (notImported > 0)
        done += QStringLiteral(" · ") + tr("%n row(s) of the table not imported", "", notImported);
    // what the rows imported leave out: the preview that listed it is gone
    if (plan.notes.count() == 1)
        done += QStringLiteral(" · ") + plan.notes.first();
    else if (plan.notes.count() > 1)
        done += QStringLiteral(" · ") + tr("%n problem(s)", "", int(plan.notes.count()));
    return done;
}
