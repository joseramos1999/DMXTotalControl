/*
  DMX Total Control
  palettemanager.cpp

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
#include <QSet>
#include <algorithm>
#include <climits>
#include <cmath>

#include "palettemanager.h"
#include "patchmanager.h"
#include "programmer.h"

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "fixturegroup.h"
#include "sequence.h"
#include "cuelist.h"
#include "fxscene.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"

/** The highest palette ID or number a show file can give */
static const int kMaxPaletteId = INT_MAX / 2;

PaletteManager::PaletteManager(Doc *doc, Programmer *programmer, PatchManager *patch, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_programmer(programmer)
    , m_patch(patch)
    , m_lastId(0)
    , m_statesDirty(true)
{
    connect(m_doc, SIGNAL(functionRemoved(quint32)), this, SLOT(slotFunctionRemoved(quint32)));
    // the cuelists and steps keep their references in the cues and steps:
    // their counts follow the edits
    connect(m_doc, SIGNAL(functionAdded(quint32)), this, SLOT(slotFunctionChanged(quint32)));
    connect(m_doc, SIGNAL(functionChanged(quint32)), this, SLOT(slotFunctionChanged(quint32)));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotFixtureRemoved(quint32)));
    // the palettes lit on the selection follow the programmer
    connect(m_programmer, &Programmer::selectionChanged, this, &PaletteManager::slotStatesChanged);
    connect(m_programmer, &Programmer::valuesChanged, this, &PaletteManager::slotStatesChanged);
}

QStringList PaletteManager::types()
{
    return { QStringLiteral("intensity"), QStringLiteral("position"), QStringLiteral("colour"), QStringLiteral("beam"),
             QStringLiteral("all") };
}

bool PaletteManager::records(const QString &type, const QLCChannel *ch)
{
    // (a palette click must not switch lamps on nor reset heads)
    if (type == QLatin1String("all"))
        return ch != nullptr && ch->group() != QLCChannel::Maintenance;
    return typeOf(ch) == type;
}

QString PaletteManager::word(const QString &type)
{
    static const QHash<QString, QString> words = {
        { "intensity", "INT" }, { "position", "POS" }, { "colour", "COL" }, { "beam", "BEAM" }, { "all", "PAL" } };
    return words.value(type);
}

QString PaletteManager::typeOf(const QLCChannel *ch)
{
    if (ch == nullptr)
        return QString();

    // (the cue times of an attribute follow these rules too: Cuelist::timeGroupOf)
    const QString attribute = FXScene::attributeOf(ch);
    switch (ch->group())
    {
        case QLCChannel::Intensity:
            if (ch->colour() == QLCChannel::NoColour && attribute != QLatin1String("hue") &&
                attribute != QLatin1String("saturation") && attribute != QLatin1String("lightness"))
                return QStringLiteral("intensity");
            return QStringLiteral("colour");
        // the strobe goes with the intensity, the iris with the beam
        case QLCChannel::Shutter:
            return attribute == QLatin1String("iris") ? QStringLiteral("beam") : QStringLiteral("intensity");
        // (as the grid: pan and tilt, not a rotation of them)
        case QLCChannel::Pan:
        case QLCChannel::Tilt:
            return Programmer::isPositionChannel(ch) ? QStringLiteral("position") : QString();
        case QLCChannel::Colour:
            return QStringLiteral("colour");
        case QLCChannel::Speed:
            return attribute == QLatin1String("ptspeed") ? QStringLiteral("position") : QStringLiteral("beam");
        case QLCChannel::Gobo:
        case QLCChannel::Prism:
        case QLCChannel::Beam:
        case QLCChannel::Effect:
            return QStringLiteral("beam");
        default:
            return QString();
    }
}

/*********************************************************************
 * Palettes
 *********************************************************************/

PaletteManager::Palette *PaletteManager::find(int id)
{
    for (Palette &p : m_palettes)
        if (p.id == id)
            return &p;
    return nullptr;
}

const PaletteManager::Palette *PaletteManager::find(int id) const
{
    for (const Palette &p : m_palettes)
        if (p.id == id)
            return &p;
    return nullptr;
}

void PaletteManager::sort()
{
    const QStringList order = types();
    std::sort(m_palettes.begin(), m_palettes.end(), [&order](const Palette &a, const Palette &b) {
        if (a.type != b.type)
            return order.indexOf(a.type) < order.indexOf(b.type);
        return a.number < b.number;
    });
}

QVariantMap PaletteManager::toMap(const Palette &p) const
{
    int scenes = 0;
    for (auto it = m_sceneRefs.cbegin(); it != m_sceneRefs.cend(); ++it)
    {
        for (int pid : it.value())
        {
            if (pid == p.id)
            {
                scenes++;
                break;
            }
        }
    }
    // and the cuelists and steps that use it (each once)
    for (auto it = m_listRefs.cbegin(); it != m_listRefs.cend(); ++it)
        if (it.value().contains(p.id))
            scenes++;
    // (the places it uses: of fixtures still there, in a fan)
    int spread = 0;
    for (auto it = p.places.cbegin(); it != p.places.cend(); ++it)
        if (takesPlace(p, m_doc->fixture(it.key())))
            spread++;

    QVariantMap map;
    map.insert("id", p.id);
    map.insert("type", p.type);
    map.insert("number", p.number);
    map.insert("name", p.name);
    map.insert("code", word(p.type) + QLatin1Char(' ') + QString::number(p.number));
    map.insert("color", p.color.isValid() ? p.color.name() : QString());
    map.insert("fixtures", int(p.entries.count()));
    map.insert("fan", isFan(p));
    map.insert("spread", spread);
    map.insert("scenes", scenes);
    return map;
}

QVariantList PaletteManager::palettes() const
{
    QVariantList list;
    for (const Palette &p : m_palettes)
        list.append(toMap(p));
    return list;
}

QVariantMap PaletteManager::palette(int id) const
{
    const Palette *p = find(id);
    return p != nullptr ? toMap(*p) : QVariantMap();
}

int PaletteManager::paletteByNumber(const QString &type, int number) const
{
    for (const Palette &p : m_palettes)
        if (p.type == type && p.number == number)
            return p.id;
    return -1;
}

QStringList PaletteManager::namesOf(const QSet<int> &ids) const
{
    QStringList names;
    for (const Palette &p : m_palettes)
        if (ids.contains(p.id))
            names.append(p.name);
    return names;
}

int PaletteManager::nextNumber(const QString &type) const
{
    int last = 0;
    for (const Palette &p : m_palettes)
        if (p.type == type)
            last = qMax(last, p.number);
    return last + 1;
}

QVariantMap PaletteManager::states() const
{
    if (m_statesDirty == false)
        return m_states;

    QVariantMap map;
    const QHash<QPair<quint32, quint32>, int> refs = m_programmer->paletteRefs();
    const QList<quint32> selection = m_programmer->selectionIds();
    for (const Palette &p : m_palettes)
    {
        int applies = 0, linked = 0;
        for (quint32 fid : selection)
        {
            const QMap<quint32, uchar> values = valuesFor(p.id, fid);
            if (values.isEmpty())
                continue;
            applies++;
            bool all = true;
            for (auto it = values.cbegin(); it != values.cend() && all; ++it)
                all = refs.value(qMakePair(fid, it.key()), -1) == p.id;
            linked += all ? 1 : 0;
        }
        map.insert(QString::number(p.id), linked == 0 ? 0 : (linked == applies ? 2 : 1));
    }
    m_states = map;

    // what each type would record, made with the states
    m_recordable.clear();
    for (const QString &type : types())
    {
        int count = 0;
        for (const Entry &e : entriesFromProgrammer(type))
            count += int(e.values.count());
        m_recordable.insert(type, count);
    }
    m_statesDirty = false;
    return m_states;
}

QVariantMap PaletteManager::recordable() const
{
    states();
    return m_recordable;
}

void PaletteManager::slotStatesChanged()
{
    m_statesDirty = true;
    emit statesChanged();
}

QList<PaletteManager::Entry> PaletteManager::entriesFromProgrammer(const QString &type) const
{
    QList<Entry> list;
    const QMap<QPair<quint32, quint32>, uchar> &values = m_programmer->valueMap();
    for (quint32 fid : m_programmer->selectionIds())
    {
        Fixture *fxi = m_doc->fixture(fid);
        if (fxi == nullptr)
            continue;

        Entry entry;
        entry.fixture = fid;
        if (QLCFixtureDef *def = fxi->fixtureDef())
        {
            entry.manufacturer = def->manufacturer();
            entry.model = def->model();
        }
        if (QLCFixtureMode *mode = fxi->fixtureMode())
            entry.mode = mode->name();

        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            auto it = values.constFind(qMakePair(fid, c));
            const QLCChannel *ch = fxi->channel(c);
            if (it == values.constEnd() || records(type, ch) == false)
                continue;

            Value v;
            v.channel = c;
            v.key = m_programmer->attributeKey(fid, c);
            v.value = it.value();
            // the capability there: "Open", "Strobe slow to fast"... and where in it
            QLCCapability *cap = ch->searchCapability(v.value);
            if (cap != nullptr && ch->capabilities().count() > 1)
                v.capability = cap->name();
            entry.values.append(v);
        }
        if (entry.values.isEmpty() == false)
            list.append(entry);
    }
    return list;
}

bool PaletteManager::sameType(const Entry &entry, const Fixture *fxi) const
{
    if (fxi == nullptr)
        return false;
    QLCFixtureDef *def = fxi->fixtureDef();
    QLCFixtureMode *mode = fxi->fixtureMode();
    if ((def != nullptr ? def->manufacturer() : QString()) != entry.manufacturer ||
        (def != nullptr ? def->model() : QString()) != entry.model ||
        (mode != nullptr ? mode->name() : QString()) != entry.mode)
        return false;
    for (const Value &v : entry.values)
        if (v.channel >= fxi->channels())
            return false;
    return true;
}

bool PaletteManager::isFan(const Palette &p)
{
    for (int i = 0; i < p.entries.count(); i++)
    {
        const Entry &a = p.entries.at(i);
        for (int j = i + 1; j < p.entries.count(); j++)
        {
            const Entry &b = p.entries.at(j);
            if (a.manufacturer == b.manufacturer && a.model == b.model && a.mode == b.mode)
                return true;
        }
    }
    return false;
}

bool PaletteManager::takesPlace(const Palette &p, const Fixture *fxi) const
{
    if (fxi == nullptr || isFan(p) == false)
        return false;
    int same = 0;
    for (const Entry &e : p.entries)
    {
        if (sameType(e, fxi) == false)
            continue;
        // (recorded on it: its own values)
        if (e.fixture == fxi->id())
            return false;
        same++;
    }
    return same != 1;
}

/** The colour of a capability of a wheel: its colour, else the one its name says
 *  ("Red", "Rojo", "Light blue"); none for "Split colours", "Rotation"... */
static QColor capabilityColour(const QLCCapability *cap)
{
    if (cap->presetType() == QLCCapability::SingleColor || cap->presetType() == QLCCapability::DoubleColor)
        return cap->resource(0).value<QColor>();

    static const QHash<QString, QString> spanish = {
        { "rojo", "red" }, { "verde", "green" }, { "azul", "blue" }, { "amarillo", "yellow" },
        { "naranja", "orange" }, { "blanco", "white" }, { "rosa", "pink" }, { "morado", "purple" },
        { "violeta", "violet" }, { "cian", "cyan" }, { "ambar", "orange" }, { "amber", "orange" },
        { "open", "white" }, { "abierto", "white" }, { "uv", "darkviolet" } };
    QString name = cap->name().toLower().normalized(QString::NormalizationForm_D);
    name.erase(std::remove_if(name.begin(), name.end(), [](QChar c) { return c.category() == QChar::Mark_NonSpacing; }),
               name.end());
    static const QRegularExpression separators(QStringLiteral("[^a-z]+"));
    const QStringList words = name.split(separators, Qt::SkipEmptyParts);
    if (words.isEmpty() || words.count() > 2)
        return QColor();
    for (const QString &word : words)
    {
        const QString colour = spanish.value(word, word);
        if (QColor::isValidColorName(colour))
            return QColor::fromString(colour);
    }
    return QColor();
}

QColor PaletteManager::colourOf(const Entry &entry) const
{
    Fixture *fxi = m_doc->fixture(entry.fixture);
    if (fxi == nullptr)
        return QColor();

    int r = -1, g = -1, b = -1, w = 0;
    QColor wheel;
    for (const Value &v : entry.values)
    {
        const QLCChannel *ch = fxi->channel(v.channel);
        if (ch == nullptr || ch->controlByte() != QLCChannel::MSB)
            continue;
        if (ch->group() == QLCChannel::Intensity)
        {
            switch (ch->colour())
            {
                case QLCChannel::Red: r = v.value; break;
                case QLCChannel::Green: g = v.value; break;
                case QLCChannel::Blue: b = v.value; break;
                case QLCChannel::White: w = v.value; break;
                case QLCChannel::Cyan: r = 255 - v.value; break;
                case QLCChannel::Magenta: g = 255 - v.value; break;
                case QLCChannel::Yellow: b = 255 - v.value; break;
                default: break;
            }
        }
        else if (ch->group() == QLCChannel::Colour && wheel.isValid() == false)
        {
            // the colour of the slot of a colour wheel
            QLCCapability *cap = ch->searchCapability(v.value);
            if (cap != nullptr)
                wheel = capabilityColour(cap);
        }
    }
    if (r >= 0 || g >= 0 || b >= 0)
    {
        // the white emitter, on the three
        return QColor(qMin(255, qMax(0, r) + w), qMin(255, qMax(0, g) + w), qMin(255, qMax(0, b) + w));
    }
    if (w > 0)
        return QColor(w, w, w);
    return wheel;
}

QColor PaletteManager::colourOf(const QString &type, const QList<Entry> &entries) const
{
    if (type != QLatin1String("colour") && type != QLatin1String("all"))
        return QColor();
    for (const Entry &e : entries)
    {
        const QColor colour = colourOf(e);
        if (colour.isValid())
            return colour;
    }
    return QColor();
}

/** A key without its head ("red 2") nor its repetition ("gobo#2") */
static QString baseKey(const QString &key)
{
    static const QRegularExpression suffix(QStringLiteral("( \\d+|#\\d+)+$"));
    QString base = key;
    return base.remove(suffix);
}

QMap<quint32, uchar> PaletteManager::genericValues(const Palette &p, const Fixture *fxi, qreal place) const
{
    QMap<quint32, uchar> values;
    if (fxi == nullptr)
        return values;

    // colours: the mix of the colour of the palette (at its place in a fan),
    // or the closest slot of a wheel
    const bool all = p.type == QLatin1String("all");
    const bool fanned = place >= 0.0 && isFan(p);
    const QColor colour = fanned ? fanColour(p, place) : p.color;
    if ((p.type == QLatin1String("colour") || all) && colour.isValid())
    {
        values = Programmer::colourMix(fxi, colour);
        if (values.isEmpty())
        {
            for (quint32 c = 0; c < fxi->channels(); c++)
            {
                const QLCChannel *ch = fxi->channel(c);
                if (ch == nullptr || ch->group() != QLCChannel::Colour || ch->controlByte() != QLCChannel::MSB)
                    continue;
                int best = -1, distance = INT_MAX;
                for (QLCCapability *cap : ch->capabilities())
                {
                    const QColor slot = capabilityColour(cap);
                    if (slot.isValid() == false)
                        continue;
                    int d = qAbs(slot.red() - colour.red()) + qAbs(slot.green() - colour.green()) +
                            qAbs(slot.blue() - colour.blue());
                    if (d < distance)
                    {
                        distance = d;
                        best = (cap->min() + cap->max()) / 2;
                    }
                }
                if (best >= 0)
                {
                    values.insert(c, uchar(best));
                    break;
                }
            }
        }
        if (values.isEmpty() == false && all == false)
            return values;
    }
    // ("all": its colour is that mix, the rest goes by attribute)
    const bool mixed = values.isEmpty() == false;

    // the same attributes: as they are on continuous channels (zoom, pan...),
    // by the name of the capability on the others (shutter "Open"). In a
    // fan, each one from the nearest recorded fixture that has it
    QList<int> order;
    for (int i = 0; i < p.entries.count(); i++)
        order.append(i);
    if (fanned)
    {
        // (halfway between two, the later one, as fanValues())
        const qreal x = place * qreal(p.entries.count() - 1);
        std::stable_sort(order.begin(), order.end(), [x](int a, int b) {
            const qreal da = qAbs(a - x), db = qAbs(b - x);
            return da != db ? da < db : a > b;
        });
    }
    QHash<QString, const Value *> byKey, byBase;
    for (int i : std::as_const(order))
    {
        for (const Value &v : p.entries.at(i).values)
        {
            if (byKey.contains(v.key) == false)
                byKey.insert(v.key, &v);
            if (byBase.contains(baseKey(v.key)) == false)
                byBase.insert(baseKey(v.key), &v);
        }
    }

    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        if (ch == nullptr || records(p.type, ch) == false || values.contains(c) ||
            (mixed && typeOf(ch) == QLatin1String("colour")))
            continue;
        const QString key = m_programmer->attributeKey(fxi->id(), c);
        const Value *v = byKey.value(key, byBase.value(baseKey(key), nullptr));
        if (v == nullptr)
            continue;

        if (ch->capabilities().count() <= 1)
        {
            values.insert(c, v->value);
            continue;
        }
        if (v->capability.isEmpty())
            continue;
        // wheels only share their open slot: gobo 3 of a fixture is not the one of another
        const bool wheel = key.startsWith(QLatin1String("gobo")) || key.startsWith(QLatin1String("colorwheel")) ||
                           key.startsWith(QLatin1String("colormacro"));
        if (wheel && v->capability.contains("open", Qt::CaseInsensitive) == false &&
            v->capability.contains("white", Qt::CaseInsensitive) == false)
            continue;
        for (QLCCapability *cap : ch->capabilities())
        {
            if (cap->name().compare(v->capability, Qt::CaseInsensitive) == 0)
            {
                values.insert(c, uchar((cap->min() + cap->max()) / 2));
                break;
            }
        }
    }
    return values;
}

QMap<quint32, uchar> PaletteManager::valuesFor(int id, quint32 fixtureId) const
{
    QMap<quint32, uchar> values;
    const Palette *p = find(id);
    Fixture *fxi = m_doc->fixture(fixtureId);
    if (p == nullptr || fxi == nullptr)
        return values;

    // its own values
    for (const Entry &e : p->entries)
        if (e.fixture == fixtureId && sameType(e, fxi))
            return valuesOf(e);
    // else the fan of the fixtures of the same type, at its place (without
    // one yet, the first of them); else its attributes
    QList<const Entry *> same;
    for (const Entry &e : p->entries)
        if (sameType(e, fxi))
            same.append(&e);
    const qreal place = p->places.value(fixtureId, -1.0);
    if (same.count() >= 2 && place >= 0.0)
        return fanValues(same, fxi, place);
    if (same.isEmpty() == false)
        return valuesOf(*same.first());
    return genericValues(*p, fxi, place);
}

QMap<quint32, uchar> PaletteManager::valuesOf(const Entry &entry)
{
    QMap<quint32, uchar> map;
    for (const Value &v : entry.values)
        map.insert(v.channel, v.value);
    return map;
}

/** Where $place (0..1) falls in a fan of $count: the one before it and how far to the next */
static int fanSegment(qreal place, int count, qreal *fraction)
{
    const qreal x = qBound(0.0, place, 1.0) * qreal(count - 1);
    const int k = qMin(int(std::floor(x)), count - 2);
    *fraction = x - k;
    return k;
}

QMap<quint32, uchar> PaletteManager::fanValues(const QList<const Entry *> &fan, const Fixture *fxi, qreal place) const
{
    qreal f = 0.0;
    const int k = fanSegment(place, int(fan.count()), &f);
    const QMap<quint32, uchar> a = valuesOf(*fan.at(k));
    const QMap<quint32, uchar> b = valuesOf(*fan.at(k + 1));

    // 16 bit: a fine channel goes with the coarse one of its attribute and head
    QHash<quint32, quint32> fineOf;
    QSet<quint32> fines;
    for (auto l = a.cbegin(); l != a.cend(); ++l)
    {
        const QLCChannel *lsb = fxi->channel(l.key());
        if (lsb == nullptr || lsb->controlByte() != QLCChannel::LSB || b.contains(l.key()) == false)
            continue;
        const QString attribute = FXScene::attributeOf(lsb);
        const int head = FXScene::headOf(fxi, l.key());
        for (auto m = a.cbegin(); m != a.cend(); ++m)
        {
            const QLCChannel *msb = fxi->channel(m.key());
            if (msb == nullptr || msb->controlByte() != QLCChannel::MSB || msb->group() != lsb->group() ||
                b.contains(m.key()) == false || fineOf.contains(m.key()) ||
                FXScene::attributeOf(msb) != attribute || FXScene::headOf(fxi, m.key()) != head)
                continue;
            fineOf.insert(m.key(), l.key());
            fines.insert(l.key());
            break;
        }
    }

    QMap<quint32, uchar> values = b;
    for (auto it = a.cbegin(); it != a.cend(); ++it)
    {
        const quint32 c = it.key();
        // (what only one of them has, as it has it)
        if (b.contains(c) == false)
        {
            values.insert(c, it.value());
            continue;
        }
        // (with its coarse channel)
        if (fines.contains(c))
            continue;
        const int from = it.value(), to = b.value(c);
        const QLCChannel *ch = fxi->channel(c);
        auto fine = fineOf.constFind(c);
        if (fine != fineOf.cend())
        {
            const int wideFrom = (from << 8) | a.value(fine.value());
            const int wideTo = (to << 8) | b.value(fine.value());
            const int wide = qRound(wideFrom + (wideTo - wideFrom) * f);
            values.insert(c, uchar(wide >> 8));
            values.insert(fine.value(), uchar(wide & 0xff));
        }
        else if (ch == nullptr || ch->capabilities().count() <= 1)
        {
            values.insert(c, uchar(qRound(from + (to - from) * f)));
        }
        else
        {
            // a wheel, a shutter...: the slot of the nearest one, never one in between
            values.insert(c, uchar(f < 0.5 ? from : to));
        }
    }
    return values;
}

QColor PaletteManager::fanColour(const Palette &p, qreal place) const
{
    QList<QColor> colours;
    for (const Entry &e : p.entries)
    {
        const QColor colour = colourOf(e);
        if (colour.isValid())
            colours.append(colour);
    }
    if (colours.count() < 2)
        return colours.isEmpty() ? p.color : colours.first();

    qreal f = 0.0;
    const int k = fanSegment(place, int(colours.count()), &f);
    const QColor &from = colours.at(k), &to = colours.at(k + 1);
    return QColor(qRound(from.red() + (to.red() - from.red()) * f),
                  qRound(from.green() + (to.green() - from.green()) * f),
                  qRound(from.blue() + (to.blue() - from.blue()) * f));
}

/*********************************************************************
 * Record, update, delete
 *********************************************************************/

int PaletteManager::record(const QString &type, const QString &name, int number)
{
    if (types().contains(type) == false)
        return -1;
    const QList<Entry> entries = entriesFromProgrammer(type);
    if (entries.isEmpty())
        return -1;

    Palette p;
    p.id = ++m_lastId;
    p.type = type;
    p.number = number > 0 && paletteByNumber(type, number) < 0 ? number : nextNumber(type);
    p.entries = entries;
    p.color = colourOf(type, entries);
    p.name = name.trimmed();
    if (p.name.isEmpty())
    {
        if (type == QLatin1String("intensity"))
            p.name = tr("Intensity %1").arg(p.number);
        else if (type == QLatin1String("position"))
            p.name = tr("Position %1").arg(p.number);
        else if (type == QLatin1String("colour"))
            p.name = tr("Colour %1").arg(p.number);
        else if (type == QLatin1String("all"))
            p.name = tr("All %1").arg(p.number);
        else
            p.name = tr("Beam %1").arg(p.number);
    }
    m_palettes.append(p);
    sort();

    // what was recorded now comes from the palette
    linkProgrammer(p.id);
    m_doc->setModified();
    emit palettesChanged();
    slotStatesChanged();
    return p.id;
}

void PaletteManager::linkProgrammer(int id)
{
    const Palette *p = find(id);
    if (p == nullptr)
        return;
    QHash<QPair<quint32, quint32>, int> refs = m_programmer->paletteRefs();
    const QList<quint32> selection = m_programmer->selectionIds();
    for (const Entry &e : p->entries)
    {
        if (selection.contains(e.fixture) == false)
            continue;
        for (const Value &v : e.values)
            if (m_programmer->valueMap().contains(qMakePair(e.fixture, v.channel)))
                refs.insert(qMakePair(e.fixture, v.channel), id);
    }
    m_programmer->setPaletteRefs(refs);
}

bool PaletteManager::update(int id)
{
    Palette *p = find(id);
    if (p == nullptr)
        return false;
    const QList<Entry> entries = entriesFromProgrammer(p->type);
    if (entries.isEmpty())
        return false;

    for (const Entry &entry : entries)
    {
        auto it = std::find_if(p->entries.begin(), p->entries.end(),
                               [&entry](const Entry &e) { return e.fixture == entry.fixture; });
        if (it == p->entries.end() || it->mode != entry.mode || it->model != entry.model)
        {
            if (it != p->entries.end())
                p->entries.erase(it);
            p->entries.append(entry);
            continue;
        }
        // the channels set now replace theirs, the others stay
        for (const Value &v : entry.values)
        {
            auto old = std::find_if(it->values.begin(), it->values.end(),
                                    [&v](const Value &o) { return o.channel == v.channel; });
            if (old != it->values.end())
                *old = v;
            else
                it->values.append(v);
        }
    }
    // (a palette that holds no colour any more keeps the one it had)
    const QColor colour = colourOf(p->type, p->entries);
    if (colour.isValid())
        p->color = colour;
    // recorded now: their own values, no place in the fan
    for (const Entry &entry : entries)
        p->places.remove(entry.fixture);

    linkProgrammer(id);
    propagate(id);
    m_doc->setModified();
    emit palettesChanged();
    slotStatesChanged();
    return true;
}

bool PaletteManager::replace(int id)
{
    Palette *p = find(id);
    if (p == nullptr)
        return false;
    const QList<Entry> entries = entriesFromProgrammer(p->type);
    if (entries.isEmpty())
        return false;

    p->entries = entries;
    p->color = colourOf(p->type, entries);
    for (const Entry &entry : entries)
        p->places.remove(entry.fixture);

    linkProgrammer(id);
    propagate(id);
    m_doc->setModified();
    emit palettesChanged();
    slotStatesChanged();
    return true;
}

void PaletteManager::propagate(int id)
{
    // the programmer: its values of the palette take the new ones
    QHash<QPair<quint32, quint32>, int> refs = m_programmer->paletteRefs();
    bool dropped = false;
    for (auto it = refs.begin(); it != refs.end();)
    {
        if (it.value() != id)
        {
            ++it;
            continue;
        }
        const QMap<quint32, uchar> values = valuesFor(id, it.key().first);
        if (values.contains(it.key().second) == false)
        {
            // the palette doesn't say anything there any more: a plain value
            it = refs.erase(it);
            dropped = true;
            continue;
        }
        if (m_programmer->valueMap().value(it.key()) != values.value(it.key().second))
            m_programmer->setPaletteValue(it.key().first, it.key().second, values.value(it.key().second));
        ++it;
    }
    if (dropped)
        m_programmer->setPaletteRefs(refs);

    // the scenes recorded with it
    for (auto f = m_sceneRefs.begin(); f != m_sceneRefs.end(); ++f)
    {
        Function *function = m_doc->function(f.key());
        Scene *scene = qobject_cast<Scene *>(function);
        FXScene *fx = qobject_cast<FXScene *>(function);
        for (auto it = f.value().begin(); it != f.value().end();)
        {
            if (it.value() != id)
            {
                ++it;
                continue;
            }
            const QMap<quint32, uchar> values = valuesFor(id, it.key().first);
            if (values.contains(it.key().second) == false)
            {
                it = f.value().erase(it);
                continue;
            }
            const uchar value = values.value(it.key().second);
            if (scene != nullptr)
                scene->setValue(it.key().first, it.key().second, value);
            else if (fx != nullptr)
                fx->setValue(it.key().first, it.key().second, value);
            ++it;
        }
    }

    // the cuelists: their cues keep the references. A running list plays the
    // new values at once, and the cues that track them follow
    QHash<quint32, QMap<quint32, uchar>> byFixture;
    auto valueOf = [&](quint32 fixture, quint32 channel) {
        auto found = byFixture.constFind(fixture);
        if (found == byFixture.cend())
            found = byFixture.insert(fixture, valuesFor(id, fixture));
        return found.value().contains(channel) ? int(found.value().value(channel)) : -1;
    };
    for (Function *f : m_doc->functionsByType(Function::CuelistType))
        if (Cuelist *cuelist = qobject_cast<Cuelist *>(f))
            cuelist->applyPalette(id, valueOf);
    // and the steps, the same way: the step on stage plays them at once
    for (Function *f : m_doc->functionsByType(Function::SequenceType))
        if (Sequence *sequence = qobject_cast<Sequence *>(f))
            sequence->applyPalette(id, valueOf);
}

void PaletteManager::rename(int id, const QString &name)
{
    Palette *p = find(id);
    if (p == nullptr || name.trimmed().isEmpty() || p->name == name.trimmed())
        return;
    p->name = name.trimmed();
    m_doc->setModified();
    emit palettesChanged();
}

QString PaletteManager::setNumber(int id, int number)
{
    Palette *p = find(id);
    if (p == nullptr)
        return tr("Palette not found");
    if (number < 1)
        return tr("The number must be 1 or more");
    int other = paletteByNumber(p->type, number);
    if (other >= 0 && other != id)
        return tr("Number %1 is taken by %2").arg(number).arg(find(other)->name);
    if (p->number == number)
        return QString();
    p->number = number;
    sort();
    m_doc->setModified();
    emit palettesChanged();
    return QString();
}

void PaletteManager::remove(int id)
{
    const int before = int(m_palettes.count());
    m_palettes.erase(std::remove_if(m_palettes.begin(), m_palettes.end(),
                                    [id](const Palette &p) { return p.id == id; }), m_palettes.end());
    if (m_palettes.count() == before)
        return;

    // the scenes keep the values, as plain values
    for (auto f = m_sceneRefs.begin(); f != m_sceneRefs.end(); ++f)
    {
        for (auto it = f.value().begin(); it != f.value().end();)
        {
            if (it.value() == id)
                it = f.value().erase(it);
            else
                ++it;
        }
    }
    // and so do the cues and the steps
    for (Function *f : m_doc->functionsByType(Function::CuelistType))
        if (Cuelist *cuelist = qobject_cast<Cuelist *>(f))
            cuelist->forgetPalette(id);
    for (Function *f : m_doc->functionsByType(Function::SequenceType))
        if (Sequence *sequence = qobject_cast<Sequence *>(f))
            sequence->forgetPalette(id);
    m_programmer->forgetPalette(id);
    m_doc->setModified();
    emit palettesChanged();
    slotStatesChanged();
}

/*********************************************************************
 * Apply
 *********************************************************************/

int PaletteManager::placeFixtures(Palette &p, const QList<quint32> &ordered, bool again)
{
    if (isFan(p) == false)
        return 0;
    // the fixtures it was not recorded on, that it applies to, in their order
    // (one of a type recorded on a single fixture takes its values: no place)
    QList<quint32> candidates;
    for (quint32 fid : ordered)
    {
        const Fixture *fxi = m_doc->fixture(fid);
        if (candidates.contains(fid) || takesPlace(p, fxi) == false)
            continue;
        if (valuesFor(p.id, fid).isEmpty() == false)
            candidates.append(fid);
    }

    // as the position fan: from one end to the other, one alone in the
    // middle (rounded as the file keeps it: opened again, the same values)
    const int n = int(candidates.count());
    int placed = 0;
    bool changed = false;
    for (int i = 0; i < n; i++)
    {
        const quint32 fid = candidates.at(i);
        if (again == false && p.places.contains(fid))
            continue;
        const qreal t = n > 1 ? qRound(10000.0 * i / (n - 1)) / 10000.0 : 0.5;
        placed++;
        if (p.places.value(fid, -1.0) != t)
        {
            p.places.insert(fid, t);
            changed = true;
        }
    }
    if (changed)
    {
        // (what used one of them through the first fixture of its type
        // follows its place now)
        m_doc->setModified();
        propagate(p.id);
        emit palettesChanged();
        slotStatesChanged();
    }
    return placed;
}

int PaletteManager::spreadAgain(int id)
{
    Palette *p = find(id);
    if (p == nullptr)
        return 0;
    const int placed = placeFixtures(*p, m_programmer->selectionIds(), true);
    if (placed > 0)
        apply(id);
    return placed;
}

int PaletteManager::apply(int id)
{
    Palette *p = find(id);
    if (p == nullptr)
        return 0;
    // the fixtures it was not recorded on take their place in its fan
    placeFixtures(*p, m_programmer->selectionIds(), false);
    QHash<quint32, QMap<quint32, uchar>> values;
    for (quint32 fid : m_programmer->selectionIds())
    {
        QMap<quint32, uchar> fixtureValues = valuesFor(id, fid);
        if (fixtureValues.isEmpty() == false)
            values.insert(fid, fixtureValues);
    }
    if (values.isEmpty() == false)
        m_programmer->applyPaletteValues(values, id);
    return int(values.count());
}

bool PaletteManager::load(int id)
{
    const Palette *p = find(id);
    if (p == nullptr)
        return false;
    QVariantList fixtures;
    for (const Entry &e : p->entries)
        if (m_doc->fixture(e.fixture) != nullptr)
            fixtures.append(e.fixture);
    if (fixtures.isEmpty())
        return false;
    m_programmer->selectFixtures(fixtures);
    return apply(id) > 0;
}

QList<quint32> PaletteManager::liveFixtures(const Palette &p, int groupId) const
{
    QList<quint32> candidates;
    if (groupId >= 0)
    {
        if (FixtureGroup *group = m_doc->fixtureGroup(quint32(groupId)))
            candidates = group->fixtureList();
    }
    else
    {
        for (const Entry &e : p.entries)
            candidates.append(e.fixture);
    }

    QList<quint32> fixtures;
    for (quint32 fid : std::as_const(candidates))
        if (fixtures.contains(fid) == false && valuesFor(p.id, fid).isEmpty() == false)
            fixtures.append(fid);
    return fixtures;
}

void PaletteManager::setLive(int id, int groupId, bool on)
{
    Palette *p = find(id);
    if (p == nullptr)
        return;
    const QList<quint32> fixtures = liveFixtures(*p, groupId);
    if (on)
    {
        // (its fan in the order of the group)
        placeFixtures(*p, fixtures, false);
        QHash<quint32, QMap<quint32, uchar>> values;
        for (quint32 fid : fixtures)
            values.insert(fid, valuesFor(id, fid));
        m_programmer->applyPaletteValues(values, id);
    }
    else
    {
        m_programmer->releasePaletteValues(fixtures, id);
    }
}

bool PaletteManager::isLive(int id, int groupId) const
{
    const Palette *p = find(id);
    if (p == nullptr)
        return false;
    const QList<quint32> fixtures = liveFixtures(*p, groupId);
    if (fixtures.isEmpty())
        return false;
    const QHash<QPair<quint32, quint32>, int> refs = m_programmer->paletteRefs();
    for (quint32 fid : fixtures)
    {
        const QMap<quint32, uchar> values = valuesFor(id, fid);
        for (auto it = values.cbegin(); it != values.cend(); ++it)
            if (refs.value(qMakePair(fid, it.key()), -1) != id)
                return false;
    }
    return true;
}

/*********************************************************************
 * Scenes
 *********************************************************************/

QHash<QPair<quint32, quint32>, int> PaletteManager::sceneRefs(quint32 functionId) const
{
    return m_sceneRefs.value(functionId);
}

void PaletteManager::setSceneRefs(quint32 functionId, const QHash<QPair<quint32, quint32>, int> &refs)
{
    if (refs.isEmpty())
    {
        if (m_sceneRefs.remove(functionId) == 0)
            return;
    }
    else
    {
        m_sceneRefs.insert(functionId, refs);
    }
    emit palettesChanged();
}

void PaletteManager::mergeSceneRefs(quint32 functionId, const QHash<QPair<quint32, quint32>, int> &refs,
                                    const QList<QPair<quint32, quint32>> &written)
{
    QHash<QPair<quint32, quint32>, int> merged = m_sceneRefs.value(functionId);
    for (const QPair<quint32, quint32> &key : written)
        merged.remove(key);
    for (auto it = refs.cbegin(); it != refs.cend(); ++it)
        merged.insert(it.key(), it.value());
    setSceneRefs(functionId, merged);
}

QVariantList PaletteManager::scenePalettes(quint32 functionId) const
{
    QMap<int, int> channels;
    const QHash<QPair<quint32, quint32>, int> refs = m_sceneRefs.value(functionId);
    for (int pid : refs)
        channels[pid]++;

    QVariantList list;
    for (const Palette &p : m_palettes)
    {
        if (channels.contains(p.id) == false)
            continue;
        QVariantMap map = toMap(p);
        map.insert("channels", channels.value(p.id));
        list.append(map);
    }
    return list;
}

void PaletteManager::slotFunctionRemoved(quint32 id)
{
    const bool scene = m_sceneRefs.remove(id) > 0;
    const QSet<int> list = m_listRefs.take(id);
    if (scene || list.isEmpty() == false)
        emit palettesChanged();
}

void PaletteManager::slotFunctionChanged(quint32 id)
{
    Function *function = m_doc->function(id);
    QSet<int> used;
    if (Cuelist *cuelist = qobject_cast<Cuelist *>(function))
    {
        for (const Cuelist::Cue &cue : cuelist->cues())
            for (int pid : cue.palettes)
                used.insert(pid);
    }
    else if (Sequence *sequence = qobject_cast<Sequence *>(function))
    {
        for (const ChaserStep &step : sequence->steps())
            for (int pid : step.palettes)
                used.insert(pid);
    }

    // (only a change of what it uses changes the counts)
    if (used == m_listRefs.value(id))
        return;
    if (used.isEmpty())
        m_listRefs.remove(id);
    else
        m_listRefs.insert(id, used);
    emit palettesChanged();
}

void PaletteManager::slotFixtureRemoved(quint32 id)
{
    bool changed = false;
    for (auto f = m_sceneRefs.begin(); f != m_sceneRefs.end();)
    {
        for (auto it = f.value().begin(); it != f.value().end();)
        {
            if (it.key().first == id)
            {
                it = f.value().erase(it);
                changed = true;
            }
            else
            {
                ++it;
            }
        }
        // a scene with no values from palettes left doesn't use any
        if (f.value().isEmpty())
            f = m_sceneRefs.erase(f);
        else
            ++f;
    }
    // and its place in the fans
    for (Palette &p : m_palettes)
        if (p.places.remove(id) > 0)
            changed = true;
    // the scenes each palette is used by, and the palettes of a scene
    if (changed)
        emit palettesChanged();
}

/*********************************************************************
 * Load & save
 *********************************************************************/

void PaletteManager::clear()
{
    m_palettes.clear();
    m_sceneRefs.clear();
    m_lastId = 0;
    emit palettesChanged();
    slotStatesChanged();
}

bool PaletteManager::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLDTCPalettes)
        return false;

    m_palettes.clear();
    m_sceneRefs.clear();
    // the IDs of deleted palettes are not given again: the SHOW buttons and
    // mappings that still name one would apply another palette
    // (an ID out of range, in a damaged file, would overflow the next ones)
    m_lastId = qBound(0, root.attributes().value("LastId").toInt(), kMaxPaletteId);
    QHash<quint32, QHash<QPair<quint32, quint32>, int>> refs;
    while (root.readNextStartElement())
    {
        if (root.name() == QLatin1String("Palette"))
        {
            Palette p;
            bool idOk = false;
            p.id = root.attributes().value("ID").toInt(&idOk);
            p.type = root.attributes().value("Type").toString();
            p.number = root.attributes().value("Number").toInt();
            p.name = root.attributes().value("Name").toString();
            p.color = QColor(root.attributes().value("Color").toString());
            while (root.readNextStartElement())
            {
                // the place of a fixture in its fan: one that is there, from 0 to 1
                if (root.name() == QLatin1String("Place"))
                {
                    bool fixtureOk = false, atOk = false;
                    const quint32 fixture = root.attributes().value("Fixture").toUInt(&fixtureOk);
                    const qreal at = root.attributes().value("At").toDouble(&atOk);
                    if (fixtureOk && atOk && qIsFinite(at) && at >= 0.0 && at <= 1.0 && m_doc->fixture(fixture) != nullptr)
                        p.places.insert(fixture, at);
                    root.skipCurrentElement();
                    continue;
                }
                if (root.name() != QLatin1String("Fixture"))
                {
                    root.skipCurrentElement();
                    continue;
                }
                Entry e;
                bool fixtureOk = false;
                e.fixture = root.attributes().value("ID").toUInt(&fixtureOk);
                e.manufacturer = root.attributes().value("Manufacturer").toString();
                e.model = root.attributes().value("Model").toString();
                e.mode = root.attributes().value("Mode").toString();
                while (root.readNextStartElement())
                {
                    if (root.name() != QLatin1String("Value"))
                    {
                        root.skipCurrentElement();
                        continue;
                    }
                    Value v;
                    bool channelOk = false, valueOk = false;
                    v.channel = root.attributes().value("Channel").toUInt(&channelOk);
                    v.key = root.attributes().value("Key").toString();
                    v.capability = root.attributes().value("Capability").toString();
                    // (what a newer version puts inside is skipped)
                    const int value = root.readElementText(QXmlStreamReader::SkipChildElements).trimmed().toInt(&valueOk);
                    v.value = uchar(qBound(0, value, 255));
                    if (channelOk && valueOk)
                        e.values.append(v);
                }
                if (fixtureOk && e.values.isEmpty() == false)
                    p.entries.append(e);
            }
            // (a fixture recorded in it has its own values)
            for (const Entry &e : std::as_const(p.entries))
                if (sameType(e, m_doc->fixture(e.fixture)))
                    p.places.remove(e.fixture);
            if (types().contains(p.type) && idOk && p.id > 0 && p.id <= kMaxPaletteId && find(p.id) == nullptr)
            {
                if (p.number < 1 || p.number > kMaxPaletteId || paletteByNumber(p.type, p.number) >= 0)
                    p.number = nextNumber(p.type);
                m_lastId = qMax(m_lastId, p.id);
                m_palettes.append(p);
            }
        }
        else if (root.name() == QLatin1String("Refs"))
        {
            bool functionOk = false;
            const quint32 fid = root.attributes().value("Function").toUInt(&functionOk);
            for (const QString &ref : root.readElementText(QXmlStreamReader::SkipChildElements).split(',', Qt::SkipEmptyParts))
            {
                const QStringList parts = ref.trimmed().split(':');
                bool fixtureOk = false, channelOk = false, paletteOk = false;
                const quint32 fixture = parts.value(0).toUInt(&fixtureOk);
                const quint32 channel = parts.value(1).toUInt(&channelOk);
                const int palette = parts.value(2).toInt(&paletteOk);
                if (functionOk && parts.count() == 3 && fixtureOk && channelOk && paletteOk)
                    refs[fid].insert(qMakePair(fixture, channel), palette);
            }
        }
        else
        {
            root.skipCurrentElement();
        }
    }

    // only the references to what is there
    for (auto f = refs.cbegin(); f != refs.cend(); ++f)
    {
        if (m_doc->function(f.key()) == nullptr)
            continue;
        QHash<QPair<quint32, quint32>, int> kept;
        for (auto it = f.value().cbegin(); it != f.value().cend(); ++it)
            if (find(it.value()) != nullptr && m_doc->fixture(it.key().first) != nullptr)
                kept.insert(it.key(), it.value());
        if (kept.isEmpty() == false)
            m_sceneRefs.insert(f.key(), kept);
    }
    sort();
    emit palettesChanged();
    slotStatesChanged();
    return true;
}

bool PaletteManager::saveXML(QXmlStreamWriter *doc) const
{
    // (with every palette deleted, the last ID still counts)
    if (m_palettes.isEmpty() && m_sceneRefs.isEmpty() && m_lastId == 0)
        return true;

    doc->writeStartElement(KXMLDTCPalettes);
    doc->writeAttribute("LastId", QString::number(m_lastId));
    for (const Palette &p : m_palettes)
    {
        doc->writeStartElement("Palette");
        doc->writeAttribute("ID", QString::number(p.id));
        doc->writeAttribute("Type", p.type);
        doc->writeAttribute("Number", QString::number(p.number));
        doc->writeAttribute("Name", p.name);
        if (p.color.isValid())
            doc->writeAttribute("Color", p.color.name());
        for (const Entry &e : p.entries)
        {
            doc->writeStartElement("Fixture");
            doc->writeAttribute("ID", QString::number(e.fixture));
            doc->writeAttribute("Manufacturer", e.manufacturer);
            doc->writeAttribute("Model", e.model);
            doc->writeAttribute("Mode", e.mode);
            for (const Value &v : e.values)
            {
                doc->writeStartElement("Value");
                doc->writeAttribute("Channel", QString::number(v.channel));
                doc->writeAttribute("Key", v.key);
                if (v.capability.isEmpty() == false)
                    doc->writeAttribute("Capability", v.capability);
                doc->writeCharacters(QString::number(v.value));
                doc->writeEndElement();
            }
            doc->writeEndElement();
        }
        // the places in its fan, by fixture
        QList<quint32> placed = p.places.keys();
        std::sort(placed.begin(), placed.end());
        for (quint32 fid : std::as_const(placed))
        {
            if (m_doc->fixture(fid) == nullptr)
                continue;
            doc->writeEmptyElement("Place");
            doc->writeAttribute("Fixture", QString::number(fid));
            doc->writeAttribute("At", QString::number(p.places.value(fid), 'f', 4));
        }
        doc->writeEndElement();
    }

    // the scenes and the palettes of their values: fixture:channel:palette
    QList<quint32> functions = m_sceneRefs.keys();
    std::sort(functions.begin(), functions.end());
    for (quint32 fid : std::as_const(functions))
    {
        if (m_doc->function(fid) == nullptr)
            continue;
        QStringList refs;
        const QHash<QPair<quint32, quint32>, int> &map = m_sceneRefs[fid];
        QList<QPair<quint32, quint32>> keys = map.keys();
        std::sort(keys.begin(), keys.end());
        for (const QPair<quint32, quint32> &key : std::as_const(keys))
            refs.append(QString("%1:%2:%3").arg(key.first).arg(key.second).arg(map.value(key)));
        doc->writeStartElement("Refs");
        doc->writeAttribute("Function", QString::number(fid));
        doc->writeCharacters(refs.join(','));
        doc->writeEndElement();
    }
    doc->writeEndElement();
    return true;
}

void PaletteManager::afterLoad(int usedId)
{
    // a palette recorded later would get the ID of a missing one: the
    // cues and steps that name it would follow a palette they never had
    for (Function *f : m_doc->functionsByType(Function::CuelistType))
    {
        Cuelist *cuelist = qobject_cast<Cuelist *>(f);
        if (cuelist == nullptr)
            continue;
        QSet<int> missing;
        for (const Cuelist::Cue &cue : cuelist->cues())
            for (int pid : cue.palettes)
                if (find(pid) == nullptr)
                    missing.insert(pid);
        for (int pid : std::as_const(missing))
            cuelist->forgetPalette(pid);
    }
    for (Function *f : m_doc->functionsByType(Function::SequenceType))
    {
        Sequence *sequence = qobject_cast<Sequence *>(f);
        if (sequence == nullptr)
            continue;
        QSet<int> missing;
        for (const ChaserStep &step : sequence->steps())
            for (int pid : step.palettes)
                if (find(pid) == nullptr)
                    missing.insert(pid);
        for (int pid : std::as_const(missing))
            sequence->forgetPalette(pid);
    }

    // (the SHOW buttons and the mappings keep the ID of a deleted palette:
    // it is never given again)
    if (usedId > m_lastId && usedId <= kMaxPaletteId)
        m_lastId = usedId;
}
