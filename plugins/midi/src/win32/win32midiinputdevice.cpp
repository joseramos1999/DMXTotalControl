/*
  Q Light Controller
  win32midiinputdevice.cpp

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

#include <QDebug>

#include "win32midiinputdevice.h"
#include "midiprotocol.h"

extern "C" {
static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg,
                                DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                DWORD_PTR dwParam2)
{
    Q_UNUSED(dwParam2)
    Win32MidiInputDevice* self = (Win32MidiInputDevice*) dwInstance;
    Q_ASSERT(self != NULL);

    // DMX Total Control: a SysEx message (a full frame of a MIDI timecode);
    // a broken one (MIM_LONGERROR) brings its buffer back too
    if (wMsg == MIM_LONGDATA || wMsg == MIM_LONGERROR)
    {
        self->processSysExBuffer(hMidiIn, (MIDIHDR*) dwParam1, wMsg == MIM_LONGDATA);
        return;
    }

    if (wMsg == MIM_DATA)
    {
        BYTE cmd = dwParam1 & 0xFF;
        BYTE data1 = (dwParam1 & 0xFF00) >> 8;
        BYTE data2 = (dwParam1 & 0xFF0000) >> 16;

        // DMX Total Control: a quarter frame of a MIDI timecode
        if (cmd == MIDI_TIME_CODE)
        {
            self->processQuarterFrame(data1);
            return;
        }

        if (cmd >= MIDI_BEAT_CLOCK && cmd <= MIDI_BEAT_STOP)
        {
            if (self->processMBC(cmd) == false)
                return;
        }
        if (MIDI_CMD(cmd) == MIDI_PROGRAM_CHANGE)
            data2 = 127;

        quint32 channel = 0;
        uchar value = 0;

        if (QLCMIDIProtocol::midiToInput(cmd, data1, data2,
            uchar(self->midiChannel()), &channel, &value) == true)
        {
            self->emitValueChanged(channel, value);
            // for MIDI beat clock signals,
            // generate a synthetic release event
            if (cmd >= MIDI_BEAT_CLOCK && cmd <= MIDI_BEAT_STOP)
                self->emitValueChanged(channel, 0);
        }
    }
}
}

Win32MidiInputDevice::Win32MidiInputDevice(const QVariant& uid, const QString& name, UINT id,
                                           QObject* parent)
    : MidiInputDevice(uid, name, parent)
    , m_id(id)
    , m_handle(NULL)
    , m_universe(MAX_MIDI_DMX_CHANNELS, char(0))
    , m_mbc_counter(UINT_MAX)
    , m_closing(false)
{
    qDebug() << Q_FUNC_INFO;
    ZeroMemory(m_sysExHeaders, sizeof(m_sysExHeaders));
}

Win32MidiInputDevice::~Win32MidiInputDevice()
{
    qDebug() << Q_FUNC_INFO;
}

bool Win32MidiInputDevice::open()
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle != NULL)
        return false;

    // DMX Total Control: a MIDI timecode starts again
    resetTimecode();
    m_closing = false;

    MMRESULT result = midiInOpen(&m_handle, m_id, (DWORD_PTR) MidiInProc, (DWORD_PTR) this, CALLBACK_FUNCTION);
    if (result != MMSYSERR_NOERROR)
    {
        qWarning() << Q_FUNC_INFO << "Unable to open MIDI input device with id:" << m_id
                   << "name:" << name() << ":" << result;
        m_handle = NULL;
        return false;
    }
    else
    {
        // DMX Total Control: the buffers of the SysEx messages
        for (int i = 0; i < SysExBuffers; i++)
        {
            MIDIHDR* header = &m_sysExHeaders[i];
            ZeroMemory(header, sizeof(MIDIHDR));
            header->lpData = m_sysExData[i];
            header->dwBufferLength = SysExSize;
            if (midiInPrepareHeader(m_handle, header, sizeof(MIDIHDR)) == MMSYSERR_NOERROR)
                midiInAddBuffer(m_handle, header, sizeof(MIDIHDR));
        }
        midiInStart(m_handle);
    }
    return true;
}

void Win32MidiInputDevice::close()
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle == NULL)
        return;

    MMRESULT result = midiInStop(m_handle);
    if (result != MMSYSERR_NOERROR)
        qWarning() << Q_FUNC_INFO << "Unable to stop MIDI input with id:" << m_id
                   << "name:" << name() << ":" << result;

    // DMX Total Control: the SysEx buffers come back (not to be queued
    // again) and are released before the device closes
    m_closing = true;
    midiInReset(m_handle);
    for (int i = 0; i < SysExBuffers; i++)
    {
        if (m_sysExHeaders[i].dwFlags & MHDR_PREPARED)
            midiInUnprepareHeader(m_handle, &m_sysExHeaders[i], sizeof(MIDIHDR));
    }

    result = midiInClose(m_handle);
    if (result != MMSYSERR_NOERROR)
        qWarning() << Q_FUNC_INFO << "Unable to close MIDI input with id:" << m_id
                   << "name:" << name() << ":" << result;
    m_handle = NULL;
}

bool Win32MidiInputDevice::isOpen() const
{
    if (m_handle != NULL)
        return true;
    else
        return false;
}

void Win32MidiInputDevice::processSysExBuffer(HMIDIIN handle, MIDIHDR* header, bool complete)
{
    // DMX Total Control: (a closing device gets its buffers back empty; a
    // broken message is not read, its buffer is queued again all the same)
    if (header == NULL)
        return;
    if (complete && header->dwBytesRecorded > 0)
        processSysEx(QByteArray(header->lpData, int(header->dwBytesRecorded)));
    if (m_closing == false)
        midiInAddBuffer(handle, header, sizeof(MIDIHDR));
}

bool Win32MidiInputDevice::processMBC(int type)
{
    if (type == MIDI_BEAT_START || type == MIDI_BEAT_STOP)
    {
        m_mbc_counter = 1;
        return true;
    }
    else if (type == MIDI_BEAT_CONTINUE)
    {
        return true;
    }
    else if (type == MIDI_BEAT_CLOCK)
    {
        if (m_mbc_counter == UINT_MAX)
        {
            m_mbc_counter = 1;
            return true;
        }
        m_mbc_counter++;
        if (m_mbc_counter == MIDI_BEAT_CLOCK_PPQ)
        {
            m_mbc_counter = 0;
            return true;
        }
    }
    return false;
}
