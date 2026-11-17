/*
  DMX Total Control
  safetyparks.cpp

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
#include <QSet>
#include <functional>
#include <iterator>

#include "safetymanager.h"
#include "channelmodifier.h"
#include "inputoutputmap.h"
#include "qlcfixturemode.h"
#include "patchmanager.h"
#include "outputlayer.h"
#include "programmer.h"
#include "qlcchannel.h"
#include "scenevalue.h"
#include "universe.h"
#include "fixture.h"
#include "doc.h"

/*********************************************************************
 * Parked channels: a value per channel at the output, above every
 * source, the grand master and Stop all (OutputLayer::parks)
 *********************************************************************/

QString SafetyManager::modeOf(const Fixture *fxi)
{
    const QLCFixtureMode *mode = fxi != nullptr ? fxi->fixtureMode() : nullptr;
    return mode != nullptr ? mode->name() : QString();
}

void SafetyManager::setProgrammer(Programmer *programmer)
{
    m_programmer = programmer;
}

bool SafetyManager::protectedNow(quint32 id) const
{
    // (as buildLayers makes its units: the one rule of the patch)
    return m_patch->isProtected(id);
}

QVariantMap SafetyManager::parkedFixtures() const
{
    QVariantMap map;
    for (const ParkEntry &park : m_parks)
    {
        const QString key = QString::number(park.fixture);
        map.insert(key, map.value(key).toInt() + 1);
    }
    return map;
}

int SafetyManager::parkedCount() const
{
    QSet<quint32> fixtures;
    for (const ParkEntry &park : m_parks)
        fixtures.insert(park.fixture);
    return int(fixtures.count());
}

bool SafetyManager::isParked(quint32 id) const
{
    const auto it = m_parks.lowerBound(qMakePair(id, quint32(0)));
    return it != m_parks.constEnd() && it->fixture == id;
}

bool SafetyManager::keepsBlackout(quint32 id) const
{
    for (auto it = m_parks.lowerBound(qMakePair(id, quint32(0))); it != m_parks.constEnd() && it->fixture == id; ++it)
        if (it->inBlackout)
            return true;
    return false;
}

bool SafetyManager::dropParks(quint32 id)
{
    bool had = false;
    for (auto it = m_parks.lowerBound(qMakePair(id, quint32(0))); it != m_parks.end() && it->fixture == id;)
    {
        it = m_parks.erase(it);
        had = true;
    }
    return had;
}

void SafetyManager::parkAllOff(quint32 id, bool inBlackout)
{
    dropParks(id);
    const Fixture *fxi = m_doc->fixture(id);
    if (fxi == nullptr)
        return;
    const QString mode = modeOf(fxi);
    for (quint32 c = 0; c < fxi->channels(); c++)
        m_parks.insert(qMakePair(id, c), ParkEntry { id, c, 0, inBlackout, mode });
}

void SafetyManager::parksEdited()
{
    // the universes have them from their next frame
    push();
    m_doc->setModified();
    emit parksChanged();
}

void SafetyManager::tellParked(const QString &text, int protectedCount)
{
    if (protectedCount > 0)
        emit message(text + QStringLiteral(" · ") + tr("A protected special effect can only be parked off"), false);
    else
        emit message(text, false);
}

/** What goes out now on each channel of $fxi: the last frame sent, its
 *  output stage included, before the swaps; a channel parked now, its park
 *  ($parkedAt: its value, -1 none: the frame may be older than the park).
 *  $off: its intensities (dimmers, colours) 0, and all of it when it has
 *  none (a fog machine). $underneath: what the show sends under the show
 *  stop or a fade to black (their open white, open shutters and ramp are
 *  not the show's: never parked) */
static QVector<uchar> valuesNow(Doc *doc, const Fixture *fxi, bool off, bool underneath,
                                const std::function<int(int)> &parkedAt)
{
    const QList<Universe *> universes = doc->inputOutputMap()->universes();
    // (a fixture has at most a universe of channels)
    const int channels = int(qMin(fxi->channels(), quint32(UNIVERSE_SIZE)));
    auto intensity = [fxi](int c) {
        const QLCChannel *ch = fxi->channel(quint32(c));
        return ch != nullptr && ch->group() == QLCChannel::Intensity;
    };
    bool anyIntensity = false;
    for (int c = 0; c < channels; c++)
        anyIntensity = anyIntensity || intensity(c);

    QHash<int, QByteArray> sent;
    QVector<uchar> values(qsizetype(channels), uchar(0));
    for (int c = 0; c < channels; c++)
    {
        if (off && (anyIntensity == false || intensity(c)))
            continue;
        const int parked = parkedAt(c);
        if (parked >= 0)
        {
            values[c] = uchar(parked);
            continue;
        }
        const quint32 ua = fxi->universeAddress() + quint32(c);
        const int u = int(ua / UNIVERSE_SIZE);
        if (u >= universes.count())
            continue;
        if (underneath)
        {
            values[c] = universes.at(u)->postGMValue(int(ua % UNIVERSE_SIZE));
            continue;
        }
        if (sent.contains(u) == false)
            sent.insert(u, universes.at(u)->finalValues());
        const QByteArray &frame = sent[u];
        const int address = int(ua % UNIVERSE_SIZE);
        values[c] = address < frame.size() ? uchar(frame.at(address)) : 0;
    }
    return values;
}

int SafetyManager::parkCurrent(const QVariantList &ids)
{
    return parkNow(ids, false);
}

int SafetyManager::parkOff(const QVariantList &ids)
{
    return parkNow(ids, true);
}

int SafetyManager::parkNow(const QVariantList &ids, bool off)
{
    // what goes out during the show stop or a fade to black is theirs (open
    // white, open shutters, a ramp): parked as it is now, it would stay so
    // for good, saved with the show. Off: the rest as the show sends it
    const bool overlay = m_showStop || m_rampId != 0;
    if (overlay && off == false)
    {
        emit message(m_showStop ? tr("The show stop is on: take it away to park as it is now")
                                : tr("A fade to black runs: park as it is now once it is over"), true);
        return 0;
    }
    int count = 0;
    int protectedCount = 0;
    QSet<quint32> done;
    for (const QVariant &v : ids)
    {
        const quint32 id = v.toUInt();
        const Fixture *fxi = m_doc->fixture(id);
        if (fxi == nullptr || done.contains(id))
            continue;
        done.insert(id);
        count++;
        // (parked again: it keeps «also in blackout»)
        const bool inBlackout = keepsBlackout(id);
        // a protected special effect: only off, all of it (it never fires)
        if (protectedNow(id))
        {
            parkAllOff(id, inBlackout);
            protectedCount++;
            continue;
        }
        const QString mode = modeOf(fxi);
        const QVector<uchar> values = valuesNow(m_doc, fxi, off, overlay, [this, id, &mode](int c) {
            const auto it = m_parks.constFind(qMakePair(id, quint32(c)));
            return it != m_parks.constEnd() && it->mode == mode ? int(it->value) : -1;
        });
        dropParks(id);
        for (int c = 0; c < values.count(); c++)
            m_parks.insert(qMakePair(id, quint32(c)), ParkEntry { id, quint32(c), values.at(c), inBlackout, mode });
    }
    if (count == 0)
        return 0;
    parksEdited();
    if (off)
        tellParked(tr("%n fixture(s) parked off", "", count), 0);
    else
        tellParked(tr("%n fixture(s) parked as they are now", "", count), protectedCount);
    return count;
}

bool SafetyManager::canParkProgrammer() const
{
    if (m_programmer == nullptr)
        return false;
    const QList<quint32> selection = m_programmer->selectionIds();
    const QSet<quint32> selected(selection.cbegin(), selection.cend());
    for (const SceneValue &scv : m_programmer->sceneValues())
        if (selected.contains(scv.fxi) && m_doc->fixture(scv.fxi) != nullptr)
            return true;
    return false;
}

int SafetyManager::parkProgrammer()
{
    if (m_programmer == nullptr)
        return 0;
    // the values the programmer holds for the fixtures selected
    const QList<quint32> selection = m_programmer->selectionIds();
    const QSet<quint32> selected(selection.cbegin(), selection.cend());
    QMap<quint32, QList<SceneValue>> values;
    for (const SceneValue &scv : m_programmer->sceneValues())
        if (selected.contains(scv.fxi) && m_doc->fixture(scv.fxi) != nullptr)
            values[scv.fxi].append(scv);
    if (values.isEmpty())
    {
        emit message(tr("The programmer has no values on the selected fixtures to park"), true);
        return 0;
    }

    const QList<Universe *> universes = m_doc->inputOutputMap()->universes();
    QSet<QPair<quint32, quint32>> keys;
    int protectedCount = 0;
    for (auto it = values.cbegin(); it != values.cend(); ++it)
    {
        const quint32 id = it.key();
        const Fixture *fxi = m_doc->fixture(id);
        const bool inBlackout = keepsBlackout(id);
        for (const SceneValue &scv : it.value())
            keys.insert(qMakePair(id, scv.channel));
        // (what it holds for a CO2 never went out: it is parked off)
        if (protectedNow(id))
        {
            parkAllOff(id, inBlackout);
            protectedCount++;
            continue;
        }
        const QString mode = modeOf(fxi);
        // (parks of another mode it had: its channels were others)
        for (auto park = m_parks.lowerBound(qMakePair(id, quint32(0))); park != m_parks.end() && park->fixture == id;)
            park = park->mode == mode ? std::next(park) : m_parks.erase(park);
        for (const SceneValue &scv : it.value())
        {
            if (scv.channel >= fxi->channels())
                continue;
            // the byte that went out: through the limits of the channel as
            // they are now (they don't change a park later)
            const quint32 ua = fxi->universeAddress() + scv.channel;
            const int u = int(ua / UNIVERSE_SIZE);
            if (u >= universes.count())
                continue;
            ChannelModifier *modifier = universes.at(u)->channelModifier(ushort(ua % UNIVERSE_SIZE));
            const uchar value = modifier != nullptr ? modifier->getValue(scv.value) : scv.value;
            m_parks.insert(qMakePair(id, scv.channel), ParkEntry { id, scv.channel, value, inBlackout, mode });
        }
    }
    // parked first (the next frame has them), then the programmer lets them go
    parksEdited();
    m_programmer->releaseKeys(keys);
    tellParked(tr("%n fixture(s) parked at the values of the programmer", "", int(values.count())), protectedCount);
    return int(values.count());
}

void SafetyManager::unpark(const QVariantList &ids)
{
    int count = 0;
    for (const QVariant &v : ids)
        count += dropParks(v.toUInt()) ? 1 : 0;
    if (count == 0)
        return;
    parksEdited();
    emit message(tr("%n fixture(s) unparked", "", count), false);
}

void SafetyManager::unparkAll()
{
    const int count = parkedCount();
    if (count == 0)
        return;
    m_parks.clear();
    parksEdited();
    emit message(tr("%n fixture(s) unparked", "", count), false);
}

void SafetyManager::setParkInBlackout(const QVariantList &ids, bool on)
{
    bool changed = false;
    for (const QVariant &v : ids)
    {
        const quint32 id = v.toUInt();
        for (auto it = m_parks.lowerBound(qMakePair(id, quint32(0))); it != m_parks.end() && it->fixture == id; ++it)
        {
            if (it->inBlackout != on)
            {
                it->inBlackout = on;
                changed = true;
            }
        }
    }
    if (changed)
        parksEdited();
}

bool SafetyManager::parkInBlackout(const QVariantList &ids) const
{
    bool any = false;
    for (const QVariant &v : ids)
    {
        const quint32 id = v.toUInt();
        for (auto it = m_parks.lowerBound(qMakePair(id, quint32(0))); it != m_parks.constEnd() && it->fixture == id; ++it)
        {
            if (it->inBlackout == false)
                return false;
            any = true;
        }
    }
    return any;
}

QVariantMap SafetyManager::channelMarks(int universe) const
{
    QVariantMap marks;
    const QList<Universe *> universes = m_doc->inputOutputMap()->universes();
    if (universe < 0 || universe >= universes.count())
        return marks;
    // what the universe has now: the truth of what goes out
    Universe *u = universes.at(universe);
    const QSharedPointer<const OutputLayer> layer = u->outputLayer();
    if (layer.isNull())
        return marks;
    // the show stop puts its own values above the parks; a fade to black
    // takes the parked intensities down (not those also in blackout)
    QHash<int, bool> forced;
    if (layer->showStop)
    {
        for (const OutputLayer::Forced &value : layer->forced)
            forced.insert(value.address, value.hold);
    }
    for (const OutputLayer::Park &park : layer->parks)
    {
        const bool ramp = layer->rampId != 0 && park.inBlackout == false &&
                          (u->channelCapabilities(park.address) & Universe::HTP);
        marks.insert(QString::number(park.address), QVariantMap({ { "kind", "park" }, { "value", int(park.value) },
                                                                  { "inBlackout", park.inBlackout },
                                                                  { "showStop", forced.contains(park.address) },
                                                                  { "ramp", ramp } }));
    }
    for (auto it = forced.cbegin(); it != forced.cend(); ++it)
    {
        if (marks.contains(QString::number(it.key())) == false)
            marks.insert(QString::number(it.key()), QVariantMap({ { "kind", "showStop" }, { "hold", it.value() } }));
    }
    // (a protected channel is the gate's, parked or not)
    const bool armed = layer->armToken != 0;
    for (const OutputLayer::Unit &unit : layer->units)
    {
        const QVariantMap mark({ { "kind", "effect" }, { "parked", unit.parked }, { "armed", armed } });
        for (quint16 address : unit.fire)
            marks.insert(QString::number(address), mark);
        for (quint16 address : unit.aux)
            marks.insert(QString::number(address), mark);
        if (unit.safety >= 0)
            marks.insert(QString::number(unit.safety), mark);
    }
    return marks;
}

QList<SafetyManager::ParkEntry> SafetyManager::parks() const
{
    return m_parks.values();
}

void SafetyManager::slotFixtureRemoved(quint32 id)
{
    // (a show that goes: cleared all at once)
    if (m_doc->isClearingOrLoading())
        return;
    if (dropParks(id))
        emit parksChanged();
}

void SafetyManager::slotFixtureChanged(quint32 id)
{
    if (m_patch->loading() || m_doc->isClearingOrLoading() || isParked(id) == false)
        return;
    const Fixture *fxi = m_doc->fixture(id);
    if (fxi == nullptr)
        return;
    // moved: its parks go with it (by channel). Of another mode: its
    // channels are others
    const QString mode = modeOf(fxi);
    bool stale = false;
    for (auto it = m_parks.lowerBound(qMakePair(id, quint32(0))); it != m_parks.constEnd() && it->fixture == id; ++it)
        stale = stale || it->mode != mode || it->channel >= fxi->channels();
    if (stale == false)
        return;
    // a protected special effect stays parked off (fail closed)
    if (protectedNow(id))
    {
        parkAllOff(id, keepsBlackout(id));
    }
    else
    {
        dropParks(id);
        emit message(tr("The parked channels of %1 were removed: its mode changed").arg(fxi->name()), false);
    }
    // (the layer follows: Doc told the patch)
    emit parksChanged();
}

/*********************************************************************
 * The show file
 *********************************************************************/

void SafetyManager::saveXML(QXmlStreamWriter *doc) const
{
    // (a show without parks nor a show stop of its own: nothing of this, as before)
    if (m_parks.isEmpty() && showStopSaved() == false)
        return;
    doc->writeStartElement(KXMLDTCSafety);
    for (const ParkEntry &park : m_parks)
    {
        doc->writeStartElement(KXMLDTCSafetyPark);
        doc->writeAttribute("Fixture", QString::number(park.fixture));
        doc->writeAttribute("Channel", QString::number(park.channel));
        doc->writeAttribute("Value", QString::number(park.value));
        if (park.inBlackout)
            doc->writeAttribute("Blackout", "1");
        doc->writeAttribute("Mode", park.mode);
        doc->writeEndElement();
    }
    // the fixtures and the level of the show stop
    saveShowStopXML(doc);
    doc->writeEndElement();
}

void SafetyManager::loadXML(QXmlStreamReader &reader)
{
    while (reader.readNextStartElement())
    {
        if (reader.name() == KXMLDTCSafetyPark)
        {
            // (a bad one, a file edited by hand: skipped)
            const QXmlStreamAttributes attrs = reader.attributes();
            bool fixtureOk = false, channelOk = false, valueOk = false;
            const quint32 fixture = attrs.value("Fixture").toUInt(&fixtureOk);
            const quint32 channel = attrs.value("Channel").toUInt(&channelOk);
            const int value = attrs.value("Value").toInt(&valueOk);
            if (fixtureOk && channelOk && valueOk && value >= 0 && value <= 255 && channel < UNIVERSE_SIZE)
                m_parks.insert(qMakePair(fixture, channel),
                               ParkEntry { fixture, channel, uchar(value), attrs.value("Blackout") == QLatin1String("1"),
                                           attrs.value("Mode").toString() });
        }
        else if (reader.name() == KXMLDTCSafetyShowStop)
        {
            loadShowStopXML(reader.attributes());
        }
        reader.skipCurrentElement();
    }
}

void SafetyManager::afterLoad()
{
    // the groups of the show stop that are there (none left: every light)
    checkShowStopGroups();

    // a protected special effect with any park stays parked off (fail closed)
    QHash<quint32, bool> protectedParked;
    for (const ParkEntry &park : std::as_const(m_parks))
        if (m_doc->fixture(park.fixture) != nullptr && protectedNow(park.fixture))
            protectedParked.insert(park.fixture, protectedParked.value(park.fixture) || park.inBlackout);

    // the parks of fixtures that aren't there, of channels they don't
    // have, of another mode (a file edited by hand, a fixture changed in
    // QLC+): left out. One without a mode: the fixture's
    for (auto it = m_parks.begin(); it != m_parks.end();)
    {
        const Fixture *fxi = m_doc->fixture(it->fixture);
        if (fxi != nullptr && it->mode.isEmpty())
            it->mode = modeOf(fxi);
        if (fxi == nullptr || it->channel >= fxi->channels() || it->mode != modeOf(fxi))
            it = m_parks.erase(it);
        else
            ++it;
    }
    for (auto it = protectedParked.cbegin(); it != protectedParked.cend(); ++it)
        parkAllOff(it.key(), it.value());
    emit parksChanged();

    // they go out at once (a hazer parked at 30 %): said once the show is
    // open, with their names, in the order of their numbers
    QList<quint32> fixtures;
    for (const ParkEntry &park : std::as_const(m_parks))
        if (fixtures.contains(park.fixture) == false)
            fixtures.append(park.fixture);
    if (fixtures.isEmpty())
        return;
    std::stable_sort(fixtures.begin(), fixtures.end(), [this](quint32 a, quint32 b) {
        return m_patch->fixtureNumber(a) < m_patch->fixtureNumber(b);
    });
    QStringList names;
    for (int i = 0; i < fixtures.count() && i < 6; i++)
        names.append(fixtureName(fixtures.at(i)));
    if (fixtures.count() > names.count())
        names.append(QStringLiteral("…"));
    const QString text = tr("%n parked fixture(s): %1 (Patch)", "", int(fixtures.count())).arg(names.join(QStringLiteral(", ")));
    QMetaObject::invokeMethod(this, [this, text]() { emit message(text, false); }, Qt::QueuedConnection);
}
