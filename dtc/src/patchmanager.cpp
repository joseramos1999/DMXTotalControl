/*
  DMX Total Control
  patchmanager.cpp

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

#include <QDebug>
#include <QVector3D>
#include <QTimer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QBuffer>
#include <QSet>
#include <algorithm>

#include "patchmanager.h"
#include "qlcfixturedefcache.h"
#include "monitorproperties.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "inputoutputmap.h"
#include "genericdmxsource.h"
#include "qlcmodifierscache.h"
#include "channelmodifier.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "fixturegroup.h"
#include "universe.h"
#include "fixture.h"
#include "doc.h"

/* Spacing of the automatic 2D layout, in millimeters */
#define AUTO_LAYOUT_STEP 600.0

/* The highest fixture number a show file can give (the patch editor's too) */
static const int kMaxFixtureNumber = 99999;

PatchManager::PatchManager(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_identifySource(new GenericDMXSource(doc))
    , m_blinkTimer(new QTimer(this))
    , m_fixturesTimer(new QTimer(this))
    , m_blinkOn(false)
    , m_lastSelectionId(0)
{
    // A show loading or cleared, or many fixtures patched, add or remove them
    // one by one: the list (read whole by the UI, in several places) is told
    // once, when Doc is done
    m_fixturesTimer->setSingleShot(true);
    m_fixturesTimer->setInterval(0);
    connect(m_fixturesTimer, &QTimer::timeout, this, &PatchManager::notifyFixtures);

    // first: the fixture has its number when the others hear of it
    connect(m_doc, SIGNAL(fixtureAdded(quint32)), this, SLOT(slotFixtureAdded(quint32)));
    connect(m_doc, SIGNAL(fixtureAdded(quint32)), this, SLOT(slotFixturesDirty()));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotFixturesDirty()));
    connect(m_doc, SIGNAL(fixtureChanged(quint32)), this, SLOT(slotFixtureChanged(quint32)));
    connect(m_doc, SIGNAL(loaded()), this, SLOT(notifyFixtures()));
    connect(m_doc, SIGNAL(cleared()), this, SLOT(notifyFixtures()));

    connect(m_doc, SIGNAL(fixtureGroupAdded(quint32)), this, SIGNAL(groupsChanged()));
    connect(m_doc, SIGNAL(fixtureGroupRemoved(quint32)), this, SIGNAL(groupsChanged()));
    connect(m_doc, SIGNAL(fixtureGroupChanged(quint32)), this, SIGNAL(groupsChanged()));
    connect(m_doc, SIGNAL(loaded()), this, SIGNAL(groupsChanged()));
    connect(m_doc, SIGNAL(cleared()), this, SIGNAL(groupsChanged()));
    connect(m_doc, SIGNAL(loaded()), this, SIGNAL(stageSizeChanged()));
    connect(m_doc, SIGNAL(cleared()), this, SIGNAL(stageSizeChanged()));

    m_identifySource->setOutputEnabled(true);
    m_blinkTimer->setInterval(350);
    connect(m_blinkTimer, &QTimer::timeout, this, &PatchManager::slotBlink);
    connect(m_doc, SIGNAL(cleared()), this, SLOT(slotDocCleared()));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotFixtureRemoved()));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotFixtureGone(quint32)));
    connect(m_doc, SIGNAL(fixtureGroupRemoved(quint32)), this, SLOT(slotGroupRemoved(quint32)));

    // shows saved with an inverted axis need the modifier when they load
    invertModifier();
}

PatchManager::~PatchManager()
{
    delete m_identifySource;
}

void PatchManager::slotFixturesDirty()
{
    m_fixturesTimer->start();
}

void PatchManager::notifyFixtures()
{
    m_fixturesTimer->stop();
    emit fixturesChanged();
}

void PatchManager::slotFixtureChanged(quint32 id)
{
    // an identified fixture with another mode or address: the blinking
    // follows it (the channels of its old mode would stay lit, on whatever
    // is patched after it now)
    if (m_identified.contains(id))
    {
        m_identifySource->unsetAll();
        writeIdentify();
    }
    slotFixturesDirty();
}

void PatchManager::applyPatch(Doc *doc, Fixture *fxi, QLCFixtureDef *def, QLCFixtureMode *mode,
                              quint32 universe, quint32 address)
{
    // the timer ticks with the universes claimed: none while they are here
    doc->inputOutputMap()->claimUniverses();
    {
        // Doc hears of it only at the end: each setter tells it, with a
        // range in between that may overlap another fixture
        const QSignalBlocker blocker(fxi);
        if (def != nullptr && mode != nullptr && (def != fxi->fixtureDef() || mode != fxi->fixtureMode()))
            fxi->setFixtureDefinition(def, mode);
        fxi->setUniverse(universe);
        fxi->setAddress(address);
    }
    doc->inputOutputMap()->releaseUniverses(false);

    // now Doc (the map of the addresses) and the rest hear of it, once
    fxi->setAddress(address);
}

QVariantList PatchManager::fixtures() const
{
    MonitorProperties *monitor = m_doc->monitorProperties();
    QVariantList list;
    for (Fixture *fxi : m_doc->fixtures())
    {
        QVariantMap info;
        info.insert("id", fxi->id());
        info.insert("name", fxi->name());
        info.insert("number", m_numbers.value(fxi->id(), 0));
        info.insert("type", fxi->typeString());
        info.insert("universe", fxi->universe());
        info.insert("address", fxi->address() + 1);
        info.insert("channels", fxi->channels());

        QLCFixtureDef *def = fxi->fixtureDef();
        QLCFixtureMode *mode = fxi->fixtureMode();
        // generic dimmers and RGB panels are built on the fly, with a
        // "Generic - Generic" definition that says nothing to the user
        bool generic = def == nullptr || (def->manufacturer() == "Generic" && def->model() == "Generic");
        info.insert("manufacturer", generic ? QString() : def->manufacturer());
        info.insert("model", generic ? QString() : def->model());
        info.insert("mode", mode != nullptr ? mode->name() : QString());
        info.insert("editableMode", generic == false);

        bool hasPan = false, hasTilt = false;
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            hasPan = hasPan || (ch != nullptr && ch->group() == QLCChannel::Pan);
            hasTilt = hasTilt || (ch != nullptr && ch->group() == QLCChannel::Tilt);
        }
        info.insert("hasPan", hasPan);
        info.insert("hasTilt", hasTilt);
        info.insert("invertPan", hasPan && isInverted(fxi, QLCChannel::Pan));
        info.insert("invertTilt", hasTilt && isInverted(fxi, QLCChannel::Tilt));
        info.insert("identified", m_identified.contains(fxi->id()));
        info.insert("swapPanTilt", fxi->swapPanTilt());
        // limited: a limit on the dimmer, the pan or the tilt (not just an inversion)
        auto limited = [&](int group, int colour) {
            QPair<int, int> r = channelRange(fxi, group, colour);
            return qMin(r.first, r.second) > 0 || qMax(r.first, r.second) < 255;
        };
        info.insert("limited", limited(QLCChannel::Intensity, QLCChannel::NoColour) ||
                               (hasPan && limited(QLCChannel::Pan, -1)) || (hasTilt && limited(QLCChannel::Tilt, -1)));

        // the plan draws fixtures with several heads (bars, matrices) as a row of cells
        info.insert("heads", fxi->heads());
        info.insert("width", mode != nullptr ? mode->physical().width() : 0);

        bool positioned = monitor != nullptr && monitor->containsFixture(fxi->id());
        QVector3D pos = positioned ? monitor->fixturePosition(fxi->id(), 0, 0) : QVector3D();
        info.insert("positioned", positioned);
        info.insert("x", pos.x());
        info.insert("y", pos.y());
        info.insert("rotation", positioned ? monitor->fixtureRotation(fxi->id(), 0, 0).z() : 0.0);
        info.insert("shape", m_shapes.value(fxi->id(), "round"));
        list.append(info);
    }
    return list;
}

int PatchManager::fixturesCount() const
{
    return m_doc->fixturesCount();
}

QStringList PatchManager::manufacturers() const
{
    QStringList list = m_doc->fixtureDefCache()->manufacturers();
    list.removeAll("Generic");
    list.sort(Qt::CaseInsensitive);
    // the basic generic fixtures are the first thing to look at
    list.prepend("Generic");
    return list;
}

QStringList PatchManager::models(const QString &manufacturer) const
{
    QStringList list = m_doc->fixtureDefCache()->models(manufacturer);
    list.sort(Qt::CaseInsensitive);
    if (manufacturer == "Generic")
        list.prepend(KGenericDimmerModel);
    return list;
}

QVariantList PatchManager::modes(const QString &manufacturer, const QString &model) const
{
    QVariantList list;

    if (model == KGenericDimmerModel)
    {
        QVariantMap info;
        info.insert("name", tr("Dimmers"));
        info.insert("channels", 1);
        list.append(info);
        return list;
    }

    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef(manufacturer, model);
    if (def == nullptr)
        return list;

    for (QLCFixtureMode *mode : def->modes())
    {
        QVariantMap info;
        info.insert("name", mode->name());
        info.insert("channels", mode->channels().count());
        list.append(info);
    }
    return list;
}

bool PatchManager::isRangeFree(int universe, int address, int channels, quint32 ignoreId) const
{
    return isRangeFree(universe, address, channels, QList<quint32>() << ignoreId);
}

bool PatchManager::isRangeFree(int universe, int address, int channels, const QList<quint32> &ignoreIds) const
{
    // address is 0-based here
    if (address < 0 || address + channels > UNIVERSE_SIZE)
        return false;

    for (Fixture *fxi : m_doc->fixtures())
    {
        if (ignoreIds.contains(fxi->id()) || int(fxi->universe()) != universe)
            continue;

        int start = int(fxi->address());
        int end = start + int(fxi->channels());
        if (address < end && start < address + channels)
            return false;
    }
    return true;
}

int PatchManager::nextFreeAddress(int universe, int channels) const
{
    if (channels <= 0)
        channels = 1;

    for (int address = 0; address + channels <= UNIVERSE_SIZE; address++)
    {
        if (isRangeFree(universe, address, channels, Fixture::invalidId()))
            return address + 1;
    }
    return -1;
}

int PatchManager::addFixtures(const QString &manufacturer, const QString &model,
                              const QString &mode, const QString &name,
                              int universe, int address, int quantity,
                              int gap, int channels,
                              const QString &groupName)
{
    InputOutputMap *ioMap = m_doc->inputOutputMap();
    const bool isDimmer = (model == KGenericDimmerModel);
    m_lastError.clear();

    QLCFixtureDef *def = nullptr;
    QLCFixtureMode *fxMode = nullptr;

    if (isDimmer == false)
    {
        def = m_doc->fixtureDefCache()->fixtureDef(manufacturer, model);
        fxMode = def != nullptr ? def->mode(mode) : nullptr;
        if (fxMode == nullptr && def != nullptr && def->modes().isEmpty() == false)
            fxMode = def->modes().first();
        if (def == nullptr || fxMode == nullptr)
        {
            qWarning() << Q_FUNC_INFO << "Fixture definition not found:" << manufacturer << model << mode;
            m_lastError = tr("Fixture %1 %2 not found in the library").arg(manufacturer, model);
            return 0;
        }
        channels = fxMode->channels().count();
    }

    channels = qMax(1, channels);
    gap = qMax(0, gap);
    // a fixture lives in one universe: the universe writes nothing past its end
    if (channels > UNIVERSE_SIZE)
    {
        m_lastError = tr("A fixture of %1 channels doesn't fit in a universe").arg(channels);
        return 0;
    }

    // where each one goes, checked before patching any: all of them or none
    QList<QPair<quint32, quint32>> places;
    quint32 fxAddress = quint32(qMax(1, address) - 1);
    quint32 uniIdx = quint32(qMax(0, universe));
    for (int i = 0; i < quantity; i++)
    {
        if (fxAddress + channels > UNIVERSE_SIZE)
        {
            // continue on the next universe (created when patching)
            uniIdx++;
            fxAddress = 0;
        }
        if (isRangeFree(int(uniIdx), int(fxAddress), channels, Fixture::invalidId()) == false)
        {
            m_lastError = tr("Addresses %1-%2 of universe %3 are used by another fixture")
                              .arg(fxAddress + 1).arg(fxAddress + quint32(channels)).arg(uniIdx + 1);
            return 0;
        }
        places.append(qMakePair(uniIdx, fxAddress));
        fxAddress += quint32(channels + gap);
    }

    int added = 0;
    QVariantList addedIds;
    for (int i = 0; i < places.count(); i++)
    {
        const quint32 uni = places.at(i).first;
        while (uni >= ioMap->universesCount())
        {
            ioMap->addUniverse();
            ioMap->startUniverses();
        }

        Fixture *fxi = new Fixture(m_doc);
        fxi->setAddress(places.at(i).second);
        fxi->setUniverse(uni);

        if (isDimmer)
        {
            QLCFixtureDef *dimmerDef = fxi->genericDimmerDef(channels);
            fxi->setFixtureDefinition(dimmerDef, fxi->genericDimmerMode(dimmerDef, channels));
        }
        else
        {
            fxi->setFixtureDefinition(def, fxMode);
        }

        QString baseName = name.isEmpty() ? (isDimmer ? tr("Dimmer") : model) : name;
        fxi->setName(quantity > 1 ? QString("%1 %2").arg(baseName).arg(i + 1) : baseName);

        if (m_doc->addFixture(fxi) == false)
        {
            // (checked above: Doc refuses it all the same) the others go too
            delete fxi;
            for (const QVariant &id : std::as_const(addedIds))
                m_doc->deleteFixture(id.toUInt());
            m_doc->refreshChannelCapabilities();
            m_lastError = tr("Addresses %1-%2 of universe %3 are used by another fixture")
                              .arg(places.at(i).second + 1).arg(places.at(i).second + quint32(channels)).arg(uni + 1);
            notifyFixtures();
            return 0;
        }

        added++;
        addedIds.append(fxi->id());
    }

    if (added > 0)
    {
        autoLayout(false);
        if (groupName.trimmed().isEmpty() == false)
            createGroup(groupName, addedIds);
        notifyFixtures();
    }

    return added;
}

QString PatchManager::lastError() const
{
    return m_lastError;
}

void PatchManager::removeFixture(quint32 id)
{
    if (m_doc->deleteFixture(id))
    {
        m_doc->refreshChannelCapabilities();
        notifyFixtures();
    }
}

void PatchManager::renameFixture(quint32 id, const QString &name)
{
    Fixture *fxi = m_doc->fixture(id);
    if (fxi == nullptr || name.isEmpty())
        return;

    fxi->setName(name);
    m_doc->setModified();
    notifyFixtures();
}

bool PatchManager::setFixtureAddress(quint32 id, int universe, int address)
{
    Fixture *fxi = m_doc->fixture(id);
    if (fxi == nullptr)
        return false;

    int addr0 = address - 1;
    if (universe < 0 || quint32(universe) >= m_doc->inputOutputMap()->universesCount() ||
        isRangeFree(universe, addr0, int(fxi->channels()), id) == false)
        return false;

    applyPatch(m_doc, fxi, nullptr, nullptr, quint32(universe), quint32(addr0));
    m_doc->setModified();
    m_doc->refreshChannelCapabilities();
    notifyFixtures();
    return true;
}

void PatchManager::removeFixtures(const QVariantList &ids)
{
    bool changed = false;
    for (const QVariant &id : ids)
        changed |= m_doc->deleteFixture(id.toUInt());

    if (changed)
    {
        m_doc->refreshChannelCapabilities();
        notifyFixtures();
    }
}

QString PatchManager::updateFixture(quint32 id, const QString &name, const QString &mode,
                                    int universe, int address)
{
    Fixture *fxi = m_doc->fixture(id);
    if (fxi == nullptr)
        return tr("Fixture not found");

    QLCFixtureDef *def = fxi->fixtureDef();
    QLCFixtureMode *newMode = fxi->fixtureMode();
    // its definition was updated in the library since (and its new mode
    // didn't fit then): edited, it takes the new version
    QLCFixtureDef *current = def != nullptr ? m_doc->fixtureDefCache()->fixtureDef(def->manufacturer(), def->model())
                                            : nullptr;
    if (current != nullptr && current != def)
    {
        def = current;
        const QString modeName = mode.isEmpty() && newMode != nullptr ? newMode->name() : mode;
        newMode = def->mode(modeName);
        if (newMode == nullptr)
            return tr("Mode %1 not found").arg(modeName);
    }
    else if (def != nullptr && mode.isEmpty() == false && (newMode == nullptr || newMode->name() != mode))
    {
        newMode = def->mode(mode);
        if (newMode == nullptr)
            return tr("Mode %1 not found").arg(mode);
    }

    int channels = newMode != nullptr ? newMode->channels().count() : int(fxi->channels());
    if (universe < 0 || quint32(universe) >= m_doc->inputOutputMap()->universesCount())
        return tr("Universe %1 doesn't exist").arg(universe + 1);
    if (address < 1 || address - 1 + channels > UNIVERSE_SIZE)
        return tr("The fixture doesn't fit in the universe at address %1").arg(address);
    if (isRangeFree(universe, address - 1, channels, id) == false)
        return tr("Addresses %1-%2 are used by another fixture").arg(address).arg(address + channels - 1);

    if (name.trimmed().isEmpty() == false)
        fxi->setName(name.trimmed());
    applyPatch(m_doc, fxi, def, newMode, quint32(universe), quint32(address - 1));

    m_doc->setModified();
    m_doc->refreshChannelCapabilities();
    notifyFixtures();
    return QString();
}

QString PatchManager::readdressFixtures(const QVariantList &ids, int universe, int address, int gap)
{
    QList<quint32> fixtureIds;
    for (const QVariant &v : ids)
    {
        if (m_doc->fixture(v.toUInt()) != nullptr)
            fixtureIds.append(v.toUInt());
    }
    if (fixtureIds.isEmpty())
        return QString();
    if (universe < 0 || quint32(universe) >= m_doc->inputOutputMap()->universesCount())
        return tr("Universe %1 doesn't exist").arg(universe + 1);

    // check everything first, so the operation is all or nothing
    QList<QPair<quint32, int>> plan;
    int addr = address - 1;
    for (quint32 id : fixtureIds)
    {
        int channels = int(m_doc->fixture(id)->channels());
        if (isRangeFree(universe, addr, channels, fixtureIds) == false)
            return tr("Not enough free addresses from %1").arg(address);
        plan.append(qMakePair(id, addr));
        addr += channels + qMax(0, gap);
    }

    for (const auto &step : plan)
        applyPatch(m_doc, m_doc->fixture(step.first), nullptr, nullptr, quint32(universe), quint32(step.second));

    m_doc->setModified();
    m_doc->refreshChannelCapabilities();
    notifyFixtures();
    return QString();
}

int PatchManager::importFixtures(Doc *from, QHash<quint32, quint32> *copies)
{
    QList<Fixture *> sources = from->fixtures();
    std::sort(sources.begin(), sources.end(), [](Fixture *a, Fixture *b) {
        return a->universeAddress() < b->universeAddress();
    });

    InputOutputMap *ioMap = m_doc->inputOutputMap();
    MonitorProperties *monitor = m_doc->monitorProperties();
    QHash<quint32, quint32> fixtureMap;
    for (Fixture *src : sources)
    {
        // a copy through its XML: definition, mode, channel options, limits
        QByteArray xml;
        QBuffer buffer(&xml);
        buffer.open(QIODevice::WriteOnly);
        QXmlStreamWriter writer(&buffer);
        src->saveXML(&writer);
        buffer.close();
        QXmlStreamReader reader(xml);
        reader.readNextStartElement();
        Fixture *fxi = new Fixture(m_doc);
        if (fxi->loadXML(reader, m_doc, m_doc->fixtureDefCache()) == false)
        {
            delete fxi;
            continue;
        }

        // a fixture across universes (in a show from QLC+) longer than one: it can't be patched here
        int channels = int(fxi->channels());
        if (channels > UNIVERSE_SIZE)
        {
            delete fxi;
            continue;
        }

        // its own address when free, else the next free one from there on in
        // the universes there are; else a new universe
        const int srcUniverse = int(src->universe());
        int universe = -1;
        int address = -1;
        for (int u = srcUniverse; u < int(ioMap->universesCount()) && address < 0; u++)
        {
            for (int a = u == srcUniverse ? int(src->address()) : 0; a + channels <= UNIVERSE_SIZE; a++)
            {
                if (isRangeFree(u, a, channels, Fixture::invalidId()))
                {
                    universe = u;
                    address = a;
                    break;
                }
            }
        }
        if (address < 0)
        {
            universe = qMax(srcUniverse, int(ioMap->universesCount()));
            address = universe == srcUniverse && int(src->address()) + channels <= UNIVERSE_SIZE ? int(src->address()) : 0;
            while (quint32(universe) >= ioMap->universesCount())
            {
                ioMap->addUniverse();
                ioMap->startUniverses();
            }
        }
        fxi->setUniverse(quint32(universe));
        fxi->setAddress(quint32(address));
        // it fits in its universe now
        fxi->setCrossUniverse(false);
        if (m_doc->addFixture(fxi) == false)
        {
            delete fxi;
            continue;
        }
        fixtureMap.insert(src->id(), fxi->id());

        if (from->monitorProperties()->containsFixture(src->id()))
        {
            monitor->setFixturePosition(fxi->id(), 0, 0, from->monitorProperties()->fixturePosition(src->id(), 0, 0));
            monitor->setFixtureRotation(fxi->id(), 0, 0, from->monitorProperties()->fixtureRotation(src->id(), 0, 0));
        }
    }

    // the fixture groups, on the new fixtures (not the ones of the colour effects)
    for (FixtureGroup *src : from->fixtureGroups())
    {
        if (src->name().endsWith(" (FX)") || src->name().endsWith(" (FX 2D)"))
            continue;
        FixtureGroup *group = new FixtureGroup(m_doc);
        group->setName(src->name());
        group->setSize(src->size());
        if (m_doc->addFixtureGroup(group) == false)
        {
            delete group;
            continue;
        }
        QMap<QLCPoint, GroupHead> heads = src->headsMap();
        for (auto it = heads.cbegin(); it != heads.cend(); ++it)
            if (fixtureMap.contains(it.value().fxi))
                group->assignHead(it.key(), GroupHead(fixtureMap.value(it.value().fxi), it.value().head));
    }

    if (fixtureMap.isEmpty() == false)
    {
        autoLayout(false);
        m_doc->setModified();
        notifyFixtures();
        emit groupsChanged();
    }
    if (copies != nullptr)
        *copies = fixtureMap;
    return int(fixtureMap.count());
}

/*********************************************************************
 * Fixture groups
 *********************************************************************/

QVariantList PatchManager::groups() const
{
    QVariantList list;
    for (FixtureGroup *grp : m_doc->fixtureGroups())
    {
        // the groups made for colour effects belong to them, not to the user
        if (grp->name().endsWith(" (FX)") || grp->name().endsWith(" (FX 2D)"))
            continue;

        QVariantList fixtures;
        for (quint32 fid : grp->fixtureList())
            fixtures.append(fid);

        QVariantMap info;
        info.insert("id", grp->id());
        info.insert("name", grp->name());
        info.insert("fixtures", fixtures);
        list.append(info);
    }
    return list;
}

int PatchManager::createGroup(const QString &name, const QVariantList &fixtureIds)
{
    FixtureGroup *grp = new FixtureGroup(m_doc);
    grp->setName(name.trimmed().isEmpty() ? tr("Group %1").arg(m_doc->fixtureGroups().count() + 1)
                                          : name.trimmed());
    if (m_doc->addFixtureGroup(grp) == false)
    {
        delete grp;
        return -1;
    }

    setGroupFixtures(grp->id(), fixtureIds);
    return int(grp->id());
}

void PatchManager::deleteGroup(quint32 id)
{
    if (m_doc->deleteFixtureGroup(id))
        emit groupsChanged();
}

void PatchManager::renameGroup(quint32 id, const QString &name)
{
    FixtureGroup *grp = m_doc->fixtureGroup(id);
    if (grp == nullptr || name.trimmed().isEmpty())
        return;

    grp->setName(name.trimmed());
    m_doc->setModified();
    emit groupsChanged();
}

void PatchManager::setGroupFixtures(quint32 id, const QVariantList &fixtureIds)
{
    FixtureGroup *grp = m_doc->fixtureGroup(id);
    if (grp == nullptr)
        return;

    // an RGB matrix playing on the group copies its heads in the timer
    // thread: they change while it doesn't tick (the universes claimed), and
    // the group tells the others once, after
    m_doc->inputOutputMap()->claimUniverses();
    {
        const QSignalBlocker blocker(grp);
        grp->reset();
        grp->setSize(QSize(qMax(1, int(fixtureIds.count())), 1));
        for (const QVariant &v : fixtureIds)
        {
            if (m_doc->fixture(v.toUInt()) != nullptr)
                grp->assignFixture(v.toUInt());
        }
    }
    m_doc->inputOutputMap()->releaseUniverses(false);
    grp->setSize(grp->size());

    m_doc->setModified();
    emit groupsChanged();
}

/*********************************************************************
 * 2D stage plan
 *********************************************************************/

QSizeF PatchManager::stageSize() const
{
    QVector3D grid = m_doc->monitorProperties()->gridSize();
    return QSizeF(grid.x() * 1000.0, grid.y() * 1000.0);
}

void PatchManager::setStageSize(qreal width, qreal height)
{
    MonitorProperties *monitor = m_doc->monitorProperties();
    QVector3D grid = monitor->gridSize();
    grid.setX(qBound(1.0, width / 1000.0, 200.0));
    grid.setY(qBound(1.0, height / 1000.0, 200.0));
    monitor->setGridSize(grid);
    m_doc->setModified();
    emit stageSizeChanged();
}

void PatchManager::setFixturePosition(quint32 id, qreal x, qreal y)
{
    if (m_doc->fixture(id) == nullptr)
        return;

    QSizeF stage = stageSize();
    x = qBound(0.0, x, stage.width());
    y = qBound(0.0, y, stage.height());
    m_doc->monitorProperties()->setFixturePosition(id, 0, 0, QVector3D(float(x), float(y), 0));
    m_doc->setModified();
    notifyFixtures();
}

void PatchManager::autoLayout(bool all)
{
    MonitorProperties *monitor = m_doc->monitorProperties();
    QSizeF stage = stageSize();
    int perRow = qMax(1, int((stage.width() - AUTO_LAYOUT_STEP / 2) / AUTO_LAYOUT_STEP));

    // fixtures are laid out in patch order, after the ones already placed
    QList<Fixture *> fixtures = m_doc->fixtures();
    std::sort(fixtures.begin(), fixtures.end(), [](Fixture *a, Fixture *b) {
        return a->universeAddress() < b->universeAddress();
    });

    // A fixture counts as placed when it has its own spot: shows coming from
    // QLC+ often have every fixture at the origin, or several on the same spot
    QHash<QPair<int, int>, int> spots;
    for (Fixture *fxi : fixtures)
    {
        if (monitor->containsFixture(fxi->id()))
        {
            QVector3D p = monitor->fixturePosition(fxi->id(), 0, 0);
            spots[qMakePair(qRound(p.x()), qRound(p.y()))]++;
        }
    }
    auto isPlaced = [&](Fixture *fxi) {
        if (monitor->containsFixture(fxi->id()) == false)
            return false;
        QVector3D p = monitor->fixturePosition(fxi->id(), 0, 0);
        QPair<int, int> spot = qMakePair(qRound(p.x()), qRound(p.y()));
        return spot != qMakePair(0, 0) && spots.value(spot) == 1;
    };

    QSet<quint32> placed;
    if (all == false)
    {
        for (Fixture *fxi : fixtures)
        {
            if (isPlaced(fxi))
                placed.insert(fxi->id());
        }
    }
    // the others go to the first slots of the grid away from the ones placed
    // (after a fixture is deleted, its slot is free, not the next one)
    QList<QPointF> taken;
    for (Fixture *fxi : fixtures)
    {
        if (placed.contains(fxi->id()))
        {
            QVector3D p = monitor->fixturePosition(fxi->id(), 0, 0);
            taken.append(QPointF(p.x(), p.y()));
        }
    }
    auto isFree = [&taken](const QPointF &spot) {
        for (const QPointF &p : std::as_const(taken))
        {
            if (qAbs(p.x() - spot.x()) < AUTO_LAYOUT_STEP / 2 && qAbs(p.y() - spot.y()) < AUTO_LAYOUT_STEP / 2)
                return false;
        }
        return true;
    };

    int slot = 0;
    bool changed = false;
    for (Fixture *fxi : fixtures)
    {
        if (placed.contains(fxi->id()))
            continue;

        QPointF spot;
        while (true)
        {
            qreal x = AUTO_LAYOUT_STEP / 2 + (slot % perRow) * AUTO_LAYOUT_STEP;
            qreal y = AUTO_LAYOUT_STEP / 2 + (slot / perRow) * AUTO_LAYOUT_STEP;
            slot++;
            spot = QPointF(x, qMin(y, stage.height()));
            // past the end of the stage: its last row, taken or not
            if (y > stage.height() || isFree(spot))
                break;
        }
        monitor->setFixturePosition(fxi->id(), 0, 0, QVector3D(float(spot.x()), float(spot.y()), 0));
        taken.append(spot);
        changed = true;
    }

    if (changed)
    {
        m_doc->setModified();
        notifyFixtures();
    }
}

/*********************************************************************
 * Pan/tilt invert, identify
 *********************************************************************/

ChannelModifier *PatchManager::invertModifier() const
{
    ChannelModifier *mod = m_doc->modifiersCache()->modifier("Invert");
    if (mod == nullptr)
    {
        mod = new ChannelModifier();
        mod->setName("Invert");
        mod->setType(ChannelModifier::SystemTemplate);
        mod->setModifierMap(QList<QPair<uchar, uchar>>() << qMakePair(uchar(0), uchar(255))
                                                         << qMakePair(uchar(255), uchar(0)));
        m_doc->modifiersCache()->addModifier(mod);
    }
    return mod;
}

QPair<int, int> PatchManager::channelRange(Fixture *fxi, int group, int colour) const
{
    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        if (ch == nullptr || ch->group() != group || ch->controlByte() != QLCChannel::MSB ||
            (colour >= 0 && ch->colour() != colour))
            continue;
        ChannelModifier *mod = fxi->channelModifier(c);
        if (mod == nullptr)
            return qMakePair(0, 255);
        return qMakePair(int(mod->getValue(0)), int(mod->getValue(255)));
    }
    return qMakePair(0, 255);
}

void PatchManager::setChannelRange(Fixture *fxi, int group, int from, int to, int colour)
{
    // the coarse channel gets the range, the fine one only follows the direction
    ChannelModifier *coarse = nullptr;
    if (from == 255 && to == 0)
        coarse = invertModifier();
    else if (from != 0 || to != 255)
        coarse = m_doc->modifiersCache()->modifier(QLCModifiersCache::rangeModifierName(from, to));
    ChannelModifier *fine = from > to ? invertModifier() : nullptr;

    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        if (ch == nullptr || ch->group() != group || (colour >= 0 && ch->colour() != colour))
            continue;
        fxi->setChannelModifier(c, ch->controlByte() == QLCChannel::MSB ? coarse : fine);
    }
}

bool PatchManager::isInverted(Fixture *fxi, int group) const
{
    QPair<int, int> range = channelRange(fxi, group);
    return range.first > range.second;
}

void PatchManager::setInverted(const QVariantList &ids, const QString &axis, bool inverted)
{
    setLimits(ids, QVariantMap({ { axis == "tilt" ? "invertTilt" : "invertPan", inverted } }));
}

/*********************************************************************
 * Limits and channel options
 *********************************************************************/

QVariantMap PatchManager::fixtureOptions(quint32 id) const
{
    QVariantMap info;
    Fixture *fxi = m_doc->fixture(id);
    if (fxi == nullptr)
        return info;

    bool hasDimmer = false, hasPan = false, hasTilt = false;
    QVariantList channels;
    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        if (ch == nullptr)
            continue;
        hasDimmer |= ch->group() == QLCChannel::Intensity && ch->colour() == QLCChannel::NoColour;
        hasPan |= ch->group() == QLCChannel::Pan;
        hasTilt |= ch->group() == QLCChannel::Tilt;

        QVariantMap channel;
        channel.insert("index", c);
        channel.insert("name", ch->name());
        channel.insert("group", QLCChannel::groupToString(ch->group()));
        channel.insert("canFade", fxi->channelCanFade(int(c)));
        channel.insert("dimmer", fxi->channelFollowsDimmer(int(c)));
        channels.append(channel);
    }

    auto percent = [](int value) { return qRound(value * 100.0 / 255.0); };
    QPair<int, int> dimmer = channelRange(fxi, QLCChannel::Intensity, QLCChannel::NoColour);
    QPair<int, int> pan = channelRange(fxi, QLCChannel::Pan);
    QPair<int, int> tilt = channelRange(fxi, QLCChannel::Tilt);

    info.insert("id", id);
    info.insert("name", fxi->name());
    info.insert("hasDimmer", hasDimmer);
    info.insert("hasPan", hasPan);
    info.insert("hasTilt", hasTilt);
    info.insert("dimmerMin", percent(qMin(dimmer.first, dimmer.second)));
    info.insert("dimmerMax", percent(qMax(dimmer.first, dimmer.second)));
    info.insert("panMin", percent(qMin(pan.first, pan.second)));
    info.insert("panMax", percent(qMax(pan.first, pan.second)));
    info.insert("tiltMin", percent(qMin(tilt.first, tilt.second)));
    info.insert("tiltMax", percent(qMax(tilt.first, tilt.second)));
    info.insert("invertPan", pan.first > pan.second);
    info.insert("invertTilt", tilt.first > tilt.second);
    info.insert("swapPanTilt", fxi->swapPanTilt());
    info.insert("channels", channels);
    return info;
}

QVariantMap PatchManager::selectionOptions(const QVariantList &ids) const
{
    // a dimmer first in a selection with moving heads: their pan and tilt
    // are shown all the same, with the values of the first head
    QVariantMap info;
    bool dimmer = false, pan = false, tilt = false, both = false;
    for (const QVariant &v : ids)
    {
        const QVariantMap options = fixtureOptions(v.toUInt());
        if (options.isEmpty())
            continue;
        if (info.isEmpty())
            info = options;
        auto take = [&](bool &taken, const char *has, const QStringList &keys) {
            if (taken || options.value(has).toBool() == false)
                return;
            taken = true;
            info.insert(has, true);
            for (const QString &key : keys)
                info.insert(key, options.value(key));
        };
        take(dimmer, "hasDimmer", { "dimmerMin", "dimmerMax" });
        take(pan, "hasPan", { "panMin", "panMax", "invertPan" });
        take(tilt, "hasTilt", { "tiltMin", "tiltMax", "invertTilt" });
        if (both == false && options.value("hasPan").toBool() && options.value("hasTilt").toBool())
        {
            both = true;
            info.insert("swapPanTilt", options.value("swapPanTilt"));
        }
    }
    return info;
}

void PatchManager::setLimits(const QVariantList &ids, const QVariantMap &limits)
{
    auto value = [](const QVariant &percent) { return qBound(0, qRound(percent.toDouble() * 255.0 / 100.0), 255); };

    for (const QVariant &v : ids)
    {
        Fixture *fxi = m_doc->fixture(v.toUInt());
        if (fxi == nullptr)
            continue;
        QVariantMap options = fixtureOptions(fxi->id());

        // what is not given stays exactly as it is
        auto apply = [&](int group, int colour, const QString &key, const QString &invertKey) {
            QPair<int, int> range = channelRange(fxi, group, colour);
            int lo = qMin(range.first, range.second), hi = qMax(range.first, range.second);
            bool inverted = range.first > range.second;
            if (limits.contains(key + "Min"))
                lo = value(limits.value(key + "Min"));
            if (limits.contains(key + "Max"))
                hi = value(limits.value(key + "Max"));
            if (invertKey.isEmpty() == false && limits.contains(invertKey))
                inverted = limits.value(invertKey).toBool();
            if (lo > hi)
                std::swap(lo, hi);
            setChannelRange(fxi, group, inverted ? hi : lo, inverted ? lo : hi, colour);
        };
        if (options.value("hasDimmer").toBool())
            apply(QLCChannel::Intensity, QLCChannel::NoColour, "dimmer", QString());
        if (options.value("hasPan").toBool())
            apply(QLCChannel::Pan, -1, "pan", "invertPan");
        if (options.value("hasTilt").toBool())
            apply(QLCChannel::Tilt, -1, "tilt", "invertTilt");
        if (limits.contains("swapPanTilt"))
            fxi->setSwapPanTilt(limits.value("swapPanTilt").toBool() &&
                                options.value("hasPan").toBool() && options.value("hasTilt").toBool());

        // the universe applies the modifiers and the swap: give it the new ones
        m_doc->updateFixtureChannelCapabilities(fxi->id(), fxi->forcedHTPChannels(), fxi->forcedLTPChannels());
    }
    m_doc->setModified();
    notifyFixtures();
}

void PatchManager::setChannelOptions(quint32 id, int channel, bool canFade, bool followsDimmer)
{
    Fixture *fxi = m_doc->fixture(id);
    if (fxi == nullptr || channel < 0 || quint32(channel) >= fxi->channels())
        return;

    fxi->setChannelCanFade(channel, canFade);
    fxi->setChannelFollowsDimmer(channel, followsDimmer);
    m_doc->updateFixtureChannelCapabilities(fxi->id(), fxi->forcedHTPChannels(), fxi->forcedLTPChannels());
    m_doc->setModified();
    notifyFixtures();
}

QVariantList PatchManager::identified() const
{
    QVariantList list;
    for (quint32 id : m_identified)
        list.append(id);
    return list;
}

bool PatchManager::isIdentified(quint32 id) const
{
    return m_identified.contains(id);
}

void PatchManager::identify(const QVariantList &ids)
{
    QList<quint32> list;
    for (const QVariant &v : ids)
    {
        if (m_doc->fixture(v.toUInt()) != nullptr && list.contains(v.toUInt()) == false)
            list.append(v.toUInt());
    }
    if (list == m_identified)
        return;

    m_identified = list;
    m_identifySource->unsetAll();
    m_blinkOn = true;
    writeIdentify();

    if (m_identified.isEmpty())
        m_blinkTimer->stop();
    else
        m_blinkTimer->start();
    emit identifiedChanged();
    notifyFixtures();
}

void PatchManager::toggleIdentify(quint32 id)
{
    QVariantList ids = identified();
    if (m_identified.contains(id))
        ids.removeAll(id);
    else
        ids.append(id);
    identify(ids);
}

void PatchManager::stopIdentify()
{
    identify(QVariantList());
}

void PatchManager::slotDocCleared()
{
    stopIdentify();
    m_numbers.clear();
    m_shapes.clear();
}

void PatchManager::slotFixtureRemoved()
{
    QVariantList ids;
    for (quint32 id : std::as_const(m_identified))
        if (m_doc->fixture(id) != nullptr)
            ids.append(id);
    identify(ids);

    // the selections forget it too
    bool changed = false;
    for (int i = m_selections.count() - 1; i >= 0; i--)
    {
        Selection &sel = m_selections[i];
        int before = sel.fixtures.count();
        sel.fixtures.erase(std::remove_if(sel.fixtures.begin(), sel.fixtures.end(),
                                          [this](quint32 fid) { return m_doc->fixture(fid) == nullptr; }),
                           sel.fixtures.end());
        changed |= sel.fixtures.count() != before;
        if (sel.fixtures.isEmpty())
            m_selections.removeAt(i);
    }
    if (changed)
        emit selectionsChanged();
}

void PatchManager::slotBlink()
{
    m_blinkOn = !m_blinkOn;
    writeIdentify();
}

static int capabilityValue(const QLCChannel *ch, const QStringList &words)
{
    for (const QString &word : words)
    {
        for (QLCCapability *cap : ch->capabilities())
        {
            if (cap->name().contains(word, Qt::CaseInsensitive))
                return (cap->min() + cap->max()) / 2;
        }
    }
    return -1;
}

void PatchManager::writeIdentify()
{
    // open white light that blinks on the intensity: easy to spot among the others
    uchar level = m_blinkOn ? 255 : 0;

    for (quint32 fid : std::as_const(m_identified))
    {
        Fixture *fxi = m_doc->fixture(fid);
        if (fxi == nullptr)
            continue;

        bool hasDimmer = false;
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch != nullptr && ch->group() == QLCChannel::Intensity &&
                ch->colour() == QLCChannel::NoColour && ch->controlByte() == QLCChannel::MSB)
                hasDimmer = true;
        }

        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch == nullptr || ch->controlByte() != QLCChannel::MSB)
                continue;

            if (ch->group() == QLCChannel::Intensity)
            {
                switch (ch->colour())
                {
                case QLCChannel::NoColour:
                    m_identifySource->set(fid, c, level);
                    break;
                case QLCChannel::Red: case QLCChannel::Green: case QLCChannel::Blue: case QLCChannel::White:
                    m_identifySource->set(fid, c, hasDimmer ? 255 : level);
                    break;
                case QLCChannel::Cyan: case QLCChannel::Magenta: case QLCChannel::Yellow:
                    m_identifySource->set(fid, c, 0);
                    break;
                default:
                    break;
                }
            }
            else if (ch->group() == QLCChannel::Shutter)
            {
                int open = capabilityValue(ch, QStringList() << "open" << "abierto");
                if (open >= 0)
                    m_identifySource->set(fid, c, uchar(open));
            }
        }
    }
}

/*********************************************************************
 * Fixture numbers
 *********************************************************************/

int PatchManager::fixtureNumber(quint32 id) const
{
    return m_numbers.value(id, 0);
}

int PatchManager::fixtureByNumber(int number) const
{
    for (auto it = m_numbers.cbegin(); it != m_numbers.cend(); ++it)
        if (it.value() == number && m_doc->fixture(it.key()) != nullptr)
            return int(it.key());
    return -1;
}

QString PatchManager::setFixtureNumber(quint32 id, int number)
{
    if (m_doc->fixture(id) == nullptr)
        return tr("Fixture not found");
    if (number < 1)
        return tr("The number must be 1 or more");
    int other = fixtureByNumber(number);
    if (other >= 0 && quint32(other) != id)
        return tr("Number %1 is taken by %2").arg(number).arg(m_doc->fixture(quint32(other))->name());
    if (m_numbers.value(id) == number)
        return QString();
    m_numbers.insert(id, number);
    m_doc->setModified();
    notifyFixtures();
    return QString();
}

QString PatchManager::renumberFixtures(const QVariantList &ids, int start)
{
    QList<quint32> fixtures;
    for (const QVariant &v : ids)
        if (m_doc->fixture(v.toUInt()) != nullptr && fixtures.contains(v.toUInt()) == false)
            fixtures.append(v.toUInt());
    if (fixtures.isEmpty())
        return QString();
    if (start < 1)
        return tr("The number must be 1 or more");

    // the numbers must be free, except the ones of these fixtures
    for (int i = 0; i < fixtures.count(); i++)
    {
        int other = fixtureByNumber(start + i);
        if (other >= 0 && fixtures.contains(quint32(other)) == false)
            return tr("Number %1 is taken by %2").arg(start + i).arg(m_doc->fixture(quint32(other))->name());
    }
    for (int i = 0; i < fixtures.count(); i++)
        m_numbers.insert(fixtures.at(i), start + i);
    m_doc->setModified();
    notifyFixtures();
    return QString();
}

QList<quint32> PatchManager::fixturesByNumber() const
{
    QList<quint32> ids;
    for (Fixture *fxi : m_doc->fixtures())
        ids.append(fxi->id());
    std::sort(ids.begin(), ids.end(), [this](quint32 a, quint32 b) {
        return m_numbers.value(a) < m_numbers.value(b);
    });
    return ids;
}

void PatchManager::slotFixtureAdded(quint32 id)
{
    // a new fixture takes the number after the last one
    if (m_numbers.contains(id))
        return;
    int last = 0;
    for (int n : std::as_const(m_numbers))
        last = qMax(last, n);
    m_numbers.insert(id, last + 1);
}

void PatchManager::slotFixtureGone(quint32 id)
{
    m_numbers.remove(id);
    // Doc gives its ID to the next fixture patched: not its shape
    m_shapes.remove(id);
}

/*********************************************************************
 * Selections
 *********************************************************************/

QVariantList PatchManager::selections() const
{
    QVariantList list;
    for (const Selection &sel : m_selections)
    {
        QVariantList fixtures;
        for (quint32 fid : sel.fixtures)
            fixtures.append(fid);
        QVariantMap info;
        info.insert("id", sel.id);
        info.insert("name", sel.name);
        info.insert("groupId", sel.groupId);
        info.insert("fixtures", fixtures);
        list.append(info);
    }
    return list;
}

int PatchManager::addSelection(const QString &name, quint32 groupId, const QVariantList &fixtureIds)
{
    Selection sel;
    sel.id = ++m_lastSelectionId;
    sel.groupId = groupId;
    for (const QVariant &v : fixtureIds)
    {
        if (m_doc->fixture(v.toUInt()) != nullptr && sel.fixtures.contains(v.toUInt()) == false)
            sel.fixtures.append(v.toUInt());
    }
    if (sel.fixtures.isEmpty())
        return -1;
    sel.name = name.trimmed().isEmpty() ? tr("Selection %1").arg(sel.id) : name.trimmed();
    m_selections.append(sel);
    m_doc->setModified();
    emit selectionsChanged();
    return sel.id;
}

void PatchManager::renameSelection(int id, const QString &name)
{
    for (Selection &sel : m_selections)
    {
        if (sel.id == id && name.trimmed().isEmpty() == false)
        {
            sel.name = name.trimmed();
            m_doc->setModified();
            emit selectionsChanged();
        }
    }
}

void PatchManager::removeSelection(int id)
{
    for (int i = 0; i < m_selections.count(); i++)
    {
        if (m_selections.at(i).id == id)
        {
            m_selections.removeAt(i);
            m_doc->setModified();
            emit selectionsChanged();
            return;
        }
    }
}

QVariantList PatchManager::patternSelection(const QVariantList &fixtureIds, const QString &pattern) const
{
    QVariantList out;
    int n = fixtureIds.count();
    if (pattern == "odd" || pattern == "even")
    {
        for (int i = pattern == "odd" ? 0 : 1; i < n; i += 2)
            out.append(fixtureIds.at(i));
    }
    else if (pattern == "reverse")
    {
        for (int i = n - 1; i >= 0; i--)
            out.append(fixtureIds.at(i));
    }
    else if (pattern == "firstHalf")
    {
        out = fixtureIds.mid(0, (n + 1) / 2);
    }
    else if (pattern == "secondHalf")
    {
        out = fixtureIds.mid((n + 1) / 2);
    }
    else if (pattern == "outsideIn" || pattern == "insideOut")
    {
        // both ends first, then towards the middle (or the other way)
        for (int i = 0; i < (n + 1) / 2; i++)
        {
            out.append(fixtureIds.at(i));
            if (n - 1 - i != i)
                out.append(fixtureIds.at(n - 1 - i));
        }
        if (pattern == "insideOut")
            std::reverse(out.begin(), out.end());
    }
    else
    {
        out = fixtureIds;
    }
    return out;
}

void PatchManager::clearSelections()
{
    m_selections.clear();
    m_lastSelectionId = 0;
    emit selectionsChanged();
}

void PatchManager::slotGroupRemoved(quint32 id)
{
    int before = m_selections.count();
    m_selections.erase(std::remove_if(m_selections.begin(), m_selections.end(),
                                      [id](const Selection &sel) { return sel.groupId == id; }),
                       m_selections.end());
    if (m_selections.count() != before)
        emit selectionsChanged();
}

bool PatchManager::loadXML(QXmlStreamReader &root)
{
    if (root.name() == KXMLDTCNumbers)
    {
        // the saved numbers replace the ones given while loading. A number
        // given twice or out of range (a damaged file) is not taken: the
        // fixture gets a free one
        QHash<quint32, int> saved;
        QSet<int> taken;
        while (root.readNextStartElement())
        {
            bool idOk = false, numberOk = false;
            const quint32 id = root.attributes().value("ID").toUInt(&idOk);
            const int number = root.attributes().value("Number").toInt(&numberOk);
            if (root.name() == QLatin1String("Fixture") && idOk && numberOk && m_doc->fixture(id) != nullptr &&
                number > 0 && number <= kMaxFixtureNumber && saved.contains(id) == false && taken.contains(number) == false)
            {
                saved.insert(id, number);
                taken.insert(number);
            }
            root.skipCurrentElement();
        }
        int last = 0;
        for (int n : std::as_const(taken))
            last = qMax(last, n);
        // the others keep theirs when it is free, else take the next ones, in
        // the order they had (each number once)
        for (quint32 id : fixturesByNumber())
        {
            int number = saved.value(id, 0);
            if (number == 0)
            {
                number = m_numbers.value(id, 0);
                if (number <= 0 || taken.contains(number))
                    number = last + 1;
                taken.insert(number);
            }
            last = qMax(last, number);
            m_numbers.insert(id, number);
        }
        notifyFixtures();
        return true;
    }
    if (root.name() == KXMLDTCPlan)
    {
        // how the fixtures look in the plan
        m_shapes.clear();
        while (root.readNextStartElement())
        {
            quint32 id = root.attributes().value("ID").toUInt();
            QString shape = root.attributes().value("Shape").toString();
            if (root.name() == QLatin1String("Fixture") && m_doc->fixture(id) != nullptr && shape.isEmpty() == false)
                m_shapes.insert(id, shape);
            root.skipCurrentElement();
        }
        notifyFixtures();
        return true;
    }
    if (root.name() != KXMLDTCSelections)
        return false;

    m_selections.clear();
    QList<Selection> renumbered;
    while (root.readNextStartElement())
    {
        if (root.name() == QLatin1String("Selection"))
        {
            Selection sel;
            bool idOk = false, groupOk = false;
            sel.id = root.attributes().value("ID").toInt(&idOk);
            sel.name = root.attributes().value("Name").toString();
            sel.groupId = root.attributes().value("Group").toUInt(&groupOk);
            // (what a newer version puts inside is skipped)
            for (const QString &text : root.readElementText(QXmlStreamReader::SkipChildElements).split(",", Qt::SkipEmptyParts))
            {
                bool ok = false;
                const quint32 fid = text.trimmed().toUInt(&ok);
                if (ok && m_doc->fixture(fid) != nullptr && sel.fixtures.contains(fid) == false)
                    sel.fixtures.append(fid);
            }
            // a selection of a group that isn't there goes, as when its
            // group is deleted (a group made later would get its ID)
            if (sel.fixtures.isEmpty() || groupOk == false || m_doc->fixtureGroup(sel.groupId) == nullptr)
                continue;
            if (idOk == false || sel.id <= 0 || sel.id >= INT_MAX / 2 ||
                std::any_of(m_selections.cbegin(), m_selections.cend(), [&sel](const Selection &s) { return s.id == sel.id; }))
            {
                // an ID given twice: a new one, once they are all read
                renumbered.append(sel);
                continue;
            }
            m_lastSelectionId = qMax(m_lastSelectionId, sel.id);
            m_selections.append(sel);
        }
        else
        {
            root.skipCurrentElement();
        }
    }
    for (Selection &sel : renumbered)
    {
        sel.id = ++m_lastSelectionId;
        m_selections.append(sel);
    }
    emit selectionsChanged();
    return true;
}

bool PatchManager::saveXML(QXmlStreamWriter *doc) const
{
    if (m_shapes.isEmpty() == false)
    {
        doc->writeStartElement(KXMLDTCPlan);
        // by fixture: the same show is the same file (a hash has no order)
        QList<quint32> ids = m_shapes.keys();
        std::sort(ids.begin(), ids.end());
        for (quint32 id : std::as_const(ids))
        {
            if (m_doc->fixture(id) == nullptr)
                continue;
            doc->writeStartElement("Fixture");
            doc->writeAttribute("ID", QString::number(id));
            doc->writeAttribute("Shape", m_shapes.value(id));
            doc->writeEndElement();
        }
        doc->writeEndElement();
    }

    if (m_selections.isEmpty() == false)
    {
        doc->writeStartElement(KXMLDTCSelections);
        for (const Selection &sel : m_selections)
        {
            QStringList fixtures;
            for (quint32 fid : sel.fixtures)
                fixtures.append(QString::number(fid));
            doc->writeStartElement("Selection");
            doc->writeAttribute("ID", QString::number(sel.id));
            doc->writeAttribute("Name", sel.name);
            doc->writeAttribute("Group", QString::number(sel.groupId));
            doc->writeCharacters(fixtures.join(","));
            doc->writeEndElement();
        }
        doc->writeEndElement();
    }

    if (m_numbers.isEmpty() == false)
    {
        doc->writeStartElement(KXMLDTCNumbers);
        for (quint32 id : fixturesByNumber())
        {
            doc->writeStartElement("Fixture");
            doc->writeAttribute("ID", QString::number(id));
            doc->writeAttribute("Number", QString::number(m_numbers.value(id)));
            doc->writeEndElement();
        }
        doc->writeEndElement();
    }
    return true;
}
