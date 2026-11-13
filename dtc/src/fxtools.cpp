/*
  DMX Total Control
  fxtools.cpp

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

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSaveFile>
#include <QFile>
#include <QDir>
#include <QUuid>
#include <QDebug>
#include <climits>
#include <cmath>

#include "fxtools.h"
#include "programmer.h"
#include "qlcconfig.h"
#include "qlcchannel.h"
#include "efxfixture.h"
#include "curvefx.h"
#include "qlcfile.h"
#include "fixture.h"
#include "efx.h"
#include "doc.h"

/*********************************************************************
 * Layers
 *********************************************************************/

QVariantMap FxTools::layerToMap(const FXScene::Layer &layer)
{
    QVariantList fixtures;
    for (quint32 fid : layer.fixtures)
        fixtures.append(fid);

    return {
        { "param", layer.param },
        { "panTilt", layer.isPanTilt() },
        { "mode", layer.mode },
        { "figure", layer.figure },
        { "swing", layer.swing },
        { "swingTilt", layer.swingTilt },
        { "bpm", layer.bpm },
        { "syncBpm", layer.syncBpm },
        { "multiplier", layer.multiplier },
        { "wave", layer.wave },
        { "step", layer.step },
        { "grouping", layer.grouping },
        { "groupSize", layer.groupSize },
        { "order", layer.order },
        { "seed", layer.seed },
        { "phase", layer.phase },
        { "wait", int(layer.wait) },
        { "rotate", layer.rotate },
        { "panInvert", layer.panInvert },
        { "tiltInvert", layer.tiltInvert },
        { "reverse", layer.reverse },
        { "linked", layer.linked },
        // DMX Total Control: the level of the virtual dimmer (-1: auto)
        { "virtualBase", layer.virtualBase },
        { "fixtures", fixtures }
    };
}

FXScene::Layer FxTools::layerFromMap(const QVariantMap &map, const FXScene::Layer &base)
{
    FXScene::Layer layer = base;
    if (map.contains("param")) layer.param = map.value("param").toString().toLower();
    if (map.contains("mode")) layer.mode = qBound(1, map.value("mode").toInt(), FXScene::ModeCount);
    if (map.contains("figure")) layer.figure = qBound(0, map.value("figure").toInt(), int(FXScene::FigureCount) - 1);
    if (map.contains("swing")) layer.swing = qBound(0, map.value("swing").toInt(), 250);
    if (map.contains("swingTilt")) layer.swingTilt = qBound(0, map.value("swingTilt").toInt(), 250);
    if (map.contains("bpm")) layer.bpm = qBound(0.0, map.value("bpm").toDouble(), 600.0);
    if (map.contains("syncBpm")) layer.syncBpm = map.value("syncBpm").toBool();
    if (map.contains("multiplier")) layer.multiplier = qBound(0.0625, map.value("multiplier").toDouble(), 16.0);
    if (map.contains("wave")) layer.wave = qMax(-1, map.value("wave").toInt());
    if (map.contains("step")) layer.step = qMax(-1, map.value("step").toInt());
    if (map.contains("grouping")) layer.grouping = qBound(0, map.value("grouping").toInt(), int(FXScene::ByGroup));
    if (map.contains("groupSize")) layer.groupSize = qMax(1, map.value("groupSize").toInt());
    if (map.contains("order")) layer.order = qBound(0, map.value("order").toInt(), int(FXScene::Sorted));
    if (map.contains("seed")) layer.seed = map.value("seed").toInt();
    if (map.contains("phase")) layer.phase = ((map.value("phase").toInt() % 360) + 360) % 360;
    if (map.contains("wait")) layer.wait = uint(qMax(0, map.value("wait").toInt()));
    if (map.contains("rotate")) layer.rotate = ((map.value("rotate").toInt() % 360) + 360) % 360;
    if (map.contains("panInvert")) layer.panInvert = map.value("panInvert").toBool();
    if (map.contains("tiltInvert")) layer.tiltInvert = map.value("tiltInvert").toBool();
    if (map.contains("reverse")) layer.reverse = map.value("reverse").toBool();
    if (map.contains("linked")) layer.linked = map.value("linked").toBool();
    if (map.contains("virtualBase")) layer.virtualBase = qBound(-1, map.value("virtualBase").toInt(), 255);
    if (map.contains("fixtures"))
    {
        layer.fixtures.clear();
        for (const QVariant &v : map.value("fixtures").toList())
            layer.fixtures.append(v.toUInt());
    }
    return layer;
}

FXScene::Layer FxTools::defaultLayer(const QString &param)
{
    FXScene::Layer layer;
    layer.param = param;
    layer.bpm = 30;
    if (layer.isPanTilt())
    {
        layer.figure = FXScene::Circle;
        layer.swing = 20;
        layer.swingTilt = 20;
    }
    else
    {
        layer.mode = 1;
        layer.swing = 50;
    }
    return layer;
}

QStringList FxTools::timingKeys()
{
    return { "bpm", "syncBpm", "multiplier", "wave", "step", "grouping", "groupSize", "order", "seed", "wait" };
}

QStringList FxTools::ownKeys()
{
    return { "mode", "figure", "swing", "swingTilt", "phase", "rotate", "panInvert", "tiltInvert", "reverse" };
}

void FxTools::setOwn(FXScene::Layer &layer, const QList<quint32> &fixtures, const QVariantMap &changes)
{
    // the values as the layer would take them
    const FXScene::Layer bounded = layerFromMap(changes, layer);
    for (quint32 fixture : fixtures)
    {
        if (layer.fixtures.contains(fixture) == false)
            continue;
        FXScene::Own &own = layer.own[fixture];
        if (changes.contains("mode")) own.mode = bounded.mode;
        if (changes.contains("figure")) own.figure = bounded.figure;
        if (changes.contains("swing")) own.swing = bounded.swing;
        if (changes.contains("swingTilt")) own.swingTilt = bounded.swingTilt;
        if (changes.contains("phase")) own.phase = bounded.phase;
        if (changes.contains("rotate")) own.rotate = bounded.rotate;
        if (changes.contains("panInvert")) own.panInvert = bounded.panInvert;
        if (changes.contains("tiltInvert")) own.tiltInvert = bounded.tiltInvert;
        if (changes.contains("reverse")) own.reverse = bounded.reverse;
    }
    layer.own = layer.tidyOwn();
}

void FxTools::clearOwn(FXScene::Layer &layer, const QStringList &keys, const QList<quint32> &fixtures)
{
    for (auto it = layer.own.begin(); it != layer.own.end(); ++it)
    {
        if (fixtures.isEmpty() == false && fixtures.contains(it.key()) == false)
            continue;
        FXScene::Own &own = it.value();
        if (keys.contains("mode")) own.mode.reset();
        if (keys.contains("figure")) own.figure.reset();
        if (keys.contains("swing")) own.swing.reset();
        if (keys.contains("swingTilt")) own.swingTilt.reset();
        if (keys.contains("phase")) own.phase.reset();
        if (keys.contains("rotate")) own.rotate.reset();
        if (keys.contains("panInvert")) own.panInvert.reset();
        if (keys.contains("tiltInvert")) own.tiltInvert.reset();
        if (keys.contains("reverse")) own.reverse.reset();
        if (keys.contains("shift")) own.shift.reset();
        if (keys.contains("delay")) own.delay.reset();
    }
    layer.own = layer.tidyOwn();
}

QStringList FxTools::offsetKeys()
{
    return { "shift", "delay" };
}

void FxTools::setOffset(FXScene::Layer &layer, const QList<quint32> &fixtures, const QString &key, int value)
{
    const bool shift = key == QLatin1String("shift");
    if (shift == false && key != QLatin1String("delay"))
        return;
    // up to 4 cycles behind, up to a minute later
    value = shift ? qBound(0, value, 1440) : qBound(0, value, 60000);
    for (quint32 fixture : fixtures)
    {
        FXScene::Own &own = layer.own[fixture];
        if (shift)
            own.shift = value;
        else
            own.delay = value;
    }
    layer.own = layer.tidyOwn();
}

void FxTools::fanOffset(FXScene::Layer &layer, const QList<quint32> &inEffectOrder, const QString &key, int last)
{
    // linear, from 0 on the first; one alone gets the value of the last
    const int n = int(inEffectOrder.count());
    for (int i = 0; i < n; i++)
        setOffset(layer, { inEffectOrder.at(i) }, key, n == 1 ? last : qRound(qreal(last) * i / (n - 1)));
}

int FxTools::ownShapeCount(const FXScene::Layer &layer)
{
    const QMap<quint32, FXScene::Own> tidy = layer.tidyOwn();
    int count = 0;
    for (const FXScene::Own &own : tidy)
        count += own.hasShape() ? 1 : 0;
    return count;
}

int FxTools::offsetCount(const FXScene::Layer &layer)
{
    const QMap<quint32, FXScene::Own> tidy = layer.tidyOwn();
    int count = 0;
    for (const FXScene::Own &own : tidy)
        count += own.hasOffset() ? 1 : 0;
    return count;
}

QVariantList FxTools::fixtureRows(const Doc *doc, const FXScene::Layer &layer, const QList<quint32> &sceneFixtures,
                                  const QList<quint32> &selected)
{
    // the sets as FXScene::positionOf finds them
    const QList<quint32> order = FXScene::orderedFixtures(layer, sceneFixtures);
    int sets = 1;
    const QList<int> setOf = FXScene::groupSets(doc, order, layer.grouping, layer.groupSize, &sets);
    const QList<bool> twins = FXScene::mirrorTwins(layer, order);
    const int step = layer.step < 0 ? sets : layer.step;
    const int wave = layer.wave < 0 ? sets : layer.wave;

    QVariantList rows;
    for (int i = 0; i < order.count(); i++)
    {
        const quint32 fid = order.at(i);
        const FXScene::Shape shape = layer.shapeOf(fid);
        // a mirror: the place of its twin; a mode that travels back: from the last set
        int set = i < twins.count() && twins.at(i) ? setOf.at(order.count() - 1 - i) : setOf.at(i);
        if (FXScene::modeReversed(shape.mode) && layer.isPanTilt() == false)
            set = sets - 1 - set;
        const Fixture *fxi = doc != nullptr ? doc->fixture(fid) : nullptr;
        rows.append(QVariantMap({
            { "id", fid },
            { "name", fxi != nullptr ? fxi->name() : QString() },
            { "selected", selected.contains(fid) },
            { "place", step == 0 && wave > 0 ? qRound(360.0 * (set % wave) / wave) : -1 },
            { "step", step > 0 ? set % step + 1 : 0 },
            { "shift", shape.shift },
            { "delay", shape.delay },
            // DMX Total Control: a dimmer FX dims its colour
            { "virtual", layer.param == QLatin1String("dimmer") && FXScene::virtualDimmerChannels(fxi).isEmpty() == false }
        }));
    }
    return rows;
}

QVariantMap FxTools::virtualDimmer(const Doc *doc, const FXScene::Layer &layer, const QList<quint32> &fixtures,
                                   const QHash<QPair<quint32, quint32>, uchar> &values)
{
    // (only a dimmer FX has one: the others don't look at the fixtures)
    if (layer.param != QLatin1String("dimmer"))
        return { { "virtualCount", 0 }, { "virtualBase", layer.virtualBase },
                 { "virtualLevel", layer.virtualBase >= 0 ? layer.virtualBase : 255 } };

    const QList<quint32> order = FXScene::orderedFixtures(layer, fixtures);
    int count = 0;
    if (doc != nullptr)
    {
        for (quint32 fid : order)
            count += FXScene::virtualDimmerChannels(doc->fixture(fid)).isEmpty() ? 0 : 1;
    }
    const int level = layer.virtualBase >= 0 ? layer.virtualBase : FXScene::autoVirtualBase(doc, order, values);
    return { { "virtualCount", count }, { "virtualBase", layer.virtualBase }, { "virtualLevel", level } };
}

QVariantMap FxTools::ownToMap(const FXScene::Own &own)
{
    QVariantMap map;
    if (own.mode) map.insert("mode", *own.mode);
    if (own.figure) map.insert("figure", *own.figure);
    if (own.swing) map.insert("swing", *own.swing);
    if (own.swingTilt) map.insert("swingTilt", *own.swingTilt);
    if (own.phase) map.insert("phase", *own.phase);
    if (own.rotate) map.insert("rotate", *own.rotate);
    if (own.panInvert) map.insert("panInvert", *own.panInvert);
    if (own.tiltInvert) map.insert("tiltInvert", *own.tiltInvert);
    if (own.reverse) map.insert("reverse", *own.reverse);
    if (own.shift) map.insert("shift", *own.shift);
    if (own.delay) map.insert("delay", *own.delay);
    return map;
}

FXScene::Own FxTools::ownFromMap(const QVariantMap &map)
{
    // bounded as a layer takes them; a value that isn't a number is not there
    auto number = [&map](const char *key, int low, int high) -> std::optional<int> {
        bool ok = false;
        const int value = map.value(key).toInt(&ok);
        if (map.contains(key) == false || ok == false)
            return std::nullopt;
        return qBound(low, value, high);
    };
    auto degrees = [&number](const char *key) -> std::optional<int> {
        const std::optional<int> value = number(key, INT_MIN, INT_MAX);
        if (value.has_value() == false)
            return std::nullopt;
        return ((*value % 360) + 360) % 360;
    };
    auto flag = [&map](const char *key) -> std::optional<bool> {
        if (map.contains(key) == false)
            return std::nullopt;
        return map.value(key).toBool();
    };
    FXScene::Own own;
    own.mode = number("mode", 1, FXScene::ModeCount);
    own.figure = number("figure", 0, int(FXScene::FigureCount) - 1);
    own.swing = number("swing", 0, 250);
    own.swingTilt = number("swingTilt", 0, 250);
    own.phase = degrees("phase");
    own.rotate = degrees("rotate");
    own.panInvert = flag("panInvert");
    own.tiltInvert = flag("tiltInvert");
    own.reverse = flag("reverse");
    own.shift = number("shift", 0, 1440);
    own.delay = number("delay", 0, 60000);
    return own;
}

bool FxTools::interpolates(const QString &param)
{
    // (an unknown channel spreads too)
    static const QStringList discrete = { "colorwheel", "colormacro", "gobo", "prism", "shutter" };
    return discrete.contains(param) == false;
}

int FxTools::nearestPlace(int i, int n, int m)
{
    if (n == m)
        return qBound(0, i, qMax(0, m - 1));
    if (n <= 1 || m <= 1)
        return 0;
    return qBound(0, qRound(qreal(i) * (m - 1) / (n - 1)), m - 1);
}

FxTools::FixtureLook FxTools::lookAt(const QList<FixtureLook> &looks, int i, int n)
{
    const int m = int(looks.count());
    if (m == 0)
        return FixtureLook();

    // the shape and the slots of its nearest place: a mirror stays a mirror
    FixtureLook look = looks.at(nearestPlace(i, n, m));
    // the numbers in between the two places around it: a fan stays a fan
    qreal t = 0;
    if (n == m)
        t = nearestPlace(i, n, m);
    else if (n > 1 && m > 1)
        t = qBound(0.0, qreal(i) * (m - 1) / (n - 1), qreal(m - 1));
    const int low = int(std::floor(t));
    const int high = qMin(m - 1, int(std::ceil(t)));
    const qreal f = t - low;
    const FixtureLook &a = looks.at(low);
    const FixtureLook &b = looks.at(high);
    auto between = [f](int from, int to) { return qRound(from + (to - from) * f); };
    for (auto it = a.bases.cbegin(); it != a.bases.cend(); ++it)
    {
        if (interpolates(it.key()) && b.bases.contains(it.key()))
            look.bases.insert(it.key(), between(it.value(), b.bases.value(it.key())));
    }
    const int shift = between(a.own.shift.value_or(0), b.own.shift.value_or(0));
    const int delay = between(a.own.delay.value_or(0), b.own.delay.value_or(0));
    look.own.shift = shift > 0 ? std::optional<int>(shift) : std::nullopt;
    look.own.delay = delay > 0 ? std::optional<int>(delay) : std::nullopt;
    return look;
}

QVariantList FxTools::modeCurve(int mode, int samples)
{
    QVariantList list;
    samples = qBound(2, samples, 1000);
    for (int i = 0; i < samples; i++)
        list.append(FXScene::modeValue(mode, qreal(i) / qreal(samples)));
    return list;
}

QVariantList FxTools::figurePath(int figure, int samples)
{
    QVariantList list;
    samples = qBound(2, samples, 1000);
    for (int i = 0; i <= samples; i++)
    {
        QPointF p = FXScene::figurePoint(figure, qreal(i) / qreal(samples));
        list.append(QVariantMap({ { "x", p.x() }, { "y", p.y() } }));
    }
    return list;
}

/** The wave per X for an old effect spread over $degrees between the first and the last of $count */
static int waveOfSpread(int degrees, int count)
{
    if (degrees == 0)
        return 0;
    if (degrees < 0 || degrees == 360)
        return -1;
    return qMax(1, int(std::round(qreal(count) * 360.0 / qreal(degrees))));
}

/** The speed of $layer from the cycle of an old effect: in ms, or in beats
 *  x 1000 when it follows the BPM (then the layer follows it too) */
static void setLayerSpeed(FXScene::Layer &layer, const Function *function)
{
    const qreal duration = qreal(function->duration());
    if (function->tempoType() == Function::Beats && duration > 0)
    {
        // cycles per beat
        layer.syncBpm = true;
        layer.multiplier = qBound(0.01, 1000.0 / duration, 64.0);
    }
    else
    {
        layer.bpm = duration > 0 ? 60000.0 / duration : 30.0;
    }
}

static void copyCommon(const Function *from, FXScene *to)
{
    to->setName(from->name());
    // fades in beats x 1000 when it follows the BPM (first: setTempoType
    // converts the times already there)
    to->setTempoType(from->tempoType());
    to->setFadeInSpeed(from->fadeInSpeed());
    to->setFadeOutSpeed(from->fadeOutSpeed());
    to->setPriority(from->priority());
    to->setPath(from->path(true));
    to->setVisible(from->isVisible());
}

FXScene *FxTools::convert(Doc *doc, const Function *function)
{
    if (const EFX *efx = qobject_cast<const EFX *>(function))
    {
        FXScene *fx = new FXScene(doc);
        copyCommon(efx, fx);

        QList<quint32> fixtures;
        for (EFXFixture *ef : efx->fixtures())
            if (fixtures.contains(ef->head().fxi) == false)
                fixtures.append(ef->head().fxi);
        fx->setFixtures(fixtures);

        FXScene::Layer layer = defaultLayer("pantilt");
        switch (efx->algorithm())
        {
            case EFX::Eight: layer.figure = FXScene::Eight; break;
            case EFX::Line: layer.figure = FXScene::PanLine; break;
            case EFX::Line2: layer.figure = FXScene::Diagonal; break;
            case EFX::Diamond: layer.figure = FXScene::Diamond; break;
            case EFX::Square: case EFX::SquareChoppy: case EFX::SquareTrue: layer.figure = FXScene::Square; break;
            case EFX::Leaf: layer.figure = FXScene::Leaf; break;
            case EFX::Lissajous: layer.figure = FXScene::Lissajous; break;
            default: layer.figure = FXScene::Circle; break;
        }
        // width and height are radiuses in 0-255
        layer.swing = qBound(0, int(std::round(efx->width() * 200.0 / 255.0)), 250);
        layer.swingTilt = qBound(0, int(std::round(efx->height() * 200.0 / 255.0)), 250);
        layer.rotate = efx->rotation() % 360;
        layer.reverse = efx->direction() == Function::Backward;
        setLayerSpeed(layer, efx);
        if (efx->propagationMode() != EFX::Parallel)
            layer.wave = waveOfSpread(efx->phaseSpread(), fixtures.count());
        if (efx->isSymmetric())
        {
            layer.grouping = FXScene::Mirror;
            layer.groupSize = 2;
        }
        // DMX Total Control: its start offset is the phase, and the one of each
        // fixture its shift: the effect puts them ahead, a shift goes behind
        layer.phase = efx->startOffset() % 360;
        QList<quint32> offsetDone;
        for (EFXFixture *ef : efx->fixtures())
        {
            const quint32 fid = ef->head().fxi;
            if (offsetDone.contains(fid))
                continue;
            offsetDone.append(fid);
            if (ef->startOffset() % 360 != 0)
                FxTools::setOffset(layer, { fid }, "shift", (360 - ef->startOffset() % 360) % 360);
        }
        QList<FXScene::Layer> layers = { layer };

        // "Control dimmer": the heads lit on the stroke, dark on the way back,
        // each with its movement (a dimmer layer at the same speed, linked)
        if (efx->dimmerControlEnabled())
        {
            FXScene::Layer dimmer = defaultLayer("dimmer");
            dimmer.mode = 9;
            dimmer.swing = 200;
            dimmer.syncBpm = layer.syncBpm;
            dimmer.multiplier = layer.multiplier;
            dimmer.bpm = layer.bpm;
            dimmer.wave = layer.wave;
            dimmer.grouping = layer.grouping;
            dimmer.groupSize = layer.groupSize;
            dimmer.reverse = layer.reverse;
            // (DMX Total Control: each dimmer with the start offset of its fixture)
            dimmer.own = layer.own;
            dimmer.linked = true;
            layers.first().linked = true;
            layers.append(dimmer);
            for (quint32 fid : fixtures)
            {
                quint32 fine = QLCChannel::invalid();
                quint32 channel = FXScene::channelFor(doc->fixture(fid), "dimmer", &fine);
                if (channel != QLCChannel::invalid())
                    fx->setValue(fid, channel, 0);
            }
        }

        // the centre of the movement is its base, unless it moves around what's below
        if (efx->isRelative() == false)
        {
            for (quint32 fid : fixtures)
            {
                Fixture *fxi = doc->fixture(fid);
                quint32 fine = QLCChannel::invalid();
                quint32 pan = FXScene::channelFor(fxi, "pan", &fine);
                if (pan != QLCChannel::invalid())
                    fx->setValue(fid, pan, uchar(qBound(0, efx->xOffset(), 255)));
                quint32 tilt = FXScene::channelFor(fxi, "tilt", &fine);
                if (tilt != QLCChannel::invalid())
                    fx->setValue(fid, tilt, uchar(qBound(0, efx->yOffset(), 255)));
            }
        }
        fx->setLayers(layers);
        return fx;
    }

    if (const CurveFX *curve = qobject_cast<const CurveFX *>(function))
    {
        FXScene *fx = new FXScene(doc);
        copyCommon(curve, fx);
        const QList<quint32> fixtures = curve->fixtures();
        fx->setFixtures(fixtures);

        const int low = curve->low(), high = curve->high();
        const int span = qMax(0, high - low);
        int mode = 1;
        bool fromLow = false;
        bool random = false;
        switch (curve->waveform())
        {
            case CurveFX::Triangle: mode = 19; break;
            case CurveFX::RampUp: mode = 16; fromLow = true; break;
            case CurveFX::RampDown: mode = 15; fromLow = true; break;
            case CurveFX::Square: mode = 7; break;
            case CurveFX::Strobe: mode = 9; fromLow = true; break;
            case CurveFX::RandomStrobe: mode = 9; fromLow = true; random = true; break;
            case CurveFX::Random: mode = 9; fromLow = true; random = true; break;
            default: mode = 1; break;
        }

        // a layer for each parameter its channels are
        QStringList params;
        for (quint32 fid : fixtures)
        {
            Fixture *fxi = doc->fixture(fid);
            if (fxi == nullptr)
                continue;
            for (quint32 c = 0; c < fxi->channels(); c++)
            {
                const QLCChannel *ch = fxi->channel(c);
                if (ch == nullptr || curve->features().contains(ch->name().toLower()) == false)
                    continue;
                QString param = FXScene::paramOf(ch);
                if (param.isEmpty())
                    continue;
                if (params.contains(param) == false)
                    params.append(param);
                if (curve->isRelative() == false)
                    fx->setValue(fid, c, uchar(fromLow ? low : (low + high) / 2));
            }
        }

        QList<FXScene::Layer> layers;
        for (const QString &param : std::as_const(params))
        {
            FXScene::Layer layer = defaultLayer(param);
            layer.mode = mode;
            // symmetric waves swing around the middle, the others go up from the low value
            layer.swing = qBound(0, int(std::round(span * (fromLow ? 200.0 : 100.0) / 255.0)), 250);
            setLayerSpeed(layer, curve);
            layer.phase = ((curve->phaseOffset() % 360) + 360) % 360;
            layer.wave = waveOfSpread(curve->spread(), fixtures.count());
            if (random)
            {
                layer.step = -1;
                layer.wave = 0;
                layer.order = FXScene::Shuffled;
            }
            layer.linked = params.count() > 1;
            layers.append(layer);
        }
        fx->setLayers(layers);
        return fx;
    }

    return nullptr;
}

/*********************************************************************
 * Macros
 *********************************************************************/

/** DMX Total Control: what each fixture of a layer did, in a macro file:
 *  [{ bases: { param: 0-255 }, own: { ...FxTools::ownToMap } }]. ("fixtures"
 *  would be the IDs of a layer) */
static QJsonArray looksToJson(const QList<FxTools::FixtureLook> &looks)
{
    QJsonArray array;
    for (const FxTools::FixtureLook &look : looks)
    {
        QJsonObject bases;
        for (auto it = look.bases.cbegin(); it != look.bases.cend(); ++it)
            bases.insert(it.key(), it.value());
        array.append(QJsonObject({ { "bases", bases }, { "own", QJsonObject::fromVariantMap(FxTools::ownToMap(look.own)) } }));
    }
    return array;
}

static QList<FxTools::FixtureLook> looksFromJson(const QJsonArray &array)
{
    QList<FxTools::FixtureLook> looks;
    for (const QJsonValue &value : array)
    {
        const QJsonObject obj = value.toObject();
        FxTools::FixtureLook look;
        const QJsonObject bases = obj.value("bases").toObject();
        for (auto it = bases.begin(); it != bases.end(); ++it)
            look.bases.insert(it.key(), qBound(0, it.value().toInt(), 255));
        look.own = FxTools::ownFromMap(obj.value("own").toObject().toVariantMap());
        looks.append(look);
    }
    return looks;
}

FxMacros::FxMacros(Programmer *programmer, QObject *parent)
    : QObject(parent)
    , m_programmer(programmer)
{
    buildFactory();
    m_path = QLCFile::userDirectory(QString(USERQLCPLUSDIR), QString(USERQLCPLUSDIR), QStringList())
                 .absoluteFilePath("FX Macros.json");
    load();
}

void FxMacros::buildFactory()
{
    auto pantilt = [](int figure, int swing, int swingTilt, qreal bpm) {
        FXScene::Layer l = FxTools::defaultLayer("pantilt");
        l.figure = figure;
        l.swing = swing;
        l.swingTilt = swingTilt;
        l.bpm = bpm;
        return l;
    };
    auto wave = [](const QString &param, int mode, int swing, qreal bpm) {
        FXScene::Layer l = FxTools::defaultLayer(param);
        l.mode = mode;
        l.swing = swing;
        l.bpm = bpm;
        return l;
    };
    auto add = [this](const QString &id, const QString &name, const QString &category,
                      const QList<FXScene::Layer> &layers, const QMap<QString, int> &bases = {}) {
        Macro m;
        m.id = id;
        m.name = name;
        m.category = category;
        m.factory = true;
        m.layers = layers;
        m.bases = bases;
        m_factory.append(m);
    };

    // position
    add("circle", tr("Circle"), "position", { pantilt(FXScene::Circle, 20, 20, 30) });
    FXScene::Layer circleWave = pantilt(FXScene::Circle, 20, 20, 30);
    circleWave.wave = -1;
    add("circle-wave", tr("Circle wave"), "position", { circleWave });
    add("eight", tr("Eight"), "position", { pantilt(FXScene::Eight, 25, 15, 24) });
    FXScene::Layer ballyhoo = pantilt(FXScene::Ballyhoo, 35, 30, 18);
    ballyhoo.wave = -1;
    add("ballyhoo", tr("Ballyhoo"), "position", { ballyhoo });
    FXScene::Layer sweep = pantilt(FXScene::PanLine, 40, 0, 15);
    sweep.wave = -1;
    add("pan-sweep", tr("Pan sweep"), "position", { sweep });
    FXScene::Layer cancan = pantilt(FXScene::TiltLine, 0, 25, 40);
    cancan.wave = 2;
    add("can-can", tr("Can-can"), "position", { cancan });
    FXScene::Layer mirror = pantilt(FXScene::PanLine, 30, 0, 20);
    mirror.grouping = FXScene::Mirror;
    mirror.groupSize = 2;
    mirror.panInvert = true;
    add("pan-mirror", tr("Pan in mirror"), "position", { mirror });
    add("square", tr("Square"), "position", { pantilt(FXScene::Square, 20, 20, 20) });
    add("triangle", tr("Triangle"), "position", { pantilt(FXScene::Triangle, 20, 20, 20) });
    add("leaf", tr("Leaf"), "position", { pantilt(FXScene::Leaf, 25, 25, 15) });

    // intensity
    FXScene::Layer dimmerWave = wave("dimmer", 3, 200, 40);
    dimmerWave.wave = -1;
    add("dimmer-wave", tr("Dimmer wave"), "intensity", { dimmerWave }, { { "dimmer", 0 } });
    FXScene::Layer chase = wave("dimmer", 9, 200, 120);
    chase.step = -1;
    add("chase", tr("Chase"), "intensity", { chase }, { { "dimmer", 0 } });
    FXScene::Layer random = chase;
    random.order = FXScene::Shuffled;
    random.bpm = 90;
    add("random", tr("Random"), "intensity", { random }, { { "dimmer", 0 } });
    add("pulse", tr("Pulse"), "intensity", { wave("dimmer", 15, 200, 60) }, { { "dimmer", 0 } });
    add("breathe", tr("Breathe"), "intensity", { wave("dimmer", 3, 200, 12) }, { { "dimmer", 0 } });
    FXScene::Layer strobe = wave("dimmer", 9, 200, 300);
    strobe.multiplier = 2;
    add("strobe", tr("Strobe"), "intensity", { strobe }, { { "dimmer", 0 } });
    FXScene::Layer inOut = wave("dimmer", 3, 200, 40);
    inOut.wave = -1;
    inOut.grouping = FXScene::Mirror;
    inOut.groupSize = 2;
    add("dimmer-mirror", tr("Outside in"), "intensity", { inOut }, { { "dimmer", 0 } });

    // colour
    QList<FXScene::Layer> rainbow;
    const QStringList rgb = { "red", "green", "blue" };
    for (int i = 0; i < rgb.count(); i++)
    {
        FXScene::Layer l = wave(rgb.at(i), 3, 200, 20);
        l.phase = i * 120;
        l.wave = -1;
        l.linked = true;
        rainbow.append(l);
    }
    add("rainbow", tr("Rainbow"), "colour", rainbow, { { "red", 0 }, { "green", 0 }, { "blue", 0 } });
    FXScene::Layer red = wave("red", 9, 200, 60);
    red.linked = true;
    FXScene::Layer blue = wave("blue", 9, 200, 60);
    blue.phase = 180;
    blue.linked = true;
    add("red-blue", tr("Red / blue"), "colour", { red, blue }, { { "red", 0 }, { "green", 0 }, { "blue", 0 } });
    FXScene::Layer fireRed = wave("red", 1, 20, 90);
    fireRed.order = FXScene::Shuffled;
    fireRed.wave = -1;
    FXScene::Layer fireGreen = wave("green", 1, 60, 70);
    fireGreen.order = FXScene::Shuffled;
    fireGreen.wave = -1;
    add("fire", tr("Fire"), "colour", { fireRed, fireGreen }, { { "red", 230 }, { "green", 40 }, { "blue", 0 } });
    FXScene::Layer cyan = wave("cyan", 13, 200, 60);
    cyan.linked = true;
    FXScene::Layer magenta = wave("magenta", 7, 200, 60);
    magenta.linked = true;
    add("cyan-magenta", tr("Cyan / magenta"), "colour", { cyan, magenta }, { { "cyan", 128 }, { "magenta", 128 } });

    // beam
    add("zoom", tr("Zoom"), "beam", { wave("zoom", 1, 80, 20) });
    add("iris", tr("Iris"), "beam", { wave("iris", 3, 150, 30) });
    add("focus", tr("Focus"), "beam", { wave("focus", 1, 60, 15) });
}

QVariantList FxMacros::macros() const
{
    QVariantList list;
    for (const QList<Macro> *group : { &m_factory, &m_user })
    {
        for (const Macro &m : *group)
        {
            const FXScene::Layer &first = m.layers.isEmpty() ? FXScene::Layer() : m.layers.first();
            // DMX Total Control: saved fixture by fixture, on how many
            int fixtureCount = 0;
            for (const QList<FxTools::FixtureLook> &looks : m.looks)
                fixtureCount = qMax(fixtureCount, int(looks.count()));
            list.append(QVariantMap({
                { "perFixture", fixtureCount > 0 },
                { "fixtureCount", fixtureCount },
                { "id", m.id },
                { "name", m.name },
                { "category", m.category },
                { "factory", m.factory },
                { "panTilt", first.isPanTilt() },
                { "figure", first.figure },
                { "mode", first.mode },
                { "params", [&m]() { QStringList p; for (const FXScene::Layer &l : m.layers) p.append(l.param); return p; }() }
            }));
        }
    }
    return list;
}

const FxMacros::Macro *FxMacros::find(const QString &id) const
{
    for (const QList<Macro> *group : { &m_factory, &m_user })
        for (const Macro &m : *group)
            if (m.id == id)
                return &m;
    return nullptr;
}

bool FxMacros::apply(const QString &id)
{
    const Macro *m = find(id);
    if (m == nullptr || m_programmer == nullptr)
        return false;
    return m_programmer->applyFxMacro(m->layers, m->bases, m->looks) > 0;
}

QString FxMacros::save(const QString &name)
{
    if (m_programmer == nullptr)
        return QString();
    QList<FXScene::Layer> layers = m_programmer->fxLayerList();
    if (layers.isEmpty())
        return QString();

    Macro m;
    m.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m.name = name.trimmed().isEmpty() ? tr("Macro %1").arg(m_user.count() + 1) : name.trimmed();
    m.category = "user";
    // the macro is global: no fixtures. DMX Total Control: what each of them
    // did goes by its place in the FX (its bases, its own way, its shift and
    // delay); when they all did the same, nothing: it is for any selection
    for (FXScene::Layer layer : layers)
    {
        bool known = false;
        for (const FXScene::Layer &l : std::as_const(m.layers))
            known = known || l.param == layer.param;
        if (known)
            continue;
        QList<FxTools::FixtureLook> looks = m_programmer->fxFixtureLooks(layer);
        // (the bases of the first fixture: what an older version gives them all)
        if (looks.isEmpty() == false)
            m.bases.insert(looks.constFirst().bases);
        bool same = true;
        for (const FxTools::FixtureLook &look : std::as_const(looks))
            same = same && look.own.isEmpty() && look.bases == looks.constFirst().bases;
        if (same)
            looks.clear();
        layer.fixtures.clear();
        layer.own.clear();
        m.layers.append(layer);
        m.looks.append(looks);
    }
    m_user.append(m);
    write();
    emit macrosChanged();
    return m.id;
}

void FxMacros::rename(const QString &id, const QString &name)
{
    for (Macro &m : m_user)
    {
        if (m.id == id && name.trimmed().isEmpty() == false)
        {
            m.name = name.trimmed();
            write();
            emit macrosChanged();
            return;
        }
    }
}

void FxMacros::remove(const QString &id)
{
    for (int i = 0; i < m_user.count(); i++)
    {
        if (m_user.at(i).id == id)
        {
            m_user.removeAt(i);
            write();
            emit macrosChanged();
            return;
        }
    }
}

QString FxMacros::filePath() const
{
    return m_path;
}

void FxMacros::setFilePath(const QString &path)
{
    m_path = path;
    m_user.clear();
    load();
    emit macrosChanged();
}

void FxMacros::load()
{
    QFile file(m_path);
    if (file.open(QIODevice::ReadOnly) == false)
        return;

    QJsonArray array = QJsonDocument::fromJson(file.readAll()).object().value("macros").toArray();
    for (const QJsonValue &value : array)
    {
        QJsonObject obj = value.toObject();
        Macro m;
        m.id = obj.value("id").toString();
        m.name = obj.value("name").toString();
        m.category = "user";
        for (const QJsonValue &l : obj.value("layers").toArray())
        {
            FXScene::Layer layer = FxTools::layerFromMap(l.toObject().toVariantMap());
            if (layer.param.isEmpty())
                continue;
            m.layers.append(layer);
            // DMX Total Control: what each fixture did (older files: nothing)
            m.looks.append(looksFromJson(l.toObject().value("looks").toArray()));
        }
        QJsonObject bases = obj.value("bases").toObject();
        for (auto it = bases.begin(); it != bases.end(); ++it)
            m.bases.insert(it.key(), qBound(0, it.value().toInt(), 255));
        if (m.id.isEmpty() == false && m.layers.isEmpty() == false)
            m_user.append(m);
    }
}

bool FxMacros::write() const
{
    QJsonArray array;
    for (const Macro &m : m_user)
    {
        QJsonArray layers;
        for (int i = 0; i < m.layers.count(); i++)
        {
            const FXScene::Layer &l = m.layers.at(i);
            QVariantMap map = FxTools::layerToMap(l);
            map.remove("fixtures");
            map.remove("panTilt");
            // DMX Total Control: an auto virtual dimmer is the default
            if (l.param != QLatin1String("dimmer") || l.virtualBase < 0)
                map.remove("virtualBase");
            QJsonObject layer = QJsonObject::fromVariantMap(map);
            // DMX Total Control: what each fixture did, when they didn't all do the same
            const QList<FxTools::FixtureLook> looks = m.looks.value(i);
            if (looks.isEmpty() == false)
                layer.insert("looks", looksToJson(looks));
            layers.append(layer);
        }
        QJsonObject bases;
        for (auto it = m.bases.cbegin(); it != m.bases.cend(); ++it)
            bases.insert(it.key(), it.value());
        array.append(QJsonObject({ { "id", m.id }, { "name", m.name }, { "layers", layers }, { "bases", bases } }));
    }

    QSaveFile file(m_path);
    if (file.open(QIODevice::WriteOnly) == false)
    {
        qWarning() << "Unable to write the FX macros to" << m_path;
        return false;
    }
    file.write(QJsonDocument(QJsonObject({ { "macros", array } })).toJson());
    return file.commit();
}
