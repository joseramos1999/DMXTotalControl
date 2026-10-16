/*
  Q Light Controller
  showrunner.h

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

#ifndef SHOWRUNNER_H
#define SHOWRUNNER_H

#include <QObject>
#include <QMutex>
#include <QMap>
#include <QHash>
#include <QList>

#include <function.h>

class ShowFunction;
class Function;
class Track;
class Show;
class Doc;

/** @addtogroup engine_functions Functions
 * @{
 */

class ShowRunner final : public QObject
{
    Q_OBJECT

public:
    ShowRunner(const Doc *doc, quint32 showID, quint32 startTime = 0);
    ~ShowRunner();

    /** Start the runner */
    void start();

    /** If running, pauses the runner and all the current running functions. */
    void setPause(bool enable);

    /** Stop the runner */
    void stop();

    void write(MasterTimer *timer);

private:
    const Doc *m_doc;

    /** The reference of the show to play */
    Show* m_show;

    /** The list of time-based Functions the Show needs to play */
    QList <ShowFunction *> m_timeFunctions;

    /** Index of the item in m_timeFunctions to be considered for playback */
    int m_currentTimeFunctionIndex;

    /** Elapsed time since runner start. Used also to move the cursor in the track view */
    quint32 m_elapsedTime;

    /** The list of beat-based Functions the Show needs to play */
    QList <ShowFunction *> m_beatFunctions;

    /** Index of the item in m_beatFunctions to be considered for playback */
    int m_currentBeatFunctionIndex;

    /** Elapsed beats since runner start */
    quint32 m_elapsedBeats;

    /** Flag used to sinchronize playback to beats */
    bool beatSynced;

    /** m_elapsedTime at the moment beatSynced became true, and the ms
     *  equivalent of m_elapsedBeats at that same moment (i.e. the resumed
     *  beat position, if any). Used to derive a smoothly advancing, but
     *  beat-zeroed, cursor position for a BPM based Show (see write()) */
    quint32 m_syncElapsedTime;
    quint32 m_syncBeatsTime;

    /** Total time (in ms) the runner has to run, computed from m_timeFunctions */
    quint32 m_totalRunTime;

    /** Total time (in beats, expressed as ms, i.e. 1000 per beat) the
     *  runner has to run, computed from m_beatFunctions */
    quint32 m_totalRunBeats;

    /** DMX Total Control: the running blocks (in place of the list of the
     *  running Functions). The function of a block by its ID: a function
     *  deleted or replaced meanwhile is not followed. The time the block
     *  stops, its overrides and whether it stretches its function.
     *  $function is the one the block started: never read unless the Doc
     *  still has it under that ID (blockFunction), and forgotten before the
     *  Doc deletes it (forgetFunction) */
    struct RunningBlock
    {
        ShowFunction *block;
        quint32 functionId;
        Function *function;
        quint32 stopTime;
        int intensityOverride;
        int phaseOverride;
        bool stretched;
    };
    QList <RunningBlock> m_running;

    /** DMX Total Control: the function a running block started, while the
     *  Doc has it (NULL: deleted, or replaced by another one with its ID) */
    Function *blockFunction(const RunningBlock &rb) const;

    /** DMX Total Control: the blocks whose function was replaced while they
     *  played: the next frame starts the new one, for the rest of the block */
    QList <ShowFunction *> m_replacedBlocks;
    void restartReplaced();

    /** DMX Total Control: every block, for a show that loops, and the end
     *  marker (0: none) */
    QList <ShowFunction *> m_allTimeFunctions;
    QList <ShowFunction *> m_allBeatFunctions;
    quint32 m_endTime;

    /** DMX Total Control: start a block, end it (its function stops unless
     *  another block plays it), apply the automation of the tracks, start
     *  again at the end of a looping show */
    void startBlock(ShowFunction *sf, Function *f, quint32 offset);
    void endBlock(const RunningBlock &block);
    void applyAutomation();
    void restart();

public:
    /** DMX Total Control: a block or a track (with its blocks) goes away:
     *  the runner drops them, what they play stops. Called with the runner
     *  mutex of the show held */
    void forgetBlock(ShowFunction *sf);
    void forgetTrack(Track *track);

    /** DMX Total Control: the function $fid is deleted ($current NULL) or
     *  replaced by $current (steps converted into a cuelist, a scene that got
     *  FX), and the Doc deletes the old one right after: the blocks that play
     *  the old one stop it and forget it; a replaced one plays the new
     *  function from the next frame. Called with the runner mutex of the
     *  show held */
    void forgetFunction(quint32 fid, Function *current);

private:
    FunctionParent functionParent() const;

signals:
    void timeChanged(quint32 time);
    void showFinished();

    /************************************************************************
     * Intensity
     ************************************************************************/
public:
    /**
     * Adjust the intensity of show track
     */
    void adjustIntensity(qreal fraction, const Track *track);

private:
    QMap<quint32, qreal> m_intensityMap;

};

/** @} */

#endif
