/*
  DMX Total Control
  testmidiplugin.cpp

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

#include "testmidiplugin.h"
#include "outputpatch.h"

void TestMidiPlugin::init()
{
    setProperty("sent", QStringList());
}

QString TestMidiPlugin::name() const
{
    // (a MIDI plugin to the mappings: its name says MIDI)
    return QStringLiteral("Test MIDI");
}

int TestMidiPlugin::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input | QLCIOPlugin::Feedback;
}

QString TestMidiPlugin::pluginInfo() const
{
    return QStringLiteral("A MIDI plugin for the tests, without devices.");
}

bool TestMidiPlugin::openOutput(quint32 output, quint32 universe)
{
    if (output >= quint32(outputs().size()))
        return false;
    addToMap(universe, output, Output);
    note(QString("open %1").arg(output));
    return true;
}

void TestMidiPlugin::closeOutput(quint32 output, quint32 universe)
{
    removeFromMap(universe, output, Output);
}

QStringList TestMidiPlugin::outputs()
{
    // (the second port of the mk2, Note mode, is not a controller)
    return { "APC mini mk2", "APC40 mkII", "APC MINI", "MIDIIN2 (APC mini mk2)" };
}

void TestMidiPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)
    Q_UNUSED(data)
    Q_UNUSED(dataChanged)
}

bool TestMidiPlugin::openInput(quint32 input, quint32 universe)
{
    if (input >= quint32(inputs().size()))
        return false;
    addToMap(universe, input, Input);
    return true;
}

void TestMidiPlugin::closeInput(quint32 input, quint32 universe)
{
    removeFromMap(universe, input, Input);
}

QStringList TestMidiPlugin::inputs()
{
    return outputs();
}

void TestMidiPlugin::sendFeedBack(quint32 universe, quint32 output, quint32 channel, uchar value,
                                  const QVariant &params)
{
    Q_UNUSED(universe)
    Q_UNUSED(params)
    note(QString("feedback %1 %2 %3").arg(output).arg(channel).arg(value));
}

void TestMidiPlugin::setParameter(quint32 universe, quint32 line, Capability type,
                                  QString name, QVariant value)
{
    // a SysEx is sent, not kept (as the MIDI plugin does)
    if (type == Output && name == KPluginSendSysEx)
    {
        note(QString("sysex %1 %2").arg(line).arg(QString::fromLatin1(value.toByteArray().toHex().toUpper())));
        return;
    }
    if (type == Output)
        note(QString("param %1 %2=%3").arg(line).arg(name, value.toString()));
    QLCIOPlugin::setParameter(universe, line, type, name, value);
}

void TestMidiPlugin::note(const QString &what)
{
    setProperty("sent", property("sent").toStringList() << what);
}
