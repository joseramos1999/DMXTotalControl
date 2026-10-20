/*
  DMX Total Control
  scenetaps.cpp

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

#include <QElapsedTimer>
#include <QHash>
#include <QList>

#include "scenetaps.h"
#include "function.h"

namespace
{

/** The presses of a scene: the last one, and the spacing of the last beats */
struct Presses
{
    qint64 last = -1;
    QList<qint64> intervals;
};

QHash<quint32, Presses> &presses()
{
    static QHash<quint32, Presses> map;
    return map;
}

SceneTaps::Mode s_mode = SceneTaps::Toggle;
quint32 s_focus = Function::invalidId();

qint64 clockMs()
{
    static QElapsedTimer clock;
    if (clock.isValid() == false)
        clock.start();
    return clock.elapsed();
}

/** A press of a running scene with steps: a beat when it comes in a row (a
 *  step now, and its rhythm from the second beat on). @return whether it
 *  was one; $always: a step now anyway (Go: each press is a step) */
bool beat(Function *f, qint64 now, bool always)
{
    Presses &p = presses()[f->id()];
    const qint64 since = p.last >= 0 ? now - p.last : -1;
    p.last = now;
    // (a double click: one press)
    if (since >= 0 && since < 100)
        return always;
    const bool inRow = since >= 100 && since <= SceneTaps::window();
    if (inRow == false)
        p.intervals.clear();
    if (inRow == false && always == false)
        return false;

    // its step now (not while paused: it would come on resuming)...
    if (f->isPaused() == false)
        f->tapBeat();
    if (inRow == false)
        return true;
    // ...and the rhythm, the average of the last beats, in its own time:
    // with a live speed it plays what was tapped
    p.intervals.append(since);
    while (p.intervals.count() > 4)
        p.intervals.removeFirst();
    qint64 sum = 0;
    for (qint64 interval : std::as_const(p.intervals))
        sum += interval;
    f->setTapPeriod(uint(qRound64(qreal(sum) / p.intervals.count() * qMax(0.01, f->speedFactor()))));
    return true;
}

/** A press that starts it (or stops it): the count starts again */
void restart(Function *f, qint64 now)
{
    Presses &p = presses()[f->id()];
    p.last = now;
    p.intervals.clear();
}

} // namespace

void SceneTaps::setMode(Mode mode)
{
    s_mode = mode;
}

SceneTaps::Mode SceneTaps::mode()
{
    return s_mode;
}

bool SceneTaps::tappable(const Function *f)
{
    return f != nullptr && (f->type() == Function::ChaserType || f->type() == Function::SequenceType ||
                            f->type() == Function::CuelistType);
}

quint32 SceneTaps::focus()
{
    return s_focus;
}

SceneTaps::Outcome SceneTaps::press(Function *f, bool active, qint64 now)
{
    if (f == nullptr)
        return Keep;
    if (now < 0)
        now = clockMs();
    const bool steps = tappable(f);
    if (steps)
        s_focus = f->id();

    switch (s_mode)
    {
        case Stop:
            if (steps)
                restart(f, now);
            return active ? StopIt : Keep;
        case Go:
            if (active == false)
            {
                if (steps)
                    restart(f, now);
                return StartIt;
            }
            // running: each press a step, in a row its rhythm
            if (steps)
                beat(f, now, true);
            return Keep;
        case Toggle:
        default:
            if (steps == false)
                return ToggleIt;
            if (active == false)
            {
                restart(f, now);
                return ToggleIt;
            }
            // presses in a row of a running one are beats; one after a pause stops it
            return beat(f, now, false) ? Keep : ToggleIt;
    }
}

void SceneTaps::clear()
{
    presses().clear();
    s_focus = Function::invalidId();
}
