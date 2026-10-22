/*
  Q Light Controller Plus
  mastertimer-win32.h

  Copyright (C) Heikki Junnila
                Massimo Callegari

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

#ifndef MASTERTIMER_PRIVATE_H
#define MASTERTIMER_PRIVATE_H

#include <Windows.h>

class MasterTimer;
class QThread;

/** @addtogroup engine Engine
 * @{
 */

class MasterTimerPrivate final
{
public:
    MasterTimerPrivate(MasterTimer* masterTimer);
    ~MasterTimerPrivate();

    void start();
    void stop();
    bool isRunning() const;

    void timerTick();

private:
    /** DMX Total Control: a high resolution waitable timer (Windows 10 1803
     *  and later) in a thread of its own, on a fixed schedule: a tick every
     *  period from the start, not on the 15.6 ms clock of the system.
     *  False when the system has none (the timer queue does it then) */
    bool startHighResolution();
    void runHighResolution();

private:
    MasterTimer* m_masterTimer;
    UINT m_systemTimerResolution;
    HANDLE m_phTimer;
    bool m_run;

    // DMX Total Control: the high resolution timer, its thread, and the
    // event that stops it
    HANDLE m_waitableTimer;
    HANDLE m_stopEvent;
    QThread *m_thread;
};

/** @} */

#endif
