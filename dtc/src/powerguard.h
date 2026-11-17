/*
  DMX Total Control
  powerguard.h

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

#ifndef POWERGUARD_H
#define POWERGUARD_H

#include <QObject>
#include <functional>

class AppSettings;
class QTimer;

/**
 * The computer during a show: Windows keeps it awake and its screen on
 * while the program runs (Options › Keep the computer awake), and the
 * program knows when the laptop runs on its battery, when the battery is
 * low, and when Windows may suspend the USB ports (the DMX interface).
 * It only reads the power settings of the system: it never changes one.
 */
class PowerGuard : public QObject
{
    Q_OBJECT

    /** Windows was told to keep the computer awake and the screen on */
    Q_PROPERTY(bool keepAwakeActive READ keepAwakeActive NOTIFY changed)
    /** The computer has a battery (a laptop) */
    Q_PROPERTY(bool hasBattery READ hasBattery NOTIFY changed)
    /** It runs on its battery: the charger is not plugged in */
    Q_PROPERTY(bool onBattery READ onBattery NOTIFY changed)
    /** What is left of the battery, 0-100; -1: unknown */
    Q_PROPERTY(int batteryPercent READ batteryPercent NOTIFY changed)
    /** On the battery, at 20 % or less */
    Q_PROPERTY(bool batteryLow READ batteryLow NOTIFY changed)
    /** The power plan lets Windows suspend the USB ports that seem idle
     *  (USB selective suspend): plugged in, or on the battery of a laptop.
     *  Read once, when the program starts */
    Q_PROPERTY(bool usbSelectiveSuspend READ usbSelectiveSuspend NOTIFY changed)

public:
    /** What the system says (or a test) */
    struct Reading
    {
        bool hasBattery = false;
        bool onBattery = false;
        /** 0-100; -1: unknown */
        int batteryPercent = -1;
        /** USB selective suspend of the active power plan, plugged in and on
         *  the battery: 1 on, 0 off, -1 unknown */
        int usbSuspendAc = -1;
        int usbSuspendDc = -1;
    };

    /** The battery at this level or below is low */
    static constexpr int LowBatteryPercent = 20;
    /** How often the battery is looked at (and on each power change) */
    static constexpr int PollMs = 30000;

    explicit PowerGuard(AppSettings *settings, QObject *parent = nullptr);
    ~PowerGuard();

    bool keepAwakeActive() const { return m_keepAwakeActive; }
    bool hasBattery() const { return m_reading.hasBattery; }
    bool onBattery() const { return m_reading.onBattery; }
    int batteryPercent() const { return m_reading.batteryPercent; }
    bool batteryLow() const;
    bool usbSelectiveSuspend() const;

    /** Look at the battery again (the timer and the power changes do) */
    Q_INVOKABLE void refresh();
    /** The interval of the timer that looks at the battery */
    int pollInterval() const;

    /** A message of the window (a Windows MSG): a power change looks at
     *  the battery at once. False: not one of those */
    bool nativeMessage(void *message);
    /** The same, from its message and wParam: WM_POWERBROADCAST with
     *  PBT_APMPOWERSTATUSCHANGE (the charger plugged in or out, the battery) */
    bool powerMessage(unsigned message, quintptr wParam);
    static constexpr unsigned WmPowerBroadcast = 0x0218;
    static constexpr quintptr PbtPowerStatusChange = 0x000A;

    /** What the system says now (read only) */
    static Reading systemReading();

    /** Tests and the developer screenshots: the readings come from here
     *  instead of the system (empty: the system again) */
    static void setReaderForTest(const std::function<Reading()> &reader);
    /** Tests: what SetThreadExecutionState would be given goes here (it
     *  returns what it would: 0 refused), not to Windows (empty: Windows) */
    static void setExecutionStateHookForTest(const std::function<unsigned(unsigned)> &hook);

    /** The flags of SetThreadExecutionState (the same values as Windows) */
    static constexpr unsigned EsContinuous = 0x80000000u;
    static constexpr unsigned EsSystemRequired = 0x00000001u;
    static constexpr unsigned EsDisplayRequired = 0x00000002u;

signals:
    void changed();

private slots:
    void applyKeepAwake();

private:
    static Reading read();
    static unsigned setExecutionState(unsigned flags);

    AppSettings *m_settings;
    QTimer *m_timer;
    Reading m_reading;
    /** Windows was asked to keep the computer awake (keepAwakeActive: and it said yes) */
    bool m_keepAwakeRequested = false;
    bool m_keepAwakeActive = false;
};

#endif
