/*
  DMX Total Control
  curvefx.cpp

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
#include <QDebug>
#include <QIcon>
#include <qmath.h>

#include "genericfader.h"
#include "mastertimer.h"
#include "fadechannel.h"
#include "qlcchannel.h"
#include "universe.h"
#include "curvefx.h"
#include "fixture.h"
#include "doc.h"

CurveFX::CurveFX(Doc *doc)
    : Function(doc, Function::CurveFXType)
    , m_waveform(Sine)
    , m_low(0)
    , m_high(255)
    , m_phaseOffset(0)
    , m_spread(0)
    , m_duty(50)
    , m_relative(false)
    , m_targetCount(0)
    , m_cycleElapsed(0)
    , m_cycleNumber(0)
{
    setName(tr("New Curve FX"));
    setDuration(2000);

    registerAttribute(tr("Size"), Function::LastWins, 0.0, 2.0, 1.0);
    registerAttribute(tr("Phase"), Function::LastWins, 0.0, 359.0, 0.0);

}

CurveFX::~CurveFX()
{
}

QIcon CurveFX::getIcon() const
{
    return QIcon(":/efx.png");
}

quint32 CurveFX::totalDuration()
{
    return duration();
}

/*********************************************************************
 * Copying
 *********************************************************************/

Function *CurveFX::createCopy(Doc *doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function *copy = new CurveFX(doc);
    if (copy->copyFrom(this) == false)
    {
        delete copy;
        copy = NULL;
    }
    if (addToDoc == true && copy != NULL && doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }
    return copy;
}

bool CurveFX::copyFrom(const Function *function)
{
    const CurveFX *curve = qobject_cast<const CurveFX *>(function);
    if (curve == NULL)
        return false;

    m_fixtures = curve->m_fixtures;
    m_features = curve->m_features;
    m_waveform = curve->m_waveform;
    m_low = curve->m_low;
    m_high = curve->m_high;
    m_phaseOffset = curve->m_phaseOffset;
    m_spread = curve->m_spread;
    m_duty = curve->m_duty;
    m_relative = curve->m_relative;

    return Function::copyFrom(function);
}

/*********************************************************************
 * Contents
 *********************************************************************/

void CurveFX::setFixtures(const QList<quint32> &fixtures)
{
    QList<quint32> list;
    for (quint32 id : fixtures)
        if (list.contains(id) == false)
            list.append(id);
    {
        // (preRun reads them in the timer thread)
        QMutexLocker locker(&m_targetsMutex);
        m_fixtures = list;
    }
    updateTargets();
    emit changed(id());
}

QList<quint32> CurveFX::fixtures() const
{
    return m_fixtures;
}

void CurveFX::setFeatures(const QStringList &features)
{
    QStringList list;
    for (const QString &f : features)
    {
        QString key = f.trimmed().toLower();
        if (key.isEmpty() == false && list.contains(key) == false)
            list.append(key);
    }
    {
        QMutexLocker locker(&m_targetsMutex);
        m_features = list;
    }
    updateTargets();
    emit changed(id());
}

QStringList CurveFX::features() const
{
    return m_features;
}

void CurveFX::setWaveform(Waveform wave)
{
    m_waveform = wave;
    emit changed(id());
}

CurveFX::Waveform CurveFX::waveform() const
{
    return m_waveform;
}

QString CurveFX::waveformToString(Waveform wave)
{
    switch (wave)
    {
        case Triangle: return "Triangle";
        case RampUp: return "RampUp";
        case RampDown: return "RampDown";
        case Square: return "Square";
        case Random: return "Random";
        case Strobe: return "Strobe";
        case RandomStrobe: return "RandomStrobe";
        case Sine:
        default: return "Sine";
    }
}

CurveFX::Waveform CurveFX::stringToWaveform(const QString &str)
{
    if (str == "Triangle") return Triangle;
    if (str == "RampUp") return RampUp;
    if (str == "RampDown") return RampDown;
    if (str == "Square") return Square;
    if (str == "Random") return Random;
    if (str == "Strobe") return Strobe;
    if (str == "RandomStrobe") return RandomStrobe;
    return Sine;
}

QStringList CurveFX::waveformNames()
{
    return QStringList() << "Sine" << "Triangle" << "RampUp" << "RampDown" << "Square" << "Random" << "Strobe" << "RandomStrobe";
}

void CurveFX::setLow(int value)
{
    m_low = qBound(0, value, 255);
    emit changed(id());
}

int CurveFX::low() const
{
    return m_low;
}

void CurveFX::setHigh(int value)
{
    m_high = qBound(0, value, 255);
    emit changed(id());
}

int CurveFX::high() const
{
    return m_high;
}

void CurveFX::setPhaseOffset(int degrees)
{
    m_phaseOffset = ((degrees % 360) + 360) % 360;
    emit changed(id());
}

int CurveFX::phaseOffset() const
{
    return m_phaseOffset;
}

void CurveFX::setSpread(int degrees)
{
    m_spread = qBound(0, degrees, 1440);
    emit changed(id());
}

void CurveFX::setDuty(int percent)
{
    m_duty = qBound(1, percent, 99);
    emit changed(id());
}

int CurveFX::duty() const
{
    return m_duty;
}

int CurveFX::spread() const
{
    return m_spread;
}

void CurveFX::setRelative(bool relative)
{
    m_relative = relative;
    emit changed(id());
}

bool CurveFX::isRelative() const
{
    return m_relative;
}

qreal CurveFX::waveValue(Waveform wave, qreal pos, quint32 seed, qreal duty)
{
    pos = pos - qFloor(pos);
    // a hash of the seed: random, but the same for the whole cycle
    auto random = [seed]() {
        quint32 x = seed * 2654435761u + 0x9e3779b9u;
        x ^= x >> 16;
        x *= 0x45d9f3bu;
        x ^= x >> 16;
        return x;
    };
    switch (wave)
    {
        case Triangle:
            return pos < 0.5 ? pos * 2.0 : (1.0 - pos) * 2.0;
        case RampUp:
            return pos;
        case RampDown:
            return 1.0 - pos;
        case Square:
        case Strobe:
            return pos < duty ? 1.0 : 0.0;
        case RandomStrobe:
            // each cycle, one fixture in three flashes
            return random() % 3 == 0 && pos < duty ? 1.0 : 0.0;
        case Random:
            // a new random level every cycle, the same for the whole cycle
            return qreal(random() % 1000) / 999.0;
        case Sine:
        default:
            // starts at the bottom, like a fade in
            return (1.0 - qCos(pos * 2.0 * M_PI)) / 2.0;
    }
}

qreal CurveFX::valueAt(qreal pos, int index, int count) const
{
    qreal phase = qreal(m_phaseOffset) / 360.0;
    if (count > 1)
        phase += qreal(m_spread) / 360.0 * qreal(index) / qreal(count);
    qreal w = waveValue(m_waveform, pos + phase, quint32(index), m_duty / 100.0);
    return qreal(m_low) + (qreal(m_high) - qreal(m_low)) * w;
}

void CurveFX::slotFixtureRemoved(quint32 fxi_id)
{
    int count;
    {
        // DMX Total Control: a running curve lets go of its channels too
        // (they would keep the wave at the old address of the fixture)
        QMutexLocker locker(&m_targetsMutex);
        count = m_fixtures.removeAll(fxi_id);
        for (int i = m_targets.count() - 1; i >= 0; i--)
        {
            if (m_targets.at(i).fixture == fxi_id)
                m_droppedTargets.append(m_targets.takeAt(i));
        }
    }
    if (count > 0)
        emit changed(id());
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool CurveFX::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCFunction);
    saveXMLCommon(doc);

    saveXMLTempoType(doc);
    saveXMLSpeed(doc);
    saveXMLRunOrder(doc);

    doc->writeStartElement(KXMLDTCCurve);
    doc->writeAttribute("Wave", waveformToString(m_waveform));
    doc->writeAttribute("Low", QString::number(m_low));
    doc->writeAttribute("High", QString::number(m_high));
    doc->writeAttribute("Phase", QString::number(m_phaseOffset));
    doc->writeAttribute("Spread", QString::number(m_spread));
    if (m_duty != 50)
        doc->writeAttribute("Duty", QString::number(m_duty));
    if (m_relative)
        doc->writeAttribute("Relative", "1");
    doc->writeEndElement();

    for (const QString &feature : m_features)
        doc->writeTextElement(KXMLDTCCurveFeature, feature);
    for (quint32 fid : m_fixtures)
        doc->writeTextElement(KXMLDTCCurveFixture, QString::number(fid));

    doc->writeEndElement();
    return true;
}

bool CurveFX::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << "Function node not found!";
        return false;
    }

    if (root.attributes().value(KXMLQLCFunctionType).toString() != typeToString(Function::CurveFXType))
    {
        qWarning() << "Function is not a Curve FX!";
        return false;
    }

    m_fixtures.clear();
    m_features.clear();

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(root);
        }
        else if (root.name() == KXMLQLCFunctionTempoType)
        {
            loadXMLTempoType(root);
        }
        else if (root.name() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(root);
        }
        else if (root.name() == KXMLDTCCurve)
        {
            QXmlStreamAttributes attrs = root.attributes();
            m_waveform = stringToWaveform(attrs.value("Wave").toString());
            m_low = qBound(0, attrs.value("Low").toInt(), 255);
            m_high = attrs.hasAttribute("High") ? qBound(0, attrs.value("High").toInt(), 255) : 255;
            m_phaseOffset = attrs.value("Phase").toInt();
            m_spread = attrs.value("Spread").toInt();
            m_duty = attrs.hasAttribute("Duty") ? qBound(1, attrs.value("Duty").toInt(), 99) : 50;
            m_relative = attrs.value("Relative") == QLatin1String("1");
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLDTCCurveFeature)
        {
            // (what a newer version puts inside is skipped)
            QString feature = root.readElementText(QXmlStreamReader::SkipChildElements).trimmed().toLower();
            if (feature.isEmpty() == false && m_features.contains(feature) == false)
                m_features.append(feature);
        }
        else if (root.name() == KXMLDTCCurveFixture)
        {
            bool ok = false;
            quint32 fid = root.readElementText(QXmlStreamReader::SkipChildElements).trimmed().toUInt(&ok);
            if (ok && m_fixtures.contains(fid) == false)
                m_fixtures.append(fid);
        }
        else
        {
            root.skipCurrentElement();
        }
    }

    return true;
}

void CurveFX::postLoad()
{
    Doc *show = doc();
    if (show == NULL)
        return;

    // like the scenes: the fixtures that aren't in the show leave it
    QMutexLocker locker(&m_targetsMutex);
    m_fixtures.removeIf([show](quint32 fixture) { return show->fixture(fixture) == NULL; });
}

/*********************************************************************
 * Running
 *********************************************************************/

QList<CurveFX::Target> CurveFX::buildTargets(int *count) const
{
    // the channels of the features, fixture by fixture, in index order
    QList<Target> targets;
    int index = 0;
    for (quint32 fid : std::as_const(m_fixtures))
    {
        Fixture *fxi = doc()->fixture(fid);
        if (fxi == NULL)
            continue;

        bool found = false;
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch == NULL || m_features.contains(ch->name().toLower()) == false)
                continue;

            Target t;
            t.fixture = fid;
            t.channel = c;
            t.universe = fxi->universe();
            t.index = index;
            targets.append(t);
            found = true;
        }
        if (found)
            index++;
    }
    *count = index;
    return targets;
}

void CurveFX::updateTargets()
{
    if (isRunning() == false)
        return;

    QMutexLocker locker(&m_targetsMutex);
    int count = 0;
    QList<Target> targets = buildTargets(&count);
    // the channels it doesn't drive any more leave its faders
    for (const Target &old : std::as_const(m_targets))
    {
        bool kept = false;
        for (const Target &t : std::as_const(targets))
            kept = kept || (t.fixture == old.fixture && t.channel == old.channel);
        if (kept == false)
            m_droppedTargets.append(old);
    }
    m_targets = targets;
    m_targetCount = count;
}

void CurveFX::preRun(MasterTimer *timer)
{
    QMutexLocker locker(&m_targetsMutex);
    m_droppedTargets.clear();
    m_targets = buildTargets(&m_targetCount);
    m_cycleElapsed = 0;
    m_cycleNumber = 0;
    locker.unlock();

    Function::preRun(timer);
}

QSharedPointer<GenericFader> CurveFX::getFader(Universe *universe)
{
    if (universe == NULL)
        return QSharedPointer<GenericFader>();

    QSharedPointer<GenericFader> fader = m_fadersMap.value(universe->id(), QSharedPointer<GenericFader>());
    if (fader.isNull())
    {
        fader = universe->requestFader(Universe::Auto, playbackPriority());
        fader->adjustIntensity(getAttributeValue(Intensity));
        fader->setBlendMode(blendMode());
        fader->setName(name());
        fader->setParentFunctionID(id());
        m_fadersMap[universe->id()] = fader;
    }
    return fader;
}

void CurveFX::write(MasterTimer *timer, QList<Universe *> universes)
{
    // DMX Total Control: the targets don't change meanwhile, and the
    // channels of the fixtures removed leave the faders
    QMutexLocker locker(&m_targetsMutex);
    for (const Target &t : std::as_const(m_droppedTargets))
    {
        QSharedPointer<GenericFader> fader = t.universe < quint32(universes.count())
                ? m_fadersMap.value(universes.at(int(t.universe))->id()) : QSharedPointer<GenericFader>();
        if (fader.isNull() == false)
            fader->removeChannel(t.fixture, t.channel);
    }
    m_droppedTargets.clear();

    if (isPaused())
        return;

    uint loop = tempoType() == Beats ? beatsToTime(duration(), timer->beatTimeDuration()) : duration();
    if (loop == 0 || m_targets.isEmpty())
        return;

    advanceFrameTick();

    // BeatGO and pulses: on to the next division
    int jumps = takeDivisionJumps();
    if (jumps > 0)
    {
        uint division = qMax(uint(1), loop / uint(divisions()));
        m_cycleElapsed = (m_cycleElapsed / division + uint(jumps)) * division;
    }
    else
    {
        m_cycleElapsed += frameTick();
    }

    if (m_cycleElapsed >= loop)
    {
        m_cycleElapsed %= loop;
        m_cycleNumber++;

        if (runOrder() == SingleShot)
        {
            stop(FunctionParent::master());
            return;
        }
        CycleResult result = cycleCompleted();
        if (result == CycleStop)
        {
            stop(FunctionParent::master());
            return;
        }
        if (result == CycleHold)
        {
            // the last frame of the cycle stays
            setPause(true);
            return;
        }
    }

    qreal pos = qreal(m_cycleElapsed) / qreal(loop);
    if (runOrder() == PingPong && (m_cycleNumber % 2) == 1)
        pos = 1.0 - pos;

    const qreal size = getAttributeValue(Size);
    const qreal phase = (qreal(m_phaseOffset) + getAttributeValue(Phase)) / 360.0;
    const qreal mid = (qreal(m_low) + qreal(m_high)) / 2.0;
    const qreal swing = (qreal(m_high) - qreal(m_low)) / 2.0;

    for (const Target &t : std::as_const(m_targets))
    {
        if (t.universe >= quint32(universes.count()))
            continue;

        qreal p = pos + phase;
        if (m_targetCount > 1)
            p += qreal(m_spread) / 360.0 * qreal(t.index) / qreal(m_targetCount);

        // random levels change every cycle, and differ from fixture to fixture
        qreal w = waveValue(m_waveform, p, m_cycleNumber * 7919u + quint32(t.index), m_duty / 100.0);
        qreal level = (w * 2.0 - 1.0) * swing * size;
        quint32 value = m_relative
                ? quint32(qBound(0.0, 128.0 + level, 255.0))
                : quint32(qBound(0.0, mid + level, 255.0));

        Universe *universe = universes.at(int(t.universe));
        QSharedPointer<GenericFader> fader = getFader(universe);
        if (fader.isNull())
            continue;

        const bool relative = m_relative;
        fader->updateChannel(doc(), universe, t.fixture, t.channel, [value, relative](FadeChannel &fc)
        {
            if (relative)
                fc.addFlag(FadeChannel::Relative);
            fc.setStart(fc.current());
            fc.setTarget(value);
            fc.setElapsed(0);
            fc.setReady(false);
            fc.setFadeTime(0);
        });
    }

    incrementElapsed();
}

void CurveFX::postRun(MasterTimer *timer, QList<Universe *> universes)
{
    uint fadeout = overrideFadeOutSpeed() == defaultSpeed() ? fadeOutSpeed() : overrideFadeOutSpeed();
    if (fadeout == 0 || m_relative)
    {
        dismissAllFaders();
    }
    else
    {
        if (tempoType() == Beats)
            fadeout = beatsToTime(fadeout, timer->beatTimeDuration());
        for (QSharedPointer<GenericFader> fader : std::as_const(m_fadersMap))
        {
            if (!fader.isNull())
                fader->setFadeOut(true, fadeout);
        }
        m_fadersMap.clear();
    }

    Function::postRun(timer, universes);
}

int CurveFX::adjustAttribute(qreal fraction, int attributeId)
{
    int attrIndex = Function::adjustAttribute(fraction, attributeId);

    if (attrIndex == Intensity)
    {
        for (QSharedPointer<GenericFader> fader : std::as_const(m_fadersMap))
        {
            if (!fader.isNull())
                fader->adjustIntensity(getAttributeValue(Function::Intensity));
        }
    }
    return attrIndex;
}
