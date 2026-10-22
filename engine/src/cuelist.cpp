/*
  DMX Total Control
  cuelist.cpp

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
#include <QMutexLocker>
#include <QDebug>
#include <QIcon>
#include <algorithm>
#include <climits>

#include "qlcfixturemode.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "qlcchannel.h"
#include "universe.h"
#include "cuelist.h"
#include "fixture.h"
#include "doc.h"

Cuelist::Cuelist(Doc *doc)
    : Function(doc, Function::CuelistType)
    , m_nextUid(1)
    , m_tracking(true)
    , m_atEnd(AtEndLoop)
    , m_resetOnRelease(true)
    , m_backTime(-1)
    , m_releaseOnNextGo(false)
    , m_preselectUid(0)
    , m_startUid(0)
    , m_lastUid(0)
    , m_currentUid(0)
    , m_current(-1)
    , m_cueElapsed(0)
    , m_cueLength(0)
    , m_fadeLength(0)
    , m_seekRemainder(0)
    , m_followArmed(false)
    , m_firstOfRun(true)
    , m_dataDirty(false)
    , m_releaseRequested(false)
    , m_fadersPaused(false)
    , m_appliedIntensity(-1)
    , m_fxPlayer(nullptr)
{
    setName(tr("New Cuelist"));
    // the fade of new cues, and the release time
    setFadeInSpeed(1000);
    setFadeOutSpeed(1000);

    // the FX of the cues play in a scene of its own, not in the show
    m_fxPlayer = new FXScene(doc);
    m_fxPlayer->setName(name() + QStringLiteral(" (FX)"));
}

Cuelist::~Cuelist()
{
    // the player runs only inside our own frames, and our postRun stops it
    // in the same frame: once the list has stopped (stopAndWait before a
    // delete, MasterTimer::stop before a clear) nothing holds it, and there
    // is nothing to wait for. A list deleted without a release (driven by
    // no timer thread) leaves no FX faders behind either. DMX Total Control:
    // nor FX it was fading out, held by a pause
    m_fxPlayer->setFadersPaused(false);
    if (m_fxPlayer->isRunning())
    {
        m_fxPlayer->setFadeOutSpeed(0);
        m_fxPlayer->postRun(doc() != NULL ? doc()->masterTimer() : NULL, QList<Universe *>());
    }
    delete m_fxPlayer;
}

QIcon Cuelist::getIcon() const
{
    return QIcon(":/sequence.png");
}

quint32 Cuelist::totalDuration()
{
    QMutexLocker locker(&m_mutex);
    if (m_cues.isEmpty())
        return 0;

    // the time from cue $from to cue $to, false when $to waits for a GO
    auto gap = [this](int from, int to, quint64 *ms)
    {
        const Cue &next = m_cues.at(to);
        if (next.trigger == TriggerFollow)
            *ms += quint64(m_cues.at(from).length()) + next.triggerTime;
        else if (next.trigger == TriggerWait)
            *ms += next.triggerTime;
        return next.trigger != TriggerGo;
    };

    quint64 total = 0;
    for (int i = 1; i < m_cues.count(); i++)
    {
        if (gap(i - 1, i, &total) == false)
            return infiniteSpeed();
    }

    // DMX Total Control: one cycle back to the first cue, or a Release list
    // that releases itself after the last cue: with the first cue's trigger
    if (m_atEnd == AtEndLoop || (m_atEnd == AtEndRelease && m_cues.first().trigger != TriggerGo))
    {
        if (gap(m_cues.count() - 1, 0, &total) == false)
            return infiniteSpeed();
    }
    else
    {
        total += m_cues.last().length();
    }
    return total >= quint64(infiniteSpeed()) ? infiniteSpeed() : quint32(total);
}

bool Cuelist::Cue::operator==(const Cue &o) const
{
    return uid == o.uid && number == o.number && name == o.name && note == o.note &&
           trigger == o.trigger && triggerTime == o.triggerTime && fade == o.fade &&
           downFade == o.downFade && delay == o.delay && skip == o.skip && block == o.block &&
           values == o.values && palettes == o.palettes && releases == o.releases &&
           fx == o.fx && fxReleases == o.fxReleases;
}

bool Cuelist::State::operator==(const State &o) const
{
    return values == o.values && owner == o.owner && palettes == o.palettes &&
           fx == o.fx && fxOwner == o.fxOwner;
}

/*********************************************************************
 * Copying
 *********************************************************************/

Function *Cuelist::createCopy(Doc *doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function *copy = new Cuelist(doc);
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

bool Cuelist::copyFrom(const Function *function)
{
    const Cuelist *cl = qobject_cast<const Cuelist *>(function);
    if (cl == NULL)
        return false;

    if (cl != this)
    {
        QMutexLocker locker(&m_mutex);
        QMutexLocker otherLocker(&cl->m_mutex);
        // the uids and the palette references too
        m_cues = cl->m_cues;
        m_nextUid = cl->m_nextUid;
        m_tracking = cl->m_tracking;
        m_atEnd = cl->m_atEnd;
        m_resetOnRelease = cl->m_resetOnRelease;
        m_backTime = cl->m_backTime;
        m_releaseOnNextGo = cl->m_releaseOnNextGo;
        m_dataDirty = true;
        followEditsLocked();
    }

    return Function::copyFrom(function);
}

/*********************************************************************
 * Cue numbers
 *********************************************************************/

int Cuelist::parseNumber(const QString &text)
{
    const QString trimmed = text.trimmed();
    int whole = 0;
    int fraction = 0;
    int digits = 0;
    int decimals = 0;
    bool point = false;

    for (const QChar ch : trimmed)
    {
        const char16_t c = ch.unicode();
        if (c == u'.' || c == u',')
        {
            if (point)
                return -1;
            point = true;
            continue;
        }
        if (c < u'0' || c > u'9')
            return -1;

        const int digit = int(c - u'0');
        if (point)
        {
            if (++decimals > 3)
                return -1;
            fraction = fraction * 10 + digit;
        }
        else
        {
            whole = whole * 10 + digit;
            if (whole > maxNumber() / 1000)
                return -1;
            digits++;
        }
    }
    if (digits + decimals == 0)
        return -1;

    for (int d = decimals; d < 3; d++)
        fraction *= 10;
    const int number = whole * 1000 + fraction;
    return number >= 1 && number <= maxNumber() ? number : -1;
}

QString Cuelist::numberToString(int number)
{
    if (number < 0)
        return QString();

    QString text = QString::number(number / 1000);
    const int fraction = number % 1000;
    if (fraction != 0)
    {
        QString decimals = QString::number(fraction).rightJustified(3, QLatin1Char('0'));
        while (decimals.endsWith(QLatin1Char('0')))
            decimals.chop(1);
        text += QLatin1Char('.') + decimals;
    }
    return text;
}

QStringList Cuelist::fxAxes(const QString &param)
{
    if (param == QLatin1String("pantilt"))
        return { QStringLiteral("pan"), QStringLiteral("tilt") };
    return { param };
}

QSet<Cuelist::FxKey> Cuelist::fxKeysOf(const FXScene::Layer &layer)
{
    QSet<FxKey> keys;
    const QStringList axes = fxAxes(layer.param);
    for (quint32 fixture : layer.fixtures)
        for (const QString &axis : axes)
            keys.insert(FxKey(axis, fixture));
    return keys;
}

/*********************************************************************
 * Contents
 *********************************************************************/

int Cuelist::cueCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_cues.count();
}

QList<Cuelist::Cue> Cuelist::cues() const
{
    QMutexLocker locker(&m_mutex);
    return m_cues;
}

Cuelist::Cue Cuelist::cueAt(int index) const
{
    QMutexLocker locker(&m_mutex);
    return index >= 0 && index < m_cues.count() ? m_cues.at(index) : Cue();
}

int Cuelist::indexOfUid(quint32 uid) const
{
    QMutexLocker locker(&m_mutex);
    return indexOfUidLocked(uid);
}

int Cuelist::indexOfNumber(int number) const
{
    QMutexLocker locker(&m_mutex);
    return indexOfNumberLocked(number);
}

int Cuelist::insertIndexFor(int number) const
{
    QMutexLocker locker(&m_mutex);
    return insertIndexLocked(number);
}

int Cuelist::indexOfUidLocked(quint32 uid) const
{
    if (uid == 0)
        return -1;
    for (int i = 0; i < m_cues.count(); i++)
    {
        if (m_cues.at(i).uid == uid)
            return i;
    }
    return -1;
}

int Cuelist::indexOfNumberLocked(int number) const
{
    const int i = insertIndexLocked(number);
    return i < m_cues.count() && m_cues.at(i).number == number ? i : -1;
}

int Cuelist::insertIndexLocked(int number) const
{
    // the first cue that doesn't come before it (the list is sorted)
    auto it = std::lower_bound(m_cues.cbegin(), m_cues.cend(), number,
                               [](const Cue &cue, int n) { return cue.number < n; });
    return int(it - m_cues.cbegin());
}

int Cuelist::addCue(const Cue &cue)
{
    if (cue.number < 1 || cue.number > maxNumber())
        return -1;

    int index = -1;
    {
        QMutexLocker locker(&m_mutex);
        if (indexOfNumberLocked(cue.number) >= 0)
            return -1;

        Cue added = cue;
        sanitize(added);
        if (added.uid == 0 || indexOfUidLocked(added.uid) >= 0)
            added.uid = m_nextUid++;
        else
            m_nextUid = qMax(m_nextUid, added.uid + 1);

        index = insertIndexLocked(added.number);
        m_cues.insert(index, added);
        m_dataDirty = true;
        followEditsLocked();
    }
    emit changed(id());
    return index;
}

bool Cuelist::replaceCue(const Cue &cue)
{
    if (cue.number < 1 || cue.number > maxNumber())
        return false;

    {
        QMutexLocker locker(&m_mutex);
        const int from = indexOfUidLocked(cue.uid);
        if (from < 0)
            return false;
        const int sameNumber = indexOfNumberLocked(cue.number);
        if (sameNumber >= 0 && sameNumber != from)
            return false;

        Cue replaced = cue;
        sanitize(replaced);
        if (replaced == m_cues.at(from))
            return true;

        // a new number moves it
        m_cues.removeAt(from);
        m_cues.insert(insertIndexLocked(replaced.number), replaced);
        m_dataDirty = true;
        followEditsLocked();
    }
    emit changed(id());
    return true;
}

bool Cuelist::removeCue(quint32 uid)
{
    {
        QMutexLocker locker(&m_mutex);
        const int index = indexOfUidLocked(uid);
        if (index < 0)
            return false;
        m_cues.removeAt(index);
        m_dataDirty = true;
        followEditsLocked();
    }
    emit changed(id());
    return true;
}

void Cuelist::setCues(const QList<Cue> &cues)
{
    {
        QMutexLocker locker(&m_mutex);
        QList<Cue> list = cues;
        normalizeLocked(list);
        if (list == m_cues)
            return;
        m_cues = list;
        m_dataDirty = true;
        followEditsLocked();
    }
    emit changed(id());
}

void Cuelist::normalizeLocked(QList<Cue> &cues)
{
    // sorted by number; of two cues with the same number the first one stays
    std::stable_sort(cues.begin(), cues.end(), [](const Cue &a, const Cue &b) { return a.number < b.number; });

    QList<Cue> kept;
    for (const Cue &cue : std::as_const(cues))
    {
        if (cue.number < 1 || cue.number > maxNumber())
        {
            qWarning() << "Cuelist" << name() << ": cue number" << cue.number << "is not valid, cue dropped";
            continue;
        }
        if (kept.isEmpty() == false && kept.last().number == cue.number)
        {
            qWarning() << "Cuelist" << name() << ": cue" << numberToString(cue.number) << "twice, the second one dropped";
            continue;
        }
        kept.append(cue);
    }

    // uids are unique: a cue without one, or with the uid of a cue before
    // it, gets a new one past every uid of the list
    QSet<quint32> used;
    QList<int> fresh;
    for (int i = 0; i < kept.count(); i++)
    {
        Cue &cue = kept[i];
        sanitize(cue);
        if (cue.uid == 0 || used.contains(cue.uid))
        {
            fresh.append(i);
            continue;
        }
        used.insert(cue.uid);
        m_nextUid = qMax(m_nextUid, cue.uid + 1);
    }
    for (int i : std::as_const(fresh))
        kept[i].uid = m_nextUid++;

    cues = kept;
}

void Cuelist::sanitize(Cue &cue)
{
    // a value wins over a release of the same channel
    for (auto it = cue.values.cbegin(); it != cue.values.cend(); ++it)
        cue.releases.remove(it.key());

    // palettes only refer to values
    for (auto it = cue.palettes.begin(); it != cue.palettes.end();)
    {
        if (cue.values.contains(it.key()))
            ++it;
        else
            it = cue.palettes.erase(it);
    }

    // FX layers have a parameter and their own fixtures, once each
    for (int l = cue.fx.count() - 1; l >= 0; l--)
    {
        FXScene::Layer &layer = cue.fx[l];
        QList<quint32> unique;
        for (quint32 fixture : std::as_const(layer.fixtures))
            if (unique.contains(fixture) == false)
                unique.append(fixture);
        if (unique.count() != layer.fixtures.count())
            layer.fixtures = unique;
        if (layer.param.isEmpty() || layer.fixtures.isEmpty())
            cue.fx.removeAt(l);
    }

    // an FX wins over a release of the same slot
    for (const FXScene::Layer &layer : std::as_const(cue.fx))
        for (const FxKey &key : fxKeysOf(layer))
            cue.fxReleases.remove(key);

    if (cue.downFade < -1)
        cue.downFade = -1;
}

bool Cuelist::removeFixturesLocked(const std::function<bool(quint32)> &remove)
{
    bool removed = false;
    for (Cue &cue : m_cues)
    {
        for (auto it = cue.values.begin(); it != cue.values.end();)
        {
            if (remove(it.key().first))
            {
                it = cue.values.erase(it);
                removed = true;
            }
            else
            {
                ++it;
            }
        }
        for (auto it = cue.palettes.begin(); it != cue.palettes.end();)
        {
            if (remove(it.key().first))
                it = cue.palettes.erase(it);
            else
                ++it;
        }
        for (auto it = cue.releases.begin(); it != cue.releases.end();)
        {
            if (remove(it->first))
            {
                it = cue.releases.erase(it);
                removed = true;
            }
            else
            {
                ++it;
            }
        }
        for (int l = cue.fx.count() - 1; l >= 0; l--)
        {
            FXScene::Layer &layer = cue.fx[l];
            if (layer.fixtures.removeIf(remove) > 0)
            {
                removed = true;
                layer.own = layer.tidyOwn();
            }
            if (layer.fixtures.isEmpty())
                cue.fx.removeAt(l);
        }
        for (auto it = cue.fxReleases.begin(); it != cue.fxReleases.end();)
        {
            if (remove(it->second))
            {
                it = cue.fxReleases.erase(it);
                removed = true;
            }
            else
            {
                ++it;
            }
        }
    }
    return removed;
}

/*********************************************************************
 * Tracking
 *********************************************************************/

namespace
{
/** An FX layer of a cue: (cue uid, layer index) */
typedef QPair<quint32, int> FxOrigin;

/** A layer, and the fixtures it still holds */
struct FxHeld
{
    FXScene::Layer layer;
    QSet<quint32> owned;
};

/** The fold of the cues: the state, and which layer holds each FX slot */
struct Fold
{
    Cuelist::State state;
    QList<FxOrigin> order;
    QHash<FxOrigin, FxHeld> layers;
    QHash<Cuelist::FxKey, FxOrigin> holders;

    /** A layer that loses one slot of a fixture loses the whole fixture: a layer can't be split */
    void loseFixture(const FxOrigin &origin, quint32 fixture)
    {
        auto it = layers.find(origin);
        if (it == layers.end())
            return;
        it->owned.remove(fixture);
        for (const QString &axis : Cuelist::fxAxes(it->layer.param))
        {
            auto held = holders.find(Cuelist::FxKey(axis, fixture));
            if (held != holders.end() && held.value() == origin)
                holders.erase(held);
        }
    }

    void apply(const Cuelist::Cue &cue)
    {
        for (const Cuelist::Key &key : cue.releases)
        {
            state.values.remove(key);
            state.owner.remove(key);
            state.palettes.remove(key);
        }
        for (auto it = cue.values.cbegin(); it != cue.values.cend(); ++it)
        {
            state.values.insert(it.key(), it.value());
            state.owner.insert(it.key(), cue.uid);
            auto palette = cue.palettes.constFind(it.key());
            if (palette != cue.palettes.cend())
                state.palettes.insert(it.key(), palette.value());
            else
                state.palettes.remove(it.key());
        }

        for (const Cuelist::FxKey &key : cue.fxReleases)
        {
            auto held = holders.constFind(key);
            if (held != holders.cend())
                loseFixture(FxOrigin(held.value()), key.second);
        }
        for (int l = 0; l < cue.fx.count(); l++)
        {
            const FXScene::Layer &layer = cue.fx.at(l);
            const FxOrigin origin(cue.uid, l);
            const QStringList axes = Cuelist::fxAxes(layer.param);
            FxHeld held;
            held.layer = layer;
            for (quint32 fixture : layer.fixtures)
            {
                for (const QString &axis : axes)
                {
                    const Cuelist::FxKey key(axis, fixture);
                    auto other = holders.constFind(key);
                    if (other != holders.cend() && other.value() != origin)
                        loseFixture(FxOrigin(other.value()), fixture);
                    holders.insert(key, origin);
                }
                held.owned.insert(fixture);
            }
            layers.insert(origin, held);
            order.append(origin);
        }
    }

    /** The state, with each tracked layer on the fixtures it still holds (in its order) */
    Cuelist::State finish()
    {
        for (const FxOrigin &origin : std::as_const(order))
        {
            const FxHeld held = layers.value(origin);
            if (held.owned.isEmpty())
                continue;
            FXScene::Layer layer = held.layer;
            layer.fixtures.clear();
            for (quint32 fixture : held.layer.fixtures)
                if (held.owned.contains(fixture))
                    layer.fixtures.append(fixture);
            layer.own = layer.tidyOwn();
            state.fx.append(layer);
        }
        for (auto it = holders.cbegin(); it != holders.cend(); ++it)
            state.fxOwner.insert(it.key(), it.value().first);
        return state;
    }
};
}

Cuelist::State Cuelist::foldLocked(int index, bool withCue) const
{
    Fold fold;
    const int count = m_cues.count();
    // a skip cue (cue only) doesn't track on
    if (m_tracking)
    {
        for (int i = 0; i < qMin(index, count); i++)
        {
            if (m_cues.at(i).skip == false)
                fold.apply(m_cues.at(i));
        }
    }
    // a cue always shows itself, skip included
    if (withCue && index >= 0 && index < count)
        fold.apply(m_cues.at(index));
    return fold.finish();
}

Cuelist::State Cuelist::stateBefore(int index) const
{
    QMutexLocker locker(&m_mutex);
    return foldLocked(index, false);
}

Cuelist::State Cuelist::trackedState(int index) const
{
    QMutexLocker locker(&m_mutex);
    if (index < 0 || index >= m_cues.count())
        return State();
    return foldLocked(index, true);
}

/*********************************************************************
 * Palettes
 *********************************************************************/

void Cuelist::applyPalette(int paletteId, const std::function<int(quint32, quint32)> &valueOf)
{
    if (!valueOf)
        return;

    // the channels that use it, then their values outside the lock (the
    // caller reads its own palettes)
    QSet<Key> keys;
    {
        QMutexLocker locker(&m_mutex);
        for (const Cue &cue : std::as_const(m_cues))
            for (auto it = cue.palettes.cbegin(); it != cue.palettes.cend(); ++it)
                if (it.value() == paletteId)
                    keys.insert(it.key());
    }
    if (keys.isEmpty())
        return;

    QHash<Key, int> newValues;
    for (const Key &key : std::as_const(keys))
        newValues.insert(key, valueOf(key.first, key.second));

    bool modified = false;
    {
        QMutexLocker locker(&m_mutex);
        for (Cue &cue : m_cues)
        {
            for (auto it = cue.palettes.begin(); it != cue.palettes.end();)
            {
                auto value = newValues.constFind(it.key());
                if (it.value() != paletteId || value == newValues.cend())
                {
                    ++it;
                    continue;
                }
                // the palette doesn't have this channel any more: a plain value
                if (value.value() < 0)
                {
                    it = cue.palettes.erase(it);
                    modified = true;
                    continue;
                }
                const uchar v = uchar(qBound(0, value.value(), 255));
                auto current = cue.values.find(it.key());
                if (current != cue.values.end() && current.value() != v)
                {
                    current.value() = v;
                    modified = true;
                }
                ++it;
            }
        }
        if (modified)
            m_dataDirty = true;
    }
    if (modified)
        emit changed(id());
}

void Cuelist::forgetPalette(int paletteId)
{
    bool modified = false;
    {
        QMutexLocker locker(&m_mutex);
        for (Cue &cue : m_cues)
        {
            for (auto it = cue.palettes.begin(); it != cue.palettes.end();)
            {
                if (it.value() == paletteId)
                {
                    it = cue.palettes.erase(it);
                    modified = true;
                }
                else
                {
                    ++it;
                }
            }
        }
        if (modified)
            m_dataDirty = true;
    }
    if (modified)
        emit changed(id());
}

/*********************************************************************
 * Options
 *********************************************************************/

bool Cuelist::tracking() const
{
    QMutexLocker locker(&m_mutex);
    return m_tracking;
}

void Cuelist::setTracking(bool on)
{
    {
        QMutexLocker locker(&m_mutex);
        if (on == m_tracking)
            return;
        m_tracking = on;
        // what each cue shows changes
        m_dataDirty = true;
    }
    emit changed(id());
}

Cuelist::AtEnd Cuelist::atEnd() const
{
    QMutexLocker locker(&m_mutex);
    return m_atEnd;
}

void Cuelist::setAtEnd(AtEnd mode)
{
    {
        QMutexLocker locker(&m_mutex);
        if (mode == m_atEnd)
            return;
        m_atEnd = mode;
    }
    emit changed(id());
}

bool Cuelist::resetOnRelease() const
{
    QMutexLocker locker(&m_mutex);
    return m_resetOnRelease;
}

void Cuelist::setResetOnRelease(bool on)
{
    {
        QMutexLocker locker(&m_mutex);
        if (on == m_resetOnRelease)
            return;
        m_resetOnRelease = on;
    }
    emit changed(id());
}

int Cuelist::backTime() const
{
    QMutexLocker locker(&m_mutex);
    return m_backTime;
}

void Cuelist::setBackTime(int ms)
{
    ms = qMax(-1, ms);
    {
        QMutexLocker locker(&m_mutex);
        if (ms == m_backTime)
            return;
        m_backTime = ms;
    }
    emit changed(id());
}

bool Cuelist::releaseOnNextGo() const
{
    QMutexLocker locker(&m_mutex);
    return m_releaseOnNextGo;
}

void Cuelist::setReleaseOnNextGo(bool on)
{
    {
        QMutexLocker locker(&m_mutex);
        if (on == m_releaseOnNextGo)
            return;
        m_releaseOnNextGo = on;
    }
    emit changed(id());
}

/*********************************************************************
 * Transport
 *********************************************************************/

void Cuelist::go()
{
    // GO on a paused list resumes its fades, it doesn't go on
    if (isRunning() && isPaused())
    {
        setPause(false);
        return;
    }

    // a stopped list drops it: nothing waits for the next run. Started and
    // not run yet, it is kept: preRun plays the start before it
    QMutexLocker locker(&m_mutex);
    if (stopped())
        return;
    m_actions.append(Action());
}

void Cuelist::back()
{
    // a paused list goes back too: write() waits while paused
    if (isRunning() && isPaused())
        setPause(false);

    Action action;
    action.type = ActionBack;
    QMutexLocker locker(&m_mutex);
    if (stopped())
        return;
    m_actions.append(action);
}

void Cuelist::goToCue(int index, bool snap)
{
    QMutexLocker locker(&m_mutex);
    if (stopped() || index < 0 || index >= m_cues.count())
        return;
    if (isRunning() && isPaused())
        setPause(false);
    Action action;
    action.type = ActionGoTo;
    action.uid = m_cues.at(index).uid;
    action.snap = snap;
    m_actions.append(action);
}

void Cuelist::setNextCue(int index)
{
    QMutexLocker locker(&m_mutex);
    m_preselectUid = index >= 0 && index < m_cues.count() ? m_cues.at(index).uid : 0;
    // published at once: a paused or stopped list has no write() to do it
    updateStatusLocked();
}

void Cuelist::setStartCue(int index)
{
    QMutexLocker locker(&m_mutex);
    m_startUid = index >= 0 && index < m_cues.count() ? m_cues.at(index).uid : 0;
    // a stopped list shows the new start as its next cue
    updateStatusLocked();
}

int Cuelist::nextIndex() const
{
    QMutexLocker locker(&m_mutex);
    // stopped: where the next start begins. Right after stop() the run is
    // still on its cue until postRun (the next frame): the one to resume from
    if (stopped())
        return startIndexLocked(m_current >= 0 ? m_currentUid : m_lastUid);
    return naturalNextLocked();
}

Cuelist::Status Cuelist::status() const
{
    QMutexLocker locker(&m_statusMutex);
    return m_status;
}

QSet<Cuelist::Key> Cuelist::takeChangedKeys()
{
    QSet<Key> keys;
    QMutexLocker locker(&m_mutex);
    keys.swap(m_changed);
    return keys;
}

/*********************************************************************
 * Fixtures
 *********************************************************************/

void Cuelist::slotFixtureRemoved(quint32 fxi_id)
{
    {
        QMutexLocker locker(&m_mutex);
        if (removeFixturesLocked([fxi_id](quint32 fixture) { return fixture == fxi_id; }) == false)
            return;
        m_dataDirty = true;
    }
    emit changed(id());
}

/*********************************************************************
 * Save & Load
 *********************************************************************/

QString Cuelist::triggerToString(Trigger trigger)
{
    switch (trigger)
    {
        case TriggerFollow: return QStringLiteral("Follow");
        case TriggerWait: return QStringLiteral("Wait");
        default: return QStringLiteral("Go");
    }
}

Cuelist::Trigger Cuelist::stringToTrigger(const QString &str)
{
    if (str.compare(QLatin1String("Follow"), Qt::CaseInsensitive) == 0)
        return TriggerFollow;
    if (str.compare(QLatin1String("Wait"), Qt::CaseInsensitive) == 0)
        return TriggerWait;
    return TriggerGo;
}

QString Cuelist::atEndToString(AtEnd mode)
{
    switch (mode)
    {
        case AtEndRelease: return QStringLiteral("Release");
        case AtEndStay: return QStringLiteral("Stay");
        default: return QStringLiteral("Loop");
    }
}

Cuelist::AtEnd Cuelist::stringToAtEnd(const QString &str)
{
    if (str.compare(QLatin1String("Release"), Qt::CaseInsensitive) == 0)
        return AtEndRelease;
    if (str.compare(QLatin1String("Stay"), Qt::CaseInsensitive) == 0)
        return AtEndStay;
    return AtEndLoop;
}

void Cuelist::saveCueXML(QXmlStreamWriter *doc, const Cue &cue)
{
    doc->writeStartElement(KXMLDTCCue);
    doc->writeAttribute("Uid", QString::number(cue.uid));
    // thousandths: the locale never matters
    doc->writeAttribute("Number", QString::number(cue.number));
    if (cue.name.isEmpty() == false)
        doc->writeAttribute("Name", cue.name);
    if (cue.trigger != TriggerGo)
        doc->writeAttribute("Trigger", triggerToString(cue.trigger));
    if (cue.triggerTime != 0)
        doc->writeAttribute("TriggerTime", QString::number(cue.triggerTime));
    doc->writeAttribute("Fade", QString::number(cue.fade));
    if (cue.downFade >= 0)
        doc->writeAttribute("DownFade", QString::number(cue.downFade));
    if (cue.delay != 0)
        doc->writeAttribute("Delay", QString::number(cue.delay));
    if (cue.skip)
        doc->writeAttribute("Skip", "1");
    if (cue.block)
        doc->writeAttribute("Block", "1");

    if (cue.note.isEmpty() == false)
        doc->writeTextElement(KXMLDTCCueNote, cue.note);

    // sorted, so the file doesn't change for nothing
    QList<Key> keys = cue.values.keys();
    std::sort(keys.begin(), keys.end());
    for (const Key &key : std::as_const(keys))
    {
        doc->writeStartElement(KXMLDTCCueValue);
        doc->writeAttribute("F", QString::number(key.first));
        doc->writeAttribute("C", QString::number(key.second));
        auto palette = cue.palettes.constFind(key);
        if (palette != cue.palettes.cend())
            doc->writeAttribute("P", QString::number(palette.value()));
        doc->writeCharacters(QString::number(cue.values.value(key)));
        doc->writeEndElement();
    }

    QList<Key> released = cue.releases.values();
    std::sort(released.begin(), released.end());
    for (const Key &key : std::as_const(released))
    {
        doc->writeEmptyElement(KXMLDTCCueRelease);
        doc->writeAttribute("F", QString::number(key.first));
        doc->writeAttribute("C", QString::number(key.second));
    }

    for (const FXScene::Layer &layer : cue.fx)
        FXScene::saveLayerXML(doc, layer);

    QList<FxKey> fxReleased = cue.fxReleases.values();
    std::sort(fxReleased.begin(), fxReleased.end());
    for (const FxKey &key : std::as_const(fxReleased))
    {
        doc->writeEmptyElement(KXMLDTCCueFxRelease);
        doc->writeAttribute("Param", key.first);
        doc->writeAttribute("F", QString::number(key.second));
    }

    doc->writeEndElement();
}

bool Cuelist::loadCueXML(QXmlStreamReader &root, Cue &cue)
{
    const QXmlStreamAttributes attrs = root.attributes();
    bool numberOk = false;
    cue.uid = attrs.value("Uid").toUInt();
    cue.number = attrs.value("Number").toInt(&numberOk);
    cue.name = attrs.value("Name").toString();
    cue.trigger = stringToTrigger(attrs.value("Trigger").toString());
    // (a time that isn't a number, in a damaged file, keeps the default)
    cue.triggerTime = attrs.value("TriggerTime").toUInt();
    bool fadeOk = false, downOk = false;
    const uint fade = attrs.value("Fade").toUInt(&fadeOk);
    if (fadeOk)
        cue.fade = fade;
    const int downFade = attrs.value("DownFade").toInt(&downOk);
    cue.downFade = downOk ? qMax(-1, downFade) : -1;
    cue.delay = attrs.value("Delay").toUInt();
    cue.skip = attrs.value("Skip") == QLatin1String("1");
    cue.block = attrs.value("Block") == QLatin1String("1");

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLDTCCueNote)
        {
            cue.note = root.readElementText(QXmlStreamReader::SkipChildElements);
        }
        else if (root.name() == KXMLDTCCueValue)
        {
            const QXmlStreamAttributes va = root.attributes();
            bool fixtureOk = false, channelOk = false, paletteOk = false, valueOk = false;
            const Key key(va.value("F").toUInt(&fixtureOk), va.value("C").toUInt(&channelOk));
            const int palette = va.value("P").toInt(&paletteOk);
            const int value = root.readElementText(QXmlStreamReader::SkipChildElements).trimmed().toInt(&valueOk);
            if (fixtureOk && channelOk && valueOk)
            {
                cue.values.insert(key, uchar(qBound(0, value, 255)));
                if (paletteOk)
                    cue.palettes.insert(key, palette);
            }
        }
        else if (root.name() == KXMLDTCCueRelease)
        {
            const QXmlStreamAttributes ra = root.attributes();
            bool fixtureOk = false, channelOk = false;
            const Key key(ra.value("F").toUInt(&fixtureOk), ra.value("C").toUInt(&channelOk));
            if (fixtureOk && channelOk)
                cue.releases.insert(key);
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLDTCFXLayer)
        {
            FXScene::Layer layer = FXScene::loadLayerXML(root);
            if (layer.param.isEmpty() == false)
                cue.fx.append(layer);
        }
        else if (root.name() == KXMLDTCCueFxRelease)
        {
            const QXmlStreamAttributes fa = root.attributes();
            bool fixtureOk = false;
            const QString param = fa.value("Param").toString().toLower();
            const quint32 fixture = fa.value("F").toUInt(&fixtureOk);
            if (fixtureOk && param.isEmpty() == false)
                cue.fxReleases.insert(FxKey(param, fixture));
            root.skipCurrentElement();
        }
        else
        {
            // a newer version: what we don't know is skipped
            root.skipCurrentElement();
        }
    }

    return numberOk;
}

bool Cuelist::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCFunction);
    saveXMLCommon(doc);
    // the fade of new cues and the release time
    saveXMLSpeed(doc);

    QMutexLocker locker(&m_mutex);
    doc->writeStartElement(KXMLDTCCuelist);
    doc->writeAttribute("Tracking", m_tracking ? "1" : "0");
    doc->writeAttribute("AtEnd", atEndToString(m_atEnd));
    doc->writeAttribute("ResetOnRelease", m_resetOnRelease ? "1" : "0");
    doc->writeAttribute("BackTime", QString::number(m_backTime));
    doc->writeAttribute("ReleaseOnNextGo", m_releaseOnNextGo ? "1" : "0");
    doc->writeAttribute("NextUid", QString::number(m_nextUid));
    doc->writeEndElement();

    for (const Cue &cue : m_cues)
        saveCueXML(doc, cue);

    doc->writeEndElement();
    return true;
}

bool Cuelist::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << "Function node not found!";
        return false;
    }

    if (root.attributes().value(KXMLQLCFunctionType).toString() != typeToString(Function::CuelistType))
    {
        qWarning() << "Function is not a cuelist!";
        return false;
    }

    QList<Cue> cues;
    bool tracking = true;
    AtEnd atEnd = AtEndLoop;
    bool resetOnRelease = true;
    int backTime = -1;
    bool releaseOnNextGo = false;
    quint32 nextUid = 1;

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(root);
        }
        else if (root.name() == KXMLDTCCuelist)
        {
            const QXmlStreamAttributes attrs = root.attributes();
            tracking = attrs.value("Tracking") != QLatin1String("0");
            atEnd = stringToAtEnd(attrs.value("AtEnd").toString());
            resetOnRelease = attrs.value("ResetOnRelease") != QLatin1String("0");
            bool backOk = false;
            const int back = attrs.value("BackTime").toInt(&backOk);
            if (backOk)
                backTime = qMax(-1, back);
            releaseOnNextGo = attrs.value("ReleaseOnNextGo") == QLatin1String("1");
            nextUid = qMax(1u, attrs.value("NextUid").toUInt());
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLDTCCue)
        {
            Cue cue;
            if (loadCueXML(root, cue))
                cues.append(cue);
            else
                qWarning() << "Cuelist" << name() << ": a cue without a number, dropped";
        }
        else
        {
            qWarning() << "Unknown cuelist tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    QMutexLocker locker(&m_mutex);
    m_tracking = tracking;
    m_atEnd = atEnd;
    m_resetOnRelease = resetOnRelease;
    m_backTime = backTime;
    m_releaseOnNextGo = releaseOnNextGo;
    // sorted, one cue per number, unique uids, and uids are never reused
    m_nextUid = nextUid;
    normalizeLocked(cues);
    m_cues = cues;
    m_dataDirty = true;
    followEditsLocked();

    return true;
}

void Cuelist::postLoad()
{
    Doc *show = doc();
    if (show == NULL)
        return;

    // the channels of fixtures that are not in the show
    QMutexLocker locker(&m_mutex);
    if (removeFixturesLocked([show](quint32 fixture) { return show->fixture(fixture) == NULL; }))
        m_dataDirty = true;

    // and, like the scenes, the channels their fixtures don't have (a
    // damaged or edited file)
    auto missing = [show](const Key &key) {
        Fixture *fxi = show->fixture(key.first);
        return fxi == NULL || key.second >= fxi->channels();
    };
    for (Cue &cue : m_cues)
    {
        for (auto it = cue.values.begin(); it != cue.values.end();)
        {
            if (missing(it.key()))
            {
                cue.palettes.remove(it.key());
                it = cue.values.erase(it);
                m_dataDirty = true;
            }
            else
            {
                ++it;
            }
        }
        for (auto it = cue.releases.begin(); it != cue.releases.end();)
        {
            if (missing(*it))
            {
                it = cue.releases.erase(it);
                m_dataDirty = true;
            }
            else
            {
                ++it;
            }
        }
    }
}

/*********************************************************************
 * Running
 *********************************************************************/

void Cuelist::preRun(MasterTimer *timer)
{
    {
        QMutexLocker locker(&m_mutex);
        m_playing.clear();
        // (the player is stopped: our last postRun stopped it)
        m_fxPlaying.clear();
        m_fxBases.clear();
        m_fxChannels.clear();
        // (m_actions is kept: the GOs and jumps that came after start(), a
        // second press in the same tick. postRun dropped the older ones)
        m_preselectUid = 0;
        m_changed.clear();
        m_notify.clear();
        m_current = -1;
        m_currentUid = 0;
        m_firstOfRun = true;
        m_followArmed = false;
        m_releaseRequested = false;
        m_dataDirty = false;
        m_fadersPaused = false;
        m_appliedIntensity = -1;
        m_cueElapsed = 0;
        m_cueLength = 0;
        m_fadeLength = 0;
        m_seekRemainder = 0;

        // the cue set with setStartCue(), else a GO. DMX Total Control: a start
        // cue deleted since is a GO, which starts where the status said
        // (startIndexLocked skips it too), not a jump to nothing
        Action first;
        if (m_startUid != 0 && indexOfUidLocked(m_startUid) >= 0)
        {
            first.type = ActionGoTo;
            first.uid = m_startUid;
        }

        // a super scene that starts the block after its beginning (start()
        // set the elapsed time, in real ms; the block may stretch the list)
        const quint64 offset = quint64(qMax(qint64(0), qRound64(qreal(elapsed()) * speedFactor())));
        if (offset > 0 && m_cues.isEmpty() == false)
        {
            quint64 remainder = 0;
            bool moved = false;
            const int index = seekLocked(offset, &remainder, &moved);
            // past a trigger: the cue the chain is on at that time, snapped.
            // Still on the first cue (the show runs in ticks, so most blocks
            // start a few ms in): the run starts as usual, with the block
            // fade-in and the cue fade, and only its timers count from there
            if (moved)
            {
                first.type = ActionGoTo;
                first.uid = m_cues.at(index).uid;
                first.snap = true;
            }
            m_seekRemainder = uint(qMin(remainder, quint64(UINT_MAX)));
        }
        // before them: they go on from the cue the run starts on
        m_actions.prepend(first);
        m_startUid = 0;
    }
    {
        QMutexLocker locker(&m_statusMutex);
        m_status = Status();
    }

    Function::preRun(timer);
}

int Cuelist::seekLocked(quint64 offset, quint64 *remainder, bool *moved) const
{
    const int count = m_cues.count();
    *moved = false;

    // the time from cue $from to cue $to; false when $to waits for a GO
    auto gap = [this](int from, int to, quint64 *ms)
    {
        const Cue &next = m_cues.at(to);
        *ms = next.triggerTime;
        if (next.trigger == TriggerFollow)
            *ms += m_cues.at(from).length();
        return next.trigger != TriggerGo;
    };

    // a loop with no GO in it plays the same cue in every cycle
    if (m_atEnd == AtEndLoop)
    {
        quint64 cycle = 0;
        bool endless = true;
        for (int i = 0; i < count && endless; i++)
        {
            quint64 ms = 0;
            endless = gap(i, (i + 1) % count, &ms);
            cycle += ms;
        }
        if (endless && cycle > 0)
        {
            // whole cycles already played
            if (offset >= cycle)
                *moved = true;
            offset %= cycle;
        }
    }

    int current = 0;
    quint64 start = 0;
    // (a guard against loops of cues without times)
    for (int guard = 0; guard < 10000; guard++)
    {
        int next = current + 1;
        if (next >= count)
        {
            if (m_atEnd != AtEndLoop)
                break;
            next = 0;
        }
        quint64 ms = 0;
        if (gap(current, next, &ms) == false || start + ms > offset)
            break;
        current = next;
        start += ms;
        *moved = true;
    }

    *remainder = offset - start;
    return current;
}

QSharedPointer<GenericFader> Cuelist::getFader(Universe *universe)
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

int Cuelist::startIndexLocked() const
{
    return startIndexLocked(m_lastUid);
}

int Cuelist::startIndexLocked(quint32 lastUid) const
{
    const int count = m_cues.count();
    if (count == 0)
        return -1;

    const int start = indexOfUidLocked(m_startUid);
    if (start >= 0)
        return start;

    // resume after the cue the last release left
    if (m_resetOnRelease == false)
    {
        const int last = indexOfUidLocked(lastUid);
        if (last >= 0)
        {
            if (last + 1 < count)
                return last + 1;
            return m_atEnd == AtEndStay ? last : 0;
        }
    }
    return 0;
}

int Cuelist::naturalNextLocked() const
{
    const int count = m_cues.count();
    if (count == 0)
        return -1;

    const int preselected = indexOfUidLocked(m_preselectUid);
    if (preselected >= 0)
        return preselected;
    if (m_current < 0)
        return startIndexLocked();
    if (m_current + 1 < count)
        return m_current + 1;
    return m_atEnd == AtEndLoop ? 0 : -1;
}

const Cuelist::Cue *Cuelist::autoTriggerLocked(int next) const
{
    if (m_current < 0 || m_cues.isEmpty())
        return nullptr;
    if (next >= 0 && next < m_cues.count())
        return m_cues.at(next).trigger != TriggerGo ? &m_cues.at(next) : nullptr;

    // DMX Total Control: past the end, as Loop wraps with it, Release goes
    // with the trigger of the first cue: the list releases itself (a
    // Sequence played once, converted, ends when its last step does)
    if (m_atEnd == AtEndRelease && m_current + 1 >= m_cues.count() && m_cues.first().trigger != TriggerGo)
        return &m_cues.first();
    return nullptr;
}

void Cuelist::followEditsLocked()
{
    // DMX Total Control: write() re-applies the current cue by its uid, but
    // not while the list is paused: until then the index and the status
    // would point at the cue that took its place
    if (m_current >= 0)
    {
        int index = indexOfUidLocked(m_currentUid);
        // deleted: the one before it, the cue reapplyLocked would play
        if (index < 0 && m_cues.isEmpty() == false)
            index = qBound(0, m_current - 1, int(m_cues.count()) - 1);
        m_current = index;
        m_currentUid = index >= 0 ? m_cues.at(index).uid : 0;
    }
    updateStatusLocked();
}

uint Cuelist::scaledTime(uint ms) const
{
    const qreal factor = speedFactor();
    if (ms == 0 || factor <= 0)
        return ms;
    return uint(qMin(qRound64(qreal(ms) / factor), qint64(UINT_MAX)));
}

quint32 Cuelist::pairedChannel(const Fixture *fixture, quint32 channel)
{
    const QLCFixtureMode *mode = fixture->fixtureMode();
    if (mode == NULL)
        return QLCChannel::invalid();

    // a fine channel: its coarse one
    const quint32 primary = mode->primaryChannel(channel);
    if (primary != QLCChannel::invalid())
        return primary;

    // a coarse channel: the fine channel that names it
    for (quint32 c = 0; c < fixture->channels(); c++)
    {
        if (c != channel && mode->primaryChannel(c) == channel)
            return c;
    }
    return QLCChannel::invalid();
}

bool Cuelist::isDimmer(const Fixture *fixture, quint32 channel)
{
    // RGB and CMY are intensities in QLC+ too: they take the fade, as the moves
    const QLCChannel *ch = fixture->channel(channel);
    return ch != NULL && ch->group() == QLCChannel::Intensity && ch->colour() == QLCChannel::NoColour;
}

void Cuelist::write(MasterTimer *timer, QList<Universe *> universes)
{
    // pause and level reach the faders here only, in the timer thread: the
    // other threads never touch them
    const bool paused = isPaused();
    if (paused != m_fadersPaused)
    {
        for (const QSharedPointer<GenericFader> &fader : std::as_const(m_fadersMap))
        {
            if (!fader.isNull())
                fader->setPaused(paused);
        }
        m_fxPlayer->setPause(paused);
        // DMX Total Control: its faders hold too (a base fading in with the
        // cue), and the ones of FX it is still fading out: we don't write a
        // paused player, so it can't do it itself
        m_fxPlayer->setFadersPaused(paused);
        m_fadersPaused = paused;
    }
    const qreal level = getAttributeValue(Intensity);
    if (level != m_appliedIntensity)
    {
        for (const QSharedPointer<GenericFader> &fader : std::as_const(m_fadersMap))
        {
            if (!fader.isNull())
                fader->adjustIntensity(level);
        }
        // the base level of the FX player: no override ids to keep
        m_fxPlayer->adjustAttribute(level, Function::Intensity);
        m_appliedIntensity = level;
    }

    // paused: the faders hold their values and the cue timers stop. BeatGO
    // divisions are dropped, not saved up for a burst of GOs on resume
    if (paused)
    {
        takeDivisionJumps();
        return;
    }

    advanceFrameTick();

    QList<quint32> started;
    bool release = false;
    {
        QMutexLocker locker(&m_mutex);

        // BeatGO and pulses (a GO per division) and its own rhythm (a GO a beat)
        for (int jumps = takeRhythmSteps(); jumps > 0; jumps--)
            m_actions.append(Action());

        // the cues changed while playing: the new data shows at once, and the
        // next GO still goes on from this cue (e.g. after an Update)
        if (m_dataDirty)
        {
            m_dataDirty = false;
            reapplyLocked(universes);
        }

        // in the order they came: the last one wins on the channels they share
        bool applied = false;
        while (m_actions.isEmpty() == false)
        {
            if (processActionLocked(m_actions.takeFirst(), universes))
                applied = true;
        }
        // only the first cue of a run starts in the middle
        m_seekRemainder = 0;

        // Follow and Wait: at most one automatic step per frame (with a rhythm
        // of its own, its beats move the cues instead)
        if (applied == false && m_followArmed && m_current >= 0 && tapPeriod() == 0)
        {
            const int next = naturalNextLocked();
            const Cue *trigger = autoTriggerLocked(next);
            if (trigger != nullptr)
            {
                // Follow: after the current cue completes; Wait: after its GO
                quint64 due = trigger->triggerTime;
                if (trigger->trigger == TriggerFollow)
                    due += m_cueLength;
                if (m_cueElapsed >= due)
                {
                    if (next >= 0)
                    {
                        const Cue &cue = m_cues.at(next);
                        const uint fade = cue.fade;
                        const uint down = cue.effectiveDown();
                        const uint delay = cue.delay;
                        m_preselectUid = 0;
                        applyCueLocked(universes, next, fade, down, delay, ApplyNormal);
                    }
                    else
                    {
                        // DMX Total Control: past the end of a Release list,
                        // as a GO on the last cue would
                        m_releaseRequested = true;
                    }
                }
            }
        }

        const uint tick = frameTick();
        m_cueElapsed = m_cueElapsed < UINT_MAX - tick ? m_cueElapsed + tick : UINT_MAX;
        updateStatusLocked();

        started.swap(m_notify);
        release = m_releaseRequested;
        m_releaseRequested = false;
    }

    // the FX of the cues play in this frame, after our own values. The
    // MasterTimer doesn't know the player: a new priority (ours, passed on by
    // updateFadersPriority) reaches its faders here. DMX Total Control: and
    // our live speed (a group's dial, a stretched block) its waves, as the
    // cue fades and the Follow/Wait timers (FXScene fades in real ms: the
    // fade it got is already at our speed)
    if (timer != NULL && m_fxPlayer->isRunning())
    {
        m_fxPlayer->setSpeedFactor(speedFactor());
        m_fxPlayer->setMotionSpeed(motionSpeed());
        m_fxPlayer->applyPriorityChange();
        m_fxPlayer->write(timer, universes);
    }

    // the programmer releases what a GO changed (live edit)
    if (timer != NULL)
    {
        for (quint32 uid : std::as_const(started))
            timer->notifyCueStarted(id(), uid);
    }

    incrementElapsed();

    // AtEndRelease: GO on the last cue (or the trigger of the first cue after
    // it) releases the list, as a chaser stops itself
    if (release)
        stop(FunctionParent::master());
}

bool Cuelist::processActionLocked(const Action &action, const QList<Universe *> &universes)
{
    const int count = m_cues.count();
    int target = -1;

    if (action.type == ActionGo)
    {
        m_followArmed = true;
        const int preselected = indexOfUidLocked(m_preselectUid);
        m_preselectUid = 0;
        if (preselected >= 0)
            target = preselected;
        else if (m_current < 0)
            target = startIndexLocked();
        else if (m_current + 1 < count)
            target = m_current + 1;
        else if (m_atEnd == AtEndLoop)
            target = 0;
        else if (m_atEnd == AtEndRelease)
            m_releaseRequested = true;
        // AtEndStay: it stays on the last cue
    }
    else if (action.type == ActionBack)
    {
        // nothing before the first cue
        if (m_current <= 0)
            return false;
        target = m_current - 1;
        // the list waits on that look: the next GO or jump arms it again
        m_followArmed = false;
        if (m_backTime >= 0)
        {
            applyCueLocked(universes, target, uint(m_backTime), uint(m_backTime), 0, ApplyNormal);
            return true;
        }
    }
    else
    {
        target = indexOfUidLocked(action.uid);
        if (target < 0)
            return false;
        m_followArmed = true;
        if (action.snap)
        {
            applyCueLocked(universes, target, 0, 0, 0, ApplyNormal);
            return true;
        }
    }

    if (target < 0 || target >= count)
        return false;

    // the cue's own times (a jump uses the times of the cue it goes to)
    const Cue &cue = m_cues.at(target);
    uint fade = cue.fade;
    uint down = cue.effectiveDown();
    const uint delay = cue.delay;
    // the first cue of a run fades in with the group or the super-scene block
    if (m_firstOfRun && overrideFadeInSpeed() != defaultSpeed())
        fade = down = overrideFadeInSpeed();
    applyCueLocked(universes, target, fade, down, delay, ApplyNormal);
    return true;
}

void Cuelist::reapplyLocked(const QList<Universe *> &universes)
{
    // nothing plays yet (an empty list, or no GO yet)
    if (m_current < 0 && m_playing.isEmpty() && m_fxPlaying.isEmpty())
        return;

    int index = indexOfUidLocked(m_currentUid);
    // the current cue was deleted: the one before it plays
    if (index < 0 && m_current >= 0 && m_cues.isEmpty() == false)
        index = qBound(0, m_current - 1, int(m_cues.count()) - 1);

    m_current = index;
    m_currentUid = index >= 0 ? m_cues.at(index).uid : 0;
    m_cueLength = index >= 0 ? m_cues.at(index).length() : 0;
    // snapped, and the cue timers go on
    applyCueLocked(universes, index, 0, 0, 0, ApplyLiveReapply);
}

void Cuelist::applyCueLocked(const QList<Universe *> &universes, int index,
                             uint fade, uint down, uint delay, ApplyMode mode)
{
    const bool live = mode == ApplyLiveReapply;
    const State state = index >= 0 && index < m_cues.count() ? foldLocked(index, true) : State();

    // the channels the FX layers move belong to the FX player, with their
    // static values as its bases: our faders never hold them (an FX that
    // modulates what's below would find our fader above it)
    const QHash<Key, int> fxChannels = fxChannelsOf(state.fx);
    QHash<Key, uchar> values = state.values;
    QHash<Key, uchar> bases;
    for (auto it = fxChannels.cbegin(); it != fxChannels.cend(); ++it)
    {
        auto value = values.find(it.key());
        if (value == values.end())
            continue;
        // (the other byte of a pair that the FX doesn't name is no base: it
        // goes back to what's below)
        if (it.value() >= 0)
            bases.insert(it.key(), value.value());
        values.erase(value);
    }

    // FadeChannels count real ms, the cue timers virtual ms (the live speed)
    const uint fadeMs = live ? 0 : scaledTime(fade);
    const uint downMs = live ? 0 : scaledTime(down);
    const uint delayMs = live ? 0 : scaledTime(delay);
    // what our faders scale the intensities by
    const qreal level = m_appliedIntensity >= 0 ? m_appliedIntensity : getAttributeValue(Intensity);

    // the coarse/fine channels updated by this apply (they share a FadeChannel)
    QSet<Key> pairsTouched;

    // new and changed targets. The others are not touched: a fade that runs
    // goes on, and nothing is taken back from the other playbacks (no dips)
    for (auto it = values.cbegin(); it != values.cend(); ++it)
    {
        const Key key = it.key();
        const uchar value = it.value();
        auto playing = m_playing.constFind(key);
        if (playing != m_playing.cend() && playing.value() == value)
            continue;

        Fixture *fxi = doc()->fixture(key.first);
        if (fxi == NULL || key.second >= fxi->channels())
            continue;
        // the universe of the channel, as Scene finds it
        const int universeIndex = int((fxi->universeAddress() + key.second) / UNIVERSE_SIZE);
        if (universeIndex >= universes.count())
            continue;
        Universe *universe = universes.at(universeIndex);
        QSharedPointer<GenericFader> fader = getFader(universe);
        if (fader.isNull())
            continue;

        // new to us, or on its way back: the channel holds what the stage shows
        const bool retake = playing == m_playing.cend();
        const bool dimmer = isDimmer(fxi, key.second);
        const quint32 paired = pairedChannel(fxi, key.second);
        const Key pairKey(key.first, paired);
        const bool pairHeld = paired != QLCChannel::invalid() && values.contains(pairKey);
        // the other byte stays ours and keeps its target: the fade it runs
        // must not end sooner because this byte takes a shorter one
        bool pairKept = false;
        if (pairHeld)
        {
            auto pairPlaying = m_playing.constFind(pairKey);
            pairKept = pairsTouched.contains(pairKey) == false && pairPlaying != m_playing.cend() &&
                       pairPlaying.value() == values.value(pairKey);
            pairsTouched.insert(key);
        }

        fader->updateChannel(doc(), universe, key.first, key.second, [&](FadeChannel &fc)
        {
            const int i = fc.channelIndex(key.second);
            // what is left of the fade the channel runs (its delay included)
            const quint64 runs = quint64(fc.fadeTime()) + fc.delay();
            const uint left = fc.isReady() || fc.elapsed() >= runs ? 0 : uint(qMin(runs - fc.elapsed(), quint64(UINT_MAX)));
            // handed over to the FX player in this frame (AutoRemove alone):
            // it still holds our value, before the level
            const bool handedOver = (fc.flags() & FadeChannel::AutoRemove) &&
                                    (fc.flags() & FadeChannel::SetTarget) == 0;
            if (fc.flags() & FadeChannel::SetTarget)
            {
                // it was going back to what's below: ours again
                fc.removeFlag(FadeChannel::SetTarget);
            }
            fc.removeFlag(FadeChannel::AutoRemove);
            int from = fc.current(i);
            // the stage shows it at our level: an intensity starts from
            // there, without a dip or a jump
            if (retake && handedOver == false && fc.canFade() && (fc.flags() & FadeChannel::Intensity))
                from = level > 0.004 ? qMin(255, qRound(qreal(from) / level)) : value;
            fc.setStart(uchar(from), i);
            fc.setCurrent(uchar(from), i);
            fc.setTarget(value, i);
            // the other byte of a coarse/fine pair: the fade starts again, so
            // it goes on from where it is (no jump back to where its own fade
            // began), to the target it has when it is ours too, else it stays
            for (int j = 0; j < fc.channelCount(); j++)
            {
                if (j == i)
                    continue;
                fc.setStart(fc.current(j), j);
                if (pairHeld == false)
                    fc.setTarget(fc.current(j), j);
            }
            fc.setElapsed(0);
            fc.setReady(false);
            // the down fade is for dimmers going down; colours and moves take the fade
            uint time = dimmer && fc.target() < fc.start() ? downMs : fadeMs;
            // one fade for both bytes: the one still on its way doesn't jump ahead
            if (pairKept)
                time = qMax(time, left);
            fc.setFadeTime(fc.canFade() == false ? 0 : time);
            fc.setDelay(delayMs);
        });

        m_playing.insert(key, value);
        if (live == false)
            m_changed.insert(key);
    }

    // what we hold and the cue doesn't (released, not tracked, Tracking off,
    // moved into an FX): it glides to what's below (or to its rest value),
    // then leaves the fader
    QSet<quint32> removedFixtures;
    for (auto it = m_playing.begin(); it != m_playing.end();)
    {
        if (values.contains(it.key()))
        {
            ++it;
            continue;
        }
        const Key key = it.key();
        it = m_playing.erase(it);
        if (live == false)
            m_changed.insert(key);

        Fixture *fxi = doc()->fixture(key.first);
        if (fxi == NULL)
        {
            removedFixtures.insert(key.first);
            continue;
        }
        if (key.second >= fxi->channels())
            continue;
        // it shares a FadeChannel with a channel that stays
        const quint32 paired = pairedChannel(fxi, key.second);
        if (paired != QLCChannel::invalid() && values.contains(Key(key.first, paired)))
            continue;
        const int universeIndex = int((fxi->universeAddress() + key.second) / UNIVERSE_SIZE);
        if (universeIndex >= universes.count())
            continue;
        Universe *universe = universes.at(universeIndex);
        QSharedPointer<GenericFader> fader = m_fadersMap.value(universe->id());
        if (fader.isNull())
            continue;
        const bool dimmer = isDimmer(fxi, key.second);
        // its value became the base of an FX (both bytes of a pair go
        // together): the player takes it over from what the stage shows (an
        // intensity from stage / level, FXScene::write), so it holds that
        // one more frame and leaves. Going to what's below first would flash
        // the HTP values of the lower playbacks
        const bool handover = bases.contains(key) ||
                              (paired != QLCChannel::invalid() && bases.contains(Key(key.first, paired)));

        fader->updateChannel(doc(), universe, key.first, key.second, [&](FadeChannel &fc)
        {
            // already on its way out
            if (fc.flags() & (FadeChannel::SetTarget | FadeChannel::AutoRemove))
                return;
            if (handover)
            {
                fc.setStart(fc.current());
                fc.setTarget(fc.current());
                fc.addFlag(FadeChannel::AutoRemove);
                fc.setElapsed(0);
                fc.setReady(false);
                fc.setFadeTime(0);
                fc.setDelay(0);
                return;
            }
            // from what it sends now: the fader scales intensities by the level
            const bool scaled = fc.canFade() && (fc.flags() & FadeChannel::Intensity) &&
                                (fc.flags() & FadeChannel::CrossFade) == 0;
            const quint32 out = scaled ? fc.current(level) : fc.current();
            fc.setStart(out);
            fc.setCurrent(out);
            // GenericFader::write aims it at what's below every frame, and
            // drops it once it is there
            fc.addFlag(FadeChannel::SetTarget);
            fc.addFlag(FadeChannel::AutoRemove);
            fc.setTarget(quint32(0));
            fc.setElapsed(0);
            fc.setReady(false);
            fc.setFadeTime(fc.canFade() == false ? 0 : (dimmer ? downMs : fadeMs));
            fc.setDelay(delayMs);
        });
    }

    // the channels of deleted fixtures leave the faders at once
    if (removedFixtures.isEmpty() == false)
    {
        for (const QSharedPointer<GenericFader> &fader : std::as_const(m_fadersMap))
        {
            if (fader.isNull())
                continue;
            const QHash<quint32, FadeChannel> channels = fader->channels();
            for (const FadeChannel &fc : channels)
            {
                if (removedFixtures.contains(fc.fixture()))
                    fader->remove(&fc);
            }
        }
    }

    // the tracked FX layers, each on the fixtures it still holds
    playFxLocked(universes, state.fx, bases, fxChannels, fadeMs, level, mode);

    if (live == false && index >= 0 && index < m_cues.count())
    {
        const Cue &cue = m_cues.at(index);
        m_current = index;
        m_currentUid = cue.uid;
        // a super-scene seek starts the first cue in the middle
        m_cueElapsed = m_seekRemainder;
        m_seekRemainder = 0;
        m_cueLength = cue.length();
        // DMX Total Control: the fades as they run, which a BACK time or the
        // fade-in of a group or a block change: the status shows them (and
        // Pause/Back pauses a fade that runs), Follow keeps the cue's length
        const quint64 fadeEnd = quint64(m_cueElapsed) + delay + qMax(fade, down);
        m_fadeLength = uint(qMin(fadeEnd, quint64(UINT_MAX)));
        m_notify.append(cue.uid);
        m_firstOfRun = false;
    }
}

QHash<Cuelist::Key, int> Cuelist::fxChannelsOf(const QList<FXScene::Layer> &layers) const
{
    QHash<Key, int> channels;
    for (int l = 0; l < layers.count(); l++)
    {
        const FXScene::Layer &layer = layers.at(l);
        const QStringList axes = fxAxes(layer.param);
        for (quint32 fid : layer.fixtures)
        {
            const Fixture *fxi = doc()->fixture(fid);
            if (fxi == NULL)
                continue;
            for (const QString &axis : axes)
            {
                // the channels FXScene finds for the axis; the first layer
                // that finds a channel moves it, as in FXScene
                quint32 fine = QLCChannel::invalid();
                const quint32 channel = FXScene::channelFor(fxi, axis, &fine);
                if (channel == QLCChannel::invalid() || channels.value(Key(fid, channel), -1) >= 0)
                    continue;
                channels.insert(Key(fid, channel), l);
                if (fine != QLCChannel::invalid())
                    channels.insert(Key(fid, fine), l);
                // GenericFader keeps a coarse channel and the fine one after
                // it in one FadeChannel: that byte can't stay in our faders
                const quint32 paired = pairedChannel(fxi, channel);
                if (paired != QLCChannel::invalid() && channels.contains(Key(fid, paired)) == false)
                    channels.insert(Key(fid, paired), -1);
            }
        }
    }
    return channels;
}

void Cuelist::playFxLocked(const QList<Universe *> &universes, const QList<FXScene::Layer> &layers,
                           const QHash<Key, uchar> &bases, const QHash<Key, int> &channels,
                           uint fadeMs, qreal level, ApplyMode mode)
{
    // a GO changes the FX channels whose layer or base changes, and the ones
    // an FX starts or stops on: the programmer releases them too (live edit)
    if (mode == ApplyNormal)
    {
        auto differs = [&](const Key &key)
        {
            // its layer: an index, -1 (the other byte of a pair), -2 (not an FX channel)
            const int before = m_fxChannels.value(key, -2);
            const int now = channels.value(key, -2);
            if (before < 0 || now < 0)
                return before != now;
            if (m_fxPlaying.at(before) != layers.at(now) || bases.contains(key) != m_fxBases.contains(key))
                return true;
            return bases.contains(key) && bases.value(key) != m_fxBases.value(key);
        };
        for (auto it = channels.cbegin(); it != channels.cend(); ++it)
        {
            if (differs(it.key()))
                m_changed.insert(it.key());
        }
        for (auto it = m_fxChannels.cbegin(); it != m_fxChannels.cend(); ++it)
        {
            if (channels.contains(it.key()) == false)
                m_changed.insert(it.key());
        }
    }

    if (layers.isEmpty())
    {
        // the FX fade out to what's below them: our values, or the playbacks
        // under the list
        if (m_fxPlayer->isRunning())
        {
            m_fxPlayer->setFadeOutSpeed(fadeMs);
            m_fxPlayer->postRun(doc()->masterTimer(), universes);
        }
    }
    else if (m_fxPlayer->isRunning() == false || layers != m_fxPlaying || bases != m_fxBases)
    {
        QList<quint32> fixtures;
        QSet<quint32> seen;
        for (const FXScene::Layer &layer : layers)
        {
            for (quint32 fid : layer.fixtures)
            {
                if (seen.contains(fid) == false)
                {
                    seen.insert(fid);
                    fixtures.append(fid);
                }
            }
        }
        // running, it takes new layers and bases at once (FXScene snaps live
        // edits), and each layer goes on from where its wave is
        m_fxPlayer->setFixtures(fixtures);
        m_fxPlayer->setValues(bases);
        m_fxPlayer->setLayers(layers);
        if (m_fxPlayer->isRunning() == false)
        {
            // it fades in with the cue, at our priority and level. write()
            // runs it from the next frame on (this one included)
            m_fxPlayer->setName(name() + QStringLiteral(" (FX)"));
            m_fxPlayer->setFadeInSpeed(fadeMs);
            m_fxPlayer->setFadeOutSpeed(fadeMs);
            m_fxPlayer->setParentPriority(playbackPriority());
            m_fxPlayer->adjustAttribute(level, Function::Intensity);
            m_fxPlayer->preRun(doc()->masterTimer());
        }
    }

    m_fxPlaying = layers;
    m_fxBases = bases;
    m_fxChannels = channels;
}

void Cuelist::updateStatusLocked()
{
    Status status;
    status.current = m_current;
    status.currentUid = m_currentUid;
    status.next = naturalNextLocked();
    if (m_current >= 0)
    {
        // DMX Total Control: of the fades as they run (m_fadeLength), not the cue's own times
        status.progress = m_fadeLength > 0 ? qMin(1.0, qreal(m_cueElapsed) / qreal(m_fadeLength)) : 1.0;
        status.fading = m_cueElapsed < m_fadeLength;
        status.waiting = m_followArmed && autoTriggerLocked(status.next) != nullptr;
    }

    QMutexLocker locker(&m_statusMutex);
    m_status = status;
}

void Cuelist::postRun(MasterTimer *timer, QList<Universe *> universes)
{
    // the release time, or the fade out of the group or the super-scene block
    const uint release = overrideFadeOutSpeed() != defaultSpeed() ? overrideFadeOutSpeed() : fadeOutSpeed();

    // a paused list that is released would freeze: its faders go on first.
    // DMX Total Control: the player's too, and the FX it is still fading out
    // (when it doesn't run, its postRun below doesn't do it)
    for (const QSharedPointer<GenericFader> &fader : std::as_const(m_fadersMap))
    {
        if (!fader.isNull())
            fader->setPaused(false);
    }
    m_fxPlayer->setFadersPaused(false);
    m_fadersPaused = false;

    if (release == 0)
    {
        dismissAllFaders();
    }
    else
    {
        // everything crossfades to what's below; the universe drops the faders
        for (const QSharedPointer<GenericFader> &fader : std::as_const(m_fadersMap))
        {
            if (!fader.isNull())
                fader->setFadeOut(true, release);
        }
        m_fadersMap.clear();
    }

    // the FX fade out with the same time, in this frame: when the list has
    // stopped (stopAndWait) the player has too, and nothing runs it any more
    if (m_fxPlayer->isRunning())
    {
        m_fxPlayer->setFadeOutSpeed(release);
        m_fxPlayer->postRun(timer, universes);
    }

    {
        QMutexLocker locker(&m_mutex);
        // to resume from; m_startUid is kept (a stop and a start in the same tick)
        m_lastUid = m_currentUid;
        m_playing.clear();
        m_fxPlaying.clear();
        m_fxBases.clear();
        m_fxChannels.clear();
        m_actions.clear();
        m_notify.clear();
        m_preselectUid = 0;
        m_current = -1;
        m_currentUid = 0;
        m_followArmed = false;
        m_releaseRequested = false;
        m_cueElapsed = 0;
        m_cueLength = 0;
        m_fadeLength = 0;

        Status status;
        status.next = startIndexLocked();
        QMutexLocker statusLocker(&m_statusMutex);
        m_status = status;
    }

    Function::postRun(timer, universes);
}

void Cuelist::updateFadersPriority()
{
    Function::updateFadersPriority();
    m_fxPlayer->setParentPriority(playbackPriority());
}

qreal Cuelist::frameMotion() const
{
    return 1.0;
}
