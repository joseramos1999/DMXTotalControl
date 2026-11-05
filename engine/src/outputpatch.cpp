/*
  Q Light Controller
  outputpatch.cpp

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

#if defined(WIN32) || defined(Q_OS_WIN)
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#else
#   include <unistd.h>
#endif

#include <QStringList>
#include <QVariant>

#include "qlcioplugin.h"
#include "outputpatch.h"

#define GRACE_MS 1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

OutputPatch::OutputPatch(QObject* parent)
    : QObject(parent)
    , m_plugin(NULL)
    , m_pluginLine(QLCIOPlugin::invalidLine())
    , m_universe(UINT_MAX)
    , m_lineNameIndex(0)
    , m_lineGone(false)
    , m_paused(false)
    , m_blackout(false)
{
}

OutputPatch::OutputPatch(quint32 universe, QObject* parent)
    : QObject(parent)
    , m_plugin(NULL)
    , m_pluginLine(QLCIOPlugin::invalidLine())
    , m_universe(universe)
    , m_lineNameIndex(0)
    , m_lineGone(false)
    , m_paused(false)
    , m_blackout(false)
{
}

OutputPatch::~OutputPatch()
{
    // (a line gone isn't this patch's: its number may be another one's now)
    if (m_plugin != NULL && m_lineGone == false)
        m_plugin->closeOutput(m_pluginLine, m_universe);
}

/****************************************************************************
 * Plugin & Output
 ****************************************************************************/

bool OutputPatch::set(QLCIOPlugin* plugin, quint32 output, const QString &expectedName)
{
    if (m_plugin != NULL && m_pluginLine != QLCIOPlugin::invalidLine() && m_lineGone == false)
        m_plugin->closeOutput(m_pluginLine, m_universe);

    m_plugin = plugin;
    m_pluginLine = output;
    m_lineGone = false;

    // DMX Total Control: its interface isn't there (a show opened before it
    // was plugged in): it waits for it by its name, not on the line that
    // has its number
    if (m_plugin != NULL && m_pluginLine != QLCIOPlugin::invalidLine() &&
        m_plugin->property(KPluginLinesByName).toBool() &&
        expectedName.isEmpty() == false && expectedName != KOutputNone &&
        m_plugin->outputs().contains(expectedName) == false)
    {
        m_lineName = expectedName;
        m_lineNameIndex = 0;
        m_lineGone = true;
        emit pluginNameChanged();
        emit outputNameChanged();
        return false;
    }
    rememberLineName();

    if (m_plugin != NULL)
    {
        emit pluginNameChanged();
        if (m_pluginLine != QLCIOPlugin::invalidLine())
            emit outputNameChanged();
    }

    if (m_plugin != NULL && m_pluginLine != QLCIOPlugin::invalidLine())
        return m_plugin->openOutput(m_pluginLine, m_universe);

    return false;
}

void OutputPatch::rememberLineName()
{
    m_lineName.clear();
    m_lineNameIndex = 0;
    if (m_plugin == NULL || m_pluginLine == QLCIOPlugin::invalidLine())
        return;

    const QStringList lines = m_plugin->outputs();
    if (m_pluginLine >= quint32(lines.size()))
        return;
    m_lineName = lines.at(m_pluginLine);
    for (quint32 i = 0; i < m_pluginLine; i++)
    {
        if (lines.at(i) == m_lineName)
            m_lineNameIndex++;
    }
}

bool OutputPatch::lineGone() const
{
    return m_lineGone;
}

bool OutputPatch::reconnect()
{
    if (m_plugin != NULL && m_pluginLine != QLCIOPlugin::invalidLine())
    {
        // DMX Total Control: a plugin that numbers its lines anew when it
        // rescans: the line of this patch by its name (the same one of the
        // lines with that name), never the one that took its number. Not
        // there (unplugged), it waits, writing nowhere
        if (m_plugin->property(KPluginLinesByName).toBool() && m_lineName.isEmpty() == false)
        {
            const QStringList lines = m_plugin->outputs();
            int found = -1;
            for (int i = 0, same = 0; i < lines.size() && found < 0; i++)
            {
                if (lines.at(i) == m_lineName && same++ == m_lineNameIndex)
                    found = i;
            }
            const bool wasGone = m_lineGone;
            m_lineGone = found < 0;
            if (m_lineGone)
            {
                if (wasGone == false)
                    emit outputNameChanged();
                return false;
            }
            m_pluginLine = quint32(found);
            if (wasGone)
                emit outputNameChanged();
        }

        m_plugin->closeOutput(m_pluginLine, m_universe);
#if defined(WIN32) || defined(Q_OS_WIN)
        Sleep(GRACE_MS);
#else
        usleep(GRACE_MS * 1000);
#endif
        bool ret = m_plugin->openOutput(m_pluginLine, m_universe);
        if (ret == true)
        {
            QMap<QString, QVariant>::iterator it = m_parametersCache.begin();
            for (; it != m_parametersCache.end(); it++)
                m_plugin->setParameter(m_universe, m_pluginLine, QLCIOPlugin::Output, it.key(), it.value());
            // (DMX Total Control: one set while its interface wasn't there
            // learns the name of its line now)
            if (m_lineName.isEmpty())
                rememberLineName();
        }
        return ret;
    }
    return false;
}

QString OutputPatch::pluginName() const
{
    if (m_plugin != NULL)
        return m_plugin->name();
    else
        return KOutputNone;
}

QLCIOPlugin* OutputPatch::plugin() const
{
    return m_plugin;
}

QString OutputPatch::outputName() const
{
    // (a line gone: the one it waits for, also saved with the show)
    if (m_lineGone)
        return m_lineName;
    if (m_plugin != NULL && m_pluginLine != QLCIOPlugin::invalidLine() &&
        m_pluginLine < quint32(m_plugin->outputs().size()))
    {
        return m_plugin->outputs()[m_pluginLine];
    }
    else
    {
        return KOutputNone;
    }
}

QString OutputPatch::outputUID() const
{
    if (m_lineGone)
        return QString();
    if (m_plugin != NULL && m_pluginLine != QLCIOPlugin::invalidLine() &&
        m_pluginLine < quint32(m_plugin->outputs().size()))
    {
        return m_plugin->outputsUID()[m_pluginLine];
    }
    else
    {
        return KOutputNone;
    }
}

quint32 OutputPatch::output() const
{
    return m_pluginLine;
}

bool OutputPatch::isPatched() const
{
    return output() != QLCIOPlugin::invalidLine() && m_plugin != NULL;
}

void OutputPatch::setPluginParameter(QString prop, QVariant value)
{
    m_parametersCache[prop] = value;
    // (a line gone: it gets them when it comes back)
    if (m_plugin != NULL && m_lineGone == false)
        m_plugin->setParameter(m_universe, m_pluginLine, QLCIOPlugin::Output, prop, value);
}

QMap<QString, QVariant> OutputPatch::getPluginParameters()
{
    if (m_lineGone)
        return m_parametersCache;
    if (m_plugin != NULL)
        return m_plugin->getParameters(m_universe, m_pluginLine, QLCIOPlugin::Output);

    return QMap<QString, QVariant>();
}

/*****************************************************************************
 * Value dump
 *****************************************************************************/
bool OutputPatch::paused() const
{
    return m_paused;
}

void OutputPatch::setPaused(bool paused)
{
    if (m_paused == paused)
        return;

    m_paused = paused;

    if (m_pauseBuffer.length())
        m_pauseBuffer.clear();

    emit pausedChanged(m_paused);
}

bool OutputPatch::blackout() const
{
    return m_blackout;
}

void OutputPatch::setBlackout(bool blackout)
{
    if (m_blackout == blackout)
        return;

    m_blackout = blackout;
    emit blackoutChanged(m_blackout);
}

void OutputPatch::dump(quint32 universe, const QByteArray& data, bool dataChanged)
{
    /* Don't do anything if there is no plugin and/or output line. */
    // (DMX Total Control: nor while its line is gone: its number may be
    // another interface's now)
    if (m_plugin != NULL && m_pluginLine != QLCIOPlugin::invalidLine() && m_lineGone == false)
    {
        if (m_paused)
        {
            if (m_pauseBuffer.isNull())
                m_pauseBuffer.append(data);

            m_plugin->writeUniverse(universe, m_pluginLine, m_pauseBuffer, dataChanged);
        }
        else
        {
            m_plugin->writeUniverse(universe, m_pluginLine, data, dataChanged);
        }
    }
}
