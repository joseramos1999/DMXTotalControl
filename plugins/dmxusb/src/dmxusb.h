/*
  Q Light Controller Plus
  dmxusb.h

  Copyright (C) Heikki Junnila
  Copyright (C) Massimo Callegari

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

#ifndef DMXUSB_H
#define DMXUSB_H

#include <QMutex>
#include <QSet>
#include <QHash>

#include "qlcioplugin.h"

class DMXUSBWidget;

class DMXUSB final : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /** @reimp */
    virtual ~DMXUSB();

    /** @reimp */
    void init() override;

    /** @reimp */
    QString name() const override;

    /** @reimp */
    int capabilities() const override;

    /** @reimp */
    QString pluginInfo() const override;

    /** Find out what kinds of widgets there are currently connected */
    Q_INVOKABLE bool rescanWidgets();

    /** Get currently connected widgets (input & output) */
    QList <DMXUSBWidget*> widgets() const;

private:
    /** List of references to the discovered USB widgets */
    QList <DMXUSBWidget*> m_widgets;

    /** DMX Total Control: the universes write from their own threads while
     *  a rescan (a hot plugged device) replaces the widgets: the lists of
     *  widgets and lines change under this lock, and writeUniverse() reads
     *  them under it */
    QMutex m_widgetsMutex;

    /** DMX Total Control: the outputs opened since their last write: the
     *  first one gives the whole universe (a widget made by a rescan starts
     *  dark, and a look that doesn't change would never reach it) */
    QSet <quint32> m_freshOutputs;

    /** DMX Total Control: the universe that opened each output line: only
     *  that one writes there (after a rescan renumbers the lines, a patch
     *  that still has the old number until it reconnects writes nowhere) */
    QHash <quint32, quint32> m_openedOutputs;

    /** DMX Total Control: the interfaces present at the last rescan, listed
     *  without opening any: a hot plug event that changes nothing (the same
     *  device twice, its driver still loading, another USB device) doesn't
     *  close and open the lines again. Empty where they can't be listed so */
    QStringList presentInterfaces() const;
    QStringList m_present;
    bool m_presentKnown = false;

    /************************************************************************
     * Outputs
     ************************************************************************/
public:
    /** @reimp */
    bool openOutput(quint32 output, quint32 universe) override;

    /** @reimp */
    void closeOutput(quint32 output, quint32 universe) override;

    /** @reimp */
    QStringList outputs() override;

    /** @reimp */
    QString outputInfo(quint32 output) override;

    /** @reimp */
    void writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged) override;

private:
    /**
     *  List of references to USB widgets ordered by output lines.
     *  If a widget has multiple outputs, it will appear in this
     *  list multiple times
     */
    QList <DMXUSBWidget*> m_outputs;

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    bool openInput(quint32 input, quint32 universe) override;

    /** @reimp */
    void closeInput(quint32 input, quint32 universe) override;

    /** @reimp */
    QStringList inputs() override;

    /** @reimp */
    QString inputInfo(quint32 input) override;

private:
    /**
     *  List of references to USB widgets ordered by input lines.
     *  If a widget has multiple inputs, it will appear in this
     *  list multiple times
     */
    QList <DMXUSBWidget*> m_inputs;

    /********************************************************************
     * Configuration
     ********************************************************************/
public:
    /** @reimp */
    void configure() override;

    /** @reimp */
    bool canConfigure() const override;

    /********************************************************************
     * RDM
     ********************************************************************/
public:
    /** @reimp */
    bool sendRDMCommand(quint32 universe, quint32 line, uchar command, QVariantList params) override;

signals:
    void rdmValueChanged(quint32 universe, quint32 line, QVariantMap data);

    /********************************************************************
     * Hotplug
     ********************************************************************/
public slots:
    void slotDeviceAdded(uint vid, uint pid);
    void slotDeviceRemoved(uint vid, uint pid);
};

#endif
