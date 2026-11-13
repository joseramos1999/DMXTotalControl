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
#include <QLocale>
#include <functional>
#include <algorithm>
#include <climits>
#include <utility>

#include "cuelisteditor.h"
#include "effecteditor.h"
#include "palettemanager.h"
#include "zonemanager.h"
#include "qlcexchange.h"
#include "programmer.h"
#include "zone.h"

#include "mastertimer.h"
#include "chaserstep.h"
#include "fadechannel.h"
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

/** The uids of the cues at $indexes, sorted ("3,7"): the cells an undo step names */
QString uidsOf(const QList<Cuelist::Cue> &cues, const QVariantList &indexes)
{
    QList<quint32> uids;
    for (const QVariant &v : indexes)
    {
        const int index = v.toInt();
        if (index >= 0 && index < cues.count() && uids.contains(cues.at(index).uid) == false)
            uids.append(cues.at(index).uid);
    }
    std::sort(uids.begin(), uids.end());
    QStringList parts;
    for (quint32 uid : std::as_const(uids))
        parts.append(QString::number(uid));
    return parts.join(QLatin1Char(','));
}

/** The index in $now of the first cue that is new or changed since $was, or -1 */
int firstChange(const QList<Cuelist::Cue> &was, const QList<Cuelist::Cue> &now)
{
    QHash<quint32, int> indexOf;
    for (int i = 0; i < was.count(); i++)
        indexOf.insert(was.at(i).uid, i);
    for (int i = 0; i < now.count(); i++)
    {
        const int old = indexOf.value(now.at(i).uid, -1);
        if (old < 0 || was.at(old) != now.at(i))
            return i;
    }
    return -1;
}

/** The steps a cuelist keeps, and how close two edits of the same cells
 *  are to be one step (the wheel on a time) */
const int kUndoSteps = 50;
const qint64 kUndoMergeMs = 2000;

/** A time in seconds, in the language of the app: "2 s", "0.5 s", "1.25 s" */
QString secondsText(uint ms)
{
    const int decimals = ms % 1000 == 0 ? 0 : ms % 100 == 0 ? 1 : ms % 10 == 0 ? 2 : 3;
    return QLocale().toString(ms / 1000.0, 'f', decimals) + QStringLiteral(" s");
}

/** A new cue of a Timecode list with no clock running: this long after the cue before it */
const int kTimecodeGap = 5000;

/** The cues without a time get one, $kTimecodeGap after the cue before them
 *  (the first one: 00:00). @return whether one got a time */
bool giveCueTimes(QList<Cuelist::Cue> &cues)
{
    bool given = false;
    int last = -1;
    for (Cuelist::Cue &cue : cues)
    {
        if (cue.time < 0)
        {
            cue.time = last < 0 ? 0 : qMin(last + kTimecodeGap, Cuelist::maxCueTime());
            given = true;
        }
        last = cue.time;
    }
    return given;
}

/** The fade and delay of a part: a time, -1 unchanged, -2 the cue's again */
void changePart(Cuelist::PartTime &part, int fade, int delay)
{
    if (fade >= 0 || fade == -2)
        part.fade = fade >= 0 ? fade : -1;
    if (delay >= 0 || delay == -2)
        part.delay = delay >= 0 ? delay : -1;
}

/** The times a part edit changes ("fade,delay"): the cells of its undo step */
QString partCells(int fade, int delay)
{
    QStringList cells;
    if (fade >= 0 || fade == -2)
        cells.append(QStringLiteral("fade"));
    if (delay >= 0 || delay == -2)
        cells.append(QStringLiteral("delay"));
    return cells.join(QLatin1Char(','));
}

/** A time fan over $count fixtures, from $from to $to ms: "line" one after
 *  the other, "middle" from the middle out, "ends" from both ends in */
QList<int> fanTimes(int count, int from, int to, const QString &shape)
{
    QList<int> times;
    for (int i = 0; i < count; i++)
    {
        const qreal u = count == 1 ? 0.0 : qreal(i) / qreal(count - 1);
        qreal w = u;
        if (shape == QLatin1String("middle"))
            w = qAbs(2.0 * u - 1.0);
        else if (shape == QLatin1String("ends"))
            w = 1.0 - qAbs(2.0 * u - 1.0);
        times.append(qRound(qBound(0.0, qreal(from) + (qreal(to) - qreal(from)) * w, 3600000.0)));
    }
    return times;
}

/** Whether an action names a function $gone() says is gone */
bool namesGone(const Cuelist::CueAction &action, const std::function<bool(quint32)> &gone)
{
    bool ok = false;
    const int fid = action.value(QStringLiteral("functionId")).toInt(&ok);
    return ok && fid >= 0 && gone(quint32(fid));
}

/** Whether one of $actions names one */
bool namesGone(const QList<Cuelist::CueAction> &actions, const std::function<bool(quint32)> &gone)
{
    for (const Cuelist::CueAction &action : actions)
        if (namesGone(action, gone))
            return true;
    return false;
}

/** Whether an action of $cues names one (read only: the cues stay shared) */
bool namesGone(const QList<Cuelist::Cue> &cues, const std::function<bool(quint32)> &gone)
{
    for (const Cuelist::Cue &cue : cues)
        if (namesGone(cue.actions, gone))
            return true;
    return false;
}

/** The actions of $cues on the functions $gone() says get functionId -1 (a
 *  function made later may get the ID). Only the cues they are in are
 *  copied. @return whether one changed */
bool forgetFunctions(QList<Cuelist::Cue> &cues, const std::function<bool(quint32)> &gone)
{
    bool changed = false;
    for (qsizetype i = 0; i < cues.count(); i++)
    {
        if (namesGone(std::as_const(cues).at(i).actions, gone) == false)
            continue;
        for (Cuelist::CueAction &action : cues[i].actions)
        {
            if (namesGone(action, gone))
                action.insert(QStringLiteral("functionId"), QStringLiteral("-1"));
        }
        changed = true;
    }
    return changed;
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
    , m_scopeDepth(0)
    , m_scopeId(Function::invalidId())
{
    m_undoClock.start();
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
    m_undo.clear();
    m_redo.clear();
    emit undoChanged(Function::invalidId());
}

void CuelistEditor::slotFunctionRemoved(quint32 id)
{
    if (int(id) == m_selectedId)
        setSelectedId(-1);
    if (id == m_editId)
        stopEditing();
    dropUndo(id);
    // the actions of the cues on it do nothing, as the mappings (not while
    // the show is emptied: a loaded show is read again in afterLoad())
    if (m_doc->isClearing() == false)
        forgetActionsOn([id](quint32 fid) { return fid == id; });
}

void CuelistEditor::slotFunctionReplaced(quint32 id)
{
    // another function with the same ID: its cues are not the ones edited
    if (id == m_editId)
        stopEditing();
    if (int(id) == m_selectedId && cuelist(id) == nullptr)
        setSelectedId(-1);
    // (steps converted to a cuelist: it starts a history of its own)
    dropUndo(id);
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

    const QList<Cuelist::Cue> all = cl->cues();
    // the numbers of the cues the links go to
    QHash<quint32, int> numberOf;
    for (const Cuelist::Cue &cue : all)
        numberOf.insert(cue.uid, cue.number);
    // the time of the cue before each one that has a time
    int lastTime = -1;
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
        map.insert("curve", Cuelist::curveToString(cue.curve));
        map.insert("skip", cue.skip);
        map.insert("block", cue.block);
        map.insert("noMib", cue.noMib);
        map.insert("changes", int(cue.values.count()));
        map.insert("releases", int(cue.releases.count()));
        map.insert("fx", int(cue.fx.count()));
        map.insert("fxReleases", int(cue.fxReleases.count()));

        // palette names, in the order of the palettes (by type and number)
        QSet<int> used;
        for (int pid : cue.palettes)
            used.insert(pid);
        map.insert("palettes", m_palettes != nullptr ? m_palettes->namesOf(used) : QStringList());
        map.insert("ownTimes", int(cue.groupTimes.count() + cue.fixtureTimes.count()));
        map.insert("timesText", timesText(cue));
        map.insert("link", cue.link);
        map.insert("linkNumber", cue.link != 0 && numberOf.contains(cue.link)
                                     ? Cuelist::numberToString(numberOf.value(cue.link)) : QString());
        map.insert("linkCount", cue.linkCount);
        map.insert("actions", int(cue.actions.count()));
        QStringList actions;
        for (const Cuelist::CueAction &action : cue.actions)
            actions.append(describeAction(action));
        map.insert("actionsText", actions.join(QStringLiteral(", ")));
        // its time on the clock (a Timecode list), and whether it comes
        // before the one of the cue before it
        map.insert("time", cue.time);
        map.insert("timeWarning", cue.time >= 0 && cue.time < lastTime);
        if (cue.time >= 0)
            lastTime = cue.time;
        list.append(map);
    }
    return list;
}

QString CuelistEditor::curveName(int curve)
{
    switch (curve)
    {
        case FadeChannel::CurveSmooth: return tr("Smooth");
        case FadeChannel::CurveSlowStart: return tr("Slow start");
        case FadeChannel::CurveFastStart: return tr("Fast start");
        default: return tr("Linear");
    }
}

QString CuelistEditor::timesText(const Cuelist::Cue &cue)
{
    // the curve of the cue when it isn't linear, each attribute with the
    // delay and the fade it takes (and a curve of its own), then the fixtures
    QStringList parts;
    if (cue.curve != FadeChannel::CurveLinear)
        parts.append(curveName(cue.curve));
    for (auto it = cue.groupTimes.cbegin(); it != cue.groupTimes.cend(); ++it)
    {
        QString name;
        switch (it.key())
        {
            case Cuelist::TimeIntensity: name = tr("Intensity"); break;
            case Cuelist::TimeColour: name = tr("Colour"); break;
            case Cuelist::TimePosition: name = tr("Position"); break;
            default: name = tr("Beam"); break;
        }
        const Cuelist::Times times = cue.timesFor(Fixture::invalidId(), it.key());
        QString part = QStringLiteral("%1 %2 + %3").arg(name, secondsText(times.delay), secondsText(times.fade));
        if (it.value().curve >= 0)
            part += QStringLiteral(" (%1)").arg(curveName(it.value().curve));
        parts.append(part);
    }
    if (cue.fixtureTimes.isEmpty() == false)
        parts.append(tr("%n fixtures", "", int(cue.fixtureTimes.count())));
    // (a middle dot between them)
    return parts.join(QStringLiteral(" %1 ").arg(QChar(0x00B7)));
}

QVariantMap CuelistEditor::options(quint32 id) const
{
    QVariantMap map;
    Cuelist *cl = cuelist(id);
    if (cl == nullptr)
        return map;

    map.insert("mode", Cuelist::modeToString(cl->mode()).toLower());
    map.insert("tracking", cl->tracking());
    map.insert("atEnd", Cuelist::atEndToString(cl->atEnd()).toLower());
    map.insert("resetOnRelease", cl->resetOnRelease());
    map.insert("releaseTime", cl->fadeOutSpeed());
    map.insert("defaultFade", cl->fadeInSpeed());
    map.insert("backTime", cl->backTime());
    map.insert("releaseOnNextGo", cl->releaseOnNextGo());
    map.insert("priority", cl->priority());
    map.insert("moveInBlack", cl->moveInBlack());
    map.insert("mibDelay", cl->mibDelay());
    map.insert("mibFade", cl->mibFade());
    // a Chase: its speed, its change (-1: the times of the cues), its direction
    map.insert("chaseBpm", cl->chaseBpm());
    map.insert("chaseShowTempo", cl->chaseShowTempo());
    map.insert("chaseMultiplier", cl->chaseMultiplier());
    map.insert("chaseXfade", cl->chaseXfade());
    map.insert("chaseDirection", Cuelist::chaseDirectionToString(cl->chaseDirection()).toLower());
    // a Timecode list: its clock, and where the 00:00 of a MIDI timecode is
    map.insert("tcSource", Cuelist::tcSourceToString(cl->tcSource()).toLower());
    map.insert("tcOffset", cl->tcOffset());
    return map;
}

void CuelistEditor::setOption(quint32 id, const QString &key, const QVariant &value)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr)
        return;

    EditScope scope(this, id, tr("Cuelist options"), QStringLiteral("option:") + key);
    const Cuelist::Mode mode = cl->mode();
    if (applyOption(cl, key, value) == false)
        return;
    // an Override list goes above the others: from the default priority to
    // the highest one, in the same step (the programmer stays above it)
    if (key == "mode" && mode != Cuelist::ModeOverride && cl->mode() == Cuelist::ModeOverride &&
        cl->priority() == Function::defaultPriority())
        cl->setPriority(Function::maxPriority());
    // a Timecode list plays its cues by their time: the ones without one
    // get one, in the same step (5 s after the cue before them)
    if (key == "mode" && mode != Cuelist::ModeTimecode && cl->mode() == Cuelist::ModeTimecode)
    {
        QList<Cuelist::Cue> all = cl->cues();
        if (giveCueTimes(all))
            cl->setCues(all);
    }
    touch(id);
}

bool CuelistEditor::applyOption(Cuelist *cl, const QString &key, const QVariant &value)
{
    if (key == "mode")
        cl->setMode(Cuelist::stringToMode(value.toString()));
    else if (key == "tracking")
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
    else if (key == "moveInBlack")
        cl->setMoveInBlack(value.toBool());
    else if (key == "mibDelay")
        cl->setMibDelay(uint(qMax(0, value.toInt())));
    else if (key == "mibFade")
        cl->setMibFade(uint(qMax(0, value.toInt())));
    else if (key == "chaseBpm")
        cl->setChaseBpm(value.toInt());
    else if (key == "chaseShowTempo")
        cl->setChaseShowTempo(value.toBool());
    else if (key == "chaseMultiplier")
        cl->setChaseMultiplier(value.toDouble());
    else if (key == "chaseXfade")
        cl->setChaseXfade(value.toInt());
    else if (key == "chaseDirection")
        cl->setChaseDirection(Cuelist::stringToChaseDirection(value.toString()));
    else if (key == "tcSource")
        cl->setTcSource(Cuelist::stringToTcSource(value.toString()));
    else if (key == "tcOffset")
        cl->setTcOffset(uint(qMax(0, value.toInt())));
    else
        return false;
    return true;
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

    EditScope scope(this, id, tr("Record cue %1").arg(Cuelist::numberToString(n)));
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
        scope.setLabel(tr("Record cue %1").arg(Cuelist::numberToString(point)));
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
    // the times of the cue before it, its curve and attribute times too (not
    // the ones of its fixtures); the first one gets the default fade of the list
    if (index > 0)
    {
        const Cuelist::Cue previous = cl->cueAt(index - 1);
        cue.fade = previous.fade;
        cue.downFade = previous.downFade;
        cue.delay = previous.delay;
        cue.curve = previous.curve;
        cue.groupTimes = previous.groupTimes;
    }
    else
    {
        cue.fade = cl->fadeInSpeed();
    }
    // DMX Total Control: a Timecode list gives it a time on its clock
    if (cl->mode() == Cuelist::ModeTimecode)
        cue.time = newCueTime(cl, index);
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

int CuelistEditor::newCueTime(const Cuelist *cl, int index)
{
    // the clock while it runs (a paused list holds it: as stopped)
    const Cuelist::Status status = cl->status();
    if (status.clock >= 0 && cl->isPaused() == false)
        return int(qMin(status.clock, qint64(Cuelist::maxCueTime())));
    for (int i = index - 1; i >= 0; i--)
    {
        const int time = cl->cueAt(i).time;
        if (time >= 0)
            return qMin(time + kTimecodeGap, Cuelist::maxCueTime());
    }
    return 0;
}

bool CuelistEditor::updateCue(quint32 id, int index, const QVariantMap &opts)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return false;

    EditScope scope(this, id, tr("Update cue %1").arg(Cuelist::numberToString(cl->cueAt(index).number)));
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
    EditScope scope(this, id, tr("Take out of cue %1").arg(Cuelist::numberToString(cue.number)));
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
    EditScope scope(this, id, tr("Release in cue %1").arg(Cuelist::numberToString(cue.number)));
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
    EditScope scope(this, id, tr("Take out of cue %1").arg(Cuelist::numberToString(cue.number)));
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
    EditScope scope(this, id, tr("Release in cue %1").arg(Cuelist::numberToString(cue.number)));
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

    EditScope scope(this, id, order.count() == 1
                                  ? tr("Delete cue %1").arg(Cuelist::numberToString(all.at(order.first()).number))
                                  : tr("Delete %n cues", "", int(order.count())));
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

    // the links to the deleted cues go (the list drops them too): said
    QSet<quint32> kept;
    for (const Cuelist::Cue &cue : std::as_const(all))
        kept.insert(cue.uid);
    for (Cuelist::Cue &cue : all)
    {
        if (cue.link == 0 || kept.contains(cue.link))
            continue;
        cue.link = 0;
        cue.linkCount = 0;
        emit message(tr("Cue %1 linked to a deleted cue: its link is gone").arg(Cuelist::numberToString(cue.number)));
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

    // its name and times come along; a new uid. Not its link nor its
    // actions: the copy would jump or start things a second time
    Cuelist::Cue copy = cl->cueAt(index);
    EditScope scope(this, id, tr("Copy cue %1").arg(Cuelist::numberToString(copy.number)));
    copy.uid = 0;
    copy.number = n;
    copy.link = 0;
    copy.linkCount = 0;
    copy.actions.clear();
    // DMX Total Control: in a Timecode list, a time as a new cue there
    if (cl->mode() == Cuelist::ModeTimecode)
        copy.time = newCueTime(cl, cl->insertIndexFor(n));
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

    EditScope scope(this, id, tr("Number of cue %1").arg(Cuelist::numberToString(cue.number)));
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
    // the wheel on the same cells: one step
    QStringList cells;
    if (fade >= 0)
        cells.append(QStringLiteral("fade"));
    if (downFade >= 0 || downFade == -2)
        cells.append(QStringLiteral("down"));
    if (delay >= 0)
        cells.append(QStringLiteral("delay"));
    EditScope scope(this, id, tr("Times"),
                    QStringLiteral("times:%1:%2").arg(cells.join(QLatin1Char(',')), uidsOf(all, indexes)));
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

void CuelistEditor::scaleCueTimes(quint32 id, int index, qreal factor)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount() || factor <= 0)
        return;

    auto scaled = [factor](qint64 ms) { return qBound(qint64(0), qRound64(qreal(ms) * factor), qint64(INT_MAX)); };
    // (a time of a part that is the cue's, -1, stays the cue's)
    auto scalePart = [&](Cuelist::PartTime &part) {
        if (part.fade >= 0)
            part.fade = int(scaled(part.fade));
        if (part.delay >= 0)
            part.delay = int(scaled(part.delay));
    };
    QList<Cuelist::Cue> all = cl->cues();
    Cuelist::Cue &cue = all[index];
    EditScope scope(this, id, tr("Live time of cue %1").arg(Cuelist::numberToString(cue.number)));
    cue.fade = uint(scaled(cue.fade));
    if (cue.downFade >= 0)
        cue.downFade = int(scaled(cue.downFade));
    cue.delay = uint(scaled(cue.delay));
    for (Cuelist::PartTime &part : cue.groupTimes)
        scalePart(part);
    for (Cuelist::PartTime &part : cue.fixtureTimes)
        scalePart(part);
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
    EditScope scope(this, id, tr("Trigger"),
                    QStringLiteral("trigger:%1%2:%3").arg(known ? QStringLiteral("type") : QString(),
                                                          time >= 0 ? QStringLiteral("time") : QString(),
                                                          uidsOf(all, indexes)));
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
    EditScope scope(this, id, tr("Name of cue %1").arg(Cuelist::numberToString(cue.number)),
                    QStringLiteral("name:%1").arg(cue.uid));
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
    EditScope scope(this, id, tr("Note of cue %1").arg(Cuelist::numberToString(cue.number)),
                    QStringLiteral("note:%1").arg(cue.uid));
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
    EditScope scope(this, id, tr("Skip and block"));
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

void CuelistEditor::setCueMib(quint32 id, const QVariantList &indexes, bool on)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr)
        return;

    QList<Cuelist::Cue> all = cl->cues();
    EditScope scope(this, id, tr("Move in black"));
    for (const QVariant &v : indexes)
    {
        const int index = v.toInt();
        if (index >= 0 && index < all.count())
            all[index].noMib = on == false;
    }
    cl->setCues(all);
    touch(id);
}

void CuelistEditor::setCueTime(quint32 id, int index, int ms)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return;
    Cuelist::Cue cue = cl->cueAt(index);
    // (the wheel on it: one step)
    EditScope scope(this, id, tr("Time of cue %1").arg(Cuelist::numberToString(cue.number)),
                    QStringLiteral("time:%1").arg(cue.uid));
    cue.time = qBound(-1, ms, Cuelist::maxCueTime());
    cl->replaceCue(cue);
    touch(id);
}

bool CuelistEditor::stampCueTime(quint32 id, int index)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return false;
    // (paused, the clock holds its time: that one)
    const qint64 clock = cl->status().clock;
    if (clock < 0)
        return false;
    setCueTime(id, index, int(qMin(clock, qint64(Cuelist::maxCueTime()))));
    return true;
}

/*********************************************************************
 * Times of attributes and fixtures
 *********************************************************************/

QVariantMap CuelistEditor::cueTimes(quint32 id, int index) const
{
    QVariantMap map;
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return map;

    const Cuelist::Cue cue = cl->cueAt(index);
    map.insert("fade", cue.fade);
    map.insert("downFade", cue.downFade);
    map.insert("delay", cue.delay);
    map.insert("curve", Cuelist::curveToString(cue.curve));
    QVariantList attributes;
    for (int group = 0; group < Cuelist::TimeGroupCount; group++)
    {
        const Cuelist::PartTime part = cue.groupTimes.value(group);
        QVariantMap row;
        row.insert("attribute", Cuelist::timeGroupToString(group));
        row.insert("fade", part.fade);
        row.insert("delay", part.delay);
        // "": the cue's
        row.insert("curve", part.curve >= 0 ? Cuelist::curveToString(part.curve) : QString());
        attributes.append(row);
    }
    map.insert("attributes", attributes);
    QVariantList fixtures;
    for (auto it = cue.fixtureTimes.cbegin(); it != cue.fixtureTimes.cend(); ++it)
    {
        const Fixture *fixture = m_doc->fixture(it.key());
        QVariantMap row;
        row.insert("fixture", it.key());
        row.insert("name", fixture != nullptr ? fixture->name() : QString("#%1").arg(it.key()));
        row.insert("fade", it.value().fade);
        row.insert("delay", it.value().delay);
        fixtures.append(row);
    }
    map.insert("fixtures", fixtures);
    map.insert("length", cue.length());
    return map;
}

void CuelistEditor::setAttributeTime(quint32 id, const QVariantList &indexes, const QString &attribute,
                                     int fade, int delay, const QString &curve)
{
    Cuelist *cl = cuelist(id);
    const int group = Cuelist::stringToTimeGroup(attribute);
    if (cl == nullptr || group < 0)
        return;

    // "cue": the cue's (-1)
    const bool curveSet = curve.isEmpty() == false;
    const int curveValue = curve == QLatin1String("cue") ? -1 : Cuelist::stringToCurve(curve);
    QString cells = partCells(fade, delay);
    if (curveSet)
        cells += cells.isEmpty() ? QStringLiteral("curve") : QStringLiteral(",curve");
    QList<Cuelist::Cue> all = cl->cues();
    EditScope scope(this, id, tr("Attribute times"),
                    QStringLiteral("attr:%1:%2:%3").arg(Cuelist::timeGroupToString(group), cells, uidsOf(all, indexes)));
    for (const QVariant &v : indexes)
    {
        const int index = v.toInt();
        if (index < 0 || index >= all.count())
            continue;
        // (a part with no time left goes: the list drops it)
        Cuelist::PartTime &part = all[index].groupTimes[group];
        changePart(part, fade, delay);
        if (curveSet)
            part.curve = curveValue;
    }
    cl->setCues(all);
    touch(id);
}

void CuelistEditor::setCueCurve(quint32 id, const QVariantList &indexes, const QString &curve)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr)
        return;

    const int value = Cuelist::stringToCurve(curve);
    QList<Cuelist::Cue> all = cl->cues();
    EditScope scope(this, id, tr("Curve"), QStringLiteral("curve:%1").arg(uidsOf(all, indexes)));
    for (const QVariant &v : indexes)
    {
        const int index = v.toInt();
        if (index >= 0 && index < all.count())
            all[index].curve = value;
    }
    cl->setCues(all);
    touch(id);
}

QList<quint32> CuelistEditor::fixtureIds(const QVariantList &fixtures) const
{
    QList<quint32> ids;
    for (const QVariant &v : fixtures)
    {
        bool ok = false;
        const quint32 fid = v.toUInt(&ok);
        if (ok && m_doc->fixture(fid) != nullptr && ids.contains(fid) == false)
            ids.append(fid);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

void CuelistEditor::addFixtureTimes(quint32 id, const QVariantList &indexes, const QVariantList &fixtures)
{
    Cuelist *cl = cuelist(id);
    const QList<quint32> ids = fixtureIds(fixtures);
    if (cl == nullptr || ids.isEmpty())
        return;

    QList<Cuelist::Cue> all = cl->cues();
    EditScope scope(this, id, tr("Fixture times"));
    for (const QVariant &v : indexes)
    {
        const int index = v.toInt();
        if (index < 0 || index >= all.count())
            continue;
        Cuelist::Cue &cue = all[index];
        for (quint32 fid : ids)
        {
            if (cue.fixtureTimes.contains(fid))
                continue;
            Cuelist::PartTime part;
            part.fade = int(qMin(cue.fade, 3600000u));
            part.delay = int(qMin(cue.delay, 3600000u));
            cue.fixtureTimes.insert(fid, part);
        }
    }
    cl->setCues(all);
    touch(id);
}

void CuelistEditor::setFixtureTimes(quint32 id, const QVariantList &indexes, const QVariantList &fixtures,
                                    int fade, int delay)
{
    Cuelist *cl = cuelist(id);
    const QList<quint32> ids = fixtureIds(fixtures);
    if (cl == nullptr || ids.isEmpty())
        return;

    QList<Cuelist::Cue> all = cl->cues();
    QStringList fixtureList;
    for (quint32 fid : ids)
        fixtureList.append(QString::number(fid));
    EditScope scope(this, id, tr("Fixture times"),
                    QStringLiteral("fixture:%1:%2:%3").arg(fixtureList.join(QLatin1Char(',')), partCells(fade, delay),
                                                            uidsOf(all, indexes)));
    for (const QVariant &v : indexes)
    {
        const int index = v.toInt();
        if (index < 0 || index >= all.count())
            continue;
        for (quint32 fid : ids)
            changePart(all[index].fixtureTimes[fid], fade, delay);
    }
    cl->setCues(all);
    touch(id);
}

void CuelistEditor::fanFixtureTimes(quint32 id, const QVariantList &indexes, const QVariantList &fixtures,
                                    const QString &what, int fromMs, int toMs, const QString &shape)
{
    Cuelist *cl = cuelist(id);
    const bool delay = what == QLatin1String("delay");
    if (cl == nullptr || (delay == false && what != QLatin1String("fade")))
        return;
    // the fixtures of the show, in the order given, once each
    QList<quint32> ids;
    for (const QVariant &v : fixtures)
    {
        bool ok = false;
        const quint32 fid = v.toUInt(&ok);
        if (ok && m_doc->fixture(fid) != nullptr && ids.contains(fid) == false)
            ids.append(fid);
    }
    if (ids.isEmpty())
        return;

    const QList<int> fan = fanTimes(int(ids.count()), fromMs, toMs, shape);
    QList<Cuelist::Cue> all = cl->cues();
    EditScope scope(this, id, tr("Time fan"));
    for (const QVariant &v : indexes)
    {
        const int index = v.toInt();
        if (index < 0 || index >= all.count())
            continue;
        // the other time of each fixture stays
        for (int i = 0; i < ids.count(); i++)
        {
            Cuelist::PartTime &part = all[index].fixtureTimes[ids.at(i)];
            (delay ? part.delay : part.fade) = fan.at(i);
        }
    }
    cl->setCues(all);
    touch(id);
}

void CuelistEditor::clearFixtureTimes(quint32 id, const QVariantList &indexes, const QVariantList &fixtures)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr)
        return;

    // (the IDs as they are: the times of a fixture deleted meanwhile go too)
    QSet<quint32> ids;
    for (const QVariant &v : fixtures)
        ids.insert(v.toUInt());
    QList<Cuelist::Cue> all = cl->cues();
    EditScope scope(this, id, tr("Fixture times"));
    for (const QVariant &v : indexes)
    {
        const int index = v.toInt();
        if (index < 0 || index >= all.count())
            continue;
        if (fixtures.isEmpty())
            all[index].fixtureTimes.clear();
        else
            all[index].fixtureTimes.removeIf(
                [&](QMap<quint32, Cuelist::PartTime>::iterator it) { return ids.contains(it.key()); });
    }
    cl->setCues(all);
    touch(id);
}

QVariantList CuelistEditor::cueFixtures(quint32 id, int index) const
{
    QVariantList list;
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return list;

    const Cuelist::Cue cue = cl->cueAt(index);
    QSet<quint32> ids;
    for (auto it = cue.values.cbegin(); it != cue.values.cend(); ++it)
        ids.insert(it.key().first);
    for (const Key &key : cue.releases)
        ids.insert(key.first);
    for (const FXScene::Layer &layer : cue.fx)
        for (quint32 fid : layer.fixtures)
            ids.insert(fid);
    for (const FxKey &key : cue.fxReleases)
        ids.insert(key.second);
    QList<quint32> sorted(ids.cbegin(), ids.cend());
    std::sort(sorted.begin(), sorted.end());
    for (quint32 fid : std::as_const(sorted))
        list.append(fid);
    return list;
}

/*********************************************************************
 * Link and actions
 *********************************************************************/

void CuelistEditor::setCueLink(quint32 id, int index, quint32 linkUid, int count)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return;
    if (linkUid != 0 && cl->indexOfUid(linkUid) < 0)
        return;

    Cuelist::Cue cue = cl->cueAt(index);
    // (the wheel on its times: one step)
    EditScope scope(this, id, tr("Link of cue %1").arg(Cuelist::numberToString(cue.number)),
                    QStringLiteral("link:%1").arg(cue.uid));
    cue.link = linkUid;
    cue.linkCount = linkUid != 0 ? qBound(0, count, 999) : 0;
    cl->replaceCue(cue);
    touch(id);
}

QVariantList CuelistEditor::cueActions(quint32 id, int index) const
{
    QVariantList list;
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return list;

    for (const Cuelist::CueAction &action : cl->cueAt(index).actions)
    {
        QVariantMap map = actionToMap(action);
        map.insert("text", describeAction(action));
        list.append(map);
    }
    return list;
}

void CuelistEditor::setCueActions(quint32 id, int index, const QVariantList &actions)
{
    Cuelist *cl = cuelist(id);
    if (cl == nullptr || index < 0 || index >= cl->cueCount())
        return;

    Cuelist::Cue cue = cl->cueAt(index);
    // (a BPM on the wheel: one step)
    EditScope scope(this, id, tr("Actions of cue %1").arg(Cuelist::numberToString(cue.number)),
                    QStringLiteral("actions:%1").arg(cue.uid));
    cue.actions.clear();
    for (const QVariant &v : actions)
    {
        const Cuelist::CueAction action = actionFrom(v.toMap());
        if (action.isEmpty() == false)
            cue.actions.append(action);
    }
    cl->replaceCue(cue);
    touch(id);
}

QVariantMap CuelistEditor::actionToMap(const Cuelist::CueAction &action)
{
    static const QSet<QString> numbers = { "functionId", "cue", "cueUid", "zoneId", "slot", "value" };
    QVariantMap map;
    for (auto it = action.cbegin(); it != action.cend(); ++it)
    {
        if (numbers.contains(it.key()) == false)
            map.insert(it.key(), it.value());
        else if (it.key() == QLatin1String("cueUid"))
            map.insert(it.key(), it.value().toUInt());
        else
            map.insert(it.key(), it.value().toInt());
    }
    return map;
}

Cuelist::CueAction CuelistEditor::actionFrom(const QVariantMap &map)
{
    const QString target = map.value("target").toString();
    auto number = [&](const char *key, int fallback) {
        bool ok = false;
        const int value = map.value(QLatin1String(key)).toInt(&ok);
        return ok ? value : fallback;
    };

    Cuelist::CueAction action;
    auto put = [&](const char *key, const QString &value) { action.insert(QLatin1String(key), value); };
    if (target == QLatin1String("playbackGo") || target == QLatin1String("playbackGoTo") ||
        target == QLatin1String("playbackRelease"))
    {
        // a function wherever it is, never a slot of the current page
        put("target", target);
        put("functionId", QString::number(qMax(-1, number("functionId", -1))));
        put("slot", QStringLiteral("-1"));
        if (target == QLatin1String("playbackGoTo"))
        {
            // the cue by its uid, as the SHOW buttons: cues recorded or
            // deleted before it don't move it
            put("cue", QString::number(qMax(0, number("cue", 0))));
            const uint uid = map.value("cueUid").toUInt();
            if (uid != 0)
                put("cueUid", QString::number(uid));
        }
    }
    else if (target == QLatin1String("scene"))
    {
        put("target", target);
        put("functionId", QString::number(qMax(-1, number("functionId", -1))));
        put("mode", map.value("mode").toString() == QLatin1String("off") ? QStringLiteral("off") : QStringLiteral("on"));
    }
    else if (target == QLatin1String("groupRelease"))
    {
        // -1: all the groups, -2: a group that is gone
        put("target", target);
        put("zoneId", QString::number(qMax(-2, number("zoneId", -1))));
    }
    else if (target == QLatin1String("playbackReleaseAll"))
    {
        put("target", target);
    }
    else if (target == QLatin1String("bpm"))
    {
        put("target", target);
        put("value", QString::number(qBound(20, number("value", 120), 300)));
    }
    return action;
}

QString CuelistEditor::describeAction(const Cuelist::CueAction &action) const
{
    const QString target = action.value("target");
    const int fid = action.value("functionId", "-1").toInt();
    Function *f = fid >= 0 ? m_doc->function(quint32(fid)) : nullptr;
    const QString name = f != nullptr ? f->name() : tr("(deleted)");

    if (target == QLatin1String("playbackGo"))
        return tr("GO of %1").arg(name);
    if (target == QLatin1String("playbackGoTo"))
    {
        // the cue by its uid (gone: none), else by its place
        const Cuelist *list = qobject_cast<const Cuelist *>(f);
        const quint32 uid = action.value("cueUid").toUInt();
        const int index = list == nullptr ? -1 : uid != 0 ? list->indexOfUid(uid) : action.value("cue").toInt();
        const Cuelist::Cue cue = list != nullptr ? list->cueAt(index) : Cuelist::Cue();
        return tr("Go to cue %1 of %2").arg(cue.uid != 0 ? Cuelist::numberToString(cue.number) : QStringLiteral("?"), name);
    }
    if (target == QLatin1String("playbackRelease"))
        return tr("Release %1").arg(name);
    if (target == QLatin1String("scene"))
        return action.value("mode") == QLatin1String("off") ? tr("Stop %1").arg(name) : tr("Start %1").arg(name);
    if (target == QLatin1String("groupRelease"))
    {
        const int zoneId = action.value("zoneId", "-1").toInt();
        if (zoneId == -1)
            return tr("Release all the groups");
        Zone *zone = zoneId >= 0 && m_zones != nullptr ? m_zones->zoneById(quint32(zoneId)) : nullptr;
        return zone != nullptr ? tr("Release the group %1").arg(zone->name()) : tr("Release a deleted group");
    }
    if (target == QLatin1String("playbackReleaseAll"))
        return tr("Release all playbacks");
    if (target == QLatin1String("bpm"))
        return tr("BPM %1").arg(action.value("value").toInt());
    return target;
}

void CuelistEditor::forgetActionsOn(const std::function<bool(quint32)> &gone)
{
    for (Function *f : m_doc->functionsByType(Function::CuelistType))
    {
        Cuelist *cl = qobject_cast<Cuelist *>(f);
        if (cl == nullptr)
            continue;
        // (read first: the cues are copied only when an action names one)
        if (namesGone(cl->cues(), gone))
        {
            QList<Cuelist::Cue> cues = cl->cues();
            forgetFunctions(cues, gone);
            cl->setCues(cues);
            // (the show changed already: a function went, or it was loaded)
            emit edited(cl->id());
            if (cl->id() == m_editId)
                emit editingChanged();
        }
        // an undo doesn't bring the old ID back either (the steps that don't
        // name one stay shared)
        for (QHash<quint32, QList<UndoEntry>> *history : { &m_undo, &m_redo })
        {
            auto it = history->find(cl->id());
            if (it == history->end())
                continue;
            for (qsizetype i = 0; i < it->count(); i++)
            {
                const UndoEntry &entry = std::as_const(*it).at(i);
                const bool before = namesGone(entry.before.cues, gone);
                const bool after = namesGone(entry.after.cues, gone);
                if (before)
                    forgetFunctions((*it)[i].before.cues, gone);
                if (after)
                    forgetFunctions((*it)[i].after.cues, gone);
            }
        }
    }
}

void CuelistEditor::afterLoad()
{
    forgetActionsOn([this](quint32 fid) { return m_doc->function(fid) == nullptr; });
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

int CuelistEditor::convertFromSteps(quint32 stepsId)
{
    // a Sequence or a Chaser (of a QLC+ show)
    Chaser *chaser = qobject_cast<Chaser *>(m_doc->function(stepsId));
    if (chaser == nullptr)
        return -1;
    QlcExchange::StepsImport steps;
    QString error;
    if (QlcExchange::stepsToCues(m_doc, chaser, m_doc->masterTimer()->beatTimeDuration(), &steps, &error) == false)
    {
        if (error.isEmpty() == false)
            emit message(error);
        return -1;
    }

    if (replaceSteps(chaser, QlcExchange::newCuelist(m_doc, chaser, steps)) == false)
        return -1;

    // what doesn't play the same (a random order, beats, steps that are no scenes)
    if (steps.warnings.isEmpty() == false)
        emit message(tr("Converted to a cuelist: %1").arg(steps.warnings.join(QStringLiteral("; "))));
    return int(stepsId);
}

bool CuelistEditor::replaceSteps(Chaser *chaser, Cuelist *cl)
{
    // stopped wherever it plays (also when started in this frame, not run
    // yet); the super scenes and chasers that play the steps play the
    // cuelist instead (Doc::functionReplaced), and the Doc deletes the steps
    // only once the timer holds them no more
    if (chaser->stopped() == false || chaser->isRunning())
        chaser->stopAndWait();

    // the same ID: groups, SHOW buttons and mappings play the cuelist now
    const quint32 stepsId = chaser->id();
    Sequence *sequence = qobject_cast<Sequence *>(chaser);
    const quint32 boundId = sequence != nullptr ? sequence->boundSceneID() : Function::invalidId();
    if (m_doc->replaceFunction(stepsId, cl) == false)
    {
        delete cl;
        return false;
    }
    // the hidden channels of the steps go with them, unless they are a
    // scene of the user or other steps use them too (QLC+ shows, copies).
    // The scenes of a chaser stay
    if (sequence != nullptr && boundId != stepsId)
        EffectEditor::deleteBoundScene(m_doc, boundId);

    touch(stepsId);
    return true;
}

/*********************************************************************
 * Undo
 *********************************************************************/

CuelistEditor::EditScope::EditScope(CuelistEditor *editor, quint32 id, const QString &label, const QString &mergeKey)
    : m_editor(editor)
    , m_outer(editor->m_scopeDepth == 0)
{
    editor->m_scopeDepth++;
    if (m_outer == false)
        return;
    editor->m_scopeId = id;
    editor->m_scopeLabel = label;
    editor->m_scopeMerge = mergeKey;
    editor->m_scopeBefore = editor->undoStateOf(id);
}

CuelistEditor::EditScope::~EditScope()
{
    m_editor->m_scopeDepth--;
    if (m_outer)
        m_editor->commitUndo();
}

void CuelistEditor::EditScope::setLabel(const QString &label)
{
    if (m_outer)
        m_editor->m_scopeLabel = label;
}

CuelistEditor::UndoState CuelistEditor::undoStateOf(quint32 id) const
{
    UndoState state;
    if (Cuelist *cl = cuelist(id))
    {
        state.cues = cl->cues();
        state.options = options(id);
    }
    return state;
}

void CuelistEditor::commitUndo()
{
    const quint32 id = std::exchange(m_scopeId, Function::invalidId());
    const UndoState before = std::exchange(m_scopeBefore, UndoState());
    if (cuelist(id) == nullptr)
        return;
    // nothing changed (a Record that asks first, an Update with the same values)
    const UndoState after = undoStateOf(id);
    if (after == before)
        return;

    QList<UndoEntry> &undo = m_undo[id];
    const qint64 now = m_undoClock.elapsed();
    if (m_scopeMerge.isEmpty() == false && undo.isEmpty() == false && undo.last().mergeKey == m_scopeMerge &&
        m_redo.value(id).isEmpty() && now - undo.last().at < kUndoMergeMs)
    {
        // the same cells again: one step, gone when they are back as they were
        UndoEntry &last = undo.last();
        last.after = after;
        last.at = now;
        if (last.before == last.after)
            undo.removeLast();
    }
    else
    {
        UndoEntry entry;
        entry.label = m_scopeLabel;
        entry.mergeKey = m_scopeMerge;
        entry.at = now;
        entry.before = before;
        entry.after = after;
        undo.append(entry);
        while (undo.count() > kUndoSteps)
            undo.removeFirst();
    }
    if (undo.isEmpty())
        m_undo.remove(id);
    m_redo.remove(id);
    emit undoChanged(id);
}

QVariantMap CuelistEditor::undoState(quint32 id) const
{
    const QList<UndoEntry> undo = m_undo.value(id);
    const QList<UndoEntry> redo = m_redo.value(id);
    QVariantMap map;
    map.insert("canUndo", undo.isEmpty() == false);
    map.insert("canRedo", redo.isEmpty() == false);
    map.insert("undo", undo.isEmpty() ? QString() : undo.last().label);
    map.insert("redo", redo.isEmpty() ? QString() : redo.last().label);
    return map;
}

int CuelistEditor::undo(quint32 id)
{
    return step(id, false);
}

int CuelistEditor::redo(quint32 id)
{
    return step(id, true);
}

int CuelistEditor::step(quint32 id, bool redo)
{
    QHash<quint32, QList<UndoEntry>> &from = redo ? m_redo : m_undo;
    auto it = from.find(id);
    if (it == from.end() || it->isEmpty() || cuelist(id) == nullptr)
        return -1;
    UndoEntry entry = it->takeLast();
    if (it->isEmpty())
        from.erase(it);
    // a step undone or redone is closed: the next edit is a step of its own
    entry.mergeKey.clear();
    (redo ? m_undo : m_redo)[id].append(entry);

    const int index = restore(id, entry, redo);
    emit undoChanged(id);
    return index;
}

int CuelistEditor::restore(quint32 id, const UndoEntry &entry, bool redo)
{
    Cuelist *cl = cuelist(id);
    const UndoState &target = redo ? entry.after : entry.before;

    int changed = -1;
    if (entry.before.cues != entry.after.cues)
    {
        // what changed outside the editor since then stays: deleted
        // fixtures don't come back, palettes keep their values of now
        QList<Cuelist::Cue> cues = target.cues;
        pruneMissing(cues);
        refreshPalettes(cues);
        const QList<Cuelist::Cue> was = cl->cues();
        cl->setCues(cues);
        changed = firstChange(was, cl->cues());
    }
    // only the options of this step: a priority or a release time changed
    // since then (Live, the playbacks) stays
    for (auto it = target.options.cbegin(); it != target.options.cend(); ++it)
    {
        if (entry.before.options.value(it.key()) != entry.after.options.value(it.key()))
            applyOption(cl, it.key(), it.value());
    }

    // the cue in the programmer may be gone
    if (m_editId == id && cl->indexOfUid(m_editUid) < 0)
        stopEditing();
    touch(id);
    return changed;
}

void CuelistEditor::pruneMissing(QList<Cuelist::Cue> &cues) const
{
    // as a show loads (Cuelist::postLoad)
    auto gone = [this](quint32 fixture) { return m_doc->fixture(fixture) == nullptr; };
    auto missing = [this](const Key &key) {
        Fixture *fixture = m_doc->fixture(key.first);
        return fixture == nullptr || key.second >= fixture->channels();
    };
    for (Cuelist::Cue &cue : cues)
    {
        cue.values.removeIf([&](const QHash<Key, uchar>::iterator &it) { return missing(it.key()); });
        cue.palettes.removeIf([&](const QHash<Key, int>::iterator &it) { return missing(it.key()); });
        cue.releases.removeIf(missing);
        for (int l = int(cue.fx.count()) - 1; l >= 0; l--)
        {
            FXScene::Layer &layer = cue.fx[l];
            if (layer.fixtures.removeIf(gone) > 0)
                layer.own = layer.tidyOwn();
            if (layer.fixtures.isEmpty())
                cue.fx.removeAt(l);
        }
        cue.fxReleases.removeIf([&](const FxKey &key) { return gone(key.second); });
        cue.fixtureTimes.removeIf([&](QMap<quint32, Cuelist::PartTime>::iterator it) { return gone(it.key()); });
    }
    // the actions on functions that are gone do nothing, as after a load
    forgetFunctions(cues, [this](quint32 fid) { return m_doc->function(fid) == nullptr; });
}

void CuelistEditor::refreshPalettes(QList<Cuelist::Cue> &cues) const
{
    if (m_palettes == nullptr)
        return;
    // (palette, fixture) -> channel -> value; empty: the palette is gone or
    // doesn't apply to the fixture
    QHash<QPair<int, quint32>, QMap<quint32, uchar>> values;
    for (Cuelist::Cue &cue : cues)
    {
        for (auto it = cue.palettes.begin(); it != cue.palettes.end();)
        {
            const QPair<int, quint32> of(it.value(), it.key().first);
            auto found = values.constFind(of);
            if (found == values.cend())
                found = values.insert(of, m_palettes->valuesFor(of.first, of.second));
            if (found.value().contains(it.key().second) == false)
            {
                // a plain value now, as when the palette is deleted
                it = cue.palettes.erase(it);
                continue;
            }
            cue.values.insert(it.key(), found.value().value(it.key().second));
            ++it;
        }
    }
}

void CuelistEditor::dropUndo(quint32 id)
{
    const bool had = m_undo.remove(id) || m_redo.contains(id);
    m_redo.remove(id);
    if (had)
        emit undoChanged(id);
}
