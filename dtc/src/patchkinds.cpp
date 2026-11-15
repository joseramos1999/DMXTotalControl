/*
  DMX Total Control
  patchkinds.cpp

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

/* PatchManager: the kind of each fixture (the glyph the plan draws it with:
 * a moving head, a strobe, a blinder, a CO2 jet...), the icon chosen by hand,
 * and the fixture types of the show the plan colours and lists in its legend */

#include <QRegularExpression>
#include <algorithm>

#include "patchmanager.h"
#include "qlcfixturedef.h"
#include "fixture.h"
#include "doc.h"

namespace
{

/* The kinds, in the order of the Look menu */
const char *const kKinds[] = {
    "movinghead", "scanner", "wash", "ledbar", "strobe", "blinder", "dimmer", "laser", "smoke", "hazer",
    "co2", "flame", "spark", "confetti", "bubbles", "snow", "fan", "effect", "other"
};

/* The words of each kind, in the order they are looked for: the special
 * effects first ("Strobe Flame" is a flame), then the lights. In lower case
 * and without accents; "starts": the start of a word ("flame" in "Flamer"),
 * "wholes": a word of its own ("par", not "parade"); "anywhere": inside a
 * word too ("co2" in "MEGACO2JET", but not the end of "FOCO2" or "FOCO21":
 * a "co2" glued to letters on its left needs letters on its right). Some
 * starts leave out a longer common word ("Sparkle", "Snowball", "Firefly") */
struct KindWords
{
    const char *kind;
    const char *starts;
    const char *wholes;
    const char *anywhere;
};
const KindWords kKindWords[] = {
    { "co2", "cryo", "", "(?<![a-z])co2|co2(?=[a-z])" },
    { "flame", "flame|flamme|fiamm|fire(?!fly|ball| ?exit)|fuego|feuer|pyro", "llamas?|fogo|chamas?", "" },
    { "spark", "spark(?!l)|chispa|etincel|scintill|funken|faisc", "", "" },
    { "confetti", "confet|konfet|streamer|serpentin|coriandol", "", "" },
    { "bubbles", "bubbl|burbuj|bolha|seifenblase", "bolle|bulles?", "" },
    { "snow", "snow(?!ball)|schnee", "nieve|neige|neve", "" },
    { "blinder", "blinder|molefay|cegador", "", "" },
    { "strobe", "strob|estrob", "", "" },
    { "laser", "laser", "", "" },
    { "hazer", "haze", "", "" },
    { "smoke", "smok|fog|niebla|nebel|fumee|fumaca|nebbia", "humo|fumo", "" },
    { "fan", "ventil", "fans?", "" },
    { "scanner", "scan|escan", "", "" },
    { "movinghead", "moving ?heads?|cabeza|testa mobile", "lyres?", "" },
    { "ledbar", "batten|strip|pixel ?bar|leiste", "bars?|barras?|tiras?", "" },
    { "wash", "wash|cyclorama|ciclorama", "pars?|cyc", "" },
    { "dimmer", "dimmer|fresnel|convencional|conventional", "", "" },
    { "effect", "effect|efect|effet|effekt|flower|derby|mirror ?ball|disco ?ball|bola de espejo", "", "" },
};

/* The special effects: what words say of a dimmer, a smoke machine... */
const QStringList kSpecials = { "co2", "flame", "spark", "confetti", "bubbles", "snow" };

/* The kinds the words of a definition may say instead of the kind of its
 * type: a moving head with "Wash" in its name is still a moving head; a
 * "Dimmer" or a "Color Changer" called "Blinder" is a blinder; an "Effect"
 * or an "Other" is what its words say */
QStringList overridable(const QString &typeKind)
{
    if (typeKind == QLatin1String("wash"))
        return { "blinder", "strobe" };
    if (typeKind == QLatin1String("strobe"))
        return { "blinder" };
    if (typeKind == QLatin1String("dimmer"))
        return kSpecials + QStringList({ "blinder", "strobe", "laser", "hazer", "smoke", "fan", "effect" });
    if (typeKind == QLatin1String("smoke"))
        return kSpecials + QStringList({ "hazer" });
    if (typeKind == QLatin1String("hazer") || typeKind == QLatin1String("fan"))
        return kSpecials;
    if (typeKind == QLatin1String("effect") || typeKind == QLatin1String("other"))
        return PatchManager::fixtureKinds();
    // moving heads, scanners, lasers, LED bars: their type says it
    return QStringList();
}

/* $text as the words are looked for in it: the words of a name written
 * together apart ("BigFlame"), in lower case, without accents */
QString wordsText(const QString &text)
{
    static const QRegularExpression camel(QStringLiteral("([a-z])([A-Z])"));
    QString split = text;
    split.replace(camel, QStringLiteral("\\1 \\2"));
    const QString decomposed = split.normalized(QString::NormalizationForm_KD);
    QString out;
    out.reserve(decomposed.size());
    for (const QChar c : decomposed)
    {
        if (c.category() != QChar::Mark_NonSpacing)
            out.append(c.toLower());
    }
    return out;
}

/* The expression of the words of each kind of kKindWords, in its order */
const QList<QPair<QString, QRegularExpression>> &kindExpressions()
{
    static const QList<QPair<QString, QRegularExpression>> expressions = []() {
        QList<QPair<QString, QRegularExpression>> list;
        for (const KindWords &words : kKindWords)
        {
            QStringList parts;
            if (*words.starts)
                parts.append(QStringLiteral("(?<![a-z0-9])(?:%1)").arg(QLatin1String(words.starts)));
            if (*words.wholes)
                parts.append(QStringLiteral("(?<![a-z0-9])(?:%1)(?![a-z])").arg(QLatin1String(words.wholes)));
            if (*words.anywhere)
                parts.append(QStringLiteral("(?:%1)").arg(QLatin1String(words.anywhere)));
            list.append(qMakePair(QString::fromLatin1(words.kind), QRegularExpression(parts.join(QLatin1Char('|')))));
        }
        return list;
    }();
    return expressions;
}

/* The first kind of $allowed (every kind: all of them) the words of $text say */
QString wordsKind(const QString &text, const QStringList *allowed = nullptr)
{
    if (text.trimmed().isEmpty())
        return QString();
    const QString words = wordsText(text);
    for (const auto &kind : kindExpressions())
    {
        if (allowed != nullptr && allowed->contains(kind.first) == false)
            continue;
        if (kind.second.match(words).hasMatch())
            return kind.first;
    }
    return QString();
}

} // namespace

QStringList PatchManager::fixtureKinds()
{
    QStringList kinds;
    for (const char *kind : kKinds)
        kinds.append(QString::fromLatin1(kind));
    return kinds;
}

QString PatchManager::kindOfType(const QString &type)
{
    if (type == QLatin1String("Moving Head"))
        return QStringLiteral("movinghead");
    if (type == QLatin1String("Scanner"))
        return QStringLiteral("scanner");
    if (type == QLatin1String("Laser"))
        return QStringLiteral("laser");
    if (type == QLatin1String("LED Bar (Beams)") || type == QLatin1String("LED Bar (Pixels)"))
        return QStringLiteral("ledbar");
    if (type == QLatin1String("Color Changer"))
        return QStringLiteral("wash");
    if (type == QLatin1String("Dimmer"))
        return QStringLiteral("dimmer");
    if (type == QLatin1String("Strobe"))
        return QStringLiteral("strobe");
    if (type == QLatin1String("Smoke"))
        return QStringLiteral("smoke");
    if (type == QLatin1String("Hazer"))
        return QStringLiteral("hazer");
    if (type == QLatin1String("Fan"))
        return QStringLiteral("fan");
    if (type == QLatin1String("Effect") || type == QLatin1String("Flower"))
        return QStringLiteral("effect");
    return QStringLiteral("other");
}

QString PatchManager::kindFromWords(const QString &text)
{
    return wordsKind(text);
}

QString PatchManager::fixtureKind(const QString &manufacturer, const QString &model, const QString &type,
                                  const QString &name, const QString &icon)
{
    // chosen by hand
    if (icon.isEmpty() == false && fixtureKinds().contains(icon))
        return icon;

    const QString typeKind = kindOfType(type);
    const QStringList allowed = overridable(typeKind);
    // a Generic definition: the fixture's own name says what it is (a
    // generic dimmer stands in for anything the library doesn't have)
    if (manufacturer.compare(KXMLFixtureGeneric, Qt::CaseInsensitive) == 0)
    {
        const bool dimmer = model.compare(KXMLFixtureGeneric, Qt::CaseInsensitive) == 0;
        const QString named = wordsKind(name, dimmer ? nullptr : &allowed);
        if (named.isEmpty() == false)
            return named;
    }
    // the words of the definition, where they say more than its type
    const QString said = wordsKind(manufacturer + QLatin1Char(' ') + model, &allowed);
    return said.isEmpty() ? typeKind : said;
}

QString PatchManager::kindOf(const Fixture *fxi) const
{
    const QLCFixtureDef *def = fxi->fixtureDef();
    return fixtureKind(def != nullptr ? def->manufacturer() : QString(KXMLFixtureGeneric),
                       def != nullptr ? def->model() : QString(KXMLFixtureGeneric),
                       fxi->typeString(), fxi->name(), m_icons.value(fxi->id()));
}

QList<PatchManager::TypeEntry> PatchManager::typeEntries() const
{
    QList<Fixture *> all = m_doc->fixtures();
    std::sort(all.begin(), all.end(), [](const Fixture *a, const Fixture *b) { return a->id() < b->id(); });

    // in the order of their lowest ID: the same show always looks the same.
    // A fixture of the library: one type (one colour) per definition, an
    // icon chosen by hand on some of them changes their glyph, not their
    // type. Generic dimmers: one type per thing they stand in for
    QList<TypeEntry> entries;
    QHash<QString, int> index;
    for (const Fixture *fxi : std::as_const(all))
    {
        const QLCFixtureDef *def = fxi->fixtureDef();
        // generic dimmers are made on the fly, as "Generic - Generic"
        const bool generic = def == nullptr || (def->manufacturer() == KXMLFixtureGeneric &&
                                                def->model() == KXMLFixtureGeneric);
        const QString manufacturer = generic ? QString() : def->manufacturer();
        const QString model = generic ? QString() : def->model();
        const QString kind = generic ? kindOf(fxi) : QString();
        QString key = manufacturer + QLatin1Char('\n') + model;
        if (generic)
            key += QLatin1Char('\n') + kind;
        int i = index.value(key, -1);
        if (i < 0)
        {
            TypeEntry entry;
            entry.key = key;
            entry.manufacturer = manufacturer;
            entry.model = model;
            // (a definition: its own kind, that of its first fixture without
            // the icon chosen by hand)
            entry.kind = generic ? kind : fixtureKind(manufacturer, model, fxi->typeString(), fxi->name(), QString());
            i = int(entries.count());
            entries.append(entry);
            index.insert(key, i);
        }
        entries[i].ids.append(fxi->id());
    }
    return entries;
}

QVariantList PatchManager::fixtureTypes() const
{
    QVariantList list;
    const QList<TypeEntry> entries = typeEntries();
    for (int i = 0; i < entries.count(); i++)
    {
        const TypeEntry &entry = entries.at(i);
        QVariantList ids;
        for (quint32 id : entry.ids)
            ids.append(id);
        QVariantMap type;
        type.insert("key", entry.key);
        type.insert("manufacturer", entry.manufacturer);
        type.insert("model", entry.model);
        type.insert("kind", entry.kind);
        type.insert("colorIndex", i);
        type.insert("count", int(entry.ids.count()));
        type.insert("ids", ids);
        list.append(type);
    }
    return list;
}

void PatchManager::setFixtureIcon(const QVariantList &ids, const QString &kind)
{
    const bool automatic = kind.isEmpty() || kind == QLatin1String("auto");
    if (automatic == false && fixtureKinds().contains(kind) == false)
        return;
    bool changed = false;
    for (const QVariant &v : ids)
    {
        const quint32 id = v.toUInt();
        if (m_doc->fixture(id) == nullptr)
            continue;
        if (automatic)
            changed = m_icons.remove(id) > 0 || changed;
        else if (m_icons.value(id) != kind)
        {
            m_icons.insert(id, kind);
            changed = true;
        }
    }
    if (changed == false)
        return;
    m_doc->setModified();
    notifyFixtures();
}

QString PatchManager::fixtureIcon(quint32 id) const
{
    return m_icons.value(id);
}
