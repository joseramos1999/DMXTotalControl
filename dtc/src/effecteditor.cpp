/*
  DMX Total Control
  effecteditor.cpp

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

#include <QPolygonF>
#include <QVector3D>
#include <QUrl>
#include <QFileInfo>
#include <QSet>
#include <QDebug>

#include "effecteditor.h"
#include "appsettings.h"
#include "zonemanager.h"
#include "programmer.h"
#include "zone.h"
#include "scenetaps.h"

#include "rgbscriptv4.h"
#include "fixturegroup.h"
#include "inputoutputmap.h"
#include "universe.h"
#include "functionparent.h"
#include "mastertimer.h"
#include "efxfixture.h"
#include "collection.h"
#include "rgbmatrix.h"
#include "qlcchannel.h"
#include "sequence.h"
#include "rgbtext.h"
#include "rgbimage.h"
#include "curvefx.h"
#include "cuelist.h"
#include "fxtools.h"
#include "fxscene.h"
#include "track.h"
#include "show.h"
#include "monitorproperties.h"
#include "effectexchange.h"
#include "fixture.h"
#include "channelmodifier.h"
#include "qlcfixturehead.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"

#define DEFAULT_STEP_HOLD   1000
#define DEFAULT_COLOUR_FX   QStringLiteral("Gradient")

EffectEditor::EffectEditor(Doc *doc, Programmer *programmer, ZoneManager *zones, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_programmer(programmer)
    , m_zones(zones)
    , m_settings(nullptr)
{
}

void EffectEditor::setSettings(AppSettings *settings)
{
    m_settings = settings;
}

void EffectEditor::setHeldByFader(const std::function<bool(quint32)> &held)
{
    m_heldByFader = held;
}

void EffectEditor::touch(quint32 id)
{
    m_doc->setModified();
    emit edited(id);
}

QString EffectEditor::uniqueName(const QString &pattern) const
{
    QSet<QString> names;
    for (Function *f : m_doc->functions())
        names.insert(f->name());
    int n = 1;
    while (names.contains(pattern.arg(n)))
        n++;
    return pattern.arg(n);
}

void EffectEditor::applyDefaultPriority(Function *function) const
{
    if (m_settings != nullptr)
        function->setPriority(m_settings->defaultPriority());
}

int EffectEditor::addNew(Function *function, const QString &name, int zoneIndex)
{
    // no name: the name of its kind in the UI, numbered
    QString pattern;
    switch (function->type())
    {
        case Function::SequenceType: pattern = tr("Steps %1"); break;
        case Function::EFXType: pattern = tr("Move FX %1"); break;
        case Function::RGBMatrixType: pattern = tr("Colour FX %1"); break;
        case Function::CollectionType: pattern = tr("Effect rack %1"); break;
        case Function::CurveFXType: pattern = tr("Curve FX %1"); break;
        case Function::ShowType: pattern = tr("Super scene %1"); break;
        default: pattern = tr("Scene %1"); break;
    }
    function->setName(name.trimmed().isEmpty() ? uniqueName(pattern) : name.trimmed());
    applyDefaultPriority(function);
    if (m_doc->addFunction(function) == false)
    {
        delete function;
        return -1;
    }

    Zone *zone = m_zones->zone(zoneIndex);
    if (zone != nullptr)
        zone->addFunction(function->id());

    return int(function->id());
}

/*********************************************************************
 * Common
 *********************************************************************/

static QString colourHex(const QColor &c)
{
    return c.isValid() ? c.name() : QString();
}

QVariantMap EffectEditor::info(quint32 id) const
{
    QVariantMap map;
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return map;

    map.insert("id", f->id());
    map.insert("name", f->name());
    map.insert("type", Function::typeToString(f->type()));
    map.insert("fadeIn", f->fadeInSpeed());
    map.insert("fadeOut", f->fadeOutSpeed());
    map.insert("duration", f->duration());
    map.insert("followBpm", f->tempoType() == Function::Beats);
    map.insert("singleShot", f->runOrder() == Function::SingleShot);
    map.insert("priority", f->priority());
    map.insert("running", f->isRunning());

    // playback settings; "Play once" of older shows is one loop
    map.insert("loops", f->runOrder() == Function::SingleShot && f->loopCount() == 0 ? 1 : f->loopCount());
    map.insert("endAction", f->endAction() == Function::EndPause ? "pause" : "release");
    map.insert("jumpTo", f->jumpTo());
    QString drive = "off";
    if (f->driveMode() == Function::DriveBeatGO)
        drive = "beatgo";
    else if (f->driveMode() == Function::DrivePulse)
        drive = "pulse";
    else if (f->tempoType() == Function::Beats)
        drive = "bpm";
    map.insert("drive", drive);
    map.insert("divisions", f->divisions());
    map.insert("startMode", f->startMode() == Function::StartPaused ? "paused" : "play");
    map.insert("stagger", f->stagger());
    // its own rhythm, tapped on its button (ms a step, 0: none), and whether
    // it takes one (steps, cuelists, FX at the BPM of the show)
    const bool tappable = SceneTaps::tappable(f);
    map.insert("tap", tappable ? f->tapPeriod() : 0u);
    map.insert("tappable", tappable);
    map.insert("cycle", EffectExchange::cycleDuration(m_doc, id));

    if (Scene *scene = qobject_cast<Scene *>(f))
    {
        map.insert("channels", scene->values().count());
        map.insert("fixtures", scene->fixtures().count());
    }
    else if (FXScene *fx = qobject_cast<FXScene *>(f))
    {
        // the FX: its base values and its layers
        map.insert("channels", fx->values().count());
        map.insert("fixtures", fx->fixtures().count());
        QVariantList layers;
        const QList<FXScene::Layer> list = fx->layers();
        for (int i = 0; i < list.count(); i++)
        {
            QVariantMap layer = FxTools::layerToMap(list.at(i));
            const QList<quint32> fixtures = list.at(i).fixtures.isEmpty() ? fx->fixtures() : list.at(i).fixtures;
            int sets = 1;
            FXScene::groupSets(m_doc, FXScene::orderedFixtures(list.at(i), fixtures), list.at(i).grouping,
                               list.at(i).groupSize, &sets);
            layer.insert("index", i);
            layer.insert("count", fixtures.count());
            layer.insert("sets", sets);
            // the fixtures that do it their own way (made in the programmer)
            layer.insert("ownCount", list.at(i).tidyOwn().count());
            layers.append(layer);
        }
        map.insert("layers", layers);
    }
    else if (qobject_cast<Sequence *>(f) != nullptr)
    {
        map.insert("steps", steps(id));
    }
    else if (Cuelist *cuelist = qobject_cast<Cuelist *>(f))
    {
        map.insert("cueCount", cuelist->cueCount());
    }
    else if (EFX *efx = qobject_cast<EFX *>(f))
    {
        map.insert("shape", EFX::algorithmToString(efx->algorithm()));
        map.insert("width", efx->width());
        map.insert("height", efx->height());
        map.insert("rotation", efx->rotation());
        map.insert("xOffset", efx->xOffset());
        map.insert("yOffset", efx->yOffset());
        map.insert("phase", efx->startOffset());
        map.insert("spread", int(efx->propagationMode()));
        map.insert("symmetric", efx->isSymmetric());
        map.insert("phaseSpread", efx->phaseSpread());
        map.insert("relative", efx->isRelative());
        map.insert("xFrequency", efx->xFrequency());
        map.insert("yFrequency", efx->yFrequency());
        map.insert("dimmer", efx->dimmerControlEnabled());
        map.insert("fixtures", efx->fixtures().count());
    }
    else if (RGBMatrix *matrix = qobject_cast<RGBMatrix *>(f))
    {
        RGBAlgorithm *algo = matrix->algorithm();
        map.insert("effect", algo != nullptr ? algo->name() : QString());
        map.insert("acceptColors", algo != nullptr ? algo->acceptColors() : 0);

        QVariantList colors;
        for (int i = 0; i < RGBAlgorithmColorDisplayCount; i++)
            colors.append(colourHex(matrix->getColor(i)));
        map.insert("colors", colors);
        map.insert("mode", int(matrix->controlMode()));
        map.insert("dimmerControl", matrix->dimmerControl());

        if (algo != nullptr && algo->type() == RGBAlgorithm::Text)
            map.insert("text", static_cast<RGBText *>(algo)->text());

        QVariantList properties;
        if (algo != nullptr && algo->type() == RGBAlgorithm::Script)
        {
            RGBScript *script = static_cast<RGBScript *>(algo);
            for (const RGBScriptProperty &prop : script->properties())
            {
                QVariantMap p;
                p.insert("name", prop.m_name);
                p.insert("label", prop.m_displayName);
                p.insert("value", matrix->property(prop.m_name));
                switch (prop.m_type)
                {
                    case RGBScriptProperty::List:
                        p.insert("type", "list");
                        p.insert("values", prop.m_listValues);
                    break;
                    case RGBScriptProperty::Range:
                        p.insert("type", "range");
                        p.insert("min", prop.m_rangeMinValue);
                        p.insert("max", prop.m_rangeMaxValue);
                    break;
                    case RGBScriptProperty::Float:
                        p.insert("type", "float");
                    break;
                    default:
                        p.insert("type", "string");
                    break;
                }
                properties.append(p);
            }
        }
        map.insert("properties", properties);

        FixtureGroup *group = m_doc->fixtureGroup(matrix->fixtureGroup());
        map.insert("fixtures", group != nullptr ? group->fixtureList().count() : 0);
        // laid in one row (selection order) or like the 2D plan (mappings)
        map.insert("layout", group != nullptr && group->name().endsWith(" (FX 2D)") ? "plan" : "line");

        if (algo != nullptr && algo->type() == RGBAlgorithm::Image)
        {
            RGBImage *image = static_cast<RGBImage *>(algo);
            map.insert("image", image->filename());
            map.insert("animation", RGBImage::animationStyleToString(image->animationStyle()));
        }
    }
    else if (CurveFX *curve = qobject_cast<CurveFX *>(f))
    {
        map.insert("wave", CurveFX::waveformToString(curve->waveform()));
        map.insert("low", curve->low());
        map.insert("high", curve->high());
        map.insert("phase", curve->phaseOffset());
        map.insert("spread", curve->spread());
        map.insert("duty", curve->duty());
        map.insert("relative", curve->isRelative());
        map.insert("features", curve->features());
        map.insert("fixtures", curve->fixtures().count());
    }
    else if (Collection *rack = qobject_cast<Collection *>(f))
    {
        QVariantList items;
        for (quint32 fid : rack->functions())
        {
            Function *item = m_doc->function(fid);
            if (item == nullptr)
                continue;
            QVariantMap i;
            i.insert("id", fid);
            i.insert("name", item->name());
            i.insert("type", Function::typeToString(item->type()));
            items.append(i);
        }
        map.insert("items", items);
    }

    return map;
}

void EffectEditor::setTiming(quint32 id, int fadeIn, int fadeOut, int duration)
{
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return;

    if (fadeIn >= 0)
        f->setFadeInSpeed(uint(fadeIn));
    if (fadeOut >= 0)
        f->setFadeOutSpeed(uint(fadeOut));
    if (duration >= 0)
        f->setDuration(uint(qMax(1, duration)));
    touch(id);
}

void EffectEditor::setFollowBpm(quint32 id, bool follow)
{
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return;

    f->setTempoType(follow ? Function::Beats : Function::Time);
    touch(id);
}

void EffectEditor::setSingleShot(quint32 id, bool singleShot)
{
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return;

    f->setRunOrder(singleShot ? Function::SingleShot : Function::Loop);
    touch(id);
}

void EffectEditor::togglePreview(quint32 id)
{
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return;

    if (f->isRunning())
    {
        // (one a fader of the SHOW holds up: only that fader stops it)
        if (m_heldByFader == nullptr || m_heldByFader(id) == false)
            f->stop(FunctionParent::master());
        return;
    }

    // the programmer overrides everything: an effect that is tried takes back
    // the channels it plays, or it would run unseen
    QList<quint32> fixtures;
    std::function<bool(const QLCChannel *)> match;
    if (EFX *efx = qobject_cast<EFX *>(f))
    {
        for (EFXFixture *ef : efx->fixtures())
            fixtures.append(ef->head().fxi);
        match = [](const QLCChannel *ch) {
            return ch->group() == QLCChannel::Pan || ch->group() == QLCChannel::Tilt;
        };
    }
    else if (RGBMatrix *matrix = qobject_cast<RGBMatrix *>(f))
    {
        if (FixtureGroup *group = m_doc->fixtureGroup(matrix->fixtureGroup()))
            fixtures = group->fixtureList();
        match = [](const QLCChannel *ch) { return ch->colour() != QLCChannel::NoColour; };
    }
    else if (CurveFX *curve = qobject_cast<CurveFX *>(f))
    {
        fixtures = curve->fixtures();
        const QStringList features = curve->features();
        match = [features](const QLCChannel *ch) { return features.contains(ch->name().toLower()); };
    }
    if (match && m_programmer->releaseChannels(fixtures, match) > 0)
        emit message(tr("The programmer released the channels of the effect, so you can see it"), false);

    f->start(m_doc->masterTimer(), FunctionParent::master());

    if (qobject_cast<EFX *>(f) != nullptr && !isLit(fixtures))
        emit message(tr("The effect is running, but the heads are dark: open their beam in Program to see it"), false);
}

bool EffectEditor::isLit(const QList<quint32> &fixtures) const
{
    QList<Universe *> universes = m_doc->inputOutputMap()->claimUniverses();
    bool lit = false;
    for (quint32 fid : fixtures)
    {
        Fixture *fxi = m_doc->fixture(fid);
        if (fxi == nullptr)
            continue;
        quint32 dimmer = fxi->masterIntensityChannel();
        if (dimmer == QLCChannel::invalid() || fxi->universe() >= quint32(universes.count()))
        {
            lit = true; // no dimmer to check
            break;
        }
        if (universes.at(fxi->universe())->preGMValue(fxi->address() + dimmer) > 0)
        {
            lit = true;
            break;
        }
    }
    m_doc->inputOutputMap()->releaseUniverses(false);
    return lit || fixtures.isEmpty();
}

bool EffectEditor::isRunning(quint32 id) const
{
    Function *f = m_doc->function(id);
    return f != nullptr && f->isRunning();
}

quint32 EffectEditor::duplicate(quint32 id)
{
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return Function::invalidId();

    // a cuelist copies its cues with their palettes
    Function *copy = f->createCopy(m_doc);
    if (copy == nullptr)
        return Function::invalidId();

    copy->setName(tr("%1 (copy)").arg(f->name()));

    // steps get their own channels: deleting the copy deletes its hidden
    // scene, and it was the one of the original
    if (Sequence *sequence = qobject_cast<Sequence *>(copy))
    {
        Function *bound = m_doc->function(sequence->boundSceneID());
        Function *boundCopy = bound != nullptr ? bound->createCopy(m_doc) : nullptr;
        if (boundCopy != nullptr)
        {
            boundCopy->setName(bound->isVisible() ? tr("%1 (copy)").arg(bound->name())
                                                  : tr("%1 (channels)").arg(copy->name()));
            sequence->setBoundSceneID(boundCopy->id());
            const QList<ChaserStep> steps = sequence->steps();
            for (int i = 0; i < steps.count(); i++)
            {
                ChaserStep step = steps.at(i);
                step.fid = boundCopy->id();
                sequence->replaceStep(step, i);
            }
        }
    }
    return copy->id();
}

/*********************************************************************
 * Steps
 *********************************************************************/

int EffectEditor::createSteps(const QString &name, int zoneIndex)
{
    QList<SceneValue> values = m_programmer->sceneValues();
    if (values.isEmpty())
    {
        emit message(tr("Set some values in the programmer first: they become the first step"), true);
        return -1;
    }

    // the channels the steps control live in a hidden scene (the bound scene)
    Scene *bound = new Scene(m_doc);
    bound->setVisible(false);
    for (const SceneValue &scv : values)
    {
        if (bound->fixtures().contains(scv.fxi) == false)
            bound->addFixture(scv.fxi);
        bound->setValue(scv.fxi, scv.channel, scv.value);
    }
    if (m_doc->addFunction(bound) == false)
    {
        delete bound;
        return -1;
    }

    Sequence *sequence = new Sequence(m_doc);
    sequence->setBoundSceneID(bound->id());
    sequence->setFadeInMode(Chaser::PerStep);
    sequence->setFadeOutMode(Chaser::PerStep);
    sequence->setDurationMode(Chaser::PerStep);
    sequence->setRunOrder(Function::Loop);

    int id = addNew(sequence, name, zoneIndex);
    if (id < 0)
    {
        m_doc->deleteFunction(bound->id());
        return -1;
    }
    bound->setName(tr("%1 (channels)").arg(sequence->name()));

    sequence->applyDumpValues(values, -1);
    setStepTimes(quint32(id), QVariantList() << 0, 0, DEFAULT_STEP_HOLD, 0);
    return id;
}

QVariantList EffectEditor::steps(quint32 id) const
{
    QVariantList list;
    Sequence *sequence = qobject_cast<Sequence *>(m_doc->function(id));
    if (sequence == nullptr)
        return list;

    const QList<ChaserStep> chaserSteps = sequence->steps();
    for (int i = 0; i < chaserSteps.count(); i++)
    {
        const ChaserStep &step = chaserSteps.at(i);
        int lit = 0;
        for (const SceneValue &scv : step.values)
        {
            if (scv.value > 0)
                lit++;
        }

        QVariantMap s;
        s.insert("index", i);
        s.insert("fadeIn", step.fadeIn);
        // an infinite hold (QLC+ manual chasers: the step waits for GO) is
        // no time to show (as an int it is -2)
        const bool waits = step.hold == Function::infiniteSpeed();
        s.insert("hold", waits ? 0u : step.hold);
        s.insert("waits", waits);
        s.insert("fadeOut", step.fadeOut);
        s.insert("channels", lit);
        s.insert("note", step.note);
        list.append(s);
    }
    return list;
}

// make sure the hidden scene knows every channel of the programmer. Steps
// whose scene is gone (QLC+ steps bound to a scene of the user, deleted:
// their steps went with it) get a new hidden one. @return false without one
static bool extendBoundScene(Doc *doc, Sequence *sequence, const QList<SceneValue> &values)
{
    Scene *bound = qobject_cast<Scene *>(doc->function(sequence->boundSceneID()));
    if (bound == nullptr)
    {
        // (only for steps that have none left: the steps are all of that scene)
        if (sequence->stepsCount() > 0)
            return false;
        bound = new Scene(doc);
        bound->setVisible(false);
        if (doc->addFunction(bound) == false)
        {
            delete bound;
            return false;
        }
        bound->setName(EffectEditor::tr("%1 (channels)").arg(sequence->name()));
        bound->setStagger(sequence->stagger());
        sequence->setBoundSceneID(bound->id());
    }

    for (const SceneValue &scv : values)
    {
        if (bound->checkValue(scv))
            continue;
        if (bound->fixtures().contains(scv.fxi) == false)
            bound->addFixture(scv.fxi);
        bound->setValue(scv.fxi, scv.channel, 0);
    }
    return true;
}

bool EffectEditor::addStep(quint32 id, int after)
{
    Sequence *sequence = qobject_cast<Sequence *>(m_doc->function(id));
    QList<SceneValue> values = m_programmer->sceneValues();
    if (sequence == nullptr || values.isEmpty())
    {
        if (values.isEmpty())
            emit message(tr("The programmer is empty: set the values of the new step first"), true);
        return false;
    }

    const int count = sequence->stepsCount();
    if (extendBoundScene(m_doc, sequence, values))
        sequence->applyDumpValues(values, -1);
    if (sequence->stepsCount() != count + 1)
    {
        emit message(tr("These steps lost their channels: the step can't be added"), true);
        return false;
    }

    int last = sequence->stepsCount() - 1;
    // new steps take the times of the step they follow
    int reference = (after >= 0 && after < last) ? after : last - 1;
    ChaserStep step = sequence->steps().at(last);
    if (reference >= 0)
    {
        ChaserStep ref = sequence->steps().at(reference);
        step.fadeIn = ref.fadeIn;
        step.hold = ref.hold;
        step.fadeOut = ref.fadeOut;
    }
    else
    {
        step.hold = DEFAULT_STEP_HOLD;
    }
    // speedAdd: an infinite hold stays infinite (a plain + overflows)
    step.duration = Function::speedAdd(step.fadeIn, step.hold);
    sequence->replaceStep(step, last);

    if (after >= 0 && after + 1 < last)
        sequence->moveStep(last, after + 1);

    touch(id);
    return true;
}

bool EffectEditor::updateStep(quint32 id, int index)
{
    Sequence *sequence = qobject_cast<Sequence *>(m_doc->function(id));
    QList<SceneValue> values = m_programmer->sceneValues();
    if (sequence == nullptr || index < 0 || index >= sequence->stepsCount() || values.isEmpty())
        return false;

    extendBoundScene(m_doc, sequence, values);
    sequence->applyDumpValues(values, index);
    touch(id);
    return true;
}

bool EffectEditor::loadStep(quint32 id, int index)
{
    Sequence *sequence = qobject_cast<Sequence *>(m_doc->function(id));
    if (sequence == nullptr || index < 0 || index >= sequence->stepsCount())
        return false;

    m_programmer->loadValues(sequence->steps().at(index).values);
    return true;
}

bool EffectEditor::duplicateStep(quint32 id, int index)
{
    Sequence *sequence = qobject_cast<Sequence *>(m_doc->function(id));
    if (sequence == nullptr || index < 0 || index >= sequence->stepsCount())
        return false;

    sequence->addStep(sequence->steps().at(index), index + 1);
    touch(id);
    return true;
}

bool EffectEditor::removeStep(quint32 id, int index)
{
    Sequence *sequence = qobject_cast<Sequence *>(m_doc->function(id));
    if (sequence == nullptr || sequence->stepsCount() <= 1)
        return false;

    bool ok = sequence->removeStep(index);
    touch(id);
    return ok;
}

bool EffectEditor::moveStep(quint32 id, int from, int to)
{
    Sequence *sequence = qobject_cast<Sequence *>(m_doc->function(id));
    if (sequence == nullptr || to < 0 || to >= sequence->stepsCount())
        return false;

    bool ok = sequence->moveStep(from, to);
    touch(id);
    return ok;
}

void EffectEditor::setStepTimes(quint32 id, const QVariantList &indexes, int fadeIn, int hold, int fadeOut)
{
    Sequence *sequence = qobject_cast<Sequence *>(m_doc->function(id));
    if (sequence == nullptr)
        return;

    for (const QVariant &v : indexes)
    {
        int index = v.toInt();
        if (index < 0 || index >= sequence->stepsCount())
            continue;

        ChaserStep step = sequence->steps().at(index);
        if (fadeIn >= 0)
            step.fadeIn = uint(fadeIn);
        if (hold >= 0)
            step.hold = uint(hold);
        if (fadeOut >= 0)
            step.fadeOut = uint(fadeOut);
        step.duration = Function::speedAdd(step.fadeIn, step.hold);
        sequence->replaceStep(step, index);
    }
    touch(id);
}

void EffectEditor::setStepsWait(quint32 id, const QVariantList &indexes, bool wait)
{
    Sequence *sequence = qobject_cast<Sequence *>(m_doc->function(id));
    if (sequence == nullptr)
        return;

    for (const QVariant &v : indexes)
    {
        const int index = v.toInt();
        if (index < 0 || index >= sequence->stepsCount())
            continue;
        ChaserStep step = sequence->steps().at(index);
        const bool waits = step.hold == Function::infiniteSpeed();
        if (waits == wait)
            continue;
        step.hold = wait ? Function::infiniteSpeed() : DEFAULT_STEP_HOLD;
        step.duration = Function::speedAdd(step.fadeIn, step.hold);
        sequence->replaceStep(step, index);
    }
    touch(id);
}

bool EffectEditor::deleteBoundScene(Doc *doc, quint32 sceneId)
{
    Function *bound = doc->function(sceneId);
    if (bound == nullptr || bound->type() != Function::SceneType)
        return false;
    // the user's scene, or the channels of other steps (Doc::getUsage
    // counts the steps bound to it, racks, super scenes, scripts)
    if (bound->isVisible() || doc->getUsage(sceneId).isEmpty() == false)
        return false;
    // Doc::deleteFunction doesn't stop it: the timer would keep it
    bound->stopAndWait();
    return doc->deleteFunction(sceneId);
}

/*********************************************************************
 * Move FX
 *********************************************************************/

QStringList EffectEditor::moveShapes() const
{
    return EFX::algorithmList();
}

QList<quint32> EffectEditor::selectedWithPanTilt() const
{
    QList<quint32> ids;
    for (quint32 fid : m_programmer->selectionIds())
    {
        Fixture *fxi = m_doc->fixture(fid);
        if (fxi == nullptr)
            continue;
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch != nullptr && (ch->group() == QLCChannel::Pan || ch->group() == QLCChannel::Tilt))
            {
                ids.append(fid);
                break;
            }
        }
    }
    return ids;
}

int EffectEditor::createMoveFx(const QString &name, int zoneIndex)
{
    QList<quint32> fixtures = selectedWithPanTilt();
    if (fixtures.isEmpty())
    {
        emit message(tr("Select moving heads or scanners in the programmer first"), true);
        return -1;
    }

    EFX *efx = new EFX(m_doc);
    for (quint32 fid : fixtures)
        efx->addFixture(fid, 0);
    efx->setDuration(4000);
    return addNew(efx, name, zoneIndex);
}

void EffectEditor::setMoveFx(quint32 id, const QString &key, const QVariant &value)
{
    EFX *efx = qobject_cast<EFX *>(m_doc->function(id));
    if (efx == nullptr)
        return;

    if (key == "shape")
        efx->setAlgorithm(EFX::stringToAlgorithm(value.toString()));
    else if (key == "width")
        efx->setWidth(value.toInt());
    else if (key == "height")
        efx->setHeight(value.toInt());
    else if (key == "rotation")
        efx->setRotation(value.toInt());
    else if (key == "xOffset")
        efx->setXOffset(value.toInt());
    else if (key == "yOffset")
        efx->setYOffset(value.toInt());
    else if (key == "phase")
        efx->setStartOffset(value.toInt());
    else if (key == "spread")
        efx->setPropagationMode(EFX::PropagationMode(qBound(0, value.toInt(), 2)));
    else if (key == "relative")
        efx->setIsRelative(value.toBool());
    else if (key == "xFrequency")
        efx->setXFrequency(value.toInt());
    else if (key == "yFrequency")
        efx->setYFrequency(value.toInt());
    else if (key == "dimmer")
        efx->setDimmerControlEnabled(value.toBool());
    else if (key == "symmetric")
        efx->setSymmetric(value.toBool());
    else if (key == "phaseSpread")
        efx->setPhaseSpread(value.toInt());
    else
        return;

    touch(id);
}

QVariantList EffectEditor::movePreview(quint32 id) const
{
    QVariantList points;
    EFX *efx = qobject_cast<EFX *>(m_doc->function(id));
    if (efx == nullptr)
        return points;

    QPolygonF polygon;
    efx->preview(polygon);
    for (const QPointF &p : std::as_const(polygon))
        points.append(QPointF(p.x() / 255.0, p.y() / 255.0));
    return points;
}

/*********************************************************************
 * Colour FX
 *********************************************************************/

QStringList EffectEditor::colourEffects() const
{
    return RGBAlgorithm::algorithms(m_doc);
}

quint32 EffectEditor::makeGroup(const QString &name, const QList<quint32> &fixtures)
{
    // one row, in selection order: the order the effect runs along
    int heads = 0;
    for (quint32 fid : fixtures)
    {
        Fixture *fxi = m_doc->fixture(fid);
        if (fxi != nullptr)
            heads += qMax(1, fxi->heads());
    }

    FixtureGroup *group = new FixtureGroup(m_doc);
    group->setName(name);
    group->setSize(QSize(qMax(1, heads), 1));
    if (m_doc->addFixtureGroup(group) == false)
    {
        delete group;
        return FixtureGroup::invalidId();
    }

    int x = 0;
    for (quint32 fid : fixtures)
    {
        Fixture *fxi = m_doc->fixture(fid);
        if (fxi == nullptr)
            continue;
        group->assignFixture(fid, QLCPoint(x, 0));
        x += qMax(1, fxi->heads());
    }
    return group->id();
}

int EffectEditor::createColourFx(const QString &name, int zoneIndex)
{
    QList<quint32> fixtures = m_programmer->selectionIds();
    if (fixtures.isEmpty())
    {
        emit message(tr("Select the fixtures of the effect in the programmer first"), true);
        return -1;
    }

    RGBMatrix *matrix = new RGBMatrix(m_doc);
    int id = addNew(matrix, name, zoneIndex);
    if (id < 0)
        return -1;

    matrix->setFixtureGroup(makeGroup(tr("%1 (FX)").arg(matrix->name()), fixtures));

    RGBAlgorithm *algo = RGBAlgorithm::algorithm(m_doc, DEFAULT_COLOUR_FX);
    if (algo == nullptr)
        algo = RGBAlgorithm::algorithm(m_doc, RGBAlgorithm::algorithms(m_doc).value(0));
    matrix->setColor(0, QColor(Qt::red));
    matrix->setColor(1, QColor(Qt::blue));
    if (algo != nullptr)
    {
        algo->setColors(QVector<QColor>() << matrix->getColor(0) << matrix->getColor(1)
                                          << QColor() << QColor() << QColor());
        matrix->setAlgorithm(algo);
    }
    matrix->setDuration(500);
    return id;
}

void EffectEditor::setColourFx(quint32 id, const QString &key, const QVariant &value)
{
    RGBMatrix *matrix = qobject_cast<RGBMatrix *>(m_doc->function(id));
    if (matrix == nullptr)
        return;

    if (key == "effect")
    {
        RGBAlgorithm *algo = RGBAlgorithm::algorithm(m_doc, value.toString());
        if (algo == nullptr)
            return;
        QVector<QColor> colors;
        for (int i = 0; i < RGBAlgorithmColorDisplayCount; i++)
            colors.append(matrix->getColor(i));
        algo->setColors(colors);
        matrix->setAlgorithm(algo);
    }
    else if (key.startsWith("color"))
    {
        int index = key.mid(5).toInt();
        if (index < 0 || index >= RGBAlgorithmColorDisplayCount)
            return;
        QColor color(value.toString());
        if (color.isValid() && matrix->controlMode() != RGBMatrix::ControlModeRgb)
        {
            // single colour modes work on brightness
            int gray = qGray(color.rgb());
            color = QColor(gray, gray, gray);
        }
        matrix->setColor(index, color);
    }
    else if (key == "mode")
    {
        matrix->setControlMode(RGBMatrix::ControlMode(qBound(0, value.toInt(), int(RGBMatrix::ControlModeShutter))));
    }
    else if (key == "text")
    {
        if (matrix->algorithm() == nullptr || matrix->algorithm()->type() != RGBAlgorithm::Text)
            return;
        {
            // (the timer renders it meanwhile)
            QMutexLocker locker(&matrix->algorithmMutex());
            static_cast<RGBText *>(matrix->algorithm())->setText(value.toString());
        }
        // another text, another number of steps (its width)
        matrix->setFixtureGroup(matrix->fixtureGroup());
    }
    else if (key == "dimmerControl")
    {
        matrix->setDimmerControl(value.toBool());
    }
    else if (key == "layout")
    {
        // the same fixtures, in one row or like the 2D plan
        QList<quint32> fixtures = groupFixtures(matrix->fixtureGroup());
        if (fixtures.isEmpty())
            return;
        replaceGroup(id, value.toString() == "plan" ? makePlanGroup(matrix->name(), fixtures)
                                                    : makeGroup(tr("%1 (FX)").arg(matrix->name()), fixtures));
    }
    else if (key == "image" || key == "animation")
    {
        if (matrix->algorithm() == nullptr || matrix->algorithm()->type() != RGBAlgorithm::Image)
            return;
        RGBImage *image = static_cast<RGBImage *>(matrix->algorithm());
        {
            // (the timer renders it meanwhile)
            QMutexLocker locker(&matrix->algorithmMutex());
            if (key == "image")
            {
                QUrl url(value.toString());
                image->setFilename(url.isLocalFile() ? url.toLocalFile() : value.toString());
            }
            else
            {
                image->setAnimationStyle(RGBImage::stringToAnimationStyle(value.toString()));
            }
        }
        // the steps follow the image and how it moves (its width, its height, 1)
        matrix->setFixtureGroup(matrix->fixtureGroup());
    }
    else
    {
        return;
    }
    touch(id);
}

void EffectEditor::setColourFxProperty(quint32 id, const QString &name, const QString &value)
{
    RGBMatrix *matrix = qobject_cast<RGBMatrix *>(m_doc->function(id));
    if (matrix == nullptr)
        return;

    matrix->setProperty(name, value);
    touch(id);
}

bool EffectEditor::useSelection(quint32 id)
{
    Function *f = m_doc->function(id);

    if (EFX *efx = qobject_cast<EFX *>(f))
    {
        QList<quint32> fixtures = selectedWithPanTilt();
        if (fixtures.isEmpty())
        {
            emit message(tr("The selection has no fixtures with pan/tilt"), true);
            return false;
        }
        // not while the timer goes through its heads: a running effect stops
        // and starts again with the new ones (each with its place in the
        // spread), for whoever played it, at their level. The heads it lets
        // go are deleted (the EFX doesn't)
        const QList<FunctionParent> parents = efx->interruptAndWait();
        for (EFXFixture *ef : efx->fixtures())
            if (efx->removeFixture(ef))
                delete ef;
        for (quint32 fid : fixtures)
            efx->addFixture(fid, 0);
        efx->resume(m_doc->masterTimer(), parents);
        touch(id);
        return true;
    }

    if (RGBMatrix *matrix = qobject_cast<RGBMatrix *>(f))
    {
        QList<quint32> fixtures = m_programmer->selectionIds();
        if (fixtures.isEmpty())
            return false;

        FixtureGroup *current = m_doc->fixtureGroup(matrix->fixtureGroup());
        bool plan = current != nullptr && current->name().endsWith(" (FX 2D)");
        replaceGroup(id, plan ? makePlanGroup(matrix->name(), fixtures) : makeGroup(tr("%1 (FX)").arg(matrix->name()), fixtures));
        touch(id);
        return true;
    }

    if (CurveFX *curve = qobject_cast<CurveFX *>(f))
    {
        QList<quint32> fixtures = m_programmer->selectionIds();
        if (fixtures.isEmpty())
            return false;
        curve->setFixtures(fixtures);
        touch(id);
        return true;
    }

    return false;
}

void EffectEditor::replaceGroup(quint32 matrixId, quint32 newGroup)
{
    RGBMatrix *matrix = qobject_cast<RGBMatrix *>(m_doc->function(matrixId));
    if (matrix == nullptr || newGroup == FixtureGroup::invalidId())
        return;

    quint32 oldGroup = matrix->fixtureGroup();
    matrix->setFixtureGroup(newGroup);
    if (oldGroup == newGroup)
        return;

    // the old group was made for this effect: drop it if nothing else uses it
    bool shared = false;
    for (Function *other : m_doc->functionsByType(Function::RGBMatrixType))
    {
        RGBMatrix *m = qobject_cast<RGBMatrix *>(other);
        if (m != matrix && m != nullptr && m->fixtureGroup() == oldGroup)
            shared = true;
    }
    FixtureGroup *old = m_doc->fixtureGroup(oldGroup);
    if (shared == false && old != nullptr && (old->name().endsWith(" (FX)") || old->name().endsWith(" (FX 2D)")))
        m_doc->deleteFixtureGroup(oldGroup);
}

/*********************************************************************
 * Effect rack
 *********************************************************************/

int EffectEditor::createRack(const QString &name, const QVariantList &functionIds, int zoneIndex)
{
    Collection *rack = new Collection(m_doc);
    int id = addNew(rack, name, zoneIndex);
    if (id < 0)
        return -1;

    for (const QVariant &v : functionIds)
        rack->addFunction(v.toUInt());
    touch(quint32(id));
    return id;
}

void EffectEditor::addToRack(quint32 id, quint32 functionId)
{
    Collection *rack = qobject_cast<Collection *>(m_doc->function(id));
    Function *member = m_doc->function(functionId);
    if (rack == nullptr || functionId == id || member == nullptr)
        return;
    // a rack (or steps, a super scene) that plays this one: it would play itself
    if (member->contains(id))
    {
        emit message(tr("“%1” plays this rack: it can't go into it").arg(member->name()), true);
        return;
    }

    if (rack->addFunction(functionId))
        touch(id);
}

void EffectEditor::removeFromRack(quint32 id, quint32 functionId)
{
    Collection *rack = qobject_cast<Collection *>(m_doc->function(id));
    if (rack != nullptr && rack->removeFunction(functionId))
        touch(id);
}

QVariantList EffectEditor::playersOf(quint32 id) const
{
    QVariantList players;
    for (Function *f : m_doc->functions())
        if (f->id() != id && f->contains(id))
            players.append(f->id());
    return players;
}

/*********************************************************************
 * Mappings: colour effects on the 2D plan
 *********************************************************************/

quint32 EffectEditor::makePlanGroup(const QString &name, const QList<quint32> &fixtures)
{
    // where the fixtures stand on the plan (mm), each head next to the previous one
    struct Item { quint32 fid; int heads; qreal x; qreal y; };
    QList<Item> items;
    MonitorProperties *monitor = m_doc->monitorProperties();
    int headCount = 0;
    for (quint32 fid : fixtures)
    {
        Fixture *fxi = m_doc->fixture(fid);
        if (fxi == nullptr)
            continue;
        QVector3D pos = monitor != nullptr && monitor->containsFixture(fid)
                ? monitor->fixturePosition(fid, 0, 0) : QVector3D(headCount * 500, 0, 0);
        items.append({ fid, qMax(1, fxi->heads()), pos.x(), pos.y() });
        headCount += qMax(1, fxi->heads());
    }
    if (items.isEmpty())
        return FixtureGroup::invalidId();

    qreal minX = items.first().x, maxX = minX, minY = items.first().y, maxY = minY;
    for (const Item &it : std::as_const(items))
    {
        minX = qMin(minX, it.x); maxX = qMax(maxX, it.x);
        minY = qMin(minY, it.y); maxY = qMax(maxY, it.y);
    }
    qreal spanX = qMax(1.0, maxX - minX), spanY = qMax(1.0, maxY - minY);

    // a grid fine enough for each fixture to get its own cell
    int cols = qBound(4, headCount * 2, 64);
    int rows = qBound(1, qRound(cols * spanY / spanX), 64);
    if (maxY - minY < 1.0)
        rows = 1;

    QSet<QPair<int, int>> taken;
    QList<QPair<QLCPoint, GroupHead>> placed;
    int widest = cols;
    for (const Item &it : std::as_const(items))
    {
        int cx = qRound((it.x - minX) / spanX * (cols - 1));
        int cy = rows > 1 ? qRound((it.y - minY) / spanY * (rows - 1)) : 0;

        // the nearest free cell, looking around. Every cell right of the
        // widest row is free: the search always ends there, at the latest
        // (a fixture never lands on the heads of another one)
        bool found = false;
        for (int radius = 0; found == false; radius++)
        {
            // (the rows of the grid; the cells inside the ring didn't fit)
            for (int dy = qMax(-radius, -cy); dy <= qMin(radius, rows - 1 - cy) && found == false; dy++)
            {
                for (int dx = -radius; dx <= radius && found == false; dx++)
                {
                    int x = cx + dx, y = cy + dy;
                    if (x < 0 || (qAbs(dx) != radius && qAbs(dy) != radius))
                        continue;
                    bool fits = true;
                    for (int h = 0; h < it.heads; h++)
                        fits = fits && taken.contains(qMakePair(x + h, y)) == false;
                    if (fits)
                    {
                        cx = x;
                        cy = y;
                        found = true;
                    }
                }
            }
        }
        for (int h = 0; h < it.heads; h++)
        {
            taken.insert(qMakePair(cx + h, cy));
            placed.append(qMakePair(QLCPoint(cx + h, cy), GroupHead(it.fid, h)));
            widest = qMax(widest, cx + h + 1);
        }
    }

    FixtureGroup *group = new FixtureGroup(m_doc);
    group->setName(name + " (FX 2D)");
    group->setSize(QSize(widest, rows));
    if (m_doc->addFixtureGroup(group) == false)
    {
        delete group;
        return FixtureGroup::invalidId();
    }
    for (const auto &p : std::as_const(placed))
        group->assignHead(p.first, p.second);
    return group->id();
}

QList<quint32> EffectEditor::groupFixtures(quint32 groupId) const
{
    QList<quint32> list;
    FixtureGroup *group = m_doc->fixtureGroup(groupId);
    if (group == nullptr)
        return list;

    // row by row, left to right
    QMap<QLCPoint, GroupHead> heads = group->headsMap();
    QList<QLCPoint> points = heads.keys();
    std::sort(points.begin(), points.end(), [](const QLCPoint &a, const QLCPoint &b) {
        return a.y() != b.y() ? a.y() < b.y() : a.x() < b.x();
    });
    for (const QLCPoint &pt : std::as_const(points))
        if (list.contains(heads.value(pt).fxi) == false)
            list.append(heads.value(pt).fxi);
    return list;
}

int EffectEditor::createMapping(const QString &name, int zoneIndex)
{
    int id = createColourFx(name.trimmed().isEmpty() ? uniqueName(tr("Mapping %1")) : name, zoneIndex);
    if (id < 0)
        return id;

    RGBMatrix *matrix = qobject_cast<RGBMatrix *>(m_doc->function(quint32(id)));
    replaceGroup(quint32(id), makePlanGroup(matrix->name(), m_programmer->selectionIds()));
    touch(quint32(id));
    return id;
}

/*********************************************************************
 * Curve FX
 *********************************************************************/

QStringList EffectEditor::curveWaveforms() const
{
    return CurveFX::waveformNames();
}

int EffectEditor::createCurveFx(const QString &name, int zoneIndex)
{
    QList<quint32> fixtures = m_programmer->selectionIds();
    if (fixtures.isEmpty())
    {
        emit message(tr("Select the fixtures of the effect in the programmer first"), true);
        return -1;
    }

    // the dimmer by default, or the first intensity channel
    QString feature;
    for (quint32 fid : std::as_const(fixtures))
    {
        Fixture *fxi = m_doc->fixture(fid);
        for (quint32 c = 0; fxi != nullptr && c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch == nullptr)
                continue;
            if (ch->name().compare("dimmer", Qt::CaseInsensitive) == 0)
            {
                feature = "dimmer";
                break;
            }
            if (feature.isEmpty() && ch->group() == QLCChannel::Intensity)
                feature = ch->name().toLower();
        }
        if (feature == "dimmer")
            break;
    }

    CurveFX *curve = new CurveFX(m_doc);
    curve->setFixtures(fixtures);
    if (feature.isEmpty() == false)
        curve->setFeatures(QStringList() << feature);
    if (fixtures.count() > 1)
        curve->setSpread(360);
    return addNew(curve, name, zoneIndex);
}

int EffectEditor::createSuperScene(const QString &name, int zoneIndex)
{
    Show *show = new Show(m_doc);
    // blocks in milliseconds, whatever the tempo of their scenes
    show->setMsTimeline(true);
    int id = addNew(show, name.isEmpty() ? tr("Super scene") : name, zoneIndex);
    if (id < 0)
        return id;

    for (int i = 1; i <= 2; i++)
    {
        Track *track = new Track(Function::invalidId(), show);
        track->setName(tr("Track %1").arg(i));
        show->addTrack(track);
    }
    touch(quint32(id));
    return id;
}

int EffectEditor::createStrobe(const QString &name, int zoneIndex)
{
    int id = createCurveFx(name.isEmpty() ? tr("Strobe") : name, zoneIndex);
    CurveFX *curve = qobject_cast<CurveFX *>(m_doc->function(quint32(qMax(0, id))));
    if (id < 0 || curve == nullptr)
        return id;

    // short flashes, all together, 10 times a second
    curve->setWaveform(CurveFX::Strobe);
    curve->setDuty(20);
    curve->setSpread(0);
    curve->setDuration(100);
    touch(quint32(id));
    return id;
}

void EffectEditor::setCurveFx(quint32 id, const QString &key, const QVariant &value)
{
    CurveFX *curve = qobject_cast<CurveFX *>(m_doc->function(id));
    if (curve == nullptr)
        return;

    if (key == "wave")
        curve->setWaveform(CurveFX::stringToWaveform(value.toString()));
    else if (key == "low")
        curve->setLow(value.toInt());
    else if (key == "high")
        curve->setHigh(value.toInt());
    else if (key == "phase")
        curve->setPhaseOffset(value.toInt());
    else if (key == "spread")
        curve->setSpread(value.toInt());
    else if (key == "duty")
        curve->setDuty(value.toInt());
    else if (key == "relative")
        curve->setRelative(value.toBool());
    else if (key == "features")
        curve->setFeatures(value.toStringList());
    else
        return;
    touch(id);
}

QVariantList EffectEditor::curveFeatures(quint32 id) const
{
    QVariantList list;
    CurveFX *curve = qobject_cast<CurveFX *>(m_doc->function(id));
    if (curve == nullptr)
        return list;

    QStringList seen;
    for (quint32 fid : curve->fixtures())
    {
        Fixture *fxi = m_doc->fixture(fid);
        for (quint32 c = 0; fxi != nullptr && c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch == nullptr || seen.contains(ch->name().toLower()))
                continue;
            seen.append(ch->name().toLower());
            QVariantMap item;
            item.insert("key", ch->name().toLower());
            item.insert("name", ch->name());
            item.insert("selected", curve->features().contains(ch->name().toLower()));
            list.append(item);
        }
    }
    return list;
}

QVariantList EffectEditor::curvePreview(quint32 id) const
{
    QVariantList lines;
    CurveFX *curve = qobject_cast<CurveFX *>(m_doc->function(id));
    if (curve == nullptr)
        return lines;

    int count = qMax(1, int(curve->fixtures().count()));
    for (int index = 0; index < qMin(count, 8); index++)
    {
        QVariantList points;
        for (int i = 0; i < 64; i++)
            points.append(curve->valueAt(qreal(i) / 63.0, index, count));
        lines.append(QVariant(points));
    }
    return lines;
}

/*********************************************************************
 * Playback settings
 *********************************************************************/

void EffectEditor::setPlayback(quint32 id, const QString &key, const QVariant &value)
{
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return;

    if (key == "loops")
    {
        // the loops replace the old "Play once"
        if (f->runOrder() == Function::SingleShot)
            f->setRunOrder(Function::Loop);
        f->setLoopCount(qMax(0, value.toInt()));
    }
    else if (key == "endAction")
    {
        f->setEndAction(value.toString() == "pause" ? Function::EndPause : Function::EndRelease);
    }
    else if (key == "jumpTo")
    {
        f->setJumpTo(value.toInt());
    }
    else if (key == "drive")
    {
        QString drive = value.toString();
        f->setTempoType(drive == "bpm" ? Function::Beats : Function::Time);
        f->setDriveMode(drive == "beatgo" ? Function::DriveBeatGO
                        : drive == "pulse" ? Function::DrivePulse : Function::DriveOff);
    }
    else if (key == "divisions")
    {
        f->setDivisions(value.toInt());
    }
    else if (key == "tap")
    {
        f->setTapPeriod(uint(qMax(0, value.toInt())));
    }
    else if (key == "startMode")
    {
        f->setStartMode(value.toString() == "paused" ? Function::StartPaused : Function::StartPlaying);
    }
    else if (key == "stagger")
    {
        f->setStagger(uint(qMax(0, value.toInt())));
        // the steps fade through their channels scene
        if (Sequence *sequence = qobject_cast<Sequence *>(f))
        {
            if (Function *bound = m_doc->function(sequence->boundSceneID()))
                bound->setStagger(f->stagger());
        }
    }
    else
    {
        return;
    }
    touch(id);
}

/*********************************************************************
 * Export, import, convert
 *********************************************************************/

bool EffectEditor::exportEffect(quint32 id, const QUrl &url)
{
    QString error;
    QString path = url.isLocalFile() ? url.toLocalFile() : url.toString();
    if (EffectExchange::exportToFile(m_doc, id, path, &error) == false)
    {
        emit message(error.isEmpty() ? tr("Unable to export the effect") : error, true);
        return false;
    }
    emit message(tr("Saved to %1").arg(QFileInfo(path).fileName()), false);
    return true;
}

int EffectEditor::importEffect(const QUrl &url, int zoneIndex)
{
    QString error;
    int missing = 0;
    QString path = url.isLocalFile() ? url.toLocalFile() : url.toString();
    quint32 id = EffectExchange::importFromFile(m_doc, path, &error, &missing);
    if (id == Function::invalidId())
    {
        emit message(error.isEmpty() ? tr("Unable to import the effect") : error, true);
        return -1;
    }

    if (Zone *zone = m_zones->zone(zoneIndex))
        zone->addFunction(id);
    if (missing > 0)
        emit message(tr("%n fixture(s) of the file are not in this show: their part was left out", "", missing), false);
    touch(id);
    return int(id);
}

int EffectEditor::convertToSteps(quint32 id, int stepMs)
{
    QString error;
    quint32 steps = EffectExchange::renderToSteps(m_doc, id, uint(qMax(20, stepMs)), &error);
    if (steps == Function::invalidId())
    {
        emit message(error.isEmpty() ? tr("Unable to convert the effect") : error, true);
        return -1;
    }
    if (EffectExchange::cycleDuration(m_doc, id) > EffectExchange::MaxRenderCycle)
        emit message(tr("The cycle of the effect is longer than %n minute(s): its first %n minute(s) were recorded", "",
                        int(EffectExchange::MaxRenderCycle / 60000)), false);
    touch(steps);
    return int(steps);
}

/*********************************************************************
 * Live view: what the effect sends, fixture by fixture
 *********************************************************************/

QVariantMap EffectEditor::liveView(quint32 id) const
{
    QVariantMap view;
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return view;

    view.insert("running", f->isRunning());
    QVariantList items;

    QList<Universe *> universes = m_doc->inputOutputMap()->claimUniverses();

    // a channel of a fixture as it leaves the software, 0..1
    auto level = [&](Fixture *fxi, quint32 channel) -> qreal
    {
        if (fxi == nullptr || channel == QLCChannel::invalid() || fxi->universe() >= quint32(universes.count()))
            return 0.0;
        return qreal(universes.at(int(fxi->universe()))->preGMValue(int(fxi->address() + channel))) / 255.0;
    };
    // the first channel of a group (pan, tilt) with its fine channel, 0..1
    auto position = [&](Fixture *fxi, QLCChannel::Group group) -> qreal
    {
        quint32 msb = QLCChannel::invalid(), lsb = QLCChannel::invalid();
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch == nullptr || ch->group() != group)
                continue;
            if (ch->controlByte() == QLCChannel::MSB && msb == QLCChannel::invalid())
                msb = c;
            else if (ch->controlByte() == QLCChannel::LSB && lsb == QLCChannel::invalid())
                lsb = c;
        }
        if (msb == QLCChannel::invalid())
            return -1.0;
        // the values before the modifiers: an inverted axis (Patch) still shows the point of the shape
        return level(fxi, msb) + (lsb != QLCChannel::invalid() ? level(fxi, lsb) / 255.0 : 0.0);
    };
    // the colour of a head: its RGB (or white level) scaled by the dimmer
    auto headColour = [&](Fixture *fxi, int head) -> QColor
    {
        qreal dim = 1.0;
        quint32 master = fxi->masterIntensityChannel();
        if (master != QLCChannel::invalid())
            dim = level(fxi, master);
        QVector<quint32> rgb = fxi->rgbChannels(head);
        if (rgb.count() == 3)
            return QColor::fromRgbF(float(level(fxi, rgb.at(0)) * dim), float(level(fxi, rgb.at(1)) * dim),
                                    float(level(fxi, rgb.at(2)) * dim));
        QVector<quint32> cmy = fxi->cmyChannels(head);
        if (cmy.count() == 3)
            return QColor::fromRgbF(float((1.0 - level(fxi, cmy.at(0))) * dim), float((1.0 - level(fxi, cmy.at(1))) * dim),
                                    float((1.0 - level(fxi, cmy.at(2))) * dim));
        quint32 intensity = fxi->head(head).channelNumber(QLCChannel::Intensity, QLCChannel::MSB);
        qreal v = intensity != QLCChannel::invalid() ? level(fxi, intensity) : dim;
        return QColor::fromRgbF(float(v), float(v), float(v));
    };

    if (EFX *efx = qobject_cast<EFX *>(f))
    {
        // where each head points, and where its path starts
        QVector<QPolygonF> paths;
        efx->previewFixtures(paths);
        int index = 0;
        for (EFXFixture *ef : efx->fixtures())
        {
            Fixture *fxi = m_doc->fixture(ef->head().fxi);
            if (fxi == nullptr)
                continue;
            QVariantMap item;
            item.insert("index", ++index);
            item.insert("name", fxi->name());
            item.insert("pan", position(fxi, QLCChannel::Pan));
            item.insert("tilt", position(fxi, QLCChannel::Tilt));
            QPointF start = index - 1 < paths.count() && paths.at(index - 1).isEmpty() == false
                    ? paths.at(index - 1).first() : QPointF(127, 127);
            item.insert("startX", start.x() / 255.0);
            item.insert("startY", start.y() / 255.0);
            items.append(item);
        }
    }
    else if (RGBMatrix *matrix = qobject_cast<RGBMatrix *>(f))
    {
        // every head where it sits in the effect, with its colour
        FixtureGroup *group = m_doc->fixtureGroup(matrix->fixtureGroup());
        if (group != nullptr)
        {
            view.insert("cols", group->size().width());
            view.insert("rows", group->size().height());
            QList<quint32> order = groupFixtures(group->id());
            QMap<QLCPoint, GroupHead> heads = group->headsMap();
            for (auto it = heads.cbegin(); it != heads.cend(); ++it)
            {
                Fixture *fxi = m_doc->fixture(it.value().fxi);
                if (fxi == nullptr)
                    continue;
                QVariantMap item;
                item.insert("index", int(order.indexOf(fxi->id())) + 1);
                item.insert("name", fxi->name());
                item.insert("x", it.key().x());
                item.insert("y", it.key().y());
                item.insert("color", headColour(fxi, it.value().head).name());
                items.append(item);
            }
        }
    }
    else if (CurveFX *curve = qobject_cast<CurveFX *>(f))
    {
        // the level of the first driven channel of each fixture
        int index = 0;
        for (quint32 fid : curve->fixtures())
        {
            Fixture *fxi = m_doc->fixture(fid);
            if (fxi == nullptr)
                continue;
            qreal value = 0.0;
            for (quint32 c = 0; c < fxi->channels(); c++)
            {
                const QLCChannel *ch = fxi->channel(c);
                if (ch != nullptr && curve->features().contains(ch->name().toLower()))
                {
                    value = level(fxi, c);
                    break;
                }
            }
            QVariantMap item;
            item.insert("index", ++index);
            item.insert("name", fxi->name());
            item.insert("level", value);
            items.append(item);
        }
    }

    m_doc->inputOutputMap()->releaseUniverses(false);
    view.insert("items", items);
    return view;
}

/*********************************************************************
 * FX
 *********************************************************************/

void EffectEditor::setFxLayer(quint32 id, int index, const QVariantMap &props)
{
    FXScene *fx = qobject_cast<FXScene *>(m_doc->function(id));
    if (fx == nullptr)
        return;
    QList<FXScene::Layer> layers = fx->layers();
    if (index < 0 || index >= layers.count())
        return;

    const bool wasLinked = layers.at(index).linked;
    QVariantMap changes = props;
    changes.remove("param");
    changes.remove("fixtures");
    layers[index] = FxTools::layerFromMap(changes, layers.at(index));
    // for all its fixtures: the ones that did it their own way do it like the others
    FxTools::clearOwn(layers[index], changes.keys());

    // linked layers share the speed and the timing
    if (layers.at(index).linked)
    {
        QVariantMap timing;
        int from = -1;
        if (wasLinked == false)
        {
            for (int i = 0; i < layers.count() && from < 0; i++)
                if (i != index && layers.at(i).linked)
                    from = i;
            if (from >= 0)
            {
                QVariantMap theirs = FxTools::layerToMap(layers.at(from));
                for (const QString &key : FxTools::timingKeys())
                    timing.insert(key, theirs.value(key));
                layers[index] = FxTools::layerFromMap(timing, layers.at(index));
            }
        }
        else
        {
            for (const QString &key : FxTools::timingKeys())
                if (changes.contains(key))
                    timing.insert(key, changes.value(key));
            for (int i = 0; i < layers.count(); i++)
                if (i != index && layers.at(i).linked)
                    layers[i] = FxTools::layerFromMap(timing, layers.at(i));
        }
    }

    fx->setLayers(layers);
    m_doc->setModified();
    emit edited(id);
}

void EffectEditor::removeFxLayer(quint32 id, int index)
{
    FXScene *fx = qobject_cast<FXScene *>(m_doc->function(id));
    if (fx == nullptr)
        return;
    QList<FXScene::Layer> layers = fx->layers();
    if (index < 0 || index >= layers.count())
        return;
    layers.removeAt(index);
    fx->setLayers(layers);
    m_doc->setModified();
    emit edited(id);
}

QVariantList EffectEditor::fxPreview(quint32 id, int index, qreal cyclePos) const
{
    FXScene *fx = qobject_cast<FXScene *>(m_doc->function(id));
    return fx != nullptr ? fx->previewLayer(index, cyclePos) : QVariantList();
}

QVariantList EffectEditor::fxModeCurve(int mode, int samples) const
{
    return FxTools::modeCurve(mode, samples);
}

QVariantList EffectEditor::fxFigurePath(int figure, int samples) const
{
    return FxTools::figurePath(figure, samples);
}

bool EffectEditor::convertToFx(quint32 id)
{
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return false;
    FXScene *fx = FxTools::convert(m_doc, f);
    if (fx == nullptr)
        return false;

    // (the groups, playbacks and parents that played it let it go: the
    // effect tried here plays on, converted)
    const QList<FunctionParent> parents = f->interruptAndWait();
    if (m_doc->replaceFunction(id, fx) == false)
    {
        delete fx;
        f->resume(m_doc->masterTimer(), parents);
        return false;
    }
    if (parents.count() == 1 && parents.first() == FunctionParent::master())
        fx->start(m_doc->masterTimer(), FunctionParent::master());
    emit edited(id);
    return true;
}
