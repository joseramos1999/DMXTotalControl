/*
  Q Light Controller Plus
  mastertimer-win32.cpp

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

// Let's assume we have at least W2K (http://msdn.microsoft.com/en-us/library/Aa383745)
#ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x05000000
  #define _WIN32_WINDOWS 0x05000000
  #define WINVER 0x05000000
#endif

#include <QElapsedTimer>
#include <QThread>
#include <QDebug>

#include "mastertimer-win32.h"
#include "mastertimer.h"
#include "qlcmacros.h"

/****************************************************************************
 * Timer callback
 ****************************************************************************/
extern "C"
{
    void CALLBACK masterTimerWin32Callback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
    {
        Q_UNUSED(TimerOrWaitFired);

        MasterTimerPrivate* mtp = (MasterTimerPrivate*) lpParameter;
        Q_ASSERT(mtp != NULL);
        mtp->timerTick();
    }
}

/****************************************************************************
 * MasterTimerPrivate
 ****************************************************************************/

MasterTimerPrivate::MasterTimerPrivate(MasterTimer* masterTimer)
    : m_masterTimer(masterTimer)
    , m_systemTimerResolution(0)
    , m_phTimer(NULL)
    , m_run(false)
    , m_waitableTimer(NULL)
    , m_stopEvent(NULL)
    , m_thread(NULL)
{
    Q_ASSERT(masterTimer != NULL);
}

MasterTimerPrivate::~MasterTimerPrivate()
{
    stop();
}

void MasterTimerPrivate::start()
{
    if (m_run == true)
        return;

    /* Find out the smallest possible timer tick in milliseconds */
    TIMECAPS ptc;
    MMRESULT result = timeGetDevCaps(&ptc, sizeof(TIMECAPS));
    if (result != TIMERR_NOERROR)
    {
        qWarning() << Q_FUNC_INFO << "Unable to query system timer resolution.";
        return;
    }

    /* Adjust system timer to operate on its minimum tick period */
    // DMX Total Control: its minimum indeed (1 ms). It asked for the tick
    // (20 ms), coarser than the default 15.6 ms: it did nothing
    m_systemTimerResolution = ptc.wPeriodMin;
    result = timeBeginPeriod(m_systemTimerResolution);
    if (result != TIMERR_NOERROR)
    {
        qWarning() << Q_FUNC_INFO << "Unable to adjust system timer resolution.";
        return;
    }

    if (startHighResolution())
    {
        m_run = true;
        return;
    }

    // DMX Total Control: without a high resolution timer, the timer queue.
    // It fires on the clock of the system: every 15.6 ms unless something
    // raises its resolution, so a 20 ms tick comes after 15.6 or 31.2 ms
    BOOL ok = CreateTimerQueueTimer(&m_phTimer,
                                    NULL,
                                    (WAITORTIMERCALLBACK) masterTimerWin32Callback,
                                    this,
                                    0,
                                    m_masterTimer->tick(),
                                    WT_EXECUTELONGFUNCTION);
    if (!ok)
    {
        qWarning() << Q_FUNC_INFO << "Unable to create a timer:" << GetLastError();
        timeEndPeriod(m_systemTimerResolution);
        m_systemTimerResolution = 0;
        return;
    }

    m_run = true;
}

void MasterTimerPrivate::stop()
{
    if (m_run == false)
        return;

    if (m_thread != NULL)
    {
        // DMX Total Control: the thread ends after the tick in progress
        SetEvent(m_stopEvent);
        m_thread->wait();
        delete m_thread;
        m_thread = NULL;
        CloseHandle(m_waitableTimer);
        m_waitableTimer = NULL;
        CloseHandle(m_stopEvent);
        m_stopEvent = NULL;
        timeEndPeriod(m_systemTimerResolution);
    }
    // Destroy the timer and wait for it to complete its last firing (if applicable)
    else if (DeleteTimerQueueTimer(NULL, m_phTimer, INVALID_HANDLE_VALUE))
    {
        timeEndPeriod(m_systemTimerResolution);
    }

    m_systemTimerResolution = 0;
    m_phTimer = NULL;
    m_run = false;
}

bool MasterTimerPrivate::startHighResolution()
{
    m_waitableTimer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
                                             TIMER_ALL_ACCESS);
    if (m_waitableTimer == NULL)
    {
        qDebug() << Q_FUNC_INFO << "No high resolution timer:" << GetLastError();
        return false;
    }

    m_stopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (m_stopEvent == NULL)
    {
        qWarning() << Q_FUNC_INFO << "Unable to create an event:" << GetLastError();
        CloseHandle(m_waitableTimer);
        m_waitableTimer = NULL;
        return false;
    }

    m_thread = QThread::create([this]() { runHighResolution(); });
    m_thread->start();
    return true;
}

void MasterTimerPrivate::runHighResolution()
{
    // the ticks are due every period from the start (no drift): one that
    // comes late doesn't move the next ones. More than a period late (a
    // tick that took too long), the schedule starts again from now: no
    // burst of ticks to catch up
    const qint64 period = qint64(m_masterTimer->tick()) * 1000000;
    const HANDLE handles[2] = { m_stopEvent, m_waitableTimer };
    QElapsedTimer clock;
    clock.start();
    qint64 due = period;

    while (true)
    {
        const qint64 wait = due - clock.nsecsElapsed();
        if (wait > 0)
        {
            // relative due time, in 100 ns units
            LARGE_INTEGER dueTime;
            dueTime.QuadPart = -qMax(qint64(1), wait / 100);
            if (SetWaitableTimer(m_waitableTimer, &dueTime, 0, NULL, NULL, FALSE) == FALSE)
            {
                qWarning() << Q_FUNC_INFO << "Unable to set the timer:" << GetLastError();
                Sleep(DWORD(wait / 1000000));
            }
            else if (WaitForMultipleObjects(2, handles, FALSE, INFINITE) == WAIT_OBJECT_0)
            {
                break;
            }
        }
        if (WaitForSingleObject(m_stopEvent, 0) == WAIT_OBJECT_0)
            break;

        timerTick();

        due += period;
        const qint64 now = clock.nsecsElapsed();
        if (now - due > period)
            due = now;
    }
}

bool MasterTimerPrivate::isRunning() const
{
    return m_run;
}

void MasterTimerPrivate::timerTick()
{
    m_masterTimer->timerTick();
}
