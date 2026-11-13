/*
  Q Light Controller
  midiinputdevice.cpp

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
#include "midiinputdevice.h"
#include "midiprotocol.h"

MidiInputDevice::MidiInputDevice(const QVariant& uid, const QString& name, QObject* parent)
    : MidiDevice(uid, name, Input, parent)
    , m_mtcSerial(0)
{
    //qDebug() << Q_FUNC_INFO;
}

MidiInputDevice::~MidiInputDevice()
{
    //qDebug() << Q_FUNC_INFO;
}

void MidiInputDevice::emitValueChanged(uint channel, uchar value)
{
    emit valueChanged(uid(), channel, value);
}

/****************************************************************************
 * DMX Total Control: MIDI timecode
 ****************************************************************************/

void MidiInputDevice::processQuarterFrame(uchar data)
{
    if (m_mtc.feedQuarterFrame(data))
        emitTimecode();
}

void MidiInputDevice::processSysEx(const QByteArray& message)
{
    if (m_mtc.feedFullFrame(message))
        emitTimecode();
}

void MidiInputDevice::resetTimecode()
{
    m_mtc.reset();
}

void MidiInputDevice::emitTimecode()
{
    // (1 to 254: never 0, which the input would take as a release)
    m_mtcSerial = (m_mtcSerial + 1) % 254;
    emit keyValueChanged(uid(), CHANNEL_OFFSET_MTC, uchar(1 + m_mtcSerial),
                         QStringLiteral("mtc:%1:%2").arg(m_mtc.ms()).arg(m_mtc.fps()));
}
