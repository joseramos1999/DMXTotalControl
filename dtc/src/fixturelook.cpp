/*
  DMX Total Control
  fixturelook.cpp

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

#include "fixturelook.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "fxscene.h"
#include "fixture.h"

int FixtureLook::capabilityValue(const QLCChannel *ch, const QStringList &words)
{
    if (ch == nullptr)
        return -1;
    // the first word has precedence over the others
    for (const QString &word : words)
    {
        for (QLCCapability *cap : ch->capabilities())
        {
            if (cap->name().contains(word, Qt::CaseInsensitive))
                return (cap->min() + cap->max()) / 2;
        }
    }
    return -1;
}

/* The middle of the range $ch's definition marks open (ShutterOpen), -1
 * when none does. Of two marked so, one not named off or closed (a slip of
 * a definition: «LED off» and «LED on» both marked open) */
static int presetOpenValue(const QLCChannel *ch)
{
    const QLCCapability *found = nullptr;
    bool foundDark = false;
    for (const QLCCapability *cap : ch->capabilities())
    {
        if (cap->preset() != QLCCapability::ShutterOpen)
            continue;
        const QString name = cap->name().toLower();
        const bool dark = name.contains(QLatin1String("off")) || name.contains(QLatin1String("close")) ||
                          name.contains(QLatin1String("blackout"));
        if (found == nullptr || (foundDark && dark == false))
        {
            found = cap;
            foundDark = dark;
        }
    }
    return found != nullptr ? (found->min() + found->max()) / 2 : -1;
}

/* As FixtureLook::capabilityValue, among the ranges $ch's definition marks
 * open (ShutterOpen) only */
static int markedOpenValue(const QLCChannel *ch, const QStringList &words)
{
    for (const QString &word : words)
    {
        for (const QLCCapability *cap : ch->capabilities())
        {
            if (cap->preset() == QLCCapability::ShutterOpen && cap->name().contains(word, Qt::CaseInsensitive))
                return (cap->min() + cap->max()) / 2;
        }
    }
    return -1;
}

QMap<quint32, uchar> FixtureLook::openWhite(const Fixture *fxi)
{
    QMap<quint32, uchar> values;
    if (fxi == nullptr)
        return values;

    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        if (ch == nullptr)
            continue;
        const bool msb = ch->controlByte() == QLCChannel::MSB;
        switch (ch->group())
        {
            case QLCChannel::Intensity:
                switch (ch->colour())
                {
                    case QLCChannel::NoColour:
                        values.insert(c, msb ? 255 : 0);
                        break;
                    // white: the emitters that make it at full, the others off
                    case QLCChannel::Red: case QLCChannel::Green: case QLCChannel::Blue: case QLCChannel::White:
                        values.insert(c, msb ? 255 : 0);
                        break;
                    default:
                        // CMY open, amber, UV, lime... off
                        values.insert(c, 0);
                        break;
                }
                break;
            case QLCChannel::Shutter:
            {
                if (FXScene::attributeOf(ch) == QLatin1String("iris"))
                    break;
                int value = capabilityValue(ch, QStringList() << "open" << "abierto");
                if (value >= 0 && msb)
                    values.insert(c, uchar(value));
                break;
            }
            case QLCChannel::Colour:
            {
                // the open or white slot of a colour wheel
                int value = capabilityValue(ch, QStringList() << "open" << "white" << "blanco" << "abierto");
                if (value >= 0 && msb)
                    values.insert(c, uchar(value));
                break;
            }
            default:
                break;
        }
    }
    return values;
}

QMap<quint32, uchar> FixtureLook::shutterOpen(const Fixture *fxi)
{
    QMap<quint32, uchar> values;
    if (fxi == nullptr)
        return values;
    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        if (ch == nullptr || ch->group() != QLCChannel::Shutter || ch->controlByte() != QLCChannel::MSB ||
            FXScene::attributeOf(ch) == QLatin1String("iris"))
            continue;
        // the range its definition marks open, when it has one («LED ON»: its
        // rest may be «LED OFF»): of several, the one named open or no
        // strobe, else one not named off. The names alone pick strobe effects
        // there («Slow open - fast close», «Opening pulse», with a preset or
        // not) or the dark «No function» of many PARs. Without one: open; a
        // strobe channel without it: no strobe; a "Strobe slow to fast" all
        // along, as a PAR has it: 0 is no strobe. A "fast to slow" one
        // without any of them is left as it is
        const QStringList open = QStringList() << "open" << "abierto";
        const QStringList noStrobe = QStringList() << "no strobe" << "strobe off" << "no function" << "sin estrobo";
        int value = presetOpenValue(ch);
        if (value >= 0)
        {
            int named = markedOpenValue(ch, open);
            if (named < 0)
                named = markedOpenValue(ch, noStrobe);
            if (named >= 0)
                value = named;
        }
        else
        {
            value = capabilityValue(ch, open);
            if (value < 0)
                value = capabilityValue(ch, noStrobe);
            if (value < 0 && ch->preset() == QLCChannel::ShutterStrobeSlowFast)
                value = 0;
        }
        if (value >= 0)
            values.insert(c, uchar(value));
    }
    return values;
}

QMap<quint32, uchar> FixtureLook::showStopLight(const Fixture *fxi, uchar level)
{
    QMap<quint32, uchar> values;
    if (fxi == nullptr)
        return values;

    // the dimmers a dimmer level sets (not the flash of a strobe with an
    // aura, the fog of a fogger with LEDs, a lamp control): at the level
    const QList<quint32> dimmers = FXScene::levelDimmerChannels(fxi);
    for (quint32 c : dimmers)
        values.insert(c, level);
    const bool hasDimmer = dimmers.isEmpty() == false;

    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        if (ch == nullptr || dimmers.contains(c))
            continue;
        const bool msb = ch->controlByte() == QLCChannel::MSB;
        switch (ch->group())
        {
            case QLCChannel::Intensity:
                switch (ch->colour())
                {
                    // white: the emitters that make it at full (their dimmer
                    // gives the level), at the level without a dimmer
                    case QLCChannel::Red: case QLCChannel::Green: case QLCChannel::Blue: case QLCChannel::White:
                        values.insert(c, msb ? (hasDimmer ? 255 : level) : 0);
                        break;
                    // the other intensities without a colour (a flash, a
                    // fine byte), CMY open, amber, UV, lime... off
                    default:
                        values.insert(c, 0);
                        break;
                }
                break;
            case QLCChannel::Colour:
            {
                // the open or white slot of a colour wheel
                const int value = capabilityValue(ch, QStringList() << "open" << "white" << "blanco" << "abierto");
                if (value >= 0 && msb)
                    values.insert(c, uchar(value));
                break;
            }
            case QLCChannel::Effect:
            {
                // no program of its own (a colour jump, sound active)
                const int value = capabilityValue(ch, QStringList() << "no function" << "no effect" << "none" << "off");
                if (value >= 0 && msb)
                    values.insert(c, uchar(value));
                break;
            }
            default:
                break;
        }
    }
    const QMap<quint32, uchar> open = shutterOpen(fxi);
    for (auto it = open.cbegin(); it != open.cend(); ++it)
        values.insert(it.key(), it.value());
    return values;
}

QList<quint32> FixtureLook::motionChannels(const Fixture *fxi)
{
    QList<quint32> channels;
    if (fxi == nullptr)
        return channels;
    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        if (ch != nullptr && (ch->group() == QLCChannel::Pan || ch->group() == QLCChannel::Tilt))
            channels.append(c);
    }
    return channels;
}

QList<quint32> FixtureLook::intensityChannels(const Fixture *fxi)
{
    QList<quint32> channels;
    if (fxi == nullptr)
        return channels;
    for (quint32 c = 0; c < fxi->channels(); c++)
    {
        const QLCChannel *ch = fxi->channel(c);
        if (ch != nullptr && ch->group() == QLCChannel::Intensity)
            channels.append(c);
    }
    return channels;
}
