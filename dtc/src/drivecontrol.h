/*
  DMX Total Control
  drivecontrol.h

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

#ifndef DRIVECONTROL_H
#define DRIVECONTROL_H

#include <QElapsedTimer>
#include <QSharedPointer>
#include <QAtomicInt>
#include <QObject>
#include <QStringList>

#include "function.h"

class AudioCapture;
class QTimer;
class Doc;

/**
 * The driving modes: scenes in BeatGO mode jump to their next
 * division at each BeatGO press, scenes in Pulse mode at each peak of the
 * audio input above the threshold.
 */
class DriveControl : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool pulseEnabled READ pulseEnabled WRITE setPulseEnabled NOTIFY pulseEnabledChanged)
    /** Audio level that makes a pulse, 0..1 */
    Q_PROPERTY(qreal threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged)
    /** Current audio input level, 0..1 */
    Q_PROPERTY(qreal level READ level NOTIFY levelChanged)

public:
    DriveControl(Doc *doc, QObject *parent = nullptr);
    ~DriveControl();

    /** Next division of the running scenes in BeatGO mode. @return how many */
    Q_INVOKABLE int beatGo();
    /** Next division of the running scenes in Pulse mode. @return how many */
    Q_INVOKABLE int pulse();

    bool pulseEnabled() const;
    void setPulseEnabled(bool enable);

    qreal threshold() const;
    void setThreshold(qreal threshold);

    qreal level() const;

    /** The audio inputs of the machine, by name */
    Q_INVOKABLE QStringList audioInputs() const;
    /** Another audio input was chosen: Pulse listens to it from now on (an
     *  audio matrix that is running, from its next start) */
    void restartInput();

signals:
    void pulseEnabledChanged();
    void thresholdChanged();
    void levelChanged();
    /** A BeatGO or a pulse just happened, for the UI */
    void beatGoTriggered();
    void pulseTriggered();

protected:
    /** The audio input of the pulses: the one of the show (the Doc deletes it
     *  with the show, Pulse goes on with the next one) */
    virtual QSharedPointer<AudioCapture> inputCapture() const;

private slots:
    /** From the audio capture thread */
    void slotAudioData(double *bands, int size, double maxMagnitude, quint32 power);
    void slotPulse();
    void slotUpdateLevel();
    void slotDocClearing();
    void slotDocCleared();

private:
    int jump(Function::DriveMode mode);
    /** Listen to the audio input, and let it go */
    bool attachCapture();
    void detachCapture();

private:
    Doc *m_doc;
    bool m_pulseEnabled;
    /** The input listened to while Pulse is on */
    QSharedPointer<AudioCapture> m_capture;
    QTimer *m_levelTimer;
    qreal m_level;

    // shared with the capture thread
    QAtomicInt m_levelPermille;
    QAtomicInt m_thresholdPermille;
    bool m_above;
    QElapsedTimer m_sinceLastPulse;
};

#endif
