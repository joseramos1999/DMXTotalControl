/*
  Q Light Controller - Unit test
  genericfader_test.cpp

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

#include "genericfader_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "fadechannel.h"
#include "qlcchannel.h"
#include "universe.h"
#include "qlcfile.h"
#include "doc.h"

#define private public
#include "genericfader.h"
#undef private

#include "../common/resource_paths.h"

void GenericFader_Test::initTestCase()
{
    m_doc = new Doc(this);
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->loadMap(dir) == true);
}

void GenericFader_Test::init()
{
    Fixture *fxi = new Fixture(m_doc);
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    QLCFixtureMode *mode = def->mode("Mode 1");
    QVERIFY(mode != NULL);

    fxi->setFixtureDefinition(def, mode);
    fxi->setAddress(10);
    m_doc->addFixture(fxi);
}

void GenericFader_Test::cleanup()
{
    m_doc->clearContents();
}

void GenericFader_Test::addRemove()
{
    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = QSharedPointer<GenericFader>(new GenericFader());

    FadeChannel fc(m_doc, 0, 0);
    FadeChannel fc1(m_doc, 0, 1);
    FadeChannel fc2(m_doc, 0, 2);
    FadeChannel wrong(m_doc, 0, QLCChannel::invalid());
    quint32 chHash = GenericFader::channelHash(fc.fixture(), fc.channel());

    QCOMPARE(fader->m_channels.count(), 0);
    QVERIFY(fader->m_channels.contains(chHash) == false);

    fader->add(fc);
    QVERIFY(fader->m_channels.contains(chHash) == true);
    QCOMPARE(fader->m_channels.count(), 1);

    fader->remove(&wrong);
    QVERIFY(fader->m_channels.contains(chHash) == true);
    QCOMPARE(fader->m_channels.count(), 1);

    FadeChannel *fc3 = fader->getChannelFader(m_doc, ua[0], 0, 0);
    fader->remove(fc3);
    QVERIFY(fader->m_channels.contains(chHash) == false);
    QCOMPARE(fader->m_channels.count(), 0);

    fader->add(fc);
    QVERIFY(fader->m_channels.contains(chHash) == true);

    fader->add(fc1);
    chHash = GenericFader::channelHash(fc.fixture(), fc.channel());
    QVERIFY(fader->m_channels.contains(chHash) == true);

    fader->add(fc2);
    chHash = GenericFader::channelHash(fc.fixture(), fc.channel());
    QVERIFY(fader->m_channels.contains(chHash) == true);
    QCOMPARE(fader->m_channels.count(), 3);

    fader->removeAll();
    QCOMPARE(fader->m_channels.count(), 0);

    fc.setTarget(127);
    fader->add(fc);
    chHash = GenericFader::channelHash(fc.fixture(), fc.channel());
    QCOMPARE(fader->m_channels.size(), 1);
    QCOMPARE(fader->m_channels[chHash].target(), uchar(127));

    fc.setTarget(63);
    fader->add(fc);
    QCOMPARE(fader->m_channels.size(), 1);
    QCOMPARE(fader->m_channels[chHash].target(), uchar(63));

    fc.setCurrent(63);
    fader->add(fc);
    QCOMPARE(fader->m_channels.size(), 1);
    QCOMPARE(fader->m_channels[chHash].target(), uchar(63));
}

void GenericFader_Test::writeZeroFade()
{
    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = ua[0]->requestFader();

    FadeChannel fc(m_doc, 0, 5);
    fc.setStart(0);
    fc.setTarget(255);
    fc.setFadeTime(0);

    fader->add(fc);
    QCOMPARE(ua[0]->preGMValues()[15], (char) 0);
    fader->write(ua[0], MasterTimer::tick());
    QCOMPARE(ua[0]->preGMValues()[15], (char) 255);
}

void GenericFader_Test::writeLoop()
{
    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = ua[0]->requestFader();

    FadeChannel fc(m_doc, 0, 5);
    fc.setStart(0);
    fc.setTarget(250);
    fc.setFadeTime(1000);
    fader->add(fc);

    QCOMPARE(ua[0]->preGMValues()[15], (char) 0);

    int expected = 0;
    for (int i = MasterTimer::tick(); i <= 1000; i += MasterTimer::tick())
    {
        ua[0]->zeroIntensityChannels();
        fader->write(ua[0], MasterTimer::tick());

        int actual = uchar(ua[0]->preGMValues()[15]);
        expected += 5;
        QCOMPARE(actual, expected);
    }
}

void GenericFader_Test::adjustIntensity()
{
    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = ua[0]->requestFader();

    FadeChannel fc(m_doc, 0, 5);
    FadeChannel fc1(m_doc, 0, 0);

    // HTP channel
    fc.setStart(0);
    fc.setTarget(250);
    fc.setFadeTime(1000);
    fader->add(fc);

    // LTP channel
    fc1.setStart(0);
    fc1.setTarget(250);
    fc1.setFadeTime(1000);
    fader->add(fc1);

    qreal intensity = 0.5;
    fader->adjustIntensity(intensity);
    QCOMPARE(fader->intensity(), intensity);

    int expected = 0;
    for (int i = MasterTimer::tick(); i <= 1000; i += MasterTimer::tick())
    {
        ua[0]->zeroIntensityChannels();
        fader->write(ua[0], MasterTimer::tick());

        expected += 5;

        // GenericFader should apply intensity only to HTP channels
        int actual = uchar(ua[0]->preGMValues()[15]);
        int expectedWithIntensity = floor((qreal(expected) * intensity) + 0.5);
        QVERIFY(actual == expectedWithIntensity);

        // No intensity adjustment on LTP channels
        actual = uchar(ua[0]->preGMValues()[10]);
        QVERIFY(actual == expected);
    }
}

void GenericFader_Test::writeModulatedScale()
{
    // DMX Total Control: a Modulated channel (an FX) with a scale sends its
    // value dimmed by it, after the offset (the wave of a colour FX); 1 is
    // the value as it was
    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = ua[0]->requestFader();
    auto frame = [&]() {
        ua[0]->zeroIntensityChannels();
        fader->write(ua[0], MasterTimer::tick());
        return int(uchar(ua[0]->preGMValues()[15]));
    };
    auto modulate = [&](qint32 offset, bool onBelow, qreal scale) {
        FadeChannel *fc = fader->getChannelFader(m_doc, ua[0], 0, 5);
        fc->setModulation(offset, onBelow, QLCChannel::invalid(), scale);
    };

    // an intensity at 200, no offset: half of it
    FadeChannel fc(m_doc, 0, 5);
    fc.addFlag(FadeChannel::Modulated);
    fc.setStart(200);
    fc.setCurrent(200);
    fc.setTarget(200);
    fc.setFadeTime(0);
    fc.setModulation(0, false, QLCChannel::invalid(), 0.5);
    QCOMPARE(fc.modulationScale(), 0.5);
    fader->add(fc);
    QCOMPARE(frame(), 100);
    QCOMPARE(fader->getChannelFader(m_doc, ua[0], 0, 5)->modulatedOutput(), quint32(100));

    // the offset first, then the scale: (200 + 40) / 2
    modulate(40, false, 0.5);
    QCOMPARE(frame(), 120);
    // above full it is full, then dimmed (as a dimmer dims a colour at full)
    modulate(100, false, 0.5);
    QCOMPARE(frame(), 128);
    modulate(-60, false, 0.25);
    QCOMPARE(frame(), 35);
    // 1: exactly the value with its offset, as before; 0: nothing
    modulate(40, false, 1.0);
    QCOMPARE(frame(), 240);
    modulate(0, false, 1.0);
    QCOMPARE(frame(), 200);
    modulate(40, false, 0.0);
    QCOMPARE(frame(), 0);
    // bounded to 0..1
    modulate(0, false, 2.0);
    QCOMPARE(fader->getChannelFader(m_doc, ua[0], 0, 5)->modulationScale(), 1.0);
    modulate(0, false, -1.0);
    QCOMPARE(fader->getChannelFader(m_doc, ua[0], 0, 5)->modulationScale(), 0.0);

    // on what is below: half of it, the offset too
    auto below = [&](int value) {
        ua[0]->zeroIntensityChannels();
        ua[0]->write(15, uchar(value));
        fader->write(ua[0], MasterTimer::tick());
        return int(uchar(ua[0]->preGMValues()[15]));
    };
    modulate(0, true, 0.5);
    QCOMPARE(below(180), 90);
    modulate(20, true, 0.5);
    QCOMPARE(below(180), 100);
    modulate(0, true, 1.0);
    QCOMPARE(below(180), 180);
    // the level of the fader fades the effect on what is below: at half,
    // three quarters of it; at 0, what is below
    modulate(0, true, 0.5);
    fader->adjustIntensity(0.5);
    QCOMPARE(below(200), 150);
    fader->adjustIntensity(0.0);
    QCOMPARE(below(200), 200);
    fader->adjustIntensity(1.0);

    // copied and assigned with it; a modulation without a scale has none
    FadeChannel copy(*fader->getChannelFader(m_doc, ua[0], 0, 5));
    QCOMPARE(copy.modulationScale(), 0.5);
    FadeChannel assigned;
    assigned = copy;
    QCOMPARE(assigned.modulationScale(), 0.5);
    assigned.setModulation(10, false);
    QCOMPARE(assigned.modulationScale(), 1.0);
    QCOMPARE(FadeChannel().modulationScale(), 1.0);
}

void GenericFader_Test::fadeOutIsLinear()
{
    // DMX Total Control: a channel that faded in with a curve fades out
    // linearly: setFadeOut re-arms every channel without its curve
    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = ua[0]->requestFader();

    FadeChannel fc(m_doc, 0, 0);
    fc.setStart(0);
    fc.setTarget(200);
    fc.setFadeTime(0);
    fc.setCurve(FadeChannel::CurveSlowStart);
    fader->add(fc);
    const quint32 chHash = GenericFader::channelHash(fc.fixture(), fc.channel());
    QCOMPARE(fader->m_channels[chHash].curve(), int(FadeChannel::CurveSlowStart));
    fader->write(ua[0], MasterTimer::tick());
    QCOMPARE(uchar(ua[0]->preGMValues()[10]), uchar(200));

    // out in 1 s: half way at 500 ms, as a linear fade
    fader->setFadeOut(true, 1000);
    FadeChannel out = fader->m_channels[chHash];
    QCOMPARE(out.curve(), int(FadeChannel::CurveLinear));
    QCOMPARE(out.fadeTime(), 1000u);
    QCOMPARE(out.start(), quint32(200));
    QCOMPARE(int(out.calculateCurrent(out.fadeTime(), 500)), 100);
}

void GenericFader_Test::channelCopy()
{
    // DMX Total Control: what a fader holds for a channel, copied; nothing
    // for a channel it doesn't hold
    QSharedPointer<GenericFader> fader = QSharedPointer<GenericFader>(new GenericFader());
    FadeChannel fc(m_doc, 0, 2);
    fc.setStart(10);
    fc.setTarget(127);
    fc.setFadeTime(500);
    fader->add(fc);

    FadeChannel copy;
    QVERIFY(fader->channelCopy(0, 2, &copy));
    QCOMPARE(copy.fixture(), quint32(0));
    QCOMPARE(copy.channel(), quint32(2));
    QCOMPARE(copy.target(), quint32(127));
    QCOMPARE(copy.start(), quint32(10));
    QCOMPARE(copy.fadeTime(), 500u);
    QVERIFY(fader->channelCopy(0, 3, &copy) == false);
    QVERIFY(fader->channelCopy(1, 2, &copy) == false);
    // the copy is not the channel: it stays in the fader
    QCOMPARE(fader->channelsCount(), 1);
    fader->removeChannel(0, 2);
    QVERIFY(fader->channelCopy(0, 2, &copy) == false);
}

void GenericFader_Test::mixBelow()
{
    // DMX Total Control: a MixBelow channel (an Override cuelist) sends its
    // value mixed with what's below by the level of the fader, and replaces
    // it (an HTP channel too). Going back (setFadeOut) its own value goes to
    // what's below, still mixed. One that can't fade jumps
    Universe *universe = m_doc->inputOutputMap()->universes().at(0);
    QSharedPointer<GenericFader> fader = universe->requestFader();

    // an LTP channel (address 10) and an HTP one (address 15), both at 200
    FadeChannel ltp(m_doc, 0, 0);
    FadeChannel htp(m_doc, 0, 5);
    for (FadeChannel *fc : { &ltp, &htp })
    {
        fc->setStart(200);
        fc->setTarget(200);
        fc->setFadeTime(0);
        fc->addFlag(FadeChannel::MixBelow);
        fader->add(*fc);
    }
    // below them: 100 and 240
    auto frame = [&]() {
        universe->zeroIntensityChannels();
        universe->write(10, 100);
        universe->write(15, 240);
        fader->write(universe, MasterTimer::tick());
        return QString("%1/%2").arg(uchar(universe->preGMValues()[10])).arg(uchar(universe->preGMValues()[15]));
    };
    QCOMPARE(frame(), QString("200/200"));
    fader->adjustIntensity(0.5);
    QCOMPARE(frame(), QString("150/220"));
    fader->adjustIntensity(0.0);
    QCOMPARE(frame(), QString("100/240"));
    const quint32 ltpHash = GenericFader::channelHash(ltp.fixture(), ltp.channel());
    QCOMPARE(fader->m_channels[ltpHash].modulatedOutput(), quint32(100));

    // a channel that can't fade: all of it as soon as the level is up
    fader->m_channels[ltpHash].removeFlag(FadeChannel::CanFade);
    fader->adjustIntensity(0.1);
    QCOMPARE(frame().section('/', 0, 0), QString("200"));
    fader->adjustIntensity(0.0);
    QCOMPARE(frame().section('/', 0, 0), QString("100"));
    fader->m_channels[ltpHash].addFlag(FadeChannel::CanFade);

    // back in 1 s at half the level: from its own 200, not from what it sent
    fader->adjustIntensity(0.5);
    QCOMPARE(frame(), QString("150/220"));
    fader->setFadeOut(true, 1000);
    const FadeChannel out = fader->m_channels[ltpHash];
    QVERIFY(out.flags() & FadeChannel::MixBelow);
    QVERIFY(out.flags() & FadeChannel::SetTarget);
    QCOMPARE(out.start(), quint32(200));
    // half way: its own 150 and 220, mixed with 100 and 240
    QString half;
    for (uint ms = 0; ms < 500; ms += MasterTimer::tick())
        half = frame();
    QCOMPARE(half, QString("125/230"));
    // there: what's below, and the channels leave
    for (int i = 0; i < 30; i++)
        half = frame();
    QCOMPARE(half, QString("100/240"));
    QCOMPARE(fader->channelsCount(), 0);
    QVERIFY(fader->deleteRequested());
}

void GenericFader_Test::manualChannel()
{
    // DMX Total Control: a Manual channel (a hand crossfade) doesn't run on
    // the clock: it is where its elapsed time puts it. At its end it stays,
    // even flagged AutoRemove, and can come back. Fading out, it runs on
    // the clock again and leaves
    Universe *universe = m_doc->inputOutputMap()->universes().at(0);
    QSharedPointer<GenericFader> fader = universe->requestFader();
    FadeChannel fc(m_doc, 0, 0);
    fc.setStart(0);
    fc.setTarget(200);
    fc.setFadeTime(1000);
    fc.addFlag(FadeChannel::Manual | FadeChannel::AutoRemove);
    fader->add(fc);
    const quint32 hash = GenericFader::channelHash(fc.fixture(), fc.channel());
    auto frame = [&]() {
        fader->write(universe, MasterTimer::tick());
        return int(uchar(universe->preGMValues()[10]));
    };

    QCOMPARE(frame(), 0);
    for (int i = 0; i < 10; i++)
        QCOMPARE(frame(), 0);
    fader->m_channels[hash].setElapsed(500);
    QCOMPARE(frame(), 100);
    QCOMPARE(frame(), 100);
    QCOMPARE(fader->m_channels[hash].elapsed(), 500u);

    // at its end it stays, and can come back
    fader->m_channels[hash].setElapsed(1000);
    QCOMPARE(frame(), 200);
    QCOMPARE(frame(), 200);
    QCOMPARE(fader->channelsCount(), 1);
    fader->m_channels[hash].setReady(false);
    fader->m_channels[hash].setElapsed(250);
    QCOMPARE(frame(), 50);

    // fading out: on the clock, and it leaves
    fader->setFadeOut(true, 100);
    for (int i = 0; i < 10; i++)
        frame();
    QCOMPARE(fader->channelsCount(), 0);
    QVERIFY(fader->deleteRequested());
}

QTEST_APPLESS_MAIN(GenericFader_Test)
