/*
  Q Light Controller Plus
  dmxusb.cpp

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

#include <QStringList>
#include <QMessageBox>
#include <QDebug>

#include "dmxusbconfig.h"
#include "dmxusbwidget.h"
#include "enttecdmxusbpro.h"
#include "enttecdmxusbopen.h"
#include "euroliteusbdmxpro.h"
#include "dmxusb.h"
#if defined(FTD2XX)
#include "ftd2xx-interface.h"
#endif
#if defined(QTSERIAL)
#include "qtserial-interface.h"
#endif

/****************************************************************************
 * Initialization
 ****************************************************************************/

DMXUSB::~DMXUSB()
{
    while (m_widgets.isEmpty() == false)
        delete m_widgets.takeFirst();
}

void DMXUSB::init()
{
    /* Search for new widgets */
    rescanWidgets();
}

QString DMXUSB::name() const
{
    return QString("DMX USB");
}

int DMXUSB::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input | QLCIOPlugin::RDM;
}

bool DMXUSB::rescanWidgets()
{
    // DMX Total Control: the old widgets leave the lists under the lock (no
    // universe writes to them any more) and are deleted outside it, closing
    // their devices before the new ones are probed. (What is present is
    // listed first: a device that comes during the probe changes it again)
    const QStringList present = presentInterfaces();
    int linesCount = m_inputs.count() + m_outputs.count();
    QList <DMXUSBWidget*> old;
    {
        QMutexLocker locker(&m_widgetsMutex);
        m_inputs.clear();
        m_outputs.clear();
        m_freshOutputs.clear();
        m_openedOutputs.clear();
        old.swap(m_widgets);
    }
    qDeleteAll(old);

    QList <DMXUSBWidget*> found = DMXUSBWidget::widgets();
    {
        QMutexLocker locker(&m_widgetsMutex);
        m_widgets = found;

        foreach (DMXUSBWidget* widget, m_widgets)
        {
            for (int o = 0; o < widget->outputsNumber(); o++)
                m_outputs.append(widget);

            for (int i = 0; i < widget->inputsNumber(); i++)
                m_inputs.append(widget);
        }
    }
#if !defined(LIBFTDI) && !defined(LIBFTDI1)
    m_present = present;
    m_presentKnown = true;
#endif

    // DMX Total Control: the lines there were are closed with their widgets:
    // their patches open them again, also when the count is the same
    if (linesCount > 0 || m_inputs.count() + m_outputs.count() != linesCount)
        emit configurationChanged();

    return true;
}

QList <DMXUSBWidget*> DMXUSB::widgets() const
{
    return m_widgets;
}

QStringList DMXUSB::presentInterfaces() const
{
    // (libFTDI lists them opening them: there every event rescans)
    QStringList present;
#if defined(FTD2XX)
    present << FTD2XXInterface::present();
#endif
#if defined(QTSERIAL)
    present << QtSerialInterface::present();
#endif
    return present;
}

/****************************************************************************
 * Outputs
 ****************************************************************************/

bool DMXUSB::openOutput(quint32 output, quint32 universe)
{
    if (output < quint32(m_outputs.size()))
    {
        DMXUSBWidget *widget = m_outputs.at(output);
        if (widget->supportRDM())
        {
            EnttecDMXUSBPro *pro = static_cast<EnttecDMXUSBPro*>(widget);
            connect(pro, SIGNAL(rdmValueChanged(quint32, quint32, QVariantMap)),
                    this, SIGNAL(rdmValueChanged(quint32, quint32, QVariantMap)), Qt::QueuedConnection);
        }
        addToMap(universe, output, Output);
        const bool opened = m_outputs.at(output)->open(output, false);
        // DMX Total Control: its first write gives the whole universe, and
        // only this universe writes there
        if (opened)
        {
            QMutexLocker locker(&m_widgetsMutex);
            m_freshOutputs.insert(output);
            m_openedOutputs.insert(output, universe);
        }
        return opened;
    }
    return false;
}

void DMXUSB::closeOutput(quint32 output, quint32 universe)
{
    if (output < quint32(m_outputs.size()))
    {
        // DMX Total Control: a line another universe opened (this one had its
        // number before a rescan) stays open for it
        bool othersLine = false;
        {
            QMutexLocker locker(&m_widgetsMutex);
            const quint32 owner = m_openedOutputs.value(output, UINT_MAX);
            othersLine = owner != UINT_MAX && owner != universe;
            if (othersLine == false)
            {
                m_openedOutputs.remove(output);
                m_freshOutputs.remove(output);
            }
        }
        if (othersLine)
        {
            removeFromMap(output, universe, Output);
            return;
        }
        DMXUSBWidget *widget = m_outputs.at(output);
        if (widget->supportRDM())
        {
            EnttecDMXUSBPro *pro = static_cast<EnttecDMXUSBPro*>(widget);
            disconnect(pro, SIGNAL(rdmValueChanged(quint32, quint32, QVariantMap)),
                       this , SIGNAL(rdmValueChanged(quint32, quint32, QVariantMap)));
        }
        removeFromMap(output, universe, Output);
        m_outputs.at(output)->close(output, false);
    }
}

QStringList DMXUSB::outputs()
{
    QStringList list;

    for (int w = 0; w < m_outputs.count();)
    {
        DMXUSBWidget* widget = m_outputs.at(w);
        foreach (QString name, widget->outputNames())
            list << name;
        w += widget->outputsNumber();
    }
    return list;
}

QString DMXUSB::pluginInfo() const
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides DMX output support for");
    str += QString(" DMXKing ultraDMX range, Enttec DMX USB Pro, "
                   "Enttec Open DMX USB, FTDI USB COM485 Plus1, "
                   "Vince USB-DMX512, usbdmx.com (legacy) ");
    str += tr("and compatible devices.");
    str += QString("</P>");

    return str;
}

QString DMXUSB::outputInfo(quint32 output)
{
    QString str;

    if (output == QLCIOPlugin::invalidLine())
    {
        if (m_outputs.size() == 0)
        {
            str += QString("<BR><B>%1</B>").arg(tr("No output support available."));
            str += QString("<P>");
            str += tr("Make sure that you have your hardware firmly plugged in. "
                      "NOTE: FTDI VCP interface is not supported by this plugin.");
            str += QString("</P>");
        }
    }
    else if (output < quint32(m_outputs.size()))
    {
        str += QString("<H3>%1</H3>").arg(outputs()[output]);
        str += QString("<P>");
        str += tr("Device is operating correctly.");
        str += QString("<BR>");
        str += tr("Driver in use: %1").arg(m_outputs[output]->interfaceTypeString());
        str += QString("</P>");
        QString add = m_outputs[output]->additionalInfo();
        if (add.isEmpty() == false)
            str += add;
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void DMXUSB::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    // (from the thread of the universe: not while a rescan replaces the widgets)
    QMutexLocker locker(&m_widgetsMutex);
    // (a universe that didn't open this line: one that still has the number
    // the line had before a rescan)
    if (output < quint32(m_outputs.size()) && m_openedOutputs.value(output, UINT_MAX) == universe)
    {
        // (just opened: the whole universe, even if it didn't change)
        const bool fresh = m_freshOutputs.remove(output);
        m_outputs.at(output)->writeUniverse(universe, output, data, dataChanged || fresh);
    }
}

/****************************************************************************
 * Inputs
 ****************************************************************************/

bool DMXUSB::openInput(quint32 input, quint32 universe)
{
    if (input < quint32(m_inputs.size()))
    {
        DMXUSBWidget *widget = m_inputs.at(input);
        if (widget->type() == DMXUSBWidget::ProRXTX ||
            widget->type() == DMXUSBWidget::ProMk2 ||
            widget->type() == DMXUSBWidget::OpenRX ||
            widget->type() == DMXUSBWidget::UltraPro)
        {
            EnttecDMXUSBPro *pro = static_cast<EnttecDMXUSBPro*>(widget);
            connect(pro, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
                    this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)));
        }
        addToMap(universe, input, Input);
        return widget->open(input, true);
    }
    return false;
}

void DMXUSB::closeInput(quint32 input, quint32 universe)
{
    if (input < quint32(m_inputs.size()))
    {
        DMXUSBWidget *widget = m_inputs.at(input);
        removeFromMap(input, universe, Input);
        widget->close(input, true);
        if (widget->type() == DMXUSBWidget::ProRXTX ||
            widget->type() == DMXUSBWidget::ProMk2 ||
            widget->type() == DMXUSBWidget::UltraPro)
        {
            EnttecDMXUSBPro* pro = (EnttecDMXUSBPro*) widget;
            disconnect(pro, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
                       this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)));
        }
    }
}

QStringList DMXUSB::inputs()
{
    QStringList list;

    for (int w = 0; w < m_inputs.count();)
    {
        DMXUSBWidget* widget = m_inputs.at(w);
        foreach (QString name, widget->inputNames())
            list << name;
        w += widget->inputsNumber();
    }

    return list;
}

QString DMXUSB::inputInfo(quint32 input)
{
    QString str;

    if (input == QLCIOPlugin::invalidLine())
    {
        if (m_inputs.size() == 0)
        {
            str += QString("<BR><B>%1</B>").arg(tr("No input support available."));
            /*
            str += QString("<P>");
            str += tr("Make sure that you have your hardware firmly plugged in. "
                      "NOTE: FTDI VCP interface is not supported by this plugin.");
            str += QString("</P>");
            */
        }
    }
    else if (input < quint32(m_inputs.size()))
    {
        str += QString("<H3>%1</H3>").arg(inputs()[input]);
        str += QString("<P>");
        str += tr("Device is operating correctly.");
        str += QString("<BR>");
        str += tr("Driver in use: %1").arg(m_inputs[input]->interfaceTypeString());
        str += QString("</P>");
        QString add = m_inputs[input]->additionalInfo();
        if (add.isEmpty() == false)
            str += add;
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

/****************************************************************************
 * Configuration
 ****************************************************************************/

void DMXUSB::configure()
{
    qDebug() << Q_FUNC_INFO;
    DMXUSBConfig config(this);
    config.exec();
    rescanWidgets();
    emit configurationChanged();
}

bool DMXUSB::canConfigure() const
{
    return true;
}

bool DMXUSB::sendRDMCommand(quint32 universe, quint32 line, uchar command, QVariantList params)
{
    if (line < quint32(m_outputs.size()))
    {
        DMXUSBWidget *widget = m_outputs.at(line);
        if (widget->supportRDM())
            return widget->sendRDMCommand(universe, line, command, params);
    }

    return false;
}

/*****************************************************************************
 * Hotplug
 *****************************************************************************/

void DMXUSB::slotDeviceAdded(uint vid, uint pid)
{
    qDebug() << Q_FUNC_INFO << QString::number(vid, 16) << QString::number(pid, 16);
    if (!DMXInterface::validInterface(vid, pid))
    {
        qDebug() << Q_FUNC_INFO << "Invalid DMX USB device, nothing to do";
        return;
    }
    // DMX Total Control: the same interfaces present: nothing to close and open again
    if (m_presentKnown && presentInterfaces() == m_present)
    {
        qDebug() << Q_FUNC_INFO << "The same DMX USB interfaces, nothing to do";
        return;
    }

    rescanWidgets();
}

void DMXUSB::slotDeviceRemoved(uint vid, uint pid)
{
    qDebug() << Q_FUNC_INFO << QString::number(vid, 16) << QString::number(pid, 16);
    if (!DMXInterface::validInterface(vid, pid))
    {
        qDebug() << Q_FUNC_INFO << "Invalid DMX USB device, nothing to do";
        return;
    }
    if (m_presentKnown && presentInterfaces() == m_present)
    {
        qDebug() << Q_FUNC_INFO << "The same DMX USB interfaces, nothing to do";
        return;
    }

    rescanWidgets();
}
