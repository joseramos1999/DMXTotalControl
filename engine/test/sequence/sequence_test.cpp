/*
  Q Light Controller Plus - Unit test
  sequence_test.cpp

  Copyright (c) Massimo Callegari

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
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#define protected public
#include "mastertimer_stub.h"
#include "sequence_test.h"
#include "chaserstep.h"
#include "sequence.h"
#include "universe.h"
#include "function.h"
#include "fixture.h"
#include "qlcfile.h"
#include "scene.h"
#include "doc.h"
#undef protected

void Sequence_Test::initTestCase()
{
    m_doc = new Doc(this);
}

void Sequence_Test::cleanupTestCase()
{
    delete m_doc;
}

void Sequence_Test::init()
{
}

void Sequence_Test::cleanup()
{
    m_doc->clearContents();
}

void Sequence_Test::initial()
{
    Sequence s(m_doc);
    QVERIFY(s.type() == Function::SequenceType);
    QVERIFY(s.name() == "New Sequence");
    QCOMPARE(s.id(), Function::invalidId());
    QCOMPARE(s.boundSceneID(), Function::invalidId());
    QCOMPARE(s.steps().size(), 0);
    QCOMPARE(s.direction(), Sequence::Forward);
    QCOMPARE(s.runOrder(), Sequence::Loop);
    QCOMPARE(s.fadeInMode(), Sequence::Default);
    QCOMPARE(s.fadeOutMode(), Sequence::Default);
    QCOMPARE(s.durationMode(), Sequence::Common);
}

void Sequence_Test::createCopy()
{
    Doc doc(this);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setAddress(0);
    fxi->setUniverse(0);
    fxi->setChannels(5);
    m_doc->addFixture(fxi);

    Scene *scene = new Scene(m_doc);
    scene->addFixture(fxi->id());
    scene->setValue(SceneValue(0, 0, 0));
    scene->setValue(SceneValue(0, 1, 0));
    scene->setValue(SceneValue(0, 2, 0));

    doc.addFunction(scene);
    QVERIFY(scene->id() != Function::invalidId());

    Sequence* seq = new Sequence(m_doc);
    seq->setName("First");
    seq->setFadeInSpeed(42);
    seq->setFadeOutSpeed(69);
    seq->setDuration(1337);
    seq->setDirection(Sequence::Backward);
    seq->setRunOrder(Sequence::SingleShot);
    seq->setBoundSceneID(scene->id());

    ChaserStep step;
    step.fid = scene->id();
    step.values.append(SceneValue(0, 0, 100));
    step.values.append(SceneValue(0, 1, 50));
    step.values.append(SceneValue(0, 2, 25));
    seq->addStep(step);
    step.values.replace(1, SceneValue(0, 1, 80));
    step.values.replace(2, SceneValue(0, 2, 200));
    seq->addStep(step);
    step.values.replace(0, SceneValue(0, 1, 180));
    step.values.replace(2, SceneValue(0, 2, 0));
    seq->addStep(step);
    QCOMPARE(seq->steps().size(), 3);


    doc.addFunction(seq);
    QVERIFY(seq->id() != Function::invalidId());

    Function* f = seq->createCopy(&doc);
    QVERIFY(f != NULL);
    QVERIFY(f != seq);
    QVERIFY(f->id() != seq->id());

    Sequence* copy = qobject_cast<Sequence*> (f);
    QVERIFY(copy != NULL);
    QVERIFY(copy->fadeInSpeed() == 42);
    QVERIFY(copy->fadeOutSpeed() == 69);
    QVERIFY(copy->duration() == 1337);
    QVERIFY(copy->direction() == Sequence::Backward);
    QVERIFY(copy->runOrder() == Sequence::SingleShot);
    QVERIFY(copy->boundSceneID() == scene->id());

    QCOMPARE(copy->steps().size(), 3);
    QVERIFY(copy->steps().at(0) == seq->steps().at(0));
    QVERIFY(copy->steps().at(1) == seq->steps().at(1));
    QVERIFY(copy->steps().at(2) == seq->steps().at(2));
}

void Sequence_Test::loadWrongRoot()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Wrong");
    xmlWriter.writeAttribute("Type", "Sequence");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Sequence s(m_doc);
    QVERIFY(s.loadXML(xmlReader) == false);
}

void Sequence_Test::loadWrongType()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "Wrong");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Sequence s(m_doc);
    QVERIFY(s.loadXML(xmlReader) == false);
}

void Sequence_Test::loadWithScene()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("ID", "1");
    xmlWriter.writeAttribute("Type", "Sequence");
    xmlWriter.writeAttribute("Name", "Test Sequence");
    xmlWriter.writeAttribute("BoundScene", "0");

    xmlWriter.writeStartElement("Speed");
    xmlWriter.writeAttribute("FadeIn", "42");
    xmlWriter.writeAttribute("FadeOut", "69");
    xmlWriter.writeAttribute("Duration", "1337");
    xmlWriter.writeEndElement();

    xmlWriter.writeTextElement("Direction", "Backward");
    xmlWriter.writeTextElement("RunOrder", "SingleShot");

    xmlWriter.writeStartElement("SpeedModes");
    xmlWriter.writeAttribute("FadeIn", "Common");
    xmlWriter.writeAttribute("FadeOut", "Default");
    xmlWriter.writeAttribute("Duration", "PerStep");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "0");
    xmlWriter.writeAttribute("FadeIn", "0");
    xmlWriter.writeAttribute("FadeOut", "0");
    xmlWriter.writeAttribute("Hold", "1000");
    xmlWriter.writeAttribute("Values", "12");
    xmlWriter.writeCharacters("0:6,255:1:6,255:2:6,255");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeAttribute("FadeIn", "0");
    xmlWriter.writeAttribute("FadeOut", "0");
    xmlWriter.writeAttribute("Hold", "1000");
    xmlWriter.writeAttribute("Values", "12");
    xmlWriter.writeCharacters("0:6,255,7,150:1:6,255,7,150:2:6,255,7,150");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "2");
    xmlWriter.writeAttribute("FadeIn", "0");
    xmlWriter.writeAttribute("FadeOut", "0");
    xmlWriter.writeAttribute("Hold", "1000");
    xmlWriter.writeAttribute("Values", "12");
    xmlWriter.writeCharacters("0:6,200,7,100,8,90:1:6,200,7,100,8,90:2:6,200,7,100,8,90");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    Fixture* fxi1 = new Fixture(m_doc);
    fxi1->setAddress(0);
    fxi1->setUniverse(0);
    fxi1->setChannels(8);
    m_doc->addFixture(fxi1);

    Fixture* fxi2 = new Fixture(m_doc);
    fxi2->setAddress(10);
    fxi2->setUniverse(0);
    fxi2->setChannels(8);
    m_doc->addFixture(fxi2);

    Fixture* fxi3 = new Fixture(m_doc);
    fxi3->setAddress(20);
    fxi3->setUniverse(0);
    fxi3->setChannels(8);
    m_doc->addFixture(fxi3);

    /* prepare a Scene bound to the Sequence */
    Scene *s = new Scene(m_doc);
    s->addFixture(0);
    s->addFixture(1);
    s->addFixture(2);
    s->setValue(0, 5, 0);
    s->setValue(0, 6, 0);
    s->setValue(0, 7, 0);
    s->setValue(0, 8, 0);
    s->setValue(1, 5, 0);
    s->setValue(1, 6, 0);
    s->setValue(1, 7, 0);
    s->setValue(1, 8, 0);
    s->setValue(2, 5, 0);
    s->setValue(2, 6, 0);
    s->setValue(2, 7, 0);
    s->setValue(2, 8, 0);
    QVERIFY(m_doc->addFunction(s) == true);

    QVERIFY(s->id() == 0);

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Sequence *seq = new Sequence(m_doc);

    /* load the XML contents and check that no fixup was needed
     * since the bound Scene is present in Doc */
    QVERIFY(seq->loadXML(xmlReader) == true);
    QVERIFY(seq->m_needFixup == false);

    QVERIFY(m_doc->addFunction(seq) == true);
    QVERIFY(seq->id() == 1);
    QVERIFY(seq->boundSceneID() == 0);
    QVERIFY(seq->fadeInSpeed() == 42);
    QVERIFY(seq->fadeOutSpeed() == 69);
    QVERIFY(seq->duration() == 1337);

    QVERIFY(seq->fadeInMode() == Chaser::Common);
    QVERIFY(seq->fadeOutMode() == Chaser::Default);
    QVERIFY(seq->durationMode() == Chaser::PerStep);
    QVERIFY(seq->direction() == Chaser::Backward);
    QVERIFY(seq->runOrder() == Chaser::SingleShot);

    QVERIFY(seq->stepsCount() == 3);
    QVERIFY(seq->stepAt(0)->values.count() == 12);
    QVERIFY(seq->stepAt(1)->values.count() == 12);
    QVERIFY(seq->stepAt(2)->values.count() == 12);

    /* Now steps are fully loaded and contains all the Scene
     * values (12), so check how the XML values went into
     * ChaserStep values */
    ChaserStep *cs = seq->stepAt(0);
    QVERIFY(cs->values.at(1) == SceneValue(0, 6));
    QVERIFY(cs->values.at(1).value == 255);
    QVERIFY(cs->values.at(5) == SceneValue(1, 6));
    QVERIFY(cs->values.at(5).value == 255);
    QVERIFY(cs->values.at(9) == SceneValue(2, 6));
    QVERIFY(cs->values.at(9).value == 255);

    cs = seq->stepAt(1);
    QVERIFY(cs->values.at(1) == SceneValue(0, 6));
    QVERIFY(cs->values.at(1).value == 255);
    QVERIFY(cs->values.at(2) == SceneValue(0, 7));
    QVERIFY(cs->values.at(2).value == 150);
    QVERIFY(cs->values.at(5) == SceneValue(1, 6));
    QVERIFY(cs->values.at(5).value == 255);
    QVERIFY(cs->values.at(6) == SceneValue(1, 7));
    QVERIFY(cs->values.at(6).value == 150);
    QVERIFY(cs->values.at(9) == SceneValue(2, 6));
    QVERIFY(cs->values.at(9).value == 255);
    QVERIFY(cs->values.at(10) == SceneValue(2, 7));
    QVERIFY(cs->values.at(10).value == 150);

    cs = seq->stepAt(2);
    QVERIFY(cs->values.at(1) == SceneValue(0, 6));
    QVERIFY(cs->values.at(1).value == 200);
    QVERIFY(cs->values.at(2) == SceneValue(0, 7));
    QVERIFY(cs->values.at(2).value == 100);
    QVERIFY(cs->values.at(3) == SceneValue(0, 8));
    QVERIFY(cs->values.at(3).value == 90);
    QVERIFY(cs->values.at(5) == SceneValue(1, 6));
    QVERIFY(cs->values.at(5).value == 200);
    QVERIFY(cs->values.at(6) == SceneValue(1, 7));
    QVERIFY(cs->values.at(6).value == 100);
    QVERIFY(cs->values.at(7) == SceneValue(1, 8));
    QVERIFY(cs->values.at(7).value == 90);
    QVERIFY(cs->values.at(9) == SceneValue(2, 6));
    QVERIFY(cs->values.at(9).value == 200);
    QVERIFY(cs->values.at(10) == SceneValue(2, 7));
    QVERIFY(cs->values.at(10).value == 100);
    QVERIFY(cs->values.at(11) == SceneValue(2, 8));
    QVERIFY(cs->values.at(11).value == 90);
}

void Sequence_Test::loadWithoutScene()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("ID", "0");
    xmlWriter.writeAttribute("Type", "Sequence");
    xmlWriter.writeAttribute("Name", "Test Sequence");
    xmlWriter.writeAttribute("BoundScene", "1");

    xmlWriter.writeStartElement("Speed");
    xmlWriter.writeAttribute("FadeIn", "42");
    xmlWriter.writeAttribute("FadeOut", "69");
    xmlWriter.writeAttribute("Duration", "1337");
    xmlWriter.writeEndElement();

    xmlWriter.writeTextElement("Direction", "Backward");
    xmlWriter.writeTextElement("RunOrder", "SingleShot");

    xmlWriter.writeStartElement("SpeedModes");
    xmlWriter.writeAttribute("FadeIn", "Common");
    xmlWriter.writeAttribute("FadeOut", "Default");
    xmlWriter.writeAttribute("Duration", "PerStep");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "0");
    xmlWriter.writeAttribute("FadeIn", "0");
    xmlWriter.writeAttribute("FadeOut", "0");
    xmlWriter.writeAttribute("Hold", "1000");
    xmlWriter.writeAttribute("Values", "12");
    xmlWriter.writeCharacters("0:6,255:1:6,255:2:6,255");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeAttribute("FadeIn", "0");
    xmlWriter.writeAttribute("FadeOut", "0");
    xmlWriter.writeAttribute("Hold", "1000");
    xmlWriter.writeAttribute("Values", "12");
    xmlWriter.writeCharacters("0:6,255,7,150:1:6,255,7,150:2:6,255,7,150");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Step");
    xmlWriter.writeAttribute("Number", "2");
    xmlWriter.writeAttribute("FadeIn", "0");
    xmlWriter.writeAttribute("FadeOut", "0");
    xmlWriter.writeAttribute("Hold", "1000");
    xmlWriter.writeAttribute("Values", "12");
    xmlWriter.writeCharacters("0:6,200,7,100,8,90:1:6,200,7,100,8,90:2:6,200,7,100,8,90");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Fixture* fxi1 = new Fixture(m_doc);
    fxi1->setAddress(0);
    fxi1->setUniverse(0);
    fxi1->setChannels(8);
    m_doc->addFixture(fxi1);

    Fixture* fxi2 = new Fixture(m_doc);
    fxi2->setAddress(10);
    fxi2->setUniverse(0);
    fxi2->setChannels(8);
    m_doc->addFixture(fxi2);

    Fixture* fxi3 = new Fixture(m_doc);
    fxi3->setAddress(20);
    fxi3->setUniverse(0);
    fxi3->setChannels(8);
    m_doc->addFixture(fxi3);

    Sequence *seq = new Sequence(m_doc);

    /* load the XML contents and check that a fixup is needed
     * cause the bound Scene is not present in Doc */
    QVERIFY(seq->loadXML(xmlReader) == true);
    QVERIFY(seq->m_needFixup == true);

    QVERIFY(m_doc->addFunction(seq) == true);
    QVERIFY(seq->id() == 0);
    QVERIFY(seq->boundSceneID() == 1);
    QVERIFY(seq->fadeInSpeed() == 42);
    QVERIFY(seq->fadeOutSpeed() == 69);
    QVERIFY(seq->duration() == 1337);

    QVERIFY(seq->fadeInMode() == Chaser::Common);
    QVERIFY(seq->fadeOutMode() == Chaser::Default);
    QVERIFY(seq->durationMode() == Chaser::PerStep);
    QVERIFY(seq->direction() == Chaser::Backward);
    QVERIFY(seq->runOrder() == Chaser::SingleShot);

    QVERIFY(seq->stepsCount() == 3);
    QVERIFY(seq->stepAt(0)->values.count() == 3);
    QVERIFY(seq->stepAt(1)->values.count() == 6);
    QVERIFY(seq->stepAt(2)->values.count() == 9);

    /* now add the Scene bound to the Sequence and call postLoad */
    Scene *s = new Scene(m_doc);
    s->addFixture(0);
    s->addFixture(1);
    s->addFixture(2);
    s->setValue(0, 5, 0);
    s->setValue(0, 6, 0);
    s->setValue(0, 7, 0);
    s->setValue(0, 8, 0);
    s->setValue(1, 5, 0);
    s->setValue(1, 6, 0);
    s->setValue(1, 7, 0);
    s->setValue(1, 8, 0);
    s->setValue(2, 5, 0);
    s->setValue(2, 6, 0);
    s->setValue(2, 7, 0);
    s->setValue(2, 8, 0);
    QVERIFY(m_doc->addFunction(s) == true);

    QVERIFY(s->id() == 1);

    seq->postLoad();

    QVERIFY(seq->m_needFixup == false);
    QVERIFY(seq->stepsCount() == 3);
    QVERIFY(seq->stepAt(0)->values.count() == 12);
    QVERIFY(seq->stepAt(1)->values.count() == 12);
    QVERIFY(seq->stepAt(2)->values.count() == 12);

    /* After postLoad, steps are fully loaded and contains all the Scene
     * values (12), so check how the values has been fixed into
     * ChaserStep values */
    ChaserStep *cs = seq->stepAt(0);
    QVERIFY(cs->values.at(1) == SceneValue(0, 6));
    QVERIFY(cs->values.at(1).value == 255);
    QVERIFY(cs->values.at(5) == SceneValue(1, 6));
    QVERIFY(cs->values.at(5).value == 255);
    QVERIFY(cs->values.at(9) == SceneValue(2, 6));
    QVERIFY(cs->values.at(9).value == 255);

    cs = seq->stepAt(1);
    QVERIFY(cs->values.at(1) == SceneValue(0, 6));
    QVERIFY(cs->values.at(1).value == 255);
    QVERIFY(cs->values.at(2) == SceneValue(0, 7));
    QVERIFY(cs->values.at(2).value == 150);
    QVERIFY(cs->values.at(5) == SceneValue(1, 6));
    QVERIFY(cs->values.at(5).value == 255);
    QVERIFY(cs->values.at(6) == SceneValue(1, 7));
    QVERIFY(cs->values.at(6).value == 150);
    QVERIFY(cs->values.at(9) == SceneValue(2, 6));
    QVERIFY(cs->values.at(9).value == 255);
    QVERIFY(cs->values.at(10) == SceneValue(2, 7));
    QVERIFY(cs->values.at(10).value == 150);

    cs = seq->stepAt(2);
    QVERIFY(cs->values.at(1) == SceneValue(0, 6));
    QVERIFY(cs->values.at(1).value == 200);
    QVERIFY(cs->values.at(2) == SceneValue(0, 7));
    QVERIFY(cs->values.at(2).value == 100);
    QVERIFY(cs->values.at(3) == SceneValue(0, 8));
    QVERIFY(cs->values.at(3).value == 90);
    QVERIFY(cs->values.at(5) == SceneValue(1, 6));
    QVERIFY(cs->values.at(5).value == 200);
    QVERIFY(cs->values.at(6) == SceneValue(1, 7));
    QVERIFY(cs->values.at(6).value == 100);
    QVERIFY(cs->values.at(7) == SceneValue(1, 8));
    QVERIFY(cs->values.at(7).value == 90);
    QVERIFY(cs->values.at(9) == SceneValue(2, 6));
    QVERIFY(cs->values.at(9).value == 200);
    QVERIFY(cs->values.at(10) == SceneValue(2, 7));
    QVERIFY(cs->values.at(10).value == 100);
    QVERIFY(cs->values.at(11) == SceneValue(2, 8));
    QVERIFY(cs->values.at(11).value == 90);
}

void Sequence_Test::save()
{
    Sequence* seq = new Sequence(m_doc);
    seq->setName("First");
    seq->setFadeInSpeed(42);
    seq->setFadeOutSpeed(69);
    seq->setDuration(1337);
    seq->setDirection(Sequence::Backward);
    seq->setRunOrder(Sequence::SingleShot);
    seq->setBoundSceneID(0);

    ChaserStep step;
    step.fid = 0;
    step.values.append(SceneValue(0, 0, 100));
    step.values.append(SceneValue(0, 1, 50));
    step.values.append(SceneValue(0, 2, 25));
    seq->addStep(step);
    step.values.replace(1, SceneValue(0, 1, 80));
    step.values.replace(2, SceneValue(0, 2, 200));
    seq->addStep(step);
    step.values.replace(1, SceneValue(0, 1, 180));
    step.values.replace(2, SceneValue(0, 2, 0));
    seq->addStep(step);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(seq->saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(xmlReader.name().toString() == "Function");
    QVERIFY(xmlReader.attributes().value("Type").toString() == "Sequence");

    int run = 0, dir = 0, speed = 0, fstep = 0, speedmodes = 0;

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name().toString() == "Direction")
        {
            QVERIFY(xmlReader.readElementText() == "Backward");
            dir++;
        }
        else if (xmlReader.name().toString() == "RunOrder")
        {
            QVERIFY(xmlReader.readElementText() == "SingleShot");
            run++;
        }
        else if (xmlReader.name().toString() == "Step")
        {
            QString text = xmlReader.readElementText();
            if (fstep == 0)
                QCOMPARE(text, QString("0:0,100,1,50,2,25"));
            else if (fstep == 1)
                QCOMPARE(text, QString("0:0,100,1,80,2,200"));
            else if (fstep == 2)
                QCOMPARE(text, QString("0:0,100,1,180"));
            fstep++;
        }
        else if (xmlReader.name().toString() == "Speed")
        {
            speed++;
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name().toString() == "SpeedModes")
        {
            QCOMPARE(xmlReader.attributes().value("FadeIn").toString(), QString("Default"));
            QCOMPARE(xmlReader.attributes().value("FadeOut").toString(), QString("Default"));
            QCOMPARE(xmlReader.attributes().value("Duration").toString(), QString("Common"));
            speedmodes++;
            xmlReader.skipCurrentElement();
        }
        else
        {
            QFAIL("Unhandled XML tag.");
            xmlReader.skipCurrentElement();
        }
    }

    QCOMPARE(speed, 1);
    QCOMPARE(speedmodes, 1);
    QCOMPARE(dir, 1);
    QCOMPARE(run, 1);
    QCOMPARE(fstep, 3);
}

/* DMX Total Control: a palette changed or deleted, a fixture removed. The
   steps keep their references (ChaserStep::palettes) */
void Sequence_Test::palettes()
{
    typedef QPair<quint32, quint32> Key;
    Fixture *fxi = new Fixture(m_doc);
    fxi->setAddress(0);
    fxi->setUniverse(0);
    fxi->setChannels(4);
    QVERIFY(m_doc->addFixture(fxi));
    Fixture *fxi2 = new Fixture(m_doc);
    fxi2->setAddress(10);
    fxi2->setUniverse(0);
    fxi2->setChannels(4);
    QVERIFY(m_doc->addFixture(fxi2));
    const quint32 a = fxi->id(), b = fxi2->id();

    Scene *scene = new Scene(m_doc);
    for (quint32 c = 0; c < 3; c++)
    {
        scene->setValue(SceneValue(a, c, 0));
        scene->setValue(SceneValue(b, c, 0));
    }
    QVERIFY(m_doc->addFunction(scene));
    Sequence *seq = new Sequence(m_doc);
    seq->setBoundSceneID(scene->id());
    QVERIFY(m_doc->addFunction(seq));

    // step 1: channels 0 and 1 from palette 5, channel 2 of a from palette 6;
    // step 2 the same values by hand
    ChaserStep step(scene->id());
    step.values = { SceneValue(a, 0, 10), SceneValue(a, 1, 20), SceneValue(a, 2, 30),
                    SceneValue(b, 0, 10), SceneValue(b, 1, 20), SceneValue(b, 2, 0) };
    step.palettes = { { Key(a, 0), 5 }, { Key(a, 1), 5 }, { Key(a, 2), 6 }, { Key(b, 0), 5 }, { Key(b, 1), 5 } };
    QVERIFY(seq->addStep(step));
    ChaserStep hand(scene->id());
    hand.values = step.values;
    QVERIFY(seq->addStep(hand));
    auto valueOf = [&](int index, quint32 fixture, quint32 channel) {
        for (const SceneValue &scv : seq->steps().at(index).values)
            if (scv.fxi == fixture && scv.channel == channel)
                return int(scv.value);
        return -1;
    };

    // palette 5 changed: 100 on the channels 0, the same 20 on channel 1 of
    // a, nothing any more on channel 1 of b (-1: a plain value). Said once
    QSignalSpy changed(seq, SIGNAL(changed(quint32)));
    auto palette5 = [&](quint32 fixture, quint32 channel) {
        if (channel == 0)
            return 100;
        return fixture == b ? -1 : 20;
    };
    seq->applyPalette(5, palette5);
    QCOMPARE(changed.count(), 1);
    QCOMPARE(valueOf(0, a, 0), 100);
    QCOMPARE(valueOf(0, b, 0), 100);
    QCOMPARE(valueOf(0, a, 1), 20);
    QCOMPARE(valueOf(0, b, 1), 20);
    QCOMPARE(valueOf(0, a, 2), 30);
    QCOMPARE(seq->steps().at(0).palettes,
             (QHash<Key, int>({ { Key(a, 0), 5 }, { Key(a, 1), 5 }, { Key(a, 2), 6 }, { Key(b, 0), 5 } })));
    // the step by hand doesn't follow
    QCOMPARE(valueOf(1, a, 0), 10);
    QVERIFY(seq->steps().at(1).palettes.isEmpty());

    // the same again, or a palette no step uses: nothing changes, nothing said
    seq->applyPalette(5, palette5);
    seq->applyPalette(9, palette5);
    QCOMPARE(changed.count(), 1);

    // deleted: the values stay, without the references
    seq->forgetPalette(5);
    QCOMPARE(changed.count(), 2);
    QCOMPARE(seq->steps().at(0).palettes, (QHash<Key, int>({ { Key(a, 2), 6 } })));
    QCOMPARE(valueOf(0, a, 0), 100);
    seq->forgetPalette(5);
    QCOMPARE(changed.count(), 2);

    // a fixture removed: its references go with its values
    ChaserStep linked = seq->steps().at(0);
    linked.palettes.insert(Key(b, 0), 6);
    QVERIFY(seq->replaceStep(linked, 0));
    changed.clear();
    QVERIFY(m_doc->deleteFixture(b));
    QCOMPARE(seq->steps().at(0).palettes, (QHash<Key, int>({ { Key(a, 2), 6 } })));
    QCOMPARE(valueOf(0, b, 0), -1);
    QCOMPARE(changed.count(), 1);
}

QTEST_APPLESS_MAIN(Sequence_Test)
