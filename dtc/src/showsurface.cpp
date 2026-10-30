/*
  DMX Total Control
  showsurface.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QDebug>
#include <cmath>
#include <functional>

#include "showsurface.h"
#include "zonemanager.h"
#include "scenetaps.h"
#include "zone.h"
#include "livemixer.h"
#include "dtccore.h"
#include "programmer.h"
#include "controlmapper.h"
#include "playbackmanager.h"
#include "drivecontrol.h"

#include "functionparent.h"
#include "fxscene.h"
#include "efx.h"
#include "efxfixture.h"
#include "chaser.h"
#include "chaserstep.h"
#include "scene.h"
#include "fixturegroup.h"
#include "qlcchannel.h"
#include "fixture.h"
#include "mastertimer.h"
#include "inputoutputmap.h"
#include "function.h"
#include "cuelist.h"
#include "show.h"
#include "doc.h"

#define KXMLDTCSurfacePage   QStringLiteral("Page")
#define KXMLDTCSurfaceSplit   QStringLiteral("Split")
#define KXMLDTCSurfaceSection QStringLiteral("Section")
#define KXMLDTCSurfaceItem    QStringLiteral("Item")
#define KXMLDTCSurfaceTarget  QStringLiteral("Target")
#define KXMLDTCSurfaceWidget  QStringLiteral("Widget")   // version 1 pages
#define KXMLDTCSurfaceDimmer  QStringLiteral("SceneDimmer")

ShowSurface::ShowSurface(Doc *doc, ZoneManager *zones, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_zones(zones)
    , m_lastId(0)
    , m_revision(0)
    , m_mixer(nullptr)
    , m_core(nullptr)
    , m_currentPage(0)
{
    connect(m_doc->masterTimer(), SIGNAL(functionListChanged()), this, SLOT(slotChanged()));
    connect(m_zones, SIGNAL(countChanged()), this, SLOT(slotZonesChanged()));
    connect(m_zones, SIGNAL(modelReset()), this, SLOT(slotZonesChanged()));
    // the movement speed faders follow the patch (engine signals by name: MinGW)
    connect(m_doc, SIGNAL(fixtureAdded(quint32)), this, SLOT(slotFixturesChanged()));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotFixturesChanged()));
    connect(m_doc, SIGNAL(fixtureChanged(quint32)), this, SLOT(slotFixturesChanged()));
    connect(m_doc, SIGNAL(fixtureGroupChanged(quint32)), this, SLOT(slotFixturesChanged()));
    connect(m_doc, SIGNAL(fixtureGroupRemoved(quint32)), this, SLOT(slotFixturesChanged()));
    // (the level of a scene fader is of the function it was given to)
    connect(m_doc, SIGNAL(functionRemoved(quint32)), this, SLOT(slotFunctionGone(quint32)));
    connect(m_doc, SIGNAL(functionReplaced(quint32)), this, SLOT(slotFunctionGone(quint32)));
    connect(m_doc, SIGNAL(functionRemoved(quint32)), this, SLOT(slotFunctionRemoved(quint32)));
    // (the dimmers of the scenes are the base of their intensity)
    connect(m_doc, SIGNAL(functionAdded(quint32)), this, SLOT(slotFunctionAdded(quint32)));
    connect(m_doc, SIGNAL(functionChanged(quint32)), this, SLOT(slotFunctionChanged(quint32)));
    // (and the movement speed faders on the functions that move heads)
    connect(m_doc, SIGNAL(functionAdded(quint32)), this, SLOT(slotMotionChanged()));
    connect(m_doc, SIGNAL(functionChanged(quint32)), this, SLOT(slotMotionChanged()));
    slotZonesChanged();
}

ShowSurface::~ShowSurface()
{
}

void ShowSurface::setMixer(LiveMixer *mixer)
{
    m_mixer = mixer;
}

/*********************************************************************
 * Tree helpers
 *********************************************************************/

QString ShowSurface::nextId(const QString &prefix)
{
    return prefix + QString::number(++m_lastId);
}

QVariantMap ShowSurface::sectionDefaults(const QString &type)
{
    QVariantMap s;
    s.insert("type", type);
    s.insert("title", QString());
    if (type == "buttons")
    {
        s.insert("cols", 6);
        s.insert("rows", 4);
    }
    else if (type == "faders")
    {
        s.insert("cols", 8);
    }
    else if (type == "group")
    {
        s.insert("zoneId", -1);
    }
    else if (type == "clock")
    {
        s.insert("seconds", true);
        s.insert("date", true);
    }
    else if (type == "timer")
    {
        s.insert("mode", QString("stopwatch"));
        s.insert("minutes", 5);
    }
    else if (type == "notes")
    {
        s.insert("text", QString());
        s.insert("fontSize", 20);
    }
    else if (type == "monitor")
    {
        s.insert("universe", 0);
    }
    else if (type == "dials")
    {
        s.insert("cols", 4);
        s.insert("rows", 1);
    }
    else if (type == "colour" || type == "xy")
    {
        s.insert("groupId", -1);
    }
    else if (type == "image")
    {
        s.insert("image", QString());
        s.insert("fit", QString("fit"));
    }
    else if (type == "playbacks")
    {
        s.insert("cols", 8);
        s.insert("playbackPage", -1);
        s.insert("first", 0);
        s.insert("showBack", true);
        s.insert("showFlash", true);
        s.insert("showCue", true);
    }
    return s;
}

QVariantMap ShowSurface::itemDefaults(const QString &sectionType)
{
    QVariantMap item;
    item.insert("label", QString());
    item.insert("color", QString("#3d8bfd"));
    item.insert("functionId", -1);
    item.insert("zoneId", -1);
    if (sectionType == "faders" || sectionType == "dials")
    {
        item.insert("target", QString("grandMaster"));
        // dials: a dial, or an incremental wheel that adds "step" % a notch
        if (sectionType == "dials")
        {
            item.insert("encoder", false);
            item.insert("step", 2);
        }
    }
    else
    {
        item.insert("action", QString("scene"));
        item.insert("mode", QString("toggle"));
        item.insert("key", QString());
    }
    return item;
}

std::unique_ptr<ShowSurface::Node> ShowSurface::newSection(const QString &type)
{
    auto node = std::make_unique<Node>();
    node->id = nextId("s");
    node->split = "none";
    node->props = sectionDefaults(type.isEmpty() ? QString("empty") : type);
    return node;
}

ShowSurface::Node *ShowSurface::find(Node *node, const QString &id, Node **parent) const
{
    if (node == nullptr)
        return nullptr;
    if (node->id == id)
        return node;
    for (Node *child : { node->a.get(), node->b.get() })
    {
        if (child == nullptr)
            continue;
        if (child->id == id)
        {
            if (parent)
                *parent = node;
            return child;
        }
        if (Node *found = find(child, id, parent))
            return found;
    }
    return nullptr;
}

ShowSurface::Node *ShowSurface::findSection(int page, const QString &id, Node **parent) const
{
    if (page < 0 || page >= int(m_pages.size()))
        return nullptr;
    if (parent)
        *parent = nullptr;
    return find(m_pages[size_t(page)].root.get(), id, parent);
}

QVariantMap ShowSurface::toMap(const Node *node) const
{
    QVariantMap map;
    if (node == nullptr)
        return map;

    if (node->split == "none")
    {
        map = node->props;
        map.insert("split", "none");
    }
    else
    {
        map.insert("split", node->split);
        map.insert("ratio", node->ratio);
        map.insert("a", toMap(node->a.get()));
        map.insert("b", toMap(node->b.get()));
    }
    map.insert("id", node->id);
    return map;
}

void ShowSurface::changed()
{
    pruneSceneDimmers();
    pruneHolds();
    m_doc->setModified();
    emit pagesChanged();
}

QSet<quint32> ShowSurface::controlledScenes(const QStringList &kinds, bool shownOnly) const
{
    QSet<quint32> scenes;
    std::function<void(const Node *)> walk = [&](const Node *node) {
        if (node == nullptr)
            return;
        walk(node->a.get());
        walk(node->b.get());
        // (a table made smaller keeps the cells out of it, unseen)
        const QString type = node->props.value("type").toString();
        const int cols = qMax(1, node->props.value("cols", 1).toInt());
        const int shown = type == "dials" ? cols * qMax(1, node->props.value("rows", 1).toInt()) : cols;
        const QVariantMap items = node->props.value("items").toMap();
        for (auto it = items.cbegin(); it != items.cend(); ++it)
        {
            if (shownOnly && ((type != "faders" && type != "dials") || it.key().toInt() >= shown))
                continue;
            for (const QVariant &v : faderTargets(it.value().toMap()))
            {
                const QVariantMap target = v.toMap();
                if (kinds.contains(target.value("target").toString()) && target.value("functionId", -1).toInt() >= 0)
                    scenes.insert(target.value("functionId").toUInt());
            }
        }
    };
    for (const Page &page : m_pages)
        walk(page.root.get());
    return scenes;
}

void ShowSurface::pruneSceneDimmers()
{
    if (m_sceneDimmers.isEmpty())
        return;

    // the scenes some fader or dial still dims
    const QSet<quint32> dimmed = controlledScenes({ "sceneDimmer" });

    // a dimmer nothing shows any more (its fader deleted or on another
    // scene) would dim its scene where no one can see why: back to full
    for (auto it = m_sceneDimmers.begin(); it != m_sceneDimmers.end();)
    {
        if (dimmed.contains(it.key()))
        {
            ++it;
            continue;
        }
        applySceneDimmer(m_doc->function(it.key()), 1.0);
        it = m_sceneDimmers.erase(it);
    }
}

bool ShowSurface::holds(quint32 fid) const
{
    if (zoneFor(fid) != nullptr)
        return m_zones->isHeld(fid);
    return m_levels.contains(fid) || m_runSpeeds.contains(fid);
}

QSet<quint32> ShowSurface::heldScenes() const
{
    // in their groups, and on their own (a fader plays them; one that went
    // into a group since is the group's)
    QSet<quint32> held;
    for (int i = 0; i < m_zones->count(); i++)
    {
        for (const quint32 fid : m_zones->zone(i)->heldFunctions())
            held.insert(fid);
    }
    for (auto it = m_levels.cbegin(); it != m_levels.cend(); ++it)
    {
        if (zoneFor(it.key()) == nullptr)
            held.insert(it.key());
    }
    for (auto it = m_runSpeeds.cbegin(); it != m_runSpeeds.cend(); ++it)
    {
        if (zoneFor(it.key()) == nullptr)
            held.insert(it.key());
    }
    return held;
}

void ShowSurface::pruneHolds()
{
    if (heldScenes().isEmpty())
        return;

    // a scene no fader or dial plays any more (its fader deleted, on another
    // scene, or out of its table, made smaller) would be held up for good:
    // it plays on, as any other
    const QSet<quint32> played = controlledScenes({ "scene", "sceneRun" }, true);
    for (int i = 0; i < m_zones->count(); i++)
    {
        Zone *zone = m_zones->zone(i);
        for (const quint32 fid : zone->heldFunctions())
        {
            if (played.contains(fid) == false)
                zone->dropHold(fid);
        }
    }
    for (const quint32 fid : m_levels.keys())
    {
        if (played.contains(fid) == false)
            m_levels.remove(fid);
    }
    // (at the speed it had)
    for (const quint32 fid : m_runSpeeds.keys())
    {
        if (played.contains(fid) == false)
            restoreRunSpeed(fid);
    }
}

/*********************************************************************
 * Pages and sections
 *********************************************************************/

QVariantList ShowSurface::pages() const
{
    QVariantList list;
    for (const Page &page : m_pages)
    {
        QVariantMap map;
        map.insert("name", page.name);
        map.insert("root", toMap(page.root.get()));
        list.append(map);
    }
    return list;
}

int ShowSurface::pageCount() const
{
    return int(m_pages.size());
}

int ShowSurface::addPage(const QString &name, const QString &type)
{
    Page page;
    page.name = name.isEmpty() ? tr("Page %1").arg(m_pages.size() + 1) : name;
    page.root = newSection(type.isEmpty() ? QString("buttons") : type);
    m_pages.push_back(std::move(page));
    changed();
    return int(m_pages.size()) - 1;
}

void ShowSurface::renamePage(int page, const QString &name)
{
    if (page < 0 || page >= int(m_pages.size()) || name.trimmed().isEmpty())
        return;
    m_pages[size_t(page)].name = name.trimmed();
    changed();
}

void ShowSurface::removePage(int page)
{
    if (page < 0 || page >= int(m_pages.size()))
        return;
    m_pages.erase(m_pages.begin() + page);
    changed();
}

void ShowSurface::movePage(int from, int to)
{
    int count = int(m_pages.size());
    if (from < 0 || from >= count || to < 0 || to >= count || from == to)
        return;
    Page moving = std::move(m_pages[size_t(from)]);
    m_pages.erase(m_pages.begin() + from);
    m_pages.insert(m_pages.begin() + to, std::move(moving));
    changed();
}

QString ShowSurface::splitSection(int page, const QString &sectionId, const QString &orientation)
{
    Node *node = findSection(page, sectionId);
    if (node == nullptr || node->split != "none")
        return QString();

    // the section becomes a split: its content goes to the first half
    auto first = std::make_unique<Node>();
    first->id = node->id;
    first->split = "none";
    first->props = node->props;

    auto second = newSection("empty");
    QString secondId = second->id;

    node->id = nextId("n");
    node->split = orientation == "v" ? "v" : "h";
    node->ratio = 0.5;
    node->props.clear();
    node->a = std::move(first);
    node->b = std::move(second);
    changed();
    return secondId;
}

void ShowSurface::removeSection(int page, const QString &sectionId)
{
    Node *parent = nullptr;
    Node *node = findSection(page, sectionId, &parent);
    if (node == nullptr || node->split != "none")
        return;

    if (parent == nullptr)
    {
        // the only section of the page: it just empties
        node->props = sectionDefaults("empty");
        changed();
        return;
    }

    // the other half takes the place of the split
    std::unique_ptr<Node> other = parent->a.get() == node ? std::move(parent->b) : std::move(parent->a);
    parent->id = other->id;
    parent->split = other->split;
    parent->ratio = other->ratio;
    parent->props = other->props;
    parent->a = std::move(other->a);
    parent->b = std::move(other->b);
    changed();
}

void ShowSurface::setSplitRatio(int page, const QString &splitId, qreal ratio)
{
    Node *node = findSection(page, splitId);
    if (node == nullptr || node->split == "none")
        return;
    ratio = qBound(0.1, ratio, 0.9);
    if (qFuzzyCompare(ratio, node->ratio))
        return;
    node->ratio = ratio;
    changed();
}

void ShowSurface::updateSection(int page, const QString &sectionId, const QVariantMap &props)
{
    Node *node = findSection(page, sectionId);
    if (node == nullptr || node->split != "none")
        return;

    QString oldType = node->props.value("type").toString();
    if (props.contains("type") && props.value("type").toString() != oldType)
    {
        // a new kind of section: its own settings, the items only between tables
        QString type = props.value("type").toString();
        QVariantMap fresh = sectionDefaults(type);
        fresh.insert("title", node->props.value("title"));
        if (node->props.contains("background"))
            fresh.insert("background", node->props.value("background"));
        // faders and dials are the same items: they go as they are
        const bool levels = (type == "faders" || type == "dials") && (oldType == "faders" || oldType == "dials");
        bool tables = (type == "buttons" || type == "faders" || type == "dials") &&
                      (oldType == "buttons" || oldType == "faders" || oldType == "dials");
        if (levels && node->props.contains("items"))
        {
            fresh.insert("items", node->props.value("items"));
        }
        else if (tables && node->props.contains("items"))
        {
            QVariantMap items;
            const QVariantMap old = node->props.value("items").toMap();
            for (auto it = old.cbegin(); it != old.cend(); ++it)
            {
                const QVariantMap item = convertItem(it.value().toMap(), type);
                if (item.isEmpty() == false)
                    items.insert(it.key(), item);
            }
            fresh.insert("items", items);
        }
        node->props = fresh;
    }

    for (auto it = props.constBegin(); it != props.constEnd(); ++it)
    {
        if (it.key() == "type" || it.key() == "items" || it.key() == "id" || it.key() == "split")
            continue;
        QVariant value = it.value();
        if (it.key() == "cols" || it.key() == "rows")
            value = qBound(1, value.toInt(), 32);
        // no background: the one of the theme (nothing saved)
        if (it.key() == "background" && value.toString().isEmpty())
        {
            node->props.remove("background");
            continue;
        }
        node->props.insert(it.key(), value);
    }
    changed();
}

QVariantMap ShowSurface::convertItem(const QVariantMap &item, const QString &type)
{
    QVariantMap converted = itemDefaults(type);
    converted.insert("color", item.value("color", converted.value("color")));

    // a fader or a dial: a button plays the scene it plays (its first one)
    if (type == "buttons")
    {
        for (const QVariant &v : faderTargets(item))
        {
            const QVariantMap t = v.toMap();
            const QString kind = t.value("target").toString();
            if ((kind == "scene" || kind == "sceneRun") && t.value("functionId", -1).toInt() >= 0)
            {
                converted.insert("functionId", t.value("functionId"));
                converted.insert("label", item.value("label"));
                return converted;
            }
        }
        return QVariantMap();
    }

    // a button: its scene on a fader; the next, previous and release of a
    // group the master of that group (its label named the action: not kept)
    const QString action = item.value("action").toString();
    QVariantMap target;
    if (action == "scene" && item.value("functionId", -1).toInt() >= 0)
    {
        target = QVariantMap({ { "target", "scene" }, { "functionId", item.value("functionId") } });
        converted.insert("label", item.value("label"));
    }
    else if ((action == "next" || action == "previous" || action == "release") && item.value("zoneId", -1).toInt() >= 0)
    {
        target = QVariantMap({ { "target", "group" }, { "zoneId", item.value("zoneId") } });
    }
    else
    {
        // nothing a fader does (the whole show, a palette, a mapping)
        return QVariantMap();
    }
    converted.insert("targets", QVariantList({ target }));
    converted.insert("target", target.value("target"));
    return converted;
}

QVariantMap ShowSurface::section(int page, const QString &sectionId) const
{
    Node *node = findSection(page, sectionId);
    return node != nullptr ? toMap(node) : QVariantMap();
}

void ShowSurface::setItem(int page, const QString &sectionId, int cell, const QVariantMap &props)
{
    Node *node = findSection(page, sectionId);
    if (node == nullptr || node->split != "none" || cell < 0)
        return;

    QVariantMap items = node->props.value("items").toMap();
    QString key = QString::number(cell);
    QVariantMap item = items.contains(key) ? items.value(key).toMap()
                                           : itemDefaults(node->props.value("type").toString());
    for (auto it = props.constBegin(); it != props.constEnd(); ++it)
        item.insert(it.key(), it.value());
    items.insert(key, item);
    node->props.insert("items", items);
    changed();
}

void ShowSurface::clearItem(int page, const QString &sectionId, int cell)
{
    Node *node = findSection(page, sectionId);
    if (node == nullptr)
        return;
    QVariantMap items = node->props.value("items").toMap();
    if (items.remove(QString::number(cell)) == 0)
        return;
    node->props.insert("items", items);
    changed();
}

void ShowSurface::moveItem(int page, const QString &sectionId, int from, int to)
{
    Node *node = findSection(page, sectionId);
    if (node == nullptr || from == to || from < 0 || to < 0)
        return;
    QVariantMap items = node->props.value("items").toMap();
    QVariant a = items.take(QString::number(from));
    QVariant b = items.take(QString::number(to));
    if (a.isValid())
        items.insert(QString::number(to), a);
    if (b.isValid())
        items.insert(QString::number(from), b);
    node->props.insert("items", items);
    changed();
}

QVariantMap ShowSurface::item(int page, const QString &sectionId, int cell) const
{
    Node *node = findSection(page, sectionId);
    if (node == nullptr)
        return QVariantMap();
    return node->props.value("items").toMap().value(QString::number(cell)).toMap();
}

int ShowSurface::setItemsColour(int page, const QString &sectionId, const QString &color)
{
    Node *node = findSection(page, sectionId);
    const QString type = node != nullptr ? node->props.value("type").toString() : QString();
    if ((type != "buttons" && type != "faders" && type != "dials") || QColor(color).isValid() == false)
        return 0;

    const QString name = QColor(color).name();
    QVariantMap items = node->props.value("items").toMap();
    int count = 0;
    for (auto it = items.begin(); it != items.end(); ++it)
    {
        QVariantMap item = it.value().toMap();
        if (item.value("color").toString().compare(name, Qt::CaseInsensitive) == 0)
            continue;
        item.insert("color", name);
        it.value() = item;
        count++;
    }
    if (count == 0)
        return 0;
    node->props.insert("items", items);
    changed();
    return count;
}

int ShowSurface::fillWithGroup(int page, const QString &sectionId, quint32 zoneId)
{
    Node *node = findSection(page, sectionId);
    Zone *zone = m_zones->zoneById(zoneId);
    if (node == nullptr || zone == nullptr || node->props.value("type").toString() != "buttons")
        return 0;

    const QList<quint32> functions = zone->functionIDs();
    int cols = node->props.value("cols").toInt();
    int rows = node->props.value("rows").toInt();
    // grow the table if the group does not fit
    while (cols * rows < functions.count() && rows < 32)
        rows++;
    node->props.insert("rows", rows);

    QVariantMap items;
    int cell = 0;
    for (quint32 fid : functions)
    {
        QVariantMap item = itemDefaults("buttons");
        item.insert("functionId", int(fid));
        item.insert("color", zone->color().name());
        items.insert(QString::number(cell++), item);
    }
    node->props.insert("items", items);
    if (node->props.value("title").toString().isEmpty())
        node->props.insert("title", zone->name());
    changed();
    return int(functions.count());
}

int ShowSurface::buildFromGroups()
{
    // the groups on top; masters, BPM and clock below
    int index = addPage(tr("Groups"), "groups");
    Page &page = m_pages[size_t(index)];
    QString groupsId = page.root->id;
    QString bottom = splitSection(index, groupsId, "v");
    setSplitRatio(index, page.root->id, 0.75);

    updateSection(index, bottom, QVariantMap{{"type", "faders"}, {"title", tr("Masters")}});
    setItem(index, bottom, 0, QVariantMap{{"target", "grandMaster"}, {"label", tr("Master")}, {"color", "#e7e9ef"}});
    for (int i = 0; i < m_zones->count() && i < 7; i++)
    {
        Zone *zone = m_zones->zone(i);
        setItem(index, bottom, i + 1, QVariantMap{{"target", "group"}, {"zoneId", int(zone->id())},
                                                   {"color", zone->color().name()}});
    }
    QString right = splitSection(index, bottom, "h");
    updateSection(index, right, QVariantMap{{"type", "bpm"}});
    QString clock = splitSection(index, right, "h");
    updateSection(index, clock, QVariantMap{{"type", "clock"}});
    return index;
}

void ShowSurface::clear()
{
    m_pages.clear();
    m_overrides.clear();
    m_levels.clear();
    // (the scenes still there at the speed they had)
    for (const quint32 fid : m_runSpeeds.keys())
        restoreRunSpeed(fid);
    clearSceneDimmers();
    // (no fader holds a scene of a group up any more)
    pruneHolds();
    m_lastId = 0;
    m_speedLevels.clear();
    m_speedOrder.clear();
    // (what moves goes at its own speed again: the FX of the programmer)
    applyMotion();
    emit cleared();
    emit pagesChanged();
}

/*********************************************************************
 * Playback
 *********************************************************************/

Zone *ShowSurface::zoneFor(quint32 functionId) const
{
    for (int i = 0; i < m_zones->count(); i++)
    {
        Zone *zone = m_zones->zone(i);
        if (zone->functionIDs().contains(functionId))
            return zone;
    }
    return nullptr;
}

/** The mapping target of a palette button: toggle or flash */
static QVariantMap paletteTarget(const QVariantMap &item)
{
    return QVariantMap({ { "target", "palette" }, { "paletteId", item.value("paletteId", -1) },
                         { "groupId", item.value("groupId", -1) },
                         { "mode", item.value("mode").toString() == "flash" ? "flash" : "toggle" } });
}

void ShowSurface::press(const QVariantMap &item)
{
    QString action = item.value("action").toString();

    // the actions of the whole show
    if (action == "blackout")
    {
        // through the core: everything that shows the blackout follows
        bool on = m_core != nullptr ? m_core->blackout() : m_doc->inputOutputMap()->blackout();
        bool next = item.value("mode").toString() == "flash" ? true : !on;
        if (m_core != nullptr)
            m_core->setBlackout(next);
        else
            m_doc->inputOutputMap()->setBlackout(next);
        slotChanged();
        return;
    }
    if (action == "stopAll" || action == "tap" || action == "beatgo")
    {
        if (m_core != nullptr)
        {
            if (action == "stopAll")
                m_core->stopAll();
            else if (action == "tap")
                m_core->tapTempo();
            else
                m_core->drive()->beatGo();
        }
        slotChanged();
        return;
    }
    // a palette on its fixtures or a group, as a live edit (toggle or flash)
    if (action == "palette")
    {
        if (m_core != nullptr)
            m_core->mapping()->pressTarget(paletteTarget(item), true);
        slotChanged();
        return;
    }
    // anything that can be mapped (Alt+click)
    if (action == "map")
    {
        if (m_core != nullptr)
            m_core->mapping()->pressTarget(item.value("map").toMap(), true);
        slotChanged();
        return;
    }
    Zone *zone = m_zones->zoneById(item.value("zoneId").toUInt());
    bool allZones = item.value("zoneId").toInt() < 0;

    if (action == "scene")
    {
        quint32 fid = item.value("functionId").toUInt();
        Function *f = m_doc->function(fid);
        if (f == nullptr)
            return;
        bool flash = item.value("mode").toString() == "flash";

        // a scene of a group plays in it: exclusivity, release and priority apply
        if (Zone *owner = zoneFor(fid))
        {
            if (flash)
                owner->start(fid);
            else
                owner->trigger(fid);
        }
        else if (flash)
        {
            f->start(m_doc->masterTimer(), FunctionParent::master());
        }
        else
        {
            // (started or stopped just before, it changes at the next tick:
            // the press goes by what was asked). In the mode of the buttons
            // (GO or Stop): start, stop, or a beat of its rhythm. One a fader
            // holds up: only the fader stops it
            const bool active = f->stopped() == false;
            switch (SceneTaps::press(f, active))
            {
                case SceneTaps::StartIt:
                    f->start(m_doc->masterTimer(), FunctionParent::master());
                    break;
                case SceneTaps::StopIt:
                    if (holds(fid) == false)
                        f->stop(FunctionParent::master());
                    break;
                case SceneTaps::ToggleIt:
                    if (active && holds(fid) == false)
                        f->stop(FunctionParent::master());
                    else if (active == false)
                        f->start(m_doc->masterTimer(), FunctionParent::master());
                    break;
                case SceneTaps::Keep:
                    break;
            }
        }
    }
    else if (action == "next")
    {
        if (allZones)
            m_zones->nextAll();
        else if (zone != nullptr)
            zone->next();
    }
    else if (action == "previous")
    {
        if (allZones)
            m_zones->previousAll();
        else if (zone != nullptr)
            zone->previous();
    }
    else if (action == "release")
    {
        if (allZones)
            m_zones->stopAll();
        else if (zone != nullptr)
            zone->stopAll();
    }
    slotChanged();
}

void ShowSurface::release(const QVariantMap &item)
{
    QString action = item.value("action").toString();
    if (action == "blackout" && item.value("mode").toString() == "flash")
    {
        if (m_core != nullptr)
            m_core->setBlackout(false);
        else
            m_doc->inputOutputMap()->setBlackout(false);
        slotChanged();
        return;
    }
    if (action == "map")
    {
        if (m_core != nullptr)
            m_core->mapping()->pressTarget(item.value("map").toMap(), false);
        slotChanged();
        return;
    }
    if (action == "palette")
    {
        if (m_core != nullptr)
            m_core->mapping()->pressTarget(paletteTarget(item), false);
        slotChanged();
        return;
    }
    if (action != "scene" || item.value("mode").toString() != "flash")
        return;

    quint32 fid = item.value("functionId").toUInt();
    if (Zone *owner = zoneFor(fid))
    {
        owner->stop(fid);
    }
    else if (Function *f = m_doc->function(fid))
    {
        // (a release right after the press, before the next tick: the scene
        // is started but doesn't run yet). One a fader holds up stays
        if ((f->isRunning() || f->stopped() == false) && holds(fid) == false)
            f->stop(FunctionParent::master());
    }
    slotChanged();
}

bool ShowSurface::isActive(const QVariantMap &item) const
{
    QString action = item.value("action").toString();
    if (action == "blackout")
        return m_doc->inputOutputMap()->blackout();
    if (action == "map")
    {
        if (m_core == nullptr)
            return false;
        // a playback that fades, waits or is paused is lit too (a cue
        // button only when its cue plays: 2 is the next cue there)
        const QVariantMap target = item.value("map").toMap();
        const QString kind = target.value("target").toString();
        const int state = m_core->mapping()->targetState(target);
        return kind.startsWith("playback") && kind != "playbackGoTo" ? state >= 1 : state == 1;
    }
    if (action == "palette")
        return m_core != nullptr && m_core->mapping()->targetState(paletteTarget(item)) == 1;
    if (action != "scene")
        return false;

    quint32 fid = item.value("functionId").toUInt();
    if (Zone *owner = zoneFor(fid))
        return owner->isActive(fid);
    Function *f = m_doc->function(fid);
    return f != nullptr && f->isRunning();
}

QString ShowSurface::mapLabel(const QVariantMap &target) const
{
    if (m_core == nullptr)
        return QString();
    const QString kind = target.value("target").toString();
    const int fid = target.value("functionId", -1).toInt();
    Function *f = kind.startsWith("playback") && fid >= 0 ? m_doc->function(quint32(fid)) : nullptr;
    if (f != nullptr)
    {
        // the actions of a cuelist (SHOW buttons): short, the function named
        if (kind == "playbackGo")
            return tr("GO · %1").arg(f->name());
        if (kind == "playbackBack")
            return tr("Back · %1").arg(f->name());
        if (kind == "playbackPauseBack")
            return tr("Pause / Back · %1").arg(f->name());
        if (kind == "playbackRelease")
            return tr("Release · %1").arg(f->name());
        if (kind == "playbackFlash")
            return tr("Flash · %1").arg(f->name());
        if (kind == "playbackGoTo")
        {
            const Cuelist *cuelist = qobject_cast<const Cuelist *>(f);
            // (the cue it is bound to, wherever it went)
            const int index = m_core->mapping()->cueIndexOf(target);
            const QString number = cuelist != nullptr && index >= 0 && index < cuelist->cueCount()
                                 ? Cuelist::numberToString(cuelist->cueAt(index).number) : QStringLiteral("?");
            return tr("%1 · cue %2").arg(f->name(), number);
        }
    }
    // a cuelist button (no slot) whose cuelist isn't chosen yet, or was deleted
    static const QSet<QString> functionKinds = { "playbackGo", "playbackBack", "playbackPauseBack",
                                                 "playbackRelease", "playbackFlash", "playbackGoTo" };
    if (f == nullptr && functionKinds.contains(kind) && (fid >= 0 || target.value("slot", 0).toInt() < 0))
        return tr("Choose a cuelist");
    return m_core->mapping()->describeTarget(target);
}

QVariantList ShowSurface::faderTargets(const QVariantMap &item)
{
    QVariantList targets = item.value("targets").toList();
    if (targets.isEmpty())
    {
        // an item of before: one target, with its keys in the item
        QVariantMap single;
        single.insert("target", item.value("target", "grandMaster"));
        for (const char *key : { "zoneId", "functionId", "mixerId", "groupId" })
            if (item.contains(key))
                single.insert(key, item.value(key));
        targets.append(single);
    }
    return targets;
}

bool ShowSurface::isSpeed(const QVariantMap &item)
{
    const QVariantList targets = faderTargets(item);
    for (const QVariant &v : targets)
    {
        const QString kind = v.toMap().value("target").toString();
        if (kind != "moveSpeed" && kind != "sceneSpeed" && kind != "sceneRun")
            return false;
    }
    return targets.isEmpty() == false;
}

qreal ShowSurface::resetLevel(const QVariantMap &item) const
{
    const QVariantList targets = faderTargets(item);
    if (targets.count() == 1 && targets.first().toMap().value("target").toString() == "bpm")
        return positionOf(item, (90 - 20) / 280.0);
    if (isSpeed(item) == false)
        return 0.0;

    qreal position = positionOf(item, 0.5);
    // x1 plays a scene that is stopped: not at the end that stops it (x1
    // below the range: as near to it as the travel goes)
    if (startedScenes(targets).isEmpty() == false)
    {
        qreal min, max;
        bool invert;
        ControlMapper::rangeOf(item, min, max, invert);
        const qreal nudge = 1e-6;
        position = invert ? qMin(position, 1.0 - nudge) : qMax(position, nudge);
    }
    return position;
}

bool ShowSurface::bottomStops(const QVariantMap &target)
{
    const QString kind = target.value("target").toString();
    return kind == "scene" || kind == "sceneRun" ||
           (kind == "map" && target.value("map").toMap().value("target").toString() == "playbackFader");
}

QList<quint32> ShowSurface::startedScenes(const QVariantList &targets)
{
    QList<quint32> scenes;
    for (const QVariant &v : targets)
    {
        const QVariantMap t = v.toMap();
        const QString kind = t.value("target").toString();
        if ((kind == "scene" || kind == "sceneRun") && t.value("functionId", -1).toInt() >= 0)
            scenes.append(t.value("functionId").toUInt());
    }
    return scenes;
}

void ShowSurface::setLevel(const QVariantMap &item, qreal level)
{
    level = qIsFinite(level) ? qBound(0.0, level, 1.0) : 0.0;
    qreal min, max;
    bool invert;
    ControlMapper::rangeOf(item, min, max, invert);
    const qreal value = min + (max - min) * (invert ? 1.0 - level : level);
    // a scene stops at the end of the travel where the value is lowest (the
    // bottom; the top when inverted), whatever the minimum: there its fader
    // shows it stopped (level())
    const bool atStop = invert ? level >= 1.0 : level <= 0.0;
    for (const QVariant &v : faderTargets(item))
    {
        const QVariantMap target = v.toMap();
        setTargetLevel(target, atStop && bottomStops(target) ? 0.0 : value);
    }
    slotChanged();
}

qreal ShowSurface::level(const QVariantMap &item) const
{
    QVariantList targets = faderTargets(item);
    return targets.isEmpty() ? 0.0 : positionOf(item, targetLevel(targets.first().toMap()));
}

qreal ShowSurface::value(const QVariantMap &item) const
{
    QVariantList targets = faderTargets(item);
    return targets.isEmpty() ? 0.0 : targetLevel(targets.first().toMap());
}

qreal ShowSurface::positionOf(const QVariantMap &item, qreal value) const
{
    qreal min, max;
    bool invert;
    ControlMapper::rangeOf(item, min, max, invert);
    // (no range: at the top from its value up)
    qreal position = max > min ? (value - min) / (max - min) : (value >= max ? 1.0 : 0.0);
    position = qIsFinite(position) ? qBound(0.0, position, 1.0) : 0.0;
    return invert ? 1.0 - position : position;
}

void ShowSurface::stepLevel(const QVariantMap &item, int notches)
{
    // in steps of the value (as the encoders of the mappings), so a narrow
    // range doesn't stall on the steps of a channel
    qreal min, max;
    bool invert;
    ControlMapper::rangeOf(item, min, max, invert);
    bool ok = false;
    const int stepPercent = item.value("step").toInt(&ok);
    const qreal step = (ok ? qBound(1, stepPercent, 100) : 2) / 100.0;
    // (up the travel is down the value on an inverted fader)
    const qreal delta = notches * step * (invert ? -1.0 : 1.0);
    qreal next = qBound(min, value(item) + delta, max);
    // (steps of 0.1 don't add up to the ends exactly)
    if (qAbs(next - min) < 1e-6)
        next = min;
    else if (qAbs(next - max) < 1e-6)
        next = max;
    qreal position = positionOf(item, next);
    // turned up from a stopped scene (or one below the range): it starts at
    // the minimum, it doesn't stay at the end that stops it
    const qreal nudge = 1e-6;
    if (delta > 0.0 && (invert ? position >= 1.0 : position <= 0.0))
        position = invert ? 1.0 - nudge : nudge;
    // turned down to the minimum: the end that stops it (also when From is
    // To, where the minimum has no place of its own on the travel)
    else if (delta < 0.0 && next <= min)
        position = invert ? 1.0 : 0.0;
    setLevel(item, position);
}

QVariantList ShowSurface::holdLevels(const QVariantMap &item) const
{
    QVariantList levels;
    QVariantList loose;
    for (const QVariant &v : faderTargets(item))
    {
        const QVariantMap target = v.toMap();
        const qreal level = targetLevel(target);
        levels.append(level);
        // the scenes it plays that no fader holds (started by a button): the
        // Flash gives them back as they were, not held
        if (startedScenes(QVariantList({ target })).isEmpty() == false && level > 0.0 &&
            holds(target.value("functionId").toUInt()) == false)
            loose.append(target.value("functionId"));
    }
    // (after the levels, which are as many as the targets)
    if (loose.isEmpty() == false)
        levels.append(QVariantMap({ { "loose", loose } }));
    return levels;
}

void ShowSurface::restoreLevels(const QVariantMap &item, const QVariantList &levels)
{
    const QVariantList targets = faderTargets(item);
    for (int i = 0; i < targets.count() && i < levels.count(); i++)
    {
        bool ok = false;
        const qreal level = levels.at(i).toDouble(&ok);
        if (ok && qIsFinite(level))
            setTargetLevel(targets.at(i).toMap(), qBound(0.0, level, 1.0));
    }
    if (levels.count() > targets.count())
    {
        for (const QVariant &id : levels.last().toMap().value("loose").toList())
        {
            const quint32 fid = id.toUInt();
            if (Zone *owner = zoneFor(fid))
            {
                owner->dropHold(fid);
            }
            else
            {
                // (at the level and speed it had: nothing to give back)
                m_levels.remove(fid);
                m_runSpeeds.remove(fid);
            }
        }
    }
    slotChanged();
}

void ShowSurface::setSceneDimmer(quint32 functionId, qreal dimmer)
{
    dimmer = qIsFinite(dimmer) ? qBound(0.0, dimmer, 1.0) : 1.0;
    if (qFuzzyCompare(sceneDimmer(functionId), dimmer))
        return;
    Function *f = m_doc->function(functionId);
    if (f == nullptr)
        return;

    if (dimmer >= 1.0)
        m_sceneDimmers.remove(functionId);
    else
        m_sceneDimmers.insert(functionId, dimmer);
    applySceneDimmer(f, dimmer);
    // (a super scene gets it on its tracks added later when it runs again)
    connect(f, SIGNAL(running(quint32)), this, SLOT(slotFunctionRunning(quint32)), Qt::UniqueConnection);
    m_doc->setModified();
    slotChanged();
}

qreal ShowSurface::sceneDimmer(quint32 functionId) const
{
    return m_sceneDimmers.value(functionId, 1.0);
}

void ShowSurface::clearSceneDimmers()
{
    for (auto it = m_sceneDimmers.cbegin(); it != m_sceneDimmers.cend(); ++it)
        applySceneDimmer(m_doc->function(it.key()), 1.0);
    m_sceneDimmers.clear();
}

void ShowSurface::applySceneDimmer(Function *f, qreal dimmer)
{
    if (f == nullptr)
        return;
    // a super scene: the level of each of its tracks, and its static values
    if (Show *show = qobject_cast<Show *>(f))
    {
        for (int i = 0; i < show->attributes().count(); i++)
            show->adjustAttribute(dimmer, i);
        if (Function *values = m_doc->function(show->staticScene()))
            values->adjustAttribute(dimmer, Function::Intensity);
        return;
    }
    f->adjustAttribute(dimmer, Function::Intensity);
}

void ShowSurface::setTargetLevel(const QVariantMap &t, qreal level)
{
    QString target = t.value("target").toString();

    if (target == "map")
    {
        if (m_core != nullptr)
            m_core->mapping()->setTargetLevel(t.value("map").toMap(), level);
        return;
    }
    if (target == "grandMaster")
    {
        m_doc->inputOutputMap()->setGrandMasterValue(uchar(qRound(level * 255)));
    }
    else if (target == "bpm")
    {
        m_doc->inputOutputMap()->setBpmNumber(qRound(20 + level * 280));
    }
    else if (target == "group")
    {
        if (Zone *zone = m_zones->zoneById(t.value("zoneId").toUInt()))
            zone->setMaster(level);
    }
    else if (target == "sceneDimmer")
    {
        // whoever plays the scene (it doesn't start or stop it)
        setSceneDimmer(t.value("functionId").toUInt(), level);
    }
    else if (target == "sceneSpeed")
    {
        // x0.1 .. x10, x1 in the middle
        quint32 fid = t.value("functionId").toUInt();
        const qreal speed = std::pow(10.0, level * 2.0 - 1.0);
        if (Zone *owner = zoneFor(fid))
            owner->setOption(fid, "speed", speed);
        // a scene in no group: its live speed (not saved with the show)
        else if (Function *f = m_doc->function(fid))
            f->setSpeedFactor(speed);
    }
    else if (target == "sceneRun")
    {
        // a scene and its speed: zero stops it; up from there it plays (in
        // its group, if it has one, as a scene fader starts it) at x0.1 ..
        // x10, x1 in the middle
        quint32 fid = t.value("functionId").toUInt();
        Function *f = m_doc->function(fid);
        if (f == nullptr)
            return;
        Zone *owner = zoneFor(fid);
        if (level <= 0.0)
        {
            // (its group forgets the speed of the run; a run of its own, the
            // fader started before the scene went into a group, stops too)
            if (owner != nullptr)
                owner->letGo(fid);
            if ((owner == nullptr || m_runSpeeds.contains(fid)) && (f->isRunning() || f->stopped() == false))
                f->stop(FunctionParent::master());
            restoreRunSpeed(fid);
            return;
        }
        // the speed of this run only (its Live speed and the show stay as
        // they are), given first, so it plays at it from its first frame.
        // The fader holds its scene up in its group, as a scene fader
        const qreal speed = std::pow(10.0, level * 2.0 - 1.0);
        if (owner != nullptr)
        {
            owner->hold(fid, speed);
        }
        else
        {
            // the speed it had goes back when it stops (slotFunctionStopped)
            connect(f, SIGNAL(stopped(quint32)), this, SLOT(slotFunctionStopped(quint32)), Qt::UniqueConnection);
            if (m_runSpeeds.contains(fid) == false)
                m_runSpeeds.insert(fid, RunSpeed{ f->speedFactor(), speed });
            m_runSpeeds[fid].given = speed;
            f->setSpeedFactor(speed);
            // (a scene just stopped still runs until the next tick: it starts again)
            if (f->stopped())
                f->start(m_doc->masterTimer(), FunctionParent::master());
        }
    }
    else if (target == "mixerDimmer")
    {
        if (m_mixer != nullptr)
            m_mixer->set(t.value("mixerId", -1).toInt(), "dimmer", level);
    }
    else if (target == "moveSpeed")
    {
        int groupId = t.value("groupId", -1).toInt();
        m_speedLevels.insert(groupId, level);
        // (the last one moved goes last when they are given again)
        m_speedOrder.removeAll(groupId);
        m_speedOrder.append(groupId);
        applyMotion();
    }
    else if (target == "scene")
    {
        // a scene fader: up from zero starts the scene (in its group, if it
        // has one), down to zero stops it, in between it sets its intensity
        quint32 fid = t.value("functionId").toUInt();
        Function *f = m_doc->function(fid);
        if (f == nullptr)
            return;

        // in its group the group plays it: the fader on top of its master
        // and the dimmer of the scene (the group has its one intensity). The
        // fader holds it up: it adds its scene (the others of the group, and
        // those of other faders, go on: two faders up of one exclusive group
        // would replace each other at every move, and start over), and only
        // the fader at the bottom stops it (Zone::hold)
        if (Zone *owner = zoneFor(fid))
        {
            if (level <= 0.0)
            {
                owner->letGo(fid);
                // (a run of its own, the fader started before the scene went
                // into a group, stops too)
                if (m_levels.remove(fid) > 0)
                {
                    m_overrides.remove(fid);
                    if (f->isRunning() || f->stopped() == false)
                        f->stop(FunctionParent::master());
                }
                return;
            }
            owner->hold(fid);
            owner->setSceneLevel(fid, level);
            return;
        }

        // on its own: the level lasts while it plays, and is given again
        // when it runs (a function drops its overrides when it stops,
        // whoever stops it)
        connect(f, SIGNAL(running(quint32)), this, SLOT(slotFunctionRunning(quint32)), Qt::UniqueConnection);
        connect(f, SIGNAL(stopped(quint32)), this, SLOT(slotFunctionStopped(quint32)), Qt::UniqueConnection);
        if (level <= 0.0)
        {
            m_levels.remove(fid);
            m_overrides.remove(fid);
            if (f->isRunning() || f->stopped() == false)
                f->stop(FunctionParent::master());
            return;
        }
        // the level first, so its first frame isn't at full; (a scene just
        // stopped still runs until the next tick: it starts again)
        m_levels.insert(fid, level);
        applySceneLevel(f);
        if (f->stopped())
            f->start(m_doc->masterTimer(), FunctionParent::master());
    }
}

void ShowSurface::applySceneLevel(Function *f)
{
    const quint32 fid = f->id();
    m_overrides.insert(fid, Zone::applyLevel(m_doc, f, m_levels.value(fid, 1.0), m_overrides.value(fid)));
}

void ShowSurface::slotFunctionRunning(quint32 fid)
{
    Function *f = m_doc->function(fid);
    if (f == nullptr)
        return;
    // its dimmer again (the tracks a super scene got since)
    if (m_sceneDimmers.contains(fid))
        applySceneDimmer(f, m_sceneDimmers.value(fid));
    // a scene of no group the fader started (again): its level again
    if (m_levels.contains(fid) == false || zoneFor(fid) != nullptr)
        return;
    applySceneLevel(f);
}

void ShowSurface::slotFunctionStopped(quint32 fid)
{
    // the signal of the run before, after it was started again: it keeps the
    // level of the fader, running gives it again (the overrides of the run
    // before are gone: they are asked again)
    Function *f = m_doc->function(fid);
    if (f != nullptr && f->stopped() == false)
        return;

    // stopped, whoever stopped it: the next run, whoever starts it, is at full
    // (and at the speed it had before a "sceneRun" fader played it)
    m_levels.remove(fid);
    m_overrides.remove(fid);
    restoreRunSpeed(fid);
    slotChanged();
}

void ShowSurface::restoreRunSpeed(quint32 fid)
{
    if (m_runSpeeds.contains(fid) == false)
        return;
    const RunSpeed run = m_runSpeeds.take(fid);
    // (another control gave it a speed meanwhile: that one stays)
    Function *f = m_doc->function(fid);
    if (f != nullptr && qFuzzyCompare(1.0 + f->speedFactor(), 1.0 + run.given))
        f->setSpeedFactor(run.before);
}

void ShowSurface::slotFunctionGone(quint32 fid)
{
    // deleted, or another function with its ID (an effect made an FX)
    m_levels.remove(fid);
    m_overrides.remove(fid);
    m_runSpeeds.remove(fid);
}

void ShowSurface::slotFunctionRemoved(quint32 fid)
{
    m_sceneDimmers.remove(fid);
    // a reference to it: it drives nothing now, not a function made later
    // with its ID (a playback target lets its slot go too, like a mapping)
    if (forgetFunctions([fid](const QVariant &id) { return id.toInt() >= 0 && id.toUInt() == fid; }))
        changed();
}

void ShowSurface::slotFunctionAdded(quint32 fid)
{
    // (a copy of an effect copies the base of its intensity: not the dimmer)
    Function *f = m_doc->function(fid);
    if (f != nullptr && m_sceneDimmers.contains(fid) == false &&
        f->attributes().isEmpty() == false && f->attributes().first().m_value != 1.0)
        applySceneDimmer(f, 1.0);
}

void ShowSurface::slotFunctionChanged(quint32 fid)
{
    // another function in its place (an effect made an FX), or a super scene
    // with new tracks or static values: the same dimmer, before they play
    if (m_sceneDimmers.contains(fid))
        applySceneDimmer(m_doc->function(fid), m_sceneDimmers.value(fid));
}

bool ShowSurface::forgetFunctions(const std::function<bool(const QVariant &)> &gone)
{
    auto forget = [&gone](QVariantMap &map) {
        if (map.contains("functionId") == false || gone(map.value("functionId")) == false)
            return false;
        map.insert("functionId", -1);
        if (map.contains("slot"))
            map.insert("slot", -1);
        return true;
    };

    bool any = false;
    std::function<void(Node *)> walk = [&](Node *node) {
        if (node == nullptr)
            return;
        if (node->split != "none")
        {
            walk(node->a.get());
            walk(node->b.get());
            return;
        }

        QVariantMap items = node->props.value("items").toMap();
        bool itemsChanged = false;
        for (auto it = items.begin(); it != items.end(); ++it)
        {
            QVariantMap item = it.value().toMap();
            bool changed = forget(item);
            if (item.contains("map"))
            {
                QVariantMap map = item.value("map").toMap();
                if (forget(map))
                {
                    item.insert("map", map);
                    changed = true;
                }
            }
            if (item.contains("targets"))
            {
                QVariantList targets = item.value("targets").toList();
                bool targetsChanged = false;
                for (QVariant &t : targets)
                {
                    QVariantMap target = t.toMap();
                    bool targetChanged = forget(target);
                    if (target.contains("map"))
                    {
                        QVariantMap map = target.value("map").toMap();
                        if (forget(map))
                        {
                            target.insert("map", map);
                            targetChanged = true;
                        }
                    }
                    if (targetChanged)
                    {
                        t = target;
                        targetsChanged = true;
                    }
                }
                if (targetsChanged)
                {
                    item.insert("targets", targets);
                    changed = true;
                }
            }
            if (changed)
            {
                it.value() = item;
                itemsChanged = true;
            }
        }
        if (itemsChanged)
        {
            node->props.insert("items", items);
            any = true;
        }
    };
    for (Page &page : m_pages)
        walk(page.root.get());
    return any;
}

void ShowSurface::applyMotion()
{
    QList<Function *> functions = m_doc->functions();
    // (the FX of the programmer is not in the show)
    if (m_core != nullptr && m_core->programmer() != nullptr && m_core->programmer()->fxScene() != nullptr)
        functions.append(m_core->programmer()->fxScene());

    // x0.1 at the bottom, x1 in the middle, x10 at the top
    QHash<Function *, qreal> speeds;
    for (int groupId : std::as_const(m_speedOrder))
    {
        const QSet<quint32> fixtures = motionFixtures(groupId);
        const qreal speed = std::pow(10.0, m_speedLevels.value(groupId, 0.5) * 2.0 - 1.0);
        for (Function *f : std::as_const(functions))
        {
            if (movesHeads(f, fixtures))
                speeds.insert(f, speed);
        }
    }
    for (Function *f : std::as_const(functions))
        f->setMotionSpeed(speeds.value(f, 1.0));
}

QSet<quint32> ShowSurface::motionFixtures(int groupId) const
{
    QSet<quint32> fixtures;
    if (groupId >= 0)
    {
        if (FixtureGroup *group = m_doc->fixtureGroup(quint32(groupId)))
            for (quint32 id : group->fixtureList())
                fixtures.insert(id);
        return fixtures;
    }
    for (Fixture *fxi : m_doc->fixtures())
    {
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch != nullptr && (ch->group() == QLCChannel::Pan || ch->group() == QLCChannel::Tilt))
            {
                fixtures.insert(fxi->id());
                break;
            }
        }
    }
    return fixtures;
}

bool ShowSurface::movesHeads(Function *f, const QSet<quint32> &fixtures) const
{
    if (f == nullptr || fixtures.isEmpty())
        return false;

    auto moving = [this](quint32 fixtureId, quint32 channel) {
        Fixture *fxi = m_doc->fixture(fixtureId);
        const QLCChannel *ch = fxi != nullptr ? fxi->channel(channel) : nullptr;
        return ch != nullptr && (ch->group() == QLCChannel::Pan || ch->group() == QLCChannel::Tilt);
    };
    auto layersMove = [&fixtures](const QList<FXScene::Layer> &layers) {
        for (const FXScene::Layer &layer : layers)
        {
            if (FXScene::isMotion(layer.param) == false)
                continue;
            for (quint32 id : layer.fixtures)
                if (fixtures.contains(id))
                    return true;
        }
        return false;
    };
    auto valuesMove = [&](const QList<SceneValue> &values) {
        for (const SceneValue &v : values)
            if (fixtures.contains(v.fxi) && moving(v.fxi, v.channel))
                return true;
        return false;
    };

    switch (f->type())
    {
    case Function::FXSceneType:
        return layersMove(static_cast<FXScene *>(f)->layers());
    case Function::EFXType:
        for (EFXFixture *ef : static_cast<EFX *>(f)->fixtures())
            if (fixtures.contains(ef->head().fxi))
                return true;
        return false;
    case Function::ChaserType:
    case Function::SequenceType:
        // steps with positions: their own values, or the scenes they play
        for (const ChaserStep &step : static_cast<Chaser *>(f)->steps())
        {
            if (valuesMove(step.values))
                return true;
            if (Scene *scene = qobject_cast<Scene *>(m_doc->function(step.fid)))
                if (valuesMove(scene->values()))
                    return true;
        }
        return false;
    case Function::CuelistType:
        // its FX only (the times of its cues stay)
        for (const Cuelist::Cue &cue : static_cast<Cuelist *>(f)->cues())
            if (layersMove(cue.fx))
                return true;
        return false;
    default:
        return false;
    }
}

void ShowSurface::slotMotionChanged()
{
    if (m_speedOrder.isEmpty() == false)
        applyMotion();
}

void ShowSurface::slotFixturesChanged()
{
    // the fixtures of a group, or the moving heads, may be others now
    slotMotionChanged();
}

qreal ShowSurface::targetLevel(const QVariantMap &t) const
{
    QString target = t.value("target").toString();
    if (target == "map")
        return m_core != nullptr ? m_core->mapping()->targetLevel(t.value("map").toMap()) : 0.0;
    if (target == "grandMaster")
        return m_doc->inputOutputMap()->grandMasterValue() / 255.0;
    if (target == "bpm")
        return qBound(0.0, (m_doc->inputOutputMap()->bpmNumber() - 20) / 280.0, 1.0);
    if (target == "group")
    {
        Zone *zone = m_zones->zoneById(t.value("zoneId").toUInt());
        return zone != nullptr ? zone->master() : 0.0;
    }
    if (target == "sceneDimmer")
        return sceneDimmer(t.value("functionId").toUInt());
    if (target == "sceneSpeed")
    {
        quint32 fid = t.value("functionId").toUInt();
        qreal speed = 1.0;
        // (in its group: the speed it plays at, that of a "sceneRun" fader)
        if (Zone *owner = zoneFor(fid))
            speed = owner->speed(fid);
        else if (Function *f = m_doc->function(fid))
            speed = f->speedFactor();
        return qBound(0.0, (std::log10(speed) + 1.0) / 2.0, 1.0);
    }
    if (target == "sceneRun")
    {
        // stopped (or stopping): zero; playing: the speed it plays at (in
        // its group, that of its run there), above zero also at x0.1, so it
        // isn't shown stopped
        quint32 fid = t.value("functionId").toUInt();
        Function *f = m_doc->function(fid);
        if (f == nullptr)
            return 0.0;
        Zone *owner = zoneFor(fid);
        const bool inGroup = owner != nullptr && owner->isActive(fid);
        if (inGroup == false && f->stopped())
            return 0.0;
        const qreal speed = inGroup ? owner->speed(fid) : f->speedFactor();
        return qBound(1e-6, (std::log10(speed) + 1.0) / 2.0, 1.0);
    }
    if (target == "mixerDimmer")
    {
        if (m_mixer == nullptr)
            return 1.0;
        for (const QVariant &v : m_mixer->mixers())
            if (v.toMap().value("id").toInt() == t.value("mixerId", -1).toInt())
                return v.toMap().value("dimmer", 1.0).toDouble();
        return 1.0;
    }
    if (target == "moveSpeed")
        return m_speedLevels.value(t.value("groupId", -1).toInt(), 0.5);
    if (target == "scene")
    {
        quint32 fid = t.value("functionId").toUInt();
        Function *f = m_doc->function(fid);
        if (f == nullptr)
            return 0.0;
        // played by its group: the level the fader gave it there
        Zone *owner = zoneFor(fid);
        if (owner != nullptr && owner->isActive(fid))
            return owner->sceneLevel(fid);
        // (stopped, or stopping: down)
        if (f->stopped())
            return 0.0;
        // played elsewhere (a playback), or on its own
        return owner != nullptr ? 1.0 : m_levels.value(fid, 1.0);
    }
    return 0.0;
}


int ShowSurface::revision() const
{
    return m_revision;
}

QStringList ShowSurface::images() const
{
    QStringList images;
    std::function<void(const Node *)> walk = [&](const Node *node) {
        if (node == nullptr)
            return;
        if (node->split != "none")
        {
            walk(node->a.get());
            walk(node->b.get());
            return;
        }
        const QString image = node->props.value("image").toString();
        if (image.isEmpty() == false && images.contains(image) == false)
            images.append(image);
        const QVariantMap items = node->props.value("items").toMap();
        for (const QVariant &item : items)
        {
            const QString itemImage = item.toMap().value("image").toString();
            if (itemImage.isEmpty() == false && images.contains(itemImage) == false)
                images.append(itemImage);
        }
    };
    for (const Page &page : m_pages)
        walk(page.root.get());
    return images;
}

int ShowSurface::replaceImage(const QString &from, const QString &to)
{
    int count = 0;
    std::function<void(Node *)> walk = [&](Node *node) {
        if (node == nullptr)
            return;
        if (node->split != "none")
        {
            walk(node->a.get());
            walk(node->b.get());
            return;
        }
        if (node->props.value("image").toString() == from)
        {
            node->props.insert("image", to);
            count++;
        }
        QVariantMap items = node->props.value("items").toMap();
        bool itemsChanged = false;
        for (auto it = items.begin(); it != items.end(); ++it)
        {
            QVariantMap item = it.value().toMap();
            if (item.value("image").toString() != from)
                continue;
            item.insert("image", to);
            it.value() = item;
            itemsChanged = true;
            count++;
        }
        if (itemsChanged)
            node->props.insert("items", items);
    };
    for (Page &page : m_pages)
        walk(page.root.get());

    if (count > 0)
        changed();
    return count;
}

void ShowSurface::slotChanged()
{
    m_revision++;
    emit revisionChanged();
}

void ShowSurface::refresh()
{
    slotChanged();
}

void ShowSurface::slotZonesChanged()
{
    for (int i = 0; i < m_zones->count(); i++)
    {
        Zone *zone = m_zones->zone(i);
        // UniqueConnection: zones already connected stay connected once
        connect(zone, SIGNAL(activeChanged()), this, SLOT(slotChanged()), Qt::UniqueConnection);
        connect(zone, SIGNAL(masterChanged()), this, SLOT(slotChanged()), Qt::UniqueConnection);
        // (the speed of a scene changed elsewhere: the faders on it follow)
        connect(zone, SIGNAL(functionsChanged()), this, SLOT(slotChanged()), Qt::UniqueConnection);
    }
    slotChanged();
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

static QVariant fromXmlValue(const QString &key, const QString &text)
{
    // the target of a control brought with Alt+click: a JSON object
    if (key == "map")
        return QJsonDocument::fromJson(text.toUtf8()).object().toVariantMap();
    // texts stay texts even when they look like numbers
    if (key == "label" || key == "title" || key == "text" || key == "key" || key == "color" || key == "icon" || key == "image" ||
        key == "background")
        return text;
    bool ok = false;
    int number = text.toInt(&ok);
    if (ok)
        return number;
    double real = text.toDouble(&ok);
    if (ok)
        return real;
    if (text == "true" || text == "false")
        return text == "true";
    return text;
}

static QString toXmlValue(const QVariant &value)
{
    if (value.typeId() == QMetaType::QVariantMap)
        return QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(value.toMap())).toJson(QJsonDocument::Compact));
    if (value.typeId() == QMetaType::Bool)
        return value.toBool() ? "true" : "false";
    return value.toString();
}

void ShowSurface::saveNode(QXmlStreamWriter *doc, const Node *node) const
{
    if (node == nullptr)
        return;

    if (node->split != "none")
    {
        doc->writeStartElement(KXMLDTCSurfaceSplit);
        doc->writeAttribute("ID", node->id);
        doc->writeAttribute("Orientation", node->split);
        doc->writeAttribute("Ratio", QString::number(node->ratio));
        saveNode(doc, node->a.get());
        saveNode(doc, node->b.get());
        doc->writeEndElement();
        return;
    }

    doc->writeStartElement(KXMLDTCSurfaceSection);
    doc->writeAttribute("ID", node->id);
    for (auto it = node->props.constBegin(); it != node->props.constEnd(); ++it)
        if (it.key() != "items")
            doc->writeAttribute(it.key(), toXmlValue(it.value()));

    const QVariantMap items = node->props.value("items").toMap();
    for (auto it = items.cbegin(); it != items.cend(); ++it)
    {
        doc->writeStartElement(KXMLDTCSurfaceItem);
        doc->writeAttribute("Cell", it.key());
        const QVariantMap item = it.value().toMap();
        for (auto p = item.cbegin(); p != item.cend(); ++p)
            if (p.key() != "targets")
                doc->writeAttribute(p.key(), toXmlValue(p.value()));
        // the targets of a fader, one element each
        for (const QVariant &target : item.value("targets").toList())
        {
            doc->writeStartElement(KXMLDTCSurfaceTarget);
            const QVariantMap t = target.toMap();
            for (auto p = t.cbegin(); p != t.cend(); ++p)
                doc->writeAttribute(p.key(), toXmlValue(p.value()));
            doc->writeEndElement();
        }
        doc->writeEndElement();
    }
    doc->writeEndElement();
}

bool ShowSurface::saveXML(QXmlStreamWriter *doc) const
{
    doc->writeStartElement(KXMLDTCSurface);
    doc->writeAttribute("Version", "2");
    for (const Page &page : m_pages)
    {
        doc->writeStartElement(KXMLDTCSurfacePage);
        doc->writeAttribute("Name", page.name);
        saveNode(doc, page.root.get());
        doc->writeEndElement();
    }
    // the dimmers of the scenes (in the order of their IDs: the same show
    // saves the same file)
    QList<quint32> dimmed = m_sceneDimmers.keys();
    std::sort(dimmed.begin(), dimmed.end());
    for (quint32 fid : std::as_const(dimmed))
    {
        doc->writeStartElement(KXMLDTCSurfaceDimmer);
        doc->writeAttribute("Function", QString::number(fid));
        doc->writeAttribute("Value", QString::number(m_sceneDimmers.value(fid)));
        doc->writeEndElement();
    }
    doc->writeEndElement();
    return true;
}

std::unique_ptr<ShowSurface::Node> ShowSurface::loadNode(QXmlStreamReader &root)
{
    auto node = std::make_unique<Node>();
    QXmlStreamAttributes attrs = root.attributes();
    node->id = attrs.value("ID").toString();

    // keep the IDs unique as new sections are made (checkLoaded() gives new
    // ones to those out of range)
    int number = node->id.mid(1).toInt();
    if (number < INT_MAX / 2)
        m_lastId = qMax(m_lastId, number);

    if (root.name() == KXMLDTCSurfaceSplit)
    {
        node->split = attrs.value("Orientation") == QLatin1String("v") ? "v" : "h";
        bool ratioOk = false;
        const qreal ratio = attrs.value("Ratio").toDouble(&ratioOk);
        node->ratio = ratioOk && qIsFinite(ratio) ? qBound(0.1, ratio, 0.9) : 0.5;
        while (root.readNextStartElement())
        {
            // (what a newer version puts in a split isn't one of its halves)
            if (root.name() != KXMLDTCSurfaceSplit && root.name() != KXMLDTCSurfaceSection)
            {
                root.skipCurrentElement();
                continue;
            }
            std::unique_ptr<Node> child = loadNode(root);
            if (node->a == nullptr)
                node->a = std::move(child);
            else if (node->b == nullptr)
                node->b = std::move(child);
        }
        // a broken split becomes an empty section
        if (node->a == nullptr || node->b == nullptr)
        {
            node->split = "none";
            node->props = sectionDefaults("empty");
            node->a.reset();
            node->b.reset();
        }
        return node;
    }

    node->split = "none";
    for (const QXmlStreamAttribute &attr : attrs)
        if (attr.name() != QLatin1String("ID"))
            node->props.insert(attr.name().toString(), fromXmlValue(attr.name().toString(), attr.value().toString()));
    if (node->props.contains("type") == false)
        node->props = sectionDefaults("empty");
    // the size of a table as the editor allows it (a damaged file with
    // millions of cells would never be drawn)
    const QVariantMap defaults = sectionDefaults(node->props.value("type").toString());
    for (const QString &key : { QStringLiteral("cols"), QStringLiteral("rows") })
    {
        if (node->props.contains(key) == false)
            continue;
        bool ok = false;
        const double size = node->props.value(key).toDouble(&ok);
        node->props.insert(key, ok && qIsFinite(size) ? int(qBound(1.0, size, 32.0)) : defaults.value(key, 1).toInt());
    }

    QVariantMap items;
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLDTCSurfaceItem)
        {
            QVariantMap item;
            QString cell;
            for (const QXmlStreamAttribute &attr : root.attributes())
            {
                if (attr.name() == QLatin1String("Cell"))
                    cell = attr.value().toString();
                else
                    item.insert(attr.name().toString(), fromXmlValue(attr.name().toString(), attr.value().toString()));
            }
            // the range of a fader as the editor gives it (0-100 %)
            for (const QString &key : { QStringLiteral("min"), QStringLiteral("max") })
            {
                if (item.contains(key) == false)
                    continue;
                bool ok = false;
                const double percent = item.value(key).toDouble(&ok);
                if (ok && qIsFinite(percent))
                    item.insert(key, int(qBound(0.0, percent, 100.0)));
                else
                    item.remove(key);
            }
            QVariantList targets;
            while (root.readNextStartElement())
            {
                if (root.name() == KXMLDTCSurfaceTarget)
                {
                    QVariantMap t;
                    for (const QXmlStreamAttribute &attr : root.attributes())
                        t.insert(attr.name().toString(), fromXmlValue(attr.name().toString(), attr.value().toString()));
                    targets.append(t);
                }
                root.skipCurrentElement();
            }
            if (targets.isEmpty() == false)
                item.insert("targets", targets);
            if (cell.isEmpty() == false)
                items.insert(cell, item);
            continue;
        }
        root.skipCurrentElement();
    }
    if (items.isEmpty() == false)
        node->props.insert("items", items);
    return node;
}

void ShowSurface::loadVersion1Page(QXmlStreamReader &root, Page &page)
{
    // the free-form screens of before: their buttons and faders, in a table each
    QVariantMap buttons, faders;
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLDTCSurfaceWidget)
        {
            QVariantMap item;
            QString type;
            for (const QXmlStreamAttribute &attr : root.attributes())
            {
                if (attr.name() == QLatin1String("type"))
                    type = attr.value().toString();
                else if (QStringList({ "id", "x", "y", "w", "h" }).contains(attr.name().toString()) == false)
                    item.insert(attr.name().toString(), fromXmlValue(attr.name().toString(), attr.value().toString()));
            }
            if (type == "button")
                buttons.insert(QString::number(buttons.count()), item);
            else if (type == "slider")
                faders.insert(QString::number(faders.count()), item);
        }
        root.skipCurrentElement();
    }

    auto table = [&](const QString &type, const QVariantMap &items)
    {
        auto node = newSection(type);
        if (type == "buttons")
            node->props.insert("rows", qMax(4, int((items.count() + 5) / 6)));
        else
            node->props.insert("cols", qMax(4, int(items.count())));
        node->props.insert("items", items);
        return node;
    };

    if (buttons.isEmpty() == false && faders.isEmpty() == false)
    {
        page.root = std::make_unique<Node>();
        page.root->id = nextId("n");
        page.root->split = "h";
        page.root->ratio = 0.65;
        page.root->a = table("buttons", buttons);
        page.root->b = table("faders", faders);
    }
    else if (faders.isEmpty() == false)
    {
        page.root = table("faders", faders);
    }
    else
    {
        page.root = table("buttons", buttons);
    }
}

bool ShowSurface::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLDTCSurface)
    {
        qWarning() << Q_FUNC_INFO << "Surface node not found";
        return false;
    }

    bool version1 = root.attributes().hasAttribute("Version") == false;
    m_pages.clear();
    m_lastId = 0;
    clearSceneDimmers();

    while (root.readNextStartElement())
    {
        // the dimmer of a scene that is there (0..1)
        if (root.name() == KXMLDTCSurfaceDimmer)
        {
            bool idOk = false, valueOk = false;
            const quint32 fid = root.attributes().value("Function").toUInt(&idOk);
            const double dimmer = root.attributes().value("Value").toDouble(&valueOk);
            if (idOk && valueOk && qIsFinite(dimmer) && m_doc->function(fid) != nullptr && dimmer < 1.0)
                m_sceneDimmers.insert(fid, qBound(0.0, dimmer, 1.0));
            root.skipCurrentElement();
            continue;
        }
        if (root.name() != KXMLDTCSurfacePage)
        {
            root.skipCurrentElement();
            continue;
        }

        Page page;
        page.name = root.attributes().value("Name").toString();
        if (version1)
        {
            loadVersion1Page(root, page);
        }
        else
        {
            while (root.readNextStartElement())
            {
                if (page.root == nullptr &&
                    (root.name() == KXMLDTCSurfaceSplit || root.name() == KXMLDTCSurfaceSection))
                    page.root = loadNode(root);
                else
                    root.skipCurrentElement();
            }
        }
        if (page.root == nullptr)
            page.root = newSection("empty");
        // (the functions are loaded before the DMX Total Control section)
        bindGoToCues(page.root.get());
        m_pages.push_back(std::move(page));
    }

    checkLoaded();
    // (the functions are there: they get their dimmers, if a control is on them)
    pruneSceneDimmers();
    pruneHolds();
    for (auto it = m_sceneDimmers.cbegin(); it != m_sceneDimmers.cend(); ++it)
    {
        Function *f = m_doc->function(it.key());
        applySceneDimmer(f, it.value());
        connect(f, SIGNAL(running(quint32)), this, SLOT(slotFunctionRunning(quint32)), Qt::UniqueConnection);
    }
    emit pagesChanged();
    return true;
}

void ShowSurface::checkLoaded()
{
    // the sections are found by their IDs (the mappings of SHOW faders too)
    QSet<QString> seen;
    std::function<void(Node *)> walk = [&](Node *node) {
        if (node == nullptr)
            return;
        bool ok = false;
        const int number = node->id.mid(1).toInt(&ok);
        if (ok == false || number < 0 || number >= INT_MAX / 2 || seen.contains(node->id))
            node->id = nextId(node->split == "none" ? "s" : "n");
        seen.insert(node->id);
        walk(node->a.get());
        walk(node->b.get());
    };
    for (Page &page : m_pages)
        walk(page.root.get());

    // what drives a function that isn't there (or names none: a wrong
    // number) drives nothing, as when it is deleted (a function made later
    // gets its ID)
    forgetFunctions([this](const QVariant &id) {
        bool ok = false;
        const double number = id.toDouble(&ok);
        if (ok == false || qIsFinite(number) == false || number != std::floor(number) || number < 0 || number >= INT_MAX / 2)
            return !(ok && number == -1);
        return m_doc->function(quint32(number)) == nullptr;
    });
}

void ShowSurface::bindGoToCues(Node *node)
{
    if (node == nullptr)
        return;
    if (node->split != "none")
    {
        bindGoToCues(node->a.get());
        bindGoToCues(node->b.get());
        return;
    }

    QVariantMap items = node->props.value("items").toMap();
    bool itemsChanged = false;
    for (auto it = items.begin(); it != items.end(); ++it)
    {
        QVariantMap item = it.value().toMap();
        QVariantMap map = item.value("map").toMap();
        if (item.value("action").toString() != "map" || map.value("target").toString() != "playbackGoTo" ||
            map.value("cueUid", 0).toUInt() != 0)
            continue;
        const int fid = map.value("functionId", -1).toInt();
        const Cuelist *cuelist = fid >= 0 ? qobject_cast<const Cuelist *>(m_doc->function(quint32(fid))) : nullptr;
        const int index = map.value("cue").toInt();
        if (cuelist == nullptr || index < 0 || index >= cuelist->cueCount())
            continue;
        map.insert("cueUid", cuelist->cueAt(index).uid);
        item.insert("map", map);
        it.value() = item;
        itemsChanged = true;
    }
    if (itemsChanged)
        node->props.insert("items", items);
}

/*********************************************************************
 * Core, colour wheels, XY grids, Alt+click (v0.9)
 *********************************************************************/

void ShowSurface::setCore(DtcCore *core)
{
    m_core = core;
    // the playback buttons follow their playbacks (not their progress)
    if (m_core != nullptr && m_core->playbacks() != nullptr)
    {
        connect(m_core->playbacks(), &PlaybackManager::stateChanged, this, &ShowSurface::slotChanged);
        // the controls and the rows of playbacks of a page follow the page
        connect(m_core->playbacks(), &PlaybackManager::pagesRenumbered,
                this, &ShowSurface::slotPlaybackPagesRenumbered);
    }
    // (and the names of what the buttons drive: a renamed function)
    if (m_core != nullptr)
        connect(m_core, &DtcCore::functionsChanged, this, &ShowSurface::slotChanged);
    // the FX of the programmer go at the speed of the movement too
    if (m_core != nullptr && m_core->programmer() != nullptr)
        connect(m_core->programmer(), &Programmer::fxChanged, this, &ShowSurface::slotMotionChanged);
}

void ShowSurface::slotPlaybackPagesRenumbered(const QList<int> &newIndexes, const QHash<int, quint32> &removedSlots)
{
    bool any = false;
    std::function<void(Node *)> walk = [&](Node *node) {
        if (node == nullptr)
            return;
        if (node->split != "none")
        {
            walk(node->a.get());
            walk(node->b.get());
            return;
        }

        // a row of playbacks of a page: that page; deleted, the current one
        // (its header pages: never the playbacks of another page as its own)
        const int shown = node->props.value("playbackPage", -1).toInt();
        if (node->props.value("type").toString() == "playbacks" && shown >= 0 && shown < newIndexes.count() &&
            newIndexes.at(shown) != shown)
        {
            node->props.insert("playbackPage", newIndexes.at(shown));
            any = true;
        }

        // buttons and faders bound to a slot of a page (Alt+click on a playback)
        QVariantMap items = node->props.value("items").toMap();
        bool itemsChanged = false;
        for (auto it = items.begin(); it != items.end(); ++it)
        {
            QVariantMap item = it.value().toMap();
            bool changed = false;
            if (item.contains("map"))
            {
                QVariantMap map = item.value("map").toMap();
                if (ControlMapper::followPlaybackPages(map, newIndexes, removedSlots))
                {
                    item.insert("map", map);
                    changed = true;
                }
            }
            if (item.contains("targets"))
            {
                QVariantList targets = item.value("targets").toList();
                bool targetsChanged = false;
                for (QVariant &t : targets)
                {
                    QVariantMap target = t.toMap();
                    QVariantMap map = target.value("map").toMap();
                    if (target.value("target").toString() != "map" ||
                        ControlMapper::followPlaybackPages(map, newIndexes, removedSlots) == false)
                        continue;
                    target.insert("map", map);
                    t = target;
                    targetsChanged = true;
                }
                if (targetsChanged)
                {
                    item.insert("targets", targets);
                    changed = true;
                }
            }
            if (changed)
            {
                it.value() = item;
                itemsChanged = true;
            }
        }
        if (itemsChanged)
        {
            node->props.insert("items", items);
            any = true;
        }
    };
    for (Page &page : m_pages)
        walk(page.root.get());
    if (any)
        changed();
}

int ShowSurface::currentPage() const
{
    return m_currentPage;
}

void ShowSurface::setCurrentPage(int page)
{
    if (page == m_currentPage)
        return;
    m_currentPage = page;
    emit currentPageChanged();
}

QList<quint32> ShowSurface::sectionFixtures(const Node *node) const
{
    QList<quint32> list;
    if (node == nullptr)
        return list;
    const QString type = node->props.value("type").toString();
    const int groupId = node->props.value("groupId", -1).toInt();

    QList<quint32> candidates;
    if (groupId >= 0)
    {
        if (FixtureGroup *group = m_doc->fixtureGroup(quint32(groupId)))
            candidates = group->fixtureList();
    }
    else
    {
        for (Fixture *f : m_doc->fixtures())
            candidates.append(f->id());
    }

    // the fixtures that have what the control moves
    for (quint32 fid : std::as_const(candidates))
    {
        Fixture *fxi = m_doc->fixture(fid);
        if (fxi == nullptr || list.contains(fid))
            continue;
        bool has = false;
        for (quint32 c = 0; c < fxi->channels() && has == false; c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch == nullptr)
                continue;
            if (type == "xy")
                has = Programmer::isPositionChannel(ch);
            else
                has = ch->group() == QLCChannel::Intensity && ch->colour() != QLCChannel::NoColour;
        }
        if (has)
            list.append(fid);
    }
    return list;
}

void ShowSurface::setColour(int page, const QString &sectionId, const QColor &colour)
{
    Node *node = findSection(page, sectionId);
    if (node == nullptr || m_core == nullptr || colour.isValid() == false)
        return;
    m_core->programmer()->setColorFor(sectionFixtures(node), colour);
    slotChanged();
}

QColor ShowSurface::colour(int page, const QString &sectionId) const
{
    Node *node = findSection(page, sectionId);
    if (node == nullptr || m_core == nullptr)
        return QColor();
    return m_core->programmer()->colorFor(sectionFixtures(node));
}

void ShowSurface::setXY(int page, const QString &sectionId, qreal x, qreal y)
{
    Node *node = findSection(page, sectionId);
    if (node == nullptr || m_core == nullptr)
        return;
    m_core->programmer()->setPositionFor(sectionFixtures(node), x, y);
    slotChanged();
}

QVariantMap ShowSurface::xy(int page, const QString &sectionId) const
{
    Node *node = findSection(page, sectionId);
    if (node == nullptr || m_core == nullptr)
        return QVariantMap({ { "x", 0.5 }, { "y", 0.5 }, { "available", false } });
    return m_core->programmer()->positionFor(sectionFixtures(node));
}

void ShowSurface::releaseSection(int page, const QString &sectionId)
{
    Node *node = findSection(page, sectionId);
    if (node == nullptr || m_core == nullptr)
        return;
    const bool position = node->props.value("type").toString() == "xy";
    m_core->programmer()->releaseChannels(sectionFixtures(node), [position](const QLCChannel *ch) {
        if (position)
            return Programmer::isPositionChannel(ch);
        return ch->group() == QLCChannel::Intensity && ch->colour() != QLCChannel::NoColour;
    });
    slotChanged();
}

QVariantMap ShowSurface::addFromTarget(const QVariantMap &origin, int page)
{
    const QString kind = origin.value("target").toString();
    if (kind.isEmpty() || kind == "surfaceFader" || m_core == nullptr)
        return QVariantMap();

    // a control of a playback of the current page is bound to that playback,
    // not to whatever page is shown later
    QVariantMap target = origin;
    if (kind.startsWith("playback") && target.contains("slot") && target.value("pbPage", -1).toInt() < 0)
        target.insert("pbPage", m_core->playbacks()->currentPage());

    // what the control is: a button or a fader, the native ones where there is one
    const bool button = m_core->mapping()->isButton(kind);
    QVariantMap item = itemDefaults(button ? "buttons" : "faders");
    if (button)
    {
        static const QHash<QString, QString> actions = {
            { "scene", "scene" }, { "groupNext", "next" }, { "groupPrevious", "previous" },
            { "groupRelease", "release" }, { "blackout", "blackout" }, { "stopAll", "stopAll" },
            { "tap", "tap" }, { "beatgo", "beatgo" }
        };
        if (actions.contains(kind))
        {
            item.insert("action", actions.value(kind));
            item.insert("functionId", target.value("functionId", -1));
            item.insert("zoneId", target.value("zoneId", -1));
        }
        else if (kind == "palette")
        {
            // a palette button: on its own fixtures, with its colour
            const QVariantMap palette = m_core->palettes()->palette(target.value("paletteId", -1).toInt());
            item.insert("action", "palette");
            item.insert("paletteId", target.value("paletteId", -1));
            item.insert("groupId", target.value("groupId", -1));
            if (palette.value("color").toString().isEmpty() == false)
                item.insert("color", palette.value("color"));
        }
        else
        {
            item.insert("action", "map");
            item.insert("map", target);
            // (a playback is named when the button shows: its slot can
            // hold another function later)
            if (kind.startsWith("playback") == false)
                item.insert("label", m_core->mapping()->describeTarget(target));
        }
    }
    else
    {
        QVariantMap t;
        if (kind == "grandMaster" || kind == "bpm")
            t.insert("target", kind);
        else if (kind == "groupMaster")
            t = QVariantMap({ { "target", "group" }, { "zoneId", target.value("zoneId", -1) } });
        else if (kind == "mixerDimmer")
            t = QVariantMap({ { "target", "mixerDimmer" }, { "mixerId", target.value("mixerId", -1) } });
        // (the same speed: of its group, or of the scene alone)
        else if (kind == "sceneSpeed")
            t = QVariantMap({ { "target", "sceneSpeed" }, { "functionId", target.value("functionId", -1) } });
        else
        {
            t = QVariantMap({ { "target", "map" }, { "map", target } });
            if (kind.startsWith("playback") == false)
                item.insert("label", m_core->mapping()->describeTarget(target));
        }
        item.insert("targets", QVariantList({ t }));
        item.insert("target", t.value("target"));
    }

    // a page to put it on
    if (m_pages.empty())
        addPage(tr("Touch"));
    if (page < 0 || page >= int(m_pages.size()))
        page = qBound(0, m_currentPage, int(m_pages.size()) - 1);

    // the first free cell of a section of its kind
    const QString type = button ? "buttons" : "faders";
    std::function<Node *(Node *)> firstFree;
    int freeCell = -1;
    firstFree = [&](Node *node) -> Node * {
        if (node == nullptr)
            return nullptr;
        if (node->split != "none")
        {
            if (Node *n = firstFree(node->a.get()))
                return n;
            return firstFree(node->b.get());
        }
        if (node->props.value("type").toString() != type)
            return nullptr;
        int cells = node->props.value("cols", 1).toInt() * (button ? node->props.value("rows", 1).toInt() : 1);
        const QVariantMap items = node->props.value("items").toMap();
        for (int c = 0; c < cells; c++)
        {
            if (items.contains(QString::number(c)) == false)
            {
                freeCell = c;
                return node;
            }
        }
        return nullptr;
    };
    Node *section = firstFree(m_pages[size_t(page)].root.get());

    if (section == nullptr)
    {
        // none: an empty section becomes one, or the last section is split
        std::function<Node *(Node *)> findEmpty = [&](Node *node) -> Node * {
            if (node == nullptr)
                return nullptr;
            if (node->split != "none")
            {
                if (Node *n = findEmpty(node->a.get()))
                    return n;
                return findEmpty(node->b.get());
            }
            return node->props.value("type").toString() == "empty" ? node : nullptr;
        };
        Node *target = findEmpty(m_pages[size_t(page)].root.get());
        QString sid;
        if (target != nullptr)
        {
            sid = target->id;
        }
        else
        {
            Node *last = m_pages[size_t(page)].root.get();
            while (last != nullptr && last->split != "none")
                last = last->b.get();
            if (last == nullptr)
                return QVariantMap();
            sid = splitSection(page, last->id, button ? "v" : "h");
        }
        updateSection(page, sid, { { "type", type }, { "title", tr("Touch") } });
        section = findSection(page, sid);
        freeCell = 0;
        if (section == nullptr)
            return QVariantMap();
    }

    setItem(page, section->id, freeCell, item);
    QVariantMap where({ { "page", page }, { "section", section->id }, { "cell", freeCell },
                        { "pageName", m_pages[size_t(page)].name } });
    emit controlAdded(where);
    return where;
}
