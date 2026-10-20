/*
  DMX Total Control
  scenetaps.h

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

#ifndef SCENETAPS_H
#define SCENETAPS_H

#include <QtGlobal>

class Function;

/**
 * What a press of a scene's own button does (a tile of Live, a button of
 * the SHOW, a mapping, the phone), and the rhythm of a scene with steps or
 * of a cuelist, tapped on that button.
 *
 * The buttons work in one of three modes (the GO and Stop of the top bar):
 * - Toggle: a press starts it or stops it; presses of a running scene with
 *   steps less than window() apart are beats instead (no stop);
 * - Go: a press starts it, and the presses of a running one are beats; it
 *   never stops it;
 * - Stop: a press stops it.
 * A beat plays its step (a cue) now; from two beats in a row it plays a
 * step a beat, the average of the last ones (Function::setTapPeriod, saved
 * with it). Only presses of a running scene are beats: the one that starts
 * it starts the count.
 */
class SceneTaps
{
public:
    enum Mode { Toggle, Go, Stop };
    static void setMode(Mode mode);
    static Mode mode();

    /** What the caller does with the press */
    enum Outcome
    {
        ToggleIt,   ///< as always: start it, or stop it
        StartIt,    ///< start it (it doesn't play)
        StopIt,     ///< stop it (it plays)
        Keep        ///< nothing: a beat, or nothing to do in this mode
    };

    /** A press of $f's own button, running ($active) or not, at $now (ms, -1:
     *  the clock) */
    static Outcome press(Function *f, bool active, qint64 now = -1);

    /** Whether $f takes a rhythm: steps and cuelists */
    static bool tappable(const Function *f);

    /** The scene with steps whose button was pressed last (the rhythm bar
     *  follows it while it plays); an invalid ID if none */
    static quint32 focus();

    /** Presses further apart are not beats (ms) */
    static int window() { return 2000; }

    /** Forget the presses (a new show) */
    static void clear();
};

#endif
