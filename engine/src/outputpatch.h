/*
  Q Light Controller
  outputpatch.h

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

#ifndef OUTPUTPATCH_H
#define OUTPUTPATCH_H

#include <QObject>
#include <QMap>

class QLCIOPlugin;

/** @addtogroup engine Engine
 * @{
 */

#define KOutputNone QObject::tr("None")

/** DMX Total Control: a plugin with this property (true) numbers its lines
 *  anew when it rescans its devices (DMX USB: an interface plugged in or
 *  out): its patches find their line again by its name, never the line that
 *  took its number, and wait while it isn't there */
#define KPluginLinesByName "linesByName"

#define KXMLQLCOutputPatch          QStringLiteral("Patch")
#define KXMLQLCOutputPatchUniverse  QStringLiteral("Universe")
#define KXMLQLCOutputPatchPlugin    QStringLiteral("Plugin")
#define KXMLQLCOutputPatchOutput    QStringLiteral("Output")

class OutputPatch : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputPatch)

    Q_PROPERTY(QString outputName READ outputName NOTIFY outputNameChanged)
    Q_PROPERTY(QString pluginName READ pluginName NOTIFY pluginNameChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(bool blackout READ blackout WRITE setBlackout NOTIFY blackoutChanged)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    OutputPatch(QObject* parent = 0);
    OutputPatch(quint32 universe, QObject* parent = 0);
    virtual ~OutputPatch();

    /********************************************************************
     * Plugin & output
     ********************************************************************/
public:
    /**
     * Set the plugin to use and the plugin line number to output data on.
     * DMX Total Control: $expectedName, the name of the line saved with it
     * (a show, the defaults): with a plugin with KPluginLinesByName, a line
     * of that name that isn't there is waited for (lineGone()), and the
     * line with that number isn't opened
     */
    bool set(QLCIOPlugin* plugin, quint32 output, const QString &expectedName = QString());

    /**
     * If a valid plugin and line have been set, close
     * the output line and re-open it again
     */
    bool reconnect();

    /** The plugin instance that has been assigned to a patch */
    QLCIOPlugin* plugin() const;

    /** Friendly name of the plugin assigned to a patch ("None" if none) */
    QString pluginName() const;

    /** An output line provided by the assigned plugin */
    quint32 output() const;

    /** Friendly name of the assigned output line */
    QString outputName() const;

    /** Stable UID of the assigned output line, used for project XML persistence */
    QString outputUID() const;

    /** Returns true if a valid plugin line has been set */
    bool isPatched() const;

    /** Set a parameter specific to the patched plugin */
    void setPluginParameter(QString prop, QVariant value);

    /** Retrieve the map of custom parameters set to the patched plugin */
    QMap<QString, QVariant> getPluginParameters();

signals:
    void outputNameChanged();
    void pluginNameChanged();

public:
    /** DMX Total Control: its line is gone (a plugin with KPluginLinesByName
     *  rescanned without it: an interface unplugged). It writes nowhere, and
     *  outputName() is the name of that line, until reconnect() finds it */
    bool lineGone() const;

private:
    /** The reference of the plugin associated by this Output patch */
    QLCIOPlugin* m_plugin;
    /** The plugin line open by this Output patch */
    quint32 m_pluginLine;
    /** The universe that this Output patch is attached to */
    quint32 m_universe;
    /** The patch parameters cache */
    QMap<QString, QVariant>m_parametersCache;

    /** DMX Total Control: the name of the line, and which one of the lines
     *  with that name it is (identical interfaces), for reconnect() */
    QString m_lineName;
    int m_lineNameIndex;
    bool m_lineGone;
    void rememberLineName();

    /********************************************************************
     * Value dump
     ********************************************************************/
public:
    /** Get/Set the output patch pause state */
    bool paused() const;
    void setPaused(bool paused);

    /** Get/Set the output patch blackout state */
    bool blackout() const;
    void setBlackout(bool blackout);

    /** Write the contents of a 512 channel value buffer to the plugin.
      * Called periodically by OutputMap. No need to call manually. */
    void dump(quint32 universe, const QByteArray &data, bool dataChanged);

signals:
    void pausedChanged(bool paused);
    void blackoutChanged(bool blackout);

private:
    /** A buffer used when this output patch is paused */
    QByteArray m_pauseBuffer;
    bool m_paused;
    bool m_blackout;
};

/** @} */

#endif
