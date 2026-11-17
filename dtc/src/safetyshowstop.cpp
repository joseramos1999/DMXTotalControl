/*
  DMX Total Control
  safetyshowstop.cpp

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

#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDateTime>
#include <QTimer>
#include <QSet>

#include "safetymanager.h"
#include "patchmanager.h"
#include "qlcfixturedef.h"
#include "fixturegroup.h"
#include "fixturelook.h"
#include "livemixer.h"
#include "outputlayer.h"
#include "universe.h"
#include "fixture.h"
#include "doc.h"

/*********************************************************************
 * The show stop: a safety light at the output, above every source, the
 * grand master, Blackout and the parks (only the gate is above it:
 * OutputLayer::forced). Held a second to put it, held again to take it away
 *********************************************************************/

void SafetyManager::setMixer(LiveMixer *mixer)
{
    m_mixer = mixer;
    if (m_mixer != nullptr)
        m_mixer->setStrobeSuppressed(m_showStop);
}

void SafetyManager::holdShowStop(bool down)
{
    if (down)
    {
        // (the program closes: nothing more)
        if (m_closing)
            return;
        // the first control down: the time runs (the others held with it
        // count as the same hold)
        if (m_holdCount++ == 0)
        {
            m_holdTimer->start();
            emit holdingChanged();
        }
        return;
    }
    // an up of nothing held: a note off lost before, the second control of
    // a hold that toggled already
    if (m_holdCount == 0)
        return;
    if (--m_holdCount == 0)
    {
        // let go too soon: nothing
        m_holdTimer->stop();
        emit holdingChanged();
    }
}

bool SafetyManager::keyPending() const
{
    return m_keyTimer->isActive();
}

void SafetyManager::pressShowStopKey()
{
    // (the program closes: nothing more)
    if (m_closing)
        return;
    // the second press within the time: deliberate, at once
    if (m_keyTimer->isActive())
    {
        m_keyTimer->stop();
        emit keyPendingChanged();
        setShowStop(m_showStop == false);
        return;
    }
    // the first one: nothing yet (a key hit by chance never does it)
    m_keyTimer->start();
    emit keyPendingChanged();
}

void SafetyManager::slotHoldDone()
{
    // the ups that follow are of this hold: they do nothing (one lost
    // never blocks the next hold)
    m_holdCount = 0;
    setShowStop(m_showStop == false);
    emit holdingChanged();
}

void SafetyManager::setShowStop(bool on)
{
    if (on == m_showStop || (on && m_closing))
        return;
    m_showStop = on;
    // (put or taken away by another control: a press of Pause waited for
    // would take back what was just done)
    if (m_keyTimer->isActive())
    {
        m_keyTimer->stop();
        emit keyPendingChanged();
    }
    // no strobe of the live mixer while it lasts (the scenes': their
    // shutters open, the strobes dark)
    if (m_mixer != nullptr)
        m_mixer->setStrobeSuppressed(on);
    writeLog(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz")) +
             QStringLiteral("\t\t\t\t") + (on ? tr("Show stop on") : tr("Show stop off")));
    // the special effects disarmed (its layer already has the show stop),
    // and they stay disarmed: arm() refuses while it lasts
    if (on && m_armed)
        disarm(QStringLiteral("showStop"));
    else
        push();
    emit showStopChanged();
}

void SafetyManager::setShowStopLevel(int level)
{
    level = qBound(0, level, 100);
    if (level == m_showStopLevel)
        return;
    m_showStopLevel = level;
    m_doc->setModified();
    emit showStopSettingsChanged();
    if (m_showStop)
        push();
}

QVariantList SafetyManager::showStopGroups() const
{
    QVariantList list;
    for (quint32 id : m_showStopGroups)
        list.append(id);
    return list;
}

void SafetyManager::setShowStopGroups(const QVariantList &groups)
{
    // (only groups of the show, each once)
    QList<quint32> ids;
    for (const QVariant &v : groups)
    {
        bool ok = false;
        const quint32 id = v.toUInt(&ok);
        if (ok && m_doc->fixtureGroup(id) != nullptr && ids.contains(id) == false)
            ids.append(id);
    }
    if (ids == m_showStopGroups)
        return;
    m_showStopGroups = ids;
    m_doc->setModified();
    emit showStopSettingsChanged();
    if (m_showStop)
        push();
}

void SafetyManager::slotGroupRemoved(quint32 id)
{
    // (a show that goes: cleared all at once)
    if (m_doc->isClearingOrLoading() || m_showStopGroups.removeAll(id) == 0)
        return;
    emit showStopSettingsChanged();
    if (m_showStop)
        push();
}

void SafetyManager::slotGroupChanged(quint32 id)
{
    // the fixtures of a group it lights changed: its light follows
    if (m_showStop && m_doc->isClearingOrLoading() == false && m_showStopGroups.contains(id))
        push();
}

void SafetyManager::clearShowStop()
{
    m_holdTimer->stop();
    if (m_keyTimer->isActive())
    {
        m_keyTimer->stop();
        emit keyPendingChanged();
    }
    const bool held = m_holdCount > 0;
    const bool was = m_showStop;
    const bool settings = showStopSaved();
    m_holdCount = 0;
    m_showStop = false;
    m_showStopLevel = 100;
    m_showStopGroups.clear();
    if (was && m_mixer != nullptr)
        m_mixer->setStrobeSuppressed(false);
    if (held)
        emit holdingChanged();
    if (was)
        emit showStopChanged();
    if (settings)
        emit showStopSettingsChanged();
}

bool SafetyManager::showStopLights(const QString &kind)
{
    // the lights; never smoke, hazers, fans, bubbles, snow, the other
    // effects nor a special effect (a dimmer named HAZER is a hazer)
    static const QStringList lights = { "movinghead", "scanner", "wash", "ledbar", "blinder", "dimmer" };
    return lights.contains(kind);
}

bool SafetyManager::genericDimmerLight(const QString &name, const QString &icon)
{
    // the icon chosen for it in the plan: what it is, as the user says
    if (icon.isEmpty() == false && PatchManager::fixtureKinds().contains(icon))
        return showStopLights(icon);
    // its words as the kinds take them: in lower case, without accents
    // ("Relé" is "rele"), the ones written together apart ("FrontVideo" is
    // "front video": a video trigger, never lit)
    const QString words = PatchManager::nameWords(name);
    // a word of something else, whatever more it says: a video trigger, a
    // relay, a motor, a curtain, a screen, pyro...
    static const QRegularExpression other(QStringLiteral(
        "(?<![a-z0-9])(?:video|vid(?![a-z])|trigger|disparador|declench|auslos|relay|rele|relais|motor|moteur|"
        "pantalla|screen|ecran|schermo|leinwand|beamer|projector|proyector|media|kabuki|curtain|telon|rideau|"
        "vorhang|sipario|piro|pyro)"));
    if (other.match(words).hasMatch())
        return false;
    // a light its words say (PAR, FRESNEL, DIMMER, BLINDER, BATTEN, CYC…)
    if (showStopLights(PatchManager::kindFromWords(name)))
        return true;
    // or the names of the lights of a stage, of a hall
    static const QRegularExpression light(QStringLiteral(
        "(?<![a-z0-9])(?:light|lamp|spot|front|contra|contre|cenital|lateral|foco|luz|luces|luce|luci|licht|"
        "lumiere|projecteur|proiettore|faro|fari|refletor|scheinwerfer|recorte|decoupe|sagomatore|profil|perfil|"
        "halogen|tungst|sala|salle|saal|house|platea|publico|audience)|(?<![a-z0-9])pc(?![a-z])"));
    return light.match(words).hasMatch();
}

QMap<quint32, SafetyManager::StopValue> SafetyManager::showStopValues(Doc *doc, PatchManager *patch, int level,
                                                                      const QList<quint32> &groups)
{
    QMap<quint32, StopValue> values;
    // the fixtures of its groups (none of them in the show: every light)
    QSet<quint32> chosen;
    bool every = true;
    for (quint32 id : groups)
    {
        if (FixtureGroup *group = doc->fixtureGroup(id))
        {
            every = false;
            for (quint32 fid : group->fixtureList())
                chosen.insert(fid);
        }
    }
    const uchar dmxLevel = uchar(qRound(qBound(0, level, 100) * 255 / 100.0));

    for (const Fixture *fxi : doc->fixtures())
    {
        // a protected special effect: the gate gives it 0, always
        if (patch->isProtected(fxi->id()))
            continue;
        auto put = [&values, fxi](quint32 channel, uchar value, bool hold) {
            values.insert(fxi->universeAddress() + channel, StopValue { value, hold });
        };
        const QString kind = patch->kindOf(fxi);

        // every head stays where it is, every shutter opens (no strobe)
        for (quint32 c : FixtureLook::motionChannels(fxi))
            put(c, 0, true);
        const QMap<quint32, uchar> open = FixtureLook::shutterOpen(fxi);
        for (auto it = open.cbegin(); it != open.cend(); ++it)
            put(it.key(), it.value(), false);

        // strobes and lasers dark: a strobe's intensities (all of it when it
        // has none), all of a laser (at 0 a DMX laser is off)
        if (kind == QLatin1String("strobe") || kind == QLatin1String("laser"))
        {
            QList<quint32> dark = kind == QLatin1String("strobe") ? FixtureLook::intensityChannels(fxi) : QList<quint32>();
            if (dark.isEmpty())
            {
                for (quint32 c = 0; c < fxi->channels(); c++)
                    dark.append(c);
            }
            for (quint32 c : dark)
                put(c, 0, false);
            continue;
        }

        // the lights chosen, at open white. A generic dimmer stands in for
        // anything (a CO2 jet, a video trigger, a relay, a motor): only when
        // its icon or its name says it is a light (also in a group chosen:
        // a group of everything has the video triggers too)
        const QLCFixtureDef *def = fxi->fixtureDef();
        const bool genericDimmer = def == nullptr || (def->manufacturer() == KXMLFixtureGeneric &&
                                                      def->model() == KXMLFixtureGeneric);
        if (showStopLights(kind) && (every || chosen.contains(fxi->id())) &&
            (genericDimmer == false || genericDimmerLight(fxi->name(), patch->fixtureIcon(fxi->id()))))
        {
            const QMap<quint32, uchar> light = FixtureLook::showStopLight(fxi, dmxLevel);
            for (auto it = light.cbegin(); it != light.cend(); ++it)
                put(it.key(), it.value(), false);
        }
    }
    return values;
}

/*********************************************************************
 * The show file
 *********************************************************************/

bool SafetyManager::showStopSaved() const
{
    return m_showStopLevel != 100 || m_showStopGroups.isEmpty() == false;
}

void SafetyManager::saveShowStopXML(QXmlStreamWriter *doc) const
{
    // (never on: a show opens without it)
    if (showStopSaved() == false)
        return;
    doc->writeStartElement(KXMLDTCSafetyShowStop);
    doc->writeAttribute("Level", QString::number(m_showStopLevel));
    if (m_showStopGroups.isEmpty() == false)
    {
        QStringList ids;
        for (quint32 id : m_showStopGroups)
            ids.append(QString::number(id));
        doc->writeAttribute("Groups", ids.join(QLatin1Char(',')));
    }
    doc->writeEndElement();
}

void SafetyManager::loadShowStopXML(const QXmlStreamAttributes &attrs)
{
    // a bad level (a file edited by hand): full, the light of a show stop
    bool ok = false;
    const int level = attrs.value("Level").toInt(&ok);
    m_showStopLevel = ok && level >= 0 && level <= 100 ? level : 100;
    m_showStopGroups.clear();
    for (const QString &text : attrs.value("Groups").toString().split(QLatin1Char(','), Qt::SkipEmptyParts))
    {
        const quint32 id = text.trimmed().toUInt(&ok);
        if (ok && m_showStopGroups.contains(id) == false)
            m_showStopGroups.append(id);
    }
    emit showStopSettingsChanged();
}

void SafetyManager::checkShowStopGroups()
{
    const qsizetype before = m_showStopGroups.size();
    m_showStopGroups.removeIf([this](quint32 id) { return m_doc->fixtureGroup(id) == nullptr; });
    if (m_showStopGroups.size() != before)
        emit showStopSettingsChanged();
}
