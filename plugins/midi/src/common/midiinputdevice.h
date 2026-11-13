/*
  Q Light Controller
  midiinputdevice.h

  Copyright (c) Heikki Junnila

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

#ifndef MIDIINPUTDEVICE_H
#define MIDIINPUTDEVICE_H

#include "mididevice.h"
#include "mtcdecoder.h"

class MidiInputDevice : public MidiDevice
{
    Q_OBJECT

public:
    MidiInputDevice(const QVariant& uid, const QString& name, QObject* parent = 0);
    virtual ~MidiInputDevice();

    void emitValueChanged(uint channel, uchar value);

    /** DMX Total Control: the MIDI timecode, in the thread the MIDI comes
     *  in. The data byte of a quarter frame (F1), or a SysEx message (a full
     *  frame): each time it gives goes out on CHANNEL_OFFSET_MTC, with the
     *  key "mtc:<ms>:<fps>" (keyValueChanged) */
    void processQuarterFrame(uchar data);
    void processSysEx(const QByteArray& message);
    /** The pieces of a time that came before are forgotten (on open) */
    void resetTimecode();

signals:
    void valueChanged(const QVariant& uid, ushort channel, uchar value);
    /** DMX Total Control: a value with a key (the MIDI timecode) */
    void keyValueChanged(const QVariant& uid, ushort channel, uchar value, const QString& key);

private:
    /** DMX Total Control: the time the decoder has, out on CHANNEL_OFFSET_MTC */
    void emitTimecode();

private:
    MtcDecoder m_mtc;
    /** (the value of the next time: it changes each time, so the input
     *  passes each one on, the last one in each frame) */
    uint m_mtcSerial;
};

#endif
