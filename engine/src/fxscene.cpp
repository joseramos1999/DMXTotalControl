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
#include <QRegularExpression>
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
#include <limits>

#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "fixturegroup.h"
#include "qlcfixturehead.h"
#include "qlcfixturemode.h"
#include "qlccapability.h"
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

    // DMX Total Control: a fixture moved to another address or universe, or
    // in another mode: its channels are found again (the FX of the
    // programmer and the FX player of a cuelist, not in the show, too)
    if (doc != NULL)
        connect(doc, SIGNAL(fixtureChanged(quint32)), this, SLOT(slotFixtureChanged(quint32)));
}

FXScene::~FXScene()
{
    // DMX Total Control: what its stops left fading out goes on to the end
    // of its fade in the faders (still: nothing moves it any more)
    MasterTimer *timer = doc() != nullptr ? doc()->masterTimer() : nullptr;
    if (m_tailSource && timer != nullptr)
        timer->unregisterDMXSource(this);
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
    // DMX Total Control: the shift and the delay of the fixture too
    return mode == o.mode && figure == o.figure && swing == o.swing && swingTilt == o.swingTilt &&
           phase == o.phase && rotate == o.rotate && panInvert == o.panInvert &&
           tiltInvert == o.tiltInvert && reverse == o.reverse && shift == o.shift && delay == o.delay;
}

bool FXScene::Own::isEmpty() const
{
    return hasShape() == false && hasOffset() == false;
}

bool FXScene::Own::hasShape() const
{
    return mode || figure || swing || swingTilt || phase || rotate || panInvert || tiltInvert || reverse;
}

bool FXScene::Own::hasOffset() const
{
    return shift || delay;
}

bool FXScene::Own::operator==(const Own &o) const
{
    return mode == o.mode && figure == o.figure && swing == o.swing && swingTilt == o.swingTilt &&
           phase == o.phase && rotate == o.rotate && panInvert == o.panInvert &&
           tiltInvert == o.tiltInvert && reverse == o.reverse && shift == o.shift && delay == o.delay;
}

FXScene::Shape FXScene::Layer::shape() const
{
    // (DMX Total Control: a layer has no shift nor delay: those are its fixtures')
    return { mode, figure, swing, swingTilt, phase, rotate, panInvert, tiltInvert, reverse, 0, 0 };
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
    s.shift = o.shift.value_or(0);
    s.delay = o.delay.value_or(0);
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
        // DMX Total Control: no shift or delay is the layer's
        if (o.shift == 0) o.shift.reset();
        if (o.delay == 0) o.delay.reset();
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
    // grouping, the seed of an order that is not random. Nor its spread: it
    // keeps the places of fixtures that left, the FX is the same
    const bool panTilt = isPanTilt();
    return param == o.param && mode == o.mode && swing == o.swing &&
           (panTilt == false || (figure == o.figure && swingTilt == o.swingTilt)) &&
           qFuzzyCompare(bpm + 1, o.bpm + 1) && syncBpm == o.syncBpm &&
           qFuzzyCompare(multiplier, o.multiplier) && wave == o.wave && step == o.step &&
           grouping == o.grouping && (grouping == NoGrouping || groupSize == o.groupSize) &&
           order == o.order && (order != Shuffled || seed == o.seed) &&
           phase == o.phase && wait == o.wait && rotate == o.rotate && panInvert == o.panInvert &&
           tiltInvert == o.tiltInvert && reverse == o.reverse && link == o.link && fixtures == o.fixtures &&
           // DMX Total Control: the level of the virtual dimmer, on a dimmer layer
           (param != QLatin1String("dimmer") || virtualBase == o.virtualBase) &&
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
        keepLookLocked();
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
        keepLookLocked();
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
        keepLookLocked();
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
        // (DMX Total Control: the spread too, which the layers leave out)
        bool sameSpread = layers.count() == m_layers.count();
        for (int i = 0; sameSpread && i < layers.count(); i++)
            sameSpread = layers.at(i).spread == m_layers.at(i).spread;
        if (sameSpread && layers == m_layers)
            return;
        keepLookLocked();
        // only what changes the channels needs new targets: the speed,
        // the swing... are read every frame
        bool sameTargets = layers.count() == m_layers.count();
        for (int i = 0; sameTargets && i < layers.count(); i++)
        {
            const Layer &a = layers.at(i), &b = m_layers.at(i);
            sameTargets = a.param == b.param && a.fixtures == b.fixtures && a.grouping == b.grouping &&
                          a.groupSize == b.groupSize && a.order == b.order && a.seed == b.seed &&
                          a.spread == b.spread;
        }
        // DMX Total Control: each layer goes on from where its wave is, also
        // when the ones before it go or come (the positions went by index:
        // the layers after a stopped one jumped to its place): the same
        // parameter on fixtures it had
        QList<qreal> positions(layers.count(), -1.0);
        // the old layer it goes on from, -1 for a new one; and the one whose
        // clock it takes (its age, its pass, the beat of the show)
        QList<int> from(layers.count(), -1);
        QList<int> clock(layers.count(), -1);
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
                    from[i] = j;
                    clock[i] = j;
                    break;
                }
            }
        }
        // a new layer goes on the clock of what plays: from the layer its
        // fixtures all come from (part of an FX given another shape, a
        // macro), else from another layer of its parameter at the same speed.
        // Not one with a delay: its fixtures wait at their base from 0
        auto hasDelay = [](const Layer &layer) {
            for (const Own &own : layer.own)
                if (own.delay.value_or(0) > 0)
                    return true;
            return false;
        };
        auto sameSpeed = [](const Layer &a, const Layer &b) {
            return a.syncBpm == b.syncBpm && qFuzzyCompare(a.multiplier, b.multiplier) &&
                   (a.syncBpm || qFuzzyCompare(a.bpm + 1, b.bpm + 1));
        };
        for (int i = 0; i < layers.count(); i++)
        {
            const Layer &layer = layers.at(i);
            if (positions.at(i) >= 0 || layer.fixtures.isEmpty() || hasDelay(layer))
                continue;
            for (int j = 0; j < m_layers.count() && positions.at(i) < 0; j++)
            {
                const Layer &old = m_layers.at(j);
                bool all = old.param == layer.param;
                for (int f = 0; all && f < layer.fixtures.count(); f++)
                    all = old.fixtures.contains(layer.fixtures.at(f));
                if (all)
                {
                    positions[i] = m_layerPos.value(j, 0.0);
                    clock[i] = j;
                }
            }
            for (int k = 0; k < layers.count() && positions.at(i) < 0; k++)
            {
                if (from.at(k) >= 0 && layers.at(k).param == layer.param && sameSpeed(layers.at(k), layer))
                {
                    positions[i] = positions.at(k);
                    clock[i] = clock.at(k);
                }
            }
        }
        // linked layers go on one clock: one that joins a group takes the
        // place of the ones already in it (they don't jump to it); a new
        // group takes the place of a layer of it that plays, else of its
        // first one. The ones in it go on as they are
        QList<int> groups;
        for (const Layer &layer : layers)
            if (layer.link > 0 && groups.contains(layer.link) == false)
                groups.append(layer.link);
        for (int group : std::as_const(groups))
        {
            auto stays = [&](int i) { return from.at(i) >= 0 && m_layers.at(from.at(i)).link == group; };
            int anchor = -1;
            for (int pass = 0; pass < 3 && anchor < 0; pass++)
            {
                for (int i = 0; i < layers.count() && anchor < 0; i++)
                {
                    if (layers.at(i).link != group || positions.at(i) < 0)
                        continue;
                    if (pass == 2 || (pass == 0 && stays(i)) || (pass == 1 && from.at(i) >= 0))
                        anchor = i;
                }
            }
            for (int i = 0; i < layers.count() && anchor >= 0; i++)
            {
                if (layers.at(i).link == group && stays(i) == false)
                {
                    positions[i] = positions.at(anchor);
                    clock[i] = clock.at(anchor);
                }
            }
        }
        for (int i = 0; i < layers.count(); i++)
            if (positions.at(i) < 0)
                positions[i] = 0.0;
        m_layerPos = positions;
        // DMX Total Control: a new layer starts its delays from 0 and, on the
        // beat of the show, goes to it
        QList<qreal> ages, passes, onBeat, rates;
        for (int i = 0; i < layers.count(); i++)
        {
            const int j = clock.at(i);
            ages.append(j >= 0 ? m_layerAge.value(j, 0.0) : 0.0);
            passes.append(j >= 0 ? m_layerPass.value(j, 0.0) : 0.0);
            onBeat.append(j >= 0 ? m_layerOnBeat.value(j, -1.0) : -1.0);
            rates.append(j >= 0 ? m_layerRate.value(j, 0.0) : 0.0);
        }
        m_layerAge = ages;
        m_layerPass = passes;
        m_layerOnBeat = onBeat;
        m_layerRate = rates;
        // DMX Total Control: the look under a change goes on at the places of
        // the layers it goes on as
        if (m_looks.isEmpty() == false)
        {
            QList<int> &next = m_looks.last().next;
            for (int &n : next)
                n = n >= 0 ? int(from.indexOf(n)) : -1;
        }

        m_layers = layers;
        for (const Layer &layer : layers)
            for (quint32 fid : layer.fixtures)
                if (m_fixtures.contains(fid) == false)
                    m_fixtures.append(fid);
        if (sameTargets == false)
            m_targetsDirty = true;
    }
    // DMX Total Control: a tapped rhythm is for the layers that follow the
    // BPM: with none any more it goes (else it would show on its buttons
    // with no way to clear it, and come back unseen with a new one)
    // (the FX player of a cuelist, not in the show, tells nothing)
    if (tapPeriod() > 0 && hasSyncedLayer() == false)
        setTapPeriodMs(0, id() != Function::invalidId());
    emit changed(id());
}

QList<FXScene::Layer> FXScene::layers() const
{
    QMutexLocker locker(&m_mutex);
    return m_layers;
}

QList<qreal> FXScene::layerPositions() const
{
    QMutexLocker locker(&m_mutex);
    return m_layerPos;
}

bool FXScene::hasSyncedLayer() const
{
    QMutexLocker locker(&m_mutex);
    for (const Layer &layer : m_layers)
    {
        if (layer.syncBpm)
            return true;
    }
    return false;
}

bool FXScene::followsShowBeat() const
{
    if (tapPeriodMs() > 0)
        return false;
    QMutexLocker locker(&m_mutex);
    for (const Layer &layer : m_layers)
        if (layer.syncBpm == false)
            return false;
    return m_layers.isEmpty() == false;
}

/** DMX Total Control: the length of a run of each layer of $fx as it plays
 *  (ms; 0: a layer stopped), the layers that follow the BPM at $synced */
static QList<qreal> runLengths(const Doc *doc, const QList<FXScene::Layer> &layers,
                               const QList<quint32> &fixtures, qreal synced)
{
    QList<qreal> lengths;
    for (const FXScene::Layer &layer : layers)
    {
        const qreal bpm = layer.syncBpm ? synced : layer.bpm;
        if (bpm * layer.multiplier <= 0)
        {
            lengths.append(0);
            continue;
        }
        const qreal cycle = 60000.0 / (bpm * layer.multiplier);
        // its sets as it plays them (a step over all of them runs once on each)
        int sets = 1;
        FXScene::groupSets(doc, FXScene::timingOrder(layer, fixtures, doc), layer.grouping, layer.groupSize, &sets);
        const qreal pass = FXScene::passCycles(layer, sets, bpm);
        lengths.append(cycle * (pass > 0 ? pass : 1.0));
    }
    return lengths;
}

uint FXScene::cycleDuration() const
{
    // DMX Total Control: as they play (it took 120 BPM for the layers that
    // follow the BPM, the wait without the run of the wave, and a step over
    // every fixture instead of its sets)
    const qreal synced = syncedBpm();
    QMutexLocker locker(&m_mutex);
    qreal longest = 0;
    for (qreal length : runLengths(doc(), m_layers, m_fixtures, synced))
        longest = qMax(longest, length);
    return uint(std::round(longest));
}

/** DMX Total Control: the first number of runs of the slowest of $lengths
 *  that is a whole number of runs of each of the others (to $slack), and,
 *  with $unit, a whole number of it; up to $longest. 0: none */
static qreal commonLength(const QList<qreal> &lengths, qreal longest, qreal slack, qreal unit = 0)
{
    qreal slowest = 0;
    for (qreal length : lengths)
        slowest = qMax(slowest, length);
    if (slowest <= 0)
        return 0;
    for (int runs = 1; slowest * runs <= longest + slack; runs++)
    {
        const qreal loop = slowest * runs;
        bool whole = unit <= 0 || qAbs(loop / unit - std::round(loop / unit)) * unit <= slack;
        for (qreal length : lengths)
        {
            if (length > 0 && qAbs(loop - std::round(loop / length) * length) > slack)
                whole = false;
        }
        if (whole)
            return loop;
    }
    return 0;
}

uint FXScene::loopDuration(uint longest) const
{
    const qreal synced = syncedBpm();
    QMutexLocker locker(&m_mutex);
    const QList<qreal> lengths = runLengths(doc(), m_layers, m_fixtures, synced);
    const qreal loop = commonLength(lengths, longest, 1.0);
    if (loop > 0)
        return uint(std::round(loop));
    // none: the slowest one
    qreal slowest = 0;
    for (qreal length : lengths)
        slowest = qMax(slowest, length);
    return uint(std::round(slowest));
}

qreal FXScene::loopBeats(qreal longest) const
{
    const qreal synced = syncedBpm();
    if (synced <= 0)
        return 0;
    QMutexLocker locker(&m_mutex);
    QList<qreal> beats;
    for (qreal length : runLengths(doc(), m_layers, m_fixtures, synced))
        beats.append(length * synced / 60000.0);
    return commonLength(beats, longest, 1e-6, 0.001);
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
    // goes (with none it would move every fixture of the scene), and its
    // clock with it: the layers after it keep theirs (they took the one of
    // the layer before them)
    for (int l = m_layers.count() - 1; l >= 0; l--)
    {
        Layer &layer = m_layers[l];
        layer.spread.removeIf(remove);
        if (layer.fixtures.removeIf(remove) == 0)
            continue;
        removed = true;
        if (layer.fixtures.isEmpty() == false)
        {
            layer.own = layer.tidyOwn();
            continue;
        }
        m_layers.removeAt(l);
        for (QList<qreal> *clock : { &m_layerPos, &m_layerAge, &m_layerPass, &m_layerOnBeat, &m_layerRate })
        {
            if (l < clock->count())
                clock->removeAt(l);
        }
        if (l < m_layerOrders.count())
            m_layerOrders.removeAt(l);
        if (l < m_layerSets.count())
            m_layerSets.removeAt(l);
        if (m_looks.isEmpty() == false)
        {
            for (int &n : m_looks.last().next)
                n = n == l ? -1 : (n > l ? n - 1 : n);
        }
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
        case 13: return -squareWave(pos);               // snap down, snap above
        case 14: return squareWave(pos);                // DMX Total Control: 7 reversed (it was 8)
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
            // the «Intensity» of an HSI fixture is its dimmer (its group says so below)
            if (name.contains(QLatin1String("intens")) || name.contains(QLatin1String("dimmer")) ||
                name.contains(QLatin1String("bright")))
                break;
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

QString FXScene::fixtureParamOf(const Fixture *fixture, quint32 channel)
{
    if (fixture == NULL)
        return QString();
    const QLCChannel *ch = fixture->channel(channel);
    const QString param = paramOf(ch);
    if (param == QLatin1String("dimmer") && headOf(fixture, channel) < 0 &&
        channelFor(fixture, param) != channel)
    {
        // "dimmer:" and its name, when the name finds it again: a name of its
        // own, that no other channel (a cell, the master of another fixture
        // with that name) can answer to
        const QString key = QStringLiteral("dimmer:") + ch->name().trimmed().toLower();
        if (key.length() > 7 && channelFor(fixture, key) == channel)
            return key;
    }
    return param;
}

QList<quint32> FXScene::levelDimmerChannels(const Fixture *fixture)
{
    QList<quint32> channels;
    if (fixture == NULL)
        return channels;
    const quint32 master = channelFor(fixture, QStringLiteral("dimmer"));
    for (quint32 c = 0; c < fixture->channels(); c++)
    {
        const QLCChannel *ch = fixture->channel(c);
        if (ch == NULL || ch->group() != QLCChannel::Intensity ||
            ch->colour() != QLCChannel::NoColour || ch->controlByte() != QLCChannel::MSB ||
            attributeOf(ch) != QLatin1String("dimmer"))
            continue;
        if (c != master && headOf(fixture, c) < 0)
        {
            // a plain level: a dimmer preset, one range 0-255, or ranges that
            // are all levels (a GDTF dimmer: Closed, Dimmer, Open)
            static const QRegularExpression level(QStringLiteral("^(closed?|off|dimmer|dimming|intensity|open|on|full)\\b"),
                                                  QRegularExpression::CaseInsensitiveOption);
            const QList<QLCCapability *> caps = ch->capabilities();
            const bool levels = caps.count() > 1 &&
                std::all_of(caps.cbegin(), caps.cend(), [](const QLCCapability *cap) {
                    return level.match(cap->name().trimmed()).hasMatch(); });
            const bool plain = ch->preset() == QLCChannel::IntensityDimmer ||
                               ch->preset() == QLCChannel::IntensityMasterDimmer ||
                               (caps.count() == 1 && caps.first()->min() == 0 && caps.first()->max() == 255) || levels;
            if (plain == false || QLCFixtureMode::isNotLight(ch->name()))
                continue;
        }
        channels.append(c);
    }
    // an HSL/HSV mode without a dimmer: its lightness is its level
    if (channels.isEmpty())
    {
        for (quint32 c = 0; c < fixture->channels(); c++)
        {
            const QLCChannel *ch = fixture->channel(c);
            if (ch != NULL && ch->controlByte() == QLCChannel::MSB && attributeOf(ch) == QLatin1String("lightness"))
            {
                channels.append(c);
                break;
            }
        }
    }
    return channels;
}

/** DMX Total Control: the fine channel of $msb on $fixture, as its mode
 *  pairs them: the LSB of its group and colour right after it; else, the
 *  only channel of its group and colour takes the only LSB of them (Pan,
 *  Tilt, Pan fine, Tilt fine). Never the fine of another channel of its
 *  group (a zoom with the focus fine) */
static quint32 fineChannelOf(const Fixture *fixture, quint32 msb)
{
    const QLCChannel *coarse = fixture != NULL ? fixture->channel(msb) : NULL;
    if (coarse == NULL || coarse->controlByte() != QLCChannel::MSB)
        return QLCChannel::invalid();
    auto kin = [coarse](const QLCChannel *ch) {
        return ch != NULL && ch->group() == coarse->group() && ch->colour() == coarse->colour();
    };
    const QLCChannel *next = fixture->channel(msb + 1);
    if (kin(next) && next->controlByte() == QLCChannel::LSB)
        return msb + 1;
    quint32 lsb = QLCChannel::invalid();
    int coarseCount = 0, fineCount = 0;
    for (quint32 c = 0; c < fixture->channels(); c++)
    {
        const QLCChannel *ch = fixture->channel(c);
        if (kin(ch) == false)
            continue;
        if (ch->controlByte() == QLCChannel::LSB)
        {
            fineCount++;
            lsb = c;
        }
        else
        {
            coarseCount++;
        }
    }
    return coarseCount == 1 && fineCount == 1 ? lsb : QLCChannel::invalid();
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
    // the dimmer of a fixture with several is its master (the engine chooses
    // it: not the flash intensity of a strobe that also has a colour aura)
    if (param == QLatin1String("dimmer"))
    {
        const quint32 master = fixture->masterIntensityChannel();
        if (master != QLCChannel::invalid() && paramOf(fixture->channel(master)) == param)
            found = master;
    }
    // "dimmer:<name>" (fixtureParamOf): a dimmer of the whole fixture, not its
    // master, with that name; never a cell nor the master of another fixture
    else if (param.startsWith(QLatin1String("dimmer:")))
    {
        const QString name = param.mid(7);
        const quint32 master = channelFor(fixture, QStringLiteral("dimmer"));
        for (quint32 c = 0; c < fixture->channels() && found == QLCChannel::invalid(); c++)
        {
            const QLCChannel *ch = fixture->channel(c);
            if (c != master && ch != NULL && ch->controlByte() == QLCChannel::MSB &&
                paramOf(ch) == QLatin1String("dimmer") && headOf(fixture, c) < 0 &&
                ch->name().trimmed().toLower() == name)
                found = c;
        }
        if (found == QLCChannel::invalid())
            return found;
    }
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

    // its fine channel (DMX Total Control: the one its mode pairs with it)
    *fine = fineChannelOf(fixture, found);
    return found;
}

QList<quint32> FXScene::channelsFor(const Fixture *fixture, const QString &param, QList<quint32> *fines)
{
    QList<quint32> list;
    if (fines != NULL)
        fines->clear();
    quint32 fine = QLCChannel::invalid();
    const quint32 first = channelFor(fixture, param, &fine);
    if (first == QLCChannel::invalid())
        return list;
    // a channel of a head (no channel of the whole fixture does it): the
    // first one of each head with the parameter, in the order of the heads
    const int head = headOf(fixture, first);
    if (head < 0 || paramOf(fixture->channel(first)) != param)
    {
        list.append(first);
        if (fines != NULL)
            fines->append(fine);
        return list;
    }
    for (int h = 0; h < fixture->heads(); h++)
    {
        quint32 found = QLCChannel::invalid();
        for (quint32 c : fixture->head(h).channels())
            if (c < found && paramOf(fixture->channel(c)) == param)
                found = c;
        if (found == QLCChannel::invalid())
            continue;
        list.append(found);
        if (fines != NULL)
            fines->append(found == first ? fine : fineChannelOf(fixture, found));
    }
    return list;
}

/** DMX Total Control: a channel that gives light of a colour (not CMY) */
static bool isEmitter(const QLCChannel *ch)
{
    if (ch == NULL || ch->group() != QLCChannel::Intensity)
        return false;
    switch (ch->colour())
    {
        case QLCChannel::Red: case QLCChannel::Green: case QLCChannel::Blue:
        case QLCChannel::White: case QLCChannel::Amber: case QLCChannel::UV:
        case QLCChannel::Lime: case QLCChannel::Indigo:
            return true;
        default:
            return false;
    }
}

QList<QPair<quint32, quint32>> FXScene::virtualDimmerChannels(const Fixture *fixture)
{
    QList<QPair<quint32, quint32>> list;
    if (fixture == NULL || channelFor(fixture, QStringLiteral("dimmer")) != QLCChannel::invalid())
        return list;

    // the emitters the grand master dims: not the CMY, nor the ones whose
    // 'Dimmer' option is off (in a fixture protected from the grand master
    // too: they are still its light)
    for (quint32 c = 0; c < fixture->channels(); c++)
    {
        const QLCChannel *ch = fixture->channel(c);
        if (isEmitter(ch) == false || ch->controlByte() != QLCChannel::MSB ||
            fixture->channelCarriesLight(int(c)) == false)
            continue;
        // its fine channel (as channelFor)
        list.append(qMakePair(c, fineChannelOf(fixture, c)));
    }
    return list;
}

bool FXScene::hasParam(const Fixture *fixture, const QString &param)
{
    if (fixture == NULL)
        return false;
    if (param == QLatin1String("pantilt"))
        return channelFor(fixture, QStringLiteral("pan")) != QLCChannel::invalid() &&
               channelFor(fixture, QStringLiteral("tilt")) != QLCChannel::invalid();
    if (channelFor(fixture, param) != QLCChannel::invalid())
        return true;
    return param == QLatin1String("dimmer") && virtualDimmerChannels(fixture).isEmpty() == false;
}

QList<QPair<quint32, quint32>> FXScene::layerChannels(const Fixture *fixture, const QString &param)
{
    QList<QPair<quint32, quint32>> list;
    const QStringList params = param == QLatin1String("pantilt") ? QStringList({ "pan", "tilt" }) : QStringList({ param });
    for (const QString &p : params)
    {
        // (DMX Total Control: on every head that has it)
        QList<quint32> fines;
        const QList<quint32> channels = channelsFor(fixture, p, &fines);
        for (int i = 0; i < channels.count(); i++)
            list.append(qMakePair(channels.at(i), fines.at(i)));
    }
    // without a dimmer, the emitters of its virtual dimmer
    if (list.isEmpty() && param == QLatin1String("dimmer"))
        list = virtualDimmerChannels(fixture);
    return list;
}

int FXScene::autoVirtualBase(const Doc *doc, const QList<quint32> &order,
                             const QHash<QPair<quint32, quint32>, uchar> &values)
{
    for (quint32 fid : order)
    {
        const quint32 c = channelFor(doc != NULL ? doc->fixture(fid) : NULL, QStringLiteral("dimmer"));
        if (c == QLCChannel::invalid())
            continue;
        auto it = values.constFind(qMakePair(fid, c));
        if (it != values.constEnd())
            return it.value();
    }
    return 255;
}

/*********************************************************************
 * DMX Total Control: the numbers and the groups of the patch
 *********************************************************************/

namespace
{
struct PatchOrder
{
    QHash<quint32, int> numbers;
    QList<quint32> groups;
};

QMutex &patchOrderMutex()
{
    static QMutex mutex;
    return mutex;
}

QHash<const Doc *, PatchOrder> &patchOrders()
{
    static QHash<const Doc *, PatchOrder> orders;
    return orders;
}

PatchOrder patchOrderOf(const Doc *doc)
{
    QMutexLocker locker(&patchOrderMutex());
    return patchOrders().value(doc);
}

/** What sorts $fixture by number: its number; the ones without one after them, by ID */
qint64 numberKey(const QHash<quint32, int> &numbers, quint32 fixture)
{
    const int number = numbers.value(fixture, 0);
    return number > 0 ? qint64(number) : (qint64(1) << 40) + qint64(fixture);
}
}

void FXScene::setPatchOrder(const Doc *doc, const QHash<quint32, int> &numbers, const QList<quint32> &groupOrder)
{
    QMutexLocker locker(&patchOrderMutex());
    patchOrders().insert(doc, { numbers, groupOrder });
}

void FXScene::clearPatchOrder(const Doc *doc)
{
    QMutexLocker locker(&patchOrderMutex());
    patchOrders().remove(doc);
}

void FXScene::copyPatchOrder(const Doc *from, const Doc *to)
{
    QMutexLocker locker(&patchOrderMutex());
    auto it = patchOrders().constFind(from);
    if (it == patchOrders().constEnd())
        patchOrders().remove(to);
    else
        patchOrders().insert(to, it.value());
}

int FXScene::fixtureNumber(const Doc *doc, quint32 fixture)
{
    QMutexLocker locker(&patchOrderMutex());
    auto it = patchOrders().constFind(doc);
    return it != patchOrders().constEnd() ? it->numbers.value(fixture, 0) : 0;
}

void FXScene::sortByNumber(const Doc *doc, QList<quint32> &fixtures)
{
    const QHash<quint32, int> numbers = patchOrderOf(doc).numbers;
    std::stable_sort(fixtures.begin(), fixtures.end(), [&numbers](quint32 a, quint32 b) {
        return numberKey(numbers, a) < numberKey(numbers, b);
    });
}

int FXScene::placeByNumber(const Doc *doc, const QList<quint32> &order, quint32 fixture)
{
    if (order.isEmpty())
        return 0;
    const QHash<quint32, int> numbers = patchOrderOf(doc).numbers;
    const qint64 key = numberKey(numbers, fixture);
    // the way the order goes: up, or down (its first one above its last one)
    const bool down = order.count() > 1 && numberKey(numbers, order.constFirst()) > numberKey(numbers, order.constLast());
    for (int i = 0; i < order.count(); i++)
    {
        const qint64 other = numberKey(numbers, order.at(i));
        if (down ? other < key : other > key)
            return i;
    }
    return int(order.count());
}

/** DMX Total Control: «By group»: the set of each fixture of $order, and the
 *  group of each set in $groupOfSet */
static QList<int> setsByGroup(const Doc *doc, const QList<quint32> &order, QList<quint32> &groupOfSet)
{
    const int n = int(order.count());
    QHash<quint32, int> place;
    for (int i = 0; i < n; i++)
        place.insert(order.at(i), i);

    // the groups of the patch (not the hidden ones of the effects) with some
    // of the fixtures but not all of them; of two with the same ones, the
    // smaller group, then the first in the order of the patch
    struct Candidate
    {
        QList<bool> in;
        int count = 0;
        int size = 0;
        int rank = 0;
        quint32 id = 0;
    };
    QList<Candidate> candidates;
    if (doc != NULL && n > 0)
    {
        const QList<quint32> ranks = patchOrderOf(doc).groups;
        for (FixtureGroup *g : doc->fixtureGroups())
        {
            if (g->name().endsWith(QLatin1String(" (FX)")) || g->name().endsWith(QLatin1String(" (FX 2D)")))
                continue;
            Candidate c;
            c.in = QList<bool>(n, false);
            const QList<quint32> members = g->fixtureList();
            for (quint32 fid : members)
            {
                auto it = place.constFind(fid);
                if (it != place.constEnd() && c.in.at(it.value()) == false)
                {
                    c.in[it.value()] = true;
                    c.count++;
                }
            }
            if (c.count == 0 || c.count == n)
                continue;
            c.size = int(members.count());
            const qsizetype r = ranks.indexOf(g->id());
            c.rank = r >= 0 ? int(r) : int(ranks.count()) + int(g->id());
            c.id = g->id();
            bool known = false;
            for (Candidate &other : candidates)
            {
                if (other.in != c.in)
                    continue;
                known = true;
                if (c.size < other.size || (c.size == other.size && c.rank < other.rank))
                    other = c;
            }
            if (known == false)
                candidates.append(c);
        }
    }
    std::sort(candidates.begin(), candidates.end(), [](const Candidate &a, const Candidate &b) {
        if (a.count != b.count)
            return a.count < b.count;
        if (a.size != b.size)
            return a.size < b.size;
        return a.rank < b.rank;
    });

    // from each one, the smaller ones that don't overlap it nor each other:
    // the split with the most groups, then the fewest fixtures alone, then
    // the smallest groups, then the first in the order of the patch
    QList<int> best;
    int bestCovered = 0, bestSize = 0;
    QList<int> bestRanks;
    for (int s = 0; s < candidates.count(); s++)
    {
        QList<int> chosen = { s };
        QList<bool> used = candidates.at(s).in;
        int covered = candidates.at(s).count;
        for (int c = 0; c < candidates.count(); c++)
        {
            if (c == s)
                continue;
            bool overlaps = false;
            for (int i = 0; i < n && overlaps == false; i++)
                overlaps = used.at(i) && candidates.at(c).in.at(i);
            if (overlaps)
                continue;
            chosen.append(c);
            covered += candidates.at(c).count;
            for (int i = 0; i < n; i++)
                used[i] = used.at(i) || candidates.at(c).in.at(i);
        }
        int size = 0;
        QList<int> ranks;
        for (int c : std::as_const(chosen))
        {
            size += candidates.at(c).size;
            ranks.append(candidates.at(c).rank);
        }
        std::sort(ranks.begin(), ranks.end());
        bool better = best.isEmpty();
        if (better == false && chosen.count() != best.count())
            better = chosen.count() > best.count();
        else if (better == false && covered != bestCovered)
            better = covered > bestCovered;
        else if (better == false && size != bestSize)
            better = size < bestSize;
        else if (better == false)
            better = ranks < bestRanks;
        if (better)
        {
            best = chosen;
            bestCovered = covered;
            bestSize = size;
            bestRanks = ranks;
        }
    }

    // the sets as they come along the order; a fixture in none of them alone
    QList<int> groupAt(n, -1);
    for (int c : std::as_const(best))
        for (int i = 0; i < n; i++)
            if (candidates.at(c).in.at(i))
                groupAt[i] = c;
    QList<int> result;
    QHash<int, int> setOfGroup;
    for (int i = 0; i < n; i++)
    {
        const int g = groupAt.at(i);
        if (g >= 0 && setOfGroup.contains(g))
        {
            result.append(setOfGroup.value(g));
            continue;
        }
        const int set = int(groupOfSet.count());
        groupOfSet.append(g >= 0 ? candidates.at(g).id : FixtureGroup::invalidId());
        if (g >= 0)
            setOfGroup.insert(g, set);
        result.append(set);
    }
    return result;
}

QList<int> FXScene::groupSets(const Doc *doc, const QList<quint32> &order, int grouping, int groupSize, int *sets,
                              QList<quint32> *setGroups)
{
    const int n = order.count();
    const int size = qMax(1, groupSize);
    QList<int> result;
    int count = n;
    if (setGroups != nullptr)
        setGroups->clear();

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
            // DMX Total Control: X parts, one apart in size at most (with
            // X/n rounded up a part, 6 into 4 made 3)
            const int parts = qMax(1, qMin(size, n));
            for (int i = 0; i < n; i++)
                result.append(i * parts / n);
            count = parts;
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
            // DMX Total Control: the groups that split the fixtures (it took
            // the first group with each one: a group of all of them made one set)
            QList<quint32> groups;
            result = setsByGroup(doc, order, groups);
            count = int(groups.count());
            if (setGroups != nullptr)
                *setGroups = groups;
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

QList<quint32> FXScene::orderedFixtures(const Layer &layer, const QList<quint32> &fixtures, const Doc *doc)
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
            // DMX Total Control: by the numbers of the patch (it went by ID)
            sortByNumber(doc, order);
            break;
        default:
            break;
    }
    return order;
}

QList<quint32> FXScene::timingOrder(const Layer &layer, const QList<quint32> &fixtures, const Doc *doc)
{
    if (layer.spread.isEmpty() || layer.fixtures.isEmpty())
        return orderedFixtures(layer, fixtures, doc);
    // DMX Total Control: the places of all of them, the ones that left too
    Layer all = layer;
    all.fixtures = layer.spread;
    for (quint32 fid : layer.fixtures)
        if (all.fixtures.contains(fid) == false)
            all.fixtures.append(fid);
    return orderedFixtures(all, fixtures, doc);
}

qreal FXScene::positionOf(const Layer &layer, qreal cyclePos, int set, int sets)
{
    return positionOf(layer, layer.shape(), cyclePos, set, sets, layer.bpm);
}

qreal FXScene::syncedBpm() const
{
    // (DMX Total Control: with their fractions)
    const qreal tap = tapPeriodMs();
    if (tap > 0)
        return 60000.0 / tap;
    return doc() != nullptr ? qMax(0.0, doc()->masterTimer()->bpm()) : 120.0;
}

qreal FXScene::positionOf(const Layer &layer, const Shape &shape, qreal cyclePos, int set, int sets, qreal bpm)
{
    // DMX Total Control: its wait (ms) in cycles at the BPM it goes at (one
    // that follows the BPM kept its own, unused, in layer.bpm)
    bpm = qMax(0.0, bpm);
    // DMX Total Control: its shift and its delay put it behind the others,
    // on top of the wave and the phase
    cyclePos -= qreal(shape.shift) / 360.0 + delayCycles(layer, shape, bpm);
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
        const int slot = set % step;
        const qreal period = passCycles(layer, sets, bpm);
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
        const qreal period = passCycles(layer, sets, bpm);
        qreal s = cyclePos - std::floor(cyclePos / period) * period;
        qreal local = s - offset;
        return local >= 0.0 && local < 1.0 ? local : -1.0;
    }

    qreal local = cyclePos - offset;
    return local - std::floor(local);
}

qreal FXScene::delayCycles(const Layer &layer, const Shape &shape, qreal bpm)
{
    // (the same conversion as the wait)
    return qreal(shape.delay) / 60000.0 * qMax(0.0, bpm) * layer.multiplier;
}

qreal FXScene::passCycles(const Layer &layer, int sets, qreal bpm)
{
    sets = qMax(1, sets);
    const int step = layer.step < 0 ? sets : layer.step;
    const int wave = layer.wave < 0 ? sets : layer.wave;
    // the run: a step over its sets, or the whole wave (the last set included)
    qreal run = 0;
    if (step > 0)
        run = step;
    else if (layer.wait > 0)
        run = 1.0 + (wave > 0 ? qreal(wave - 1) / qreal(wave) : 0.0);
    else
        return 0;
    const qreal waitCycles = layer.wait > 0 ? qreal(layer.wait) / 1000.0 * qMax(0.0, bpm) * layer.multiplier / 60.0 : 0.0;
    qreal pass = run + waitCycles;
    // DMX Total Control: at the BPM of the show (or a tapped rhythm) the pass
    // with its wait is the nearest whole number of beats, at least the run:
    // each one starts on a beat (a beat is $multiplier cycles)
    if (layer.syncBpm && layer.wait > 0 && layer.multiplier > 0)
    {
        const qreal beats = qMax(std::round(pass / layer.multiplier), std::ceil(run / layer.multiplier - 1e-6));
        pass = qMax(1.0, beats) * layer.multiplier;
    }
    return pass;
}

uint FXScene::longestDelay() const
{
    QMutexLocker locker(&m_mutex);
    uint longest = 0;
    for (const Layer &layer : m_layers)
    {
        const QMap<quint32, Own> own = layer.tidyOwn();
        for (const Own &o : own)
            longest = qMax(longest, uint(o.delay.value_or(0)));
    }
    return longest;
}

QVariantList FXScene::previewLayer(int index, qreal cyclePos) const
{
    QMutexLocker locker(&m_mutex);
    return previewLocked(index, cyclePos, -1.0);
}

QVariantList FXScene::livePreviewLayer(int index, qreal cyclePos) const
{
    QMutexLocker locker(&m_mutex);
    // while it plays: where it is (the phase of the scene too), and how far
    // it went since it started
    if (isRunning() && index >= 0 && index < m_layerPos.count() && index < m_layerAge.count())
        return previewLocked(index, m_layerPos.at(index) + getAttributeValue(Phase) / 360.0, m_layerAge.at(index));
    return previewLocked(index, cyclePos, -1.0);
}

QVariantList FXScene::previewLocked(int index, qreal cyclePos, qreal live) const
{
    QVariantList list;
    if (index < 0 || index >= m_layers.count())
        return list;

    const Layer &layer = m_layers.at(index);
    // (DMX Total Control: in the places of its spread, the fixtures it has)
    QList<quint32> order = timingOrder(layer, m_fixtures, doc());
    int sets = 1;
    QList<int> setOf = groupSets(doc(), order, layer.grouping, layer.groupSize, &sets);
    // pan/tilt: -1..1 is the widest swing of its fixtures
    int scale = qMax(1, qMax(layer.swing, layer.swingTilt));
    for (auto it = layer.own.cbegin(); it != layer.own.cend(); ++it)
        scale = qMax(scale, qMax(it->swing.value_or(0), it->swingTilt.value_or(0)));
    // a mirror: the ones on the other side go with their twins
    const QList<bool> twins = mirrorTwins(layer, order);
    // DMX Total Control: its wait as it plays (at the BPM of the show or its
    // tapped rhythm, for a layer that follows the BPM)
    const qreal bpm = layer.syncBpm ? syncedBpm() : layer.bpm;
    // DMX Total Control: live, how far it went since it started (its delays)
    const qreal age = live >= 0 ? live : cyclePos;
    for (int i = 0; i < order.count(); i++)
    {
        if (layer.spread.isEmpty() == false && layer.fixtures.contains(order.at(i)) == false)
            continue;
        const Shape shape = layer.shapeOf(order.at(i));
        const int set = i < twins.count() && twins.at(i) ? setOf.at(order.count() - 1 - i) : setOf.at(i);
        qreal pos = positionOf(layer, shape, cyclePos, set, sets, bpm);
        // DMX Total Control: a fixture with a delay waits at its base when the FX starts
        if (shape.delay > 0 && age < delayCycles(layer, shape, bpm))
            pos = -1.0;
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

bool FXScene::reverseMirrorsPan(int figure)
{
    // backwards it is at -pos: the mirror in pan when that is (-x, y) all along
    for (int i = 0; i < 64; i++)
    {
        const qreal pos = (qreal(i) + 0.37) / 64.0;
        const QPointF ahead = figurePoint(figure, pos);
        const QPointF back = figurePoint(figure, -pos);
        if (qAbs(back.x() + ahead.x()) > 1e-6 || qAbs(back.y() - ahead.y()) > 1e-6)
            return false;
    }
    return true;
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
    // DMX Total Control: its link group, past the first one
    if (layer.link > 0)
        doc->writeAttribute("Linked", "1");
    if (layer.link > 1)
        doc->writeAttribute("LinkGroup", QString::number(layer.link));
    if (layer.fixtures.isEmpty() == false)
        doc->writeAttribute("Fixtures", idList(layer.fixtures));
    // DMX Total Control: the places of the fixtures that left it (older
    // versions spread it over the ones it has)
    if (layer.spread.isEmpty() == false && layer.fixtures.isEmpty() == false)
        doc->writeAttribute("Spread", idList(layer.spread));
    // DMX Total Control: the level of the virtual dimmer, when it is not auto
    if (layer.param == QLatin1String("dimmer") && layer.virtualBase >= 0)
        doc->writeAttribute("VirtualBase", QString::number(layer.virtualBase));

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
        // DMX Total Control: its shift and its delay (an Own with only these is
        // dropped by older versions, which play the FX without them)
        if (o.shift) doc->writeAttribute("Shift", QString::number(*o.shift));
        if (o.delay) doc->writeAttribute("Delay", QString::number(*o.delay));
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
    // DMX Total Control: linked, in the first group unless it says another
    if (attrs.value("Linked") == QLatin1String("1"))
    {
        bool groupOk = false;
        const int group = attrs.value("LinkGroup").toInt(&groupOk);
        layer.link = groupOk ? qMax(1, group) : 1;
    }
    for (const QString &fid : attrs.value("Fixtures").toString().split(',', Qt::SkipEmptyParts))
        layer.fixtures.append(fid.toUInt());
    // DMX Total Control: (once each)
    for (const QString &fid : attrs.value("Spread").toString().split(',', Qt::SkipEmptyParts))
        if (layer.spread.contains(fid.toUInt()) == false)
            layer.spread.append(fid.toUInt());
    if (layer.fixtures.isEmpty())
        layer.spread.clear();
    // DMX Total Control: the level of the virtual dimmer (not a number: auto)
    bool virtualOk = false;
    const int virtualBase = attrs.value("VirtualBase").toInt(&virtualOk);
    layer.virtualBase = virtualOk ? qBound(-1, virtualBase, 255) : -1;
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
        o.shift = number("Shift", 0, 1440);
        o.delay = number("Delay", 0, 60000);
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

void FXScene::slotFixtureChanged(quint32 fxi_id)
{
    // DMX Total Control: only when what its targets have changed (a new
    // name moves nothing)
    const Fixture *fxi = doc() != nullptr ? doc()->fixture(fxi_id) : nullptr;
    if (fxi == nullptr)
        return;
    QMutexLocker locker(&m_mutex);
    auto stamp = m_patchStamps.constFind(fxi_id);
    if (stamp == m_patchStamps.constEnd())
    {
        // (one it had no channel of before, in another mode)
        bool has = m_fixtures.contains(fxi_id);
        for (const Layer &layer : std::as_const(m_layers))
            has = has || layer.fixtures.contains(fxi_id);
        if (has)
            m_targetsDirty = true;
        return;
    }
    if (stamp.value() != PatchStamp::of(fxi))
        m_targetsDirty = true;
}

FXScene::PatchStamp FXScene::PatchStamp::of(const Fixture *fixture)
{
    PatchStamp stamp;
    stamp.universe = fixture->universe();
    stamp.address = fixture->address();
    stamp.channels = fixture->channels();
    stamp.mode = fixture->fixtureMode();
    return stamp;
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
    m_scalers.clear();
    m_layerOrders.clear();
    m_layerSets.clear();
    // DMX Total Control: where its fixtures are in the patch (a change there
    // finds their channels again)
    m_patchStamps.clear();
    for (quint32 fid : std::as_const(m_fixtures))
        if (const Fixture *fxi = doc()->fixture(fid))
            m_patchStamps.insert(fid, PatchStamp::of(fxi));
    for (const Layer &layer : std::as_const(m_layers))
        for (quint32 fid : layer.fixtures)
            if (const Fixture *fxi = doc()->fixture(fid))
                m_patchStamps.insert(fid, PatchStamp::of(fxi));
    QSet<QPair<quint32, quint32>> modulated;
    // DMX Total Control: the fixtures a virtual dimmer dims already
    QSet<quint32> scaled;

    for (int l = 0; l < m_layers.count(); l++)
    {
        const Layer &layer = m_layers.at(l);
        // DMX Total Control: the sets and the mirror over its spread (the
        // fixtures that left keep their places), the channels of the ones it has
        QList<quint32> order = timingOrder(layer, m_fixtures, doc());
        int sets = 1;
        QList<int> setOf = groupSets(doc(), order, layer.grouping, layer.groupSize, &sets);
        m_layerOrders.append(order);
        m_layerSets.append(sets);
        const bool spread = layer.spread.isEmpty() == false && layer.fixtures.isEmpty() == false;

        for (int i = 0; i < order.count(); i++)
        {
            if (spread && layer.fixtures.contains(order.at(i)) == false)
                continue;
            Fixture *fxi = doc()->fixture(order.at(i));
            if (fxi == NULL)
                continue;

            // DMX Total Control: a dimmer layer on a fixture without a dimmer
            // dims its colour: a virtual dimmer (the first such layer), whose
            // emitters come after the other layers
            if (layer.param == QLatin1String("dimmer") && channelFor(fxi, layer.param) == QLCChannel::invalid())
            {
                if (scaled.contains(fxi->id()) == false && virtualDimmerChannels(fxi).isEmpty() == false)
                {
                    scaled.insert(fxi->id());
                    m_scalers.append({ fxi->id(), l, setOf.at(i), sets, i, setOf.at(order.count() - 1 - i), 255 });
                }
                continue;
            }

            QStringList params = layer.isPanTilt() ? QStringList({ "pan", "tilt" }) : QStringList({ layer.param });
            // DMX Total Control: on a fixture with several heads, on each one
            // of them, all alike (the fixture is one place of the effect)
            QList<quint32> channels, fines;
            QStringList paramOfChannel;
            for (const QString &param : params)
            {
                QList<quint32> theirFines;
                const QList<quint32> theirs = channelsFor(fxi, param, &theirFines);
                channels.append(theirs);
                fines.append(theirFines);
                for (int k = 0; k < theirs.count(); k++)
                    paramOfChannel.append(param);
            }
            for (int k = 0; k < channels.count(); k++)
            {
                const QString &param = paramOfChannel.at(k);
                Target t;
                t.fixture = fxi->id();
                t.universe = fxi->universe();
                t.channel = channels.at(k);
                t.fine = fines.at(k);
                if (modulated.contains(qMakePair(t.fixture, t.channel)))
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

    // DMX Total Control: the emitters of the virtual dimmers, now that every
    // layer has its channels: a colour layer keeps its channel whatever the
    // order of the layers, and the dimmer scales it; the others get a target
    // of their own, on their base or on what is below
    for (int s = 0; s < m_scalers.count(); s++)
    {
        Scaler &scaler = m_scalers[s];
        scaler.autoBase = autoVirtualBase(doc(), m_layerOrders.at(scaler.layer), m_values);
        Fixture *fxi = doc()->fixture(scaler.fixture);
        const QList<QPair<quint32, quint32>> emitters = virtualDimmerChannels(fxi);
        for (const QPair<quint32, quint32> &emitter : emitters)
        {
            bool taken = false;
            for (Target &t : m_targets)
            {
                if (t.fixture == scaler.fixture && t.channel == emitter.first && t.layer >= 0 && t.scaler < 0)
                {
                    t.scaler = s;
                    taken = true;
                }
            }
            if (taken || modulated.contains(qMakePair(scaler.fixture, emitter.first)))
                continue;

            Target t;
            t.fixture = scaler.fixture;
            t.universe = fxi->universe();
            t.channel = emitter.first;
            // (8 bit past the end of the universe, as the layers)
            t.fine = emitter.second;
            if (t.fine != QLCChannel::invalid() && fxi->address() + t.fine >= UNIVERSE_SIZE)
                t.fine = QLCChannel::invalid();
            t.layer = -1;
            t.tilt = false;
            t.set = scaler.set;
            t.sets = scaler.sets;
            t.index = scaler.index;
            t.twinSet = scaler.twinSet;
            auto msb = m_values.constFind(qMakePair(t.fixture, t.channel));
            t.hasBase = msb != m_values.constEnd();
            t.base = t.hasBase ? quint32(msb.value()) : 0;
            if (t.fine != QLCChannel::invalid())
                t.base = (t.base << 8) + quint32(m_values.value(qMakePair(t.fixture, t.fine), 0));
            t.scaler = s;

            modulated.insert(qMakePair(t.fixture, t.channel));
            if (emitter.second != QLCChannel::invalid())
                modulated.insert(qMakePair(t.fixture, emitter.second));
            m_targets.append(t);
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
    while (m_layerAge.count() < m_layers.count())
        m_layerAge.append(0.0);
    while (m_layerPass.count() < m_layers.count())
        m_layerPass.append(0.0);
    while (m_layerOnBeat.count() < m_layers.count())
        m_layerOnBeat.append(-1.0);
    while (m_layerRate.count() < m_layers.count())
        m_layerRate.append(0.0);
    // DMX Total Control: the target of each channel; the FX without a base
    // (on what is below) in the faders after the ones of their priority
    m_targetIndex.clear();
    for (int i = 0; i < m_targets.count(); i++)
    {
        Target &t = m_targets[i];
        t.relative = (t.layer >= 0 || t.scaler >= 0) && t.hasBase == false;
        m_targetIndex.insert(qMakePair(t.fixture, t.channel), i);
    }
    m_targetsDirty = false;
}

void FXScene::preRun(MasterTimer *timer)
{
    // DMX Total Control: a start in the middle (a super-scene block played
    // from the cursor) is that far into its FX: where it would be, its
    // fixtures with a delay past it (start() sets elapsed(), in real ms)
    const qreal startMs = qreal(elapsed()) * speedFactor();
    const qreal synced = tapPeriodMs() > 0 ? 60000.0 / tapPeriodMs() : (timer != nullptr ? timer->bpm() : 120.0);
    {
        QMutexLocker locker(&m_mutex);
        QList<int> movingLinks;
        for (const Layer &layer : std::as_const(m_layers))
            if (layer.link > 0 && isMotion(layer.param))
                movingLinks.append(layer.link);
        m_layerPos.clear();
        m_layerAge.clear();
        m_layerPass.clear();
        m_layerOnBeat.clear();
        m_layerRate.clear();
        for (const Layer &layer : std::as_const(m_layers))
        {
            const bool moves = isMotion(layer.param) || (layer.link > 0 && movingLinks.contains(layer.link));
            const qreal bpm = layer.syncBpm ? synced : layer.bpm;
            const qreal cycles = startMs / 60000.0 * qMax(0.0, bpm) * layer.multiplier * (moves ? motionSpeed() : 1.0);
            m_layerPos.append(cycles);
            m_layerAge.append(cycles);
            m_layerPass.append(0.0);
            // (on the beat of the show from its first frame)
            m_layerOnBeat.append(-1.0);
            m_layerRate.append(0.0);
        }
        m_targetsDirty = true;
        m_valuesStarted = false;
        // DMX Total Control: a start fades as the change it was given
        // (setChangeFade): everything comes in, from nothing
        m_looks.clear();
        m_fadeScale = 1.0;
        if (m_changeSet)
        {
            Look none;
            none.change = m_change;
            m_looks.append(none);
            m_lookKept = true;
        }
    }
    // DMX Total Control: its beats count from the start
    m_beatPos = 0.0;
    m_beatTimed = false;
    m_beats.storeRelaxed(0);
    m_lastBeat = -1.0;
    m_lastAnchors = timer != nullptr ? timer->beatAnchors() : 0;
    // DMX Total Control: a start in the middle (a super-scene block) is
    // already that far into its fade (start() sets elapsed(), in real ms)
    m_fadeElapsed = elapsed();
    if (m_fadersPaused)
        setFadersPaused(false);
    Function::preRun(timer);
}

quint32 FXScene::faderKey(quint32 universe, bool relative)
{
    return relative ? (universe | RelativeFaders) : universe;
}

QSharedPointer<GenericFader> FXScene::getFader(Universe *universe, bool relative)
{
    if (universe == NULL)
        return QSharedPointer<GenericFader>();

    const quint32 key = faderKey(universe->id(), relative);
    QSharedPointer<GenericFader> fader = m_fadersMap.value(key, QSharedPointer<GenericFader>());
    if (fader.isNull())
    {
        // DMX Total Control: the FX without a base go after the faders of the
        // same priority, the ones started later too: they move around what
        // those send (a static scene started after them hid them)
        fader = universe->requestFader(Universe::Auto, playbackPriority(), relative);
        fader->adjustIntensity(getAttributeValue(Intensity));
        fader->setBlendMode(blendMode());
        fader->setName(name());
        fader->setParentFunctionID(id());
        // DMX Total Control: the static values of an FX scene of the show may
        // fire a special effect, never its layers (Modulated); the FX of the
        // programmer, not in the show, never does
        fader->setMayFire(id() != Function::invalidId());
        fader->setHandleSecondary(true);
        // DMX Total Control: its FX go on moving as they fade out (writeDMX)
        fader->setKeepsModulated(true);
        m_fadersMap[key] = fader;
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

int FXScene::beats() const
{
    return m_beats.loadRelaxed() & 0x7fffffff;
}

QList<qreal> FXScene::scalerLevels(const Look &look, qreal phaseShift, qreal syncedBpm, qreal size, qreal fadeFactor)
{
    QList<qreal> levels;
    for (const Scaler &s : look.scalers)
    {
        if (s.layer < 0 || s.layer >= look.layers.count() || s.layer >= look.pos.count())
        {
            levels.append(1.0);
            continue;
        }
        // where the fixture is in the wave of the layer, as for a dimmer
        const Layer &layer = look.layers.at(s.layer);
        const Shape shape = layer.shapeOf(s.fixture);
        const QList<bool> &mirror = look.twins.at(s.layer);
        const int set = s.index < mirror.count() && mirror.at(s.index) ? s.twinSet : s.set;
        const qreal layerBpm = layer.syncBpm ? syncedBpm : layer.bpm;
        qreal pos = positionOf(layer, shape, look.pos.at(s.layer) + phaseShift, set, s.sets, layerBpm);
        if (shape.delay > 0 && look.age.at(s.layer) < delayCycles(layer, shape, layerBpm))
            pos = -1.0;
        const qreal level = pos >= 0.0 ? modeValue(shape.mode, pos) : 0.0;
        // its level fades in from full (a dimmer fades from what the stage
        // shows) and the wave grows in with it
        const int from = layer.virtualBase >= 0 ? layer.virtualBase : s.autoBase;
        const qreal base = 255.0 + (qreal(from) - 255.0) * fadeFactor;
        const qreal value = base + level * qreal(shape.swing) / 100.0 * 0.5 * 255.0 * size * fadeFactor;
        levels.append(qBound(0.0, value, 255.0) / 255.0);
    }
    return levels;
}

FXScene::Wave FXScene::waveIn(const Look &look, const QList<qreal> &scales, int index, qreal phaseShift,
                              qreal syncedBpm)
{
    const Target &t = look.targets.at(index);
    Wave w;
    if (t.layer >= 0 && t.layer < look.layers.count() && t.layer < look.pos.count())
    {
        const Layer &layer = look.layers.at(t.layer);
        // DMX Total Control: what the fixture does its own way, on the clock of the layer
        const Shape shape = layer.shapeOf(t.fixture);
        // in a mirror, the place in the wave of its twin
        const QList<bool> &mirror = look.twins.at(t.layer);
        const int set = t.index < mirror.count() && mirror.at(t.index) ? t.twinSet : t.set;
        const qreal layerBpm = layer.syncBpm ? syncedBpm : layer.bpm;
        qreal pos = positionOf(layer, shape, look.pos.at(t.layer) + phaseShift, set, t.sets, layerBpm);
        // DMX Total Control: a fixture with a delay waits at its base (or what
        // is below) when the FX starts: the layer has gone 0 at the start, for
        // a new layer and at the first tapped beat of a row; one that goes on
        // never waits again
        if (shape.delay > 0 && look.age.at(t.layer) < delayCycles(layer, shape, layerBpm))
            pos = -1.0;
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
        w.wave = level * qreal(swing) / 100.0 * 0.5;
    }
    // DMX Total Control: dimmed by its virtual dimmer
    if (t.scaler >= 0 && t.scaler < scales.count())
        w.scale = scales.at(t.scaler);
    return w;
}

FXScene::Wave FXScene::chainWave(const QList<const Look *> &chain, const QList<QList<qreal>> &scales,
                                 const Key &key, int upTo, qreal phaseShift, qreal syncedBpm)
{
    // DMX Total Control: from the oldest look on, each one mixed into the
    // next one as far as that channel is on its way; one a look doesn't
    // have comes in from nothing
    Wave r;
    bool has = false;
    for (int i = 0; i <= upTo && i < chain.count(); i++)
    {
        const Look &look = *chain.at(i);
        const int index = look.index.value(key, -1);
        if (index < 0)
        {
            r = Wave();
            has = false;
            continue;
        }
        const Wave v = waveIn(look, scales.at(i), index, phaseShift, syncedBpm);
        if (i == 0)
        {
            r = v;
            has = true;
            continue;
        }
        const qreal k = chain.at(i - 1)->progress(key);
        const Wave from = has ? r : Wave();
        r.wave = from.wave + (v.wave - from.wave) * k;
        r.scale = from.scale + (v.scale - from.scale) * k;
        has = true;
    }
    return r;
}

QHash<FXScene::Key, FXScene::Wave> FXScene::leavingWaves(const QList<const Look *> &chain,
                                                         const QList<QList<qreal>> &scales,
                                                         qreal phaseShift, qreal syncedBpm)
{
    QHash<Key, Wave> waves;
    if (chain.isEmpty())
        return waves;
    // (the last look plays them, or nothing: none of them leaves)
    const Look &last = *chain.last();
    QSet<Key> seen;
    for (int i = int(chain.count()) - 2; i >= 0; i--)
    {
        for (const Target &t : chain.at(i)->targets)
        {
            const Key key(t.fixture, t.channel);
            if ((t.layer < 0 && t.scaler < 0) || last.index.contains(key) || seen.contains(key))
                continue;
            seen.insert(key);
            waves.insert(key, chainWave(chain, scales, key, i, phaseShift, syncedBpm));
        }
    }
    return waves;
}

void FXScene::advanceLooks(QList<Look> &looks, const QList<qreal> &lastPos, const QList<qreal> &lastAge, uint ms)
{
    // the newest first: a layer that goes on in the next look is at its place
    for (int i = int(looks.count()) - 1; i >= 0; i--)
    {
        const QList<qreal> nextPos = i + 1 < looks.count() ? looks.at(i + 1).pos : lastPos;
        const QList<qreal> nextAge = i + 1 < looks.count() ? looks.at(i + 1).age : lastAge;
        Look &look = looks[i];
        for (int l = 0; l < look.pos.count(); l++)
        {
            const int next = l < look.next.count() ? look.next.at(l) : -1;
            if (next >= 0 && next < nextPos.count() && next < nextAge.count())
            {
                look.pos[l] = nextPos.at(next);
                look.age[l] = nextAge.at(next);
                continue;
            }
            const qreal step = look.rate.value(l, 0.0) * qreal(ms);
            look.pos[l] += step;
            look.age[l] += qAbs(step);
        }
        look.elapsed += ms;
    }
}

void FXScene::writeLeaving(const QList<QSharedPointer<GenericFader>> &faders, const QHash<Key, Wave> &waves,
                           qreal size, qreal fadeFactor)
{
    // (only the channels still there: the fader drops each one at the end of its fade)
    for (const QSharedPointer<GenericFader> &fader : faders)
    {
        if (fader.isNull())
            continue;
        fader->forEachChannel([&](FadeChannel &fc) {
            if ((fc.flags() & FadeChannel::Modulated) == 0 || fc.modulationLeaving() == false)
                return;
            auto it = waves.constFind(Key(fc.fixture(), fc.channel()));
            if (it == waves.cend())
                return;
            const quint32 fine = fc.modulationFine();
            const qreal range = fine != QLCChannel::invalid() && fine < UNIVERSE_SIZE ? 65535.0 : 255.0;
            const qint32 offset = qint32(std::round(it->wave * range * size * fadeFactor));
            fc.setModulation(offset, fc.modulatesBelow(), fine, it->scale);
        });
    }
}

qreal FXScene::Look::progress(const Key &key) const
{
    const QPair<uint, uint> time = change.timeOf(key);
    if (elapsed < time.second)
        return 0.0;
    if (time.first == 0)
        return 1.0;
    const qreal p = qMin(1.0, qreal(elapsed - time.second) / qreal(time.first));
    // (as a fade on the curve of the change)
    switch (change.curve)
    {
        case FadeChannel::CurveSmooth: return p * p * (3.0 - 2.0 * p);
        case FadeChannel::CurveSlowStart: return p * p;
        case FadeChannel::CurveFastStart: return 1.0 - (1.0 - p) * (1.0 - p);
        default: return p;
    }
}

bool FXScene::Look::done() const
{
    quint64 end = quint64(change.fade) + change.delay;
    for (const QPair<uint, uint> &time : change.times)
        end = qMax(end, quint64(time.first) + time.second);
    return elapsed >= end;
}

FXScene::Look FXScene::currentLookLocked() const
{
    Look look;
    look.layers = m_layers;
    look.targets = m_targets;
    look.index = m_targetIndex;
    look.scalers = m_scalers;
    for (int l = 0; l < m_layers.count(); l++)
    {
        look.twins.append(l < m_layerOrders.count() ? mirrorTwins(m_layers.at(l), m_layerOrders.at(l))
                                                    : QList<bool>());
        look.pos.append(m_layerPos.value(l, 0.0));
        look.age.append(m_layerAge.value(l, 0.0));
        look.rate.append(m_layerRate.value(l, 0.0));
        look.next.append(l);
    }
    return look;
}

void FXScene::keepLookLocked()
{
    // DMX Total Control: once for a change that fades, while it plays (not
    // a change of targets to come: what it played)
    if (m_changeSet == false || m_lookKept || isRunning() == false || m_targetsDirty)
        return;
    Look look = currentLookLocked();
    look.change = m_change;
    m_looks.append(look);
    m_lookKept = true;
    // (a long row of them: the oldest ones are in)
    while (m_looks.count() > 8)
        m_looks.removeFirst();
}

uint FXScene::fadeInTime(MasterTimer *timer) const
{
    uint fadeIn = overrideFadeInSpeed() == defaultSpeed() ? fadeInSpeed() : overrideFadeInSpeed();
    // DMX Total Control: in beats when it was given in beats: a group gives
    // its crossfade in ms (the tempo it was started with), as a sequence
    // takes it (ChaserRunner::stepFadeTime)
    const bool overridden = overrideFadeInSpeed() != defaultSpeed();
    if ((overridden ? overrideTempoType() : tempoType()) == Beats && timer != nullptr)
        fadeIn = beatsToTime(fadeIn, timer->beatTimeDuration());
    return uint(qMin(qreal(UINT_MAX), qreal(fadeIn) * m_fadeScale));
}

void FXScene::write(MasterTimer *timer, QList<Universe *> universes)
{
    // DMX Total Control: paused, the faders hold too (the base of a layer
    // fading in, a static value), not only the waves
    if (isPaused() != m_fadersPaused)
        setFadersPaused(isPaused());
    if (isPaused())
    {
        // DMX Total Control: it goes on from where it stopped
        m_lastBeat = timer->beatPosition();
        return;
    }

    advanceFrameTick();

    QMutexLocker locker(&m_mutex);
    // DMX Total Control: a change that fades (setChangeFade): this frame
    // takes it
    const bool changing = m_changeSet;
    const Change change = m_change;
    bool rebuilt = false;
    if (m_targetsDirty)
    {
        // channels that are not played any more go back to what's below.
        // DMX Total Control: with a change that fades, in its time: an FX goes
        // on moving as it goes there (the look before it moves it)
        QList<Target> old = m_targets;
        buildTargets();
        for (const Target &o : std::as_const(old))
        {
            // DMX Total Control: of the same kind, modulated (a layer or a
            // virtual dimmer) or static: a channel that leaves the virtual
            // dimmer is static again, not scaled any more
            const bool oldModulated = o.layer >= 0 || o.scaler >= 0;
            const Key key(o.fixture, o.channel);
            const int now = m_targetIndex.value(key, -1);
            const bool sameKind = now >= 0 && (m_targets.at(now).layer >= 0 || m_targets.at(now).scaler >= 0) == oldModulated;
            // DMX Total Control: in the same universe (a fixture moved to
            // another one leaves this one) and in the same fader
            const bool here = sameKind && m_targets.at(now).universe == o.universe;
            const bool kept = here && m_targets.at(now).relative == o.relative;
            if (kept || o.universe >= quint32(universes.count()))
                continue;
            Universe *universe = universes.at(int(o.universe));
            QSharedPointer<GenericFader> fader = m_fadersMap.value(faderKey(universe->id(), o.relative));
            if (fader.isNull())
                continue;
            if (here)
            {
                // DMX Total Control: a base given or taken under the FX: the
                // channel goes on as it is in the other fader
                FadeChannel moving;
                if (fader->channelCopy(o.fixture, o.channel, &moving))
                {
                    FadeChannel gone(doc(), o.fixture, o.channel);
                    fader->remove(&gone);
                    QSharedPointer<GenericFader> to = getFader(universe, m_targets.at(now).relative);
                    if (to.isNull() == false)
                        to->replace(moving);
                }
                continue;
            }
            const QPair<uint, uint> time = changing ? change.timeOf(key) : qMakePair(0u, 0u);
            if (now < 0 && (time.first > 0 || time.second > 0) && doc()->fixture(o.fixture) != nullptr)
            {
                const qreal level = fader->intensity() * fader->parentIntensity();
                fader->updateChannel(doc(), universe, o.fixture, o.channel, [&](FadeChannel &fc)
                {
                    // an FX: it fades to what's below as it goes on moving
                    if (fc.flags() & FadeChannel::Modulated)
                    {
                        if (fc.modulationLeaving() == false)
                            fc.setModulationFade(time.first, time.second);
                        return;
                    }
                    // a static value: to what's below, then it leaves
                    const bool scaled = fc.canFade() && (fc.flags() & FadeChannel::Intensity);
                    const quint32 out = scaled ? fc.current(level) : fc.current();
                    fc.setStart(out);
                    fc.setCurrent(out);
                    fc.addFlag(FadeChannel::SetTarget);
                    fc.addFlag(FadeChannel::AutoRemove);
                    fc.setTarget(quint32(0));
                    fc.setElapsed(0);
                    fc.setReady(false);
                    fc.setFadeTime(fc.canFade() ? time.first : 0);
                    fc.setDelay(time.second);
                    fc.setCurve(FadeChannel::CurveLinear);
                });
                continue;
            }
            // the fader forgets it: from the next frame it is what's below
            FadeChannel gone(doc(), o.fixture, o.channel);
            fader->remove(&gone);
        }
        rebuilt = true;
    }

    const uint fadeIn = fadeInTime(timer);
    // DMX Total Control: in real ms, as the bases fade (FadeChannels): at a
    // live speed the waves still grow in with them
    const qreal fadeFactor = fadeIn == 0 ? 1.0 : qMin(1.0, qreal(m_fadeElapsed) / qreal(fadeIn));
    const qreal size = getAttributeValue(Size);
    const qreal phaseShift = getAttributeValue(Phase) / 360.0;
    const qreal dt = qreal(frameTick());

    // DMX Total Control: the layers that follow the BPM of the show go at the
    // rhythm tapped on its button when it has one (a cycle at x1 a beat), else
    // on the beat of the show (followsShowBeat). The first tapped beat of a
    // row starts their cycle again, as a start does (their phase and wave
    // stay); each next one of the row puts them on the nearest whole beat of
    // their clock (a chase on its next set, x1/4 a quarter further, a press a
    // bar later on the beat it is on)
    const qreal tap = tapPeriodMs();
    const bool showBeat = tap <= 0;
    const qreal syncedBpm = qMax(0.0, showBeat ? timer->bpm() : 60000.0 / tap);
    // the beat of the show: how far it went since the last frame, and
    // whether the TAP moved it (all of them go back on it)
    const qreal beatNow = timer->beatPosition();
    const qreal beatBefore = m_lastBeat;
    const qreal beatStep = beatBefore < 0 ? 0.0 : beatNow - beatBefore;
    m_lastBeat = beatNow;
    const bool anchored = timer->beatAnchors() != m_lastAnchors;
    m_lastAnchors = timer->beatAnchors();
    const TappedBeat tapped = takeTapBeat();
    // its beats go as its FX: on the beat of the show from the start and
    // after a TAP (a beat it passed counts), else from its last tapped beat
    if (showBeat && (beatBefore < 0 || anchored))
    {
        if (beatBefore >= 0 && std::floor(beatNow + 1e-7) > std::floor(beatBefore + 1e-7))
        {
            m_beats.fetchAndAddRelaxed(1);
            m_beatTimed = true;
        }
        m_beatPos = beatNow - std::floor(beatNow + 1e-7);
    }
    else
    {
        m_beatPos += showBeat ? qMax(0.0, beatStep) * speedFactor() : dt / 60000.0 * syncedBpm;
    }
    if (tapped != NoBeatTapped)
    {
        // a beat of its own; but the next one of a row less than half a beat
        // after a beat of the clock is that beat, tapped late: it only times
        // the next ones (taps in time come a little before or after them)
        const bool late = tapped == NextBeatTapped && m_beatTimed && m_beatPos < 0.5;
        if (late == false)
            m_beats.fetchAndAddRelaxed(1);
        m_beatTimed = false;
        m_beatPos = 0.0;
    }
    if (m_beatPos >= 1.0 - 1e-7)
    {
        const int whole = int(m_beatPos + 1e-7);
        m_beatPos = qMax(0.0, m_beatPos - whole);
        m_beatTimed = true;
        m_beats.fetchAndAddRelaxed(whole);
    }

    // each layer goes at its own speed; a new speed goes on from where it is.
    // The pan/tilt layers, at the speed of the movement too. DMX Total
    // Control: and the layers linked to one, which go on its clock
    QList<int> movingLinks;
    for (const Layer &layer : std::as_const(m_layers))
        if (layer.link > 0 && isMotion(layer.param))
            movingLinks.append(layer.link);
    for (int l = 0; l < m_layers.count(); l++)
    {
        const Layer &layer = m_layers.at(l);
        qreal bpm = layer.syncBpm ? syncedBpm : layer.bpm;
        const bool moves = isMotion(layer.param) || (layer.link > 0 && movingLinks.contains(layer.link));
        const qreal motion = moves ? motionSpeed() : 1.0;
        // cycles a beat (of the show, or of its own BPM)
        const qreal rate = layer.multiplier * motion;
        const bool onBeat = showBeat && layer.syncBpm;
        // DMX Total Control: how fast it goes, in real ms (a look it leaves
        // behind goes on so)
        if (l < m_layerRate.count())
            m_layerRate[l] = qMax(0.0, bpm) / 60000.0 * rate * speedFactor();
        // DMX Total Control: a new length of its pass (another speed, BPM,
        // multiplier, wait, sets) goes on from where it is in it (the place
        // in the pass was the cycles since the start modulo the new length:
        // a jump, the larger the longer it played)
        const qreal pass = passCycles(layer, m_layerSets.value(l, 1), bpm);
        const qreal oldPass = m_layerPass.value(l, 0.0);
        if (onBeat == false && pass > 0 && oldPass > 0 && qAbs(pass - oldPass) > 1e-9)
        {
            qreal passes = std::floor(m_layerPos.at(l) / oldPass);
            qreal in = m_layerPos.at(l) - passes * oldPass;
            if (in >= pass)
            {
                passes += 1.0;
                in = 0.0;
            }
            m_layerPos[l] = passes * pass + in;
        }
        m_layerPass[l] = pass;

        qreal step = onBeat ? beatStep * rate * speedFactor() : dt / 60000.0 * qMax(0.0, bpm) * rate;
        if (tapped != NoBeatTapped && layer.syncBpm)
        {
            if (tapped == FirstBeatTapped)
            {
                m_layerPos[l] = 0.0;
                m_layerAge[l] = 0.0;
            }
            else if (rate > 0)
            {
                m_layerPos[l] = std::round(m_layerPos.at(l) / rate) * rate;
            }
            // (a press puts it where it says: it stays there)
            m_layerOnBeat[l] = onBeat ? layer.multiplier : -1.0;
        }
        else if (onBeat && (anchored || m_layerOnBeat.at(l) != layer.multiplier))
        {
            // on the beat of the show: at a start, a new multiplier, a TAP
            m_layerPos[l] = beatNow * rate * speedFactor();
            m_layerOnBeat[l] = layer.multiplier;
            step = 0.0;
        }
        else if (onBeat == false)
        {
            m_layerOnBeat[l] = -1.0;
        }
        m_layerPos[l] += step;
        m_layerAge[l] += qMax(0.0, onBeat ? beatStep * rate * speedFactor() : step);
    }

    // DMX Total Control: the looks under a change go on with it, the layers
    // that go on at their new places
    advanceLooks(m_looks, m_layerPos, m_layerAge, MasterTimer::tick());

    // DMX Total Control: what plays now (the layers that are a mirror: what
    // the fixtures do their own way changes live, without new targets), and
    // the looks it goes on from, each with the level of its virtual dimmers
    const Look now = currentLookLocked();
    QList<const Look *> chain;
    QList<QList<qreal>> scales;
    for (const Look &look : std::as_const(m_looks))
    {
        chain.append(&look);
        scales.append(scalerLevels(look, phaseShift, syncedBpm, size, fadeFactor));
    }
    chain.append(&now);
    scales.append(scalerLevels(now, phaseShift, syncedBpm, size, fadeFactor));
    const int last = int(chain.count()) - 1;

    const bool startValues = m_valuesStarted == false || rebuilt;
    for (const Target &t : std::as_const(m_targets))
    {
        if (t.universe >= quint32(universes.count()))
            continue;
        Universe *universe = universes.at(int(t.universe));
        QSharedPointer<GenericFader> fader = getFader(universe, t.relative);
        if (fader.isNull())
            continue;
        const Key key(t.fixture, t.channel);
        // DMX Total Control: a change that fades takes the time of the
        // channel, the start the fade in; a live edit applies at once
        const bool first = m_valuesStarted == false;
        const QPair<uint, uint> time = changing ? change.timeOf(key) : qMakePair(first ? fadeIn : 0u, 0u);
        const int curve = changing ? change.curve : int(FadeChannel::CurveLinear);

        if (t.layer < 0 && t.scaler < 0)
        {
            // a static value: set once, it fades in like a scene
            if (startValues == false)
                continue;
            const quint32 channel = t.channel;
            const uchar value = uchar(t.base);
            const qreal level = fader->intensity() * fader->parentIntensity();
            fader->updateChannel(doc(), universe, t.fixture, channel, [=](FadeChannel &fc)
            {
                int chIndex = fc.channelIndex(channel);
                // DMX Total Control: it was an FX: from what it sent
                if (fc.flags() & FadeChannel::Modulated)
                {
                    quint32 out = fc.modulatedOutput();
                    if (fc.modulationFine() != QLCChannel::invalid())
                        out >>= 8;
                    if (fc.canFade() && (fc.flags() & FadeChannel::Intensity))
                        out = level > 0.004 ? quint32(qMin(255.0, std::round(qreal(out) / level))) : value;
                    fc.removeFlag(FadeChannel::Modulated);
                    fc.removeFlag(FadeChannel::SetTarget);
                    fc.setModulation(0, false);
                    fc.cancelModulationFade();
                    fc.setCurrent(out);
                }
                fc.setStart(fc.current(chIndex), chIndex);
                fc.setTarget(value, chIndex);
                fc.setElapsed(0);
                fc.setReady(false);
                fc.setFadeTime(fc.canFade() ? time.first : 0);
                fc.setDelay(time.second);
                fc.setCurve(curve);
            });
            continue;
        }

        const bool wide = t.fine != QLCChannel::invalid();
        // DMX Total Control: the wave of the looks it goes on from mixed into
        // this one (a change that fades); the emitter of a virtual dimmer
        // that no layer moves sends its base (or what is below) dimmed,
        // without a wave
        const Wave w = chainWave(chain, scales, key, last, phaseShift, syncedBpm);
        const qreal range = wide ? 65535.0 : 255.0;
        const qint32 offset = qint32(std::round(w.wave * range * size * fadeFactor));
        const qreal scale = w.scale;
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
        const qreal maxValue = wide ? 65535.0 : 255.0;

        fader->updateChannel(doc(), universe, t.fixture, t.channel, [=](FadeChannel &fc)
        {
            const bool fresh = (fc.flags() & FadeChannel::Modulated) == 0;
            const bool intensity = fc.canFade() && (fc.flags() & FadeChannel::Intensity);
            // DMX Total Control: played again, it stays
            if (fc.modulationLeaving())
                fc.cancelModulationFade();
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
                    if (fresh && intensity)
                    {
                        from = compIntensity > 0.004
                                   ? quint32(qMin(maxValue, std::round(qreal(from) / compIntensity)))
                                   : base;
                    }
                    else if (fresh == false && (fc.modulatesBelow() || (fc.flags() & FadeChannel::SetTarget)))
                    {
                        // DMX Total Control: it was on what is below (or on its
                        // way there): from that, without its wave
                        qint64 under = fc.modulatesBelow()
                                           ? qint64(fc.modulatedOutput()) -
                                                 qint64(qreal(fc.modulationOffset()) * (intensity ? compIntensity : 1.0))
                                           : qint64(fc.current());
                        under = qBound(qint64(0), under, qint64(maxValue));
                        from = intensity ? (compIntensity > 0.004
                                                ? quint32(qMin(maxValue, std::round(qreal(under) / compIntensity)))
                                                : base)
                                         : quint32(under);
                    }
                    fc.removeFlag(FadeChannel::SetTarget);
                    fc.setCurrent(from);
                    fc.setStart(from);
                    fc.setTarget(base);
                    fc.setElapsed(0);
                    fc.setReady(false);
                    fc.setFadeTime(fc.canFade() ? time.first : 0);
                    fc.setDelay(time.second);
                    fc.setCurve(curve);
                }
                else if (fresh == false && fc.modulatesBelow() == false && (fc.flags() & FadeChannel::SetTarget) == 0 &&
                         fc.canFade() && (time.first > 0 || time.second > 0))
                {
                    // DMX Total Control: its base released under the FX: it
                    // goes to what is below in the time of the change, then
                    // on it (GenericFader aims it there: values out of the
                    // faders, the level in them)
                    const quint32 out = intensity ? fc.current(compIntensity) : fc.current();
                    fc.setCurrent(out);
                    fc.setStart(out);
                    fc.setTarget(out);
                    fc.addFlag(FadeChannel::SetTarget);
                    fc.setElapsed(0);
                    fc.setReady(false);
                    fc.setFadeTime(time.first);
                    fc.setDelay(time.second);
                    fc.setCurve(curve);
                }
            }
            // (there: on what is below from now on)
            if (hasBase == false && (fc.flags() & FadeChannel::SetTarget) && fc.isReady())
                fc.removeFlag(FadeChannel::SetTarget);
            const bool onBelow = hasBase == false && (fc.flags() & FadeChannel::SetTarget) == 0;
            fc.setModulation(offset, onBelow, fineAddress, scale);
        });
    }

    // DMX Total Control: what leaves goes on moving as the look it was in
    // has it, while it fades to what is below
    if (m_looks.isEmpty() == false)
    {
        const QHash<Key, Wave> leaving = leavingWaves(chain, scales, phaseShift, syncedBpm);
        if (leaving.isEmpty() == false)
            writeLeaving(m_fadersMap.values(), leaving, size, fadeFactor);
    }
    chain.clear();
    // the looks that are all in go
    while (m_looks.isEmpty() == false && m_looks.first().done())
        m_looks.removeFirst();
    m_changeSet = false;
    m_lookKept = false;
    m_change = Change();

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

    const bool overridden = overrideFadeOutSpeed() != defaultSpeed();
    uint fadeout = overridden ? overrideFadeOutSpeed() : fadeOutSpeed();
    // DMX Total Control: the faders that fade out, whose FX go on moving
    QList<QSharedPointer<GenericFader>> fading;
    if (fadeout == 0)
    {
        // (DMX Total Control: the ones a stop all fades out already stay in
        // the universe to the end of it)
        for (const QSharedPointer<GenericFader> &fader : std::as_const(m_fadersMap))
            if (fader.isNull() == false && fader->isFadingOut())
                fading.append(fader);
        dismissAllFaders();
    }
    else
    {
        // DMX Total Control: in beats when it was given in beats (a group
        // gives its crossfade in ms)
        if ((overridden ? overrideTempoType() : tempoType()) == Beats && timer != nullptr)
            fadeout = beatsToTime(fadeout, timer->beatTimeDuration());
        // DMX Total Control: a pause of the cuelist that plays the scene
        // holds them too (setFadersPaused)
        for (QSharedPointer<GenericFader> fader : std::as_const(m_fadersMap))
        {
            if (!fader.isNull())
            {
                fader->setFadeOut(true, fadeout);
                fading.append(fader);
            }
        }
        m_fadersMap.clear();
    }

    bool source = false;
    {
        QMutexLocker locker(&m_mutex);
        // (the ones of the stops before that are over have left the universe)
        m_tails.removeIf([](const Tail &tail) {
            for (const QSharedPointer<GenericFader> &fader : tail.faders)
                if (fader.isNull() == false && (fader->deleteRequested() == false || fader->isFadingOut()))
                    return false;
            return true;
        });
        if (fading.isEmpty() == false)
        {
            // DMX Total Control: what played goes on as it fades out
            Tail tail;
            tail.looks = m_looks;
            Look look = currentLookLocked();
            look.next.fill(-1);
            tail.looks.append(look);
            tail.faders = fading;
            tail.phaseShift = getAttributeValue(Phase) / 360.0;
            tail.size = getAttributeValue(Size);
            const uint fadeIn = fadeInTime(timer);
            tail.fadeFactor = fadeIn == 0 ? 1.0 : qMin(1.0, qreal(m_fadeElapsed) / qreal(fadeIn));
            tail.syncedBpm = tapPeriodMs() > 0 ? 60000.0 / tapPeriodMs()
                                               : (timer != nullptr ? qMax(0.0, timer->bpm()) : 120.0);
            m_tails.append(tail);
        }
        m_looks.clear();
        m_changeSet = false;
        m_lookKept = false;
        m_change = Change();
        if (m_tails.isEmpty() == false && m_tailSource == false)
        {
            m_tailSource = true;
            source = true;
        }
    }
    if (source && timer != nullptr)
        timer->registerDMXSource(this);

    Function::postRun(timer, universes);
}

void FXScene::writeDMX(MasterTimer *timer, QList<Universe *> universes)
{
    Q_UNUSED(universes)

    QMutexLocker locker(&m_mutex);
    for (int i = int(m_tails.count()) - 1; i >= 0; i--)
    {
        Tail &tail = m_tails[i];
        // the faders at the end of their fade have left the universe
        tail.faders.removeIf([](const QSharedPointer<GenericFader> &fader) {
            return fader.isNull() || (fader->deleteRequested() && fader->isFadingOut() == false);
        });
        if (tail.faders.isEmpty())
        {
            m_tails.removeAt(i);
            continue;
        }
        if (tail.looks.isEmpty())
            continue;
        // (a cuelist that plays it holds them paused)
        bool paused = false;
        for (const QSharedPointer<GenericFader> &fader : std::as_const(tail.faders))
            paused = paused || fader->isPaused();
        if (paused == false)
            advanceLooks(tail.looks, QList<qreal>(), QList<qreal>(), MasterTimer::tick());

        // every channel leaves: each one as the last look it is in has it
        Look none;
        QList<const Look *> chain;
        QList<QList<qreal>> scales;
        for (const Look &look : std::as_const(tail.looks))
        {
            chain.append(&look);
            scales.append(scalerLevels(look, tail.phaseShift, tail.syncedBpm, tail.size, tail.fadeFactor));
        }
        chain.append(&none);
        scales.append(QList<qreal>());
        const QHash<Key, Wave> waves = leavingWaves(chain, scales, tail.phaseShift, tail.syncedBpm);
        writeLeaving(tail.faders, waves, tail.size, tail.fadeFactor);
        chain.clear();
        // (the last look stays to the end)
        while (tail.looks.count() > 1 && tail.looks.first().done())
            tail.looks.removeFirst();
    }
    if (m_tails.isEmpty() && m_tailSource)
    {
        m_tailSource = false;
        if (timer != nullptr)
            timer->unregisterDMXSource(this);
    }
}

void FXScene::setFadersPaused(bool paused)
{
    m_fadersPaused = paused;
    // DMX Total Control: resumed, its FX on the beat of the show go on from
    // where they stopped (a player that wasn't written while paused)
    if (paused == false)
        m_lastBeat = -1.0;
    for (const QSharedPointer<GenericFader> &fader : std::as_const(m_fadersMap))
    {
        if (!fader.isNull())
            fader->setPaused(paused);
    }

    // the ones its stops left fading out (the ones that ended their fade out
    // have left the universe)
    QMutexLocker locker(&m_mutex);
    for (const Tail &tail : std::as_const(m_tails))
    {
        for (const QSharedPointer<GenericFader> &fader : tail.faders)
        {
            if (fader.isNull() == false)
                fader->setPaused(paused);
        }
    }
}

void FXScene::setChangeFade(uint ms, uint delayMs, int curve, const QHash<QPair<quint32, quint32>, QPair<uint, uint>> &times)
{
    QMutexLocker locker(&m_mutex);
    m_change.fade = ms;
    m_change.delay = delayMs;
    m_change.curve = curve;
    m_change.times = times;
    m_changeSet = true;
    m_lookKept = false;
}

void FXScene::scaleFades(qreal factor)
{
    // DMX Total Control: as a cuelist scales its own (Cuelist::scaleFadesLocked)
    const bool now = factor <= 0.0;
    auto scaled = [factor](quint64 ms) {
        return uint(qBound(qint64(0), qint64(std::ceil(qreal(ms) * factor)), qint64(UINT_MAX)));
    };
    auto scaleLook = [&](Look &look) {
        if (now)
        {
            look.elapsed = std::numeric_limits<quint64>::max() / 2;
            return;
        }
        look.elapsed = scaled(look.elapsed);
        look.change.fade = scaled(look.change.fade);
        look.change.delay = scaled(look.change.delay);
        for (QPair<uint, uint> &time : look.change.times)
            time = qMakePair(scaled(time.first), scaled(time.second));
    };
    auto scaleChannel = [&](FadeChannel &fc) {
        // what leaves its FX
        if (fc.modulationLeaving())
        {
            if (now)
                fc.setModulationFade(0, 0, 1);
            else
                fc.setModulationFade(scaled(fc.modulationFadeTime()), scaled(fc.modulationFadeDelay()),
                                     scaled(fc.modulationFadeElapsed()));
        }
        // a base or a static value on its way (not one on what is below)
        if (((fc.flags() & FadeChannel::Modulated) && fc.modulatesBelow()) || fc.isReady())
            return;
        const quint64 delay = fc.delay();
        const quint64 fade = fc.fadeTime();
        if (fc.elapsed() >= delay + fade)
            return;
        if (now)
        {
            fc.setReady(true);
            return;
        }
        fc.setDelay(scaled(delay));
        fc.setFadeTime(scaled(fade));
        fc.setElapsed(scaled(fc.elapsed()));
    };

    QMutexLocker locker(&m_mutex);
    // its fade in
    if (now)
    {
        m_fadeScale = 0.0;
    }
    else
    {
        m_fadeScale *= factor;
        m_fadeElapsed = scaled(m_fadeElapsed);
    }
    for (Look &look : m_looks)
        scaleLook(look);
    QList<QSharedPointer<GenericFader>> faders = m_fadersMap.values();
    for (Tail &tail : m_tails)
    {
        for (Look &look : tail.looks)
            scaleLook(look);
        faders.append(tail.faders);
    }
    for (const QSharedPointer<GenericFader> &fader : std::as_const(faders))
    {
        if (fader.isNull() == false)
            fader->forEachChannel(scaleChannel);
    }
}

int FXScene::heldBase(quint32 fixture, quint32 channel) const
{
    QMutexLocker locker(&m_mutex);
    for (const Target &t : m_targets)
    {
        if (t.fixture != fixture || (t.layer < 0 && t.scaler < 0) || t.hasBase == false)
            continue;
        const bool fineByte = t.fine != QLCChannel::invalid() && channel == t.fine;
        if (channel != t.channel && fineByte == false)
            continue;
        const QSharedPointer<GenericFader> fader = m_fadersMap.value(t.universe);
        FadeChannel fc;
        if (fader.isNull() || fader->channelCopy(t.fixture, t.channel, &fc) == false ||
            (fc.flags() & FadeChannel::Modulated) == 0 || fc.modulatesBelow())
            return -1;
        const quint32 value = fc.current();
        if (t.fine != QLCChannel::invalid())
            return int(fineByte ? value & 0xFF : (value >> 8) & 0xFF);
        return int(value & 0xFF);
    }
    return -1;
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
