/*
  Q Light Controller - Unit test
  outputpatch_test.cpp

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

#include <QtTest>

#define private public
#include "iopluginstub.h"
#include "outputpatch_test.h"
#include "outputpatch.h"
#include "qlcfile.h"
#include "doc.h"
#undef private

#define TESTPLUGINDIR "../iopluginstub"

static QDir testPluginDir()
{
    QDir dir(TESTPLUGINDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtPlugin));
    return dir;
}

void OutputPatch_Test::initTestCase()
{
    m_doc = new Doc(this);
    m_doc->ioPluginCache()->load(testPluginDir());
    QVERIFY(m_doc->ioPluginCache()->plugins().size() != 0);
}

void OutputPatch_Test::cleanupTestCase()
{
    delete m_doc;
    m_doc = NULL;
}

void OutputPatch_Test::defaults()
{
    OutputPatch op(this);
    QVERIFY(op.m_plugin == NULL);
    QVERIFY(op.m_pluginLine == QLCIOPlugin::invalidLine());
    QVERIFY(op.pluginName() == KOutputNone);
    QVERIFY(op.outputName() == KOutputNone);
}

void OutputPatch_Test::patch()
{
    InputOutputMap om(m_doc, 4);

    IOPluginStub* stub = static_cast<IOPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    OutputPatch* op = new OutputPatch(0, this);
    op->set(stub, 0);
    QVERIFY(op->m_plugin == stub);
    QVERIFY(op->m_pluginLine == 0);
    QVERIFY(op->pluginName() == stub->name());
    QVERIFY(op->outputName() == stub->outputs()[0]);
    QVERIFY(stub->m_openOutputs.size() == 1);
    QVERIFY(stub->m_openOutputs.at(0) == 0);

    op->set(stub, 3);
    QVERIFY(op->m_plugin == stub);
    QVERIFY(op->m_pluginLine == 3);
    QVERIFY(op->pluginName() == stub->name());
    QVERIFY(op->outputName() == stub->outputs()[3]);
    QVERIFY(stub->m_openOutputs.size() == 1);
    QVERIFY(stub->m_openOutputs.at(0) == 3);

    op->reconnect();
    QVERIFY(op->m_plugin == stub);
    QVERIFY(op->m_pluginLine == 3);
    QVERIFY(op->pluginName() == stub->name());
    QVERIFY(op->outputName() == stub->outputs()[3]);
    QVERIFY(stub->m_openOutputs.size() == 1);
    QVERIFY(stub->m_openOutputs.at(0) == 3);

    delete op;
    QVERIFY(stub->m_openOutputs.size() == 0);
}

void OutputPatch_Test::dump()
{
    QByteArray uni(513, char(0));
    uni[0] = 100;
    uni[169] = 50;
    uni[511] = 25;

    OutputPatch* op = new OutputPatch(0, this);

    IOPluginStub* stub = static_cast<IOPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    op->set(stub, 0);
    QVERIFY(stub->m_universe[0] == (char) 0);
    QVERIFY(stub->m_universe[169] == (char) 0);
    QVERIFY(stub->m_universe[511] == (char) 0);

    op->dump(0, uni, true);
    QVERIFY(stub->m_universe[0] == (char) 100);
    QVERIFY(stub->m_universe[169] == (char) 50);
    QVERIFY(stub->m_universe[511] == (char) 25);

    /* Test the pause state */
    op->setPaused(true);
    op->dump(0, uni, true);
    QVERIFY(stub->m_universe[0] == (char) 100);
    QVERIFY(stub->m_universe[169] == (char) 50);
    QVERIFY(stub->m_universe[511] == (char) 25);

    uni[0] = 1;
    uni[169] = 2;
    uni[511] = 3;

    op->dump(0, uni, true);
    QVERIFY(stub->m_universe[0] == (char) 100);
    QVERIFY(stub->m_universe[169] == (char) 50);
    QVERIFY(stub->m_universe[511] == (char) 25);

    op->setPaused(false);
    op->dump(0, uni, true);
    QVERIFY(stub->m_universe[0] == (char) 1);
    QVERIFY(stub->m_universe[169] == (char) 2);
    QVERIFY(stub->m_universe[511] == (char) 3);

    delete op;
}

void OutputPatch_Test::sendSysEx()
{
    // DMX Total Control: a SysEx reaches the plugin once, as the
    // KPluginSendSysEx parameter, and is never kept: not in the parameters
    // the show saves, not sent again on reconnect
    IOPluginStub* stub = static_cast<IOPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);
    const QByteArray message = QByteArray::fromHex("F0477F4F62000100F7");

    // (on the stack: closed before the stub goes, also when a check fails)
    OutputPatch op(0, this);
    op.set(stub, 1);
    op.setPluginParameter("midichannel", 16);
    stub->m_parametersSet.clear();

    op.sendSysEx(message);
    QCOMPARE(stub->m_parametersSet.size(), 1);
    QCOMPARE(stub->m_parametersSet.at(0).first, QString(KPluginSendSysEx));
    QCOMPARE(stub->m_parametersSet.at(0).second.toByteArray(), message);
    QVERIFY(op.getPluginParameters().contains(KPluginSendSysEx) == false);
    QCOMPARE(op.getPluginParameters().value("midichannel").toInt(), 16);

    // the parameters kept are set again on reconnect, the SysEx isn't
    stub->m_parametersSet.clear();
    QVERIFY(op.reconnect());
    QCOMPARE(stub->m_parametersSet.size(), 1);
    QCOMPARE(stub->m_parametersSet.at(0).first, QString("midichannel"));

    // no plugin, no line, or its line gone (waited for by its name): nothing
    stub->m_parametersSet.clear();
    OutputPatch none(0, this);
    none.sendSysEx(message);
    OutputPatch noLine(0, this);
    noLine.set(stub, QLCIOPlugin::invalidLine());
    noLine.sendSysEx(message);
    stub->setProperty(KPluginLinesByName, true);
    OutputPatch gone(0, this);
    QVERIFY(gone.set(stub, 0, "Not plugged in") == false);
    QVERIFY(gone.lineGone());
    gone.sendSysEx(message);
    stub->setProperty(KPluginLinesByName, QVariant());
    QVERIFY(stub->m_parametersSet.isEmpty());
}

QTEST_APPLESS_MAIN(OutputPatch_Test)
