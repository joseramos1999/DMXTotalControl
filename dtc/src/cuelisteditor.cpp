/*
  DMX Total Control
  cuelisteditor.cpp

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

#include <QSet>
#include <QMap>
#include <QDebug>
#include <functional>
#include <algorithm>
#include <climits>

#include "cuelisteditor.h"
#include "effecteditor.h"
#include "palettemanager.h"
#include "zonemanager.h"
#include "programmer.h"
#include "zone.h"

#include "mastertimer.h"
#include "chaserstep.h"
#include "qlcchannel.h"
#include "sequence.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"

typedef Cuelist::Key Key;
typedef Cuelist::FxKey FxKey;

namespace
{
/** $layer loses every fixture it has on $slotKeys: a layer can't be split, a
 *  fixture that loses one axis (pan of a pan/tilt FX) loses all of them */
bool trimLayer(FXScene::Layer &layer, const QSet<FxKey> &slotKeys)
{
    const QStringList axes = Cuelist::fxAxes(layer.param);
    const qsizetype removed = layer.fixtures.removeIf([&](quint32 fixture) {
        for (const QString &axis : axes)
            if (slotKeys.contains(FxKey(axis, fixture)))
                return true;
        return false;
    });
    return removed > 0;
}

/** The layer of $layers that holds the slot $key, or nullptr */
const FXScene::Layer *layerOn(const QList<FXScene::Layer> &layers, const FxKey &key)
{
    for (const FXScene::Layer &layer : layers)
        if (layer.fixtures.contains(key.second) && Cuelist::fxAxes(layer.param).contains(key.first))
            return &layer;
    return nullptr;
}

/** The same FX, whatever fixtures they have: the fixtures they share do the
 *  same (what they do their own way included) */
bool sameFx(FXScene::Layer a, FXScene::Layer b)
{
    for (quint32 fixture : std::as_const(a.fixtures))
        if (b.fixtures.contains(fixture) && a.shapeOf(fixture) != b.shapeOf(fixture))
            return false;
    a.fixtures.clear();
    a.own.clear();
    b.fixtures.clear();
    b.own.clear();
    return a == b;
}

/** What $layer plays, $fx plays already: one of its layers is the same FX on
 *  all the fixtures of $layer, in the same order. A cue that takes some
 *  fixtures of a tracked layer leaves it on the others: loaded from the
 *  tracked state, the programmer holds that smaller copy */
bool tracksIn(const FXScene::Layer &layer, const QList<FXScene::Layer> &fx)
{
    if (layer.fixtures.isEmpty())
        return false;
    for (const FXScene::Layer &held : fx)
    {
        if (sameFx(held, layer) == false)
            continue;
        // the fixtures of $layer, one after the other among the held ones
        qsizetype next = 0;
        for (quint32 fixture : held.fixtures)
        {
            if (next < layer.fixtures.count() && layer.fixtures.at(next) == fixture)
                next++;
        }
        if (next == layer.fixtures.count())
            return true;
    }
    return false;
}

/** What a cue changes goes into $target, where $target doesn't change it
 *  itself: what the stage shows on $target stays the same without $cue */
void moveInto(Cuelist::Cue &target, const Cuelist::Cue &cue)
{
    for (auto it = cue.values.cbegin(); it != cue.values.cend(); ++it)
    {
        if (target.values.contains(it.key()) || target.releases.contains(it.key()))
            continue;
        target.values.insert(it.key(), it.value());
        auto palette = cue.palettes.constFind(it.key());
        if (palette != cue.palettes.cend())
            target.palettes.insert(it.key(), palette.value());
    }
    for (const Key &key : cue.releases)
    {
        if (target.values.contains(key) == false)
            target.releases.insert(key);
    }

    // the FX slots the target holds or releases itself
    QSet<FxKey> owned = target.fxReleases;
    for (const FXScene::Layer &layer : std::as_const(target.fx))
        owned.unite(Cuelist::fxKeysOf(layer));

    // what the cue leaves on the slots it touches, by itself: a later layer
    // of it takes the fixtures it shares with an earlier one (the whole
    // fixture: a layer can't be split), and a slot it touches without
    // holding it at the end (an FX release, or a fixture an earlier layer
    // lost) is free after it. An earlier layer must not come back on the
    // fixtures it lost
    QList<QSet<quint32>> held;
    QHash<FxKey, int> holders;
    QSet<FxKey> touched = cue.fxReleases;
    for (int l = 0; l < cue.fx.count(); l++)
    {
        const FXScene::Layer &layer = cue.fx.at(l);
        held.append(QSet<quint32>(layer.fixtures.cbegin(), layer.fixtures.cend()));
        for (quint32 fixture : layer.fixtures)
        {
            for (const QString &axis : Cuelist::fxAxes(layer.param))
            {
                const FxKey key(axis, fixture);
                touched.insert(key);
                const int other = holders.value(key, -1);
                if (other >= 0 && other != l)
                {
                    held[other].remove(fixture);
                    for (const QString &otherAxis : Cuelist::fxAxes(cue.fx.at(other).param))
                    {
                        if (holders.value(FxKey(otherAxis, fixture), -1) == other)
                            holders.remove(FxKey(otherAxis, fixture));
                    }
                }
                holders.insert(key, l);
            }
        }
    }

    QList<FXScene::Layer> moved;
    for (int l = 0; l < cue.fx.count(); l++)
    {
        FXScene::Layer layer = cue.fx.at(l);
        const QStringList axes = Cuelist::fxAxes(layer.param);
        QList<quint32> kept;
        for (quint32 fixture : std::as_const(layer.fixtures))
        {
            if (held.at(l).contains(fixture) == false)
                continue;
            bool taken = false;
            for (const QString &axis : axes)
                taken = taken || owned.contains(FxKey(axis, fixture));
            if (taken == false)
            {
                kept.append(fixture);
                continue;
            }
            // the target took a slot of it: the layer lost the whole
            // fixture there, so the slots of it the target doesn't take were free
            for (const QString &axis : axes)
                if (owned.contains(FxKey(axis, fixture)) == false)
                    target.fxReleases.insert(FxKey(axis, fixture));
        }
        layer.fixtures = kept;
        if (layer.fixtures.isEmpty() == false)
            moved.append(layer);
    }
    // the slots the cue frees (its FX releases, the fixtures its layers lost)
    for (const FxKey &key : std::as_const(touched))
    {
        if (holders.contains(key) == false && owned.contains(key) == false)
            target.fxReleases.insert(key);
    }
    // before the layers of the target: a later layer wins a slot, as before
    target.fx = moved + target.fx;
}

/** The time of a step in ms: steps that follow the BPM hold beats */
uint stepMs(uint time, bool beats, int beatDuration)
{
    if (time == Function::defaultSpeed())
        return 0;
    if (time == Function::infiniteSpeed() || beats == false)
        return time;
    return Function::beatsToTime(time, beatDuration);
}
}

CuelistEditor::CuelistEditor(Doc *doc, Programmer *programmer, ZoneManager *zones, PaletteManager *palettes,
                             QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_programmer(programmer)
    , m_zones(zones)
    , m_palettes(palettes)
    , m_selectedId(-1)
    , m_editId(Function::invalidId())
    , m_editUid(0)
    , m_editTracked(true)
{
    // engine signals go by name (MinGW)
    connect(m_doc, SIGNAL(functionRemoved(quint32)), this, SLOT(slotFunctionRemoved(quint32)));
    connect(m_doc, SIGNAL(functionReplaced(quint32)), this, SLOT(slotFunctionReplaced(quint32)));
    // a cleared programmer doesn't hold the cue any more (loading a look is
    // not a clear), nor one where a scene or a step was loaded since
    connect(m_programmer, &Programmer::cleared, this, &CuelistEditor::stopEditing);
    connect(m_programmer, &Programmer::lookLoaded, this, &CuelistEditor::stopEditing);
}

Cuelist *CuelistEditor::cuelist(quint32 id) const
{
    return qobject_cast<Cuelist *>(m_doc->function(id));
}

void CuelistEditor::touch(quint32 id)
{
    m_doc->setModified();
    emit edited(id);
    // its index or its number may have changed
    if (id == m_editId)
        emit editingChanged();
}

int CuelistEditor::selectedId() const
{
    return m_selectedId;
}

void CuelistEditor::setSelectedId(int id)
{
    if (id < 0 || cuelist(quint32(id)) == nullptr)
        id = -1;
    if (id == m_selectedId)
        return;
    m_selectedId = id;
    emit selectedIdChanged();
}

QVariantMap CuelistEditor::editing() const
{
    QVariantMap map;
    Cuelist *cl = cuelist(m_editId);
    const int index = cl != nullptr ? cl->indexOfUid(m_editUid) : -1;
    if (index < 0)
        return map;

    const Cuelist::Cue cue = cl->cueAt(index);
    map.insert("id", m_editId);
    map.insert("uid", cue.uid);
    map.insert("index", index);
    map.insert("number", Cuelist::numberToString(cue.number));
    map.insert("name", cue.name);
    map.insert("tracked", m_editTracked);
    return map;
}

void CuelistEditor::stopEditing()
{
    if (m_editId == Function::invalidId() && m_editUid == 0)
        return;
    m_editId = Function::invalidId();
    m_editUid = 0;
    emit editingChanged();
}

void CuelistEditor::clear()
{
    setSelectedId(-1);
    stopEditing();
}

void CuelistEditor::slotFunctionRemoved(quint32 id)
{
    if (int(id) == m_selectedId)
        setSelectedId(-1);
    if (id == m_editId)
        stopEditing();
}

void CuelistEditor::slotFunctionReplaced(quint32 id)
{
    // another function with the same ID: its cues are not the ones edited
    if (id == m_editId)
        stopEditing();
    if (int(id) == m_selectedId && cuelist(id) == nullptr)
        setSelectedId(-1);
}

/*********************************************************************
 * The programmer
 *********************************************************************/

QHash<Key, uchar> CuelistEditor::programmerValues() const
{
    const QMap<Key, uchar> &values = m_programmer->valueMap();
    QHash<Key, uchar> hash;
    hash.reserve(values.count());
    for (auto it = values.cbegin(); it != values.cend(); ++it)
        hash.insert(it.key(), it.value());
    return hash;
}

QHash<Key, int> CuelistEditor::programmerRefs() const
{
    return m_programmer->paletteRefs();
}

QSet<Key> CuelistEditor::programmerKeys() const
{
    QSet<Key> keys;
    const QMap<Key, uchar> &values = m_programmer->valueMap();
    for (auto it = values.cbegin(); it != values.cend(); ++it)
        keys.insert(it.key());
    return keys;
}

QSet<FxKey> CuelistEditor::programmerSlots() const
{
    QSet<FxKey> slotKeys;
    for (const FXScene::Layer &layer : m_programmer->fxLayerList())
        slotKeys.unite(Cuelist::fxKeysOf(layer));
    return slotKeys;
}

void CuelistEditor::fillDelta(Cuelist::Cue &cue, const Cuelist::State &before, bool block) const
{
    const QHash<Key, uchar> values = programmerValues();
    const QHash<Key, int> refs = programmerRefs();
    for (auto it = values.cbegin(); it != values.cend(); ++it)
    {
        // what the cue would track already, from the same palette, stays out of it
        const int ref = refs.value(it.key(), -1);
        auto tracked = before.values.constFind(it.key());
        if (block == false && tracked != before.values.cend() && tracked.value() == it.value() &&
            before.palettes.value(it.key(), -1) == ref)
            continue;
        cue.values.insert(it.key(), it.value());
        if (ref >= 0)
            cue.palettes.insert(it.key(), ref);
    }

    // the FX the cues before already play (the programmer holds them when
    // the tracked state was loaded, maybe on fewer fixtures) stay out too
    for (const FXScene::Layer &layer : m_programmer->fxLayerList())
    {
        if (block || tracksIn(layer, before.fx) == false)
            cue.fx.append(layer);
    }
}

/*********************************************************************
 * Cuelists
 *********************************************************************/

int CuelistEditor::create(const QString &name, int zoneIndex, int priority)
{
    const QHash<Key, uchar> values = programmerValues();
    const QList<FXScene::Layer> fx = m_programmer->fxLayerList();
    if (values.isEmpty() && fx.isEmpty())
    {
        emit message(tr("Set some values in the programmer first: they become cue 1"));
        return -1;
    }

    Cuelist *cl = new Cuelist(m_doc);
    if (priority >= 0)
        cl->setPriority(priority);

    QString title = name.trimmed();
    if (title.isEmpty())
    {
        // numbered: "Cuelist 1", "Cuelist 2"...
        QSet<QString> names;
        for (Function *f : m_doc->functions())
            names.insert(f->name());
        int number = m_doc->functionsByType(Function::CuelistType).count() + 1;
        while (names.contains(tr("Cuelist %1").arg(number)))
            number++;
        title = tr("Cuelist %1").arg(number);
    }
    cl->setName(title);

    // cue 1: everything the programmer holds
    Cuelist::Cue cue;
    cue.number = 1000;
    cue.fade = cl->fadeInSpeed();
    cue.values = values;
    const QHash<Key, int> refs = programmerRefs();
    for (auto it = refs.cbegin(); it != refs.cend(); ++it)
        if (values.contains(it.key()))
            cue.palettes.insert(it.key(), it.value());
    cue.fx = fx;
    cl->addCue(cue);

    if (m_doc->addFunction(cl) == false)
    {
        delete cl;
        return -1;
    }

    Zone *zone = m_zones->zone(zoneIndex);
    if (zone != nullptr)
        zone->addFunction(cl->id());

    setSelectedId(int(cl->id()));
    touch(cl->id());
    return int(cl->id());
}

QVariantList CuelistEditor::cues(quint32 id) const
{
    QVariantList list;
    Cuelist *cl = cuelist(id);
    if (cl == nullptr)
        return list;

    // palette names, in the order of the palettes (by type and number)
    QList<int> paletteOrder;
    QHash<int, QString> paletteNames;
    if (m_palettes != nullptr)
    {
        for (const QVariant &p : m_palettes->palettes())
        {
            const QVariantMap palette = p.toMap();
            paletteOrder.append(palette.value("id").toInt());
            paletteNames.insert(palette.value("id").toInt(), palette.value("name").toString());
        }
    }

    const QList<Cuelist::Cue> all = cl->cues();
    for (int i = 0; i < all.count(); i++)
    {
        const Cuelist::Cue &cue = all.at(i);
        QVariantMap map;
        map.insert("uid", cue.uid);
        map.insert("index", i);
        map.insert("number", Cuelist::numberToString(cue.number));
        map.insert("name", cue.name);
        map.insert("note", cue.note);
        map.insert("trigger", Cuelist::triggerToString(cue.trigger).toLower());
        map.insert("triggerTime", cue.triggerTime);
        map.insert("fade", cue.fade);
        map.insert("downFade", cue.downFade);
        map.insert("delay", cue.delay);
        map.insert("skip", cue.skip);
        map.insert("block", cue.block);
        map.insert("changes", int(cue.values.count()));
        map.insert("releases", int(cue.releases.count()));
        map.insert("fx", int(cue.fx.count()));
        map.insert("fxReleases", int(cue.fxReleases.count()));

        QSet<int> used;
        for (int pid : cue.palettes)
            used.insert(pid);
        QStringList names;
        for (int pid : std::as_const(paletteOrder))
            if (used.contains(pid))
                names.append(paletteNames.value(pid));
        map.insert("palettes", names);
        list.append(map);
    }
    return list;
}

QVariantMap CuelistEditor::options(quint32 id) const
{
    QVariantMap map;
    Cuelist *cl = cuelist(id);
    if (cl == nullptr)
        return map;

    map.insert("tracking", cl->tracking());
    map.insert("atEnd", Cuelist::atEndToString(cl->atEnd()).toLower());
    map.insert("resetOnRelease", cl->resetOnRelease());
    map.insert("releaseTime", cl->fadeOutSpeed());
    map.insert("defaultFade", cl->fadeInSpeed());
    map.insert("backTime", cl->backTime());
    map.insert("releaseOnNextGo", cl->releaseOnNextGo());
    map.insert("priority", cl->priority());
    return map;
}

void CuelistEditor::setOption(quint32 id, const QString &key, const QVariant &value)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr)
        return;

    if (key == "tracking")
        cl->setTracking(value.toBool());
    else if (key == "atEnd")
        cl->setAtEnd(Cuelist::stringToAtEnd(value.toString()));
    else if (key == "resetOnRelease")
        cl->setResetOnRelease(value.toBool());
    else if (key == "releaseTime")
        cl->setFadeOutSpeed(uint(qMax(0, value.toInt())));
    else if (key == "defaultFade")
        cl->setFadeInSpeed(uint(qMax(0, value.toInt())));
    else if (key == "backTime")
        cl->setBackTime(value.toInt());
    else if (key == "releaseOnNextGo")
        cl->setReleaseOnNextGo(value.toBool());
    else if (key == "priority")
        cl->setPriority(value.toInt());
    else
        return;

    touch(id);
}

/*********************************************************************
 * Cue numbers
 *********************************************************************/

QString CuelistEditor::nextNumber(quint32 id, int afterIndex) const
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr)
        return QString();
    const QList<Cuelist::Cue> all = cl->cues();
    if (all.isEmpty())
        return QStringLiteral("1");

    // after the last cue: the next whole number
    if (afterIndex == -1 || afterIndex >= all.count() - 1)
    {
        const int next = (all.last().number / 1000 + 1) * 1000;
        return next <= Cuelist::maxNumber() ? Cuelist::numberToString(next) : QString();
    }

    // between two cues (before the first one: between 0 and it), the
    // roundest number that fits: whole, then tenths, hundredths, thousandths
    const int low = afterIndex < 0 ? 0 : all.at(afterIndex).number;
    const int high = all.at(afterIndex < 0 ? 0 : afterIndex + 1).number;
    for (int grid : { 1000, 100, 10, 1 })
    {
        // the points k * grid strictly between low and high
        const int first = low / grid + 1;
        const int last = (high - 1) / grid;
        if (first > last)
            continue;

        // the one nearest the middle; the lower one on a tie
        const int twiceMiddle = low + high;
        int best = -1;
        for (int k : { twiceMiddle / (2 * grid), twiceMiddle / (2 * grid) + 1 })
        {
            k = qBound(first, k, last);
            if (best < 0 || qAbs(2 * k * grid - twiceMiddle) < qAbs(2 * best * grid - twiceMiddle))
                best = k;
        }
        return Cuelist::numberToString(best * grid);
    }
    return QString();
}

bool CuelistEditor::numberExists(quint32 id, const QString &number) const
{
    Cuelist *cl = cuelist(id);
    const int n = Cuelist::parseNumber(number);
    return cl != nullptr && n > 0 && cl->indexOfNumber(n) >= 0;
}

/*********************************************************************
 * Record, update, load
 *********************************************************************/

int CuelistEditor::recordCue(quint32 id, const QString &number, const QVariantMap &opts)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr)
        return -1;

    int n = -1;
    if (number.trimmed().isEmpty())
    {
        n = Cuelist::parseNumber(nextNumber(id));
        if (n < 0)
        {
            emit message(tr("There is no cue number left after the last cue"));
            return -1;
        }
    }
    else
    {
        n = Cuelist::parseNumber(number);
        if (n < 0)
        {
            emit message(tr("%1 is not a cue number").arg(number.trimmed()));
            return -1;
        }
    }

    const int existing = cl->indexOfNumber(n);
    if (existing < 0)
        return recordNew(cl, n, opts);

    const QString mode = opts.value("existing", "ask").toString();
    if (mode == QLatin1String("merge"))
        return updateCue(id, existing, opts) ? existing : -1;

    if (mode == QLatin1String("replace"))
    {
        // the cue holds what the programmer adds to the cues before it, and
        // nothing else; it keeps its name (unless one is given) and its times
        Cuelist::Cue cue = cl->cueAt(existing);
        if (opts.contains("cueOnly"))
            cue.skip = opts.value("cueOnly").toBool();
        if (opts.contains("block"))
            cue.block = opts.value("block").toBool();
        if (opts.value("name").toString().trimmed().isEmpty() == false)
            cue.name = opts.value("name").toString().trimmed();
        cue.values.clear();
        cue.palettes.clear();
        cue.releases.clear();
        cue.fx.clear();
        cue.fxReleases.clear();
        fillDelta(cue, cl->stateBefore(existing), cue.block);
        if (cl->replaceCue(cue) == false)
            return -1;
        if (cue.values.isEmpty() && cue.fx.isEmpty())
            emit message(tr("Cue %1 has no changes from the previous cue").arg(Cuelist::numberToString(n)));
        touch(id);
        return existing;
    }

    if (mode == QLatin1String("insertAfter"))
    {
        const int point = Cuelist::parseNumber(nextNumber(id, existing));
        if (point < 0)
        {
            emit message(tr("There is no free cue number after cue %1").arg(Cuelist::numberToString(n)));
            return -1;
        }
        return recordNew(cl, point, opts);
    }

    // ask: nothing changes, the caller offers merge, replace or insert
    return -2;
}

int CuelistEditor::recordNew(Cuelist *cl, int number, const QVariantMap &opts)
{
    const int index = cl->insertIndexFor(number);

    Cuelist::Cue cue;
    cue.number = number;
    cue.name = opts.value("name").toString().trimmed();
    cue.skip = opts.value("cueOnly").toBool();
    cue.block = opts.value("block").toBool();
    // the times of the cue before it; the first one gets the default fade of the list
    if (index > 0)
    {
        const Cuelist::Cue previous = cl->cueAt(index - 1);
        cue.fade = previous.fade;
        cue.downFade = previous.downFade;
        cue.delay = previous.delay;
    }
    else
    {
        cue.fade = cl->fadeInSpeed();
    }
    fillDelta(cue, cl->stateBefore(index), cue.block);

    const int added = cl->addCue(cue);
    if (added < 0)
    {
        emit message(tr("Cue %1 could not be recorded").arg(Cuelist::numberToString(number)));
        return -1;
    }
    if (cue.values.isEmpty() && cue.fx.isEmpty())
        emit message(tr("Cue %1 has no changes from the previous cue").arg(Cuelist::numberToString(number)));
    touch(cl->id());
    return added;
}

bool CuelistEditor::updateCue(quint32 id, int index, const QVariantMap &opts)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return false;

    const QHash<Key, uchar> values = programmerValues();
    const QList<FXScene::Layer> layers = m_programmer->fxLayerList();
    if (values.isEmpty() && layers.isEmpty())
    {
        emit message(tr("The programmer is empty: there is nothing to update"));
        return false;
    }

    QList<Cuelist::Cue> all = cl->cues();
    Cuelist::Cue &cue = all[index];
    if (opts.value("block").toBool())
        cue.block = true;
    const Cuelist::State before = cl->stateBefore(index);
    const Cuelist::State previous = cl->trackedState(index);
    const QHash<Key, int> refs = programmerRefs();

    for (auto it = values.cbegin(); it != values.cend(); ++it)
    {
        const Key &key = it.key();
        const int ref = refs.value(key, -1);
        cue.releases.remove(key);
        auto tracked = before.values.constFind(key);
        if (cue.block == false && tracked != before.values.cend() && tracked.value() == it.value() &&
            before.palettes.value(key, -1) == ref)
        {
            // what it tracks: the cue doesn't need it, it tracks again
            cue.values.remove(key);
            cue.palettes.remove(key);
            continue;
        }
        cue.values.insert(key, it.value());
        if (ref >= 0)
            cue.palettes.insert(key, ref);
        else
            cue.palettes.remove(key);
    }

    for (const FXScene::Layer &layer : layers)
    {
        const QSet<FxKey> slotKeys = Cuelist::fxKeysOf(layer);
        for (const FxKey &key : slotKeys)
            cue.fxReleases.remove(key);

        // the layers of the cue on those slots lose them; the same layer
        // stays in its place, unless the cues before play it already (a
        // layer this cue splits is in the programmer on the fixtures left to it)
        const bool tracked = cue.block == false && tracksIn(layer, before.fx);
        bool kept = false;
        QList<FXScene::Layer> fx;
        for (FXScene::Layer own : std::as_const(cue.fx))
        {
            if (kept == false && tracked == false && own == layer)
            {
                kept = true;
                fx.append(own);
                continue;
            }
            trimLayer(own, slotKeys);
            if (own.fixtures.isEmpty() == false)
                fx.append(own);
        }
        if (kept == false && tracked == false)
            fx.append(layer);
        cue.fx = fx;
    }

    // cue only: the values that changed here don't go on to the next cues,
    // they get what they showed before (up to the first cue that tracks on).
    // A new palette on the same value is a change too: the next cues would
    // follow that palette's edits
    if (opts.value("cueOnly").toBool() && cl->tracking() && cue.skip == false)
    {
        QList<Key> changed;
        for (auto it = values.cbegin(); it != values.cend(); ++it)
        {
            auto old = previous.values.constFind(it.key());
            if (old == previous.values.cend() || old.value() != it.value() ||
                previous.palettes.value(it.key(), -1) != refs.value(it.key(), -1))
                changed.append(it.key());
        }
        for (int t = index + 1; t < all.count() && changed.isEmpty() == false; t++)
        {
            Cuelist::Cue &next = all[t];
            for (const Key &key : std::as_const(changed))
            {
                if (next.values.contains(key) || next.releases.contains(key))
                    continue;
                auto old = previous.values.constFind(key);
                if (old == previous.values.cend())
                {
                    next.releases.insert(key);
                    continue;
                }
                next.values.insert(key, old.value());
                auto palette = previous.palettes.constFind(key);
                if (palette != previous.palettes.cend())
                    next.palettes.insert(key, palette.value());
            }
            if (next.skip == false)
                break;
        }
    }

    cl->setCues(all);
    touch(id);
    return true;
}

bool CuelistEditor::loadCue(quint32 id, int index, bool tracked)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return false;

    const Cuelist::Cue cue = cl->cueAt(index);
    QHash<Key, uchar> values = cue.values;
    QHash<Key, int> refs = cue.palettes;
    QList<FXScene::Layer> fx = cue.fx;
    if (tracked)
    {
        const Cuelist::State state = cl->trackedState(index);
        values = state.values;
        refs = state.palettes;
        fx = state.fx;
    }

    // the fixtures in the order they are seen: by channel, then the FX
    QMap<Key, uchar> sorted;
    for (auto it = values.cbegin(); it != values.cend(); ++it)
        sorted.insert(it.key(), it.value());
    QList<quint32> fixtures;
    for (auto it = sorted.cbegin(); it != sorted.cend(); ++it)
        if (fixtures.contains(it.key().first) == false)
            fixtures.append(it.key().first);
    for (const FXScene::Layer &layer : std::as_const(fx))
        for (quint32 fixture : layer.fixtures)
            if (fixtures.contains(fixture) == false)
                fixtures.append(fixture);

    m_programmer->loadState(sorted, fixtures, fx, refs);

    m_editId = id;
    m_editUid = cue.uid;
    m_editTracked = tracked;
    emit editingChanged();
    emit cueLoaded(id, index);
    return true;
}

bool CuelistEditor::takeOut(Cuelist::Cue &cue, const QSet<Key> &keys, const QSet<FxKey> &slotKeys, bool release)
{
    bool changed = false;
    for (const Key &key : keys)
    {
        if (cue.values.remove(key) > 0)
            changed = true;
        cue.palettes.remove(key);
        if (release)
        {
            if (cue.releases.contains(key) == false)
            {
                cue.releases.insert(key);
                changed = true;
            }
        }
        else if (cue.releases.remove(key))
        {
            changed = true;
        }
    }

    if (slotKeys.isEmpty())
        return changed;

    QList<FXScene::Layer> fx;
    for (FXScene::Layer layer : std::as_const(cue.fx))
    {
        if (trimLayer(layer, slotKeys))
            changed = true;
        if (layer.fixtures.isEmpty() == false)
            fx.append(layer);
    }
    cue.fx = fx;
    for (const FxKey &key : slotKeys)
    {
        if (release)
        {
            if (cue.fxReleases.contains(key) == false)
            {
                cue.fxReleases.insert(key);
                changed = true;
            }
        }
        else if (cue.fxReleases.remove(key))
        {
            changed = true;
        }
    }
    return changed;
}

bool CuelistEditor::removeFromCue(quint32 id, int index)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return false;

    const QSet<Key> keys = programmerKeys();
    const QSet<FxKey> slotKeys = programmerSlots();
    if (keys.isEmpty() && slotKeys.isEmpty())
    {
        emit message(tr("The programmer is empty: set the channels to take out of the cue first"));
        return false;
    }

    Cuelist::Cue cue = cl->cueAt(index);
    if (takeOut(cue, keys, slotKeys, false) == false)
    {
        emit message(tr("Cue %1 doesn't hold any channel of the programmer").arg(Cuelist::numberToString(cue.number)));
        return false;
    }
    cl->replaceCue(cue);
    touch(id);
    return true;
}

bool CuelistEditor::releaseInCue(quint32 id, int index)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return false;

    const QSet<Key> keys = programmerKeys();
    const QSet<FxKey> slotKeys = programmerSlots();
    if (keys.isEmpty() && slotKeys.isEmpty())
    {
        emit message(tr("The programmer is empty: set the channels to release in the cue first"));
        return false;
    }

    Cuelist::Cue cue = cl->cueAt(index);
    if (takeOut(cue, keys, slotKeys, true))
    {
        cl->replaceCue(cue);
        touch(id);
    }
    return true;
}

void CuelistEditor::parseKeys(const QVariantList &keys, QSet<Key> *channels, QSet<FxKey> *slotKeys)
{
    for (const QVariant &v : keys)
    {
        const QStringList parts = v.toString().split(QLatin1Char(':'));
        if (parts.count() != 2 || parts.at(1).isEmpty())
            continue;
        bool ok = false;
        const quint32 fixture = parts.at(0).toUInt(&ok);
        if (ok == false)
            continue;
        const quint32 channel = parts.at(1).toUInt(&ok);
        if (ok)
            channels->insert(Key(fixture, channel));
        else
            slotKeys->insert(FxKey(parts.at(1), fixture));
    }
}

bool CuelistEditor::removeKeysFromCue(quint32 id, int index, const QVariantList &keys)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return false;

    QSet<Key> channels;
    QSet<FxKey> slotKeys;
    parseKeys(keys, &channels, &slotKeys);
    Cuelist::Cue cue = cl->cueAt(index);
    if (takeOut(cue, channels, slotKeys, false) == false)
        return false;
    cl->replaceCue(cue);
    touch(id);
    return true;
}

bool CuelistEditor::releaseKeysInCue(quint32 id, int index, const QVariantList &keys)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return false;

    QSet<Key> channels;
    QSet<FxKey> slotKeys;
    parseKeys(keys, &channels, &slotKeys);
    if (channels.isEmpty() && slotKeys.isEmpty())
        return false;
    Cuelist::Cue cue = cl->cueAt(index);
    if (takeOut(cue, channels, slotKeys, true))
    {
        cl->replaceCue(cue);
        touch(id);
    }
    return true;
}

/*********************************************************************
 * Cues
 *********************************************************************/

bool CuelistEditor::removeCues(quint32 id, const QVariantList &indexes, bool keepLook)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr)
        return false;

    QList<Cuelist::Cue> all = cl->cues();
    QList<int> order;
    for (const QVariant &v : indexes)
    {
        const int index = v.toInt();
        if (index >= 0 && index < all.count() && order.contains(index) == false)
            order.append(index);
    }
    if (order.isEmpty())
        return false;

    // the last ones first: what each one gives to the next cue stays in the list
    std::sort(order.begin(), order.end(), std::greater<int>());
    const bool tracking = cl->tracking();
    bool editedGone = false;
    for (int index : std::as_const(order))
    {
        const Cuelist::Cue cue = all.at(index);
        // a cue only (skip) cue gives nothing to the next ones
        if (keepLook && tracking && cue.skip == false)
        {
            // the cues that showed it: the next one, and on through the cue
            // only cues up to the first one that tracks on
            for (int t = index + 1; t < all.count(); t++)
            {
                moveInto(all[t], cue);
                if (all.at(t).skip == false)
                    break;
            }
        }
        if (id == m_editId && cue.uid == m_editUid)
            editedGone = true;
        all.removeAt(index);
    }

    cl->setCues(all);
    if (editedGone)
        stopEditing();
    touch(id);
    return true;
}

int CuelistEditor::copyCue(quint32 id, int index, const QString &number, bool tracked)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return -1;

    const int n = Cuelist::parseNumber(number.trimmed().isEmpty() ? nextNumber(id) : number);
    if (n < 0)
    {
        emit message(tr("%1 is not a cue number").arg(number.trimmed()));
        return -1;
    }
    if (cl->indexOfNumber(n) >= 0)
    {
        emit message(tr("Cue %1 already exists").arg(Cuelist::numberToString(n)));
        return -1;
    }

    // its name and times come along; a new uid
    Cuelist::Cue copy = cl->cueAt(index);
    copy.uid = 0;
    copy.number = n;
    if (tracked)
    {
        // everything the stage shows on it, as hard values
        const Cuelist::State state = cl->trackedState(index);
        copy.values = state.values;
        copy.palettes = state.palettes;
        copy.fx = state.fx;
        copy.releases.clear();
        copy.fxReleases.clear();
        copy.block = true;
        copy.skip = false;
    }

    const int added = cl->addCue(copy);
    if (added < 0)
    {
        emit message(tr("Cue %1 could not be recorded").arg(Cuelist::numberToString(n)));
        return -1;
    }
    touch(id);
    return added;
}

QString CuelistEditor::renumber(quint32 id, int index, const QString &number)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return tr("There is no such cue");

    const int n = Cuelist::parseNumber(number);
    if (n < 0)
        return tr("%1 is not a cue number").arg(number.trimmed());
    Cuelist::Cue cue = cl->cueAt(index);
    if (n == cue.number)
        return QString();
    if (cl->indexOfNumber(n) >= 0)
        return tr("Cue %1 already exists").arg(Cuelist::numberToString(n));

    // its place among the other cues
    int place = cl->insertIndexFor(n);
    if (place > index)
        place--;

    cue.number = n;
    if (cl->replaceCue(cue) == false)
        return tr("Cue %1 could not be renumbered").arg(Cuelist::numberToString(n));
    // allowed: the cues around it track something else now
    if (place != index)
        emit message(tr("Moving a cue changes what the cues after it track"));
    touch(id);
    return QString();
}

void CuelistEditor::setCueTimes(quint32 id, const QVariantList &indexes, int fade, int downFade, int delay)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr)
        return;

    QList<Cuelist::Cue> all = cl->cues();
    for (const QVariant &v : indexes)
    {
        const int index = v.toInt();
        if (index < 0 || index >= all.count())
            continue;
        Cuelist::Cue &cue = all[index];
        if (fade >= 0)
            cue.fade = uint(fade);
        if (downFade >= 0)
            cue.downFade = downFade;
        else if (downFade == -2)
            cue.downFade = -1;
        if (delay >= 0)
            cue.delay = uint(delay);
    }
    cl->setCues(all);
    touch(id);
}

void CuelistEditor::setCueTrigger(quint32 id, const QVariantList &indexes, const QString &trigger, int time)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr)
        return;

    const QString name = trigger.trimmed().toLower();
    const bool known = name == QLatin1String("go") || name == QLatin1String("follow") || name == QLatin1String("wait");
    QList<Cuelist::Cue> all = cl->cues();
    for (const QVariant &v : indexes)
    {
        const int index = v.toInt();
        if (index < 0 || index >= all.count())
            continue;
        Cuelist::Cue &cue = all[index];
        if (known)
            cue.trigger = Cuelist::stringToTrigger(name);
        if (time >= 0)
            cue.triggerTime = uint(time);
    }
    cl->setCues(all);
    touch(id);
}

void CuelistEditor::setCueName(quint32 id, int index, const QString &name)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return;
    Cuelist::Cue cue = cl->cueAt(index);
    cue.name = name.trimmed();
    cl->replaceCue(cue);
    touch(id);
}

void CuelistEditor::setCueNote(quint32 id, int index, const QString &note)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return;
    Cuelist::Cue cue = cl->cueAt(index);
    cue.note = note;
    cl->replaceCue(cue);
    touch(id);
}

void CuelistEditor::setCueFlags(quint32 id, const QVariantList &indexes, int skip, int block)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr)
        return;

    QList<Cuelist::Cue> all = cl->cues();
    for (const QVariant &v : indexes)
    {
        const int index = v.toInt();
        if (index < 0 || index >= all.count())
            continue;
        if (skip >= 0)
            all[index].skip = skip != 0;
        if (block >= 0)
            all[index].block = block != 0;
    }
    cl->setCues(all);
    touch(id);
}

/*********************************************************************
 * Views
 *********************************************************************/

QString CuelistEditor::paletteName(int paletteId) const
{
    if (m_palettes == nullptr || paletteId < 0)
        return QString();
    return m_palettes->palette(paletteId).value("name").toString();
}

QVariantList CuelistEditor::trackingSheet(quint32 id, int index) const
{
    QVariantList list;
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return list;

    const Cuelist::Cue cue = cl->cueAt(index);
    const Cuelist::State before = cl->stateBefore(index);
    const Cuelist::State state = cl->trackedState(index);
    QHash<quint32, int> numbers;
    for (const Cuelist::Cue &c : cl->cues())
        numbers.insert(c.uid, c.number);
    auto numberOf = [&](quint32 uid) {
        return numbers.contains(uid) ? Cuelist::numberToString(numbers.value(uid)) : QString();
    };

    // fixture -> channel -> row, fixture -> axis -> row: sorted
    QMap<quint32, QMap<quint32, QVariantMap>> channelRows;
    QMap<quint32, QMap<QString, QVariantMap>> fxRows;

    QSet<Key> keys = cue.releases;
    for (auto it = state.values.cbegin(); it != state.values.cend(); ++it)
        keys.insert(it.key());
    for (const Key &key : std::as_const(keys))
    {
        Fixture *fixture = m_doc->fixture(key.first);
        const QLCChannel *channel = fixture != nullptr ? fixture->channel(key.second) : nullptr;
        QVariantMap row;
        row.insert("key", QString("%1:%2").arg(key.first).arg(key.second));
        row.insert("channel", key.second);
        row.insert("name", channel != nullptr ? channel->name() : QString());

        int value = -1;
        int palette = -1;
        QString status;
        QString from;
        if (cue.releases.contains(key))
        {
            // released here: from the cue that had set it
            status = QStringLiteral("released");
            if (before.owner.contains(key))
                from = numberOf(before.owner.value(key));
        }
        else
        {
            value = state.values.value(key);
            palette = state.palettes.value(key, -1);
            const quint32 owner = state.owner.value(key);
            if (owner == cue.uid)
            {
                const bool same = before.values.contains(key) && before.values.value(key) == value &&
                                  before.palettes.value(key, -1) == palette;
                status = same ? QStringLiteral("blocked") : QStringLiteral("changed");
            }
            else
            {
                status = QStringLiteral("tracked");
                from = numberOf(owner);
            }
        }
        row.insert("value", value);
        row.insert("state", status);
        row.insert("from", from);
        row.insert("palette", palette);
        row.insert("paletteName", paletteName(palette));
        channelRows[key.first].insert(key.second, row);
    }

    QSet<FxKey> slotKeys = cue.fxReleases;
    for (auto it = state.fxOwner.cbegin(); it != state.fxOwner.cend(); ++it)
        slotKeys.insert(it.key());
    for (const FxKey &key : std::as_const(slotKeys))
    {
        QVariantMap row;
        row.insert("key", QString("%1:%2").arg(key.second).arg(key.first));
        row.insert("axis", key.first);

        QString param = key.first;
        QString status;
        QString from;
        if (cue.fxReleases.contains(key))
        {
            status = QStringLiteral("released");
            if (const FXScene::Layer *layer = layerOn(before.fx, key))
                param = layer->param;
            if (before.fxOwner.contains(key))
                from = numberOf(before.fxOwner.value(key));
        }
        else
        {
            const FXScene::Layer *layer = layerOn(state.fx, key);
            if (layer != nullptr)
                param = layer->param;
            const quint32 owner = state.fxOwner.value(key);
            if (owner == cue.uid)
            {
                const FXScene::Layer *tracked = layerOn(before.fx, key);
                const bool same = layer != nullptr && tracked != nullptr && sameFx(*layer, *tracked);
                status = same ? QStringLiteral("blocked") : QStringLiteral("changed");
            }
            else
            {
                status = QStringLiteral("tracked");
                from = numberOf(owner);
            }
        }
        row.insert("param", param);
        row.insert("state", status);
        row.insert("from", from);
        fxRows[key.second].insert(key.first, row);
    }

    QList<quint32> fixtures = channelRows.keys();
    for (quint32 fixture : fxRows.keys())
        if (fixtures.contains(fixture) == false)
            fixtures.append(fixture);
    std::sort(fixtures.begin(), fixtures.end());

    for (quint32 fid : std::as_const(fixtures))
    {
        Fixture *fixture = m_doc->fixture(fid);
        QVariantMap entry;
        entry.insert("fixture", fid);
        entry.insert("fixtureName", fixture != nullptr ? fixture->name() : QString("#%1").arg(fid));
        QVariantList channels;
        for (const QVariantMap &row : channelRows.value(fid))
            channels.append(row);
        QVariantList fx;
        for (const QVariantMap &row : fxRows.value(fid))
            fx.append(row);
        entry.insert("channels", channels);
        entry.insert("fx", fx);
        list.append(entry);
    }
    return list;
}

QVariantList CuelistEditor::cuePalettes(quint32 id, int index) const
{
    QVariantList list;
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || m_palettes == nullptr || index < 0 || index >= cl->cueCount())
        return list;

    QHash<int, int> channels;
    for (int pid : cl->cueAt(index).palettes)
        channels[pid]++;

    for (const QVariant &p : m_palettes->palettes())
    {
        QVariantMap palette = p.toMap();
        const int pid = palette.value("id").toInt();
        if (channels.contains(pid) == false)
            continue;
        palette.insert("channels", channels.value(pid));
        list.append(palette);
    }
    return list;
}

/*********************************************************************
 * Steps to cuelist
 *********************************************************************/

int CuelistEditor::convertFromSteps(quint32 sequenceId)
{
    Sequence *sequence = qobject_cast<Sequence *>(m_doc->function(sequenceId));
    if (sequence == nullptr)
        return -1;
    QList<ChaserStep> steps = sequence->steps();
    if (steps.isEmpty())
    {
        emit message(tr("These steps are empty: there is nothing to convert"));
        return -1;
    }
    if (sequence->direction() == Function::Backward)
        std::reverse(steps.begin(), steps.end());

    // how many times the steps play, 0 for ever ("Play once" of older shows
    // is one loop, as the playback panel shows it). A few loops play out as
    // cues, one after the other
    const int loops = sequence->runOrder() == Function::SingleShot && sequence->loopCount() == 0
                          ? 1 : qMax(0, sequence->loopCount());
    const qint64 cueCount = qint64(steps.count()) * qMax(1, loops);
    if (cueCount > 1000)
    {
        emit message(tr("These steps play too many times to become cues: lower the number of times first"));
        return -1;
    }

    // stopped wherever it plays (also when started in this frame, not run
    // yet); the super scenes and chasers that play the steps play the
    // cuelist instead (Doc::functionReplaced), and the Doc deletes the steps
    // only once the timer holds them no more
    if (sequence->stopped() == false || sequence->isRunning())
        sequence->stopAndWait();

    // the times each step plays with, in ms
    Scene *bound = qobject_cast<Scene *>(m_doc->function(sequence->boundSceneID()));
    const bool beats = sequence->tempoType() == Function::Beats;
    const int beat = m_doc->masterTimer()->beatTimeDuration();
    auto fadeInOf = [&](const ChaserStep &step) {
        switch (sequence->fadeInMode())
        {
            case Chaser::Common: return stepMs(sequence->fadeInSpeed(), beats, beat);
            case Chaser::PerStep: return stepMs(step.fadeIn, beats, beat);
            default: return bound != nullptr ? stepMs(bound->fadeInSpeed(), beats, beat) : 0u;
        }
    };
    auto fadeOutOf = [&](const ChaserStep &step) {
        switch (sequence->fadeOutMode())
        {
            case Chaser::Common: return stepMs(sequence->fadeOutSpeed(), beats, beat);
            case Chaser::PerStep: return stepMs(step.fadeOut, beats, beat);
            default: return bound != nullptr ? stepMs(bound->fadeOutSpeed(), beats, beat) : 0u;
        }
    };
    auto durationOf = [&](const ChaserStep &step) {
        return stepMs(sequence->durationMode() == Chaser::PerStep ? step.duration : sequence->duration(),
                      beats, beat);
    };
    // a fade can't last forever
    auto finite = [](uint ms) { return ms == Function::infiniteSpeed() ? 0u : qMin(ms, uint(INT_MAX)); };
    // driven by the beats or the pulses: each division is a GO, the times don't count
    const bool driven = sequence->driveMode() == Function::DriveBeatGO || sequence->driveMode() == Function::DrivePulse;

    // cue 1 holds every channel of the steps (zeros are real), the next cues
    // what changes. Steps hold every channel of their hidden scene; one
    // without a channel keeps what the step before left there (they reuse
    // the scene), and the first one starts from zero
    QHash<Key, uchar> stage;
    if (bound != nullptr)
        for (const SceneValue &scv : bound->values())
            stage.insert(Key(scv.fxi, scv.channel), 0);

    QList<Cuelist::Cue> cues;
    for (int c = 0; c < int(cueCount); c++)
    {
        const int i = c % steps.count();
        const ChaserStep &step = steps.at(i);
        Cuelist::Cue cue;
        cue.number = (c + 1) * 1000;
        cue.note = step.note;
        QHash<Key, uchar> target = stage;
        for (const SceneValue &scv : step.values)
            target.insert(Key(scv.fxi, scv.channel), scv.value);
        for (auto it = target.cbegin(); it != target.cend(); ++it)
        {
            auto old = stage.constFind(it.key());
            if (c == 0 || old == stage.cend() || old.value() != it.value())
                cue.values.insert(it.key(), it.value());
        }
        stage = target;

        cue.fade = finite(fadeInOf(step));
        const uint fadeOut = finite(fadeOutOf(step));
        cue.downFade = fadeOut == cue.fade ? -1 : int(fadeOut);

        // each cue starts when the step before it ends; an endless step waits for a GO
        if (c > 0 && driven == false)
        {
            const uint duration = durationOf(steps.at((c - 1) % steps.count()));
            if (duration != Function::infiniteSpeed())
            {
                cue.trigger = Cuelist::TriggerWait;
                cue.triggerTime = duration;
            }
        }
        cues.append(cue);
    }

    Cuelist *cl = new Cuelist(m_doc);
    cl->setName(sequence->name());
    cl->setPriority(sequence->priority());
    cl->setPath(sequence->path(true));
    cl->setVisible(sequence->isVisible());
    // a release takes the fade out of the steps
    cl->setFadeOutSpeed(finite(fadeOutOf(steps.first())));
    if (driven)
    {
        cl->setDriveMode(sequence->driveMode());
        cl->setDivisions(sequence->divisions());
    }

    // for ever: a loop, the first cue follows the last step. Played out: it
    // holds the last look, or it's released when the last step ends (the
    // trigger of the first cue goes on past the last one, as in a loop)
    const bool holds = loops > 0 && sequence->endAction() == Function::EndPause;
    cl->setAtEnd(loops == 0 ? Cuelist::AtEndLoop : holds ? Cuelist::AtEndStay : Cuelist::AtEndRelease);
    const uint lastDuration = durationOf(steps.last());
    if (holds == false && driven == false && lastDuration != Function::infiniteSpeed())
    {
        cues[0].trigger = Cuelist::TriggerWait;
        cues[0].triggerTime = lastDuration;
    }
    cl->setCues(cues);

    // the same ID: groups, SHOW buttons and mappings play the cuelist now
    const quint32 boundId = sequence->boundSceneID();
    if (m_doc->replaceFunction(sequenceId, cl) == false)
    {
        delete cl;
        return -1;
    }
    // the hidden channels of the steps go with them, unless they are a
    // scene of the user or other steps use them too (QLC+ shows, copies)
    if (boundId != sequenceId)
        EffectEditor::deleteBoundScene(m_doc, boundId);

    touch(sequenceId);
    return int(sequenceId);
}
