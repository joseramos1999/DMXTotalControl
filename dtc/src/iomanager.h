/*
  DMX Total Control
  iomanager.h

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

#ifndef IOMANAGER_H
#define IOMANAGER_H

#include <QObject>
#include <QVariant>
#include <QByteArray>
#include <QHash>

class InputOutputMap;
class Doc;

/**
 * Universes and output patching. Outputs come from the QLC+ I/O plugins,
 * so every interface QLC+ supports (DMX USB, uDMX, Art-Net, sACN...) is
 * available here.
 */
class IOManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList universes READ universes NOTIFY universesChanged)
    Q_PROPERTY(QVariantList outputs READ outputs NOTIFY outputsChanged)

public:
    IOManager(Doc *doc, QObject *parent = nullptr);

    /** [{ index, name, plugin, line, outputName }] */
    QVariantList universes() const;

    /** Every output line of every plugin: [{ plugin, line, name, key }] */
    QVariantList outputs() const;

    Q_INVOKABLE void setOutput(int universe, const QString &plugin, int line);
    Q_INVOKABLE void clearOutput(int universe);
    Q_INVOKABLE void setUniverseName(int universe, const QString &name);

    Q_INVOKABLE void addUniverse();
    /** What uses a universe: { fixtures: how many fixtures have channels in
     *  it, devices: the names of the controllers whose input it is } */
    Q_INVOKABLE QVariantMap universeUse(int universe) const;
    /** Remove the last universe (never the only one). Refused while fixtures
     *  are patched in it: they would be left on a universe that doesn't exist
     *  (and the engine writes to the universe of each fixture). A controller
     *  whose input it is turns its LEDs off first, and is disabled.
     *  @return whether it was removed */
    Q_INVOKABLE bool removeLastUniverse();

    /** "Rescan": the DMX USB plugin probes its interfaces again (one plugged
     *  in since, one that didn't answer the first time), and the lists are
     *  read again. The DMX USB lines open are closed and opened again by
     *  their patches (a moment), each one on its interface */
    Q_INVOKABLE void refresh();

    /** Last output values of a universe (512 values, after the grand master) */
    Q_INVOKABLE QVariantList channelValues(int universe) const;
    /** The same as bytes: always 512, zeros where nothing was written (the
     *  DMX levels of the remote control) */
    QByteArray lastValues(int universe) const;

signals:
    void universesChanged();
    void outputsChanged();
    /** The last universe goes now: what it patches can still be reached */
    void universeAboutToBeRemoved(int universe);

public slots:
    /** A USB device plugged in or out (HotPlugMonitor; on Windows the window
     *  passes it the device changes, main.cpp): the DMX USB plugin looks
     *  again, if it is one of its interfaces. Only that plugin: the MIDI one
     *  would open its controllers again at every USB device */
    void slotDeviceAdded(uint vid, uint pid);
    void slotDeviceRemoved(uint vid, uint pid);

public:
    /** A DMX USB interface that came since the plugin's first search (at the
     *  start, before the window was told of the devices: main.cpp) */
    void lookForInterfaces();

private slots:
    /** A universe added or removed (a show loading: told once it is loaded) */
    void slotUniversesChanged();
    void slotPluginConfigurationChanged();
    void slotUniverseWritten(quint32 universe, const QByteArray &data);

private:
    void saveDefaults();
    /** The DMX USB plugin told of a device ($added or removed); false
     *  without the plugin */
    bool tellDmxUsb(uint vid, uint pid, bool added);
    /** Its inputs and outputs, to see whether a rescan found something */
    int dmxUsbLines() const;

private:
    Doc *m_doc;
    InputOutputMap *m_ioMap;
    QHash<quint32, QByteArray> m_lastValues;
};

#endif
