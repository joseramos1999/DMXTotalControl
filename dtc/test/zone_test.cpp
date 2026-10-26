/*
  DMX Total Control
  zone_test.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QSignalSpy>
#include <QBuffer>
#include <QtTest>

#include "zonemanager.h"
#include "zone.h"
#include "scene.h"
#include "doc.h"

class Zone_Test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void defaults();
    void addRemoveFunctions();
    void exclusiveStart();
    void triggerToggles();
    void mixKeepsAll();
    void deletedFunctionLeavesZone();
    void managerAddRemoveMove();
    void saveLoadRoundTrip();
    void foldAll();
    void crossGroupRelease();
    void releaseProtection();
    void flashScenes();
    void nextPrevious();
    void optionsRoundTrip();
    void masterMarksModified();
    void idsNeverGivenTwice();

private:
    quint32 addScene(const QString &name);

    Doc *m_doc = nullptr;
    QList<quint32> m_scenes;
};

quint32 Zone_Test::addScene(const QString &name)
{
    Scene *s = new Scene(m_doc);
    s->setName(name);
    m_doc->addFunction(s);
    return s->id();
}

void Zone_Test::init()
{
    m_doc = new Doc(this);
    m_scenes.clear();
    for (int i = 0; i < 3; i++)
        m_scenes << addScene(QString("Scene %1").arg(i));
}

void Zone_Test::cleanup()
{
    delete m_doc;
    m_doc = nullptr;
}

void Zone_Test::defaults()
{
    Zone zone(m_doc, 7);
    QCOMPARE(zone.id(), quint32(7));
    QVERIFY(zone.exclusive());
    QCOMPARE(zone.fadeTime(), -1);
    QCOMPARE(zone.master(), 1.0);
    QCOMPARE(zone.activeCount(), 0);

    zone.setMaster(1.5);
    QCOMPARE(zone.master(), 1.0);
    zone.setMaster(-1);
    QCOMPARE(zone.master(), 0.0);
    zone.setFadeTime(-30);
    QCOMPARE(zone.fadeTime(), -1);
}

void Zone_Test::addRemoveFunctions()
{
    Zone zone(m_doc, 0);
    QSignalSpy spy(&zone, &Zone::functionsChanged);

    zone.addFunction(m_scenes[0]);
    zone.addFunction(m_scenes[1]);
    zone.addFunction(m_scenes[1]);  // duplicates are ignored
    zone.addFunction(12345);        // so are unknown functions
    QCOMPARE(zone.functionIDs(), QList<quint32>() << m_scenes[0] << m_scenes[1]);
    QCOMPARE(spy.count(), 2);

    zone.moveFunction(0, 1);
    QCOMPARE(zone.functionIDs(), QList<quint32>() << m_scenes[1] << m_scenes[0]);

    zone.removeFunction(m_scenes[1]);
    QCOMPARE(zone.functionIDs(), QList<quint32>() << m_scenes[0]);
    QCOMPARE(zone.functionsInfo().count(), 1);
    QCOMPARE(zone.functionsInfo().first().toMap().value("name").toString(), QString("Scene 0"));
}

void Zone_Test::exclusiveStart()
{
    Zone zone(m_doc, 0);
    for (quint32 id : m_scenes)
        zone.addFunction(id);

    zone.start(m_scenes[0]);
    QVERIFY(zone.isActive(m_scenes[0]));
    QCOMPARE(zone.activeCount(), 1);

    zone.start(m_scenes[2]);
    QVERIFY(zone.isActive(m_scenes[0]) == false);
    QVERIFY(zone.isActive(m_scenes[2]));
    QCOMPARE(zone.activeCount(), 1);

    zone.stopAll();
    QCOMPARE(zone.activeCount(), 0);
}

void Zone_Test::triggerToggles()
{
    Zone zone(m_doc, 0);
    zone.addFunction(m_scenes[0]);

    zone.trigger(m_scenes[0]);
    QVERIFY(zone.isActive(m_scenes[0]));
    zone.trigger(m_scenes[0]);
    QVERIFY(zone.isActive(m_scenes[0]) == false);

    // functions that aren't in the zone can't be started by it
    zone.trigger(m_scenes[1]);
    QCOMPARE(zone.activeCount(), 0);
}

void Zone_Test::mixKeepsAll()
{
    Zone zone(m_doc, 0);
    zone.setExclusive(false);
    for (quint32 id : m_scenes)
        zone.addFunction(id);

    for (quint32 id : m_scenes)
        zone.start(id);
    QCOMPARE(zone.activeCount(), 3);

    zone.stop(m_scenes[1]);
    QCOMPARE(zone.activeCount(), 2);
    QVERIFY(zone.isActive(m_scenes[0]));
    QVERIFY(zone.isActive(m_scenes[2]));
}

void Zone_Test::deletedFunctionLeavesZone()
{
    Zone zone(m_doc, 0);
    zone.addFunction(m_scenes[0]);
    zone.addFunction(m_scenes[1]);
    zone.start(m_scenes[1]);

    m_doc->deleteFunction(m_scenes[1]);
    QCOMPARE(zone.functionIDs(), QList<quint32>() << m_scenes[0]);
    QCOMPARE(zone.activeCount(), 0);
}

void Zone_Test::managerAddRemoveMove()
{
    ZoneManager manager(m_doc);
    Zone *a = manager.addZone("A");
    Zone *b = manager.addZone("");
    Zone *c = manager.addZone("C");

    QCOMPARE(manager.count(), 3);
    QCOMPARE(manager.rowCount(), 3);
    QCOMPARE(b->name(), QString("Group 2"));
    QVERIFY(a->id() != b->id() && b->id() != c->id());
    QVERIFY(a->color() != b->color());

    manager.moveZone(0, 2);
    QCOMPARE(manager.zone(0), b);
    QCOMPARE(manager.zone(2), a);
    manager.moveZone(2, 0);
    QCOMPARE(manager.zone(0), a);

    manager.removeZone(1);
    QCOMPARE(manager.count(), 2);
    QCOMPARE(manager.zoneById(c->id()), c);
    QVERIFY(manager.zone(5) == nullptr);
}

void Zone_Test::saveLoadRoundTrip()
{
    ZoneManager manager(m_doc);
    Zone *wash = manager.addZone("Wash");
    wash->setColor(QColor("#123456"));
    wash->setExclusive(false);
    wash->setFadeTime(1500);
    wash->setMaster(0.5);
    wash->setFolded(true);
    wash->addFunction(m_scenes[2]);
    wash->addFunction(m_scenes[0]);
    Zone *fx = manager.addZone("FX");
    fx->addFunction(m_scenes[1]);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QXmlStreamWriter writer(&buffer);
    writer.writeStartDocument();
    QVERIFY(manager.saveXML(&writer));
    writer.writeEndDocument();
    buffer.close();

    ZoneManager loaded(m_doc);
    buffer.open(QIODevice::ReadOnly);
    QXmlStreamReader reader(&buffer);
    QVERIFY(reader.readNextStartElement());
    QVERIFY(loaded.loadXML(reader));

    QCOMPARE(loaded.count(), 2);
    Zone *w = loaded.zone(0);
    QCOMPARE(w->name(), QString("Wash"));
    QCOMPARE(w->color(), QColor("#123456"));
    QCOMPARE(w->exclusive(), false);
    QCOMPARE(w->fadeTime(), 1500);
    QCOMPARE(w->master(), 0.5);
    // folded in Live, the other one not
    QVERIFY(w->folded());
    QVERIFY(loaded.zone(1)->folded() == false);
    QCOMPARE(w->functionIDs(), QList<quint32>() << m_scenes[2] << m_scenes[0]);
    QCOMPARE(loaded.zone(1)->name(), QString("FX"));
    QCOMPARE(loaded.zone(1)->functionIDs(), QList<quint32>() << m_scenes[1]);
}

void Zone_Test::foldAll()
{
    // the button of Live folds or unfolds every group at once; allFolded
    // follows each group, the groups added and removed and a loaded show
    ZoneManager manager(m_doc);
    QSignalSpy changed(&manager, &ZoneManager::foldedChanged);
    QCOMPARE(manager.allFolded(), false);   // no groups
    Zone *a = manager.addZone("A");
    Zone *b = manager.addZone("B");
    Zone *c = manager.addZone("C");
    QCOMPARE(changed.count(), 3);
    b->setFolded(true);
    QCOMPARE(changed.count(), 4);
    QCOMPARE(manager.allFolded(), false);

    m_doc->resetModified();
    changed.clear();
    manager.setAllFolded(true);
    QVERIFY(a->folded() && b->folded() && c->folded());
    QCOMPARE(manager.allFolded(), true);
    QVERIFY(m_doc->isModified());
    QCOMPARE(changed.count(), 2);           // A and C (B was folded)
    m_doc->resetModified();
    manager.setAllFolded(true);             // nothing to change
    QCOMPARE(m_doc->isModified(), false);

    // a new group comes unfolded; the last unfolded one removed: all folded
    Zone *d = manager.addZone("D");
    QCOMPARE(d->folded(), false);
    QCOMPARE(manager.allFolded(), false);
    manager.removeZone(3);
    QCOMPARE(manager.allFolded(), true);

    manager.setAllFolded(false);
    QVERIFY(!a->folded() && !b->folded() && !c->folded());
    QCOMPARE(manager.allFolded(), false);
    manager.setAllFolded(true);

    // saved and loaded: the groups loaded tell when they fold
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QXmlStreamWriter writer(&buffer);
    writer.writeStartDocument();
    QVERIFY(manager.saveXML(&writer));
    writer.writeEndDocument();
    buffer.close();
    ZoneManager loaded(m_doc);
    QSignalSpy loadedChanged(&loaded, &ZoneManager::foldedChanged);
    buffer.open(QIODevice::ReadOnly);
    QXmlStreamReader reader(&buffer);
    QVERIFY(reader.readNextStartElement());
    QVERIFY(loaded.loadXML(reader));
    QCOMPARE(loaded.count(), 3);
    QCOMPARE(loaded.allFolded(), true);
    QCOMPARE(loadedChanged.count(), 1);     // the show loaded
    loaded.zone(1)->setFolded(false);
    QCOMPARE(loadedChanged.count(), 2);
    QCOMPARE(loaded.allFolded(), false);
    loaded.clear();
    QCOMPARE(loaded.allFolded(), false);
}

void Zone_Test::crossGroupRelease()
{
    ZoneManager manager(m_doc);
    Zone *a = manager.addZone("A");
    Zone *b = manager.addZone("B");
    b->setExclusive(false);
    a->addFunction(m_scenes[0]);
    a->addFunction(m_scenes[1]);
    b->addFunction(m_scenes[2]);

    b->start(m_scenes[2]);
    a->start(m_scenes[0]);
    // default release: only its own (exclusive) group
    QVERIFY(b->isActive(m_scenes[2]));

    // release all: the other groups too
    a->setOption(m_scenes[1], "release", Zone::ReleaseAll);
    a->start(m_scenes[1]);
    QVERIFY(a->isActive(m_scenes[0]) == false);
    QVERIFY(b->isActive(m_scenes[2]) == false);

    // release the other groups only
    a->setOption(m_scenes[0], "release", Zone::ReleaseOthers);
    b->start(m_scenes[2]);
    a->start(m_scenes[0]);
    QVERIFY(a->isActive(m_scenes[1]));
    QVERIFY(b->isActive(m_scenes[2]) == false);

    // no release, even in an exclusive group
    a->stopAll();
    a->setOption(m_scenes[1], "release", Zone::ReleaseNone);
    a->setOption(m_scenes[0], "release", Zone::ReleaseGroupDefault);
    a->start(m_scenes[0]);
    a->start(m_scenes[1]);
    QCOMPARE(a->activeCount(), 2);
}

void Zone_Test::releaseProtection()
{
    ZoneManager manager(m_doc);
    Zone *a = manager.addZone("A");
    Zone *b = manager.addZone("B");
    a->addFunction(m_scenes[0]);
    a->addFunction(m_scenes[1]);
    b->addFunction(m_scenes[2]);
    a->setOption(m_scenes[1], "release", Zone::ReleaseAll);

    b->setOption(m_scenes[2], "protection", Zone::ProtectOthers);
    b->start(m_scenes[2]);
    a->start(m_scenes[1]);
    QVERIFY(b->isActive(m_scenes[2]));

    // protected from its own group, but not from the others
    b->setOption(m_scenes[2], "protection", Zone::ProtectGroup);
    a->stop(m_scenes[1]);
    a->start(m_scenes[1]);
    QVERIFY(b->isActive(m_scenes[2]) == false);

    // a fully protected scene only stops by hand
    a->setOption(m_scenes[0], "protection", Zone::ProtectAll);
    a->stopAll();
    a->start(m_scenes[0]);
    a->start(m_scenes[1]);
    QVERIFY(a->isActive(m_scenes[0]));
    a->stop(m_scenes[0]);
    QVERIFY(a->isActive(m_scenes[0]) == false);
}

void Zone_Test::flashScenes()
{
    Zone zone(m_doc, 0);
    zone.addFunction(m_scenes[0]);
    zone.addFunction(m_scenes[1]);
    zone.setOption(m_scenes[0], "flash", true);

    zone.press(m_scenes[0]);
    QVERIFY(zone.isActive(m_scenes[0]));
    zone.unpress(m_scenes[0]);
    QVERIFY(zone.isActive(m_scenes[0]) == false);

    // normal scenes toggle on press, and ignore the release
    zone.press(m_scenes[1]);
    zone.unpress(m_scenes[1]);
    QVERIFY(zone.isActive(m_scenes[1]));
    zone.press(m_scenes[1]);
    QVERIFY(zone.isActive(m_scenes[1]) == false);
}

void Zone_Test::nextPrevious()
{
    Zone zone(m_doc, 0);
    zone.setExclusive(false);
    for (quint32 id : m_scenes)
        zone.addFunction(id);

    zone.next();
    QVERIFY(zone.isActive(m_scenes[0]));
    zone.next();
    QVERIFY(zone.isActive(m_scenes[1]));
    QCOMPARE(zone.activeCount(), 1);
    zone.previous();
    zone.previous();
    QVERIFY(zone.isActive(m_scenes[2]));
    QCOMPARE(zone.activeCount(), 1);
}

void Zone_Test::optionsRoundTrip()
{
    ZoneManager manager(m_doc);
    Zone *zone = manager.addZone("Opts");
    zone->addFunction(m_scenes[0]);
    zone->addFunction(m_scenes[1]);
    zone->setOption(m_scenes[0], "release", Zone::ReleaseOthers);
    zone->setOption(m_scenes[0], "protection", Zone::ProtectAll);
    zone->setOption(m_scenes[1], "flash", true);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QXmlStreamWriter writer(&buffer);
    writer.writeStartDocument();
    manager.saveXML(&writer);
    writer.writeEndDocument();
    buffer.close();

    ZoneManager loaded(m_doc);
    buffer.open(QIODevice::ReadOnly);
    QXmlStreamReader reader(&buffer);
    QVERIFY(reader.readNextStartElement());
    QVERIFY(loaded.loadXML(reader));

    Zone *z = loaded.zone(0);
    QCOMPARE(z->effectiveRelease(m_scenes[0]), Zone::ReleaseOthers);
    QCOMPARE(z->protection(m_scenes[0]), Zone::ProtectAll);
    QVERIFY(z->isFlash(m_scenes[1]));
    QVERIFY(z->isFlash(m_scenes[0]) == false);
    QCOMPARE(z->effectiveRelease(m_scenes[1]), Zone::ReleaseGroup);
}

void Zone_Test::masterMarksModified()
{
    // the master is saved with the show: moving it is a change to save
    Zone zone(m_doc, 0);
    m_doc->resetModified();
    zone.setMaster(0.6);
    QVERIFY(m_doc->isModified());
    // folding it too
    m_doc->resetModified();
    zone.setFolded(true);
    QVERIFY(m_doc->isModified());
}

void Zone_Test::idsNeverGivenTwice()
{
    // the newest group deleted, a new one: it doesn't get its ID (what
    // pointed at the deleted one, a SHOW fader or a mapping, would drive it)
    ZoneManager manager(m_doc);
    const quint32 a = manager.addZone("A")->id();
    const quint32 b = manager.addZone("B")->id();
    manager.removeZone(1);
    const quint32 c = manager.addZone("C")->id();
    QVERIFY(c != a && c != b);

    // nor after a save and a reload
    manager.removeZone(1);
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QXmlStreamWriter writer(&buffer);
    writer.writeStartDocument();
    QVERIFY(manager.saveXML(&writer));
    writer.writeEndDocument();
    buffer.close();

    ZoneManager loaded(m_doc);
    buffer.open(QIODevice::ReadOnly);
    QXmlStreamReader reader(&buffer);
    QVERIFY(reader.readNextStartElement());
    QVERIFY(loaded.loadXML(reader));
    QCOMPARE(loaded.count(), 1);
    QCOMPARE(loaded.zone(0)->id(), a);
    const quint32 d = loaded.addZone("D")->id();
    QVERIFY(d != a && d != b && d != c);

    // a new show starts again from the first
    loaded.clear();
    QCOMPARE(loaded.addZone("E")->id(), quint32(0));
}

QTEST_MAIN(Zone_Test)
#include "zone_test.moc"
