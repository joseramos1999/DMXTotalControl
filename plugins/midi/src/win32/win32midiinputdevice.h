/*
  Q Light Controller
  win32midiinputdevice.h

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

#ifndef WIN32MIDIINPUTDEVICE_H
#define WIN32MIDIINPUTDEVICE_H

#include <Windows.h>
#include <QObject>
#include <atomic>

#include "midiinputdevice.h"

class Win32MidiInputDevice final : public MidiInputDevice
{
    Q_OBJECT

public:
    Win32MidiInputDevice(const QVariant& uid, const QString& name, UINT id, QObject* parent = 0);
    ~Win32MidiInputDevice();

    bool open();
    void close();
    bool isOpen() const;

    bool processMBC(int type);

    /** DMX Total Control: a SysEx buffer came back (in the MIDI thread):
     *  its message is read when $complete (not a broken one), and it goes
     *  back for the next one, unless the device closes */
    void processSysExBuffer(HMIDIIN handle, MIDIHDR* header, bool complete);

private:
    UINT m_id;
    HMIDIIN m_handle;
    QByteArray m_universe;
    uint m_mbc_counter;

    /** DMX Total Control: the buffers the SysEx messages come in (a full
     *  frame of a MIDI timecode, sent on a locate) */
    enum { SysExBuffers = 4, SysExSize = 256 };
    MIDIHDR m_sysExHeaders[SysExBuffers];
    char m_sysExData[SysExBuffers][SysExSize];
    std::atomic<bool> m_closing;
};

#endif
