/*
  DMX Total Control
  iomanager.cpp

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

#include "iomanager.h"
#include "inputoutputmap.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "qlcioplugin.h"
#include "fixture.h"
#include "doc.h"

IOManager::IOManager(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_ioMap(doc->inputOutputMap())
{
    connect(m_ioMap, SIGNAL(universeAdded(quint32)), this, SIGNAL(universesChanged()));
    connect(m_ioMap, SIGNAL(universeRemoved(quint32)), this, SIGNAL(universesChanged()));
    // opening a show adds each universe before it reads its name and patches:
    // they are read again once the show is loaded
    connect(m_doc, SIGNAL(loaded()), this, SIGNAL(universesChanged()));
    // Hot plugged devices make the plugins reconfigure themselves
    connect(m_ioMap, SIGNAL(pluginConfigurationChanged(QString,bool)),
            this, SLOT(slotPluginConfigurationChanged()));
    connect(m_ioMap, SIGNAL(universeWritten(quint32,QByteArray)),
            this, SLOT(slotUniverseWritten(quint32,QByteArray)));
}

QVariantList IOManager::universes() const
{
    QVariantList list;
    const QStringList names = m_ioMap->universeNames();

    for (quint32 i = 0; i < m_ioMap->universesCount(); i++)
    {
        QVariantMap info;
        info.insert("index", i);
        // the name the engine gives a universe is shown in the language of the program
        const QString defaultName = QString("Universe %1").arg(i + 1);
        const QString name = i < quint32(names.count()) ? names.at(i) : defaultName;
        info.insert("name", name == defaultName ? tr("Universe %1").arg(i + 1) : name);

        OutputPatch *patch = m_ioMap->outputPatch(i);
        if (patch != nullptr && patch->output() != QLCIOPlugin::invalidLine())
        {
            info.insert("plugin", patch->pluginName());
            info.insert("line", patch->output());
            info.insert("outputName", patch->outputName());
        }
        else
        {
            info.insert("plugin", QString());
            info.insert("line", -1);
            info.insert("outputName", QString());
        }
        list.append(info);
    }
    return list;
}

QVariantList IOManager::outputs() const
{
    QVariantList list;
    const QStringList plugins = m_ioMap->outputPluginNames();

    for (const QString &plugin : plugins)
    {
        const QStringList lines = m_ioMap->pluginOutputs(plugin);
        for (int line = 0; line < lines.count(); line++)
        {
            QVariantMap info;
            info.insert("plugin", plugin);
            info.insert("line", line);
            info.insert("name", lines.at(line));
            info.insert("key", QString("%1|%2").arg(plugin).arg(line));
            list.append(info);
        }
    }
    return list;
}

void IOManager::setOutput(int universe, const QString &plugin, int line)
{
    if (universe < 0 || quint32(universe) >= m_ioMap->universesCount())
        return;

    m_ioMap->setOutputPatch(universe, plugin, QString(), QString(), quint32(line));
    saveDefaults();
    emit universesChanged();
}

void IOManager::clearOutput(int universe)
{
    if (universe < 0 || quint32(universe) >= m_ioMap->universesCount())
        return;

    m_ioMap->setOutputPatch(universe, KOutputNone, QString(), QString(), QLCIOPlugin::invalidLine());
    saveDefaults();
    emit universesChanged();
}

void IOManager::setUniverseName(int universe, const QString &name)
{
    if (universe < 0 || quint32(universe) >= m_ioMap->universesCount())
        return;

    m_ioMap->setUniverseName(universe, name);
    m_doc->setModified();
    emit universesChanged();
}

void IOManager::addUniverse()
{
    m_ioMap->addUniverse();
    m_ioMap->startUniverses();
    m_doc->setModified();
    emit universesChanged();
}

QVariantMap IOManager::universeUse(int universe) const
{
    int fixtures = 0;
    QStringList devices;
    if (universe >= 0 && quint32(universe) < m_ioMap->universesCount())
    {
        // (a fixture can go on into the next universe)
        for (Fixture *fixture : m_doc->fixtures())
        {
            const quint32 first = fixture->universe();
            const quint32 last = qMax(first, (fixture->universeAddress() + qMax(fixture->channels(), quint32(1)) - 1) >> 9);
            if (quint32(universe) >= first && quint32(universe) <= last)
                fixtures++;
        }
        InputPatch *input = m_ioMap->inputPatch(quint32(universe));
        if (input != nullptr && input->isPatched())
            devices.append(input->inputName());
    }
    return { { "fixtures", fixtures }, { "devices", devices } };
}

bool IOManager::removeLastUniverse()
{
    const quint32 count = m_ioMap->universesCount();
    if (count <= 1)
        return false;
    const int last = int(count) - 1;

    // its fixtures would be left on a universe that doesn't exist (nor beyond it)
    for (Fixture *fixture : m_doc->fixtures())
    {
        const quint32 end = (fixture->universeAddress() + qMax(fixture->channels(), quint32(1)) - 1) >> 9;
        if (fixture->universe() >= quint32(last) || end >= quint32(last))
            return false;
    }

    // the LEDs of its controller go off while they can still hear us
    emit universeAboutToBeRemoved(last);
    m_ioMap->removeUniverse(last);
    m_lastValues.remove(quint32(last));
    m_doc->setModified();
    emit universesChanged();
    return true;
}

void IOManager::refresh()
{
    emit outputsChanged();
    emit universesChanged();
}

void IOManager::slotUniverseWritten(quint32 universe, const QByteArray &data)
{
    m_lastValues[universe] = data;
}

QVariantList IOManager::channelValues(int universe) const
{
    QByteArray data = m_lastValues.value(quint32(universe));
    QVariantList values;
    values.reserve(512);
    for (int i = 0; i < 512; i++)
        values.append(i < data.size() ? int(uchar(data.at(i))) : 0);
    return values;
}

void IOManager::slotPluginConfigurationChanged()
{
    emit outputsChanged();
    emit universesChanged();
}

void IOManager::saveDefaults()
{
    // Remember the patch for the next session
    m_ioMap->saveDefaults();
    m_doc->setModified();
}
