/*
  DMX Total Control
  patcheffects.cpp

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

/* PatchManager: the protected special effects. CO2 jets, flame machines,
 * spark machines and confetti cannons only go out while the effects are
 * armed (SafetyManager builds the gate of the output from what is decided
 * here), each fire with its maximum time and the minimum wait before the
 * next one, and optionally a safety channel the machine needs in a window
 * of values to fire.
 *
 * Protection is a state kept for each fixture, saved in the plan of the
 * show (QLC+ never reads it). The kind of a fixture (its type, the words of
 * its definition, a generic dimmer's name, the icon chosen for it) decides
 * it when nothing else did, and a fixture whose kind changes into one of
 * the protected kinds is protected: the automatic rules only ever protect.
 * Renaming «CO2 1» to «CO2 9» keeps it unprotected if the user said so */

#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>
#include <QHash>
#include <algorithm>

#include "patchmanager.h"
#include "qlcfixturemode.h"
#include "qlcchannel.h"
#include "universe.h"
#include "fixture.h"
#include "doc.h"

namespace
{

/* The hook of each Doc that runs while applyPatch holds the universes */
QHash<const Doc *, std::function<void()>> &patchHooks()
{
    static QHash<const Doc *, std::function<void()>> hooks;
    return hooks;
}

/* The fixtures of $ids that exist, each once */
QList<quint32> effectIds(Doc *doc, const QVariantList &ids)
{
    QList<quint32> list;
    for (const QVariant &v : ids)
    {
        const quint32 id = v.toUInt();
        if (doc->fixture(id) != nullptr && list.contains(id) == false)
            list.append(id);
    }
    return list;
}

/* A number of a show file: $ok false when it is not one */
int attributeNumber(const QXmlStreamAttributes &attrs, const char *name, bool *ok)
{
    *ok = false;
    if (attrs.hasAttribute(QLatin1String(name)) == false)
        return 0;
    return attrs.value(QLatin1String(name)).toString().trimmed().toInt(ok);
}

} // namespace

void PatchManager::setPatchHook(Doc *doc, const std::function<void()> &hook)
{
    if (hook)
        patchHooks().insert(doc, hook);
    else
        patchHooks().remove(doc);
}

void PatchManager::runPatchHook(Doc *doc)
{
    const auto it = patchHooks().constFind(doc);
    if (it != patchHooks().constEnd() && it.value())
        it.value()();
}

bool PatchManager::protectedByDefault(const QString &kind)
{
    return kind == QLatin1String("co2") || kind == QLatin1String("flame") ||
           kind == QLatin1String("spark") || kind == QLatin1String("confetti");
}

int PatchManager::defaultFireMs(const QString &kind)
{
    if (kind == QLatin1String("flame") || kind == QLatin1String("confetti"))
        return 2000;
    if (kind == QLatin1String("spark"))
        return 5000;
    // CO2, and anything protected by hand
    return 3000;
}

bool PatchManager::crossesUniverses(const Fixture *fxi)
{
    // (a fixture of a show from QLC+ that goes on in the next universe; the
    // flag alone is enough: in doubt it never fires)
    return fxi != nullptr && (fxi->crossUniverse() || fxi->address() + fxi->channels() > UNIVERSE_SIZE);
}

bool PatchManager::deferProtection() const
{
    return m_loading || m_doc->isClearingOrLoading();
}

bool PatchManager::decideProtection(quint32 id)
{
    if (m_effects.contains(id))
        return false;
    const Fixture *fxi = m_doc->fixture(id);
    if (fxi == nullptr)
        return false;
    const QString kind = kindOf(fxi);
    m_effectKinds.insert(id, kind);
    EffectSetup setup;
    setup.protect = protectedByDefault(kind);
    m_effects.insert(id, setup);
    return setup.protect;
}

void PatchManager::settleProtection()
{
    // (what is decided stays; the kinds are known now, icons included)
    QList<Fixture *> fixtures = m_doc->fixtures();
    std::sort(fixtures.begin(), fixtures.end(), [this](const Fixture *a, const Fixture *b) {
        return m_numbers.value(a->id()) < m_numbers.value(b->id());
    });
    QStringList protectedNow;
    for (Fixture *fxi : std::as_const(fixtures))
    {
        if (m_effects.contains(fxi->id()))
            m_effectKinds.insert(fxi->id(), kindOf(fxi));
        else if (decideProtection(fxi->id()))
            protectedNow.append(fxi->name());
    }
    // protected by their kind, the show said nothing of them (an older
    // show, one from QLC+): said once, they go dark until armed (a fog
    // machine whose name says CO2, a lamp called «Fire»...). Once the show
    // is open (after what opening it says, and the window there)
    if (protectedNow.isEmpty() == false)
    {
        QStringList names = protectedNow.mid(0, 6);
        if (protectedNow.count() > names.count())
            names.append(QStringLiteral("…"));
        const QString text = tr("%n fixture(s) of this show are protected special effects now: %1. They only go "
                                "out armed (Patch › Special effect to change it)", "", int(protectedNow.count()))
                                 .arg(names.join(QStringLiteral(", ")));
        QMetaObject::invokeMethod(this, [this, text]() { emit protectionMessage(text); }, Qt::QueuedConnection);
    }
    // the safety channels whose channel isn't there any more (a definition
    // updated since the show was saved): none, said once too
    if (m_lostSafety.isEmpty() == false)
    {
        QStringList names = m_lostSafety.mid(0, 6);
        if (m_lostSafety.count() > names.count())
            names.append(QStringLiteral("…"));
        m_lostSafety.clear();
        const QString text = tr("The safety channel of %1 was taken away: its channels changed "
                                "(Patch › Special effect to choose it again)").arg(names.join(QStringLiteral(", ")));
        QMetaObject::invokeMethod(this, [this, text]() { emit protectionMessage(text); }, Qt::QueuedConnection);
    }
    // (what a damaged file named for fixtures that aren't there)
    for (auto it = m_effects.begin(); it != m_effects.end();)
    {
        if (m_doc->fixture(it.key()) == nullptr)
        {
            m_effectKinds.remove(it.key());
            it = m_effects.erase(it);
        }
        else
        {
            ++it;
        }
    }
    emit protectionChanged();
}

void PatchManager::slotDocLoaded()
{
    if (m_loading == false)
        settleProtection();
}

void PatchManager::recheckKinds(const QList<quint32> &ids, bool tell)
{
    // a show loading: settleProtection() decides once its icons are known
    if (deferProtection())
        return;
    bool changed = false;
    QStringList protectedNow;
    for (quint32 id : ids)
    {
        const Fixture *fxi = m_doc->fixture(id);
        if (fxi == nullptr)
            continue;
        if (m_effects.contains(id) == false)
        {
            changed = decideProtection(id) || changed;
            continue;
        }
        const QString kind = kindOf(fxi);
        const QString before = m_effectKinds.value(id);
        m_effectKinds.insert(id, kind);
        EffectSetup &setup = m_effects[id];
        if (kind != before && protectedByDefault(kind) && setup.protect == false)
        {
            // another kind, a protected one: protected, whatever was said
            // for what it was before («PAR 2» renamed «FLAME 9»)
            setup.protect = true;
            changed = true;
            protectedNow.append(fxi->name());
        }
    }
    if (changed)
        emit protectionChanged();
    // (one message for all of them)
    if (tell && protectedNow.count() == 1)
        emit protectionMessage(tr("%1 is now a protected special effect").arg(protectedNow.first()));
    else if (tell && protectedNow.count() > 1)
        emit protectionMessage(tr("%n fixture(s) are now protected special effects", "", int(protectedNow.count())));
}

bool PatchManager::safetyStillThere(const Fixture *fxi, const EffectSetup &setup)
{
    // (a fixture of one channel has none: nothing would be left to fire)
    if (fxi == nullptr || setup.safetyChannel < 0 || setup.safetyChannel >= int(fxi->channels()) ||
        fxi->channels() < 2)
        return false;
    const QLCChannel *ch = fxi->channel(quint32(setup.safetyChannel));
    return ch != nullptr && ch->name() == setup.safetyName;
}

void PatchManager::dropLostSafety(quint32 id)
{
    // (a show loading: what it says is checked as it is read)
    if (deferProtection())
        return;
    const Fixture *fxi = m_doc->fixture(id);
    const auto it = m_effects.find(id);
    if (fxi == nullptr || it == m_effects.end() || it->safetyChannel < 0 || safetyStillThere(fxi, it.value()))
        return;
    // none: the channel there now is one more of the fixture (0 unless it
    // fires), never driven to the window by the gate
    it->safetyChannel = -1;
    it->safetyName.clear();
    m_doc->setModified();
    emit protectionChanged();
    emit protectionMessage(tr("The safety channel of %1 was taken away: its channels changed "
                              "(Patch › Special effect to choose it again)").arg(fxi->name()));
}

bool PatchManager::isProtected(quint32 id) const
{
    const Fixture *fxi = m_doc->fixture(id);
    if (fxi == nullptr)
        return false;
    const auto it = m_effects.constFind(id);
    // nothing decided yet (a show loading): what its kind says
    return it != m_effects.constEnd() ? it->protect : protectedByDefault(kindOf(fxi));
}

bool PatchManager::hasProtected() const
{
    for (Fixture *fxi : m_doc->fixtures())
    {
        if (isProtected(fxi->id()))
            return true;
    }
    return false;
}

PatchManager::EffectRule PatchManager::effectRule(quint32 id) const
{
    EffectRule rule;
    const Fixture *fxi = m_doc->fixture(id);
    if (fxi == nullptr)
        return rule;
    const QString kind = kindOf(fxi);
    const auto it = m_effects.constFind(id);
    EffectSetup setup;
    if (it != m_effects.constEnd())
        setup = it.value();
    else
        setup.protect = protectedByDefault(kind);

    rule.protect = setup.protect;
    rule.maxFireMs = quint32(qBound(minFireMs(), setup.maxFireMs > 0 ? setup.maxFireMs : defaultFireMs(kind), maxFireMs()));
    rule.minWaitMs = quint32(qBound(0, setup.minWaitMs >= 0 ? setup.minWaitMs : defaultWaitMs(), maxWaitMs()));
    // a safety channel it doesn't have any more (another mode, an updated
    // definition: another channel at its place, maybe the fire), or a window
    // that can't be: none (the channel is one more of the fixture, 0
    // disarmed). (And a fixture of one channel has none: nothing would fire)
    if (safetyStillThere(fxi, setup) && fxi->channels() > 1 &&
        setup.safetyFrom >= 0 && setup.safetyTo <= 255 && setup.safetyFrom <= setup.safetyTo)
    {
        rule.safety = setup.safetyChannel;
        rule.from = uchar(setup.safetyFrom);
        rule.to = uchar(setup.safetyTo);
    }
    return rule;
}

QVariantMap PatchManager::effectSettings(const QVariantList &ids) const
{
    QVariantMap settings;
    QStringList mixed;
    QStringList firstChannels;
    bool sameMode = true;
    bool cross = false;
    int count = 0;
    for (quint32 id : effectIds(m_doc, ids))
    {
        const Fixture *fxi = m_doc->fixture(id);
        const QString kind = kindOf(fxi);
        const EffectRule rule = effectRule(id);
        const EffectSetup setup = m_effects.value(id);
        QStringList channels;
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            channels.append(ch != nullptr ? ch->name() : QString());
        }
        // as its kind says: protected or not as the kind, its times and no
        // safety channel
        const bool byKind = rule.protect == protectedByDefault(kind) &&
                            (setup.maxFireMs < 0 || setup.maxFireMs == defaultFireMs(kind)) &&
                            (setup.minWaitMs < 0 || setup.minWaitMs == defaultWaitMs()) && rule.safety < 0;
        const QVariantMap one = {
            { "protected", rule.protect },
            { "kind", kind },
            { "byKind", byKind },
            { "defaultFireMs", defaultFireMs(kind) },
            { "maxFireMs", int(rule.maxFireMs) },
            { "minWaitMs", int(rule.minWaitMs) },
            { "safetyChannel", rule.safety },
            { "safetyFrom", int(rule.from) },
            { "safetyTo", int(rule.to) },
        };
        if (count == 0)
        {
            settings = one;
            firstChannels = channels;
        }
        else
        {
            for (auto it = one.cbegin(); it != one.cend(); ++it)
            {
                if (settings.value(it.key()) != it.value() && mixed.contains(it.key()) == false)
                    mixed.append(it.key());
            }
            sameMode = sameMode && channels == firstChannels;
        }
        cross = cross || crossesUniverses(fxi);
        count++;
    }
    QVariantList channels;
    if (count > 0 && sameMode)
    {
        for (int c = 0; c < firstChannels.count(); c++)
            channels.append(QVariantMap({ { "index", c }, { "name", firstChannels.at(c) } }));
    }
    settings.insert("count", count);
    settings.insert("mixed", mixed);
    settings.insert("sameMode", count > 0 && sameMode);
    settings.insert("channels", channels);
    settings.insert("crossUniverse", cross);
    return settings;
}

bool PatchManager::setEffectSetting(const QVariantList &ids, const QString &key, const QVariant &value)
{
    m_lastError.clear();
    const QList<quint32> list = effectIds(m_doc, ids);
    if (list.isEmpty())
        return false;

    bool ok = true;
    int number = 0;
    if (key != QLatin1String("protected"))
    {
        if (value.typeId() == QMetaType::Bool)
            ok = false;
        else
            number = value.toInt(&ok);
        if (ok == false)
        {
            m_lastError = tr("%1 is not a valid value").arg(value.toString());
            return false;
        }
    }

    // each one as it would be: all of them or none
    QHash<quint32, EffectSetup> next;
    for (quint32 id : list)
    {
        const Fixture *fxi = m_doc->fixture(id);
        EffectSetup setup;
        if (m_effects.contains(id))
            setup = m_effects.value(id);
        else
            setup.protect = protectedByDefault(kindOf(fxi));

        if (key == QLatin1String("protected"))
        {
            setup.protect = value.toBool();
        }
        else if (key == QLatin1String("maxFireMs"))
        {
            setup.maxFireMs = qBound(minFireMs(), number, maxFireMs());
        }
        else if (key == QLatin1String("minWaitMs"))
        {
            setup.minWaitMs = qBound(0, number, maxWaitMs());
        }
        else if (key == QLatin1String("safetyChannel"))
        {
            if (number < -1 || number >= int(fxi->channels()))
            {
                m_lastError = tr("%1 has no channel %2").arg(fxi->name()).arg(number + 1);
                return false;
            }
            // its only channel: nothing would be left to fire
            if (number >= 0 && fxi->channels() < 2)
            {
                m_lastError = tr("%1 needs another channel to fire").arg(fxi->name());
                return false;
            }
            setup.safetyChannel = number;
            // (and which one it is: another mode may put another one there)
            const QLCChannel *ch = number >= 0 ? fxi->channel(quint32(number)) : nullptr;
            setup.safetyName = ch != nullptr ? ch->name() : QString();
        }
        else if (key == QLatin1String("safetyFrom") || key == QLatin1String("safetyTo"))
        {
            const int v = qBound(0, number, 255);
            const int from = key == QLatin1String("safetyFrom") ? v : setup.safetyFrom;
            const int to = key == QLatin1String("safetyTo") ? v : setup.safetyTo;
            if (from > to)
            {
                m_lastError = tr("From %1 to %2: the first value can't be above the second").arg(from).arg(to);
                return false;
            }
            setup.safetyFrom = from;
            setup.safetyTo = to;
        }
        else
        {
            return false;
        }
        next.insert(id, setup);
    }

    for (auto it = next.cbegin(); it != next.cend(); ++it)
    {
        m_effects.insert(it.key(), it.value());
        m_effectKinds.insert(it.key(), kindOf(m_doc->fixture(it.key())));
    }
    m_doc->setModified();
    emit protectionChanged();
    notifyFixtures();
    return true;
}

void PatchManager::resetEffectSettings(const QVariantList &ids)
{
    const QList<quint32> list = effectIds(m_doc, ids);
    if (list.isEmpty())
        return;
    for (quint32 id : list)
    {
        const QString kind = kindOf(m_doc->fixture(id));
        EffectSetup setup;
        setup.protect = protectedByDefault(kind);
        m_effects.insert(id, setup);
        m_effectKinds.insert(id, kind);
    }
    m_doc->setModified();
    emit protectionChanged();
    notifyFixtures();
}

bool PatchManager::hasEffectAttributes(quint32 id) const
{
    const Fixture *fxi = m_doc->fixture(id);
    if (fxi == nullptr)
        return false;
    const QString kind = kindOf(fxi);
    const EffectSetup setup = m_effects.value(id);
    return isProtected(id) || protectedByDefault(kind) ||
           (setup.maxFireMs > 0 && setup.maxFireMs != defaultFireMs(kind)) ||
           (setup.minWaitMs >= 0 && setup.minWaitMs != defaultWaitMs()) || setup.safetyChannel >= 0;
}

void PatchManager::writeEffectAttributes(QXmlStreamWriter *doc, quint32 id) const
{
    const Fixture *fxi = m_doc->fixture(id);
    if (fxi == nullptr)
        return;
    const QString kind = kindOf(fxi);
    const EffectSetup setup = m_effects.value(id);
    // protected: always said. Not protected: only when its kind would be
    // (unprotected by hand); a show that says nothing gets its kind's
    if (isProtected(id))
        doc->writeAttribute("Protected", "1");
    else if (protectedByDefault(kind))
        doc->writeAttribute("Protected", "0");
    // what isn't the default
    if (setup.maxFireMs > 0 && setup.maxFireMs != defaultFireMs(kind))
        doc->writeAttribute("MaxFire", QString::number(setup.maxFireMs));
    if (setup.minWaitMs >= 0 && setup.minWaitMs != defaultWaitMs())
        doc->writeAttribute("MinWait", QString::number(setup.minWaitMs));
    if (setup.safetyChannel >= 0)
    {
        doc->writeAttribute("Safety", QString::number(setup.safetyChannel));
        doc->writeAttribute("SafetyFrom", QString::number(setup.safetyFrom));
        doc->writeAttribute("SafetyTo", QString::number(setup.safetyTo));
        doc->writeAttribute("SafetyName", setup.safetyName);
    }
}

bool PatchManager::effectSetupFrom(const Fixture *fxi, const QString &kind, const QXmlStreamAttributes &attrs,
                                   EffectSetup *setup, bool *lostSafety) const
{
    if (lostSafety != nullptr)
        *lostSafety = false;
    const bool said = attrs.hasAttribute(QLatin1String("Protected"));
    bool ok = false;
    const int maxFire = attributeNumber(attrs, "MaxFire", &ok);
    const bool hasMaxFire = attrs.hasAttribute(QLatin1String("MaxFire"));
    const bool maxFireOk = ok;
    const int minWait = attributeNumber(attrs, "MinWait", &ok);
    const bool hasMinWait = attrs.hasAttribute(QLatin1String("MinWait"));
    const bool minWaitOk = ok;
    const bool hasSafety = attrs.hasAttribute(QLatin1String("Safety"));
    // nothing of it in the file: what its kind says (settleProtection)
    if (said == false && hasMaxFire == false && hasMinWait == false && hasSafety == false)
        return false;

    EffectSetup parsed;
    // fail closed: only "0" unprotects, any other value protects; not said:
    // what its kind says
    parsed.protect = said ? attrs.value(QLatin1String("Protected")).trimmed() != QLatin1String("0")
                          : protectedByDefault(kind);
    // a time that isn't a number: the default; out of range: the nearest
    if (hasMaxFire && maxFireOk)
        parsed.maxFireMs = qBound(minFireMs(), maxFire, maxFireMs());
    if (hasMinWait && minWaitOk)
        parsed.minWaitMs = qBound(0, minWait, maxWaitMs());
    // a safety channel it doesn't have, or a window that can't be: none
    // (the channel is one more of the fixture: 0 unless it fires)
    if (hasSafety)
    {
        bool channelOk = false, fromOk = true, toOk = true;
        const int channel = attributeNumber(attrs, "Safety", &channelOk);
        int from = 1, to = 255;
        if (attrs.hasAttribute(QLatin1String("SafetyFrom")))
            from = attributeNumber(attrs, "SafetyFrom", &fromOk);
        if (attrs.hasAttribute(QLatin1String("SafetyTo")))
            to = attributeNumber(attrs, "SafetyTo", &toOk);
        // (the channel it was chosen on: a show of before this was saved
        // says nothing of it, the one there now)
        const QLCChannel *ch = channelOk && channel >= 0 && channel < int(fxi->channels())
                                   ? fxi->channel(quint32(channel)) : nullptr;
        const QString name = attrs.hasAttribute(QLatin1String("SafetyName"))
                                 ? attrs.value(QLatin1String("SafetyName")).toString()
                                 : (ch != nullptr ? ch->name() : QString());
        if (channelOk && channel >= 0 && fromOk && toOk && from >= 0 && to <= 255 && from <= to)
        {
            parsed.safetyChannel = channel;
            parsed.safetyName = name;
            parsed.safetyFrom = from;
            parsed.safetyTo = to;
            // not there now (an updated definition: another channel at its
            // place, fewer channels, only one): none, and it is said
            if (safetyStillThere(fxi, parsed) == false)
            {
                parsed.safetyChannel = -1;
                parsed.safetyName.clear();
                if (lostSafety != nullptr)
                    *lostSafety = true;
            }
        }
    }
    *setup = parsed;
    return true;
}

void PatchManager::readEffectAttributes(quint32 id, const QXmlStreamAttributes &attrs)
{
    const Fixture *fxi = m_doc->fixture(id);
    if (fxi == nullptr)
        return;
    const QString kind = kindOf(fxi);
    EffectSetup setup;
    bool lost = false;
    if (effectSetupFrom(fxi, kind, attrs, &setup, &lost) == false)
        return;
    m_effects.insert(id, setup);
    m_effectKinds.insert(id, kind);
    // (said once the show is open: settleProtection)
    if (lost)
        m_lostSafety.append(fxi->name());
}

void PatchManager::importPlanSettings(const QHash<quint32, quint32> &copies,
                                      const QHash<quint32, QXmlStreamAttributes> &plan)
{
    const QStringList kinds = fixtureKinds();
    bool changed = false;
    for (auto it = copies.cbegin(); it != copies.cend(); ++it)
    {
        const quint32 id = it.value();
        const Fixture *fxi = m_doc->fixture(id);
        const auto found = plan.constFind(it.key());
        if (fxi == nullptr || found == plan.constEnd())
            continue;
        const QXmlStreamAttributes &attrs = found.value();

        // the icon chosen for it there (a CO2 jet's makes it one here too)
        const QString icon = attrs.value(QLatin1String("Icon")).toString();
        if (kinds.contains(icon) && m_icons.value(id) != icon)
        {
            m_icons.insert(id, icon);
            changed = true;
        }

        // protected there (by hand, by its icon), or its kind here is a
        // protected one: protected, with the times and the safety channel
        // it had there. Fail closed: what that show says never unprotects it
        const QString kind = kindOf(fxi);
        decideProtection(id);
        EffectSetup &setup = m_effects[id];
        const EffectSetup before = setup;
        EffectSetup said;
        const bool any = effectSetupFrom(fxi, kind, attrs, &said);
        setup.protect = setup.protect || protectedByDefault(kind) || (any && said.protect);
        if (any)
        {
            if (said.maxFireMs > 0)
                setup.maxFireMs = said.maxFireMs;
            if (said.minWaitMs >= 0)
                setup.minWaitMs = said.minWaitMs;
            if (said.safetyChannel >= 0)
            {
                setup.safetyChannel = said.safetyChannel;
                setup.safetyName = said.safetyName;
                setup.safetyFrom = said.safetyFrom;
                setup.safetyTo = said.safetyTo;
            }
        }
        m_effectKinds.insert(id, kind);
        changed = changed || setup.protect != before.protect || setup.maxFireMs != before.maxFireMs ||
                  setup.minWaitMs != before.minWaitMs || setup.safetyChannel != before.safetyChannel ||
                  setup.safetyFrom != before.safetyFrom || setup.safetyTo != before.safetyTo;
    }
    if (changed == false)
        return;
    m_doc->setModified();
    emit protectionChanged();
    notifyFixtures();
}
