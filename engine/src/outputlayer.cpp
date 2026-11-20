/*
  DMX Total Control
  outputlayer.cpp

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

#include <QVarLengthArray>
#include <QMutexLocker>
#include <QDebug>
#include <algorithm>
#include <cstring>

#include "channelmodifier.h"
#include "outputlayer.h"
#include "outputpatch.h"
#include "genericfader.h"
#include "qlcioplugin.h"
#include "mastertimer.h"
#include "universe.h"
#include "function.h"

/*****************************************************************************
 * OutputLayer
 *****************************************************************************/

void OutputLayer::addUnit(const Unit &unit)
{
    if (roles.size() < UNIVERSE_SIZE)
        roles.append(QByteArray(UNIVERSE_SIZE - roles.size(), char(Free)));

    // a channel already protected by another unit keeps its role: any role
    // protects it
    auto mark = [this](int address, Role role)
    {
        if (address >= 0 && address < UNIVERSE_SIZE && roles.at(address) == Free)
            roles[address] = char(role);
    };
    for (quint16 address : unit.fire)
        mark(address, Fire);
    for (quint16 address : unit.aux)
        mark(address, Aux);
    mark(unit.safety, Safety);
    units.append(unit);
}

void OutputLayer::addPark(int address, uchar value, bool inBlackout)
{
    if (address < 0 || address >= UNIVERSE_SIZE)
        return;
    if (parked.size() < UNIVERSE_SIZE)
        parked.append(QByteArray(UNIVERSE_SIZE - parked.size(), char(0)));
    parks.append({ quint16(address), value, inBlackout });
    parked[address] = char(inBlackout ? 2 : 1);
}

void OutputLayer::addForced(int address, uchar value, bool hold)
{
    if (address < 0 || address >= UNIVERSE_SIZE)
        return;
    forced.append({ quint16(address), value, hold });
}

int OutputLayer::span() const
{
    int span = 0;
    for (int a = qMin(int(roles.size()), UNIVERSE_SIZE) - 1; a >= 0; a--)
    {
        if (roles.at(a) != Free)
        {
            span = a + 1;
            break;
        }
    }
    for (const Park &park : parks)
        span = qMax(span, int(park.address) + 1);
    for (const Forced &value : forced)
        span = qMax(span, int(value.address) + 1);
    return qMin(span, UNIVERSE_SIZE);
}

QSharedPointer<const OutputLayer> OutputLayer::checked(const QSharedPointer<const OutputLayer> &layer)
{
    if (layer.isNull())
        return layer;

    const OutputLayer &L = *layer;
    bool ok = (L.roles.isEmpty() || L.roles.size() == UNIVERSE_SIZE) &&
              (L.parked.isEmpty() || L.parked.size() == UNIVERSE_SIZE);
    auto protectedHere = [&L](int address)
    {
        return address >= 0 && address < UNIVERSE_SIZE && L.role(address) != Free;
    };
    for (int i = 0; ok && i < L.units.size(); i++)
    {
        const Unit &unit = L.units.at(i);
        for (quint16 address : unit.fire)
            ok = ok && protectedHere(address);
        for (quint16 address : unit.aux)
            ok = ok && protectedHere(address);
        ok = ok && (unit.safety < 0 || protectedHere(unit.safety));
    }
    for (int i = 0; ok && i < L.parks.size(); i++)
        ok = L.parks.at(i).address < UNIVERSE_SIZE && L.parkMode(L.parks.at(i).address) != 0;
    for (int i = 0; ok && i < L.forced.size(); i++)
        ok = L.forced.at(i).address < UNIVERSE_SIZE;
    if (ok)
        return layer;

    // fail closed: every channel of a unit protected, nothing out of the universe
    qWarning() << "[OutputLayer] a layer had inconsistent roles or addresses: fixed";
    OutputLayer *fixed = new OutputLayer();
    fixed->serial = L.serial;
    fixed->armToken = L.armToken;
    fixed->showStop = L.showStop;
    fixed->rampId = L.rampId;
    fixed->rampMs = L.rampMs;
    fixed->roles = L.roles.left(UNIVERSE_SIZE);
    // (a short one marked some: the rest Free, so the gate sees them all)
    if (fixed->roles.isEmpty() == false && fixed->roles.size() < UNIVERSE_SIZE)
        fixed->roles.append(QByteArray(UNIVERSE_SIZE - fixed->roles.size(), char(Free)));
    for (Unit unit : L.units)
    {
        auto inside = [](QVector<quint16> &addresses)
        {
            addresses.erase(std::remove_if(addresses.begin(), addresses.end(),
                                           [](quint16 a) { return a >= UNIVERSE_SIZE; }),
                            addresses.end());
        };
        inside(unit.fire);
        inside(unit.aux);
        if (unit.safety >= UNIVERSE_SIZE)
            unit.safety = -1;
        fixed->addUnit(unit);
    }
    for (const Park &park : L.parks)
        fixed->addPark(park.address, park.value, park.inBlackout);
    for (const Forced &value : L.forced)
        fixed->addForced(value.address, value.value, value.hold);
    return QSharedPointer<const OutputLayer>(fixed);
}

/*****************************************************************************
 * OutputStage
 *****************************************************************************/

OutputStage::OutputStage()
{
    for (int a = 0; a < UNIVERSE_SIZE; a++)
    {
        m_out[a].store(0, std::memory_order_relaxed);
        m_held[a].store(-1, std::memory_order_relaxed);
    }
}

void OutputStage::install(const QSharedPointer<const OutputLayer> &layer, const QByteArray &lastFinal,
                          qint64 nowMs, QVector<Event> *events)
{
    const OutputLayer *L = layer.data();
    const OutputLayer *before = m_installed.data();

    // the units keep their state by fixture, on the same addresses; the new
    // ones start Fresh. One moved (a fixture redirected, another mode) starts
    // Fresh too: what holds it there never fires it (a frame that saw nothing
    // on its old addresses is no release). A fire it had ends; its wait stays,
    // and one that runs from a release (cut at its time, disarmed, or the fire
    // the move ends) still does
    QVector<UnitState> next(L != nullptr ? L->units.size() : 0);
    QVector<bool> taken(m_units.size(), false);
    for (int i = 0; i < next.size(); i++)
    {
        const OutputLayer::Unit &unit = L->units.at(i);
        next[i].fixture = unit.fixture;
        for (int j = 0; j < m_units.size(); j++)
        {
            if (taken.at(j) || m_units.at(j).fixture != unit.fixture)
                continue;
            taken[j] = true;
            const UnitState &old = m_units.at(j);
            const bool same = before != nullptr && j < before->units.size() && before->units.at(j).fire == unit.fire &&
                              before->units.at(j).aux == unit.aux && before->units.at(j).safety == unit.safety;
            if (same)
            {
                next[i] = old;
            }
            else
            {
                if (old.state == Firing && events != nullptr)
                    events->append({ old.fixture, OutputLayer::FireEnded, OutputLayer::Removed,
                                     uint(qMax(qint64(0), nowMs - old.since)) });
                next[i].lastEnd = old.state == Firing ? nowMs : old.lastEnd;
                next[i].waitFromRelease = old.state == Firing || old.waitFromRelease;
            }
            break;
        }
    }
    for (int j = 0; j < m_units.size(); j++)
    {
        const UnitState &old = m_units.at(j);
        if (taken.at(j) == false && old.state == Firing && events != nullptr)
            events->append({ old.fixture, OutputLayer::FireEnded, OutputLayer::Removed,
                             uint(qMax(qint64(0), nowMs - old.since)) });
    }
    m_units = next;

    // what a channel not protected now got in an earlier time it was is
    // forgotten: never a source of a step without its sources (the
    // watchdog). (The channels protected now keep what this frame got)
    for (int a = 0; a < UNIVERSE_SIZE; a++)
    {
        if (L == nullptr || L->roles.size() != UNIVERSE_SIZE || L->roles.at(a) == OutputLayer::Free)
        {
            m_src[a] = 0;
            m_srcWritten[a] = false;
        }
    }

    // the show stop holds what the Hold channels send when it comes
    std::array<bool, UNIVERSE_SIZE> hold {};
    if (L != nullptr && L->showStop)
    {
        for (const OutputLayer::Forced &value : L->forced)
            if (value.hold && value.address < UNIVERSE_SIZE)
                hold[value.address] = true;
    }
    for (int a = 0; a < UNIVERSE_SIZE; a++)
    {
        if (hold[a] == false)
            m_held[a].store(-1, std::memory_order_relaxed);
        else if (m_held[a].load(std::memory_order_relaxed) < 0)
            m_held[a].store(a < lastFinal.size() ? short(uchar(lastFinal.at(a))) : short(0),
                            std::memory_order_relaxed);
    }

    // a new fade to black starts now
    const quint32 rampId = L != nullptr ? L->rampId : 0;
    if (rampId != m_rampId)
    {
        m_rampId = rampId;
        m_rampStart = nowMs;
        m_rampOff = false;
    }

    m_installed = layer;
}

void OutputStage::captureSources(const char *roles, const char *preGM, const char *written)
{
    if (roles == nullptr)
        return;
    for (int a = 0; a < UNIVERSE_SIZE; a++)
    {
        if (roles[a] == OutputLayer::Free)
            continue;
        m_srcWritten[a] = written[a] != 0;
        m_src[a] = m_srcWritten[a] ? uchar(preGM[a]) : 0;
    }
}

void OutputStage::step(qint64 nowMs, bool cutOnly, const QVector<ChannelModifier *> &modifiers,
                       QVector<Event> *events)
{
    const OutputLayer *L = m_installed.data();

    // the fade to black: from full to dark in its time
    int ramp = 1000;
    if (L != nullptr && m_rampId != 0 && m_rampOff == false)
    {
        if (L->rampMs == 0)
            ramp = 0;
        else
            ramp = 1000 - int(qBound(qint64(0), (nowMs - m_rampStart) * 1000 / qint64(L->rampMs), qint64(1000)));
    }
    m_ramp.store(ramp, std::memory_order_relaxed);

    if (L == nullptr || L->roles.size() != UNIVERSE_SIZE)
    {
        m_anyFiring.store(false, std::memory_order_relaxed);
        return;
    }

    // the limits (modifiers): only while it fires (never light at rest)
    auto modified = [&modifiers](int address, uchar value) -> uchar
    {
        ChannelModifier *modifier = address < modifiers.size() ? modifiers.at(address) : nullptr;
        return modifier != nullptr ? modifier->getValue(value) : value;
    };
    // what an allowed source wrote on $address in this frame, 0 when none
    auto passes = [&](int address) -> uchar
    {
        if (address < 0 || address >= UNIVERSE_SIZE || m_srcWritten[address] == false)
            return 0;
        return modified(address, m_src[address]);
    };

    // the values of the gate; an address of two units gets the lower one
    std::array<uchar, UNIVERSE_SIZE> next {};
    std::array<bool, UNIVERSE_SIZE> covered {};
    auto put = [&](int address, uchar value)
    {
        if (address < 0 || address >= UNIVERSE_SIZE)
            return;
        next[address] = covered[address] ? qMin(next[address], value) : value;
        covered[address] = true;
    };

    const qint64 tick = qint64(MasterTimer::tick());
    bool anyFiring = false;
    for (int i = 0; i < L->units.size() && i < m_units.size(); i++)
    {
        const OutputLayer::Unit &unit = L->units.at(i);
        UnitState &st = m_units[i];
        auto event = [&](int kind, int reason, qint64 ms)
        {
            if (events != nullptr)
                events->append({ unit.fixture, kind, reason, uint(qMax(qint64(0), ms)) });
        };

        uchar fire = 0;
        for (quint16 address : unit.fire)
            if (address < UNIVERSE_SIZE)
                fire = qMax(fire, m_src[address]);

        const bool armedFor = L->armToken != 0 && unit.canFire && L->showStop == false;
        const int notNow = unit.canFire == false ? (unit.parked ? OutputLayer::Parked : OutputLayer::NotAllowed)
                         : L->showStop ? OutputLayer::ShowStop
                         : L->armToken == 0 ? OutputLayer::Disarmed
                         : OutputLayer::Waiting;

        switch (st.state)
        {
            case Fresh:
                // new to the gate: a source that already holds it never
                // fires it (arming, protecting it, a show that opens).
                // Decided on a frame with its sources (the watchdog has
                // those of the last frame, where it may not have been
                // protected): 0 until then. (A unit moved keeps a wait that
                // runs from its release: held, until then; else from now)
                if (cutOnly)
                    break;
                if (fire > 0)
                {
                    st.state = Latched;
                    event(OutputLayer::FireRefused, armedFor ? int(OutputLayer::Held) : notNow, 0);
                }
                else
                {
                    st.state = Idle;
                    if (st.waitFromRelease)
                    {
                        st.lastEnd = nowMs;
                        st.waitFromRelease = false;
                    }
                }
            break;
            case Idle:
                if (cutOnly || fire == 0)
                    break;
                if (armedFor && nowMs - st.lastEnd >= qint64(unit.minWaitMs))
                {
                    st.state = Firing;
                    st.since = nowMs;
                    st.token = L->armToken;
                    event(OutputLayer::FireStarted, OutputLayer::Released, 0);
                }
                else
                {
                    // refused: it waits for a release (never a late fire)
                    st.state = Latched;
                    st.waitFromRelease = false;
                    event(OutputLayer::FireRefused, notNow, 0);
                }
            break;
            case Firing:
                if (armedFor == false || L->armToken != st.token)
                {
                    // disarmed (also disarmed and armed again between two frames)
                    st.state = Latched;
                    st.waitFromRelease = true;
                    event(OutputLayer::FireEnded, notNow == OutputLayer::Waiting ? int(OutputLayer::Disarmed) : notNow,
                          nowMs - st.since);
                }
                else if (cutOnly == false && fire == 0)
                {
                    st.state = Idle;
                    st.lastEnd = nowMs;
                    event(OutputLayer::FireEnded, OutputLayer::Released, nowMs - st.since);
                }
                else if ((nowMs - st.since) + tick > qint64(unit.maxFireMs))
                {
                    // the next frame would end past its maximum time
                    st.state = Latched;
                    st.waitFromRelease = true;
                    event(OutputLayer::FireEnded, OutputLayer::TimeLimit, nowMs - st.since);
                }
            break;
            case Latched:
                if (cutOnly || fire > 0)
                    break;
                st.state = Idle;
                if (st.waitFromRelease)
                {
                    // the minimum wait runs from the release
                    st.lastEnd = nowMs;
                    st.waitFromRelease = false;
                }
                event(OutputLayer::Unlatched, OutputLayer::Released, 0);
            break;
        }

        const bool firing = st.state == Firing;
        anyFiring = anyFiring || firing;
        for (quint16 address : unit.fire)
            put(address, firing ? passes(address) : 0);
        for (quint16 address : unit.aux)
            put(address, firing || armedFor ? passes(address) : 0);
        if (unit.safety >= 0 && unit.safety < UNIVERSE_SIZE)
        {
            // in its window only while it fires ("from" when no source sets it);
            // a window upside down never opens
            uchar safety = 0;
            if (firing && unit.safetyFrom <= unit.safetyTo)
                safety = m_srcWritten[unit.safety]
                             ? qBound(unit.safetyFrom, modified(unit.safety, m_src[unit.safety]), unit.safetyTo)
                             : unit.safetyFrom;
            put(unit.safety, safety);
        }
    }

    // every protected channel: what its units allow, 0 when none
    for (int a = 0; a < UNIVERSE_SIZE; a++)
        if (L->roles.at(a) != OutputLayer::Free)
            m_out[a].store(covered[a] ? next[a] : 0, std::memory_order_relaxed);
    m_anyFiring.store(anyFiring, std::memory_order_relaxed);
}

uchar OutputStage::out(int address) const
{
    if (address < 0 || address >= UNIVERSE_SIZE)
        return 0;
    return m_out[address].load(std::memory_order_relaxed);
}

int OutputStage::held(int address) const
{
    if (address < 0 || address >= UNIVERSE_SIZE)
        return -1;
    return m_held[address].load(std::memory_order_relaxed);
}

void OutputStage::reset()
{
    for (UnitState &st : m_units)
    {
        const quint32 fixture = st.fixture;
        st = UnitState();
        st.fixture = fixture;
    }
    for (int a = 0; a < UNIVERSE_SIZE; a++)
    {
        m_out[a].store(0, std::memory_order_relaxed);
        m_held[a].store(-1, std::memory_order_relaxed);
        m_src[a] = 0;
        m_srcWritten[a] = false;
    }
    m_rampOff = true;
    m_ramp.store(1000, std::memory_order_relaxed);
    m_anyFiring.store(false, std::memory_order_relaxed);
}

/*****************************************************************************
 * Universe: the output stage (DMX Total Control)
 *****************************************************************************/

static void emitEffectEvents(Universe *universe, const QVector<OutputStage::Event> &events)
{
    for (const OutputStage::Event &e : events)
        emit universe->effectEvent(universe->id(), e.fixture, e.kind, e.reason, e.durationMs);
}

// $frame is sent unless the outputs already have it: $last keeps it
static bool frameChanged(QByteArray &last, const QByteArray &frame)
{
    const int size = qMin(int(frame.size()), int(last.size()));
    if (size <= 0 || memcmp(last.constData(), frame.constData(), size_t(size)) == 0)
        return false;
    memcpy(last.data(), frame.constData(), size_t(size));
    return true;
}

void Universe::setOutputLayer(const QSharedPointer<const OutputLayer> &layer)
{
    const QSharedPointer<const OutputLayer> checked = OutputLayer::checked(layer);
    QMutexLocker locker(&m_outputLayerMutex);
    m_outputLayer = checked;
}

QSharedPointer<const OutputLayer> Universe::outputLayer() const
{
    QMutexLocker locker(&m_outputLayerMutex);
    return m_outputLayer;
}

void Universe::takeFrameLayer()
{
    {
        QMutexLocker locker(&m_outputLayerMutex);
        m_frameLayer = m_outputLayer;
    }
    // (checked(): the roles of a layer are empty or UNIVERSE_SIZE bytes)
    m_frameRoles = m_frameLayer.isNull() == false && m_frameLayer->roles.size() == UNIVERSE_SIZE
                       ? m_frameLayer->roles.constData() : nullptr;
}

QByteArray Universe::finalValues() const
{
    {
        QMutexLocker locker(&m_finalMutex);
        if (m_finalFrame.isEmpty() == false)
            return m_finalFrame;
    }
    // no frame yet: as it would go out now
    QByteArray preSwap;
    const QSharedPointer<const OutputLayer> layer = outputLayer();
    composeFrame(false, layer.data(), &preSwap);
    return QByteArray(preSwap.constData(), preSwap.size());
}

uchar Universe::finalValue(int address) const
{
    const QByteArray values = finalValues();
    return address >= 0 && address < values.size() ? uchar(values.at(address)) : 0;
}

QByteArray Universe::swapped(const QByteArray &values, const OutputLayer *layer) const
{
    if (m_outputSourceCount <= 0)
        return values;

    // swapped channels (pan/tilt) send each other's value; a protected
    // channel never sends the value of one that is not (fixtures that
    // overlap a swapped one)
    const bool gate = layer != nullptr && layer->roles.size() == UNIVERSE_SIZE;
    QByteArray out(values);
    for (int ch = 0; ch < m_usedChannels && ch < out.size(); ch++)
    {
        const int source = m_outputSource.at(ch);
        if (source < 0 || source >= m_usedChannels || source >= values.size())
            continue;
        if (gate && layer->role(ch) != OutputLayer::Free && layer->role(source) == OutputLayer::Free)
            out[ch] = 0;
        else
            out[ch] = values.at(source);
    }
    return out;
}

QByteArray Universe::composeFrame(bool blackout, const OutputLayer *layer, QByteArray *preSwap) const
{
    const bool overlay = layer != nullptr && layer->isEmpty() == false;
    if (blackout == false && overlay == false && m_outputSourceCount == 0)
    {
        const QByteArray values = QByteArray::fromRawData(m_postGMValues->constData(), m_usedChannels);
        if (preSwap != nullptr)
            *preSwap = values;
        return values;
    }

    QByteArray values;
    if (blackout)
    {
        // without light, the other channels stay: through their modifiers
        // (limits, invert) as when the light is on
        values = QByteArray(m_blackoutValues->constData(), m_blackoutValues->size());
        for (int ch = 0; ch < m_usedChannels && ch < values.size(); ch++)
        {
            ChannelModifier *modifier = m_modifiers.at(ch);
            if (modifier == NULL || (m_channelsMask->at(ch) & HTP))
                continue;
            // a coarse channel with a range and its fine one: on the 16-bit value
            const int fine = m_rangeFine.at(ch);
            if (fine >= 0 && fine < values.size())
            {
                const int out = qMax(0, rangeOf16(modifier, (int(uchar(m_blackoutValues->at(ch))) << 8) |
                                                            int(uchar(m_blackoutValues->at(fine)))));
                values[ch] = char(out >> 8);
                values[fine] = char(out & 0xFF);
            }
            else if (m_rangeCoarse.at(ch) < 0)
            {
                values[ch] = char(modifier->getValue(uchar(m_blackoutValues->at(ch))));
            }
        }
    }
    else
    {
        values = QByteArray(m_postGMValues->constData(), m_usedChannels);
    }

    if (overlay)
    {
        // every channel the layer sets goes out, used or not
        const int span = layer->span();
        if (values.size() < span)
            values.append(QByteArray(span - values.size(), char(0)));
        char *d = values.data();
        const int size = values.size();

        // 1. parks: the byte that goes out, above the grand master, the
        // mixer and the programmer. In blackout an intensity goes dark
        // unless it is parked also in blackout
        for (const OutputLayer::Park &park : layer->parks)
        {
            if (park.address >= size)
                continue;
            if (blackout && park.inBlackout == false && (m_channelsMask->at(park.address) & HTP))
                continue;
            d[park.address] = char(park.value);
        }

        // 2. the fade to black, on the intensities (not on the special
        // effects, nor on the parks that stay in blackout)
        if (blackout == false)
        {
            const int ramp = m_stage->rampPermille();
            if (ramp < 1000)
            {
                for (int a = 0; a < size; a++)
                {
                    if ((m_channelsMask->at(a) & HTP) == 0 || layer->role(a) != OutputLayer::Free ||
                        layer->parkMode(a) == 2)
                        continue;
                    d[a] = char((int(uchar(d[a])) * ramp + 500) / 1000);
                }
            }
        }

        // 3. the show stop, above everything but the gate (blackout too)
        if (layer->showStop)
        {
            for (const OutputLayer::Forced &value : layer->forced)
            {
                if (value.address >= size || layer->role(value.address) != OutputLayer::Free)
                    continue;
                if (value.hold)
                {
                    const int held = m_stage->held(value.address);
                    if (held >= 0)
                        d[value.address] = char(held);
                }
                else
                {
                    d[value.address] = char(value.value);
                }
            }
        }

        // 4. the effect gate: the last word on every protected channel. In
        // blackout they are all 0
        if (layer->roles.size() == UNIVERSE_SIZE)
        {
            for (int a = 0; a < size; a++)
                if (layer->roles.at(a) != OutputLayer::Free)
                    d[a] = blackout ? char(0) : char(m_stage->out(a));
        }
    }

    if (preSwap != nullptr)
        *preSwap = values;
    return swapped(values, overlay ? layer : nullptr);
}

void Universe::sendComposedLocked(const OutputLayer *layer, bool forced, QByteArray *written)
{
    QByteArray preSwap;
    const QByteArray normal = composeFrame(false, layer, &preSwap);
    {
        // (a copy, not the raw postGM: the readers get what went out)
        QMutexLocker locker(&m_finalMutex);
        if (m_finalFrame.size() != preSwap.size())
            m_finalFrame.resize(preSwap.size());
        if (preSwap.isEmpty() == false)
            memcpy(m_finalFrame.data(), preSwap.constData(), size_t(preSwap.size()));
    }

    // each output its frame: the outputs in blackout get the blackout one.
    // A frame goes out as changed when its bytes changed (Open DMX and
    // Art-Net copy only then), or when it is asked for
    const int count = m_outputPatchList.count();
    QVarLengthArray<bool, 8> dark(count);
    bool anyDark = false;
    for (int i = 0; i < count; i++)
    {
        dark[i] = m_outputPatchList.at(i)->blackout();
        anyDark = anyDark || dark[i];
    }
    const QByteArray black = anyDark ? composeFrame(true, layer) : QByteArray();
    const bool changedNormal = frameChanged(m_lastSent[0], normal) || forced;
    const bool changedBlack = anyDark && (frameChanged(m_lastSent[1], black) || forced);

    for (int i = 0; i < count; i++)
    {
        OutputPatch *op = m_outputPatchList.at(i);
        if (m_totalChannelsChanged == true)
            op->setPluginParameter(PLUGIN_UNIVERSECHANNELS, m_totalChannels);
        op->dump(m_id, dark[i] ? black : normal, dark[i] ? changedBlack : changedNormal);
    }
    if (count > 0)
        m_totalChannelsChanged = false;

    if (changedNormal && written != nullptr)
        *written = QByteArray(normal.constData(), normal.size());
    m_sentSerial = layer != nullptr ? layer->serial : 0;
}

void Universe::sendFrame(uint elapsedMs)
{
    QVector<OutputStage::Event> events;
    QByteArray written;
    {
        QMutexLocker locker(&m_outputMutex);
        // the clock of the gate: real time in the thread, the frames' ms offline
        m_gateNowMs += m_liveFrame ? qMax(qint64(0), m_gateTimer.restart()) : qint64(elapsedMs);
        if (m_stage->installed() != m_frameLayer)
            m_stage->install(m_frameLayer, m_finalFrame, m_gateNowMs, &events);
        m_stage->step(m_gateNowMs, false, m_modifiers, &events);
        sendComposedLocked(m_frameLayer.data(), m_forceDump.exchange(false), &written);
    }
    // (out of the lock: a direct connection may read the universe)
    emitEffectEvents(this, events);
    if (written.isNull() == false)
        emit universeWritten(id(), written);
}

void Universe::fadeOutFunctionFaders(uint ms, const QSet<quint32> &keep)
{
    // (as setFaderFadeOut, the fade out of every function, but $keep)
    QMutexLocker fadersLocker(&m_fadersMutex);
    for (const QSharedPointer<GenericFader> &fader : std::as_const(m_faders))
    {
        // (one about to go at once stays so)
        if (fader.isNull() || (fader->deleteRequested() && fader->isFadingOut() == false))
            continue;
        const quint32 parent = fader->parentFunctionID();
        if (parent == Function::invalidId() || keep.contains(parent))
            continue;
        fader->setFadeOut(true, ms);
    }
}

int Universe::cutFadeOuts(const QSet<quint32> &keep)
{
    int cut = 0;
    QMutexLocker fadersLocker(&m_fadersMutex);
    for (const QSharedPointer<GenericFader> &fader : std::as_const(m_faders))
    {
        if (fader.isNull() || fader->isFadingOut() == false)
            continue;
        const quint32 parent = fader->parentFunctionID();
        if (parent == Function::invalidId() || keep.contains(parent))
            continue;
        // nothing left to write: the next frame drops it (its channels at
        // rest, as a function stopped without a fade)
        fader->setFadeOut(false, 0);
        fader->removeAll();
        fader->requestDelete();
        cut++;
    }
    return cut;
}

void Universe::requestFrame()
{
    m_forceDump = true;
    if (isRunning())
    {
        // its thread sends it (whole: m_forceDump)
        tick();
        return;
    }

    // offline: the latest layer, nothing fires (no sources run here)
    QVector<OutputStage::Event> events;
    QByteArray written;
    const QSharedPointer<const OutputLayer> layer = outputLayer();
    {
        QMutexLocker locker(&m_outputMutex);
        if (m_stage->installed() != layer)
            m_stage->install(layer, m_finalFrame, m_gateNowMs, &events);
        m_stage->step(m_gateNowMs, true, m_modifiers, &events);
        m_forceDump = false;
        sendComposedLocked(layer.data(), true, &written);
    }
    emitEffectEvents(this, events);
    if (written.isNull() == false)
        emit universeWritten(id(), written);
}

void Universe::watchdogFrame(uint elapsedMs)
{
    watchdog(false, elapsedMs);
}

void Universe::lockFrames()
{
    // (processFaders holds it for a whole frame; the watchdog never reads the
    // faders, it goes on)
    m_frameMutex.lock();
}

void Universe::unlockFrames()
{
    m_frameMutex.unlock();
}

void Universe::watchdog(bool live, uint elapsedMs)
{
    QVector<OutputStage::Event> events;
    QByteArray written;
    // the latest layer (a disarm cuts too), with the sources of the last
    // frame: units only stop firing (the layer of the frames is not
    // touched: a frame may be under way in another thread, offline)
    const QSharedPointer<const OutputLayer> latest = outputLayer();
    {
        QMutexLocker locker(&m_outputMutex);
        m_gateNowMs += live ? qMax(qint64(0), m_gateTimer.restart()) : qint64(elapsedMs);
        if (m_stage->installed() != latest)
            m_stage->install(latest, m_finalFrame, m_gateNowMs, &events);
        m_stage->step(m_gateNowMs, true, m_modifiers, &events);

        // the last frame, where a protected channel can only go down. It
        // never reads postGM: nothing else changes
        const OutputLayer *layer = latest.data();
        if (layer != nullptr && layer->roles.size() == UNIVERSE_SIZE && m_finalFrame.isEmpty() == false)
        {
            QByteArray frame = m_finalFrame;
            bool lowered = false;
            for (int a = 0; a < frame.size() && a < UNIVERSE_SIZE; a++)
            {
                if (layer->roles.at(a) == OutputLayer::Free)
                    continue;
                const uchar gate = m_stage->out(a);
                if (uchar(frame.at(a)) > gate)
                {
                    frame[a] = char(gate);
                    lowered = true;
                }
            }
            if (lowered)
            {
                {
                    QMutexLocker finalLocker(&m_finalMutex);
                    m_finalFrame = frame;
                }
                // (the blackout frame has every protected channel at 0 already)
                const QByteArray normal = swapped(frame, layer);
                const bool changed = frameChanged(m_lastSent[0], normal);
                for (OutputPatch *op : std::as_const(m_outputPatchList))
                    if (op->blackout() == false)
                        op->dump(m_id, normal, changed);
                if (changed)
                    written = QByteArray(normal.constData(), normal.size());
            }
        }
        m_sentSerial = layer != nullptr ? layer->serial : 0;
    }
    emitEffectEvents(this, events);
    if (written.isNull() == false)
        emit universeWritten(id(), written);
}
