/*
  DMX Total Control
  drivecontrol.cpp

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

#include <QSettings>
#include <QTimer>
#include <QDebug>
#include <qmath.h>

#include "drivecontrol.h"
#include "audiocapture.h"
#include "audioplugincache.h"
#include "audiorenderer.h"
#include "function.h"
#include "doc.h"

#define SETTINGS_THRESHOLD "drive/pulseThreshold"
// the audio power goes up to 32767; music rarely gets past a third of it
#define POWER_FULL_SCALE 12000.0
// pulses closer than this are the same peak
#define MIN_PULSE_INTERVAL 120

DriveControl::DriveControl(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_pulseEnabled(false)
    , m_levelTimer(new QTimer(this))
    , m_level(0)
    , m_above(false)
{
    QSettings settings;
    m_thresholdPermille.storeRelaxed(qBound(10, int(settings.value(SETTINGS_THRESHOLD, 0.6).toDouble() * 1000), 1000));

    m_levelTimer->setInterval(50);
    connect(m_levelTimer, &QTimer::timeout, this, &DriveControl::slotUpdateLevel);
    m_sinceLastPulse.start();

    // a new show (File > New, Open) deletes the audio input of the show
    // before: the pulses go on with the new one (engine signals by name: MinGW)
    connect(m_doc, SIGNAL(clearing()), this, SLOT(slotDocClearing()));
    connect(m_doc, SIGNAL(cleared()), this, SLOT(slotDocCleared()));
}

DriveControl::~DriveControl()
{
    setPulseEnabled(false);
}

QSharedPointer<AudioCapture> DriveControl::inputCapture() const
{
    return m_doc->audioInputCapture();
}

bool DriveControl::attachCapture()
{
    QSharedPointer<AudioCapture> capture = inputCapture();
    if (capture.isNull())
        return false;

    // the audio thread calls back directly: only the level and the edge
    // detection run there, the jumps are queued to this thread
    connect(capture.data(), SIGNAL(dataProcessed(double*,int,double,quint32)),
            this, SLOT(slotAudioData(double*,int,double,quint32)), Qt::DirectConnection);
    capture->registerBandsNumber(16);
    m_capture = capture;
    return true;
}

void DriveControl::detachCapture()
{
    if (m_capture.isNull())
        return;
    disconnect(m_capture.data(), SIGNAL(dataProcessed(double*,int,double,quint32)),
               this, SLOT(slotAudioData(double*,int,double,quint32)));
    m_capture->unregisterBandsNumber(16);
    m_capture.clear();
    m_levelPermille.storeRelaxed(0);
}

void DriveControl::slotDocClearing()
{
    // (the show lets its audio input go: this one too, before it goes)
    detachCapture();
}

void DriveControl::slotDocCleared()
{
    if (m_pulseEnabled == false)
        return;
    if (attachCapture() == false)
    {
        m_levelTimer->stop();
        m_pulseEnabled = false;
        emit pulseEnabledChanged();
    }
    slotUpdateLevel();
}

int DriveControl::jump(Function::DriveMode mode)
{
    int count = 0;
    for (Function *f : m_doc->functions())
    {
        if (f->driveMode() == mode && f->isRunning())
        {
            f->requestDivisionJump();
            count++;
        }
    }
    return count;
}

int DriveControl::beatGo()
{
    emit beatGoTriggered();
    return jump(Function::DriveBeatGO);
}

int DriveControl::pulse()
{
    emit pulseTriggered();
    return jump(Function::DrivePulse);
}

bool DriveControl::pulseEnabled() const
{
    return m_pulseEnabled;
}

void DriveControl::setPulseEnabled(bool enable)
{
    if (enable == m_pulseEnabled)
        return;

    if (enable)
    {
        if (attachCapture() == false)
            return;
        m_levelTimer->start();
    }
    else
    {
        // (the input it listens to: the show may have another one now)
        detachCapture();
        m_levelTimer->stop();
        slotUpdateLevel();
    }

    m_pulseEnabled = enable;
    emit pulseEnabledChanged();
}

qreal DriveControl::threshold() const
{
    return qreal(m_thresholdPermille.loadRelaxed()) / 1000.0;
}

void DriveControl::setThreshold(qreal threshold)
{
    int permille = qBound(10, qRound(threshold * 1000.0), 1000);
    if (permille == m_thresholdPermille.loadRelaxed())
        return;

    m_thresholdPermille.storeRelaxed(permille);
    QSettings settings;
    settings.setValue(SETTINGS_THRESHOLD, qreal(permille) / 1000.0);
    emit thresholdChanged();
}

qreal DriveControl::level() const
{
    return m_level;
}

QStringList DriveControl::audioInputs() const
{
    QStringList names;
    for (const AudioDeviceInfo &info : m_doc->audioPluginCache()->audioDevicesList())
    {
        if (info.capabilities & AUDIO_CAP_INPUT)
            names.append(info.deviceName);
    }
    return names;
}

void DriveControl::restartInput()
{
    if (m_pulseEnabled == false)
        return;

    // the capture stops when nobody listens to it, and opens the input
    // chosen when it starts again
    detachCapture();
    if (attachCapture() == false)
    {
        m_levelTimer->stop();
        m_pulseEnabled = false;
        emit pulseEnabledChanged();
    }
    slotUpdateLevel();
}

void DriveControl::slotAudioData(double *bands, int size, double maxMagnitude, quint32 power)
{
    Q_UNUSED(bands)
    Q_UNUSED(size)
    Q_UNUSED(maxMagnitude)

    // a square root makes quiet music move the meter too
    qreal level = qMin(1.0, qSqrt(qreal(power) / POWER_FULL_SCALE));
    int permille = qRound(level * 1000.0);
    m_levelPermille.storeRelaxed(permille);

    // a pulse on each rise over the threshold, with some hysteresis
    int threshold = m_thresholdPermille.loadRelaxed();
    if (m_above == false && permille >= threshold)
    {
        m_above = true;
        if (m_sinceLastPulse.elapsed() >= MIN_PULSE_INTERVAL)
        {
            m_sinceLastPulse.restart();
            QMetaObject::invokeMethod(this, "slotPulse", Qt::QueuedConnection);
        }
    }
    else if (m_above && permille < threshold * 8 / 10)
    {
        m_above = false;
    }
}

void DriveControl::slotPulse()
{
    pulse();
}

void DriveControl::slotUpdateLevel()
{
    qreal level = qreal(m_levelPermille.loadRelaxed()) / 1000.0;
    if (qAbs(level - m_level) < 0.005)
        return;
    m_level = level;
    emit levelChanged();
}
