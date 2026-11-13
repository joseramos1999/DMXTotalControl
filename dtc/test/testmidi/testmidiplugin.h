/*
  DMX Total Control
  testmidiplugin.h

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

#ifndef TESTMIDIPLUGIN_H
#define TESTMIDIPLUGIN_H

#include <QStringList>

#include "qlcioplugin.h"

/**
 * A MIDI plugin for the tests, without any device: its lines (inputs and
 * outputs) are named like the known controllers, and it notes what it gets
 * in its dynamic property "sent" (a QStringList, newest last):
 *   "open <line>"                  an output opened
 *   "sysex <line> <HEX>"           a SysEx (OutputPatch::sendSysEx)
 *   "feedback <line> <channel> <value>"
 *   "param <line> <name>=<value>"  an output parameter (kept, as QLCIOPlugin does)
 */
class TestMidiPlugin final : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)

public:
    void init() override;
    QString name() const override;
    int capabilities() const override;
    QString pluginInfo() const override;

    bool openOutput(quint32 output, quint32 universe) override;
    void closeOutput(quint32 output, quint32 universe) override;
    QStringList outputs() override;
    void writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged) override;

    bool openInput(quint32 input, quint32 universe) override;
    void closeInput(quint32 input, quint32 universe) override;
    QStringList inputs() override;

    void sendFeedBack(quint32 universe, quint32 output, quint32 channel, uchar value,
                      const QVariant &params) override;
    void setParameter(quint32 universe, quint32 line, Capability type,
                      QString name, QVariant value) override;

private:
    void note(const QString &what);
};

#endif
