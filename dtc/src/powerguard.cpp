/*
  DMX Total Control
  powerguard.cpp

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

#include <QTimer>

#include "powerguard.h"
#include "appsettings.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#include <powrprof.h>

// the USB settings of a power plan, and in them USB selective suspend
// (0 disabled, 1 enabled)
static const GUID KUsbSettingsGroup = { 0x2a737441, 0x1930, 0x4402, { 0x8d, 0x77, 0xb2, 0xbe, 0xbb, 0xa3, 0x08, 0xa3 } };
static const GUID KUsbSelectiveSuspend = { 0x48e6b7a6, 0x50f5, 0x4782, { 0xa5, 0xd4, 0x53, 0xbb, 0x8f, 0x07, 0xe2, 0x26 } };

static_assert(PowerGuard::EsContinuous == ES_CONTINUOUS, "ES_CONTINUOUS");
static_assert(PowerGuard::EsSystemRequired == ES_SYSTEM_REQUIRED, "ES_SYSTEM_REQUIRED");
static_assert(PowerGuard::EsDisplayRequired == ES_DISPLAY_REQUIRED, "ES_DISPLAY_REQUIRED");
static_assert(PowerGuard::WmPowerBroadcast == WM_POWERBROADCAST, "WM_POWERBROADCAST");
static_assert(PowerGuard::PbtPowerStatusChange == PBT_APMPOWERSTATUSCHANGE, "PBT_APMPOWERSTATUSCHANGE");
#endif

namespace
{
// (tests and screenshots: see setReaderForTest, setExecutionStateHookForTest)
std::function<PowerGuard::Reading()> s_reader;
std::function<unsigned(unsigned)> s_executionStateHook;
}

PowerGuard::PowerGuard(AppSettings *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_timer(new QTimer(this))
{
    // the power plan once, the battery from now on
    m_reading = read();

    m_timer->setInterval(PollMs);
    connect(m_timer, &QTimer::timeout, this, &PowerGuard::refresh);
    m_timer->start();

    if (m_settings != nullptr)
        connect(m_settings, &AppSettings::liveSafetyChanged, this, &PowerGuard::applyKeepAwake);
    applyKeepAwake();
}

PowerGuard::~PowerGuard()
{
    // Windows may let the computer sleep again, as before the program
    if (m_keepAwakeRequested)
        setExecutionState(EsContinuous);
}

bool PowerGuard::batteryLow() const
{
    return m_reading.onBattery && m_reading.batteryPercent >= 0 && m_reading.batteryPercent <= LowBatteryPercent;
}

bool PowerGuard::usbSelectiveSuspend() const
{
    // on the battery it counts only where there is one
    return m_reading.usbSuspendAc == 1 || (m_reading.hasBattery && m_reading.usbSuspendDc == 1);
}

void PowerGuard::refresh()
{
    const Reading now = read();
    // (the power plan was read when the program started)
    if (now.hasBattery == m_reading.hasBattery && now.onBattery == m_reading.onBattery
        && now.batteryPercent == m_reading.batteryPercent)
        return;
    m_reading.hasBattery = now.hasBattery;
    m_reading.onBattery = now.onBattery;
    m_reading.batteryPercent = now.batteryPercent;
    emit changed();
}

int PowerGuard::pollInterval() const
{
    return m_timer->interval();
}

bool PowerGuard::nativeMessage(void *message)
{
#if defined(Q_OS_WIN)
    const MSG *msg = static_cast<const MSG *>(message);
    if (msg != nullptr)
        return powerMessage(msg->message, quintptr(msg->wParam));
#else
    Q_UNUSED(message)
#endif
    return false;
}

bool PowerGuard::powerMessage(unsigned message, quintptr wParam)
{
    if (message != WmPowerBroadcast || wParam != PbtPowerStatusChange)
        return false;
    // (after the message: the status has changed by then)
    QTimer::singleShot(0, this, &PowerGuard::refresh);
    return true;
}

void PowerGuard::applyKeepAwake()
{
    const bool wanted = m_settings != nullptr && m_settings->keepAwake();
    if (wanted == m_keepAwakeRequested)
        return;
    m_keepAwakeRequested = wanted;
    bool active = false;
    if (wanted)
        active = setExecutionState(EsContinuous | EsSystemRequired | EsDisplayRequired) != 0;
    else
        setExecutionState(EsContinuous);
    if (active != m_keepAwakeActive)
    {
        m_keepAwakeActive = active;
        emit changed();
    }
}

PowerGuard::Reading PowerGuard::read()
{
    return s_reader ? s_reader() : systemReading();
}

PowerGuard::Reading PowerGuard::systemReading()
{
    Reading reading;
#if defined(Q_OS_WIN)
    SYSTEM_POWER_STATUS status;
    if (GetSystemPowerStatus(&status))
    {
        // BatteryFlag 128: no battery, 255: unknown; ACLineStatus 0: offline
        reading.hasBattery = status.BatteryFlag != 255 && (status.BatteryFlag & 128) == 0;
        reading.onBattery = status.ACLineStatus == 0;
        reading.batteryPercent = status.BatteryLifePercent <= 100 ? int(status.BatteryLifePercent) : -1;
    }
    // only read: the power plan is never changed here
    GUID *scheme = nullptr;
    if (PowerGetActiveScheme(nullptr, &scheme) == ERROR_SUCCESS && scheme != nullptr)
    {
        DWORD value = 0;
        if (PowerReadACValueIndex(nullptr, scheme, &KUsbSettingsGroup, &KUsbSelectiveSuspend, &value) == ERROR_SUCCESS)
            reading.usbSuspendAc = value != 0 ? 1 : 0;
        value = 0;
        if (PowerReadDCValueIndex(nullptr, scheme, &KUsbSettingsGroup, &KUsbSelectiveSuspend, &value) == ERROR_SUCCESS)
            reading.usbSuspendDc = value != 0 ? 1 : 0;
        LocalFree(scheme);
    }
#endif
    return reading;
}

unsigned PowerGuard::setExecutionState(unsigned flags)
{
    if (s_executionStateHook)
        return s_executionStateHook(flags);
#if defined(Q_OS_WIN)
    // (for the thread of the program: the one of the window)
    return unsigned(SetThreadExecutionState(EXECUTION_STATE(flags)));
#else
    Q_UNUSED(flags)
    return 0;
#endif
}

void PowerGuard::setReaderForTest(const std::function<Reading()> &reader)
{
    s_reader = reader;
}

void PowerGuard::setExecutionStateHookForTest(const std::function<unsigned(unsigned)> &hook)
{
    s_executionStateHook = hook;
}
