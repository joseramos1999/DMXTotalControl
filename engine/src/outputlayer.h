/*
  DMX Total Control
  outputlayer.h

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

#ifndef OUTPUTLAYER_H
#define OUTPUTLAYER_H

#include <QSharedPointer>
#include <QByteArray>
#include <QVector>
#include <atomic>
#include <array>
#include <limits>

#include "universe.h"

class ChannelModifier;

/** @addtogroup engine Engine
 * @{
 */

/**
 * What DMX Total Control puts at the output of one universe, above every
 * source: the protected special effects (the effect gate), the parked
 * channels, the show stop and the fade to black. Built in the GUI thread and
 * never changed once given to the universe (Universe::setOutputLayer): a new
 * state is a new layer.
 *
 * The order of a frame (Universe::composeFrame): the values of the sources
 * after the grand master, the limits and the passthrough, then the parks,
 * the fade to black, the show stop and, last, the effect gate. The blackout
 * frame starts from the blackout values instead (no light, the other
 * channels where they are) and gets the parks that stay in blackout, the
 * show stop and the gate (0 on every protected channel).
 */
struct OutputLayer
{
    /** What a channel of a protected fixture is */
    enum Role : char
    {
        Free = 0,       /** not protected */
        Fire = 1,       /** above 0 the fixture fires: its time runs */
        Aux = 2,        /** another channel of it: out only while the effects are armed */
        Safety = 3      /** the channel the machine needs in a window of values to fire */
    };
    /** What happened to a unit, told by Universe::effectEvent */
    enum EventKind
    {
        FireStarted = 0,    /** it fires */
        FireEnded = 1,      /** it stopped firing ($durationMs: how long it fired) */
        FireRefused = 2,    /** its source asked for it, it doesn't fire: it waits for a release */
        Unlatched = 3       /** released after a refusal or a cut: it may fire again */
    };
    enum EventReason
    {
        Released = 0,   /** its source went back to 0 */
        TimeLimit,      /** cut at its maximum time */
        Disarmed,       /** the effects are not armed (or were disarmed while it fired) */
        ShowStop,       /** the show stop is engaged */
        NotAllowed,     /** it can never fire (across two universes, parked off) */
        Waiting,        /** its minimum wait between two fires has not passed */
        Removed,        /** it is not protected any more, or gone, while it fired */
        Held,           /** its source already held it when it came (a new layer, armed) */
        Parked          /** it is parked off: it never fires while parked */
    };

    /** One protected fixture's channels in THIS universe (addresses 0-511) */
    struct Unit
    {
        quint32 fixture = 0;
        QVector<quint16> fire;          // any of them above 0: the fixture fires (16 bit: either byte)
        QVector<quint16> aux;           // its other channels: they pass while armed
        int safety = -1;                // the address of its safety channel, -1: none
        uchar safetyFrom = 1;
        uchar safetyTo = 255;
        quint32 maxFireMs = 3000;       // 100-30000
        quint32 minWaitMs = 1000;       // 0-60000
        bool canFire = true;            // false: parked off, or across two universes
        bool parked = false;            // parked off (canFire is false too)
    };
    /** A parked channel: $value is the byte that goes out */
    struct Park
    {
        quint16 address;
        uchar value;
        bool inBlackout;
    };
    /** A value of the show stop. $hold: the value it sent when the show stop came */
    struct Forced
    {
        quint16 address;
        uchar value;
        bool hold;
    };

    quint32 serial = 0;             // given by its maker: Universe::sentLayerSerial() tells when it is out
    quint32 armToken = 0;           // 0: disarmed; a new one each time the effects are armed
    QByteArray roles;               // UNIVERSE_SIZE bytes of Role, or empty: nothing protected
    QVector<Unit> units;
    QVector<Park> parks;
    QByteArray parked;              // UNIVERSE_SIZE: 0, 1 parked, 2 parked also in blackout (or empty)
    bool showStop = false;
    QVector<Forced> forced;
    quint32 rampId = 0;             // fade to black: 0 none; the same id is the same ramp
    quint32 rampMs = 0;

    bool hasUnits() const { return units.isEmpty() == false; }
    bool isEmpty() const
    {
        return units.isEmpty() && roles.isEmpty() && parks.isEmpty() && showStop == false && rampId == 0;
    }

    /** The role of $address (Free outside the universe) */
    Role role(int address) const
    {
        return address >= 0 && address < roles.size() ? Role(roles.at(address)) : Free;
    }
    /** 0, 1 parked, 2 parked also in blackout */
    int parkMode(int address) const
    {
        return address >= 0 && address < parked.size() ? int(parked.at(address)) : 0;
    }

    /** Adds $unit and marks the roles of its channels (a protected role is
     *  never taken back by another unit on the same address) */
    void addUnit(const Unit &unit);
    /** Adds a park and marks it */
    void addPark(int address, uchar value, bool inBlackout);
    /** Adds a value of the show stop */
    void addForced(int address, uchar value, bool hold);

    /** One past the highest address the layer sets (0: none) */
    int span() const;

    /** $layer as a universe can use it: the roles sized and marking every
     *  channel of every unit, the parks marked, no address out of the
     *  universe. The same pointer when it already is (fail closed: a
     *  channel of a unit is always protected) */
    static QSharedPointer<const OutputLayer> checked(const QSharedPointer<const OutputLayer> &layer);
};

/**
 * The state of the output stage of one universe: the state machine of each
 * unit (its fires, cuts and waits), what the gate sends on each protected
 * channel, the values the show stop holds and the fade to black.
 *
 * Only the thread of its universe changes it, under Universe's output mutex
 * (offline: the caller of Universe::renderFrame). out(), held() and
 * rampPermille() are atomics, read from any thread (Universe::outputValues).
 */
class OutputStage
{
public:
    struct Event
    {
        quint32 fixture;
        int kind;           // OutputLayer::EventKind
        int reason;         // OutputLayer::EventReason
        uint durationMs;
    };

    OutputStage();

    /** The layer of the frames from now on. The units keep their state by
     *  fixture on the same addresses (a new layer while one fires: it goes
     *  on firing, its time too); new units start Fresh: one whose source
     *  already holds it never fires until it is released. One moved starts
     *  Fresh too, with its minimum wait (one that runs from a release still
     *  does). A unit that is gone or moved while it fires ends with Removed.
     *  The show stop takes what $lastFinal sends on its Hold channels; a new
     *  ramp starts at $nowMs */
    void install(const QSharedPointer<const OutputLayer> &layer, const QByteArray &lastFinal,
                 qint64 nowMs, QVector<Event> *events);
    const QSharedPointer<const OutputLayer> &installed() const { return m_installed; }

    /** The sources of this frame on the protected channels ($roles: of the
     *  layer of the frame, UNIVERSE_SIZE bytes): what the faders allowed to
     *  fire wrote in it, 0 for the rest */
    void captureSources(const char *roles, const char *preGM, const char *written);

    /** One step of every unit at $nowMs (ms of the gate clock) and the
     *  values of the gate. $cutOnly (a frame without sources: the watchdog,
     *  a frame requested now): units only stop firing, nothing starts */
    void step(qint64 nowMs, bool cutOnly, const QVector<ChannelModifier *> &modifiers, QVector<Event> *events);

    bool anyFiring() const { return m_anyFiring.load(std::memory_order_relaxed); }
    /** What the gate sends on the protected $address now */
    uchar out(int address) const;
    /** What the show stop holds on $address, -1: nothing */
    int held(int address) const;
    /** The fade to black, 1000: full, 0: dark */
    int rampPermille() const { return m_ramp.load(std::memory_order_relaxed); }

    /** Every unit Fresh, nothing held, the ramp off (the universe was reset) */
    void reset();

private:
    enum State : quint8 { Fresh, Idle, Firing, Latched };
    struct UnitState
    {
        quint32 fixture = 0;
        State state = Fresh;
        qint64 since = 0;
        qint64 lastEnd = std::numeric_limits<qint64>::min() / 2;
        quint32 token = 0;
        bool waitFromRelease = false;
    };

    QSharedPointer<const OutputLayer> m_installed;     // a shared pointer: a freed address never looks the same
    QVector<UnitState> m_units;                        // parallel to m_installed->units
    std::array<uchar, UNIVERSE_SIZE> m_src {};         // the sources of the last real frame (the watchdog reuses them)
    std::array<bool, UNIVERSE_SIZE> m_srcWritten {};
    std::array<std::atomic<uchar>, UNIVERSE_SIZE> m_out;
    std::array<std::atomic<short>, UNIVERSE_SIZE> m_held;
    std::atomic<int> m_ramp { 1000 };
    quint32 m_rampId = 0;
    qint64 m_rampStart = 0;
    bool m_rampOff = false;
    std::atomic<bool> m_anyFiring { false };
};

/** @} */

#endif
