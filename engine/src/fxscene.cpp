/*
  DMX Total Control
  fxscene.cpp

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
#include <QRandomGenerator>
#include <QMutexLocker>
#include <QLocale>
#include <QtMath>
#include <QSet>
#include <QDebug>
#include <QIcon>
#include <algorithm>
#include <climits>
#include <cmath>

#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "fixturegroup.h"
#include "qlcfixturehead.h"
#include "qlcfixturemode.h"
#include "qlcchannel.h"
#include "universe.h"
#include "fxscene.h"
#include "fixture.h"
#include "doc.h"

FXScene::FXScene(Doc *doc)
    : Function(doc, Function::FXSceneType)
    , m_targetsDirty(true)
    , m_valuesStarted(false)
    , m_fadeElapsed(0)
    , m_fadersPaused(false)
{
    setName(tr("New FX"));

    registerAttribute(tr("Size"), Function::LastWins, 0.0, 2.0, 1.0);
    registerAttribute(tr("Phase"), Function::LastWins, 0.0, 359.0, 0.0);
}

FXScene::~FXScene()
{
}

QIcon FXScene::getIcon() const
{
    return QIcon(":/efx.png");
}

quint32 FXScene::totalDuration()
{
    return cycleDuration();
}

bool FXScene::Shape::operator==(const Shape &o) const
{
    return mode == o.mode && figure == o.figure && swing == o.swing && swingTilt == o.swingTilt &&
           phase == o.phase && rotate == o.rotate && panInvert == o.panInvert &&
           tiltInvert == o.tiltInvert && reverse == o.reverse;
}

bool FXScene::Own::isEmpty() const
{
    return !mode && !figure && !swing && !swingTilt && !phase && !rotate &&
           !panInvert && !tiltInvert && !reverse;
}

bool FXScene::Own::operator==(const Own &o) const
{
    return mode == o.mode && figure == o.figure && swing == o.swing && swingTilt == o.swingTilt &&
           phase == o.phase && rotate == o.rotate && panInvert == o.panInvert &&
           tiltInvert == o.tiltInvert && reverse == o.reverse;
}

FXScene::Shape FXScene::Layer::shape() const
{
    return { mode, figure, swing, swingTilt, phase, rotate, panInvert, tiltInvert, reverse };
}

FXScene::Shape FXScene::Layer::shapeOf(quint32 fixture) const
{
    Shape s = shape();
    if (own.isEmpty())
        return s;
    auto it = own.constFind(fixture);
    if (it == own.constEnd())
        return s;
    const Own &o = it.value();
    s.mode = o.mode.value_or(s.mode);
    s.figure = o.figure.value_or(s.figure);
    s.swing = o.swing.value_or(s.swing);
    s.swingTilt = o.swingTilt.value_or(s.swingTilt);
    s.phase = o.phase.value_or(s.phase);
    s.rotate = o.rotate.value_or(s.rotate);
    s.panInvert = o.panInvert.value_or(s.panInvert);
    s.tiltInvert = o.tiltInvert.value_or(s.tiltInvert);
    s.reverse = o.reverse.value_or(s.reverse);
    return s;
}

QMap<quint32, FXScene::Own> FXScene::Layer::tidyOwn() const
{
    QMap<quint32, Own> tidy;
    const bool panTilt = isPanTilt();
    for (auto it = own.cbegin(); it != own.cend(); ++it)
    {
        if (fixtures.isEmpty() == false && fixtures.contains(it.key()) == false)
            continue;
        Own o = it.value();
        // pan/tilt draws a figure, the others a mode: what the layer doesn't use goes
        if (panTilt || o.mode == mode) o.mode.reset();
        if (panTilt == false || o.figure == figure) o.figure.reset();
        if (o.swing == swing) o.swing.reset();
        if (panTilt == false || o.swingTilt == swingTilt) o.swingTilt.reset();
        if (o.phase == phase) o.phase.reset();
        if (panTilt == false || o.rotate == rotate) o.rotate.reset();
        if (panTilt == false || o.panInvert == panInvert) o.panInvert.reset();
        if (panTilt == false || o.tiltInvert == tiltInvert) o.tiltInvert.reset();
        if (panTilt == false || o.reverse == reverse) o.reverse.reset();
        if (o.isEmpty() == false)
            tidy.insert(it.key(), o);
    }
    return tidy;
}

bool FXScene::Layer::operator==(const Layer &o) const
{
    // DMX Total Control: what a layer doesn't use is not saved, so it doesn't
    // count either (a layer equals itself after a save and a load): the figure
    // and the tilt swing of a layer that is not pan/tilt, the size of no
    // grouping, the seed of an order that is not random
    const bool panTilt = isPanTilt();
    return param == o.param && mode == o.mode && swing == o.swing &&
           (panTilt == false || (figure == o.figure && swingTilt == o.swingTilt)) &&
           qFuzzyCompare(bpm + 1, o.bpm + 1) && syncBpm == o.syncBpm &&
           qFuzzyCompare(multiplier, o.multiplier) && wave == o.wave && step == o.step &&
           grouping == o.grouping && (grouping == NoGrouping || groupSize == o.groupSize) &&
           order == o.order && (order != Shuffled || seed == o.seed) &&
           phase == o.phase && wait == o.wait && rotate == o.rotate && panInvert == o.panInvert &&
           tiltInvert == o.tiltInvert && reverse == o.reverse && linked == o.linked && fixtures == o.fixtures &&
           (own.isEmpty() && o.own.isEmpty() ? true : tidyOwn() == o.tidyOwn());
}

/*********************************************************************
 * Copying
 *********************************************************************/

Function *FXScene::createCopy(Doc *doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function *copy = new FXScene(doc);
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

bool FXScene::copyFrom(const Function *function)
{
    const FXScene *fx = qobject_cast<const FXScene *>(function);
    if (fx == NULL)
        return false;

    QMutexLocker locker(&m_mutex);
    QMutexLocker otherLocker(&fx->m_mutex);
    m_fixtures = fx->m_fixtures;
    m_values = fx->m_values;
    m_layers = fx->m_layers;
    m_targetsDirty = true;
    otherLocker.unlock();
    locker.unlock();

    return Function::copyFrom(function);
}

/*********************************************************************
 * Contents
 *********************************************************************/

void FXScene::setFixtures(const QList<quint32> &fixtures)
{
    {
        QMutexLocker locker(&m_mutex);
        QList<quint32> unique;
        for (quint32 fid : fixtures)
            if (unique.contains(fid) == false)
                unique.append(fid);
        if (unique == m_fixtures)
            return;
        m_fixtures = unique;
        m_targetsDirty = true;
    }
    emit changed(id());
}

QList<quint32> FXScene::fixtures() const
{
    QMutexLocker locker(&m_mutex);
    return m_fixtures;
}

void FXScene::setValues(const QHash<QPair<quint32, quint32>, uchar> &values)
{
    {
        QMutexLocker locker(&m_mutex);
        if (values == m_values)
            return;
        m_values = values;
        for (auto it = values.cbegin(); it != values.cend(); ++it)
            if (m_fixtures.contains(it.key().first) == false)
                m_fixtures.append(it.key().first);
        m_targetsDirty = true;
    }
    emit changed(id());
}

QHash<QPair<quint32, quint32>, uchar> FXScene::values() const
{
    QMutexLocker locker(&m_mutex);
    return m_values;
}

void FXScene::setValue(quint32 fixture, quint32 channel, uchar value)
{
    {
        QMutexLocker locker(&m_mutex);
        m_values.insert(qMakePair(fixture, channel), value);
        if (m_fixtures.contains(fixture) == false)
            m_fixtures.append(fixture);
        m_targetsDirty = true;
    }
    emit changed(id());
}

void FXScene::setLayers(const QList<Layer> &layers)
{
    {
        QMutexLocker locker(&m_mutex);
        if (layers == m_layers)
            return;
        // only what changes the channels needs new targets: the speed,
        // the swing... are read every frame
        bool sameTargets = layers.count() == m_layers.count();
        for (int i = 0; sameTargets && i < layers.count(); i++)
        {
            const Layer &a = layers.at(i), &b = m_layers.at(i);
            sameTargets = a.param == b.param && a.fixtures == b.fixtures && a.grouping == b.grouping &&
                          a.groupSize == b.groupSize && a.order == b.order && a.seed == b.seed;
        }
        // DMX Total Control: each layer goes on from where its wave is, also
        // when the ones before it go or come (the positions went by index:
        // the layers after a stopped one jumped to its place): the same
        // parameter on fixtures it had. A new layer starts at 0
        QList<qreal> positions(layers.count(), -1.0);
        QList<bool> taken(m_layers.count(), false);
        for (int pass = 0; pass < 2; pass++)
        {
            for (int i = 0; i < layers.count(); i++)
            {
                if (positions.at(i) >= 0)
                    continue;
                const Layer &layer = layers.at(i);
                for (int j = 0; j < m_layers.count(); j++)
                {
                    const Layer &old = m_layers.at(j);
                    if (taken.at(j) || old.param != layer.param)
                        continue;
                    // first the layers on the same fixtures, then on some of them
                    bool match = old.fixtures == layer.fixtures;
                    for (int f = 0; pass == 1 && match == false && f < layer.fixtures.count(); f++)
                        match = old.fixtures.contains(layer.fixtures.at(f));
                    if (match == false)
                        continue;
                    taken[j] = true;
                    positions[i] = m_layerPos.value(j, 0.0);
                    break;
                }
            }
        }
        // linked layers share the speed and the timing: they go together
        // (one linked later takes the place of the others)
        qreal linkedPos = -1.0;
        for (int i = 0; i < layers.count() && linkedPos < 0; i++)
            if (layers.at(i).linked && positions.at(i) >= 0)
                linkedPos = positions.at(i);
        for (int i = 0; i < layers.count(); i++)
        {
            if (layers.at(i).linked && linkedPos >= 0)
                positions[i] = linkedPos;
            else if (positions.at(i) < 0)
                positions[i] = 0.0;
        }
        m_layerPos = positions;

        m_layers = layers;
        for (const Layer &layer : layers)
            for (quint32 fid : layer.fixtures)
                if (m_fixtures.contains(fid) == false)
                    m_fixtures.append(fid);
        if (sameTargets == false)
            m_targetsDirty = true;
    }
    emit changed(id());
}

QList<FXScene::Layer> FXScene::layers() const
{
    QMutexLocker locker(&m_mutex);
    return m_layers;
}

uint FXScene::cycleDuration() const
{
    QMutexLocker locker(&m_mutex);
    qreal longest = 0;
    for (const Layer &layer : m_layers)
    {
        qreal bpm = (layer.syncBpm ? 120.0 : layer.bpm) * layer.multiplier;
        if (bpm <= 0)
            continue;
        qreal cycle = 60000.0 / bpm;
        // a step over all the fixtures runs once per fixture
        qreal runs = layer.step > 0 ? layer.step : layer.step < 0 ? qMax(1, int(layer.fixtures.isEmpty() ? m_fixtures.count() : layer.fixtures.count())) : 1;
        longest = qMax(longest, cycle * runs + layer.wait);
    }
    return uint(longest);
}

void FXScene::slotFixtureRemoved(quint32 fxi_id)
{
    {
        QMutexLocker locker(&m_mutex);
        removeFixturesLocked([fxi_id](quint32 fixture) { return fixture == fxi_id; });
    }
    emit changed(id());
}

bool FXScene::removeFixturesLocked(const std::function<bool(quint32)> &remove)
{
    bool removed = m_fixtures.removeIf(remove) > 0;
    for (auto it = m_values.begin(); it != m_values.end();)
    {
        if (remove(it.key().first))
        {
            it = m_values.erase(it);
            removed = true;
        }
        else
        {
            ++it;
        }
    }
    // DMX Total Control: a layer on its own fixtures that loses the last one
    // goes (with none it would move every fixture of the scene)
    for (int l = m_layers.count() - 1; l >= 0; l--)
    {
        Layer &layer = m_layers[l];
        if (layer.fixtures.removeIf(remove) == 0)
            continue;
        removed = true;
        if (layer.fixtures.isEmpty())
            m_layers.removeAt(l);
        else
            layer.own = layer.tidyOwn();
    }
    if (removed)
        m_targetsDirty = true;
    return removed;
}

/*********************************************************************
 * The shapes
 *********************************************************************/

static qreal squareWave(qreal pos) { return pos < 0.5 ? 1.0 : -1.0; }

static qreal triangleWave(qreal pos)
{
    if (pos < 0.25)
        return pos * 4.0;
    if (pos < 0.75)
        return 2.0 - pos * 4.0;
    return pos * 4.0 - 4.0;
}

qreal FXScene::modeValue(int mode, qreal pos)
{
    pos = pos - std::floor(pos);
    const qreal sine = std::sin(2.0 * M_PI * pos);
    const qreal bump = (1.0 - std::cos(2.0 * M_PI * pos)) / 2.0;

    switch (mode)
    {
        case 1: return sine;                            // up, back, below, back
        case 2: return -sine;                           // below first
        case 3: case 4: return bump;                    // up and back
        case 5: case 6: return -bump;                   // down and back
        case 7: return squareWave(pos);                 // snap up, snap below
        case 8: return -squareWave(pos);
        case 9: case 10: return pos < 0.5 ? 1.0 : 0.0;  // snap up and back (chases)
        case 11: case 12: return pos < 0.5 ? -1.0 : 0.0;
        case 13: case 14: return -squareWave(pos);      // snap down, snap above
        case 15: return 1.0 - pos;                      // snap up, fade back
        case 16: return pos;                            // fade up, snap back
        case 17: return pos - 1.0;                      // snap down, fade back
        case 18: return -pos;                           // fade down, snap back
        case 19: return triangleWave(pos);              // linear above and below
        case 20: return -triangleWave(pos);
        case 21: return 1.0 - 2.0 * pos;                // ramps through the base
        case 22: return 2.0 * pos - 1.0;
        default: return sine;
    }
}

bool FXScene::modeReversed(int mode)
{
    switch (mode)
    {
        case 2: case 4: case 6: case 8: case 10: case 12: case 14: case 20:
            return true;
        default:
            return false;
    }
}

/** A point along a closed polygon of $points, at $pos (0..1) */
static QPointF alongPolygon(const QList<QPointF> &points, qreal pos)
{
    int n = points.count();
    qreal p = (pos - std::floor(pos)) * n;
    int i = qMin(int(p), n - 1);
    qreal f = p - i;
    const QPointF &a = points.at(i);
    const QPointF &b = points.at((i + 1) % n);
    return a + (b - a) * f;
}

QPointF FXScene::figurePoint(int figure, qreal pos)
{
    pos = pos - std::floor(pos);
    const qreal a = 2.0 * M_PI * pos;

    switch (figure)
    {
        case Eight:
            return QPointF(std::sin(a), std::sin(2.0 * a));
        case Square:
            return alongPolygon({ QPointF(-1, -1), QPointF(1, -1), QPointF(1, 1), QPointF(-1, 1) }, pos + 0.125);
        case Triangle:
            return alongPolygon({ QPointF(0, -1), QPointF(1, 1), QPointF(-1, 1) }, pos);
        case Diamond:
            return alongPolygon({ QPointF(0, -1), QPointF(1, 0), QPointF(0, 1), QPointF(-1, 0) }, pos);
        case PanLine:
            return QPointF(std::sin(a), 0);
        case TiltLine:
            return QPointF(0, std::sin(a));
        case Diagonal:
            return QPointF(std::sin(a), std::sin(a));
        case Leaf:
            return QPointF(std::sin(2.0 * a) * std::cos(a), std::sin(2.0 * a) * std::sin(a));
        case Lissajous:
            return QPointF(std::sin(3.0 * a), std::sin(2.0 * a));
        case Ballyhoo:
            return QPointF(0.7 * std::sin(a) + 0.3 * std::sin(5.0 * a),
                           0.6 * std::sin(2.0 * a) + 0.4 * std::cos(3.0 * a));
        case Cross:
            return pos < 0.5 ? QPointF(std::sin(4.0 * M_PI * pos), 0) : QPointF(0, std::sin(4.0 * M_PI * pos));
        case Circle:
        default:
            return QPointF(std::sin(a), std::cos(a));
    }
}

QString FXScene::attributeOf(const QLCChannel *ch)
{
    if (ch == NULL)
        return QString();

    const QString name = ch->name().trimmed().toLower();

    // the preset says it best, when the definition has one (DMX Total Control:
    // attributes by function, not by channel name, as consoles do)
    switch (ch->preset())
    {
        case QLCChannel::IntensityHue: case QLCChannel::IntensityHueFine:
            return QStringLiteral("hue");
        case QLCChannel::IntensitySaturation: case QLCChannel::IntensitySaturationFine:
            return QStringLiteral("saturation");
        case QLCChannel::IntensityLightness: case QLCChannel::IntensityLightnessFine:
            return QStringLiteral("lightness");
        case QLCChannel::SpeedPanSlowFast: case QLCChannel::SpeedPanFastSlow:
        case QLCChannel::SpeedTiltSlowFast: case QLCChannel::SpeedTiltFastSlow:
        case QLCChannel::SpeedPanTiltSlowFast: case QLCChannel::SpeedPanTiltFastSlow:
            return QStringLiteral("ptspeed");
        case QLCChannel::ColorMacro:
            return QStringLiteral("colormacro");
        case QLCChannel::ColorWheel: case QLCChannel::ColorWheelFine:
            return QStringLiteral("colorwheel");
        case QLCChannel::ColorCTOMixer:
            return QStringLiteral("cto");
        case QLCChannel::ColorCTCMixer:
            return QStringLiteral("ctc");
        case QLCChannel::ColorCTBMixer:
            return QStringLiteral("ctb");
        case QLCChannel::GoboWheel: case QLCChannel::GoboWheelFine:
            return QStringLiteral("gobo");
        case QLCChannel::GoboIndex: case QLCChannel::GoboIndexFine:
            return QStringLiteral("goborotation");
        case QLCChannel::ShutterStrobeSlowFast: case QLCChannel::ShutterStrobeFastSlow:
            return QStringLiteral("shutter");
        case QLCChannel::ShutterIrisMinToMax: case QLCChannel::ShutterIrisMaxToMin: case QLCChannel::ShutterIrisFine:
            return QStringLiteral("iris");
        case QLCChannel::BeamFocusNearFar: case QLCChannel::BeamFocusFarNear: case QLCChannel::BeamFocusFine:
            return QStringLiteral("focus");
        case QLCChannel::BeamZoomSmallBig: case QLCChannel::BeamZoomBigSmall: case QLCChannel::BeamZoomFine:
            return QStringLiteral("zoom");
        case QLCChannel::PrismRotationSlowFast: case QLCChannel::PrismRotationFastSlow:
            return QStringLiteral("prismrotation");
        default:
            break;
    }

    // else its group, and some words of its name
    switch (ch->group())
    {
        case QLCChannel::Pan:
            return QStringLiteral("pan");
        case QLCChannel::Tilt:
            return QStringLiteral("tilt");
        case QLCChannel::Intensity:
            switch (ch->colour())
            {
                case QLCChannel::NoColour: return QStringLiteral("dimmer");
                case QLCChannel::Red: return QStringLiteral("red");
                case QLCChannel::Green: return QStringLiteral("green");
                case QLCChannel::Blue: return QStringLiteral("blue");
                case QLCChannel::White: return QStringLiteral("white");
                case QLCChannel::Amber: return QStringLiteral("amber");
                case QLCChannel::UV: return QStringLiteral("uv");
                case QLCChannel::Cyan: return QStringLiteral("cyan");
                case QLCChannel::Magenta: return QStringLiteral("magenta");
                case QLCChannel::Yellow: return QStringLiteral("yellow");
                case QLCChannel::Lime: return QStringLiteral("lime");
                case QLCChannel::Indigo: return QStringLiteral("indigo");
                default: break;
            }
            break;
        case QLCChannel::Colour:
            if (name.contains("cto") || name.contains("ctc") || name.contains("ctb") || name.contains("temperat"))
                return QStringLiteral("cto");
            if (name.contains("macro"))
                return QStringLiteral("colormacro");
            return QStringLiteral("colorwheel");
        case QLCChannel::Gobo:
            if (name.contains("rot") || name.contains("index") || name.contains("spin"))
                return QStringLiteral("goborotation");
            return QStringLiteral("gobo");
        case QLCChannel::Shutter:
            return name.contains("iris") ? QStringLiteral("iris") : QStringLiteral("shutter");
        case QLCChannel::Beam:
            for (const char *word : { "zoom", "focus", "iris", "frost" })
                if (name.contains(QLatin1String(word)))
                    return QString::fromLatin1(word);
            break;
        case QLCChannel::Prism:
            return name.contains("rot") ? QStringLiteral("prismrotation") : QStringLiteral("prism");
        case QLCChannel::Speed:
            if (name.contains("pan") || name.contains("tilt") || name.contains("p/t") || name.contains("movement"))
                return QStringLiteral("ptspeed");
            break;
        default:
            break;
    }
    return name;
}

QString FXScene::paramOf(const QLCChannel *ch)
{
    if (ch == NULL || ch->controlByte() == QLCChannel::LSB)
        return QString();
    return attributeOf(ch);
}

int FXScene::headOf(const Fixture *fixture, quint32 channel)
{
    if (fixture == NULL || fixture->fixtureMode() == NULL || fixture->heads() < 2)
        return -1;
    for (int h = 0; h < fixture->heads(); h++)
        if (fixture->head(h).channels().contains(channel))
            return h;
    return -1;
}

quint32 FXScene::channelFor(const Fixture *fixture, const QString &param, quint32 *fine)
{
    if (fine != NULL)
        *fine = QLCChannel::invalid();
    if (fixture == NULL)
        return QLCChannel::invalid();

    // the channel of the whole fixture first (the master dimmer of a bar), then
    // the first head; then, for shows saved before, a channel with that name
    quint32 found = QLCChannel::invalid();
    for (int pass = 0; pass < 3 && found == QLCChannel::invalid(); pass++)
    {
        for (quint32 c = 0; c < fixture->channels(); c++)
        {
            const QLCChannel *ch = fixture->channel(c);
            bool match = pass < 2 ? paramOf(ch) == param && (pass == 1 || headOf(fixture, c) < 0)
                                  : ch != NULL && ch->controlByte() == QLCChannel::MSB &&
                                    ch->name().trimmed().toLower() == param;
            if (match)
            {
                found = c;
                break;
            }
        }
    }
    if (found == QLCChannel::invalid() || fine == NULL)
        return found;

    // its fine channel: the LSB of the same group (and colour), right after it if there are several
    const QLCChannel *msb = fixture->channel(found);
    for (quint32 c = 0; c < fixture->channels(); c++)
    {
        const QLCChannel *ch = fixture->channel(c);
        if (ch != NULL && ch->controlByte() == QLCChannel::LSB && ch->group() == msb->group() &&
            ch->colour() == msb->colour() && (msb->group() != QLCChannel::Intensity || c == found + 1))
        {
            *fine = c;
            break;
        }
    }
    return found;
}

QList<int> FXScene::groupSets(const Doc *doc, const QList<quint32> &order, int grouping, int groupSize, int *sets)
{
    const int n = order.count();
    const int size = qMax(1, groupSize);
    QList<int> result;
    int count = n;

    switch (grouping)
    {
        case Every:
            for (int i = 0; i < n; i++)
                result.append(i % size);
            count = qMin(size, n);
            break;
        case Block:
            for (int i = 0; i < n; i++)
                result.append(i / size);
            count = (n + size - 1) / size;
            break;
        case Divide:
        {
            int per = qMax(1, (n + size - 1) / size);
            for (int i = 0; i < n; i++)
                result.append(i / per);
            count = (n + per - 1) / per;
            break;
        }
        case Mirror:
        {
            // from the outside in: the first and the last are one set
            int half = qMax(1, size / 2);
            for (int i = 0; i < n; i++)
                result.append(qMin(i, n - 1 - i) / half);
            count = ((n + 1) / 2 + half - 1) / half;
            break;
        }
        case ByGroup:
        {
            // the fixture groups of the patch, in their order (not the hidden ones of the effects)
            QList<quint32> groupOf;
            QList<FixtureGroup *> groups;
            if (doc != NULL)
            {
                for (FixtureGroup *g : doc->fixtureGroups())
                    if (g->name().endsWith(QLatin1String(" (FX)")) == false &&
                        g->name().endsWith(QLatin1String(" (FX 2D)")) == false)
                        groups.append(g);
            }
            QList<quint32> setIds;
            for (quint32 fid : order)
            {
                quint32 gid = FixtureGroup::invalidId();
                for (FixtureGroup *g : groups)
                {
                    if (g->fixtureList().contains(fid))
                    {
                        gid = g->id();
                        break;
                    }
                }
                if (setIds.contains(gid) == false)
                    setIds.append(gid);
                result.append(int(setIds.indexOf(gid)));
            }
            count = setIds.count();
            break;
        }
        default:
            for (int i = 0; i < n; i++)
                result.append(i);
            break;
    }

    if (sets != NULL)
        *sets = qMax(1, count);
    return result;
}

QList<quint32> FXScene::orderedFixtures(const Layer &layer, const QList<quint32> &fixtures)
{
    QList<quint32> order = layer.fixtures.isEmpty() ? fixtures : layer.fixtures;
    switch (layer.order)
    {
        case Reversed:
            std::reverse(order.begin(), order.end());
            break;
        case Shuffled:
        {
            QRandomGenerator random(quint32(qMax(1, layer.seed)));
            for (int i = order.count() - 1; i > 0; i--)
                order.swapItemsAt(i, int(random.bounded(i + 1)));
            break;
        }
        case Sorted:
            std::sort(order.begin(), order.end());
            break;
        default:
            break;
    }
    return order;
}

qreal FXScene::positionOf(const Layer &layer, qreal cyclePos, int set, int sets)
{
    return positionOf(layer, layer.shape(), cyclePos, set, sets);
}

qreal FXScene::positionOf(const Layer &layer, const Shape &shape, qreal cyclePos, int set, int sets)
{
    cyclePos += qreal(shape.phase) / 360.0;
    sets = qMax(1, sets);
    // from the last to the first set
    if (modeReversed(shape.mode) && layer.isPanTilt() == false)
        set = sets - 1 - set;

    // -1: as many as there are sets ("amount X")
    const int step = layer.step < 0 ? sets : layer.step;
    const int wave = layer.wave < 0 ? sets : layer.wave;

    if (step > 0)
    {
        // one set after the other: each one does the whole shape, then rests
        const int slotCount = step;
        const int slot = set % slotCount;
        qreal waitCycles = layer.wait > 0 ? qreal(layer.wait) / 1000.0 * layer.bpm * layer.multiplier / 60.0 : 0.0;
        qreal period = slotCount + waitCycles;
        qreal s = cyclePos - std::floor(cyclePos / period) * period;
        qreal local = s - slot;
        return local >= 0.0 && local < 1.0 ? local : -1.0;
    }

    qreal offset = 0.0;
    if (wave > 0)
        offset = qreal(set % wave) / qreal(wave);

    if (layer.wait > 0)
    {
        // the whole run (the last set included), then the wait at the base
        qreal run = 1.0 + (wave > 0 ? qreal(wave - 1) / qreal(wave) : 0.0);
        qreal waitCycles = qreal(layer.wait) / 1000.0 * qMax(0.0, layer.bpm) * layer.multiplier / 60.0;
        qreal period = run + waitCycles;
        qreal s = cyclePos - std::floor(cyclePos / period) * period;
        qreal local = s - offset;
        return local >= 0.0 && local < 1.0 ? local : -1.0;
    }

    qreal local = cyclePos - offset;
    return local - std::floor(local);
}

QVariantList FXScene::previewLayer(int index, qreal cyclePos) const
{
    QVariantList list;
    QMutexLocker locker(&m_mutex);
    if (index < 0 || index >= m_layers.count())
        return list;

    const Layer &layer = m_layers.at(index);
    QList<quint32> order = orderedFixtures(layer, m_fixtures);
    int sets = 1;
    QList<int> setOf = groupSets(doc(), order, layer.grouping, layer.groupSize, &sets);
    // pan/tilt: -1..1 is the widest swing of its fixtures
    int scale = qMax(1, qMax(layer.swing, layer.swingTilt));
    for (auto it = layer.own.cbegin(); it != layer.own.cend(); ++it)
        scale = qMax(scale, qMax(it->swing.value_or(0), it->swingTilt.value_or(0)));
    // a mirror: the ones on the other side go with their twins
    const QList<bool> twins = mirrorTwins(layer, order);
    for (int i = 0; i < order.count(); i++)
    {
        const Shape shape = layer.shapeOf(order.at(i));
        const int set = i < twins.count() && twins.at(i) ? setOf.at(order.count() - 1 - i) : setOf.at(i);
        qreal pos = positionOf(layer, shape, cyclePos, set, sets);
        if (layer.isPanTilt())
        {
            // as the layer moves it
            QPointF p = pos < 0 ? QPointF(0, 0) : panTiltPoint(shape, pos);
            list.append(QVariantMap({ { "fixture", order.at(i) },
                                      { "x", p.x() * qreal(shape.swing) / qreal(scale) },
                                      { "y", p.y() * qreal(shape.swingTilt) / qreal(scale) } }));
        }
        else
        {
            list.append(QVariantMap({ { "fixture", order.at(i) }, { "value", pos < 0 ? 0.0 : modeValue(shape.mode, pos) } }));
        }
    }
    return list;
}

bool FXScene::isFlipped(const Shape &shape)
{
    return shape.reverse != (shape.panInvert != shape.tiltInvert);
}

QList<bool> FXScene::mirrorTwins(const Layer &layer, const QList<quint32> &order)
{
    QList<bool> twins;
    if (layer.isPanTilt() == false || layer.own.isEmpty())
        return twins;

    // the fixtures that go the other way round from the layer, and on which side
    const int n = int(order.count());
    const bool base = isFlipped(layer.shape());
    QList<bool> differs(n, false);
    bool before = false, after = false;
    for (int i = 0; i < n; i++)
    {
        differs[i] = isFlipped(layer.shapeOf(order.at(i))) != base;
        // (the one in the middle of an odd count is its own twin)
        if (differs.at(i) && 2 * i + 1 < n)
            before = true;
        if (differs.at(i) && 2 * i + 1 > n)
            after = true;
    }
    // none, or on both sides (every other one the other way round): no mirror
    if (before == after)
        return twins;

    twins.resize(n, false);
    for (int i = 0; i < n; i++)
        twins[i] = differs.at(i) && differs.at(n - 1 - i) == false;
    return twins;
}

QPointF FXScene::panTiltPoint(const Shape &shape, qreal pos)
{
    QPointF p = figurePoint(shape.figure, shape.reverse ? -pos : pos);
    if (shape.rotate != 0)
    {
        qreal r = qDegreesToRadians(qreal(shape.rotate));
        p = QPointF(p.x() * std::cos(r) - p.y() * std::sin(r), p.x() * std::sin(r) + p.y() * std::cos(r));
    }
    if (shape.panInvert)
        p.setX(-p.x());
    if (shape.tiltInvert)
        p.setY(-p.y());
    return p;
}

/*********************************************************************
 * Save & Load
 *********************************************************************/

QString FXScene::groupingToString(int grouping)
{
    switch (grouping)
    {
        case Every: return QStringLiteral("every");
        case Block: return QStringLiteral("block");
        case Divide: return QStringLiteral("divide");
        case Mirror: return QStringLiteral("mirror");
        case ByGroup: return QStringLiteral("group");
        default: return QStringLiteral("off");
    }
}

int FXScene::stringToGrouping(const QString &str)
{
    if (str == QLatin1String("every")) return Every;
    if (str == QLatin1String("block")) return Block;
    if (str == QLatin1String("divide")) return Divide;
    if (str == QLatin1String("mirror")) return Mirror;
    if (str == QLatin1String("group")) return ByGroup;
    return NoGrouping;
}

QString FXScene::orderToString(int order)
{
    switch (order)
    {
        case Reversed: return QStringLiteral("reverse");
        case Shuffled: return QStringLiteral("random");
        case Sorted: return QStringLiteral("sort");
        default: return QStringLiteral("selection");
    }
}

int FXScene::stringToOrder(const QString &str)
{
    if (str == QLatin1String("reverse")) return Reversed;
    if (str == QLatin1String("random")) return Shuffled;
    if (str == QLatin1String("sort")) return Sorted;
    return SelectionOrder;
}

static QString idList(const QList<quint32> &ids)
{
    QStringList list;
    for (quint32 id : ids)
        list.append(QString::number(id));
    return list.join(',');
}

void FXScene::saveLayerXML(QXmlStreamWriter *doc, const Layer &layer)
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLDTCFXLayer);
    doc->writeAttribute("Param", layer.param);
    if (layer.isPanTilt())
    {
        doc->writeAttribute("Figure", QString::number(layer.figure));
        doc->writeAttribute("SwingTilt", QString::number(layer.swingTilt));
    }
    doc->writeAttribute("Mode", QString::number(layer.mode));
    doc->writeAttribute("Swing", QString::number(layer.swing));
    // DMX Total Control: the shortest text that reads back exactly (a tapped
    // BPM has more than the 6 digits of the default)
    doc->writeAttribute("BPM", QString::number(layer.bpm, 'g', QLocale::FloatingPointShortest));
    if (layer.syncBpm)
        doc->writeAttribute("SyncBPM", "1");
    if (layer.multiplier != 1.0)
        doc->writeAttribute("Multiplier", QString::number(layer.multiplier, 'g', QLocale::FloatingPointShortest));
    if (layer.wave != 0)
        doc->writeAttribute("Wave", QString::number(layer.wave));
    if (layer.step != 0)
        doc->writeAttribute("Step", QString::number(layer.step));
    if (layer.grouping != NoGrouping)
    {
        doc->writeAttribute("Grouping", groupingToString(layer.grouping));
        doc->writeAttribute("GroupSize", QString::number(layer.groupSize));
    }
    if (layer.order != SelectionOrder)
        doc->writeAttribute("Order", orderToString(layer.order));
    if (layer.order == Shuffled)
        doc->writeAttribute("Seed", QString::number(layer.seed));
    if (layer.phase != 0)
        doc->writeAttribute("Phase", QString::number(layer.phase));
    if (layer.wait > 0)
        doc->writeAttribute("Wait", QString::number(layer.wait));
    if (layer.rotate != 0)
        doc->writeAttribute("Rotate", QString::number(layer.rotate));
    if (layer.panInvert)
        doc->writeAttribute("PanInvert", "1");
    if (layer.tiltInvert)
        doc->writeAttribute("TiltInvert", "1");
    if (layer.reverse)
        doc->writeAttribute("Reverse", "1");
    if (layer.linked)
        doc->writeAttribute("Linked", "1");
    if (layer.fixtures.isEmpty() == false)
        doc->writeAttribute("Fixtures", idList(layer.fixtures));

    // DMX Total Control: what its fixtures do their own way
    const QMap<quint32, Own> own = layer.tidyOwn();
    for (auto it = own.cbegin(); it != own.cend(); ++it)
    {
        const Own &o = it.value();
        doc->writeEmptyElement(KXMLDTCFXOwn);
        doc->writeAttribute("Fixture", QString::number(it.key()));
        if (o.mode) doc->writeAttribute("Mode", QString::number(*o.mode));
        if (o.figure) doc->writeAttribute("Figure", QString::number(*o.figure));
        if (o.swing) doc->writeAttribute("Swing", QString::number(*o.swing));
        if (o.swingTilt) doc->writeAttribute("SwingTilt", QString::number(*o.swingTilt));
        if (o.phase) doc->writeAttribute("Phase", QString::number(*o.phase));
        if (o.rotate) doc->writeAttribute("Rotate", QString::number(*o.rotate));
        if (o.panInvert) doc->writeAttribute("PanInvert", *o.panInvert ? "1" : "0");
        if (o.tiltInvert) doc->writeAttribute("TiltInvert", *o.tiltInvert ? "1" : "0");
        if (o.reverse) doc->writeAttribute("Reverse", *o.reverse ? "1" : "0");
    }
    doc->writeEndElement();
}

FXScene::Layer FXScene::loadLayerXML(const QXmlStreamAttributes &attrs)
{
    Layer layer;
    layer.param = attrs.value("Param").toString().toLower();
    layer.mode = attrs.hasAttribute("Mode") ? qBound(1, attrs.value("Mode").toInt(), ModeCount) : 1;
    layer.figure = qBound(0, attrs.value("Figure").toInt(), FigureCount - 1);
    layer.swing = qBound(0, attrs.value("Swing").toInt(), 250);
    layer.swingTilt = attrs.hasAttribute("SwingTilt") ? qBound(0, attrs.value("SwingTilt").toInt(), 250) : layer.swing;
    // (not a number, or an infinite one, in a damaged file: the default)
    bool bpmOk = false, multiplierOk = false;
    const qreal bpm = attrs.value("BPM").toDouble(&bpmOk);
    layer.bpm = bpmOk && qIsFinite(bpm) ? qMax(0.0, bpm) : 30.0;
    layer.syncBpm = attrs.value("SyncBPM") == QLatin1String("1");
    const qreal multiplier = attrs.value("Multiplier").toDouble(&multiplierOk);
    layer.multiplier = multiplierOk && qIsFinite(multiplier) ? qBound(0.01, multiplier, 64.0) : 1.0;
    layer.wave = qMax(-1, attrs.value("Wave").toInt());
    layer.step = qMax(-1, attrs.value("Step").toInt());
    layer.grouping = stringToGrouping(attrs.value("Grouping").toString());
    layer.groupSize = attrs.hasAttribute("GroupSize") ? qMax(1, attrs.value("GroupSize").toInt()) : 2;
    layer.order = stringToOrder(attrs.value("Order").toString());
    layer.seed = attrs.hasAttribute("Seed") ? attrs.value("Seed").toInt() : 1;
    layer.phase = attrs.value("Phase").toInt();
    layer.wait = attrs.value("Wait").toUInt();
    layer.rotate = attrs.value("Rotate").toInt();
    layer.panInvert = attrs.value("PanInvert") == QLatin1String("1");
    layer.tiltInvert = attrs.value("TiltInvert") == QLatin1String("1");
    layer.reverse = attrs.value("Reverse") == QLatin1String("1");
    layer.linked = attrs.value("Linked") == QLatin1String("1");
    for (const QString &fid : attrs.value("Fixtures").toString().split(',', Qt::SkipEmptyParts))
        layer.fixtures.append(fid.toUInt());
    return layer;
}

FXScene::Layer FXScene::loadLayerXML(QXmlStreamReader &root)
{
    Layer layer = loadLayerXML(root.attributes());

    // (what a newer version puts inside is skipped)
    while (root.readNextStartElement())
    {
        if (root.name() != KXMLDTCFXOwn)
        {
            root.skipCurrentElement();
            continue;
        }
        const QXmlStreamAttributes attrs = root.attributes();
        root.skipCurrentElement();
        bool fixtureOk = false;
        const quint32 fixture = attrs.value("Fixture").toUInt(&fixtureOk);
        if (fixtureOk == false)
            continue;

        // a value that isn't a number, in a damaged file, is not there
        auto number = [&attrs](const char *name, int low, int high) -> std::optional<int> {
            bool ok = false;
            const int value = attrs.value(QLatin1String(name)).toInt(&ok);
            if (ok == false)
                return std::nullopt;
            return qBound(low, value, high);
        };
        auto flag = [&attrs](const char *name) -> std::optional<bool> {
            const QStringView value = attrs.value(QLatin1String(name));
            if (value == QLatin1String("1"))
                return true;
            if (value == QLatin1String("0"))
                return false;
            return std::nullopt;
        };
        Own o;
        o.mode = number("Mode", 1, ModeCount);
        o.figure = number("Figure", 0, FigureCount - 1);
        o.swing = number("Swing", 0, 250);
        o.swingTilt = number("SwingTilt", 0, 250);
        o.phase = number("Phase", INT_MIN, INT_MAX);
        o.rotate = number("Rotate", INT_MIN, INT_MAX);
        o.panInvert = flag("PanInvert");
        o.tiltInvert = flag("TiltInvert");
        o.reverse = flag("Reverse");
        if (o.isEmpty() == false)
            layer.own.insert(fixture, o);
    }
    return layer;
}

bool FXScene::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCFunction);
    saveXMLCommon(doc);
    // (fades in beats when it follows the BPM: an effect converted to FX)
    saveXMLTempoType(doc);
    saveXMLSpeed(doc);

    QMutexLocker locker(&m_mutex);
    for (quint32 fid : m_fixtures)
        doc->writeTextElement(KXMLDTCFXFixture, QString::number(fid));

    // the base values, sorted so the file doesn't change for nothing
    QList<QPair<quint32, quint32>> keys = m_values.keys();
    std::sort(keys.begin(), keys.end());
    for (const QPair<quint32, quint32> &key : keys)
    {
        doc->writeStartElement(KXMLDTCFXValue);
        doc->writeAttribute("Fixture", QString::number(key.first));
        doc->writeAttribute("Channel", QString::number(key.second));
        doc->writeCharacters(QString::number(m_values.value(key)));
        doc->writeEndElement();
    }

    for (const Layer &layer : m_layers)
        saveLayerXML(doc, layer);

    doc->writeEndElement();
    return true;
}

bool FXScene::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << "Function node not found!";
        return false;
    }

    if (root.attributes().value(KXMLQLCFunctionType).toString() != typeToString(Function::FXSceneType))
    {
        qWarning() << "Function is not an FX scene!";
        return false;
    }

    QMutexLocker locker(&m_mutex);
    m_fixtures.clear();
    m_values.clear();
    m_layers.clear();

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
        else if (root.name() == KXMLDTCFXFixture)
        {
            // (what a newer version puts inside is skipped; not a number: no fixture)
            bool ok = false;
            quint32 fid = root.readElementText(QXmlStreamReader::SkipChildElements).trimmed().toUInt(&ok);
            if (ok && m_fixtures.contains(fid) == false)
                m_fixtures.append(fid);
        }
        else if (root.name() == KXMLDTCFXValue)
        {
            QXmlStreamAttributes attrs = root.attributes();
            bool fixtureOk = false, channelOk = false, valueOk = false;
            quint32 fid = attrs.value("Fixture").toUInt(&fixtureOk);
            quint32 channel = attrs.value("Channel").toUInt(&channelOk);
            int value = root.readElementText(QXmlStreamReader::SkipChildElements).trimmed().toInt(&valueOk);
            if (fixtureOk && channelOk && valueOk)
            {
                m_values.insert(qMakePair(fid, channel), uchar(qBound(0, value, 255)));
                if (m_fixtures.contains(fid) == false)
                    m_fixtures.append(fid);
            }
        }
        else if (root.name() == KXMLDTCFXLayer)
        {
            Layer layer = loadLayerXML(root);
            if (layer.param.isEmpty() == false)
                m_layers.append(layer);
        }
        else
        {
            root.skipCurrentElement();
        }
    }

    m_targetsDirty = true;
    return true;
}

void FXScene::postLoad()
{
    Doc *show = doc();
    if (show == NULL)
        return;

    // like the scenes: what is on fixtures that aren't in the show goes
    QMutexLocker locker(&m_mutex);
    removeFixturesLocked([show](quint32 fixture) { return show->fixture(fixture) == NULL; });
}

/*********************************************************************
 * Running
 *********************************************************************/

void FXScene::buildTargets()
{
    m_targets.clear();
    m_layerOrders.clear();
    QSet<QPair<quint32, quint32>> modulated;

    for (int l = 0; l < m_layers.count(); l++)
    {
        const Layer &layer = m_layers.at(l);
        QList<quint32> order = orderedFixtures(layer, m_fixtures);
        int sets = 1;
        QList<int> setOf = groupSets(doc(), order, layer.grouping, layer.groupSize, &sets);
        m_layerOrders.append(order);

        for (int i = 0; i < order.count(); i++)
        {
            Fixture *fxi = doc()->fixture(order.at(i));
            if (fxi == NULL)
                continue;

            QStringList params = layer.isPanTilt() ? QStringList({ "pan", "tilt" }) : QStringList({ layer.param });
            for (const QString &param : params)
            {
                Target t;
                t.fixture = fxi->id();
                t.universe = fxi->universe();
                t.channel = channelFor(fxi, param, &t.fine);
                if (t.channel == QLCChannel::invalid() || modulated.contains(qMakePair(t.fixture, t.channel)))
                    continue;
                // DMX Total Control: a fine channel past the end of the
                // universe (a fixture across two of them) is left alone: the
                // layer is 8 bit there (it was written out of the universe)
                const quint32 fine = t.fine;
                if (t.fine != QLCChannel::invalid() && fxi->address() + t.fine >= UNIVERSE_SIZE)
                    t.fine = QLCChannel::invalid();
                t.layer = l;
                t.tilt = param == QLatin1String("tilt");
                t.set = setOf.at(i);
                t.sets = sets;
                t.index = i;
                t.twinSet = setOf.at(order.count() - 1 - i);

                auto msb = m_values.constFind(qMakePair(t.fixture, t.channel));
                t.hasBase = msb != m_values.constEnd();
                t.base = t.hasBase ? quint32(msb.value()) : 0;
                if (t.fine != QLCChannel::invalid())
                    t.base = (t.base << 8) + quint32(m_values.value(qMakePair(t.fixture, t.fine), 0));

                modulated.insert(qMakePair(t.fixture, t.channel));
                if (fine != QLCChannel::invalid())
                    modulated.insert(qMakePair(t.fixture, fine));
                m_targets.append(t);
            }
        }
    }

    // the base values without an FX: they fade in like a scene
    for (auto it = m_values.cbegin(); it != m_values.cend(); ++it)
    {
        if (modulated.contains(it.key()))
            continue;
        Fixture *fxi = doc()->fixture(it.key().first);
        if (fxi == NULL || it.key().second >= fxi->channels())
            continue;
        Target t;
        t.fixture = fxi->id();
        t.universe = fxi->universe();
        t.channel = it.key().second;
        t.fine = QLCChannel::invalid();
        t.layer = -1;
        t.tilt = false;
        t.set = 0;
        t.sets = 1;
        t.index = 0;
        t.twinSet = 0;
        t.hasBase = true;
        t.base = it.value();
        m_targets.append(t);
    }

    while (m_layerPos.count() < m_layers.count())
        m_layerPos.append(0.0);
    m_targetsDirty = false;
}

void FXScene::preRun(MasterTimer *timer)
{
    {
        QMutexLocker locker(&m_mutex);
        m_layerPos.clear();
        for (int i = 0; i < m_layers.count(); i++)
            m_layerPos.append(0.0);
        m_targetsDirty = true;
        m_valuesStarted = false;
    }
    // DMX Total Control: a start in the middle (a super-scene block) is
    // already that far into its fade (start() sets elapsed(), in real ms)
    m_fadeElapsed = elapsed();
    if (m_fadersPaused)
        setFadersPaused(false);
    Function::preRun(timer);
}

QSharedPointer<GenericFader> FXScene::getFader(Universe *universe)
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
        fader->setHandleSecondary(true);
        m_fadersMap[universe->id()] = fader;
    }
    return fader;
}

bool FXScene::isMotion(const QString &param)
{
    return param == QLatin1String("pantilt") || param == QLatin1String("pan") || param == QLatin1String("tilt");
}

qreal FXScene::frameMotion() const
{
    return 1.0;
}

void FXScene::write(MasterTimer *timer, QList<Universe *> universes)
{
    // DMX Total Control: paused, the faders hold too (the base of a layer
    // fading in, a static value), not only the waves
    if (isPaused() != m_fadersPaused)
        setFadersPaused(isPaused());
    if (isPaused())
        return;

    advanceFrameTick();

    QMutexLocker locker(&m_mutex);
    bool rebuilt = false;
    if (m_targetsDirty)
    {
        // channels that are not played any more go back to what's below
        QList<Target> old = m_targets;
        buildTargets();
        for (const Target &o : std::as_const(old))
        {
            bool kept = false;
            for (const Target &t : std::as_const(m_targets))
                if (t.fixture == o.fixture && t.channel == o.channel && (t.layer >= 0) == (o.layer >= 0))
                    kept = true;
            if (kept || o.universe >= quint32(universes.count()))
                continue;
            QSharedPointer<GenericFader> fader = m_fadersMap.value(universes.at(int(o.universe))->id());
            if (fader.isNull())
                continue;
            // the fader forgets it: from the next frame it is what's below
            FadeChannel gone(doc(), o.fixture, o.channel);
            fader->remove(&gone);
        }
        rebuilt = true;
    }

    uint fadeIn = overrideFadeInSpeed() == defaultSpeed() ? fadeInSpeed() : overrideFadeInSpeed();
    if (tempoType() == Beats)
        fadeIn = beatsToTime(fadeIn, timer->beatTimeDuration());
    // DMX Total Control: in real ms, as the bases fade (FadeChannels): at a
    // live speed the waves still grow in with them
    const qreal fadeFactor = fadeIn == 0 ? 1.0 : qMin(1.0, qreal(m_fadeElapsed) / qreal(fadeIn));
    const qreal size = getAttributeValue(Size);
    const qreal phaseShift = getAttributeValue(Phase) / 360.0;
    const qreal dt = qreal(frameTick());

    // each layer goes at its own speed; a new speed goes on from where it is.
    // The pan/tilt layers, at the speed of the movement too
    for (int l = 0; l < m_layers.count(); l++)
    {
        const Layer &layer = m_layers.at(l);
        qreal bpm = layer.syncBpm ? qreal(timer->bpmNumber()) : layer.bpm;
        const qreal motion = isMotion(layer.param) ? motionSpeed() : 1.0;
        m_layerPos[l] += dt / 60000.0 * qMax(0.0, bpm) * layer.multiplier * motion;
    }

    // DMX Total Control: the layers that are a mirror (what the fixtures do
    // their own way changes live, without new targets)
    QList<QList<bool>> twins;
    for (int l = 0; l < m_layers.count(); l++)
        twins.append(l < m_layerOrders.count() ? mirrorTwins(m_layers.at(l), m_layerOrders.at(l)) : QList<bool>());

    const bool startValues = m_valuesStarted == false || rebuilt;
    for (const Target &t : std::as_const(m_targets))
    {
        if (t.universe >= quint32(universes.count()))
            continue;
        Universe *universe = universes.at(int(t.universe));
        QSharedPointer<GenericFader> fader = getFader(universe);
        if (fader.isNull())
            continue;

        if (t.layer < 0)
        {
            // a static value: set once, it fades in like a scene
            if (startValues == false)
                continue;
            const quint32 channel = t.channel;
            const uchar value = uchar(t.base);
            const bool first = m_valuesStarted == false;
            fader->updateChannel(doc(), universe, t.fixture, channel, [=](FadeChannel &fc)
            {
                int chIndex = fc.channelIndex(channel);
                fc.setStart(fc.current(chIndex), chIndex);
                fc.setTarget(value, chIndex);
                fc.setElapsed(0);
                fc.setReady(false);
                fc.setFadeTime(first && fc.canFade() ? fadeIn : 0);
            });
            continue;
        }

        const Layer &layer = m_layers.at(t.layer);
        // DMX Total Control: what the fixture does its own way, on the clock of the layer
        const Shape shape = layer.shapeOf(t.fixture);
        // in a mirror, the place in the wave of its twin
        const QList<bool> &mirror = twins.at(t.layer);
        const int set = t.index < mirror.count() && mirror.at(t.index) ? t.twinSet : t.set;
        qreal pos = positionOf(layer, shape, m_layerPos.at(t.layer) + phaseShift, set, t.sets);
        qreal level = 0.0;
        int swing = shape.swing;
        if (pos >= 0.0)
        {
            if (layer.isPanTilt())
            {
                QPointF p = panTiltPoint(shape, pos);
                level = t.tilt ? p.y() : p.x();
                if (t.tilt)
                    swing = shape.swingTilt;
            }
            else
            {
                level = modeValue(shape.mode, pos);
            }
        }

        const bool wide = t.fine != QLCChannel::invalid();
        const qreal range = wide ? 65535.0 : 255.0;
        const qint32 offset = qint32(std::round(level * qreal(swing) / 100.0 * 0.5 * range * size * fadeFactor));
        const bool hasBase = t.hasBase;
        const quint32 base = t.base;
        // the base fades in when the scene starts; a live edit applies at once
        const bool initial = m_valuesStarted == false;
        const bool setBase = initial || rebuilt;
        // the fine channel is sent by the coarse one (in the universe of the fixture)
        Fixture *fxi = doc()->fixture(t.fixture);
        const quint32 fineAddress = wide && fxi != NULL ? fxi->address() + t.fine : QLCChannel::invalid();
        // the level the fader applies to the base of an intensity
        const qreal compIntensity = fader->intensity() * fader->parentIntensity();

        fader->updateChannel(doc(), universe, t.fixture, t.channel, [=](FadeChannel &fc)
        {
            const bool fresh = (fc.flags() & FadeChannel::Modulated) == 0;
            if (fresh || setBase)
            {
                fc.addFlag(FadeChannel::Modulated);
                if (hasBase)
                {
                    // from where the channel is (16 bit once modulated)
                    quint32 from = fresh && wide ? fc.current() << 8 : fc.current();
                    // DMX Total Control: a new intensity holds what the stage
                    // shows, which is already at our level, and the fader
                    // scales the base by it again: it starts from stage / level,
                    // so a base taken over below full (a cuelist handing a
                    // static value to its FX player, a group master) doesn't
                    // dip and fade back up. Near 0 it starts at the base
                    if (fresh && fc.canFade() && (fc.flags() & FadeChannel::Intensity))
                    {
                        const qreal maxValue = wide ? 65535.0 : 255.0;
                        from = compIntensity > 0.004
                                   ? quint32(qMin(maxValue, std::round(qreal(from) / compIntensity)))
                                   : base;
                    }
                    fc.setCurrent(from);
                    fc.setStart(from);
                    fc.setTarget(base);
                    fc.setElapsed(0);
                    fc.setReady(false);
                    fc.setFadeTime(initial && fc.canFade() ? fadeIn : 0);
                }
            }
            fc.setModulation(offset, hasBase == false, fineAddress);
        });
    }
    m_valuesStarted = true;
    locker.unlock();

    incrementElapsed();
    if (m_fadeElapsed < UINT_MAX - MasterTimer::tick())
        m_fadeElapsed += MasterTimer::tick();
}

void FXScene::postRun(MasterTimer *timer, QList<Universe *> universes)
{
    // DMX Total Control: released while paused, the faders go on first, or
    // the fade out would hold for ever
    if (m_fadersPaused)
        setFadersPaused(false);

    uint fadeout = overrideFadeOutSpeed() == defaultSpeed() ? fadeOutSpeed() : overrideFadeOutSpeed();
    if (fadeout == 0)
    {
        dismissAllFaders();
    }
    else
    {
        if (tempoType() == Beats)
            fadeout = beatsToTime(fadeout, timer->beatTimeDuration());
        // DMX Total Control: a pause of the cuelist that plays the scene
        // holds them too (setFadersPaused). The ones of the stops before
        // that are over have left the universe
        m_fadingOut.removeIf([](const QSharedPointer<GenericFader> &fader) {
            return fader.isNull() || fader->deleteRequested();
        });
        for (QSharedPointer<GenericFader> fader : std::as_const(m_fadersMap))
        {
            if (!fader.isNull())
            {
                fader->setFadeOut(true, fadeout);
                m_fadingOut.append(fader);
            }
        }
        m_fadersMap.clear();
    }

    Function::postRun(timer, universes);
}

void FXScene::setFadersPaused(bool paused)
{
    m_fadersPaused = paused;
    for (const QSharedPointer<GenericFader> &fader : std::as_const(m_fadersMap))
    {
        if (!fader.isNull())
            fader->setPaused(paused);
    }

    // the ones that ended their fade out have left the universe
    m_fadingOut.removeIf([](const QSharedPointer<GenericFader> &fader) {
        return fader.isNull() || fader->deleteRequested();
    });
    for (const QSharedPointer<GenericFader> &fader : std::as_const(m_fadingOut))
        fader->setPaused(paused);
}

int FXScene::adjustAttribute(qreal fraction, int attributeId)
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
