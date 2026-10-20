/*
  DMX Total Control
  effectexchange.cpp

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
#include <QCoreApplication>
#include <QFileInfo>
#include <QBuffer>
#include <QFile>
#include <QHash>
#include <QSet>
#include <QDir>
#include <functional>

#include "effectexchange.h"

#include "qlcfixturedefcache.h"
#include "monitorproperties.h"
#include "rgbscriptscache.h"
#include "inputoutputmap.h"
#include "functionparent.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "fixturegroup.h"
#include "mastertimer.h"
#include "efxfixture.h"
#include "collection.h"
#include "qlcchannel.h"
#include "rgbmatrix.h"
#include "rgbscriptv4.h"
#include "universe.h"
#include "sequence.h"
#include "cuelist.h"
#include "curvefx.h"
#include "fxscene.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"

#define KXMLRoot      QStringLiteral("DMXTotalControlEffects")
#define KXMLFixtures  QStringLiteral("Fixtures")
#define KXMLGroups    QStringLiteral("FixtureGroups")
#define KXMLFunctions QStringLiteral("Functions")
#define KXMLWorkspace QStringLiteral("Workspace")

namespace
{

/** What the fixtures of a layer do their own way, on the fixtures of this
 *  show (the ones with no match go) */
QMap<quint32, FXScene::Own> mapOwn(const QMap<quint32, FXScene::Own> &own, const QHash<quint32, quint32> &fixtureMap)
{
    QMap<quint32, FXScene::Own> mapped;
    for (auto it = own.cbegin(); it != own.cend(); ++it)
        if (fixtureMap.contains(it.key()))
            mapped.insert(fixtureMap.value(it.key()), it.value());
    return mapped;
}

/** $id and what it plays, the functions it needs first. $seen: the functions
 *  already met, so racks in a cycle (A in B in A) don't recurse forever */
void collect(Doc *doc, quint32 id, QList<quint32> &functions, QList<quint32> &groups, QList<quint32> &fixtures,
             QSet<quint32> &seen)
{
    if (functions.contains(id) || seen.contains(id))
        return;
    Function *f = doc->function(id);
    if (f == nullptr)
        return;
    seen.insert(id);

    auto addFixture = [&](quint32 fid) { if (fixtures.contains(fid) == false) fixtures.append(fid); };

    if (Collection *rack = qobject_cast<Collection *>(f))
    {
        for (quint32 member : rack->functions())
            collect(doc, member, functions, groups, fixtures, seen);
    }
    else if (Sequence *sequence = qobject_cast<Sequence *>(f))
    {
        collect(doc, sequence->boundSceneID(), functions, groups, fixtures, seen);
        for (const ChaserStep &step : sequence->steps())
            for (const SceneValue &scv : step.values)
                addFixture(scv.fxi);
    }
    else if (Chaser *chaser = qobject_cast<Chaser *>(f))
    {
        for (const ChaserStep &step : chaser->steps())
            collect(doc, step.fid, functions, groups, fixtures, seen);
    }
    else if (Scene *scene = qobject_cast<Scene *>(f))
    {
        for (quint32 fid : scene->fixtures())
            addFixture(fid);
        for (const SceneValue &scv : scene->values())
            addFixture(scv.fxi);
    }
    else if (FXScene *fx = qobject_cast<FXScene *>(f))
    {
        for (quint32 fid : fx->fixtures())
            addFixture(fid);
    }
    else if (Cuelist *cuelist = qobject_cast<Cuelist *>(f))
    {
        // every fixture a cue sets, releases, moves or stops (in ID order:
        // the export doesn't change from one time to the next)
        QSet<quint32> ids;
        const QList<Cuelist::Cue> cues = cuelist->cues();
        for (const Cuelist::Cue &cue : cues)
        {
            for (auto it = cue.values.cbegin(); it != cue.values.cend(); ++it)
                ids.insert(it.key().first);
            for (const Cuelist::Key &key : cue.releases)
                ids.insert(key.first);
            for (const FXScene::Layer &layer : cue.fx)
                for (quint32 fid : layer.fixtures)
                    ids.insert(fid);
            for (const Cuelist::FxKey &key : cue.fxReleases)
                ids.insert(key.second);
        }
        QList<quint32> sorted(ids.cbegin(), ids.cend());
        std::sort(sorted.begin(), sorted.end());
        for (quint32 fid : std::as_const(sorted))
            addFixture(fid);
    }
    else if (EFX *efx = qobject_cast<EFX *>(f))
    {
        for (EFXFixture *ef : efx->fixtures())
            addFixture(ef->head().fxi);
    }
    else if (RGBMatrix *matrix = qobject_cast<RGBMatrix *>(f))
    {
        FixtureGroup *group = doc->fixtureGroup(matrix->fixtureGroup());
        if (group != nullptr)
        {
            if (groups.contains(group->id()) == false)
                groups.append(group->id());
            for (quint32 fid : group->fixtureList())
                addFixture(fid);
        }
    }
    else if (CurveFX *curve = qobject_cast<CurveFX *>(f))
    {
        for (quint32 fid : curve->fixtures())
            addFixture(fid);
    }

    functions.append(id);
}

/** An empty Doc for things read from a file: the colour effect scripts and
 *  the universes of $source. Files (images) relative to $workspace, the
 *  folder of the show that wrote them */
Doc *newTemporary(Doc *source, const QString &workspace)
{
    Doc *tmp = new Doc(nullptr);
    tmp->setWorkspacePath(workspace);

    // colour effect scripts: from the folders the show uses
    QSet<QString> scriptDirs;
    for (const QString &name : source->rgbScriptsCache()->names())
    {
        RGBScript *script = source->rgbScriptsCache()->script(name);
        if (script != nullptr)
            scriptDirs.insert(QFileInfo(script->fileName()).absolutePath());
    }
    for (const QString &dir : scriptDirs)
        tmp->rgbScriptsCache()->load(QDir(dir));

    while (tmp->inputOutputMap()->universesCount() < source->inputOutputMap()->universesCount())
        tmp->inputOutputMap()->addUniverse();
    return tmp;
}

/** A Doc with the exported fixtures, groups and functions, under their exported IDs.
 *  Its fixtures use the definitions of $source, so do its colour effects */
Doc *loadTemporary(const QByteArray &xml, Doc *source, quint32 *root, QString *error)
{
    QXmlStreamReader reader(xml);
    if (reader.readNextStartElement() == false || reader.name() != KXMLRoot)
    {
        if (error)
            *error = QCoreApplication::translate("EffectExchange", "This is not a DMX Total Control effects file");
        return nullptr;
    }
    *root = reader.attributes().value("Root").toUInt();
    // the paths of its files are relative to the show it comes from (older
    // exports: to this one)
    const QString workspace = reader.attributes().hasAttribute(KXMLWorkspace)
            ? reader.attributes().value(KXMLWorkspace).toString() : source->workspacePath();
    Doc *tmp = newTemporary(source, workspace);

    while (reader.readNextStartElement())
    {
        if (reader.name() == KXMLFixtures)
        {
            while (reader.readNextStartElement())
            {
                if (reader.name() != KXMLFixture)
                {
                    reader.skipCurrentElement();
                    continue;
                }
                Fixture *fxi = new Fixture(tmp);
                if (fxi->loadXML(reader, tmp, source->fixtureDefCache()) == false ||
                    tmp->addFixture(fxi, fxi->id(), fxi->crossUniverse()) == false)
                    delete fxi;
            }
        }
        else if (reader.name() == KXMLGroups)
        {
            while (reader.readNextStartElement())
            {
                if (reader.name() == KXMLQLCFixtureGroup)
                    FixtureGroup::loader(reader, tmp);
                else
                    reader.skipCurrentElement();
            }
        }
        else if (reader.name() == KXMLFunctions)
        {
            while (reader.readNextStartElement())
            {
                if (reader.name() == KXMLQLCFunction)
                    Function::loader(reader, tmp);
                else
                    reader.skipCurrentElement();
            }
        }
        else
        {
            reader.skipCurrentElement();
        }
    }

    if (tmp->function(*root) == nullptr)
    {
        if (error)
            *error = QCoreApplication::translate("EffectExchange", "The file has no effect to load");
        delete tmp;
        return nullptr;
    }
    return tmp;
}

QString defKey(Fixture *fxi)
{
    QLCFixtureDef *def = fxi->fixtureDef();
    QLCFixtureMode *mode = fxi->fixtureMode();
    return (def ? def->manufacturer() + "/" + def->model() : QString()) + "/" + (mode ? mode->name() : QString());
}

QString modelKey(Fixture *fxi)
{
    QLCFixtureDef *def = fxi->fixtureDef();
    return def ? def->manufacturer() + "/" + def->model() : QString();
}

} // namespace

QByteArray EffectExchange::exportXml(Doc *doc, quint32 id)
{
    QList<quint32> functions, groups, fixtures;
    QSet<quint32> seen;
    collect(doc, id, functions, groups, fixtures, seen);

    QByteArray xml;
    QBuffer buffer(&xml);
    buffer.open(QIODevice::WriteOnly);
    QXmlStreamWriter writer(&buffer);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement(KXMLRoot);
    writer.writeAttribute("Version", "1");
    writer.writeAttribute("Root", QString::number(id));
    // the files of its functions (images) are saved relative to this folder
    writer.writeAttribute(KXMLWorkspace, doc->workspacePath());

    writer.writeStartElement(KXMLFixtures);
    for (quint32 fid : fixtures)
        if (Fixture *fxi = doc->fixture(fid))
            fxi->saveXML(&writer);
    writer.writeEndElement();

    writer.writeStartElement(KXMLGroups);
    for (quint32 gid : groups)
        if (FixtureGroup *group = doc->fixtureGroup(gid))
            group->saveXML(&writer);
    writer.writeEndElement();

    writer.writeStartElement(KXMLFunctions);
    for (quint32 fid : functions)
        if (Function *f = doc->function(fid))
            f->saveXML(&writer);
    writer.writeEndElement();

    writer.writeEndElement();
    writer.writeEndDocument();
    buffer.close();
    return xml;
}

bool EffectExchange::exportToFile(Doc *doc, quint32 id, const QString &path, QString *error)
{
    if (doc->function(id) == nullptr)
        return false;

    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
    {
        if (error)
            *error = QCoreApplication::translate("EffectExchange", "Unable to write %1").arg(path);
        return false;
    }
    file.write(exportXml(doc, id));
    file.close();
    return true;
}

quint32 EffectExchange::importFromFile(Doc *doc, const QString &path, QString *error, int *missing)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly) == false)
    {
        if (error)
            *error = QCoreApplication::translate("EffectExchange", "Unable to read %1").arg(path);
        return Function::invalidId();
    }
    return importXml(doc, file.readAll(), error, missing);
}

quint32 EffectExchange::importXml(Doc *doc, const QByteArray &xml, QString *error, int *missing)
{
    quint32 root = Function::invalidId();
    Doc *tmp = loadTemporary(xml, doc, &root, error);
    if (tmp == nullptr)
        return Function::invalidId();

    QHash<quint32, quint32> functionMap = importFunctions(doc, tmp, QList<quint32>() << root, missing);
    delete tmp;
    return functionMap.value(root, Function::invalidId());
}

QHash<quint32, quint32> EffectExchange::importFunctions(Doc *doc, Doc *tmp, const QList<quint32> &roots, int *missing,
                                                        const QHash<quint32, quint32> &knownFixtures)
{
    // the functions and what they play, the fixtures and groups they use
    QList<quint32> order, usedGroups, usedFixtures;
    QSet<quint32> seen;
    for (quint32 root : roots)
        collect(tmp, root, order, usedGroups, usedFixtures, seen);

    /* Fixtures: the copies of the fixtures imported with them, then the same
     * address and model, then same name and model, then the same model in
     * address order, then the same name */
    QHash<quint32, quint32> fixtureMap;
    QSet<quint32> used;
    for (auto it = knownFixtures.cbegin(); it != knownFixtures.cend(); ++it)
    {
        if (tmp->fixture(it.key()) == nullptr || doc->fixture(it.value()) == nullptr || used.contains(it.value()))
            continue;
        fixtureMap.insert(it.key(), it.value());
        used.insert(it.value());
    }
    QList<Fixture *> targets = doc->fixtures();
    std::sort(targets.begin(), targets.end(), [](Fixture *a, Fixture *b) {
        return a->universeAddress() < b->universeAddress();
    });

    auto match = [&](const std::function<bool(Fixture *, Fixture *)> &same)
    {
        for (Fixture *src : tmp->fixtures())
        {
            if (fixtureMap.contains(src->id()) || usedFixtures.contains(src->id()) == false)
                continue;
            for (Fixture *dst : std::as_const(targets))
            {
                if (used.contains(dst->id()) == false && same(src, dst))
                {
                    fixtureMap.insert(src->id(), dst->id());
                    used.insert(dst->id());
                    break;
                }
            }
        }
    };
    match([](Fixture *s, Fixture *d) { return s->universeAddress() == d->universeAddress() && defKey(s) == defKey(d); });
    match([](Fixture *s, Fixture *d) { return s->name() == d->name() && modelKey(s) == modelKey(d); });
    match([](Fixture *s, Fixture *d) { return defKey(s) == defKey(d); });
    match([](Fixture *s, Fixture *d) { return s->name() == d->name(); });

    if (missing)
    {
        int used = 0;
        for (quint32 fid : std::as_const(usedFixtures))
            used += tmp->fixture(fid) != nullptr ? 1 : 0;
        *missing = used - int(fixtureMap.count());
    }

    // channels: the same index with the same definition and mode, else by name
    QHash<quint32, QHash<quint32, quint32>> channelMap;
    for (auto it = fixtureMap.cbegin(); it != fixtureMap.cend(); ++it)
    {
        Fixture *src = tmp->fixture(it.key());
        Fixture *dst = doc->fixture(it.value());
        QHash<quint32, quint32> &map = channelMap[it.key()];
        bool identical = defKey(src) == defKey(dst) && src->channels() == dst->channels();
        for (quint32 c = 0; c < src->channels(); c++)
        {
            if (identical)
            {
                map.insert(c, c);
                continue;
            }
            const QLCChannel *sch = src->channel(c);
            for (quint32 d = 0; sch != nullptr && d < dst->channels(); d++)
            {
                const QLCChannel *dch = dst->channel(d);
                if (dch != nullptr && dch->name().compare(sch->name(), Qt::CaseInsensitive) == 0)
                {
                    map.insert(c, d);
                    break;
                }
            }
        }
    }

    auto mapValues = [&](const QList<SceneValue> &values)
    {
        QList<SceneValue> out;
        for (const SceneValue &scv : values)
        {
            if (channelMap.contains(scv.fxi) && channelMap[scv.fxi].contains(scv.channel))
                out.append(SceneValue(fixtureMap.value(scv.fxi), channelMap[scv.fxi].value(scv.channel), scv.value));
        }
        return out;
    };

    /* Fixture groups of the colour effects */
    QHash<quint32, quint32> groupMap;
    for (quint32 gid : std::as_const(usedGroups))
    {
        FixtureGroup *src = tmp->fixtureGroup(gid);
        if (src == nullptr)
            continue;
        FixtureGroup *group = new FixtureGroup(doc);
        group->setName(src->name());
        group->setSize(src->size());
        if (doc->addFixtureGroup(group) == false)
        {
            delete group;
            continue;
        }
        QMap<QLCPoint, GroupHead> heads = src->headsMap();
        for (auto it = heads.cbegin(); it != heads.cend(); ++it)
            if (fixtureMap.contains(it.value().fxi))
                group->assignHead(it.key(), GroupHead(fixtureMap.value(it.value().fxi), it.value().head));
        groupMap.insert(src->id(), group->id());
    }

    /* Functions, what they play first */
    QHash<quint32, quint32> functionMap;
    for (quint32 fid : std::as_const(order))
    {
        Function *src = tmp->function(fid);
        Function *copy = src != nullptr ? src->createCopy(doc) : nullptr;
        if (copy == nullptr)
            continue;
        copy->setName(src->name());
        functionMap.insert(fid, copy->id());

        if (Sequence *sequence = qobject_cast<Sequence *>(copy))
        {
            Sequence *source = qobject_cast<Sequence *>(src);
            sequence->setBoundSceneID(functionMap.value(source->boundSceneID(), Function::invalidId()));
            QList<ChaserStep> steps = source->steps();
            for (int i = 0; i < steps.count(); i++)
            {
                ChaserStep step = steps.at(i);
                step.fid = sequence->boundSceneID();
                step.values = mapValues(step.values);
                sequence->replaceStep(step, i);
            }
        }
        else if (Chaser *chaser = qobject_cast<Chaser *>(copy))
        {
            QList<ChaserStep> steps = qobject_cast<Chaser *>(src)->steps();
            for (int i = 0; i < steps.count(); i++)
            {
                ChaserStep step = steps.at(i);
                step.fid = functionMap.value(step.fid, Function::invalidId());
                chaser->replaceStep(step, i);
            }
        }
        else if (Scene *scene = qobject_cast<Scene *>(copy))
        {
            Scene *source = qobject_cast<Scene *>(src);
            scene->clear();
            for (quint32 f : source->fixtures())
                if (fixtureMap.contains(f))
                    scene->addFixture(fixtureMap.value(f));
            for (const SceneValue &scv : mapValues(source->values()))
                scene->setValue(scv);
        }
        else if (FXScene *fx = qobject_cast<FXScene *>(copy))
        {
            // its fixtures, base values and layers, on the fixtures of this show
            FXScene *source = qobject_cast<FXScene *>(src);
            QList<quint32> fixtures;
            for (quint32 f : source->fixtures())
                if (fixtureMap.contains(f))
                    fixtures.append(fixtureMap.value(f));
            QList<SceneValue> values;
            const QHash<QPair<quint32, quint32>, uchar> sourceValues = source->values();
            for (auto it = sourceValues.cbegin(); it != sourceValues.cend(); ++it)
                values.append(SceneValue(it.key().first, it.key().second, it.value()));
            QHash<QPair<quint32, quint32>, uchar> mapped;
            for (const SceneValue &scv : mapValues(values))
                mapped.insert(qMakePair(scv.fxi, scv.channel), scv.value);
            // a layer on fixtures of its own goes when none of them is here
            // (with no fixtures it would play on all the fixtures of the scene)
            QList<FXScene::Layer> layers;
            for (FXScene::Layer layer : source->layers())
            {
                QList<quint32> ids;
                for (quint32 f : std::as_const(layer.fixtures))
                    if (fixtureMap.contains(f))
                        ids.append(fixtureMap.value(f));
                if (ids.isEmpty() && layer.fixtures.isEmpty() == false)
                    continue;
                layer.fixtures = ids;
                layer.own = mapOwn(layer.own, fixtureMap);
                layers.append(layer);
            }
            fx->setFixtures(fixtures);
            fx->setValues(mapped);
            fx->setLayers(layers);
        }
        else if (Cuelist *cuelist = qobject_cast<Cuelist *>(copy))
        {
            // the cues on the fixtures of this show; what has no match goes
            // (layers left without fixtures too). The palettes are the ones
            // of the other show: the values stay, without the reference
            auto mapKey = [&](const Cuelist::Key &key, Cuelist::Key *mappedKey) {
                auto channels = channelMap.constFind(key.first);
                if (channels == channelMap.cend() || channels.value().contains(key.second) == false)
                    return false;
                *mappedKey = Cuelist::Key(fixtureMap.value(key.first), channels.value().value(key.second));
                return true;
            };
            QList<Cuelist::Cue> cues = cuelist->cues();
            for (Cuelist::Cue &cue : cues)
            {
                Cuelist::Key key;
                QHash<Cuelist::Key, uchar> values;
                for (auto it = cue.values.cbegin(); it != cue.values.cend(); ++it)
                    if (mapKey(it.key(), &key))
                        values.insert(key, it.value());
                cue.values = values;
                QSet<Cuelist::Key> releases;
                for (const Cuelist::Key &release : std::as_const(cue.releases))
                    if (mapKey(release, &key))
                        releases.insert(key);
                cue.releases = releases;
                cue.palettes.clear();
                for (FXScene::Layer &layer : cue.fx)
                {
                    QList<quint32> ids;
                    for (quint32 f : std::as_const(layer.fixtures))
                        if (fixtureMap.contains(f))
                            ids.append(fixtureMap.value(f));
                    layer.fixtures = ids;
                    layer.own = mapOwn(layer.own, fixtureMap);
                }
                QSet<Cuelist::FxKey> fxReleases;
                for (const Cuelist::FxKey &release : std::as_const(cue.fxReleases))
                    if (fixtureMap.contains(release.second))
                        fxReleases.insert(Cuelist::FxKey(release.first, fixtureMap.value(release.second)));
                cue.fxReleases = fxReleases;
            }
            cuelist->setCues(cues);
        }
        else if (EFX *efx = qobject_cast<EFX *>(copy))
        {
            // (the EFX doesn't delete the ones it lets go)
            for (EFXFixture *ef : efx->fixtures())
                if (efx->removeFixture(ef))
                    delete ef;
            for (EFXFixture *ef : qobject_cast<EFX *>(src)->fixtures())
                if (fixtureMap.contains(ef->head().fxi))
                    efx->addFixture(fixtureMap.value(ef->head().fxi), ef->head().head);
        }
        else if (RGBMatrix *matrix = qobject_cast<RGBMatrix *>(copy))
        {
            matrix->setFixtureGroup(groupMap.value(qobject_cast<RGBMatrix *>(src)->fixtureGroup(),
                                                   FixtureGroup::invalidId()));
        }
        else if (CurveFX *curve = qobject_cast<CurveFX *>(copy))
        {
            QList<quint32> mapped;
            for (quint32 f : qobject_cast<CurveFX *>(src)->fixtures())
                if (fixtureMap.contains(f))
                    mapped.append(fixtureMap.value(f));
            curve->setFixtures(mapped);
        }
        else if (Collection *rack = qobject_cast<Collection *>(copy))
        {
            for (quint32 member : rack->functions())
                rack->removeFunction(member);
            for (quint32 member : qobject_cast<Collection *>(src)->functions())
                if (functionMap.contains(member))
                    rack->addFunction(functionMap.value(member));
        }
    }

    return functionMap;
}

Doc *EffectExchange::loadShow(Doc *source, const QString &path, QList<ShowZone> *zones, QString *error)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly) == false)
    {
        if (error)
            *error = QCoreApplication::translate("EffectExchange", "Unable to read %1").arg(path);
        return nullptr;
    }

    QXmlStreamReader reader(&file);
    // DMX Total Control and QLC+ shows
    if (reader.readNextStartElement() == false || reader.name() != QLatin1String("Workspace"))
    {
        if (error)
            *error = QCoreApplication::translate("EffectExchange", "%1 is not a show file").arg(path);
        return nullptr;
    }

    // its files (images, audio) are relative to its folder
    Doc *tmp = newTemporary(source, QFileInfo(path).absolutePath());
    while (reader.readNextStartElement())
    {
        if (reader.name() == KXMLQLCEngine)
        {
            while (reader.readNextStartElement())
            {
                if (reader.name() == KXMLFixture)
                {
                    Fixture *fxi = new Fixture(tmp);
                    if (fxi->loadXML(reader, tmp, source->fixtureDefCache()) == false ||
                        tmp->addFixture(fxi, fxi->id(), fxi->crossUniverse()) == false)
                        delete fxi;
                }
                else if (reader.name() == KXMLQLCFixtureGroup)
                {
                    FixtureGroup::loader(reader, tmp);
                }
                else if (reader.name() == KXMLQLCFunction)
                {
                    Function::loader(reader, tmp);
                }
                else if (reader.name() == KXMLQLCMonitorProperties)
                {
                    tmp->monitorProperties()->loadXML(reader, tmp);
                }
                else
                {
                    reader.skipCurrentElement();
                }
            }
        }
        else if (reader.name() == QLatin1String("DMXTotalControl"))
        {
            // the groups of scenes
            while (reader.readNextStartElement())
            {
                if (reader.name() != QLatin1String("Zones"))
                {
                    reader.skipCurrentElement();
                    continue;
                }
                while (reader.readNextStartElement())
                {
                    if (reader.name() != QLatin1String("Zone"))
                    {
                        reader.skipCurrentElement();
                        continue;
                    }
                    ShowZone zone;
                    zone.name = reader.attributes().value("Name").toString();
                    zone.color = reader.attributes().value("Color").toString();
                    zone.exclusive = reader.attributes().value("Exclusive").toString() != "0";
                    while (reader.readNextStartElement())
                    {
                        if (reader.name() == QLatin1String("Function"))
                            zone.functions.append(reader.attributes().value("ID").toUInt());
                        reader.skipCurrentElement();
                    }
                    if (zones)
                        zones->append(zone);
                }
            }
        }
        else
        {
            reader.skipCurrentElement();
        }
    }

    if (reader.hasError())
    {
        if (error)
            *error = QCoreApplication::translate("EffectExchange", "Error while loading %1").arg(path);
        delete tmp;
        return nullptr;
    }
    return tmp;
}

namespace
{

uint cycleOf(Doc *doc, quint32 id, QSet<quint32> &visiting)
{
    Function *f = doc->function(id);
    // a rack in a cycle (A in B in A) counts once
    if (f == nullptr || visiting.contains(id))
        return 0;

    uint bpmBeat = 60000 / uint(qMax(1, doc->masterTimer()->bpmNumber()));
    auto inTime = [&](uint value) {
        return f->tempoType() == Function::Beats ? Function::beatsToTime(value, int(bpmBeat)) : value;
    };

    if (FXScene *fx = qobject_cast<FXScene *>(f))
        return fx->cycleDuration();
    if (Collection *rack = qobject_cast<Collection *>(f))
    {
        uint longest = 0;
        visiting.insert(id);
        for (quint32 member : rack->functions())
            longest = qMax(longest, cycleOf(doc, member, visiting));
        visiting.remove(id);
        return longest;
    }
    if (Chaser *chaser = qobject_cast<Chaser *>(f))
    {
        uint total = 0;
        for (const ChaserStep &step : chaser->steps())
        {
            uint hold = chaser->durationMode() == Chaser::PerStep ? step.duration : chaser->duration();
            total += inTime(hold);
        }
        return total;
    }
    if (RGBMatrix *matrix = qobject_cast<RGBMatrix *>(f))
    {
        FixtureGroup *group = doc->fixtureGroup(matrix->fixtureGroup());
        int steps = matrix->algorithm() != nullptr && group != nullptr
                ? matrix->algorithm()->rgbMapStepCount(group->size()) : 1;
        return inTime(matrix->duration()) * uint(qMax(1, steps));
    }
    if (qobject_cast<EFX *>(f) != nullptr || qobject_cast<CurveFX *>(f) != nullptr)
        return inTime(f->duration());
    return 0;
}

} // namespace

uint EffectExchange::cycleDuration(Doc *doc, quint32 id)
{
    QSet<quint32> visiting;
    return cycleOf(doc, id, visiting);
}

quint32 EffectExchange::renderToSteps(Doc *doc, quint32 id, uint stepMs, QString *error)
{
    // the whole cycle (up to MaxRenderCycle), in 256 steps at most: longer
    // cycles get longer steps
    const uint tick = MasterTimer::tick();
    stepMs = qMax(tick, stepMs);
    uint cycle = qMin(MaxRenderCycle, cycleDuration(doc, id));
    if (cycle / stepMs > MaxRenderSteps)
    {
        stepMs = (cycle + MaxRenderSteps - 1) / MaxRenderSteps;
        stepMs = (stepMs + tick - 1) / tick * tick;
    }
    int stepCount = qBound(1, int(cycle / stepMs), int(MaxRenderSteps));

    quint32 root = Function::invalidId();
    Doc *tmp = loadTemporary(exportXml(doc, id), doc, &root, error);
    if (tmp == nullptr)
        return Function::invalidId();

    // everything in time: the offline clock has no beats
    tmp->inputOutputMap()->setBeatGeneratorType(InputOutputMap::Internal);
    tmp->inputOutputMap()->setBpmNumber(doc->masterTimer()->bpmNumber());
    for (Function *f : tmp->functions())
        f->setTempoType(Function::Time);

    auto renderFrame = [&]()
    {
        tmp->masterTimer()->renderTick();
        QList<Universe *> universes = tmp->inputOutputMap()->claimUniverses();
        for (Universe *universe : universes)
            universe->renderFrame(MasterTimer::tick());
        tmp->inputOutputMap()->releaseUniverses(false);
    };
    // every channel of its fixtures as it goes out now
    auto sampleOutput = [&]()
    {
        QList<SceneValue> sample;
        QList<Universe *> universes = tmp->inputOutputMap()->claimUniverses();
        for (Fixture *fxi : tmp->fixtures())
        {
            if (fxi->universe() >= quint32(universes.count()))
                continue;
            Universe *universe = universes.at(int(fxi->universe()));
            for (quint32 c = 0; c < fxi->channels(); c++)
                sample.append(SceneValue(fxi->id(), c, universe->preGMValue(int(fxi->address() + c))));
        }
        tmp->inputOutputMap()->releaseUniverses(false);
        return sample;
    };

    // what goes out before the effect: the default values of the channels
    // (a pan or a zoom in the middle, a shutter open...). It records what the
    // effect changes from that, not the defaults of the channels it doesn't
    // touch (the steps would hold them over the other scenes)
    renderFrame();
    QHash<QPair<quint32, quint32>, uchar> rest;
    for (const SceneValue &scv : sampleOutput())
        rest.insert(qMakePair(scv.fxi, scv.channel), scv.value);

    Function *function = tmp->function(root);
    function->start(tmp->masterTimer(), FunctionParent::master());

    QList<QList<SceneValue>> samples;
    uint now = 0;
    for (int s = 0; s < stepCount; s++)
    {
        uint target = uint(s) * stepMs;
        do
        {
            renderFrame();
            now += MasterTimer::tick();
        }
        while (now <= target);
        samples.append(sampleOutput());
    }

    function->stop(FunctionParent::master());
    tmp->masterTimer()->renderTick();
    delete tmp;

    // the channels the effect gives a value: somewhere in the cycle, other
    // than the one at rest
    QSet<QPair<quint32, quint32>> active;
    for (const QList<SceneValue> &sample : std::as_const(samples))
        for (const SceneValue &scv : sample)
            if (scv.value != rest.value(qMakePair(scv.fxi, scv.channel), 0))
                active.insert(qMakePair(scv.fxi, scv.channel));
    if (active.isEmpty())
    {
        if (error)
            *error = QCoreApplication::translate("EffectExchange", "The effect gives no output to record");
        return Function::invalidId();
    }

    auto keep = [&](const QList<SceneValue> &sample)
    {
        QList<SceneValue> out;
        for (const SceneValue &scv : sample)
            if (active.contains(qMakePair(scv.fxi, scv.channel)) && doc->fixture(scv.fxi) != nullptr)
                out.append(scv);
        return out;
    };

    Function *source = doc->function(id);
    Scene *bound = new Scene(doc);
    bound->setVisible(false);
    for (const SceneValue &scv : keep(samples.first()))
    {
        if (bound->fixtures().contains(scv.fxi) == false)
            bound->addFixture(scv.fxi);
        bound->setValue(scv);
    }
    if (doc->addFunction(bound) == false)
    {
        delete bound;
        return Function::invalidId();
    }

    Sequence *sequence = new Sequence(doc);
    sequence->setBoundSceneID(bound->id());
    sequence->setFadeInMode(Chaser::PerStep);
    sequence->setFadeOutMode(Chaser::PerStep);
    sequence->setDurationMode(Chaser::PerStep);
    sequence->setRunOrder(Function::Loop);
    sequence->setPriority(source != nullptr ? source->priority() : Function::defaultPriority());
    if (doc->addFunction(sequence) == false)
    {
        delete sequence;
        doc->deleteFunction(bound->id());
        return Function::invalidId();
    }
    sequence->setName(QCoreApplication::translate("EffectExchange", "%1 (steps)").arg(source != nullptr ? source->name() : QString()));
    bound->setName(QCoreApplication::translate("EffectExchange", "%1 (channels)").arg(sequence->name()));

    for (const QList<SceneValue> &sample : std::as_const(samples))
        sequence->applyDumpValues(keep(sample), -1);

    QList<ChaserStep> steps = sequence->steps();
    for (int i = 0; i < steps.count(); i++)
    {
        ChaserStep step = steps.at(i);
        step.fadeIn = 0;
        step.fadeOut = 0;
        step.hold = stepMs;
        step.duration = stepMs;
        sequence->replaceStep(step, i);
    }
    return sequence->id();
}
