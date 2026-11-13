/*
  DMX Total Control
  timecodereader.h

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

#ifndef TIMECODEREADER_H
#define TIMECODEREADER_H

#include <QObject>
#include <QString>
#include <atomic>

class Doc;

/**
 * The MIDI timecode (MTC) of the MIDI inputs that are on: the MIDI plugin
 * sends each time on input channel 532, with the key "mtc:<ms>:<fps>". It
 * goes to the MasterTimer, where the Timecode cuelists that follow a MIDI
 * timecode read it. QML asks status() about ten times a second.
 *
 * The frame rates are 24, 25, 29 (29.97 drop-frame) and 30.
 */
class TimecodeReader : public QObject
{
    Q_OBJECT

public:
    /** The input channel of the MIDI timecode (CHANNEL_OFFSET_MTC) */
    static const quint32 kChannel = 532;

    explicit TimecodeReader(Doc *doc, QObject *parent = nullptr);

    /** "MTC 01:00:12:05 · 25 fps", or "No MIDI timecode" when none came in
     *  the last 250 ms */
    Q_INVOKABLE QString status() const;
    /** A timecode came in the last 250 ms */
    Q_INVOKABLE bool receiving() const;
    /** The frame rate of the last timecode that came, else 25 */
    Q_INVOKABLE int frameRate() const;

    /** An offset as hh:mm:ss:ff, in frames of the last timecode that came
     *  (25 fps until one comes) */
    Q_INVOKABLE QString offsetText(int ms) const;
    /** hh:mm:ss:ff (or hh, hh:mm, hh:mm:ss; ':', ';' or '.' between them)
     *  as ms, in frames of the last timecode; -1 when it is not a time */
    Q_INVOKABLE int parseOffset(const QString &text) const;

    /** hh:mm:ss:ff of $ms at $fps */
    static QString timecodeText(qint64 ms, int fps);
    /** The ms of hh:mm:ss:ff at $fps; -1 when it is not a time at that rate */
    static qint64 timecodeMs(int hours, int minutes, int seconds, int frames, int fps);

private slots:
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value, const QString &key);

private:
    Doc *m_doc;
    /** (0: none came) */
    std::atomic<int> m_fps;
};

#endif
