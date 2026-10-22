/*
  DMX Total Control
  programmer.cpp

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

#include <QDebug>
#include <QSet>
#include <QUrl>
#include <QFileInfo>
#include <QRegularExpression>
#include <QVector3D>
#include <algorithm>

#include "programmer.h"
#include "zonemanager.h"
#include "zone.h"
#include "palettemanager.h"
#include "patchmanager.h"
#include "channelmodifier.h"
#include "aim.h"

#include "genericdmxsource.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "mastertimer.h"
#include "monitorproperties.h"
#include "inputoutputmap.h"
#include "fixturegroup.h"
#include "efxfixture.h"
#include "rgbmatrix.h"
#include "sequence.h"
#include "universe.h"
#include "cuelist.h"
#include "curvefx.h"
#include "fxtools.h"
#include "efx.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"

Programmer::Programmer(Doc *doc, ZoneManager *zones, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_zones(zones)
    , m_blind(false)
    , m_releaseMode("channels")
    , m_fx(nullptr)
    , m_palettes(nullptr)
    , m_highlight(false)
    , m_stepIndex(-1)
    , m_inStep(false)
{
    m_source = new GenericDMXSource(m_doc);
    m_source->setOutputEnabled(true);
    m_source->setPriority(Function::maxPriority());

    // highlight goes over everything, the programmer included
    m_highlightSource = new GenericDMXSource(m_doc);
    m_highlightSource->setOutputEnabled(true);
    m_highlightSource->setPriority(-1);

    // the FX of the programmer: a scene of its own, not in the show
    m_fx = new FXScene(m_doc);
    m_fx->setName(tr("Programmer FX"));
    m_fx->setPriority(Function::maxPriority());

    // Follow aims the heads of a selection
    connect(this, &Programmer::selectionChanged, this, &Programmer::dropAim);
    // (and no more once their values change another way)
    connect(this, &Programmer::valuesChanged, this, [this]() {
        if (m_aim.isEmpty() == false && aimHolds() == false)
            dropAim();
    });

    // scenes that start take back the channels they play (live edit)
    connect(m_doc->masterTimer(), SIGNAL(functionStarted(quint32)), this, SLOT(slotFunctionStarted(quint32)));
    // and so does each GO of a running cuelist, on the channels it changes
    connect(m_doc->masterTimer(), SIGNAL(cueStarted(quint32,quint32)), this, SLOT(slotCueStarted(quint32,quint32)));

    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotFixtureRemoved(quint32)));
    // a fixture moved or changed mode: its values follow it
    connect(m_doc, SIGNAL(fixtureChanged(quint32)), this, SLOT(slotFixtureChanged(quint32)));
    connect(m_doc, SIGNAL(cleared()), this, SLOT(slotDocCleared()));
    connect(this, &Programmer::selectionChanged, this, &Programmer::fxParamsChanged);
    connect(this, &Programmer::fxChanged, this, &Programmer::fxParamsChanged);
    // the programmer values on the channels of the FX are their base
    connect(this, &Programmer::valuesChanged, this, &Programmer::syncFx);
    connect(this, &Programmer::selectionChanged, this, &Programmer::slotSelectionChanged);
}

Programmer::~Programmer()
{
    // the timer lets go of the FX also when it waits for its first tick
    // (started, or stopped and started again by a stop all): deleted while
    // queued, the next tick would run a deleted function
    m_doc->masterTimer()->releaseFunction(m_fx);
    delete m_fx;
    delete m_source;
    delete m_highlightSource;
}

/*********************************************************************
 * Selection
 *********************************************************************/

QVariantList Programmer::selection() const
{
    QVariantList list;
    for (quint32 id : m_selection)
        list.append(id);
    return list;
}

bool Programmer::isSelected(quint32 fixtureId) const
{
    return m_selection.contains(fixtureId);
}

void Programmer::select(quint32 fixtureId, bool additive)
{
    if (m_doc->fixture(fixtureId) == nullptr)
        return;

    if (additive == false)
        m_selection.clear();
    if (m_selection.contains(fixtureId) == false)
        m_selection.append(fixtureId);

    emit selectionChanged();
    emit channelsChanged();
}

void Programmer::toggle(quint32 fixtureId)
{
    if (m_selection.contains(fixtureId))
        m_selection.removeAll(fixtureId);
    else if (m_doc->fixture(fixtureId) != nullptr)
        m_selection.append(fixtureId);
    else
        return;

    emit selectionChanged();
    emit channelsChanged();
}

void Programmer::selectAll()
{
    m_selection.clear();
    for (Fixture *fxi : m_doc->fixtures())
        m_selection.append(fxi->id());

    emit selectionChanged();
    emit channelsChanged();
}

void Programmer::clearSelection()
{
    if (m_selection.isEmpty())
        return;

    m_selection.clear();
    emit selectionChanged();
    emit channelsChanged();
}

/*********************************************************************
 * Values
 *********************************************************************/

QString Programmer::channelKey(quint32 fixtureId, quint32 channel) const
{
    Fixture *fxi = m_doc->fixture(fixtureId);
    if (fxi == nullptr)
        return QString();
    return keysOf(fxi).value(int(channel));
}

QString Programmer::attributeKey(quint32 fixtureId, quint32 channel) const
{
    return channelKey(fixtureId, channel);
}

QStringList Programmer::keysOf(const Fixture *fxi) const
{
    auto cached = m_keyCache.constFind(fxi->id());
    if (cached != m_keyCache.constEnd() && cached->first == fxi->fixtureMode() &&
        cached->second.count() == int(fxi->channels()))
        return cached->second;

    // by function, so "Red", "R" and "Rojo" of two fixtures are one attribute
    QStringList keys;
    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        QString key = FXScene::attributeOf(ch);
        if (ch != nullptr && ch->controlByte() == QLCChannel::LSB)
            key += QLatin1String(" fine");
        // each head of a bar keeps its own channels: "red 1", "red 2"...
        int head = FXScene::headOf(fxi, c);
        if (head >= 0)
            key += QLatin1Char(' ') + QString::number(head + 1);
        // two channels that do the same (two gobo wheels): "gobo", "gobo#2"
        if (keys.contains(key))
        {
            int n = 2;
            while (keys.contains(key + QLatin1Char('#') + QString::number(n)))
                n++;
            key += QLatin1Char('#') + QString::number(n);
        }
        keys.append(key);
    }
    m_keyCache.insert(fxi->id(), qMakePair(static_cast<const void *>(fxi->fixtureMode()), keys));
    return keys;
}

/** The features tab of a channel */
static QString featureTab(const QLCChannel *ch)
{
    QString name = ch->name().toLower();
    switch (ch->group())
    {
        case QLCChannel::Intensity:
            return ch->colour() == QLCChannel::NoColour ? "dimmer" : "color";
        case QLCChannel::Shutter:
            return "dimmer";
        case QLCChannel::Colour:
            return "color";
        // the position is pan and tilt alone: their rotations and speeds go
        // with the others
        case QLCChannel::Pan:
        case QLCChannel::Tilt:
            return Programmer::isPositionChannel(ch) ? "position" : "other";
        case QLCChannel::Gobo:
            return "gobo";
        case QLCChannel::Prism:
            return "beam";
        case QLCChannel::Beam:
            return name.contains("focus") || name.contains("zoom") ? "focus" : "beam";
        case QLCChannel::Speed:
            return "other";
        default:
            return name.contains("focus") || name.contains("zoom") ? "focus" : "other";
    }
}

QVariantList Programmer::channels() const
{
    QVariantList list;
    QStringList keys;

    for (quint32 fxid : m_selection)
    {
        Fixture *fxi = m_doc->fixture(fxid);
        if (fxi == nullptr)
            continue;

        const QStringList fixtureKeys = keysOf(fxi);
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch == nullptr)
                continue;

            const QString key = fixtureKeys.value(int(c));
            if (keys.contains(key))
                continue;
            keys.append(key);

            auto it = m_values.constFind(qMakePair(fxid, c));
            bool active = it != m_values.constEnd();

            QVariantList caps;
            for (QLCCapability *cap : ch->capabilities())
            {
                QVariantMap capInfo;
                capInfo.insert("name", cap->name());
                capInfo.insert("min", cap->min());
                capInfo.insert("max", cap->max());
                // colour wheels and gobos show what they are
                if (cap->presetType() == QLCCapability::SingleColor || cap->presetType() == QLCCapability::DoubleColor)
                {
                    capInfo.insert("color", cap->resource(0).value<QColor>().name());
                    if (cap->presetType() == QLCCapability::DoubleColor)
                        capInfo.insert("color2", cap->resource(1).value<QColor>().name());
                }
                else if (cap->presetType() == QLCCapability::Picture)
                {
                    QString path = cap->resource(0).toString();
                    if (QFileInfo::exists(path))
                        capInfo.insert("image", QUrl::fromLocalFile(path).toString());
                }
                caps.append(capInfo);
            }

            QVariantMap info;
            info.insert("key", key);
            info.insert("name", ch->name());
            info.insert("group", QLCChannel::groupToString(ch->group()));
            info.insert("colour", ch->colour() == QLCChannel::NoColour ?
                                  QString() : QColor(QRgb(ch->colour())).name());
            info.insert("value", active ? int(it.value()) : int(ch->defaultValue()));
            info.insert("active", active);
            info.insert("capabilities", caps);
            info.insert("tab", featureTab(ch));
            info.insert("fine", ch->controlByte() == QLCChannel::LSB);
            list.append(info);
        }
    }
    return list;
}

bool Programmer::hasValues() const
{
    return m_values.isEmpty() == false;
}

int Programmer::valuesCount() const
{
    return m_values.count();
}

void Programmer::setValue(quint32 fixtureId, quint32 channel, uchar value)
{
    // what each channel of the fixture is, should it change mode
    if (m_valueKeys.contains(fixtureId) == false)
    {
        if (const Fixture *fxi = m_doc->fixture(fixtureId))
            m_valueKeys.insert(fixtureId, keysOf(fxi));
    }
    m_values.insert(qMakePair(fixtureId, channel), value);
    // set by hand: it no longer comes from a palette
    m_paletteRefs.remove(qMakePair(fixtureId, channel));
    // a channel with an FX: the value is its base, the FX sends it
    if (m_blind == false && m_fxChannels.contains(qMakePair(fixtureId, channel)) == false)
        m_source->set(fixtureId, channel, value);
}

void Programmer::notifyValues()
{
    emit valuesChanged();
    emit channelsChanged();
}

void Programmer::setChannelValue(const QString &key, int value)
{
    uchar dmx = uchar(qBound(0, value, 255));
    bool changed = false;

    for (quint32 fxid : m_selection)
    {
        Fixture *fxi = m_doc->fixture(fxid);
        if (fxi == nullptr)
            continue;

        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            if (channelKey(fxid, c) == key)
            {
                setValue(fxid, c, dmx);
                changed = true;
            }
        }
    }

    if (changed)
    {
        setLastParam(paramOfKey(key));
        emit valuesChanged();
        emit channelsChanged();
    }
}

void Programmer::releaseChannel(const QString &key)
{
    bool changed = false;
    for (auto it = m_values.begin(); it != m_values.end();)
    {
        if (m_selection.contains(it.key().first) && channelKey(it.key().first, it.key().second) == key)
        {
            m_source->unset(it.key().first, it.key().second);
            it = m_values.erase(it);
            changed = true;
        }
        else
        {
            ++it;
        }
    }

    if (changed)
    {
        emit valuesChanged();
        emit channelsChanged();
    }
}

int Programmer::releaseChannels(const QList<quint32> &fixtures, const std::function<bool(const QLCChannel *)> &match)
{
    int count = 0;

    // the FX of the programmer on those channels go too
    bool fxChanged = false;
    for (FXScene::Layer &layer : m_fxLayers)
    {
        for (int i = layer.fixtures.count() - 1; i >= 0; i--)
        {
            quint32 fid = layer.fixtures.at(i);
            Fixture *fxi = fixtures.contains(fid) ? m_doc->fixture(fid) : nullptr;
            if (fxi == nullptr)
                continue;
            QStringList params = layer.isPanTilt() ? QStringList({ "pan", "tilt" }) : QStringList({ layer.param });
            for (const QString &param : params)
            {
                quint32 c = FXScene::channelFor(fxi, param);
                if (c != QLCChannel::invalid() && match(fxi->channel(c)))
                {
                    layer.own.remove(fid);
                    layer.fixtures.removeAt(i);
                    fxChanged = true;
                    break;
                }
            }
        }
    }
    if (fxChanged)
    {
        m_fxLayers.erase(std::remove_if(m_fxLayers.begin(), m_fxLayers.end(),
                                        [](const FXScene::Layer &l) { return l.fixtures.isEmpty(); }), m_fxLayers.end());
        syncFx();
    }

    for (auto it = m_values.begin(); it != m_values.end();)
    {
        Fixture *fxi = fixtures.contains(it.key().first) ? m_doc->fixture(it.key().first) : nullptr;
        const QLCChannel *ch = fxi != nullptr ? fxi->channel(it.key().second) : nullptr;
        if (ch != nullptr && match(ch))
        {
            m_source->unset(it.key().first, it.key().second);
            it = m_values.erase(it);
            count++;
        }
        else
        {
            ++it;
        }
    }

    if (count > 0)
    {
        emit valuesChanged();
        emit channelsChanged();
    }
    return count;
}

/** The dimmers of a fixture: its intensity channels without a colour (a
 *  master dimmer, the dimmer of each head) */
static QList<quint32> dimmerChannels(const Fixture *fxi)
{
    QList<quint32> channels;
    if (fxi == nullptr)
        return channels;
    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        if (ch != nullptr && ch->group() == QLCChannel::Intensity &&
            ch->colour() == QLCChannel::NoColour && ch->controlByte() == QLCChannel::MSB)
            channels.append(c);
    }
    return channels;
}

int Programmer::setDimmer(int value)
{
    uchar dmx = uchar(qBound(0, value, 255));
    int fixtures = 0;

    for (quint32 fxid : m_selection)
    {
        const QList<quint32> dimmers = dimmerChannels(m_doc->fixture(fxid));
        for (quint32 c : dimmers)
            setValue(fxid, c, dmx);
        if (dimmers.isEmpty() == false)
            fixtures++;
    }

    setLastParam("dimmer");
    emit valuesChanged();
    emit channelsChanged();
    return fixtures;
}

void Programmer::setColor(const QColor &color)
{
    setColorFor(m_selection, color);
    setLastParam("red");
}

void Programmer::setColorFor(const QList<quint32> &fixtures, const QColor &color)
{
    for (quint32 fxid : fixtures)
    {
        const QMap<quint32, uchar> values = colourMix(m_doc->fixture(fxid), color);
        for (auto it = values.cbegin(); it != values.cend(); ++it)
            setValue(fxid, it.key(), it.value());
    }

    emit valuesChanged();
    emit channelsChanged();
}

QMap<quint32, uchar> Programmer::colourMix(const Fixture *fxi, const QColor &color)
{
    QMap<quint32, uchar> values;
    if (fxi == nullptr || color.isValid() == false)
        return values;

    const int r = color.red(), g = color.green(), b = color.blue();
    // the white part of the colour goes to the white emitter, when there is one
    const int w = qMin(r, qMin(g, b));

    bool hasWhite = false;
    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        if (ch != nullptr && ch->group() == QLCChannel::Intensity && ch->colour() == QLCChannel::White)
            hasWhite = true;
    }

    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        if (ch == nullptr || ch->controlByte() != QLCChannel::MSB || ch->group() != QLCChannel::Intensity)
            continue;

        int value = -1;
        switch (ch->colour())
        {
            case QLCChannel::Red: value = hasWhite ? r - w : r; break;
            case QLCChannel::Green: value = hasWhite ? g - w : g; break;
            case QLCChannel::Blue: value = hasWhite ? b - w : b; break;
            case QLCChannel::White: value = w; break;
            case QLCChannel::Cyan: value = 255 - r; break;
            case QLCChannel::Magenta: value = 255 - g; break;
            case QLCChannel::Yellow: value = 255 - b; break;
            default: break;
        }
        if (value >= 0)
            values.insert(c, uchar(value));
    }
    return values;
}

void Programmer::clear()
{
    m_source->unsetAll();
    m_values.clear();
    m_valueKeys.clear();
    m_paletteRefs.clear();
    m_fxLayers.clear();
    // (no fan either)
    m_fanFixtures.clear();
    m_fanBases.clear();
    m_fanWritten.clear();
    m_fanPan = 0.0;
    m_fanTilt = 0.0;
    dropAim();
    syncFx();

    emit valuesChanged();
    emit channelsChanged();
    emit cleared();
}

/*********************************************************************
 * Record
 *********************************************************************/

int Programmer::recordScene(const QString &name, int zoneIndex, int priority)
{
    if (m_values.isEmpty() && m_fxLayers.isEmpty())
        return -1;

    // with FX: a scene with its base values and its FX layers
    if (m_fxLayers.isEmpty() == false)
    {
        FXScene *fx = new FXScene(m_doc);
        fx->setName(name.isEmpty() ? tr("FX %1").arg(m_doc->nextFunctionID() + 1) : name);
        if (priority >= 0)
            fx->setPriority(priority);
        QList<quint32> fixtures = valueFixturesInOrder();
        for (const FXScene::Layer &layer : std::as_const(m_fxLayers))
            for (quint32 fid : layer.fixtures)
                if (fixtures.contains(fid) == false)
                    fixtures.append(fid);
        fx->setFixtures(fixtures);
        fx->setValues(QHash<QPair<quint32, quint32>, uchar>(m_values.cbegin(), m_values.cend()));
        fx->setLayers(m_fxLayers);
        if (m_doc->addFunction(fx) == false)
        {
            delete fx;
            return -1;
        }
        Zone *zone = m_zones->zone(zoneIndex);
        if (zone != nullptr)
            zone->addFunction(fx->id());
        if (m_palettes != nullptr)
            m_palettes->setSceneRefs(fx->id(), paletteRefs());
        return int(fx->id());
    }

    Scene *scene = new Scene(m_doc);
    scene->setName(name.isEmpty() ? tr("Scene %1").arg(m_doc->nextFunctionID() + 1) : name);
    if (priority >= 0)
        scene->setPriority(priority);

    // the scene keeps the selection order: staggered fades follow it
    for (quint32 fid : valueFixturesInOrder())
        scene->addFixture(fid);
    for (auto it = m_values.cbegin(); it != m_values.cend(); ++it)
    {
        if (scene->fixtures().contains(it.key().first) == false)
            scene->addFixture(it.key().first);
        scene->setValue(it.key().first, it.key().second, it.value());
    }

    if (m_doc->addFunction(scene) == false)
    {
        delete scene;
        return -1;
    }

    Zone *zone = m_zones->zone(zoneIndex);
    if (zone != nullptr)
        zone->addFunction(scene->id());
    // the values that come from palettes follow them
    if (m_palettes != nullptr)
        m_palettes->setSceneRefs(scene->id(), paletteRefs());

    return int(scene->id());
}

bool Programmer::updateScene(quint32 sceneId)
{
    Function *f = m_doc->function(sceneId);
    if (f == nullptr || (m_values.isEmpty() && m_fxLayers.isEmpty()))
        return false;

    // a scene with FX, or a static scene that gets FX: an FX scene with its ID
    FXScene *fx = qobject_cast<FXScene *>(f);
    if (fx == nullptr && m_fxLayers.isEmpty() == false && qobject_cast<Scene *>(f) != nullptr)
    {
        // steps play their channels through their scene (a scene of the
        // user in QLC+ shows): it stays a static scene
        for (Function *other : m_doc->functionsByType(Function::SequenceType))
        {
            if (Sequence *sequence = qobject_cast<Sequence *>(other))
                if (sequence->boundSceneID() == sceneId)
                    return false;
        }
        // the old scene is deleted: the chasers and shows that play it let
        // it go (Doc::functionReplaced)
        fx = new FXScene(m_doc);
        fx->setName(f->name());
        fx->setFadeInSpeed(f->fadeInSpeed());
        fx->setFadeOutSpeed(f->fadeOutSpeed());
        fx->setPriority(f->priority());
        fx->setPath(f->path(true));
        f->stopAndWait();
        if (m_doc->replaceFunction(sceneId, fx) == false)
        {
            delete fx;
            return false;
        }
    }
    if (fx != nullptr)
    {
        QList<quint32> fixtures = valueFixturesInOrder();
        for (const FXScene::Layer &layer : std::as_const(m_fxLayers))
            for (quint32 fid : layer.fixtures)
                if (fixtures.contains(fid) == false)
                    fixtures.append(fid);
        fx->setFixtures(fixtures);
        fx->setValues(QHash<QPair<quint32, quint32>, uchar>(m_values.cbegin(), m_values.cend()));
        fx->setLayers(m_fxLayers);
        if (m_palettes != nullptr)
            m_palettes->setSceneRefs(sceneId, paletteRefs());
        m_doc->setModified();
        emit sceneUpdated(sceneId);
        return true;
    }

    Scene *scene = qobject_cast<Scene *>(f);
    if (scene == nullptr || m_values.isEmpty())
        return false;

    scene->clear();
    for (quint32 fid : valueFixturesInOrder())
        scene->addFixture(fid);
    for (auto it = m_values.cbegin(); it != m_values.cend(); ++it)
    {
        if (scene->fixtures().contains(it.key().first) == false)
            scene->addFixture(it.key().first);
        scene->setValue(it.key().first, it.key().second, it.value());
    }
    if (m_palettes != nullptr)
        m_palettes->setSceneRefs(sceneId, paletteRefs());

    m_doc->setModified();
    emit sceneUpdated(sceneId);
    return true;
}

bool Programmer::loadScene(quint32 sceneId)
{
    Function *f = m_doc->function(sceneId);
    // the values that come from palettes still do
    const QHash<QPair<quint32, quint32>, int> refs =
        m_palettes != nullptr ? m_palettes->sceneRefs(sceneId) : QHash<QPair<quint32, quint32>, int>();

    // a scene with FX: its base values and its FX
    if (FXScene *fx = qobject_cast<FXScene *>(f))
    {
        const QHash<QPair<quint32, quint32>, uchar> hash = fx->values();
        QMap<QPair<quint32, quint32>, uchar> values;
        for (auto it = hash.cbegin(); it != hash.cend(); ++it)
            values.insert(it.key(), it.value());
        // the programmer names the fixtures of each FX ("none" is every
        // fixture of the scene)
        QList<FXScene::Layer> layers = fx->layers();
        for (FXScene::Layer &layer : layers)
            if (layer.fixtures.isEmpty())
                layer.fixtures = fx->fixtures();
        loadState(values, fx->fixtures(), layers, refs);
        // a cue loaded before is not what the programmer holds now
        emit lookLoaded();
        return true;
    }

    Scene *scene = qobject_cast<Scene *>(f);
    if (scene == nullptr)
        return false;

    // a static scene: no FX (the ones the programmer had would be recorded into it)
    QMap<QPair<quint32, quint32>, uchar> values;
    QList<quint32> fixtures;
    for (const SceneValue &scv : scene->values())
    {
        values.insert(qMakePair(scv.fxi, scv.channel), scv.value);
        if (fixtures.contains(scv.fxi) == false)
            fixtures.append(scv.fxi);
    }
    loadState(values, fixtures, QList<FXScene::Layer>(), refs);
    emit lookLoaded();
    return true;
}

void Programmer::slotFixtureRemoved(quint32 fixtureId)
{
    // a show cleared: everything goes at once (slotDocCleared), not a
    // fixture at a time, each one telling the selection, the values and
    // the FX again
    if (m_doc->isClearing())
        return;
    m_keyCache.remove(fixtureId);
    m_valueKeys.remove(fixtureId);
    m_selection.removeAll(fixtureId);
    for (const FXScene::Layer &layer : QList<FXScene::Layer>(m_fxLayers))
        removeFromFx(layer.param, { fixtureId });
    for (auto it = m_values.begin(); it != m_values.end();)
    {
        if (it.key().first == fixtureId)
        {
            m_source->unset(it.key().first, it.key().second);
            it = m_values.erase(it);
        }
        else
        {
            ++it;
        }
    }

    emit selectionChanged();
    emit valuesChanged();
    emit channelsChanged();
}

void Programmer::slotFixtureChanged(quint32 fixtureId)
{
    Fixture *fxi = m_doc->fixture(fixtureId);
    if (fxi == nullptr)
        return;
    // (a new mode makes new keys, so does a definition edited)
    m_keyCache.remove(fixtureId);
    const QStringList before = m_valueKeys.take(fixtureId);

    bool hasValues = false;
    for (auto it = m_values.cbegin(); it != m_values.cend() && hasValues == false; ++it)
        hasValues = it.key().first == fixtureId;
    bool inFx = false;
    for (const FXScene::Layer &layer : std::as_const(m_fxLayers))
        inFx = inFx || layer.fixtures.contains(fixtureId);
    if (hasValues == false && inFx == false)
    {
        if (m_selection.contains(fixtureId))
        {
            updateHighlight();
            emit channelsChanged();
        }
        return;
    }

    // its values: on the same attributes as before (another mode may have
    // its dimmer on another channel; what it doesn't have any more goes),
    // sent again so that they reach where it is now
    const QStringList keys = keysOf(fxi);
    const bool sameKeys = before.isEmpty() || before == keys;
    QMap<quint32, uchar> kept;
    bool changed = false;
    for (auto it = m_values.begin(); it != m_values.end();)
    {
        if (it.key().first != fixtureId)
        {
            ++it;
            continue;
        }
        const quint32 channel = it.key().second;
        int now = sameKeys ? int(channel) : int(keys.indexOf(before.value(int(channel))));
        if (now < 0 || quint32(now) >= fxi->channels() || (sameKeys == false && before.value(int(channel)).isEmpty()))
            now = -1;
        if (now != int(channel))
        {
            // a value somewhere else, or gone: not from its palette any more
            m_paletteRefs.remove(it.key());
            changed = true;
        }
        if (now >= 0)
            kept.insert(quint32(now), it.value());
        m_source->unset(fixtureId, channel);
        it = m_values.erase(it);
    }
    for (auto it = kept.cbegin(); it != kept.cend(); ++it)
    {
        const QPair<quint32, quint32> key(fixtureId, it.key());
        m_values.insert(key, it.value());
        if (m_blind == false && m_fxChannels.contains(key) == false)
            m_source->set(fixtureId, it.key(), it.value());
    }
    if (kept.isEmpty() == false)
        m_valueKeys.insert(fixtureId, keys);

    if (m_selection.contains(fixtureId))
        updateHighlight();
    // the FX find its channels again (valuesChanged syncs them too)
    if (changed)
        emit valuesChanged();
    else
        syncFx();
    emit channelsChanged();
}

void Programmer::slotDocCleared()
{
    m_keyCache.clear();
    m_valueKeys.clear();
    m_paletteRefs.clear();
    m_stepIndex = -1;
    m_stepBase.clear();
    // a new show starts without highlight
    if (m_highlight)
    {
        m_highlight = false;
        emit highlightChanged();
    }
    m_source->unsetAll();
    m_selection.clear();
    m_values.clear();
    m_fxLayers.clear();
    syncFx();

    emit selectionChanged();
    emit valuesChanged();
    emit channelsChanged();
}

void Programmer::selectFixtures(const QVariantList &fixtureIds, bool additive)
{
    if (additive == false)
        m_selection.clear();

    for (const QVariant &v : fixtureIds)
    {
        quint32 id = v.toUInt();
        if (m_doc->fixture(id) != nullptr && m_selection.contains(id) == false)
            m_selection.append(id);
    }

    emit selectionChanged();
    emit channelsChanged();
}

/*********************************************************************
 * Selection and beam tools
 *********************************************************************/

QList<quint32> Programmer::spatialOrder() const
{
    // left to right, then front to back, like reading the 2D plan;
    // fixtures without a position follow in patch order
    MonitorProperties *monitor = m_doc->monitorProperties();
    QList<Fixture *> fixtures = m_doc->fixtures();
    std::sort(fixtures.begin(), fixtures.end(), [monitor](Fixture *a, Fixture *b) {
        bool pa = monitor->containsFixture(a->id());
        bool pb = monitor->containsFixture(b->id());
        if (pa != pb)
            return pa;
        if (pa)
        {
            QVector3D va = monitor->fixturePosition(a->id(), 0, 0);
            QVector3D vb = monitor->fixturePosition(b->id(), 0, 0);
            // fixtures less than 30 cm apart vertically are on the same row
            if (qAbs(va.y() - vb.y()) > 300)
                return va.y() < vb.y();
            if (va.x() != vb.x())
                return va.x() < vb.x();
        }
        return a->universeAddress() < b->universeAddress();
    });

    QList<quint32> ids;
    for (Fixture *fxi : fixtures)
        ids.append(fxi->id());
    return ids;
}

void Programmer::invertSelection()
{
    QList<quint32> inverted;
    for (quint32 id : spatialOrder())
    {
        if (m_selection.contains(id) == false)
            inverted.append(id);
    }
    m_selection = inverted;

    emit selectionChanged();
    emit channelsChanged();
}

void Programmer::selectEvery(int n)
{
    if (n < 2)
        return;

    QList<quint32> base = m_selection.isEmpty() ? spatialOrder() : m_selection;
    QList<quint32> picked;
    for (int i = 0; i < base.count(); i += n)
        picked.append(base.at(i));
    m_selection = picked;

    emit selectionChanged();
    emit channelsChanged();
}

void Programmer::shiftSelection(int direction)
{
    QList<quint32> order = spatialOrder();
    if (order.isEmpty() || m_selection.isEmpty())
        return;

    int count = int(order.count());
    QList<quint32> shifted;
    for (quint32 id : std::as_const(m_selection))
    {
        int idx = int(order.indexOf(id));
        if (idx < 0)
            continue;
        quint32 next = order.at(((idx + direction) % count + count) % count);
        if (shifted.contains(next) == false)
            shifted.append(next);
    }
    m_selection = shifted;

    emit selectionChanged();
    emit channelsChanged();
}

static int capabilityValue(const QLCChannel *ch, const QStringList &words)
{
    // the first word has precedence over the others
    for (const QString &word : words)
    {
        for (QLCCapability *cap : ch->capabilities())
        {
            if (cap->name().contains(word, Qt::CaseInsensitive))
                return (cap->min() + cap->max()) / 2;
        }
    }
    return -1;
}

void Programmer::setBeam(bool on)
{
    for (quint32 fxid : std::as_const(m_selection))
    {
        Fixture *fxi = m_doc->fixture(fxid);
        if (fxi == nullptr)
            continue;

        bool hasDimmer = false;
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch != nullptr && ch->group() == QLCChannel::Intensity &&
                ch->colour() == QLCChannel::NoColour && ch->controlByte() == QLCChannel::MSB)
                hasDimmer = true;
        }

        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch == nullptr || ch->controlByte() != QLCChannel::MSB)
                continue;

            if (ch->group() == QLCChannel::Intensity)
            {
                bool isWhiteMix = ch->colour() == QLCChannel::Red || ch->colour() == QLCChannel::Green ||
                                  ch->colour() == QLCChannel::Blue || ch->colour() == QLCChannel::White;
                if (ch->colour() == QLCChannel::NoColour)
                    setValue(fxid, c, on ? 255 : 0);
                else if (hasDimmer == false && isWhiteMix)
                    // without a dimmer, light comes from the colour mix
                    setValue(fxid, c, on ? 255 : 0);
            }
            else if (ch->group() == QLCChannel::Shutter)
            {
                int value = on ? capabilityValue(ch, QStringList() << "open" << "abierto")
                               : capabilityValue(ch, QStringList() << "closed" << "close" << "cerrado" << "blackout");
                if (value >= 0)
                    setValue(fxid, c, uchar(value));
            }
        }
    }
    notifyValues();
}

void Programmer::beamOn()
{
    setBeam(true);
}

void Programmer::beamOff()
{
    setBeam(false);
}

void Programmer::centreBeam()
{
    for (quint32 fxid : std::as_const(m_selection))
    {
        Fixture *fxi = m_doc->fixture(fxid);
        if (fxi == nullptr)
            continue;

        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            bool fine = false;
            if (isPositionChannel(ch, nullptr, &fine))
                setValue(fxid, c, fine ? 0 : 128);
        }
    }
    dropAim();
    notifyValues();
}

/** $channelKey is $key on a head: "dimmer 2" for "dimmer" */
static bool isHeadKey(const QString &channelKey, const QString &key)
{
    if (channelKey.length() <= key.length() + 1 || channelKey.startsWith(key + QLatin1Char(' ')) == false)
        return false;
    bool isNumber = false;
    channelKey.mid(key.length() + 1).toInt(&isNumber);
    return isNumber;
}

int Programmer::setChannelFan(const QString &key, int base, int spread, bool fromCentre)
{
    // the fixtures of the selection that have this channel, in selection
    // order; one with it on each of its heads (a bar) sets them all
    QList<QPair<quint32, QList<quint32>>> targets;
    for (quint32 fxid : std::as_const(m_selection))
    {
        Fixture *fxi = m_doc->fixture(fxid);
        if (fxi == nullptr)
            continue;
        const QStringList keys = keysOf(fxi);
        QList<quint32> channels;
        const int exact = int(keys.indexOf(key));
        if (exact >= 0)
        {
            channels.append(quint32(exact));
        }
        else
        {
            for (int c = 0; c < keys.count(); c++)
                if (isHeadKey(keys.at(c), key))
                    channels.append(quint32(c));
        }
        if (channels.isEmpty() == false)
            targets.append(qMakePair(fxid, channels));
    }

    int n = int(targets.count());
    for (int i = 0; i < n; i++)
    {
        qreal t = n > 1 ? qreal(i) / qreal(n - 1) : 0.0;
        qreal d = fromCentre ? qAbs(2.0 * t - 1.0) : t;
        int value = qBound(0, qRound(base + spread * d), 255);
        for (quint32 c : targets.at(i).second)
            setValue(targets.at(i).first, c, uchar(value));
    }
    setLastParam(paramOfKey(key));
    notifyValues();
    return n;
}

bool Programmer::blind() const
{
    return m_blind;
}

void Programmer::setBlind(bool blind)
{
    if (blind == m_blind)
        return;
    m_blind = blind;

    if (m_blind)
    {
        // keep the values, stop sending them
        m_source->unsetAll();
    }
    else
    {
        for (auto it = m_values.cbegin(); it != m_values.cend(); ++it)
            if (m_fxChannels.contains(it.key()) == false)
                m_source->set(it.key().first, it.key().second, it.value());
    }
    // blind, the FX don't play either
    syncFx();
    emit blindChanged();
}

QList<SceneValue> Programmer::sceneValues() const
{
    QList<SceneValue> list;
    for (auto it = m_values.cbegin(); it != m_values.cend(); ++it)
        list.append(SceneValue(it.key().first, it.key().second, it.value()));
    return list;
}

void Programmer::loadValues(const QList<SceneValue> &values)
{
    QMap<QPair<quint32, quint32>, uchar> map;
    QList<quint32> fixtures;
    for (const SceneValue &scv : values)
    {
        map.insert(qMakePair(scv.fxi, scv.channel), scv.value);
        if (fixtures.contains(scv.fxi) == false)
            fixtures.append(scv.fxi);
    }
    loadState(map, fixtures, QList<FXScene::Layer>(), QHash<QPair<quint32, quint32>, int>());
    // a step: a cue loaded before is not what the programmer holds now
    emit lookLoaded();
}

void Programmer::loadState(const QMap<QPair<quint32, quint32>, uchar> &values, const QList<quint32> &fixtures,
                           const QList<FXScene::Layer> &fx, const QHash<QPair<quint32, quint32>, int> &refs)
{
    m_source->unsetAll();
    m_values.clear();
    m_valueKeys.clear();
    m_paletteRefs.clear();
    m_selection.clear();

    // the FX first, on the fixtures of the show: the values of their
    // channels are then their base, not sent by the source
    m_fxLayers.clear();
    for (FXScene::Layer layer : fx)
    {
        QList<quint32> ids;
        for (quint32 fid : std::as_const(layer.fixtures))
            if (m_doc->fixture(fid) != nullptr && ids.contains(fid) == false)
                ids.append(fid);
        layer.fixtures = ids;
        layer.own = layer.tidyOwn();
        if (layer.fixtures.isEmpty() == false)
            m_fxLayers.append(layer);
    }
    m_fxChannels.clear();
    syncFx();

    for (auto it = values.cbegin(); it != values.cend(); ++it)
        if (m_doc->fixture(it.key().first) != nullptr)
            setValue(it.key().first, it.key().second, it.value());
    for (auto it = refs.cbegin(); it != refs.cend(); ++it)
        if (m_values.contains(it.key()))
            m_paletteRefs.insert(it.key(), it.value());

    // the fixtures in the order given, then the other ones with values or FX
    auto select = [this](quint32 fid) {
        if (m_doc->fixture(fid) != nullptr && m_selection.contains(fid) == false)
            m_selection.append(fid);
    };
    for (quint32 fid : fixtures)
        select(fid);
    for (auto it = m_values.cbegin(); it != m_values.cend(); ++it)
        select(it.key().first);
    for (const FXScene::Layer &layer : std::as_const(m_fxLayers))
        for (quint32 fid : layer.fixtures)
            select(fid);
    if (m_fxLayers.isEmpty() == false)
        setLastParam(m_fxLayers.first().param);

    emit selectionChanged();
    emit valuesChanged();
    emit channelsChanged();
}

QList<quint32> Programmer::selectionIds() const
{
    return m_selection;
}

QList<quint32> Programmer::valueFixturesInOrder() const
{
    // the selected fixtures first, in selection order, then the others
    QList<quint32> withValues;
    for (auto it = m_values.cbegin(); it != m_values.cend(); ++it)
        if (withValues.contains(it.key().first) == false)
            withValues.append(it.key().first);

    QList<quint32> order;
    for (quint32 fid : m_selection)
        if (withValues.contains(fid))
            order.append(fid);
    for (quint32 fid : withValues)
        if (order.contains(fid) == false)
            order.append(fid);
    return order;
}

/*********************************************************************
 * Live edits: priority, release, reset
 *********************************************************************/

void Programmer::setLivePriority(int priority)
{
    m_source->setPriority(qBound(0, priority, Function::maxPriority()));
    m_fx->setPriority(qBound(0, priority, Function::maxPriority()));
}

void Programmer::setReleaseMode(const QString &mode)
{
    m_releaseMode = mode;
}

void Programmer::slotFunctionStarted(quint32 id)
{
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return;

    // a cuelist: the channels of its first cue, drained before anything else
    if (Cuelist *cuelist = qobject_cast<Cuelist *>(f))
    {
        releaseForCue(cuelist);
        return;
    }

    // (FX alone are edits too: they are released like the values)
    if ((m_values.isEmpty() && m_fxLayers.isEmpty()) || m_releaseMode == "keep")
        return;

    if (m_releaseMode == "all")
    {
        // a scene the user triggers resets every edit (not the hidden
        // scenes that play the steps)
        if (f->isVisible())
            clear();
        return;
    }
    releaseUsedBy(id);
}

void Programmer::slotCueStarted(quint32 functionId, quint32 cueUid)
{
    Q_UNUSED(cueUid)

    // a GO inside a running list: functionStarted isn't emitted again
    if (Cuelist *cuelist = qobject_cast<Cuelist *>(m_doc->function(functionId)))
        releaseForCue(cuelist);
}

void Programmer::releaseForCue(Cuelist *cuelist)
{
    // always taken, even with nothing to release: the keys of this GO must
    // not wait for the next one (they would release the edits made since)
    const QSet<QPair<quint32, quint32>> keys = cuelist->takeChangedKeys();
    if (m_releaseMode == "keep" || (m_values.isEmpty() && m_fxLayers.isEmpty()))
        return;

    if (m_releaseMode == "all")
    {
        if (cuelist->isVisible())
            clear();
        return;
    }
    // the values on the channels the cue changed, and the FX of the programmer there
    releaseKeys(keys);
    releaseFxOn(keys);
}

bool Programmer::releaseKeys(const QSet<QPair<quint32, quint32>> &keys)
{
    bool changed = false;
    for (auto it = m_values.begin(); it != m_values.end();)
    {
        if (keys.contains(it.key()))
        {
            m_source->unset(it.key().first, it.key().second);
            it = m_values.erase(it);
            changed = true;
        }
        else
        {
            ++it;
        }
    }
    if (changed)
        notifyValues();
    return changed;
}

bool Programmer::releaseFxOn(const QSet<QPair<quint32, quint32>> &keys)
{
    bool fxChanged = false;
    for (FXScene::Layer &layer : m_fxLayers)
    {
        const QStringList params = layer.isPanTilt() ? QStringList({ "pan", "tilt" }) : QStringList({ layer.param });
        for (int i = layer.fixtures.count() - 1; i >= 0; i--)
        {
            const quint32 fid = layer.fixtures.at(i);
            for (const QString &param : params)
            {
                quint32 fine = QLCChannel::invalid();
                const quint32 c = FXScene::channelFor(m_doc->fixture(fid), param, &fine);
                if ((c != QLCChannel::invalid() && keys.contains(qMakePair(fid, c))) ||
                    (fine != QLCChannel::invalid() && keys.contains(qMakePair(fid, fine))))
                {
                    layer.own.remove(fid);
                    layer.fixtures.removeAt(i);
                    fxChanged = true;
                    break;
                }
            }
        }
    }
    if (fxChanged)
    {
        m_fxLayers.erase(std::remove_if(m_fxLayers.begin(), m_fxLayers.end(),
                                        [](const FXScene::Layer &l) { return l.fixtures.isEmpty(); }), m_fxLayers.end());
        syncFx();
    }
    return fxChanged;
}

void Programmer::releaseUsedBy(quint32 functionId)
{
    Function *f = m_doc->function(functionId);
    if (f == nullptr)
        return;

    if (Sequence *sequence = qobject_cast<Sequence *>(f))
        f = m_doc->function(sequence->boundSceneID());
    if (f == nullptr)
        return;

    if (Cuelist *cuelist = qobject_cast<Cuelist *>(f))
    {
        // what its GOs changed since the last time (the other channels it
        // holds were taken back by the GOs that set them)
        const QSet<QPair<quint32, quint32>> keys = cuelist->takeChangedKeys();
        releaseKeys(keys);
        releaseFxOn(keys);
        return;
    }

    if (Scene *scene = qobject_cast<Scene *>(f))
    {
        // exactly the channels of the scene
        QSet<QPair<quint32, quint32>> used;
        for (const SceneValue &scv : scene->values())
            used.insert(qMakePair(scv.fxi, scv.channel));
        releaseKeys(used);
        return;
    }

    if (FXScene *fx = qobject_cast<FXScene *>(f))
    {
        // its base values and the channels of its FX
        QSet<QPair<quint32, quint32>> used;
        const QHash<QPair<quint32, quint32>, uchar> values = fx->values();
        for (auto it = values.cbegin(); it != values.cend(); ++it)
            used.insert(it.key());
        for (const FXScene::Layer &layer : fx->layers())
        {
            QStringList params = layer.isPanTilt() ? QStringList({ "pan", "tilt" }) : QStringList({ layer.param });
            for (quint32 fid : layer.fixtures.isEmpty() ? fx->fixtures() : layer.fixtures)
                for (const QString &param : params)
                {
                    quint32 c = FXScene::channelFor(m_doc->fixture(fid), param);
                    if (c != QLCChannel::invalid())
                        used.insert(qMakePair(fid, c));
                }
        }
        // and the FX of the programmer there
        releaseKeys(used);
        releaseFxOn(used);
        return;
    }

    if (EFX *efx = qobject_cast<EFX *>(f))
    {
        QList<quint32> fixtures;
        for (EFXFixture *ef : efx->fixtures())
            fixtures.append(ef->head().fxi);
        bool dimmer = efx->dimmerControlEnabled();
        releaseChannels(fixtures, [dimmer](const QLCChannel *ch) {
            return ch->group() == QLCChannel::Pan || ch->group() == QLCChannel::Tilt ||
                   (dimmer && ch->group() == QLCChannel::Intensity && ch->colour() == QLCChannel::NoColour);
        });
    }
    else if (RGBMatrix *matrix = qobject_cast<RGBMatrix *>(f))
    {
        FixtureGroup *group = m_doc->fixtureGroup(matrix->fixtureGroup());
        if (group != nullptr)
        {
            bool dimmer = matrix->dimmerControl();
            releaseChannels(group->fixtureList(), [dimmer](const QLCChannel *ch) {
                return ch->colour() != QLCChannel::NoColour ||
                       (dimmer && ch->group() == QLCChannel::Intensity);
            });
        }
    }
    else if (CurveFX *curve = qobject_cast<CurveFX *>(f))
    {
        const QStringList features = curve->features();
        releaseChannels(curve->fixtures(), [features](const QLCChannel *ch) {
            return features.contains(ch->name().toLower());
        });
    }
}

void Programmer::resetSelected()
{
    bool changed = false;
    // the FX of the selection stop too
    for (const FXScene::Layer &layer : QList<FXScene::Layer>(m_fxLayers))
        removeFromFx(layer.param, m_selection);
    syncFx();
    for (auto it = m_values.begin(); it != m_values.end();)
    {
        if (m_selection.contains(it.key().first))
        {
            m_source->unset(it.key().first, it.key().second);
            it = m_values.erase(it);
            changed = true;
        }
        else
        {
            ++it;
        }
    }
    if (changed)
        notifyValues();
}

/*********************************************************************
 * Position (XY grid)
 *********************************************************************/

bool Programmer::isPositionChannel(const QLCChannel *ch, bool *pan, bool *fine)
{
    if (ch == nullptr)
        return false;
    bool isPan = false, isFine = false;
    switch (ch->preset())
    {
        case QLCChannel::PositionPan: case QLCChannel::PositionXAxis: isPan = true; break;
        case QLCChannel::PositionPanFine: isPan = true; isFine = true; break;
        case QLCChannel::PositionTilt: case QLCChannel::PositionYAxis: break;
        case QLCChannel::PositionTiltFine: isFine = true; break;
        case QLCChannel::Custom:
        {
            // no preset: its group, unless it is something else of the pan
            // and tilt. Its ranges say so (a rotation), or its name does
            // ("Pan Continuous Rotation", "Tilt Offset", "Auto Move", "Pan/Tilt
            // speed"...); a pan named "Rotation" (0-540°) or "Giro" is a pan
            if (ch->group() != QLCChannel::Pan && ch->group() != QLCChannel::Tilt)
                return false;
            for (const QLCCapability *cap : ch->capabilities())
            {
                if (cap->preset() >= QLCCapability::RotationStop &&
                    cap->preset() <= QLCCapability::RotationCounterClockwiseFastToSlow)
                    return false;
            }
            static const QRegularExpression other(QStringLiteral("contin|endless|sin fin|infinit|speed|spd|veloc|vitesse|geschw"
                                                                 "|offset|auto|macro|reset|mode|time"),
                                                  QRegularExpression::CaseInsensitiveOption);
            if (other.match(ch->name()).hasMatch())
                return false;
            isPan = ch->group() == QLCChannel::Pan;
            isFine = ch->controlByte() == QLCChannel::LSB;
            break;
        }
        // a preset of another thing of the pan or tilt (a rotation, a speed)
        default:
            return false;
    }
    if (pan != nullptr)
        *pan = isPan;
    if (fine != nullptr)
        *fine = isFine;
    return true;
}

void Programmer::setPosition(qreal x, qreal y)
{
    x = qBound(0.0, x, 1.0);
    y = qBound(0.0, y, 1.0);
    // with a fan: the centre moves, the fixtures keep their fan around it
    if (fanHolds() && (m_fanPan != 0.0 || m_fanTilt != 0.0))
    {
        for (quint32 fxid : std::as_const(m_fanFixtures))
            m_fanBases.insert(fxid, QPointF(x, y));
        applyPositionFan();
    }
    else
    {
        setPositionFor(m_selection, x, y);
    }
    setLastParam("pantilt");
}

void Programmer::writePosition(quint32 fixtureId, qreal x, qreal y, bool onlyMoves)
{
    Fixture *fxi = m_doc->fixture(fixtureId);
    if (fxi == nullptr)
        return;
    // (set this way, the heads aim at no point any more)
    dropAim();
    x = qBound(0.0, x, 1.0);
    y = qBound(0.0, y, 1.0);
    // (the fan: an axis that stays where it is isn't written, so it keeps
    // its palette, and one that isn't in the programmer stays out of it)
    bool panMoves = true, tiltMoves = true;
    if (onlyMoves)
    {
        const QVariantMap now = positionFor({ fixtureId });
        panMoves = qAbs(now.value("x").toDouble() - x) > 1e-9;
        tiltMoves = qAbs(now.value("y").toDouble() - y) > 1e-9;
    }
    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        bool pan = false, fine = false;
        if (isPositionChannel(fxi->channel(c), &pan, &fine) == false || (pan ? panMoves : tiltMoves) == false)
            continue;
        // 16 bit: the coarse channel and the fine one
        int value = qRound((pan ? x : y) * 65535.0);
        setValue(fixtureId, c, uchar(fine ? value & 0xff : value >> 8));
    }
}

void Programmer::setPositionFor(const QList<quint32> &fixtures, qreal x, qreal y)
{
    for (quint32 fxid : fixtures)
        writePosition(fxid, x, y);
    notifyValues();
}

QList<quint32> Programmer::positionedSelection() const
{
    QList<quint32> fixtures;
    for (quint32 fxid : m_selection)
    {
        if (positionFor({ fxid }).value("available").toBool())
            fixtures.append(fxid);
    }
    return fixtures;
}

bool Programmer::fanHolds() const
{
    // the same fixtures, in the same order, where the fan put them
    if (m_fanFixtures.isEmpty() || m_fanFixtures != positionedSelection())
        return false;
    for (quint32 fxid : m_fanFixtures)
    {
        const QVariantMap now = positionFor({ fxid });
        const QPointF written = m_fanWritten.value(fxid, QPointF(-1, -1));
        if (qAbs(now.value("x").toDouble() - written.x()) > 1e-9 || qAbs(now.value("y").toDouble() - written.y()) > 1e-9)
            return false;
    }
    return true;
}

void Programmer::applyPositionFan()
{
    const int n = int(m_fanFixtures.count());
    for (int i = 0; i < n; i++)
    {
        const quint32 fxid = m_fanFixtures.at(i);
        // where it is in the fan: -1..1 from one end to the other, 1 0 1 in
        // a V, 0..1 from the first one
        const qreal t = n > 1 ? qreal(i) / qreal(n - 1) : 0.5;
        qreal k = 2.0 * t - 1.0;
        if (m_fanMode == QLatin1String("v"))
            k = qAbs(k);
        else if (m_fanMode == QLatin1String("first"))
            k = n > 1 ? t : 0.0;
        const QPointF base = m_fanBases.value(fxid);
        writePosition(fxid, base.x() + m_fanPan * 0.5 * k, base.y() + m_fanTilt * 0.5 * k, true);
        // (what it has now, through its coarse and fine channels)
        const QVariantMap now = positionFor({ fxid });
        m_fanWritten.insert(fxid, QPointF(now.value("x").toDouble(), now.value("y").toDouble()));
    }
    notifyValues();
}

void Programmer::setPositionFan(qreal pan, qreal tilt, const QString &mode)
{
    pan = qIsFinite(pan) ? qBound(-1.0, pan, 1.0) : 0.0;
    tilt = qIsFinite(tilt) ? qBound(-1.0, tilt, 1.0) : 0.0;
    const bool validMode = mode == QLatin1String("spread") || mode == QLatin1String("v") || mode == QLatin1String("first");
    // no fan to open (a mode chosen first): the mode alone, nothing written
    if (pan == 0.0 && tilt == 0.0 && fanHolds() == false)
    {
        if (validMode && mode != m_fanMode)
        {
            m_fanMode = mode;
            emit channelsChanged();
        }
        return;
    }
    // a new fan (or the fixtures moved since): from where each one is now
    if (fanHolds() == false)
    {
        m_fanFixtures = positionedSelection();
        m_fanBases.clear();
        m_fanWritten.clear();
        for (quint32 fxid : std::as_const(m_fanFixtures))
        {
            const QVariantMap now = positionFor({ fxid });
            m_fanBases.insert(fxid, QPointF(now.value("x").toDouble(), now.value("y").toDouble()));
        }
    }
    if (m_fanFixtures.isEmpty())
        return;
    m_fanPan = pan;
    m_fanTilt = tilt;
    if (validMode)
        m_fanMode = mode;
    applyPositionFan();
    setLastParam("pantilt");
}

QVariantMap Programmer::positionFan() const
{
    const bool holds = fanHolds();
    return QVariantMap({ { "pan", holds ? m_fanPan : 0.0 }, { "tilt", holds ? m_fanTilt : 0.0 }, { "mode", m_fanMode } });
}

QVariantMap Programmer::position() const
{
    QVariantMap map = positionFor(m_selection);
    // where each fixture is (the fan shows)
    QVariantList points;
    for (quint32 fxid : positionedSelection())
    {
        const QVariantMap one = positionFor({ fxid });
        points.append(QVariantMap({ { "x", one.value("x") }, { "y", one.value("y") } }));
    }
    map.insert("points", points);
    // with a fan, the cross is its centre
    if ((m_fanPan != 0.0 || m_fanTilt != 0.0) && fanHolds())
    {
        qreal x = 0.0, y = 0.0;
        for (quint32 fxid : m_fanFixtures)
        {
            x += m_fanBases.value(fxid).x();
            y += m_fanBases.value(fxid).y();
        }
        map.insert("x", x / m_fanFixtures.count());
        map.insert("y", y / m_fanFixtures.count());
    }
    return map;
}

QVariantMap Programmer::positionFor(const QList<quint32> &fixtures) const
{
    QVariantMap map;
    map.insert("available", false);
    map.insert("x", 0.5);
    map.insert("y", 0.5);

    for (quint32 fxid : fixtures)
    {
        Fixture *fxi = m_doc->fixture(fxid);
        if (fxi == nullptr)
            continue;
        int pan = -1, tilt = -1, panFine = 0, tiltFine = 0;
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            bool isPan = false, fine = false;
            if (isPositionChannel(ch, &isPan, &fine) == false)
                continue;
            auto it = m_values.constFind(qMakePair(fxid, c));
            int value = it != m_values.constEnd() ? int(it.value()) : int(ch->defaultValue());
            if (fine == false)
                (isPan ? pan : tilt) = value;
            else
                (isPan ? panFine : tiltFine) = value;
        }
        if (pan < 0 && tilt < 0)
            continue;
        map.insert("available", true);
        map.insert("x", pan < 0 ? 0.5 : (pan * 256 + panFine) / 65535.0);
        map.insert("y", tilt < 0 ? 0.5 : (tilt * 256 + tiltFine) / 65535.0);
        break;
    }
    return map;
}

/*********************************************************************
 * Snapshots
 *********************************************************************/

bool Programmer::mergeIntoScene(quint32 sceneId)
{
    Scene *scene = qobject_cast<Scene *>(m_doc->function(sceneId));
    if (scene == nullptr || m_values.isEmpty())
        return false;

    for (quint32 fid : valueFixturesInOrder())
        if (scene->fixtures().contains(fid) == false)
            scene->addFixture(fid);
    for (auto it = m_values.cbegin(); it != m_values.cend(); ++it)
        scene->setValue(it.key().first, it.key().second, it.value());
    if (m_palettes != nullptr)
    {
        // the edits replace the values of the scene there, palettes included
        QList<QPair<quint32, quint32>> written = m_values.keys();
        m_palettes->mergeSceneRefs(sceneId, paletteRefs(), written);
    }

    m_doc->setModified();
    return true;
}

int Programmer::snapshotAll(const QString &name, int zoneIndex, int priority)
{
    // the highlight is over everything, and not part of the look: what is
    // under it can't be read, so it goes off first
    if (m_highlight)
        return -1;

    // what leaves the software now, before the grand master
    QList<SceneValue> values;
    QList<Universe *> universes = m_doc->inputOutputMap()->claimUniverses();
    for (Fixture *fxi : m_doc->fixtures())
    {
        if (fxi->universe() >= quint32(universes.count()))
            continue;
        Universe *universe = universes.at(int(fxi->universe()));
        QList<SceneValue> fixtureValues;
        bool lit = false;
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            uchar value = universe->preGMValue(int(fxi->address() + c));
            fixtureValues.append(SceneValue(fxi->id(), c, value));
            lit = lit || value > 0;
        }
        // fixtures at zero everywhere are not part of the look
        if (lit)
            values.append(fixtureValues);
    }
    m_doc->inputOutputMap()->releaseUniverses(false);

    if (values.isEmpty())
        return -1;

    Scene *scene = new Scene(m_doc);
    scene->setName(name.isEmpty() ? tr("Snapshot %1").arg(m_doc->nextFunctionID() + 1) : name);
    if (priority >= 0)
        scene->setPriority(priority);
    for (const SceneValue &scv : std::as_const(values))
    {
        if (scene->fixtures().contains(scv.fxi) == false)
            scene->addFixture(scv.fxi);
        scene->setValue(scv);
    }
    if (m_doc->addFunction(scene) == false)
    {
        delete scene;
        return -1;
    }
    if (Zone *zone = m_zones->zone(zoneIndex))
        zone->addFunction(scene->id());
    return int(scene->id());
}

/*********************************************************************
 * FX
 *********************************************************************/

QString Programmer::paramOfKey(const QString &key) const
{
    for (quint32 fxid : m_selection)
    {
        Fixture *fxi = m_doc->fixture(fxid);
        if (fxi == nullptr)
            continue;
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            if (channelKey(fxid, c) != key)
                continue;
            QString param = FXScene::paramOf(fxi->channel(c));
            // pan and tilt go together
            if (param == QLatin1String("pan") || param == QLatin1String("tilt"))
                return QStringLiteral("pantilt");
            return param;
        }
    }
    return QString();
}

QString Programmer::lastParam() const
{
    return m_lastParam;
}

void Programmer::setLastParam(const QString &param)
{
    if (param.isEmpty() || param == m_lastParam)
        return;
    m_lastParam = param;
    emit lastParamChanged();
}

QVariantList Programmer::fxParams() const
{
    QVariantList list;
    QStringList keys;
    bool pan = false, tilt = false;

    for (quint32 fxid : m_selection)
    {
        Fixture *fxi = m_doc->fixture(fxid);
        if (fxi == nullptr)
            continue;
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            QString param = FXScene::paramOf(ch);
            if (param.isEmpty() || keys.contains(param))
                continue;
            keys.append(param);
            pan = pan || param == QLatin1String("pan");
            tilt = tilt || param == QLatin1String("tilt");

            QVariantMap info;
            info.insert("key", param);
            info.insert("name", ch->name());
            info.insert("group", QLCChannel::groupToString(ch->group()));
            info.insert("colour", ch->colour() == QLCChannel::NoColour ? QString() : QColor(QRgb(ch->colour())).name());
            info.insert("active", fxLayerFor(param, false) >= 0);
            list.append(info);
        }
    }

    if (pan && tilt)
    {
        QVariantMap info;
        info.insert("key", "pantilt");
        info.insert("name", QString());
        info.insert("group", QLCChannel::groupToString(QLCChannel::Pan));
        info.insert("colour", QString());
        info.insert("active", fxLayerFor("pantilt", false) >= 0);
        list.prepend(info);
    }
    return list;
}

QList<quint32> Programmer::selectionWith(const QString &param) const
{
    QList<quint32> list;
    for (quint32 fid : m_selection)
    {
        Fixture *fxi = m_doc->fixture(fid);
        bool has = param == QLatin1String("pantilt")
                ? FXScene::channelFor(fxi, "pan") != QLCChannel::invalid() && FXScene::channelFor(fxi, "tilt") != QLCChannel::invalid()
                : FXScene::channelFor(fxi, param) != QLCChannel::invalid();
        if (has)
            list.append(fid);
    }
    return list;
}

int Programmer::fxLayerFor(const QString &param, bool exact) const
{
    // the selected fixtures that have the parameter
    const QList<quint32> with = selectionWith(param);
    if (with.isEmpty())
        return -1;
    const QSet<quint32> selection(with.cbegin(), with.cend());
    int partial = -1;
    for (int i = 0; i < m_fxLayers.count(); i++)
    {
        const FXScene::Layer &layer = m_fxLayers.at(i);
        if (layer.param != param)
            continue;
        const QSet<quint32> fixtures(layer.fixtures.cbegin(), layer.fixtures.cend());
        if (fixtures == selection)
            return i;
        if (partial < 0 && fixtures.intersects(selection))
            partial = i;
    }
    return exact ? -1 : partial;
}

void Programmer::removeFromFx(const QString &param, const QList<quint32> &fixtures)
{
    for (FXScene::Layer &layer : m_fxLayers)
    {
        if (layer.param != param)
            continue;
        for (quint32 fid : fixtures)
        {
            layer.fixtures.removeAll(fid);
            layer.own.remove(fid);
        }
    }
    m_fxLayers.erase(std::remove_if(m_fxLayers.begin(), m_fxLayers.end(),
                                    [](const FXScene::Layer &l) { return l.fixtures.isEmpty(); }), m_fxLayers.end());
}

FXScene *Programmer::fxScene() const
{
    return m_fx;
}

QVariantList Programmer::fxLayers() const
{
    QVariantList list;
    for (int i = 0; i < m_fxLayers.count(); i++)
    {
        QVariantMap map = FxTools::layerToMap(m_fxLayers.at(i));
        map.insert("index", i);
        map.insert("count", m_fxLayers.at(i).fixtures.count());
        map.insert("ownCount", m_fxLayers.at(i).tidyOwn().count());
        list.append(map);
    }
    return list;
}

QList<FXScene::Layer> Programmer::fxLayerList() const
{
    return m_fxLayers;
}

QVariantMap Programmer::fxOf(const QString &param) const
{
    int index = fxLayerFor(param, false);
    FXScene::Layer layer = index >= 0 ? m_fxLayers.at(index) : FxTools::defaultLayer(param);
    QVariantMap map = FxTools::layerToMap(layer);
    const QList<quint32> with = selectionWith(param);
    int ownSelected = 0;
    bool partial = false;
    if (index >= 0)
    {
        // what the first selected fixture of the FX does (its own way included)
        const QMap<quint32, FXScene::Own> own = layer.tidyOwn();
        quint32 first = Fixture::invalidId();
        for (quint32 fid : with)
        {
            if (layer.fixtures.contains(fid) == false)
                continue;
            if (first == Fixture::invalidId())
                first = fid;
            if (own.contains(fid))
                ownSelected++;
        }
        const FXScene::Shape shape = layer.shapeOf(first);
        map.insert("mode", shape.mode);
        map.insert("figure", shape.figure);
        map.insert("swing", shape.swing);
        map.insert("swingTilt", shape.swingTilt);
        map.insert("phase", shape.phase);
        map.insert("rotate", shape.rotate);
        map.insert("panInvert", shape.panInvert);
        map.insert("tiltInvert", shape.tiltInvert);
        map.insert("reverse", shape.reverse);
        map.insert("ownCount", own.count());
        // the ones on one side go the other way round: they follow the wave of their twins
        map.insert("mirror", FXScene::mirrorTwins(layer, FXScene::orderedFixtures(layer, layer.fixtures)).contains(true));
        // the FX has fixtures that are not selected: the shape changes only on the selected ones
        for (quint32 fid : std::as_const(layer.fixtures))
            partial = partial || with.contains(fid) == false;
    }
    map.insert("active", index >= 0);
    map.insert("exact", fxLayerFor(param, true) >= 0);
    map.insert("partial", partial);
    map.insert("ownSelected", ownSelected);
    map.insert("count", with.count());
    map.insert("sets", fxSets(param));
    return map;
}

int Programmer::fxSets(const QString &param) const
{
    int index = fxLayerFor(param, false);
    FXScene::Layer layer = index >= 0 ? m_fxLayers.at(index) : FxTools::defaultLayer(param);
    int sets = 1;
    FXScene::groupSets(m_doc, FXScene::orderedFixtures(layer, selectionWith(param)), layer.grouping, layer.groupSize, &sets);
    return sets;
}

void Programmer::setFx(const QString &param, const QVariantMap &props)
{
    const QList<quint32> with = selectionWith(param);
    if (with.isEmpty() || param.isEmpty())
        return;

    int index = fxLayerFor(param, false);
    if (index < 0)
    {
        // a new FX on the selection
        FXScene::Layer layer = FxTools::defaultLayer(param);
        layer.fixtures = with;
        m_fxLayers.append(layer);
        index = m_fxLayers.count() - 1;
    }
    else
    {
        // DMX Total Control: an FX stays one, on one clock:
        // the selected fixtures that are not on it join it. The ones of
        // another FX of the parameter keep what they did there as their own
        QList<quint32> joined;
        for (quint32 fid : with)
            if (m_fxLayers.at(index).fixtures.contains(fid) == false)
                joined.append(fid);
        for (quint32 fid : std::as_const(joined))
        {
            for (int i = 0; i < m_fxLayers.count(); i++)
            {
                FXScene::Layer &other = m_fxLayers[i];
                if (i == index || other.param != param || other.fixtures.contains(fid) == false)
                    continue;
                const FXScene::Shape s = other.shapeOf(fid);
                FXScene::Own was;
                was.mode = s.mode;
                was.figure = s.figure;
                was.swing = s.swing;
                was.swingTilt = s.swingTilt;
                was.phase = s.phase;
                was.rotate = s.rotate;
                was.panInvert = s.panInvert;
                was.tiltInvert = s.tiltInvert;
                was.reverse = s.reverse;
                m_fxLayers[index].own.insert(fid, was);
                other.fixtures.removeAll(fid);
                other.own.remove(fid);
            }
            m_fxLayers[index].fixtures.append(fid);
        }
        m_fxLayers[index].own = m_fxLayers.at(index).tidyOwn();
        // the FX left without fixtures go
        for (int i = m_fxLayers.count() - 1; i >= 0; i--)
        {
            if (m_fxLayers.at(i).fixtures.isEmpty())
            {
                m_fxLayers.removeAt(i);
                if (i < index)
                    index--;
            }
        }
    }

    const bool wasLinked = m_fxLayers.at(index).linked;
    QVariantMap changes = props;
    changes.remove("param");
    changes.remove("fixtures");
    // the speed, the timing and the link are the whole FX's; the shape, the
    // size and the direction are of the selected fixtures, for all of them
    // when they are all selected
    QVariantMap shape;
    for (const QString &key : FxTools::ownKeys())
        if (changes.contains(key))
            shape.insert(key, changes.take(key));
    FXScene::Layer &layer = m_fxLayers[index];
    layer = FxTools::layerFromMap(changes, layer);
    bool all = true;
    for (quint32 fid : std::as_const(layer.fixtures))
        all = all && with.contains(fid);
    if (all)
    {
        layer = FxTools::layerFromMap(shape, layer);
        FxTools::clearOwn(layer, shape.keys());
    }
    else
    {
        FxTools::setOwn(layer, with, shape);
    }
    changes.insert(shape);

    // linked FX share the speed and the timing
    if (m_fxLayers.at(index).linked)
    {
        QVariantMap timing;
        if (wasLinked == false)
        {
            // it joins the others: it takes their timing
            for (int i = 0; i < m_fxLayers.count(); i++)
            {
                if (i != index && m_fxLayers.at(i).linked)
                {
                    QVariantMap theirs = FxTools::layerToMap(m_fxLayers.at(i));
                    for (const QString &key : FxTools::timingKeys())
                        timing.insert(key, theirs.value(key));
                    m_fxLayers[index] = FxTools::layerFromMap(timing, m_fxLayers.at(index));
                    break;
                }
            }
        }
        else
        {
            for (const QString &key : FxTools::timingKeys())
                if (changes.contains(key))
                    timing.insert(key, changes.value(key));
            for (int i = 0; i < m_fxLayers.count(); i++)
                if (i != index && m_fxLayers.at(i).linked)
                    m_fxLayers[i] = FxTools::layerFromMap(timing, m_fxLayers.at(i));
        }
    }

    setLastParam(param);
    syncFx();
}

void Programmer::stopFx(const QString &param)
{
    int index = fxLayerFor(param, false);
    if (index < 0)
        return;

    // stopping a linked FX stops the ones linked to it
    QStringList params = { param };
    if (m_fxLayers.at(index).linked)
        for (const FXScene::Layer &layer : std::as_const(m_fxLayers))
            if (layer.linked && params.contains(layer.param) == false)
                params.append(layer.param);
    for (const QString &p : std::as_const(params))
        removeFromFx(p, m_selection);
    syncFx();
}

void Programmer::resetFxOwn(const QString &param)
{
    int index = fxLayerFor(param, false);
    if (index < 0)
        return;
    FXScene::Layer &layer = m_fxLayers[index];
    const QMap<quint32, FXScene::Own> before = layer.tidyOwn();
    FxTools::clearOwn(layer, FxTools::ownKeys(), selectionWith(param));
    if (layer.own != before)
        syncFx();
}

void Programmer::stopAllFx()
{
    if (m_fxLayers.isEmpty())
        return;
    m_fxLayers.clear();
    syncFx();
}

void Programmer::selectFx(int index)
{
    if (index < 0 || index >= m_fxLayers.count())
        return;
    QList<quint32> fixtures;
    for (quint32 fid : m_fxLayers.at(index).fixtures)
        if (m_doc->fixture(fid) != nullptr)
            fixtures.append(fid);
    m_selection = fixtures;
    m_lastParam = m_fxLayers.at(index).param;
    emit selectionChanged();
    emit channelsChanged();
    emit lastParamChanged();
}

QVariantList Programmer::fxPreview(const QString &param, qreal cyclePos) const
{
    int index = fxLayerFor(param, false);
    if (index < 0)
        return QVariantList();
    return m_fx->previewLayer(index, cyclePos);
}

int Programmer::applyFxMacro(const QList<FXScene::Layer> &layers, const QMap<QString, int> &bases)
{
    if (m_selection.isEmpty())
        return 0;

    int applied = 0;
    QString first;
    for (FXScene::Layer layer : layers)
    {
        // the selected fixtures that have the parameter
        QList<quint32> fixtures;
        for (quint32 fid : m_selection)
        {
            Fixture *fxi = m_doc->fixture(fid);
            bool has = layer.isPanTilt()
                    ? FXScene::channelFor(fxi, "pan") != QLCChannel::invalid() && FXScene::channelFor(fxi, "tilt") != QLCChannel::invalid()
                    : FXScene::channelFor(fxi, layer.param) != QLCChannel::invalid();
            if (has)
                fixtures.append(fid);
        }
        if (fixtures.isEmpty())
            continue;
        removeFromFx(layer.param, fixtures);
        layer.fixtures = fixtures;
        // (a macro is for any fixtures: what some did their own way stays out)
        layer.own.clear();
        m_fxLayers.append(layer);
        if (first.isEmpty())
            first = layer.param;
        applied++;
    }
    if (applied == 0)
        return 0;

    // its base values
    for (auto it = bases.cbegin(); it != bases.cend(); ++it)
    {
        for (quint32 fid : m_selection)
        {
            quint32 fine = QLCChannel::invalid();
            quint32 c = FXScene::channelFor(m_doc->fixture(fid), it.key(), &fine);
            if (c == QLCChannel::invalid())
                continue;
            setValue(fid, c, uchar(qBound(0, it.value(), 255)));
            if (fine != QLCChannel::invalid())
                setValue(fid, fine, 0);
        }
    }

    setLastParam(first);
    syncFx();
    notifyValues();
    return applied;
}

QMap<QString, int> Programmer::fxBaseValues(const QList<FXScene::Layer> &layers) const
{
    QMap<QString, int> bases;
    if (m_selection.isEmpty())
        return bases;
    Fixture *fxi = m_doc->fixture(m_selection.last());
    if (fxi == nullptr)
        return bases;
    for (const FXScene::Layer &layer : layers)
    {
        QStringList params = layer.isPanTilt() ? QStringList({ "pan", "tilt" }) : QStringList({ layer.param });
        for (const QString &param : params)
        {
            quint32 c = FXScene::channelFor(fxi, param);
            auto it = c != QLCChannel::invalid() ? m_values.constFind(qMakePair(fxi->id(), c)) : m_values.constEnd();
            if (it != m_values.constEnd())
                bases.insert(param, it.value());
        }
    }
    return bases;
}

void Programmer::syncFx()
{
    // the channels the FX move: the source leaves them to the FX
    QSet<QPair<quint32, quint32>> channels;
    for (const FXScene::Layer &layer : std::as_const(m_fxLayers))
    {
        QStringList params = layer.isPanTilt() ? QStringList({ "pan", "tilt" }) : QStringList({ layer.param });
        for (quint32 fid : layer.fixtures)
        {
            Fixture *fxi = m_doc->fixture(fid);
            for (const QString &param : params)
            {
                quint32 fine = QLCChannel::invalid();
                quint32 c = FXScene::channelFor(fxi, param, &fine);
                if (c != QLCChannel::invalid())
                    channels.insert(qMakePair(fid, c));
                if (fine != QLCChannel::invalid())
                    channels.insert(qMakePair(fid, fine));
            }
        }
    }
    if (m_blind == false)
    {
        // the channels that leave the FX: the programmer sends them again
        for (const QPair<quint32, quint32> &key : std::as_const(m_fxChannels))
        {
            auto it = m_values.constFind(key);
            if (channels.contains(key) == false && it != m_values.constEnd())
                m_source->set(key.first, key.second, it.value());
        }
        for (const QPair<quint32, quint32> &key : std::as_const(channels))
            if (m_values.contains(key))
                m_source->unset(key.first, key.second);
    }
    m_fxChannels = channels;

    const bool play = m_fxLayers.isEmpty() == false && m_blind == false;
    // started but not running yet (the timer starts it at its next tick)
    // counts too: it would run on with the layers of before. Stopped first,
    // it doesn't write what follows
    if (play == false && (m_fx->isRunning() || m_fx->stopped() == false))
        m_fx->stop(FunctionParent::master());

    // the programmer values of those channels are the base of the FX. Blind
    // too: the FX panel previews them (fxPreview)
    QHash<QPair<quint32, quint32>, uchar> bases;
    QList<quint32> fixtures;
    for (const QPair<quint32, quint32> &key : std::as_const(channels))
    {
        auto it = m_values.constFind(key);
        if (it != m_values.constEnd())
            bases.insert(key, it.value());
    }
    for (const FXScene::Layer &layer : std::as_const(m_fxLayers))
        for (quint32 fid : layer.fixtures)
            if (fixtures.contains(fid) == false)
                fixtures.append(fid);
    m_fx->setFixtures(fixtures);
    m_fx->setValues(bases);
    m_fx->setLayers(m_fxLayers);

    if (play && (m_fx->isRunning() == false || m_fx->stopped()))
        m_fx->start(m_doc->masterTimer(), FunctionParent::master());
    emit fxChanged();
}

void Programmer::restartFx()
{
    syncFx();
}

bool Programmer::isFxRunning() const
{
    return m_fx->isRunning();
}

QColor Programmer::colorFor(const QList<quint32> &fixtures) const
{
    for (quint32 fxid : fixtures)
    {
        Fixture *fxi = m_doc->fixture(fxid);
        if (fxi == nullptr)
            continue;
        int r = -1, g = -1, b = -1, w = 0;
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch == nullptr || ch->group() != QLCChannel::Intensity || ch->controlByte() != QLCChannel::MSB)
                continue;
            auto it = m_values.constFind(qMakePair(fxid, c));
            if (it == m_values.constEnd())
                continue;
            int v = it.value();
            switch (ch->colour())
            {
                case QLCChannel::Red: r = v; break;
                case QLCChannel::Green: g = v; break;
                case QLCChannel::Blue: b = v; break;
                case QLCChannel::White: w = v; break;
                case QLCChannel::Cyan: r = 255 - v; break;
                case QLCChannel::Magenta: g = 255 - v; break;
                case QLCChannel::Yellow: b = 255 - v; break;
                default: break;
            }
        }
        if (r >= 0 || g >= 0 || b >= 0)
            return QColor(qMin(255, qMax(0, r) + w), qMin(255, qMax(0, g) + w), qMin(255, qMax(0, b) + w));
    }
    return QColor();
}

/*********************************************************************
 * Console tools
 *********************************************************************/

QMap<quint32, uchar> Programmer::openWhite(const Fixture *fxi) const
{
    QMap<quint32, uchar> values;
    if (fxi == nullptr)
        return values;

    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        if (ch == nullptr)
            continue;
        const bool msb = ch->controlByte() == QLCChannel::MSB;
        switch (ch->group())
        {
            case QLCChannel::Intensity:
                switch (ch->colour())
                {
                    case QLCChannel::NoColour:
                        values.insert(c, msb ? 255 : 0);
                        break;
                    // white: the emitters that make it at full, the others off
                    case QLCChannel::Red: case QLCChannel::Green: case QLCChannel::Blue: case QLCChannel::White:
                        values.insert(c, msb ? 255 : 0);
                        break;
                    default:
                        // CMY open, amber, UV, lime... off
                        values.insert(c, 0);
                        break;
                }
                break;
            case QLCChannel::Shutter:
            {
                if (FXScene::attributeOf(ch) == QLatin1String("iris"))
                    break;
                int value = capabilityValue(ch, QStringList() << "open" << "abierto");
                if (value >= 0 && msb)
                    values.insert(c, uchar(value));
                break;
            }
            case QLCChannel::Colour:
            {
                // the open or white slot of a colour wheel
                int value = capabilityValue(ch, QStringList() << "open" << "white" << "blanco" << "abierto");
                if (value >= 0 && msb)
                    values.insert(c, uchar(value));
                break;
            }
            default:
                break;
        }
    }
    return values;
}

void Programmer::locate()
{
    for (quint32 fxid : std::as_const(m_selection))
    {
        Fixture *fxi = m_doc->fixture(fxid);
        if (fxi == nullptr)
            continue;

        QMap<quint32, uchar> values = openWhite(fxi);
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch == nullptr || values.contains(c))
                continue;
            switch (ch->group())
            {
                // centred (a rotation or a speed of them: at rest)
                case QLCChannel::Pan: case QLCChannel::Tilt:
                {
                    bool fine = false;
                    if (isPositionChannel(ch, nullptr, &fine))
                        values.insert(c, fine ? 0 : 128);
                    else
                        values.insert(c, ch->defaultValue());
                    break;
                }
                // an open beam: no gobo, no prism, the iris open, the rest at rest
                case QLCChannel::Gobo: case QLCChannel::Prism: case QLCChannel::Beam: case QLCChannel::Shutter:
                case QLCChannel::Effect:
                {
                    int value = ch->controlByte() == QLCChannel::MSB
                              ? capabilityValue(ch, QStringList() << "open" << "no gobo" << "off" << "abierto" << "none")
                              : -1;
                    values.insert(c, uchar(value >= 0 ? value : ch->defaultValue()));
                    break;
                }
                default:
                    break;
            }
        }
        for (auto it = values.cbegin(); it != values.cend(); ++it)
            setValue(fxid, it.key(), it.value());
    }
    notifyValues();
}

bool Programmer::highlight() const
{
    return m_highlight;
}

void Programmer::setHighlight(bool on)
{
    if (on == m_highlight)
        return;
    m_highlight = on;
    updateHighlight();
    emit highlightChanged();
}

void Programmer::updateHighlight()
{
    m_highlightSource->unsetAll();
    if (m_highlight == false)
        return;
    for (quint32 fxid : std::as_const(m_selection))
    {
        const QMap<quint32, uchar> values = openWhite(m_doc->fixture(fxid));
        for (auto it = values.cbegin(); it != values.cend(); ++it)
            m_highlightSource->set(fxid, it.key(), it.value());
    }
}

void Programmer::slotSelectionChanged()
{
    // a selection made another way ends the trip through the old one
    if (m_inStep == false)
    {
        m_stepIndex = -1;
        m_stepBase.clear();
    }
    updateHighlight();
}

void Programmer::setStep(int index)
{
    m_stepIndex = index;
    m_inStep = true;
    m_selection = { m_stepBase.at(index) };
    emit selectionChanged();
    emit channelsChanged();
    m_inStep = false;
}

void Programmer::nextFixture()
{
    if (m_stepIndex < 0)
    {
        // from the selection, or every fixture when nothing is selected
        m_stepBase = m_selection;
        if (m_stepBase.isEmpty())
            for (Fixture *fxi : m_doc->fixtures())
                m_stepBase.append(fxi->id());
        if (m_stepBase.isEmpty())
            return;
        setStep(0);
        return;
    }
    setStep((m_stepIndex + 1) % int(m_stepBase.count()));
}

void Programmer::previousFixture()
{
    if (m_stepIndex < 0)
    {
        m_stepBase = m_selection;
        if (m_stepBase.isEmpty())
            for (Fixture *fxi : m_doc->fixtures())
                m_stepBase.append(fxi->id());
        if (m_stepBase.isEmpty())
            return;
        setStep(int(m_stepBase.count()) - 1);
        return;
    }
    int n = int(m_stepBase.count());
    setStep((m_stepIndex - 1 + n) % n);
}

void Programmer::allFixtures()
{
    if (m_stepIndex < 0)
        return;
    QList<quint32> base;
    for (quint32 fid : std::as_const(m_stepBase))
        if (m_doc->fixture(fid) != nullptr)
            base.append(fid);
    m_stepIndex = -1;
    m_stepBase.clear();
    m_selection = base;
    emit selectionChanged();
    emit channelsChanged();
}

QVariantMap Programmer::stepping() const
{
    return QVariantMap({ { "active", m_stepIndex >= 0 },
                         { "index", m_stepIndex },
                         { "count", int(m_stepBase.count()) } });
}

int Programmer::nudgeDimmer(int delta)
{
    int fixtures = 0;
    for (quint32 fxid : std::as_const(m_selection))
    {
        const QList<quint32> dimmers = dimmerChannels(m_doc->fixture(fxid));
        for (quint32 c : dimmers)
        {
            auto it = m_values.constFind(qMakePair(fxid, c));
            int now = it != m_values.constEnd() ? int(it.value()) : 0;
            setValue(fxid, c, uchar(qBound(0, now + delta, 255)));
        }
        if (dimmers.isEmpty() == false)
            fixtures++;
    }
    setLastParam("dimmer");
    notifyValues();
    return fixtures;
}

/*********************************************************************
 * Palettes
 *********************************************************************/

void Programmer::setPalettes(PaletteManager *palettes)
{
    m_palettes = palettes;
}

/*********************************************************************
 * Follow
 *********************************************************************/

void Programmer::setPatch(PatchManager *patch)
{
    if (m_patch != nullptr)
        disconnect(m_patch, nullptr, this, nullptr);
    m_patch = patch;
    // the heads moved, turned or inverted: they aim no more
    if (m_patch != nullptr)
        connect(m_patch, &PatchManager::fixturesChanged, this, &Programmer::dropAim);
}

QVariantMap Programmer::aimAt(qreal x, qreal y, qreal z)
{
    if (m_patch == nullptr)
        return aim();

    const QVector3D target = QVector3D(float(x), float(y), float(z));
    m_aimWritten.clear();
    QVariantList aimed, unreachable, guessed, skipped;
    for (quint32 fxid : positionedSelection())
    {
        bool guess = false;
        const AimMount mount = m_patch->aimMount(fxid, &guess);
        // a scanner moves a mirror: not this geometry
        if (mount.mirror)
        {
            skipped.append(fxid);
            continue;
        }
        QPointF angles;
        const bool reached = Aim::solve(mount, target, m_aimAngles.value(fxid), m_aimAngles.contains(fxid), &angles);
        m_aimAngles.insert(fxid, angles);
        writeAim(fxid, angles.x() / mount.panRange, angles.y() / mount.tiltRange);
        aimed.append(fxid);
        if (reached == false)
            unreachable.append(fxid);
        if (guess)
            guessed.append(fxid);
    }

    m_aim = { { "active", true }, { "x", x }, { "y", y }, { "z", z }, { "aimed", aimed },
              { "unreachable", unreachable }, { "guessed", guessed }, { "skipped", skipped } };
    notifyValues();
    setLastParam("pantilt");
    emit aimChanged();
    return m_aim;
}

QVariantMap Programmer::aim() const
{
    if (m_aim.isEmpty())
        return { { "active", false } };
    return m_aim;
}

bool Programmer::aimHolds() const
{
    for (auto it = m_aimWritten.cbegin(); it != m_aimWritten.cend(); ++it)
    {
        auto value = m_values.constFind(it.key());
        if (value == m_values.cend() || value.value() != it.value())
            return false;
    }
    return true;
}

void Programmer::dropAim()
{
    m_aimAngles.clear();
    m_aimWritten.clear();
    if (m_aim.isEmpty())
        return;
    m_aim.clear();
    emit aimChanged();
}

// the value to give a channel so its modifier (an inversion, the limits)
// sends $wanted, or the nearest it can
static uchar beforeModifier(const ChannelModifier *modifier, uchar wanted)
{
    if (modifier == nullptr)
        return wanted;
    int best = wanted, bestGap = 256;
    for (int value = 0; value < 256 && bestGap > 0; value++)
    {
        const int gap = qAbs(int(modifier->getValue(uchar(value))) - int(wanted));
        if (gap < bestGap)
        {
            best = value;
            bestGap = gap;
        }
    }
    return uchar(best);
}

void Programmer::writeAim(quint32 fixtureId, qreal pan, qreal tilt)
{
    Fixture *fxi = m_doc->fixture(fixtureId);
    if (fxi == nullptr)
        return;
    const int panValue = qRound(qBound(0.0, pan, 1.0) * 65535.0);
    const int tiltValue = qRound(qBound(0.0, tilt, 1.0) * 65535.0);
    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        bool isPan = false, fine = false;
        if (isPositionChannel(fxi->channel(c), &isPan, &fine) == false)
            continue;
        // what it moves at the output: with pan and tilt swapped, the other one
        bool outPan = isPan, outFine = fine;
        const quint32 out = fxi->outputChannel(c);
        if (out != c && out < fxi->channels())
            isPositionChannel(fxi->channel(out), &outPan, &outFine);
        const int value = outPan ? panValue : tiltValue;
        const uchar wanted = uchar(outFine ? value & 0xff : value >> 8);
        const uchar given = beforeModifier(fxi->channelModifier(c), wanted);
        m_aimWritten.insert(qMakePair(fixtureId, c), given);
        setValue(fixtureId, c, given);
    }
}

PaletteManager *Programmer::palettes() const
{
    return m_palettes;
}

const QMap<QPair<quint32, quint32>, uchar> &Programmer::valueMap() const
{
    return m_values;
}

QHash<QPair<quint32, quint32>, int> Programmer::paletteRefs() const
{
    // only the values that are still there
    QHash<QPair<quint32, quint32>, int> refs;
    for (auto it = m_paletteRefs.cbegin(); it != m_paletteRefs.cend(); ++it)
        if (m_values.contains(it.key()))
            refs.insert(it.key(), it.value());
    return refs;
}

void Programmer::setPaletteRefs(const QHash<QPair<quint32, quint32>, int> &refs)
{
    m_paletteRefs.clear();
    for (auto it = refs.cbegin(); it != refs.cend(); ++it)
        if (m_values.contains(it.key()))
            m_paletteRefs.insert(it.key(), it.value());
    emit valuesChanged();
}

void Programmer::applyPaletteValues(const QHash<quint32, QMap<quint32, uchar>> &values, int paletteId)
{
    for (auto f = values.cbegin(); f != values.cend(); ++f)
    {
        for (auto it = f.value().cbegin(); it != f.value().cend(); ++it)
        {
            setValue(f.key(), it.key(), it.value());
            m_paletteRefs.insert(qMakePair(f.key(), it.key()), paletteId);
        }
    }
    notifyValues();
}

void Programmer::releasePaletteValues(const QList<quint32> &fixtures, int paletteId)
{
    bool changed = false;
    for (auto it = m_values.begin(); it != m_values.end();)
    {
        if (fixtures.contains(it.key().first) && m_paletteRefs.value(it.key(), -1) == paletteId)
        {
            m_source->unset(it.key().first, it.key().second);
            m_paletteRefs.remove(it.key());
            it = m_values.erase(it);
            changed = true;
        }
        else
        {
            ++it;
        }
    }
    if (changed)
        notifyValues();
}

void Programmer::setPaletteValue(quint32 fixtureId, quint32 channel, uchar value)
{
    const QPair<quint32, quint32> key(fixtureId, channel);
    int paletteId = m_paletteRefs.value(key, -1);
    setValue(fixtureId, channel, value);
    if (paletteId >= 0)
        m_paletteRefs.insert(key, paletteId);
    notifyValues();
}

void Programmer::forgetPalette(int paletteId)
{
    for (auto it = m_paletteRefs.begin(); it != m_paletteRefs.end();)
    {
        if (it.value() == paletteId)
            it = m_paletteRefs.erase(it);
        else
            ++it;
    }
    emit valuesChanged();
}
