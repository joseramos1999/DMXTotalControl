/*
  Q Light Controller
  showrunner.cpp

  Copyright (c) Massimo Callegari

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

#include <QMutex>
#include <QDebug>

#include "showrunner.h"
#include "function.h"
#include "track.h"
#include "show.h"
#include "doc.h"
#include "inputoutputmap.h"
#include "showfunction.h"
#include "curvefx.h"
#include "fxscene.h"
#include "efx.h"

#define TIMER_INTERVAL 50

static bool compareShowFunctions(const ShowFunction *sf1, const ShowFunction *sf2)
{
    if (sf1->startTime() < sf2->startTime())
        return true;
    return false;
}

ShowRunner::ShowRunner(const Doc* doc, quint32 showID, quint32 startTime)
    : QObject(NULL)
    , m_doc(doc)
    , m_currentTimeFunctionIndex(0)
    , m_elapsedTime(startTime)
    , m_currentBeatFunctionIndex(0)
    , m_elapsedBeats(0)
    , beatSynced(false)
    , m_syncElapsedTime(0)
    , m_syncBeatsTime(0)
    , m_totalRunTime(0)
    , m_totalRunBeats(0)
    , m_endTime(0)
{
    Q_ASSERT(m_doc != NULL);
    Q_ASSERT(showID != Show::invalidId());

    m_show = qobject_cast<Show*>(m_doc->function(showID));
    if (m_show == NULL)
        return;

    /* startTime (e.g. coming from the cursor position) is always real
       milliseconds. If playback doesn't start from 0, m_elapsedBeats needs
       the equivalent estimate in "beats as ms" too, otherwise every Play
       would always start counting beats from 0 regardless of where
       playback actually begins - making beat-based Functions that should
       already be running (or long finished) at startTime behave as if
       playback had just begun. This is only a starting estimate: once the
       first real beat lands (see beatSynced in write()), m_elapsedBeats
       keeps advancing in lockstep with the actual beat clock from there */
    if (startTime > 0)
    {
        int bpm = m_doc->inputOutputMap()->bpmNumber();
        if (bpm > 0)
            m_elapsedBeats = Function::timeToBeats(startTime, 60000 / bpm);
    }

    foreach (Track *track, m_show->tracks())
    {
        // some sanity checks
        if (track == NULL ||
            track->id() == Track::invalidId())
                continue;

        if (track->isMute())
            continue;

        // get all the functions of the track and append them to the runner queue
        foreach (ShowFunction *sfunc, track->showFunctions())
        {
            Function *f = m_doc->function(sfunc->functionID());
            if (f == NULL)
                continue;

            // DMX Total Control: on a timeline in ms every block is in ms
            const bool inTime = f->tempoType() == Function::Time || m_show->msTimeline();
            const quint32 end = sfunc->startTime() + sfunc->duration(m_doc);

            // DMX Total Control: every block, for a show that loops (of QLC+
            // too), and the whole length even when it starts later
            if (inTime)
            {
                m_allTimeFunctions.append(sfunc);
                m_totalRunTime = qMax(m_totalRunTime, end);
            }
            else
            {
                m_allBeatFunctions.append(sfunc);
                m_totalRunBeats = qMax(m_totalRunBeats, end);
            }

            if (end <= startTime)
                continue;

            if (inTime)
                m_timeFunctions.append(sfunc);
            else
                m_beatFunctions.append(sfunc);
        }

        // Initialize the intensity map
        m_intensityMap[track->id()] = 1.0;
    }

    // DMX Total Control: the end marker cuts the show there (a show of QLC+
    // edited here too)
    m_endTime = m_show->endTime();
    if (m_endTime > 0)
        m_totalRunTime = m_endTime;

    std::sort(m_timeFunctions.begin(), m_timeFunctions.end(), compareShowFunctions);
    std::sort(m_allTimeFunctions.begin(), m_allTimeFunctions.end(), compareShowFunctions);
    std::sort(m_beatFunctions.begin(), m_beatFunctions.end(), compareShowFunctions);
    std::sort(m_allBeatFunctions.begin(), m_allBeatFunctions.end(), compareShowFunctions);

#if 1
    qDebug() << "Ordered list of ShowFunctions (time):";
    foreach (ShowFunction *sfunc, m_timeFunctions)
        qDebug() << "[Show] Function ID:" << sfunc->functionID() << "start time:" << sfunc->startTime() << "duration:" << sfunc->duration(m_doc);

    qDebug() << "Ordered list of ShowFunctions (beats):";
    foreach (ShowFunction *sfunc, m_beatFunctions)
        qDebug() << "[Show] Function ID:" << sfunc->functionID() << "start time:" << sfunc->startTime() << "duration:" << sfunc->duration(m_doc);
#endif
    m_running.clear();

    qDebug() << "ShowRunner created";
}

ShowRunner::~ShowRunner()
{
}

void ShowRunner::start()
{
    qDebug() << "ShowRunner started";
}

void ShowRunner::setPause(bool enable)
{
    for (const RunningBlock &rb : std::as_const(m_running))
    {
        // DMX Total Control: (not the function that replaced it: a playback
        // may be playing that one)
        Function *f = blockFunction(rb);
        if (f != NULL)
            f->setPause(enable);
    }
}

void ShowRunner::stop()
{
    m_elapsedTime = 0;
    m_elapsedBeats = 0;
    m_currentTimeFunctionIndex = 0;
    m_currentBeatFunctionIndex = 0;

    // DMX Total Control: the functions of the running blocks
    for (const RunningBlock &rb : std::as_const(m_running))
    {
        Function *f = blockFunction(rb);
        if (f == NULL)
            continue;
        f->stop(functionParent());
        if (rb.stretched)
            f->setSpeedFactor(1.0);
    }
    m_running.clear();
    m_replacedBlocks.clear();
    qDebug() << "ShowRunner stopped";
}

/*****************************************************************************
 * DMX Total Control: blocks with fades and stretch, automation, loop
 *****************************************************************************/

/** The attribute of the phase of an effect, -1 when it has none */
static int phaseAttribute(Function *f)
{
    if (f->type() == Function::EFXType)
        return EFX::StartOffset;
    if (f->type() == Function::FXSceneType)
        return FXScene::Phase;
    if (f->type() == Function::CurveFXType)
        return CurveFX::Phase;
    return -1;
}

void ShowRunner::startBlock(ShowFunction *sf, Function *f, quint32 offset)
{
    RunningBlock rb;
    rb.block = sf;
    rb.functionId = f->id();
    rb.function = f;
    rb.intensityOverride = Function::invalidAttributeId();
    rb.phaseOverride = Function::invalidAttributeId();
    rb.stretched = false;

    // the track the block is on now (it may have moved)
    Track *track = m_show->getTrackFromShowFunctionID(sf->id());
    if (track != NULL)
    {
        // the intensity of the track, and its automation
        qreal level = m_intensityMap.value(track->id(), 1.0) * track->automationValue("dimmer", m_elapsedTime, 1.0);
        rb.intensityOverride = f->requestAttributeOverride(Function::Intensity, level);
        sf->setIntensityOverrideId(rb.intensityOverride);

        int phase = phaseAttribute(f);
        if (phase >= 0 && track->hasAutomation("phase"))
            rb.phaseOverride = f->requestAttributeOverride(phase, track->automationValue("phase", m_elapsedTime, 0.0) * 359.0);
    }

    // stretched: one cycle of the function lasts the whole block
    quint32 duration = sf->duration(m_doc);
    if (sf->stretch() && f->type() != Function::SceneType && f->type() != Function::AudioType)
    {
        quint32 natural = f->totalDuration();
        if (natural > 0 && natural != Function::infiniteSpeed() && duration > 0)
        {
            f->setSpeedFactor(qreal(natural) / qreal(duration));
            rb.stretched = true;
        }
    }

    uint fadeIn = sf->fadeIn() > 0 ? sf->fadeIn() : Function::defaultSpeed();
    uint fadeOut = sf->fadeOut() > 0 ? sf->fadeOut() : Function::defaultSpeed();
    f->start(m_doc->masterTimer(), functionParent(), offset, fadeIn, fadeOut);

    // with a fade out, the block ends faded out
    rb.stopTime = sf->startTime() + duration;
    if (sf->fadeOut() > 0 && sf->fadeOut() < duration)
        rb.stopTime -= sf->fadeOut();
    m_running.append(rb);
}

void ShowRunner::endBlock(const RunningBlock &block)
{
    Function *f = blockFunction(block);
    if (f == NULL)
        return;

    // another block plays the function on (a block and its duplicate, back
    // to back, or two tracks): it goes on, without the levels of this one
    bool stretchedElsewhere = false;
    bool playedElsewhere = false;
    // (a Single attribute, as the intensity, has one override they share)
    bool intensityShared = false;
    bool phaseShared = false;
    for (const RunningBlock &rb : std::as_const(m_running))
    {
        if (rb.functionId != block.functionId)
            continue;
        playedElsewhere = true;
        stretchedElsewhere |= rb.stretched;
        intensityShared |= rb.intensityOverride == block.intensityOverride;
        phaseShared |= rb.phaseOverride == block.phaseOverride;
    }

    if (playedElsewhere)
    {
        if (intensityShared == false)
            f->releaseAttributeOverride(block.intensityOverride);
        if (phaseShared == false)
            f->releaseAttributeOverride(block.phaseOverride);
    }
    else
    {
        f->stop(functionParent());
    }
    if (block.stretched && stretchedElsewhere == false)
        f->setSpeedFactor(1.0);
}

void ShowRunner::applyAutomation()
{
    for (const RunningBlock &rb : std::as_const(m_running))
    {
        Function *f = blockFunction(rb);
        Track *track = m_show->getTrackFromShowFunctionID(rb.block->id());
        if (f == NULL || track == NULL)
            continue;

        if (track->hasAutomation("dimmer"))
            f->adjustAttribute(m_intensityMap.value(track->id(), 1.0) *
                               track->automationValue("dimmer", m_elapsedTime, 1.0), rb.intensityOverride);
        if (rb.phaseOverride != Function::invalidAttributeId())
            f->adjustAttribute(track->automationValue("phase", m_elapsedTime, 0.0) * 359.0, rb.phaseOverride);
    }
}

void ShowRunner::restart()
{
    // what plays stops, and the show plays again from the start
    for (const RunningBlock &rb : std::as_const(m_running))
    {
        Function *f = blockFunction(rb);
        if (f == NULL)
            continue;
        f->stop(functionParent());
        if (rb.stretched)
            f->setSpeedFactor(1.0);
    }
    m_running.clear();
    m_replacedBlocks.clear();

    m_timeFunctions = m_allTimeFunctions;
    m_beatFunctions = m_allBeatFunctions;
    m_currentTimeFunctionIndex = 0;
    m_currentBeatFunctionIndex = 0;
    m_elapsedTime = 0;
    m_elapsedBeats = 0;
    // the beats start again at the next beat
    beatSynced = false;
    emit timeChanged(0);
}

void ShowRunner::forgetBlock(ShowFunction *sf)
{
    int index = m_timeFunctions.indexOf(sf);
    if (index >= 0)
    {
        m_timeFunctions.removeAt(index);
        if (index < m_currentTimeFunctionIndex)
            m_currentTimeFunctionIndex--;
    }
    index = m_beatFunctions.indexOf(sf);
    if (index >= 0)
    {
        m_beatFunctions.removeAt(index);
        if (index < m_currentBeatFunctionIndex)
            m_currentBeatFunctionIndex--;
    }
    m_allTimeFunctions.removeAll(sf);
    m_allBeatFunctions.removeAll(sf);
    m_replacedBlocks.removeAll(sf);

    for (int i = m_running.count() - 1; i >= 0; i--)
    {
        if (m_running.at(i).block == sf)
            endBlock(m_running.takeAt(i));
    }
}

void ShowRunner::forgetTrack(Track *track)
{
    if (track == NULL)
        return;
    for (ShowFunction *sf : track->showFunctions())
        forgetBlock(sf);
}

Function *ShowRunner::blockFunction(const RunningBlock &rb) const
{
    // DMX Total Control: compared, never read, unless the Doc has it
    Function *f = m_doc->function(rb.functionId);
    return f == rb.function ? f : NULL;
}

void ShowRunner::forgetFunction(quint32 fid, Function *current)
{
    // DMX Total Control: the old function is still there (the Doc deletes it
    // after telling): it stops here, with nothing else of the block (its
    // overrides and speed go with it)
    for (int i = m_running.count() - 1; i >= 0; i--)
    {
        const RunningBlock rb = m_running.at(i);
        if (rb.functionId != fid || rb.function == current)
            continue;
        m_running.removeAt(i);
        rb.function->stop(functionParent());
        if (current != NULL && m_replacedBlocks.contains(rb.block) == false)
            m_replacedBlocks.append(rb.block);
    }
}

void ShowRunner::restartReplaced()
{
    // DMX Total Control: a block whose function was replaced while it played
    // plays the new one for the rest of its time, as a chaser plays the new
    // function of its step
    const QList<ShowFunction *> blocks = m_replacedBlocks;
    m_replacedBlocks.clear();
    for (ShowFunction *sf : blocks)
    {
        Function *f = m_doc->function(sf->functionID());
        if (f == NULL)
            continue;
        // where the show is, as the end of the block is checked (write())
        const bool inTime = f->tempoType() == Function::Time || m_show->msTimeline();
        if (inTime == false && beatSynced == false)
            continue;
        const quint32 now = inTime ? m_elapsedTime : m_elapsedBeats;
        const quint32 duration = sf->duration(m_doc);
        quint32 end = sf->startTime() + duration;
        if (sf->fadeOut() > 0 && sf->fadeOut() < duration)
            end -= sf->fadeOut();
        if (now < sf->startTime() || now >= end)
            continue;
        startBlock(sf, f, now - sf->startTime());
    }
}

FunctionParent ShowRunner::functionParent() const
{
    return FunctionParent(FunctionParent::Function, m_show->id());
}

void ShowRunner::write(MasterTimer *timer)
{
    //qDebug() << Q_FUNC_INFO << "elapsed:" << m_elapsedTime << ", total:" << m_totalRunTime;

    // Phase 1. Check all the Functions that need to be started
    // m_timeFunctions is ordered by startup time, so when we found an entry
    // with start time greater than m_elapsed, this phase is over
    bool startFunctionsDone = false;

    // A Show can freely mix time-based and beat-based Functions on its
    // tracks (e.g. a beat-synced Chaser next to a Time-based Audio track),
    // regardless of the Show's own timeline display type. So beat tracking
    // must not depend on, nor gate, anything based on the Show's own
    // division: it only needs to run when this Show actually has beat-based
    // Functions to drive, and it must never block m_timeFunctions/m_elapsedTime
    // from progressing while waiting for the first beat to land.
    if (m_beatFunctions.isEmpty() == false && timer->isBeat())
    {
        if (beatSynced == false)
        {
            beatSynced = true;
            m_syncElapsedTime = m_elapsedTime;
            int syncBpm = timer->bpmNumber();
            m_syncBeatsTime = syncBpm > 0 ? Function::beatsToTime(m_elapsedBeats, 60000 / syncBpm) : 0;
            qDebug() << "Beat synced";
        }
        else
        {
            m_elapsedBeats += 1000;
        }
    }

    // DMX Total Control: blocks whose function was replaced meanwhile
    if (m_replacedBlocks.isEmpty() == false)
        restartReplaced();

    // check if there are time-based functions to start
    while (startFunctionsDone == false)
    {
        if (m_currentTimeFunctionIndex == m_timeFunctions.count())
            break;

        ShowFunction *sf = m_timeFunctions.at(m_currentTimeFunctionIndex);
        quint32 funcStartTime = sf->startTime();
        quint32 functionTimeOffset = 0;
        Function *f = m_doc->function(sf->functionID());
        if (f == nullptr)
        {
            m_currentTimeFunctionIndex++;
            continue;
        }

        // this should happen only when a Show is not started from 0
        if (m_elapsedTime > funcStartTime)
        {
            functionTimeOffset = m_elapsedTime - funcStartTime;
            funcStartTime = m_elapsedTime;
        }
        if (m_elapsedTime >= funcStartTime)
        {
            // DMX Total Control: fades, stretch and automation of the block
            startBlock(sf, f, functionTimeOffset);
            m_currentTimeFunctionIndex++;
        }
        else
            startFunctionsDone = true;
    }

    startFunctionsDone = false;

    // check if there are beat-based functions to start
    // (wait for the first real beat to land before considering any of
    // them, so m_elapsedBeats == 0 is not mistaken for "beat zero happened")
    while (startFunctionsDone == false && beatSynced)
    {
        if (m_currentBeatFunctionIndex == m_beatFunctions.count())
            break;

        ShowFunction *sf = m_beatFunctions.at(m_currentBeatFunctionIndex);
        quint32 funcStartTime = sf->startTime();
        quint32 functionTimeOffset = 0;
        Function *f = m_doc->function(sf->functionID());
        if (f == nullptr)
        {
            m_currentBeatFunctionIndex++;
            continue;
        }

        // this should happen only when a Show is not started from 0
        if (m_elapsedBeats > funcStartTime)
        {
            functionTimeOffset = m_elapsedBeats - funcStartTime;
            funcStartTime = m_elapsedBeats;
        }
        if (m_elapsedBeats >= funcStartTime)
        {
            startBlock(sf, f, functionTimeOffset);
            m_currentBeatFunctionIndex++;
        }
        else
            startFunctionsDone = true;
    }

    // Phase 2. Check if we need to stop some running Functions
    // It is done in reverse order for two reasons:
    // 1- m_runningQueue is not ordered by stop time
    // 2- to avoid messing up with indices when an entry is removed
    for (int i = m_running.count() - 1; i >= 0; i--)
    {
        // DMX Total Control: a function deleted meanwhile is forgotten; one
        // replaced is left to forgetFunction (the show is told before the
        // Doc deletes the old one), which plays the new one
        Function *func = blockFunction(m_running.at(i));
        if (func == NULL)
        {
            if (m_doc->function(m_running.at(i).functionId) == NULL)
                m_running.removeAt(i);
            continue;
        }
        quint32 stopTime = m_running.at(i).stopTime;
        quint32 currTime = func->tempoType() == Function::Time ? m_elapsedTime : m_elapsedBeats;

        // DMX Total Control: blocks on a timeline in ms
        if (m_show->msTimeline())
            currTime = m_elapsedTime;

        // if we passed the function stop time
        if (currTime >= stopTime)
        {
            // DMX Total Control: the function stops, unless another block
            // plays it on
            endBlock(m_running.takeAt(i));
        }
    }

    // DMX Total Control: dimmer, volume and phase follow the tracks
    applyAutomation();

    // Phase 3. Check if this is the end of the Show. A Show can mix
    // time-based and beat-based tracks, so it is only really over once
    // both the time-based and the beat-based timelines have completed.
    // While there are beat-based Functions but no beat has been detected
    // yet, the beat timeline hasn't even started, so it can't be "done".
    bool timeDone = m_elapsedTime >= m_totalRunTime;
    bool beatsDone = m_beatFunctions.isEmpty() ||
                      (beatSynced && m_elapsedBeats >= m_totalRunBeats);
    // DMX Total Control: the end marker cuts the beats too
    bool cut = m_endTime > 0 && m_elapsedTime >= m_endTime;

    if ((timeDone && beatsDone) || cut)
    {
        // DMX Total Control: a looping show starts again
        if (m_show != NULL && m_show->loop() && (m_totalRunTime > 0 || m_totalRunBeats > 0))
        {
            restart();
            return;
        }
        if (m_show != NULL)
            m_show->stop(functionParent());
        emit showFinished();
        return;
    }

    m_elapsedTime += MasterTimer::tick();

    // Report plain elapsed milliseconds: it advances smoothly on every
    // tick, and the UI scales it to a beat/bar position against the
    // current BPM (see ShowManager and TimeUtils.timeToBeatPosition()) so
    // it reacts immediately to live BPM changes.
    //
    // However, when the Show's own timeline is BPM based, m_elapsedTime is
    // real wall-clock time since the Show started, which includes however
    // long it took to wait for the first beat to sync (beat 0 is defined
    // by that sync moment, not by when the Show started) - reporting it
    // directly would make the cursor jump to an arbitrary, not
    // beat-aligned position the instant sync happens. Instead, report the
    // beat-zeroed resume position (m_syncBeatsTime) plus how much real
    // time has passed since the sync moment: this still advances smoothly
    // every tick (unlike reporting m_elapsedBeats directly, which only
    // moves in whole-beat jumps), while staying exactly beat-aligned at
    // the sync instant itself.
    if (m_show->timeDivisionType() == Show::Time)
    {
        emit timeChanged(m_elapsedTime);
    }
    else if (beatSynced)
    {
        emit timeChanged(m_syncBeatsTime + (m_elapsedTime - m_syncElapsedTime));
    }
}

/************************************************************************
 * Intensity
 ************************************************************************/

void ShowRunner::adjustIntensity(qreal fraction, const Track *track)
{
    if (track == NULL)
        return;

    qDebug() << Q_FUNC_INFO << "Track ID: " << track->id() << ", val:" << fraction;
    m_intensityMap[track->id()] = fraction;

    // DMX Total Control: the running blocks of the track, with its automation
    const QList<ShowFunction *> blocks = track->showFunctions();
    for (const RunningBlock &rb : std::as_const(m_running))
    {
        if (blocks.contains(rb.block) == false)
            continue;
        Function *f = blockFunction(rb);
        if (f != NULL)
            f->adjustAttribute(fraction * track->automationValue("dimmer", m_elapsedTime, 1.0), rb.intensityOverride);
    }
}

