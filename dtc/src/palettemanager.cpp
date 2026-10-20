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

#include "palettemanager.h"
#include "patchmanager.h"
#include "programmer.h"

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "fixturegroup.h"
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
    // the cuelists keep their references in the cues: their counts follow the edits
    connect(m_doc, SIGNAL(functionAdded(quint32)), this, SLOT(slotFunctionChanged(quint32)));
    connect(m_doc, SIGNAL(functionChanged(quint32)), this, SLOT(slotFunctionChanged(quint32)));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotFixtureRemoved(quint32)));
    // the palettes lit on the selection follow the programmer
    connect(m_programmer, &Programmer::selectionChanged, this, &PaletteManager::slotStatesChanged);
    connect(m_programmer, &Programmer::valuesChanged, this, &PaletteManager::slotStatesChanged);
}

QStringList PaletteManager::types()
{
    return { QStringLiteral("intensity"), QStringLiteral("position"), QStringLiteral("colour"), QStringLiteral("beam") };
}

QString PaletteManager::typeOf(const QLCChannel *ch)
{
    if (ch == nullptr)
        return QString();

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
    // and the cuelists whose cues use it
    for (auto it = m_cuelistRefs.cbegin(); it != m_cuelistRefs.cend(); ++it)
        if (it.value().contains(p.id))
            scenes++;

    QVariantMap map;
    map.insert("id", p.id);
    map.insert("type", p.type);
    map.insert("number", p.number);
    map.insert("name", p.name);
    map.insert("color", p.color.isValid() ? p.color.name() : QString());
    map.insert("fixtures", int(p.entries.count()));
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
            if (it == values.constEnd() || typeOf(ch) != type)
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

/** A key without its head ("red 2") nor its repetition ("gobo#2") */
static QString baseKey(const QString &key)
{
    static const QRegularExpression suffix(QStringLiteral("( \\d+|#\\d+)+$"));
    QString base = key;
    return base.remove(suffix);
}

QMap<quint32, uchar> PaletteManager::genericValues(const Palette &p, const Fixture *fxi) const
{
    QMap<quint32, uchar> values;
    if (fxi == nullptr)
        return values;

    // colours: the mix of the colour of the palette, or the closest slot of a wheel
    if (p.type == QLatin1String("colour") && p.color.isValid())
    {
        values = Programmer::colourMix(fxi, p.color);
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
                    const QColor colour = capabilityColour(cap);
                    if (colour.isValid() == false)
                        continue;
                    int d = qAbs(colour.red() - p.color.red()) + qAbs(colour.green() - p.color.green()) +
                            qAbs(colour.blue() - p.color.blue());
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
        if (values.isEmpty() == false)
            return values;
    }

    // the same attributes: as they are on continuous channels (zoom, pan...),
    // by the name of the capability on the others (shutter "Open")
    QHash<QString, const Value *> byKey;
    for (const Entry &e : p.entries)
        for (const Value &v : e.values)
            if (byKey.contains(v.key) == false)
                byKey.insert(v.key, &v);
    QHash<QString, const Value *> byBase;
    for (auto it = byKey.cbegin(); it != byKey.cend(); ++it)
        if (byBase.contains(baseKey(it.key())) == false)
            byBase.insert(baseKey(it.key()), it.value());

    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        if (ch == nullptr || typeOf(ch) != p.type)
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

    auto valuesOf = [](const Entry &e) {
        QMap<quint32, uchar> map;
        for (const Value &v : e.values)
            map.insert(v.channel, v.value);
        return map;
    };
    // its own values, else the ones of a fixture of the same type, else its attributes
    for (const Entry &e : p->entries)
        if (e.fixture == fixtureId && sameType(e, fxi))
            return valuesOf(e);
    for (const Entry &e : p->entries)
        if (sameType(e, fxi))
            return valuesOf(e);
    return genericValues(*p, fxi);
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
    p.color = type == QLatin1String("colour") ? colourOf(entries.first()) : QColor();
    p.name = name.trimmed();
    if (p.name.isEmpty())
    {
        if (type == QLatin1String("intensity"))
            p.name = tr("Intensity %1").arg(p.number);
        else if (type == QLatin1String("position"))
            p.name = tr("Position %1").arg(p.number);
        else if (type == QLatin1String("colour"))
            p.name = tr("Colour %1").arg(p.number);
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
    if (p->type == QLatin1String("colour"))
    {
        for (const Entry &e : p->entries)
        {
            QColor colour = colourOf(e);
            if (colour.isValid())
            {
                p->color = colour;
                break;
            }
        }
    }

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
    if (p->type == QLatin1String("colour"))
        p->color = colourOf(entries.first());

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
    // and so do the cues
    for (Function *f : m_doc->functionsByType(Function::CuelistType))
        if (Cuelist *cuelist = qobject_cast<Cuelist *>(f))
            cuelist->forgetPalette(id);
    m_programmer->forgetPalette(id);
    m_doc->setModified();
    emit palettesChanged();
    slotStatesChanged();
}

/*********************************************************************
 * Apply
 *********************************************************************/

int PaletteManager::apply(int id)
{
    if (find(id) == nullptr)
        return 0;
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
    const Palette *p = find(id);
    if (p == nullptr)
        return;
    const QList<quint32> fixtures = liveFixtures(*p, groupId);
    if (on)
    {
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
    const QSet<int> cuelist = m_cuelistRefs.take(id);
    if (scene || cuelist.isEmpty() == false)
        emit palettesChanged();
}

void PaletteManager::slotFunctionChanged(quint32 id)
{
    Cuelist *cuelist = qobject_cast<Cuelist *>(m_doc->function(id));
    QSet<int> used;
    if (cuelist != nullptr)
        for (const Cuelist::Cue &cue : cuelist->cues())
            for (int pid : cue.palettes)
                used.insert(pid);

    // (only a change of what it uses changes the counts)
    if (used == m_cuelistRefs.value(id))
        return;
    if (used.isEmpty())
        m_cuelistRefs.remove(id);
    else
        m_cuelistRefs.insert(id, used);
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
    // cues that name it would follow a palette they never had
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

    // (the SHOW buttons and the mappings keep the ID of a deleted palette:
    // it is never given again)
    if (usedId > m_lastId && usedId <= kMaxPaletteId)
        m_lastId = usedId;
}
