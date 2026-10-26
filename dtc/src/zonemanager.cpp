/*
  DMX Total Control
  zonemanager.cpp

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
#include <QDebug>
#include <utility>

#include "zonemanager.h"
#include "zone.h"
#include "doc.h"
#include "function.h"

static const char *s_zoneColors[] = {
    "#3d8bfd", "#f59f00", "#2fb344", "#d63939", "#ae3ec9", "#17a2b8", "#f76707", "#74b816"
};

ZoneManager::ZoneManager(Doc *doc, QObject *parent)
    : QAbstractListModel(parent)
    , m_doc(doc)
    , m_paused(false)
    , m_nextZoneId(0)
{
    // (added, removed, a new show: allFolded too)
    connect(this, SIGNAL(countChanged()), this, SIGNAL(foldedChanged()));
}

ZoneManager::~ZoneManager()
{
    clear();
}

int ZoneManager::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_zones.count();
}

QVariant ZoneManager::data(const QModelIndex &index, int role) const
{
    if (index.isValid() == false || index.row() >= m_zones.count())
        return QVariant();

    Zone *zone = m_zones.at(index.row());
    switch (role)
    {
        case ZoneRole: return QVariant::fromValue(zone);
        case IdRole: return zone->id();
        case NameRole:
        case Qt::DisplayRole: return zone->name();
    }
    return QVariant();
}

QHash<int, QByteArray> ZoneManager::roleNames() const
{
    return {
        { ZoneRole, "zone" },
        { IdRole, "zoneId" },
        { NameRole, "name" }
    };
}

int ZoneManager::count() const
{
    return m_zones.count();
}

Zone *ZoneManager::zone(int index) const
{
    if (index < 0 || index >= m_zones.count())
        return nullptr;
    return m_zones.at(index);
}

Zone *ZoneManager::zoneById(quint32 id) const
{
    for (Zone *zone : m_zones)
    {
        if (zone->id() == id)
            return zone;
    }
    return nullptr;
}

quint32 ZoneManager::nextZoneId() const
{
    // never the ID of a group deleted: the SHOW controls, the mappings and
    // the remote that pointed at it would drive the new one
    quint32 id = m_nextZoneId;
    for (Zone *zone : m_zones)
        id = qMax(id, zone->id() + 1);
    return id;
}

Zone *ZoneManager::addZone(const QString &name)
{
    const quint32 id = nextZoneId();
    m_nextZoneId = id + 1;
    Zone *zone = new Zone(m_doc, id, this);
    follow(zone);
    zone->setName(name.isEmpty() ? tr("Group %1").arg(m_zones.count() + 1) : name);
    zone->setColor(QColor(s_zoneColors[m_zones.count() % (sizeof(s_zoneColors) / sizeof(s_zoneColors[0]))]));

    beginInsertRows(QModelIndex(), m_zones.count(), m_zones.count());
    m_zones.append(zone);
    endInsertRows();

    m_doc->setModified();
    emit countChanged();
    return zone;
}

void ZoneManager::removeZone(int index)
{
    if (index < 0 || index >= m_zones.count())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    Zone *zone = m_zones.takeAt(index);
    endRemoveRows();

    delete zone;
    m_doc->setModified();
    emit countChanged();
}

void ZoneManager::moveZone(int from, int to)
{
    if (from < 0 || from >= m_zones.count() || to < 0 || to >= m_zones.count() || from == to)
        return;

    // beginMoveRows wants the destination as "insert before" row
    if (beginMoveRows(QModelIndex(), from, from, QModelIndex(), to > from ? to + 1 : to) == false)
        return;
    m_zones.move(from, to);
    endMoveRows();

    m_doc->setModified();
}

void ZoneManager::stopAll()
{
    for (Zone *zone : std::as_const(m_zones))
        zone->stopAll();
}

void ZoneManager::nextAll()
{
    for (Zone *zone : std::as_const(m_zones))
    {
        if (zone->activeCount() > 0)
            zone->next();
    }
}

void ZoneManager::previousAll()
{
    for (Zone *zone : std::as_const(m_zones))
    {
        if (zone->activeCount() > 0)
            zone->previous();
    }
}

bool ZoneManager::paused() const
{
    return m_paused;
}

void ZoneManager::setPaused(bool paused)
{
    if (paused == m_paused)
        return;
    m_paused = paused;

    if (paused)
    {
        // what plays; what was paused already stays so at the resume (a
        // scene waiting for its second press, a cuelist paused on its playback)
        m_pausedFunctions.clear();
        for (Function *f : m_doc->functions())
        {
            if (f->isRunning() && f->isPaused() == false)
            {
                f->setPause(true);
                m_pausedFunctions.insert(f->id());
            }
        }
    }
    else
    {
        for (quint32 id : std::as_const(m_pausedFunctions))
        {
            Function *f = m_doc->function(id);
            if (f != nullptr && f->isPaused())
                f->setPause(false);
        }
        m_pausedFunctions.clear();
    }
    emit pausedChanged();
}

void ZoneManager::resetPause()
{
    // the functions of the show before are gone with it
    m_pausedFunctions.clear();
    if (m_paused == false)
        return;
    m_paused = false;
    emit pausedChanged();
}

bool ZoneManager::allFolded() const
{
    if (m_zones.isEmpty())
        return false;
    for (Zone *zone : m_zones)
    {
        if (zone->folded() == false)
            return false;
    }
    return true;
}

void ZoneManager::setAllFolded(bool folded)
{
    for (Zone *zone : m_zones)
        zone->setFolded(folded);
}

void ZoneManager::follow(Zone *zone)
{
    connect(zone, SIGNAL(foldedChanged()), this, SIGNAL(foldedChanged()));
}

void ZoneManager::releaseFor(Zone *source, quint32 fid)
{
    if (source->effectiveRelease(fid) == Zone::ReleaseNone)
        return;

    for (Zone *zone : std::as_const(m_zones))
    {
        for (quint32 running : zone->activeFunctions())
        {
            if (releases(source, fid, zone, running))
                zone->stop(running);
        }
    }
}

bool ZoneManager::releases(Zone *source, quint32 fid, Zone *zone, quint32 running) const
{
    if (source == nullptr || zone == nullptr)
        return false;
    const bool same = (zone == source);
    if (same && running == fid)
        return false;

    const Zone::ReleaseMode mode = source->effectiveRelease(fid);
    const bool target = (mode == Zone::ReleaseAll) ||
                        (mode == Zone::ReleaseGroup && same) ||
                        (mode == Zone::ReleaseOthers && !same);
    if (target == false)
        return false;

    const Zone::Protection p = zone->protection(running);
    const bool isProtected = (p == Zone::ProtectAll) ||
                             (p == Zone::ProtectGroup && same) ||
                             (p == Zone::ProtectOthers && !same);
    return isProtected == false;
}

void ZoneManager::deleteZones()
{
    // Every group stops while all of them are still alive and listed: a stop
    // tells the SHOW right away, and what it shows reads the groups again
    for (Zone *zone : std::as_const(m_zones))
        zone->stopAll();

    // then they leave the list before they go (nothing finds a deleted one)
    const QList<Zone *> old = std::exchange(m_zones, QList<Zone *>());
    qDeleteAll(old);
}

void ZoneManager::clear()
{
    resetPause();
    m_nextZoneId = 0;
    if (m_zones.isEmpty())
        return;

    beginResetModel();
    deleteZones();
    endResetModel();

    emit countChanged();
}

bool ZoneManager::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLDTCZones)
    {
        qWarning() << Q_FUNC_INFO << "Zones node not found";
        return false;
    }

    resetPause();
    beginResetModel();
    deleteZones();
    // the IDs given before, deleted groups included (the UI and the mappings
    // hold them as ints: a damaged file's out of range ones aren't taken)
    const quint32 maxId = INT_MAX / 2;
    m_nextZoneId = qMin(root.attributes().value(KXMLDTCZonesNextID).toUInt(), maxId);

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLDTCZone)
        {
            bool idOk = false;
            quint32 id = root.attributes().value("ID").toUInt(&idOk);
            if (idOk == false || id >= maxId || zoneById(id) != nullptr)
                id = nextZoneId();

            Zone *zone = new Zone(m_doc, id, this);
            if (zone->loadXML(root))
            {
                follow(zone);
                m_zones.append(zone);
                m_nextZoneId = qMax(m_nextZoneId, id + 1);
            }
            else
            {
                delete zone;
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Zones tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    endResetModel();
    qInfo() << "Loaded" << m_zones.count() << "zones";
    emit countChanged();
    return true;
}

bool ZoneManager::saveXML(QXmlStreamWriter *doc) const
{
    doc->writeStartElement(KXMLDTCZones);
    // (the ID of a deleted group isn't given again after a reload either)
    doc->writeAttribute(KXMLDTCZonesNextID, QString::number(nextZoneId()));
    for (Zone *zone : m_zones)
        zone->saveXML(doc);
    doc->writeEndElement();
    return true;
}

void ZoneManager::moveScene(int fromZone, quint32 functionId, int toZone, int toIndex)
{
    Zone *source = zone(fromZone);
    Zone *target = zone(toZone);
    if (source == nullptr || target == nullptr)
        return;

    QList<quint32> sourceList = source->functionIDs();
    int from = int(sourceList.indexOf(functionId));
    if (from < 0)
        return;

    if (source == target)
    {
        // dropped before $toIndex: once taken out, the places after it shift
        int to = toIndex < 0 ? int(sourceList.count()) - 1 : (toIndex > from ? toIndex - 1 : toIndex);
        source->moveFunction(from, qBound(0, to, int(sourceList.count()) - 1));
        return;
    }

    if (target->functionIDs().contains(functionId))
        return;

    // the options of the scene go with it
    QVariantMap options = source->options(functionId);
    source->removeFunction(functionId);
    target->addFunction(functionId);
    for (auto it = options.cbegin(); it != options.cend(); ++it)
        if (it.key() != "waiting")
            target->setOption(functionId, it.key(), it.value());

    int last = int(target->functionIDs().count()) - 1;
    if (toIndex >= 0 && toIndex < last)
        target->moveFunction(last, toIndex);
}
