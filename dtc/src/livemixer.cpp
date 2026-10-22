/*
  DMX Total Control
  livemixer.cpp

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

#include <QSet>

#include "livemixer.h"

#include "inputoutputmap.h"
#include "qlcfixturehead.h"
#include "fixturegroup.h"
#include "qlcchannel.h"
#include "universe.h"
#include "fixture.h"
#include "doc.h"

LiveMixer::LiveMixer(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
{
    connect(m_doc, SIGNAL(fixtureAdded(quint32)), this, SLOT(slotFixturesChanged()));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotFixturesChanged()));
    connect(m_doc, SIGNAL(fixtureChanged(quint32)), this, SLOT(slotFixturesChanged()));
    connect(m_doc, SIGNAL(fixtureGroupAdded(quint32)), this, SLOT(slotFixturesChanged()));
    connect(m_doc, SIGNAL(fixtureGroupRemoved(quint32)), this, SLOT(slotFixturesChanged()));
    connect(m_doc, SIGNAL(fixtureGroupChanged(quint32)), this, SLOT(slotFixturesChanged()));
    connect(m_doc, SIGNAL(loaded()), this, SLOT(slotFixturesChanged()));
    connect(m_doc, SIGNAL(cleared()), this, SLOT(slotDocCleared()));
}

LiveMixer::~LiveMixer()
{
}

/*********************************************************************
 * Mixers
 *********************************************************************/

static bool isEffectGroup(FixtureGroup *group)
{
    // the groups made for colour effects are not mixers
    return group->name().endsWith(" (FX)") || group->name().endsWith(" (FX 2D)");
}

QVariantList LiveMixer::mixers() const
{
    QVariantList list;
    auto describe = [this](int id, const QString &name, int fixtures)
    {
        State s = m_states.value(id);
        QVariantMap map;
        map.insert("id", id);
        map.insert("name", name);
        map.insert("fixtures", fixtures);
        map.insert("dimmer", s.dimmer);
        map.insert("colour", s.colour);
        map.insert("hue", s.hue);
        map.insert("saturation", s.saturation);
        map.insert("strobe", s.strobe);
        map.insert("strobeHz", s.strobeHz);
        map.insert("full", s.full);
        map.insert("blackout", s.blackout);
        map.insert("flash", s.flash);
        map.insert("fullExclusive", s.fullExclusive);
        map.insert("flashExclusive", s.flashExclusive);
        return map;
    };

    list.append(describe(-1, tr("All fixtures"), int(m_doc->fixtures().count())));
    for (FixtureGroup *group : m_doc->fixtureGroups())
    {
        if (isEffectGroup(group) == false)
            list.append(describe(int(group->id()), group->name(), int(group->fixtureList().count())));
    }
    return list;
}

QString LiveMixer::mixerName(int mixerId) const
{
    if (mixerId == -1)
        return tr("All fixtures");
    FixtureGroup *group = m_doc->fixtureGroup(quint32(mixerId));
    return group != nullptr && isEffectGroup(group) == false ? group->name() : QString();
}

bool LiveMixer::isNeutral(const State &s) const
{
    return qFuzzyCompare(s.dimmer, 1.0) && s.colour == false && s.strobe == false &&
           s.full == false && s.blackout == false && s.flash == false &&
           s.fullExclusive == false && s.flashExclusive == false;
}

bool LiveMixer::isActive() const
{
    for (const State &s : m_states)
        if (isNeutral(s) == false)
            return true;
    return false;
}

void LiveMixer::set(int mixerId, const QString &key, const QVariant &value)
{
    State &s = m_states[mixerId];
    if (key == "dimmer")
        s.dimmer = qBound(0.0, value.toDouble(), 1.0);
    else if (key == "hue")
    {
        s.hue = qBound(0.0, value.toDouble(), 1.0);
        s.colour = true;
    }
    else if (key == "saturation")
    {
        s.saturation = qBound(0.0, value.toDouble(), 1.0);
        s.colour = true;
    }
    else if (key == "strobe")
        s.strobe = value.toBool();
    else if (key == "strobeHz")
        s.strobeHz = qBound(0.5, value.toDouble(), 20.0);
    else if (key == "full")
    {
        s.full = value.toBool();
        if (s.full)
            s.blackout = false;
    }
    else if (key == "blackout")
    {
        s.blackout = value.toBool();
        if (s.blackout)
            s.full = false;
    }
    else if (key == "flash")
        s.flash = value.toBool();
    else if (key == "fullExclusive")
        s.fullExclusive = value.toBool();
    else if (key == "flashExclusive")
        s.flashExclusive = value.toBool();
    else
        return;

    apply();
    emit mixersChanged();
}

void LiveMixer::resetColour(int mixerId)
{
    if (m_states.contains(mixerId) == false)
        return;
    m_states[mixerId].colour = false;
    apply();
    emit mixersChanged();
}

void LiveMixer::reset()
{
    m_states.clear();
    apply();
    emit mixersChanged();
}

void LiveMixer::slotFixturesChanged()
{
    // a show cleared or loading: once, at the end (cleared(), loaded()).
    // Each fixture and group made the mixers again, and their strips
    if (m_doc->isClearingOrLoading())
        return;
    // groups may be gone
    QList<int> ids = m_states.keys();
    for (int id : ids)
        if (id >= 0 && m_doc->fixtureGroup(quint32(id)) == nullptr)
            m_states.remove(id);
    apply();
    emit mixersChanged();
}

void LiveMixer::slotDocCleared()
{
    m_states.clear();
    apply();
    emit mixersChanged();
}

QList<quint32> LiveMixer::fixturesOf(int mixerId) const
{
    QList<quint32> list;
    if (mixerId < 0)
    {
        for (Fixture *fxi : m_doc->fixtures())
            list.append(fxi->id());
    }
    else if (FixtureGroup *group = m_doc->fixtureGroup(quint32(mixerId)))
    {
        list = group->fixtureList();
    }
    return list;
}

/*********************************************************************
 * Output
 *********************************************************************/

void LiveMixer::apply()
{
    // the exclusive buttons darken everything outside of their mixer
    QSet<quint32> exclusiveFixtures;
    bool exclusive = false;
    for (auto it = m_states.cbegin(); it != m_states.cend(); ++it)
    {
        if (it.value().fullExclusive || it.value().flashExclusive)
        {
            exclusive = true;
            for (quint32 fid : fixturesOf(it.key()))
                exclusiveFixtures.insert(fid);
        }
    }

    const State all = m_states.value(-1);
    QMap<quint32, QList<State>> groupStates;
    for (auto it = m_states.cbegin(); it != m_states.cend(); ++it)
    {
        if (it.key() < 0)
            continue;
        for (quint32 fid : fixturesOf(it.key()))
            groupStates[fid].append(it.value());
    }

    QMap<quint32, MixerLayer> layers;
    for (Fixture *fxi : m_doc->fixtures())
    {
        // what the mixers of this fixture say, together
        QList<State> states;
        states.append(all);
        states.append(groupStates.value(fxi->id()));

        MixerUnit unit;
        for (const State &s : states)
        {
            unit.dimmer *= s.dimmer;
            unit.full = unit.full || s.full || s.fullExclusive;
            unit.blackout = unit.blackout || s.blackout;
            unit.flash = unit.flash || s.flash || s.flashExclusive;
            if (s.strobe)
                unit.strobeHz = qMax(unit.strobeHz, s.strobeHz);
        }
        // the colour of a group wins over the colour of all the fixtures
        for (int i = int(states.count()) - 1; i >= 0; i--)
        {
            if (states.at(i).colour)
            {
                unit.colour = true;
                unit.hue = states.at(i).hue;
                unit.saturation = states.at(i).saturation;
                break;
            }
        }
        if (exclusive && exclusiveFixtures.contains(fxi->id()) == false)
        {
            unit.blackout = true;
            unit.full = false;
            unit.flash = false;
        }

        if (qFuzzyCompare(unit.dimmer, 1.0) && !unit.colour && !unit.full && !unit.blackout &&
            !unit.flash && unit.strobeHz <= 0.0)
            continue;

        // its dimmers, and the colour channels of each head
        const int base = int(fxi->address());
        QMap<int, MixerUnit::Head> heads;
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch == nullptr || ch->group() != QLCChannel::Intensity || ch->controlByte() != QLCChannel::MSB)
                continue;
            if (ch->colour() == QLCChannel::NoColour)
            {
                unit.dimmers.append(base + int(c));
                continue;
            }
            int head = 0;
            for (int h = 0; h < fxi->heads(); h++)
                if (fxi->head(h).channels().contains(c))
                    head = h;
            MixerUnit::Head &hd = heads[head];
            int address = base + int(c);
            switch (ch->colour())
            {
                case QLCChannel::Red: hd.r = address; break;
                case QLCChannel::Green: hd.g = address; break;
                case QLCChannel::Blue: hd.b = address; break;
                case QLCChannel::White: hd.w = address; break;
                case QLCChannel::Cyan: hd.c = address; break;
                case QLCChannel::Magenta: hd.m = address; break;
                case QLCChannel::Yellow: hd.y = address; break;
                default: break;
            }
        }
        unit.heads = heads.values().toVector();
        layers[fxi->universe()].append(unit);
    }

    QList<Universe *> universes = m_doc->inputOutputMap()->claimUniverses();
    for (Universe *universe : universes)
    {
        if (layers.contains(universe->id()))
            universe->setMixerLayer(QSharedPointer<const MixerLayer>(new MixerLayer(layers.value(universe->id()))));
        else
            universe->setMixerLayer(QSharedPointer<const MixerLayer>());
    }
    m_doc->inputOutputMap()->releaseUniverses(false);
}
