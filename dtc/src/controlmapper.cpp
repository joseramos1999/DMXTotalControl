/*
  DMX Total Control
  controlmapper.cpp

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
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QGuiApplication>
#include <QKeySequence>
#include <QKeyEvent>
#include <QSet>
#include <qmath.h>
#include <functional>

#include "controlmapper.h"
#include "playbackmanager.h"
#include "iomanager.h"
#include "drivecontrol.h"
#include "zonemanager.h"
#include "livemixer.h"
#include "showsurface.h"
#include "dtccore.h"
#include "zone.h"

#include "inputoutputmap.h"
#include "functionparent.h"
#include "qlcioplugin.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "mastertimer.h"
#include "function.h"
#include "cuelist.h"
#include "fixturegroup.h"
#include "doc.h"

#define KXMLDTCMapping QStringLiteral("Mapping")

// the MIDI channel in the 4 top bits of an input channel (OMNI mode)
#define MIDI_OMNI 16
#define MONITOR_SIZE 200

static int midiValue(uchar value)
{
    return value == 255 ? 127 : value >> 1;
}

static uchar dmxFromMidi(int midi)
{
    midi = qBound(0, midi, 127);
    return midi == 127 ? 255 : uchar(midi << 1);
}

/** The playback targets that act on a slot (pbPage, slot) */
static bool isSlotTarget(const QString &target)
{
    static const QSet<QString> targets = {
        "playbackGo", "playbackBack", "playbackPauseBack", "playbackFlash", "playbackRelease",
        "playbackSelect", "playbackGoTo", "playbackFader" };
    return targets.contains(target);
}

/** The slot targets that a functionId (>= 0) drives instead, wherever the
 *  function is (the ...Function entry points of the playbacks) */
static bool isFunctionTarget(const QString &target)
{
    return isSlotTarget(target) && target != "playbackSelect" && target != "playbackFader";
}

/** The zoneId of the group targets: -1 is "all groups" (release, next,
 *  previous), a deleted group drives nothing (its ID may be given to a new
 *  group) */
#define ALL_GROUPS -1
#define DELETED_GROUP -2

/** The range of a fader mapping, 0..1. From above To is an inverted range,
 *  as with Invert (absolute faders, encoders and the feedback alike) */
static void rangeOf(const QVariantMap &m, qreal &min, qreal &max, bool &invert)
{
    min = qBound(0, m.value("min").toInt(), 100) / 100.0;
    max = qBound(0, m.value("max").toInt(), 100) / 100.0;
    invert = m.value("invert").toBool();
    if (min > max)
    {
        std::swap(min, max);
        invert = !invert;
    }
}

/** Keys that only make sense one way: a playback page button is on a page
 *  (pbPage -1, the current page, would do nothing) */
static void normalizeMapping(QVariantMap &m)
{
    if (m.value("target").toString() == "playbackPage" && m.value("pbPage", 0).toInt() < 0)
        m.insert("pbPage", 0);
}

ControlMapper::ControlMapper(DtcCore *core, QObject *parent)
    : QObject(parent)
    , m_core(core)
    , m_lastId(0)
    , m_learnMode(false)
    , m_learnSourceOf(-1)
    , m_activity(0)
    , m_feedbackTimer(new QTimer(this))
    , m_namesPending(false)
{
    connect(m_core->doc()->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar,QString)),
            this, SLOT(slotInputValueChanged(quint32,quint32,uchar,QString)));
    connect(m_core->doc()->inputOutputMap(), SIGNAL(universeAdded(quint32)), this, SIGNAL(devicesChanged()));
    // a removed universe takes the input of its device with it
    connect(m_core->doc()->inputOutputMap(), SIGNAL(universeRemoved(quint32)), this, SIGNAL(devicesChanged()));
    connect(m_core->doc()->inputOutputMap(), SIGNAL(pluginConfigurationChanged(QString,bool)),
            this, SIGNAL(devicesChanged()));
    // opening a show adds its universes before it patches them: the devices
    // are read again once it is loaded
    connect(m_core->doc(), SIGNAL(loaded()), this, SIGNAL(devicesChanged()));
    // the LEDs of a device go off before its universe goes
    if (m_core->io() != nullptr)
        connect(m_core->io(), &IOManager::universeAboutToBeRemoved, this, &ControlMapper::slotUniverseAboutToBeRemoved);

    // the functions and the groups deleted: their mappings drive nothing (a
    // new function or group may get their ID)
    connect(m_core->doc(), SIGNAL(functionRemoved(quint32)), this, SLOT(slotFunctionRemoved(quint32)));
    if (m_core->zones() != nullptr)
        connect(m_core->zones(), &ZoneManager::rowsAboutToBeRemoved, this, &ControlMapper::slotZonesAboutToBeRemoved);
    // the faders of the SHOW follow their pages
    if (m_core->surface() != nullptr)
        connect(m_core->surface(), &ShowSurface::pagesChanged, this, &ControlMapper::slotSurfacePagesChanged);

    // the LEDs and motor faders follow what they drive
    m_feedbackTimer->setInterval(100);
    connect(m_feedbackTimer, &QTimer::timeout, this, &ControlMapper::slotFeedback);
    m_feedbackTimer->start();

    // the mappings on a slot of a playback page follow the page
    if (m_core->playbacks() != nullptr)
        connect(m_core->playbacks(), &PlaybackManager::pagesRenumbered,
                this, &ControlMapper::slotPlaybackPagesRenumbered);

    // the list names what each mapping drives: it follows renames (the
    // scenes, groups, palettes, fixture groups and mixers, the labels of the
    // SHOW faders, what a playback slot holds). Only edits: not what changes
    // while playing (levels, functions started)
    connect(m_core->doc(), SIGNAL(functionNameChanged(quint32)), this, SLOT(slotNamesChanged()));
    connect(m_core->doc(), SIGNAL(fixtureGroupChanged(quint32)), this, SLOT(slotNamesChanged()));
    if (m_core->palettes() != nullptr)
        connect(m_core->palettes(), &PaletteManager::palettesChanged, this, &ControlMapper::slotNamesChanged);
    if (m_core->playbacks() != nullptr)
        connect(m_core->playbacks(), &PlaybackManager::assignmentsChanged, this, &ControlMapper::slotNamesChanged);
    if (m_core->surface() != nullptr)
        connect(m_core->surface(), &ShowSurface::pagesChanged, this, &ControlMapper::slotNamesChanged);
    if (m_core->zones() != nullptr)
    {
        connect(m_core->zones(), &ZoneManager::countChanged, this, &ControlMapper::slotZonesChanged);
        connect(m_core->zones(), &ZoneManager::modelReset, this, &ControlMapper::slotZonesChanged);
        slotZonesChanged();
    }
}

ControlMapper::~ControlMapper()
{
}

/*********************************************************************
 * Devices
 *********************************************************************/

static QString kindOfPlugin(const QString &plugin)
{
    if (plugin.contains("MIDI", Qt::CaseInsensitive))
        return "midi";
    if (plugin.contains("OSC", Qt::CaseInsensitive))
        return "osc";
    return "dmx";
}

QVariantList ControlMapper::devices() const
{
    QVariantList list;
    InputOutputMap *io = m_core->doc()->inputOutputMap();
    for (const QString &plugin : io->inputPluginNames())
    {
        const QStringList lines = io->pluginInputs(plugin);
        const QStringList outputs = io->pluginOutputs(plugin);
        for (int line = 0; line < lines.count(); line++)
        {
            QString key = plugin + "|" + lines.at(line);
            int universe = universeOf(key);
            OutputPatch *feedback = universe >= 0 ? io->feedbackPatch(quint32(universe)) : nullptr;

            QVariantMap device;
            device.insert("key", key);
            device.insert("plugin", plugin);
            device.insert("name", lines.at(line));
            device.insert("kind", kindOfPlugin(plugin));
            device.insert("enabled", universe >= 0);
            device.insert("universe", universe);
            device.insert("feedback", feedback != nullptr && feedback->isPatched());
            device.insert("canFeedback", outputs.contains(lines.at(line)) || kindOfPlugin(plugin) == "osc");
            device.insert("controller", controllerOf(lines.at(line)));
            list.append(device);
        }
    }
    return list;
}

QString ControlMapper::controllerOf(const QString &deviceName)
{
    QString name = deviceName.toLower().remove(' ').remove('_').remove('-');
    // the second port of the APC mini mk2 only plays notes (Note mode)
    if (name.startsWith("midiin2") || name.startsWith("midiout2") || name.contains("notes"))
        return QString();
    if (name.contains("apcmini"))
        return name.contains("mk2") || name.contains("mkii") ? "apcmini2" : "apcmini";
    // the first APC40 speaks another protocol: not mapped
    if (name.contains("apc40"))
        return name.contains("mk2") || name.contains("mkii") ? "apc40mk2" : QString();
    return QString();
}

/** Adds one mapping of a template: its target, its input channel and its own keys */
typedef std::function<void(const QString &target, quint32 channel, const QVariantMap &props)> TemplateAdd;

/** A note or a CC as a QLC+ input channel, with the MIDI channel (0-based) in the top bits */
static quint32 midiNote(int note, int midiChannel = 0)
{
    return (quint32(midiChannel) << 12) | quint32(128 + note);
}

static quint32 midiCC(int cc, int midiChannel = 0)
{
    return (quint32(midiChannel) << 12) | quint32(cc);
}

/** The LEDs of a button: the velocity of each state and its MIDI channel
 *  (1-based, 0: the one of the control) */
static QVariantMap ledsOf(int on, int onChannel, int paused, int pausedChannel, int off, int offChannel)
{
    return { { "onValue", on }, { "onChannel", onChannel }, { "pausedValue", paused },
             { "pausedChannel", pausedChannel }, { "offValue", off }, { "offChannel", offChannel } };
}

/** $map with the keys of $more */
static QVariantMap with(QVariantMap map, const QVariantMap &more)
{
    for (auto it = more.cbegin(); it != more.cend(); ++it)
        map.insert(it.key(), it.value());
    return map;
}

/** APC mini and mk2, layout "groups": a column of pads per group, its fader
 *  and the button under the pads that releases it */
static void groupsTemplate(ZoneManager *zones, bool mk2, const TemplateAdd &add)
{
    // the LEDs: APC mini green 1, green blink 2, yellow 5; mk2 a palette of
    // colours, the MIDI channel sets the brightness (1 dim, 7 full, 10 pulse)
    const QVariantMap padColours = mk2 ? ledsOf(21, 7, 21, 10, 13, 1) : ledsOf(1, 0, 2, 0, 5, 0);
    const QVariantMap buttonLeds = ledsOf(1, 0, 2, 0, 0, 0);

    // pads: a column per group, its scenes from the top row
    int groups = qMin(8, zones->count());
    for (int col = 0; col < groups; col++)
    {
        int zoneId = int(zones->zone(col)->id());
        for (int row = 0; row < 8; row++)
            add("sceneIndex", midiNote((7 - row) * 8 + col), with(padColours, { { "zoneId", zoneId }, { "index", row } }));
        // its fader and the button under the pads
        add("groupMaster", midiCC(48 + col), { { "zoneId", zoneId }, { "feedback", false } });
        add("groupRelease", midiNote((mk2 ? 100 : 64) + col), with(buttonLeds, { { "zoneId", zoneId } }));
    }
    add("grandMaster", midiCC(56), { { "feedback", false } });

    // the buttons on the right, from the top
    const QStringList right = { "blackout", "stopAll", "pauseAll", "tap", "beatgo", "groupPrevious", "groupNext", "mixerFlash" };
    for (int i = 0; i < right.count(); i++)
        add(right.at(i), midiNote((mk2 ? 112 : 82) + i), with(buttonLeds, { { "zoneId", -1 }, { "mixerId", -1 } }));
}

/** APC mini and mk2, layout "playbacks": a column per playback (slots 1-8
 *  of the current playback page) */
static void miniPlaybacksTemplate(bool mk2, const TemplateAdd &add)
{
    // the LEDs, on / paused / off. APC mini: 1 green, 2 green blinking,
    // 3 red, 4 red blinking, 5 yellow. mk2: the colour is the velocity
    // (21 green, 13 yellow, 9 orange, 3 white, 45 blue), the MIDI channel the
    // behaviour (2: 25 %, 7: 100 %, 10: pulsing, 14: blinking)
    const QVariantMap go = mk2 ? ledsOf(21, 7, 21, 10, 13, 2) : ledsOf(1, 0, 2, 0, 5, 0);
    const QVariantMap pauseBack = mk2 ? ledsOf(9, 7, 9, 14, 0, 0) : ledsOf(3, 0, 4, 0, 0, 0);
    const QVariantMap flash = mk2 ? ledsOf(3, 7, 3, 7, 3, 2) : ledsOf(5, 0, 5, 0, 0, 0);
    // a cue: on while it plays, "paused" when the next GO plays it, off for
    // the other cues (dark when the list has no such cue)
    const QVariantMap cue = mk2 ? ledsOf(21, 7, 21, 10, 45, 2) : ledsOf(1, 0, 2, 0, 5, 0);
    // the round buttons have a single LED
    const QVariantMap buttonLeds = ledsOf(1, 0, 2, 0, 0, 0);
    // the faders have no motor: they pick the level up
    const QVariantMap fader = { { "pickup", true }, { "feedback", false } };

    // (the playback targets follow the current page: pbPage -1, their default)
    for (int col = 0; col < 8; col++)
    {
        const QVariantMap slot = { { "slot", col } };
        add("playbackFader", midiCC(48 + col), with(fader, slot));
        // the pads from the bottom row: GO, Pause/Back, Flash, then the first
        // 5 cues with cue 1 on the top row
        add("playbackGo", midiNote(col), with(go, slot));
        add("playbackPauseBack", midiNote(8 + col), with(pauseBack, slot));
        add("playbackFlash", midiNote(16 + col), with(flash, slot));
        for (int r = 0; r < 5; r++)
            add("playbackGoTo", midiNote((7 - r) * 8 + col), with(cue, { { "slot", col }, { "cue", r } }));
        // the button under the pads
        add("playbackRelease", midiNote((mk2 ? 100 : 64) + col), with(buttonLeds, slot));
    }
    add("grandMaster", midiCC(56), fader);

    // the buttons on the right, from the top (Shift stays free)
    const QStringList right = { "playbackPagePrevious", "playbackPageNext", "playbackMainGo", "playbackMainBack",
                                "blackout", "tap", "beatgo", "stopAll" };
    for (int i = 0; i < right.count(); i++)
        add(right.at(i), midiNote((mk2 ? 112 : 82) + i), buttonLeds);
}

/** APC40 mkII (Ableton mode), layout "playbacks": a track per playback
 *  (slots 1-8 of the current page), the clip grid on their first 5 cues, the
 *  scene buttons on the pages, the device knobs on slots 9-16 and the track
 *  knobs on the masters of the first 8 groups */
static void apc40PlaybacksTemplate(ZoneManager *zones, const TemplateAdd &add)
{
    // the RGB pads, solid (MIDI channel 1): green, yellow, dim blue
    const QVariantMap cue = ledsOf(21, 1, 13, 1, 46, 1);
    const QVariantMap page = ledsOf(21, 1, 21, 1, 46, 1);
    // the single LEDs: on (fading too), off; Clip Stop also blinks
    const QVariantMap lit = ledsOf(1, 0, 1, 0, 0, 0);
    const QVariantMap blinking = ledsOf(1, 0, 2, 0, 0, 0);
    const QVariantMap noLed = { { "feedback", false } };
    // the faders have no motor: they pick the level up
    const QVariantMap fader = { { "pickup", true }, { "feedback", false } };

    for (int c = 0; c < 8; c++)
    {
        // each track sends on its own MIDI channel
        const QVariantMap slot = { { "slot", c } };
        add("playbackFader", midiCC(7, c), with(fader, slot));
        add("playbackGo", midiNote(50, c), with(lit, slot));            // activator (track number)
        add("playbackPauseBack", midiNote(49, c), with(lit, slot));     // solo
        add("playbackFlash", midiNote(48, c), with(lit, slot));         // record arm
        add("playbackSelect", midiNote(51, c), with(lit, slot));        // track select
        add("playbackRelease", midiNote(52, c), with(blinking, slot));  // clip stop
        // the clip grid: cue 1 on the top row (notes 32-39)
        for (int r = 0; r < 5; r++)
            add("playbackGoTo", midiNote(32 - 8 * r + c), with(cue, { { "slot", c }, { "cue", r } }));
        // the device knobs: slots 9-16, their rings follow the level (and the page)
        add("playbackFader", midiCC(16 + c), { { "slot", 8 + c }, { "feedback", true } });
    }
    add("grandMaster", midiCC(14), fader);

    // the scene buttons: the first 5 pages (dark past the last page)
    for (int i = 0; i < 5; i++)
        add("playbackPage", midiNote(82 + i), with(page, { { "pbPage", i } }));

    add("playbackPagePrevious", midiNote(97), noLed);                    // bank left
    add("playbackPageNext", midiNote(96), noLed);                        // bank right
    add("groupPrevious", midiNote(94), with(noLed, { { "zoneId", -1 } })); // bank up
    add("groupNext", midiNote(95), with(noLed, { { "zoneId", -1 } }));     // bank down
    add("playbackReleaseAll", midiNote(81), noLed);                      // stop all clips
    add("playbackMainGo", midiNote(91), lit);                            // play
    add("playbackMainBack", midiNote(93), lit);                          // record
    add("blackout", midiNote(102), lit);                                 // session
    add("beatgo", midiNote(90), lit);                                    // metronome
    add("tap", midiNote(99), noLed);                                     // tap tempo
    // the footswitch: a CC, 127 pressed and 0 released (a button)
    add("playbackMainGo", midiCC(64), noLed);
    // the tempo knob: an encoder in two's complement, about 1 BPM a step
    add("bpm", midiCC(13), { { "mode", "relative2c" }, { "step", 2 }, { "feedback", false } });

    // the track knobs: the masters of the first 8 groups (the rings follow them)
    const int groups = qMin(8, zones->count());
    for (int i = 0; i < groups; i++)
        add("groupMaster", midiCC(48 + i), { { "zoneId", int(zones->zone(i)->id()) }, { "feedback", true } });
}

int ControlMapper::applyTemplate(const QString &deviceKey, const QString &controller, const QString &layout)
{
    const bool mini = controller == "apcmini" || controller == "apcmini2";
    const bool known = layout == "groups" ? mini
                     : layout == "playbacks" ? mini || controller == "apc40mk2"
                     : false;
    if (known == false)
        return 0;

    // the mappings of this device are replaced; their LEDs go off first (the
    // new layout may leave some controls free)
    for (int i = m_mappings.count() - 1; i >= 0; i--)
    {
        const QVariantMap m = m_mappings.at(i);
        if (m.value("device").toString() != deviceKey)
            continue;
        lightOff(m);
        const int id = m.value("id").toInt();
        m_pressed.remove(id);
        m_applied.remove(id);
        m_physical.remove(id);
        m_mappings.removeAt(i);
        if (id == m_learnSourceOf)
            dropPendingLearn();
    }

    int made = 0;
    const TemplateAdd add = [&](const QString &target, quint32 channel, const QVariantMap &props) {
        QVariantMap m = defaultsFor(target);
        m.insert("source", "midi");
        m.insert("device", deviceKey);
        m.insert("controller", controller);
        m.insert("channel", channel);
        for (auto it = props.cbegin(); it != props.cend(); ++it)
            m.insert(it.key(), it.value());
        m.insert("id", ++m_lastId);
        m_mappings.append(m);
        made++;
    };

    const bool mk2 = controller == "apcmini2";
    if (layout == "groups")
        groupsTemplate(m_core->zones(), mk2, add);
    else if (mini)
        miniPlaybacksTemplate(mk2, add);
    else
        apc40PlaybacksTemplate(m_core->zones(), add);

    // its LEDs get ready (and are all sent again)
    initControllers();
    m_core->doc()->setModified();
    emit mappingsChanged();
    return made;
}

void ControlMapper::initControllers()
{
    InputOutputMap *io = m_core->doc()->inputOutputMap();
    for (const QVariant &v : devices())
    {
        const QVariantMap device = v.toMap();
        const QString controller = device.value("controller").toString();
        if (device.value("kind").toString() != "midi" || device.value("enabled").toBool() == false ||
            device.value("feedback").toBool() == false || controller.startsWith("apc") == false)
            continue;
        const quint32 universe = device.value("universe").toUInt();
        OutputPatch *patch = io->feedbackPatch(universe);
        if (patch == nullptr || patch->isPatched() == false)
            continue;

        // the APCs ignore note offs: an LED goes dark with a note on at 0
        patch->setPluginParameter("MIDISendNoteOff", false);

        if (controller == "apc40mk2")
        {
            // Ableton mode, where the host drives the LEDs: the QLC+ MIDI
            // template sends its SysEx when the output opens
            const QString ableton = QStringLiteral("APC40 Mk2 Ableton mode");
            if (patch->getPluginParameters().value("initmessage").toString() != ableton)
            {
                patch->setPluginParameter("initmessage", ableton);
                patch->reconnect();
            }
            // the rings of the track and device knobs: a single dot
            for (quint32 cc = 56; cc <= 63; cc++)
                io->sendFeedBack(universe, cc, dmxFromMidi(1), QVariant());
            for (quint32 cc = 24; cc <= 31; cc++)
                io->sendFeedBack(universe, cc, dmxFromMidi(1), QVariant());
        }
    }
    // everything is sent again on the next tick
    m_sentFeedback.clear();
}

int ControlMapper::universeOf(const QString &deviceKey) const
{
    InputOutputMap *io = m_core->doc()->inputOutputMap();
    for (quint32 u = 0; u < io->universesCount(); u++)
    {
        InputPatch *patch = io->inputPatch(u);
        if (patch != nullptr && patch->isPatched() && patch->pluginName() + "|" + patch->inputName() == deviceKey)
            return int(u);
    }
    return -1;
}

QString ControlMapper::deviceKeyOf(quint32 universe, QString *kind) const
{
    InputPatch *patch = m_core->doc()->inputOutputMap()->inputPatch(universe);
    if (patch == nullptr || patch->isPatched() == false)
        return QString();
    if (kind)
        *kind = kindOfPlugin(patch->pluginName());
    return patch->pluginName() + "|" + patch->inputName();
}

void ControlMapper::setDeviceEnabled(const QString &key, bool enabled)
{
    InputOutputMap *io = m_core->doc()->inputOutputMap();
    QString plugin = key.section('|', 0, 0);
    QString name = key.section('|', 1);
    int current = universeOf(key);
    // the input and feedback patches are saved in the show: it has to be saved
    // again, or opening it brings the device back as it was
    bool changed = false;

    if (enabled == false)
    {
        if (current >= 0)
        {
            // its LEDs go off while it can still hear us
            for (const QVariantMap &m : std::as_const(m_mappings))
                lightOff(m, key);
            io->setInputPatch(quint32(current), KInputNone, QString(), QString(), QLCIOPlugin::invalidLine());
            io->setOutputPatch(quint32(current), KOutputNone, QString(), QString(), QLCIOPlugin::invalidLine(), true);
            changed = true;
        }
    }
    else if (current < 0)
    {
        changed = true;
        // the first universe without an input; one more if they are all taken
        int universe = -1;
        for (quint32 u = 0; u < io->universesCount() && universe < 0; u++)
        {
            InputPatch *patch = io->inputPatch(u);
            if (patch == nullptr || patch->isPatched() == false)
                universe = int(u);
        }
        if (universe < 0)
        {
            io->addUniverse();
            io->startUniverses();
            universe = int(io->universesCount()) - 1;
        }

        int line = io->pluginInputs(plugin).indexOf(name);
        io->setInputPatch(quint32(universe), plugin, QString(), name, quint32(qMax(0, line)));
        // MIDI: every channel, with the MIDI channel in the input channel
        if (InputPatch *patch = io->inputPatch(quint32(universe)))
            if (kindOfPlugin(plugin) == "midi")
                patch->setPluginParameter("midichannel", MIDI_OMNI);
        // the LEDs, when the device can show them
        if (io->pluginOutputs(plugin).contains(name))
            setDeviceFeedback(key, true);
    }

    io->saveDefaults();
    if (changed)
        m_core->doc()->setModified();
    emit devicesChanged();
}

void ControlMapper::setDeviceFeedback(const QString &key, bool enabled)
{
    InputOutputMap *io = m_core->doc()->inputOutputMap();
    int universe = universeOf(key);
    if (universe < 0)
    {
        // (not enabled: its universe was removed, say) the switch shows it again
        emit devicesChanged();
        return;
    }

    QString plugin = key.section('|', 0, 0);
    QString name = key.section('|', 1);
    if (enabled)
    {
        int line = io->pluginOutputs(plugin).indexOf(name);
        if (line < 0 && kindOfPlugin(plugin) == "osc")
            line = 0;
        if (line < 0)
        {
            emit devicesChanged();
            return;
        }
        io->setOutputPatch(quint32(universe), plugin, QString(), io->pluginOutputs(plugin).value(line),
                           quint32(line), true);
        if (OutputPatch *patch = io->feedbackPatch(quint32(universe)))
            if (kindOfPlugin(plugin) == "midi")
                patch->setPluginParameter("midichannel", MIDI_OMNI);
        // a known controller gets its LEDs ready
        initControllers();
    }
    else
    {
        // its LEDs go off while it can still hear us
        for (const QVariantMap &m : std::as_const(m_mappings))
            lightOff(m, key);
        io->setOutputPatch(quint32(universe), KOutputNone, QString(), QString(), QLCIOPlugin::invalidLine(), true);
    }
    m_sentFeedback.clear();
    io->saveDefaults();
    // (saved in the show, as the input)
    m_core->doc()->setModified();
    emit devicesChanged();
}

void ControlMapper::refreshDevices()
{
    emit devicesChanged();
}

void ControlMapper::slotUniverseAboutToBeRemoved(int universe)
{
    // the device of that universe: its LEDs go off while it can still hear us
    const QString key = deviceKeyOf(quint32(universe));
    if (key.isEmpty())
        return;
    for (const QVariantMap &m : std::as_const(m_mappings))
        lightOff(m, key);
    m_sentFeedback.clear();
}

/*********************************************************************
 * Targets
 *********************************************************************/

QVariantList ControlMapper::targets() const
{
    auto t = [](const QString &target, const QString &kind, const QString &name, const QString &needs) {
        QVariantMap map;
        map.insert("target", target);
        map.insert("kind", kind);
        map.insert("name", name);
        map.insert("needs", needs);
        return map;
    };
    return QVariantList()
        << t("scene", "button", tr("Play a scene"), "function")
        << t("sceneIndex", "button", tr("Scene N of a group"), "zoneIndex")
        << t("groupRelease", "button", tr("Release a group"), "zoneAll")
        << t("groupNext", "button", tr("Next scene of a group"), "zoneAll")
        << t("groupPrevious", "button", tr("Previous scene of a group"), "zoneAll")
        << t("blackout", "button", tr("Blackout"), "")
        << t("stopAll", "button", tr("Stop all"), "")
        << t("pauseAll", "button", tr("Pause all"), "")
        << t("tap", "button", tr("Tap tempo"), "")
        << t("beatgo", "button", tr("BeatGO"), "")
        << t("palette", "button", tr("Palette"), "palette")
        << t("mixerFull", "button", tr("Mixer: full"), "mixer")
        << t("mixerBlackout", "button", tr("Mixer: blackout"), "mixer")
        << t("mixerFlash", "button", tr("Mixer: flash"), "mixer")
        << t("mixerFlashExclusive", "button", tr("Mixer: exclusive flash"), "mixer")
        << t("mixerFullExclusive", "button", tr("Mixer: exclusive full"), "mixer")
        << t("mixerStrobe", "button", tr("Mixer: strobe"), "mixer")
        << t("playbackGo", "button", tr("Playback: GO"), "playback")
        << t("playbackBack", "button", tr("Playback: back"), "playback")
        << t("playbackPauseBack", "button", tr("Playback: pause / back"), "playback")
        << t("playbackFlash", "button", tr("Playback: flash"), "playback")
        << t("playbackRelease", "button", tr("Playback: release"), "playback")
        << t("playbackSelect", "button", tr("Playback: select"), "playback")
        << t("playbackGoTo", "button", tr("Playback: go to a cue"), "playbackCue")
        << t("playbackMainGo", "button", tr("GO of the selected playback"), "")
        << t("playbackMainBack", "button", tr("Back of the selected playback"), "")
        << t("playbackPage", "button", tr("Playback page N"), "playbackPage")
        << t("playbackPageNext", "button", tr("Next playback page"), "")
        << t("playbackPagePrevious", "button", tr("Previous playback page"), "")
        << t("playbackReleaseAll", "button", tr("Release all playbacks"), "")
        << t("grandMaster", "fader", tr("Grand master"), "")
        << t("groupMaster", "fader", tr("Master of a group"), "zone")
        << t("sceneDimmer", "fader", tr("Live dimmer of a scene"), "function")
        << t("sceneSpeed", "fader", tr("Live speed of a scene"), "function")
        << t("bpm", "fader", tr("BPM"), "")
        << t("mixerDimmer", "fader", tr("Mixer: dimmer"), "mixer")
        << t("mixerHue", "fader", tr("Mixer: hue"), "mixer")
        << t("mixerSaturation", "fader", tr("Mixer: saturation"), "mixer")
        << t("mixerStrobeHz", "fader", tr("Mixer: strobe speed"), "mixer")
        << t("surfaceFader", "fader", tr("Fader of the SHOW"), "surface")
        << t("playbackFader", "fader", tr("Playback: fader"), "playback");
}

bool ControlMapper::isButton(const QString &target) const
{
    // read for every mapping on every feedback tick: the kinds are kept
    if (m_buttonKinds.isEmpty())
    {
        for (const QVariant &v : targets())
        {
            const QVariantMap t = v.toMap();
            m_buttonKinds.insert(t.value("target").toString(), t.value("kind").toString() == "button");
        }
    }
    // (an unknown target is a button)
    return m_buttonKinds.value(target, true);
}

QVariantMap ControlMapper::defaultsFor(const QString &target) const
{
    QVariantMap m;
    m.insert("source", "midi");
    m.insert("device", QString());
    m.insert("channel", 0);
    m.insert("key", QString());
    m.insert("target", target);
    m.insert("functionId", -1);
    m.insert("zoneId", -1);
    m.insert("index", 0);
    m.insert("mixerId", -1);
    bool button = isButton(target);
    m.insert("mode", button ? (target == "mixerFlash" || target == "mixerFlashExclusive" ||
                               target == "playbackFlash" ? "flash" : "toggle")
                            : "absolute");
    m.insert("min", 0);
    m.insert("max", 100);
    m.insert("invert", false);
    m.insert("step", 5);
    m.insert("loop", false);
    m.insert("pickup", false);
    m.insert("feedback", true);
    m.insert("onValue", 127);
    m.insert("offValue", 0);
    m.insert("pausedValue", 64);
    // the MIDI channel of each state (0: the one of the control): on some
    // controllers (APC mini mk2) it sets the brightness or the blinking
    m.insert("onChannel", 0);
    m.insert("offChannel", 0);
    m.insert("pausedChannel", 0);
    m.insert("controller", QString());
    if (target == "palette")
    {
        // on its own fixtures (-1) or a fixture group
        m.insert("paletteId", -1);
        m.insert("groupId", -1);
    }
    if (target.startsWith("playback"))
    {
        // a slot of the current playback page (pbPage -1), the first cue;
        // "page" is the page of the SHOW
        m.insert("pbPage", target == "playbackPage" ? 0 : -1);
        m.insert("slot", 0);
        m.insert("cue", 0);
    }
    return m;
}

/*********************************************************************
 * Mappings
 *********************************************************************/

QVariantList ControlMapper::mappings() const
{
    QVariantList list;
    for (const QVariantMap &m : m_mappings)
    {
        QVariantMap item = m;
        item.insert("sourceText", describeSource(m));
        item.insert("targetText", describeTarget(m));
        item.insert("button", isButton(m.value("target").toString()));
        list.append(item);
    }
    return list;
}

int ControlMapper::addMapping(const QVariantMap &mapping)
{
    QVariantMap m = defaultsFor(mapping.value("target", "scene").toString());
    for (auto it = mapping.cbegin(); it != mapping.cend(); ++it)
        m.insert(it.key(), it.value());
    normalizeMapping(m);
    m.insert("id", ++m_lastId);
    m_mappings.append(m);
    m_core->doc()->setModified();
    emit mappingsChanged();
    return m_lastId;
}

void ControlMapper::updateMapping(int id, const QVariantMap &props)
{
    for (QVariantMap &m : m_mappings)
    {
        if (m.value("id").toInt() != id)
            continue;
        // moved to another control or device, or no feedback any more: the old LED goes off
        for (const char *key : { "source", "device", "channel", "key", "feedback", "target" })
        {
            if (props.contains(key) && props.value(key) != m.value(key))
            {
                lightOff(m);
                m_sentFeedback.clear();
                break;
            }
        }
        // a new target brings the modes of its kind
        if (props.contains("target") && isButton(props.value("target").toString()) != isButton(m.value("target").toString()))
            m.insert("mode", isButton(props.value("target").toString()) ? "toggle" : "absolute");
        if (props.contains("target") && props.value("target") != m.value("target"))
        {
            // and the keys only it has (pbPage, slot, cue...); a scene picked
            // for another target doesn't become the function of a playback
            const QString target = props.value("target").toString();
            const QString old = m.value("target").toString();
            const QVariantMap defaults = defaultsFor(target);
            for (auto it = defaults.cbegin(); it != defaults.cend(); ++it)
                if (m.contains(it.key()) == false)
                    m.insert(it.key(), it.value());
            if (target.startsWith("playback") && old.startsWith("playback") == false &&
                props.contains("functionId") == false)
                m.insert("functionId", -1);
            // pbPage is "page N" for a page button, the page of the slot (-1
            // the current one) for the others: it doesn't carry over between them
            if (target.startsWith("playback") && old.startsWith("playback") &&
                (target == "playbackPage") != (old == "playbackPage") && props.contains("pbPage") == false)
                m.insert("pbPage", defaults.value("pbPage"));
        }
        // another kind of control: a device of the old kind would never send it
        // (keys come from no device: one left there hides the key)
        if (props.contains("source") && props.contains("device") == false && props.value("source") != m.value("source"))
        {
            const QString source = props.value("source").toString();
            const QString device = m.value("device").toString();
            if (device.isEmpty() == false && (source == "key" || kindOfPlugin(device.section('|', 0, 0)) != source))
                m.insert("device", QString());
        }
        for (auto it = props.cbegin(); it != props.cend(); ++it)
            if (it.key() != "id")
                m.insert(it.key(), it.value());
        normalizeMapping(m);
        m_sentFeedback.remove(id);
        // a fader that picks up starts again
        m_applied.remove(id);
        m_physical.remove(id);
        m_core->doc()->setModified();
        emit mappingsChanged();
        return;
    }
}

void ControlMapper::removeMapping(int id)
{
    for (int i = 0; i < m_mappings.count(); i++)
    {
        if (m_mappings.at(i).value("id").toInt() == id)
        {
            // its LED goes off; another mapping of the same control lights it again
            lightOff(m_mappings.at(i));
            m_mappings.removeAt(i);
            m_sentFeedback.clear();
            m_pressed.remove(id);
            m_applied.remove(id);
            m_physical.remove(id);
            // the control it was learning has nothing to go to
            if (id == m_learnSourceOf)
                dropPendingLearn();
            m_core->doc()->setModified();
            emit mappingsChanged();
            return;
        }
    }
}

int ControlMapper::duplicateMapping(int id)
{
    QVariantMap m = mapping(id);
    if (m.isEmpty())
        return -1;
    m.remove("id");
    return addMapping(m);
}

QVariantMap ControlMapper::mapping(int id) const
{
    for (const QVariantMap &m : m_mappings)
        if (m.value("id").toInt() == id)
            return m;
    return QVariantMap();
}

int ControlMapper::countFor(const QVariantMap &target) const
{
    int count = 0;
    for (const QVariantMap &m : m_mappings)
    {
        bool same = m.value("target") == target.value("target");
        for (const char *key : { "functionId", "zoneId", "index", "mixerId", "page", "section", "cell", "paletteId", "groupId",
                                 "pbPage", "slot", "cue" })
            if (target.contains(key) && m.value(key) != target.value(key))
                same = false;
        if (same)
            count++;
    }
    return count;
}

void ControlMapper::clear()
{
    lightsOff();
    m_mappings.clear();
    m_sentFeedback.clear();
    m_pressed.clear();
    m_applied.clear();
    m_physical.clear();
    m_lastId = 0;
    // a control being learnt was for a mapping or a target of that show (the
    // next show reuses the IDs)
    dropPendingLearn();
    emit mappingsChanged();
}

void ControlMapper::slotFunctionRemoved(quint32 functionId)
{
    bool changed = false;
    for (QVariantMap &m : m_mappings)
    {
        if (m.value("functionId", -1).toInt() < 0 || m.value("functionId").toUInt() != functionId)
            continue;
        // a function made later may get its ID: this drives nothing, not that
        // one (and a playback GO doesn't fall back on the slot it says either)
        m.insert("functionId", -1);
        if (isFunctionTarget(m.value("target").toString()))
            m.insert("slot", -1);
        m_sentFeedback.remove(m.value("id").toInt());
        changed = true;
    }
    if (changed)
        emit mappingsChanged();
}

void ControlMapper::slotZonesAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    QSet<int> gone;
    for (int i = first; i <= last; i++)
        if (Zone *zone = m_core->zones()->zone(i))
            gone.insert(int(zone->id()));

    bool changed = false;
    for (QVariantMap &m : m_mappings)
    {
        if (m.contains("zoneId") == false || m.value("zoneId").toInt() < 0 || gone.contains(m.value("zoneId").toInt()) == false)
            continue;
        // the next group made gets its ID: this drives no group (not "all
        // groups", -1, either)
        m.insert("zoneId", DELETED_GROUP);
        m_sentFeedback.remove(m.value("id").toInt());
        m_applied.remove(m.value("id").toInt());
        m_physical.remove(m.value("id").toInt());
        changed = true;
    }
    if (changed)
        emit mappingsChanged();
}

int ControlMapper::surfacePageOf(const QVariantMap &m) const
{
    // the IDs of the sections are unique in the whole SHOW: on the page the
    // mapping says, or wherever that page went (pages moved or deleted)
    ShowSurface *surface = m_core->surface();
    const QString section = m.value("section").toString();
    if (surface == nullptr || section.isEmpty())
        return -1;
    const int page = m.value("page").toInt();
    if (surface->section(page, section).isEmpty() == false)
        return page;
    for (int p = 0; p < surface->pageCount(); p++)
        if (p != page && surface->section(p, section).isEmpty() == false)
            return p;
    return -1;
}

QVariantMap ControlMapper::surfaceFaderItem(const QVariantMap &m) const
{
    const int page = surfacePageOf(m);
    if (page < 0)
        return QVariantMap();
    return m_core->surface()->item(page, m.value("section").toString(), m.value("cell").toInt());
}

void ControlMapper::slotSurfacePagesChanged()
{
    // a fader of the SHOW is learnt with the index of its page: it follows
    // the page (the mappings list, the badges of learn mode, the file)
    bool changed = false;
    for (QVariantMap &m : m_mappings)
    {
        if (m.value("target").toString() != "surfaceFader")
            continue;
        const int page = surfacePageOf(m);
        if (page < 0 || page == m.value("page").toInt())
            continue;
        m.insert("page", page);
        changed = true;
    }
    if (changed)
        emit mappingsChanged();
}

void ControlMapper::slotNamesChanged()
{
    // (a show opened renames many things at once: told once)
    if (m_mappings.isEmpty() || m_namesPending)
        return;
    m_namesPending = true;
    QMetaObject::invokeMethod(this, [this]() {
        m_namesPending = false;
        emit mappingsChanged();
    }, Qt::QueuedConnection);
}

void ControlMapper::slotZonesChanged()
{
    for (int i = 0; i < m_core->zones()->count(); i++)
        if (Zone *zone = m_core->zones()->zone(i))
            connect(zone, &Zone::nameChanged, this, &ControlMapper::slotNamesChanged, Qt::UniqueConnection);
}

bool ControlMapper::followPlaybackPages(QVariantMap &target, const QList<int> &newIndexes,
                                        const QHash<int, quint32> &removedSlots)
{
    // a slot of a page given by its index (-1, the current page, is no page)
    const QString kind = target.value("target").toString();
    const int page = target.value("pbPage", -1).toInt();
    if (isSlotTarget(kind) == false || page < 0 || page >= newIndexes.count())
        return false;
    const int now = newIndexes.at(page);
    if (now == page)
        return false;
    if (now >= 0)
    {
        target.insert("pbPage", now);
        return true;
    }

    // its page was deleted: never the slot of another page
    target.insert("pbPage", -1);
    const int slot = target.value("slot", -1).toInt();
    if (isFunctionTarget(kind) && (target.value("functionId", -1).toInt() >= 0 || removedSlots.contains(slot)))
    {
        if (target.value("functionId", -1).toInt() < 0)
            target.insert("functionId", int(removedSlots.value(slot)));
    }
    else
    {
        target.insert("slot", -1);
    }
    return true;
}

void ControlMapper::slotPlaybackPagesRenumbered(const QList<int> &newIndexes, const QHash<int, quint32> &removedSlots)
{
    bool changed = false;
    for (QVariantMap &m : m_mappings)
    {
        if (followPlaybackPages(m, newIndexes, removedSlots) == false)
            continue;
        // what it shows and where its fader is are of another target now
        const int id = m.value("id").toInt();
        m_sentFeedback.remove(id);
        m_applied.remove(id);
        m_physical.remove(id);
        changed = true;
    }
    if (changed)
    {
        m_core->doc()->setModified();
        emit mappingsChanged();
    }
}

QString ControlMapper::describeMidi(quint32 channel)
{
    int midiChannel = int(channel >> 12) + 1;
    quint32 ch = channel & 0x0FFF;
    QString what;
    if (ch <= 127)
        what = tr("CC %1").arg(ch);
    else if (ch <= 255)
        what = tr("Note %1").arg(ch - 128);
    else if (ch <= 383)
        what = tr("Aftertouch %1").arg(ch - 256);
    else if (ch <= 511)
        what = tr("Program %1").arg(ch - 384);
    else if (ch == 512)
        what = tr("Channel aftertouch");
    else if (ch == 513)
        what = tr("Pitch wheel");
    else
        what = QString("MIDI %1").arg(ch);
    return tr("%1 · ch %2").arg(what).arg(midiChannel);
}

QString ControlMapper::describeSource(const QVariantMap &m) const
{
    QString source = m.value("source").toString();
    QString device = m.value("device").toString().section('|', 1);
    if (device.isEmpty())
        device = tr("any device");

    if (source == "key")
        return tr("Key %1").arg(m.value("key").toString().toUpper());
    if (source == "osc")
        return QString("OSC %1 · %2").arg(m.value("key").toString(), device);
    if (source == "midi")
        return QString("MIDI %1 · %2").arg(describeMidi(m.value("channel").toUInt()), device);
    return QString("DMX %1 · %2").arg(m.value("channel").toUInt() + 1).arg(device);
}

QString ControlMapper::describeTarget(const QVariantMap &m) const
{
    QString target = m.value("target").toString();
    QString name = target;
    for (const QVariant &v : targets())
        if (v.toMap().value("target").toString() == target)
            name = v.toMap().value("name").toString();

    QString detail;
    if (Function *f = m_core->doc()->function(m.value("functionId").toUInt()))
    {
        if (target == "scene" || target == "sceneDimmer" || target == "sceneSpeed")
            detail = f->name();
    }
    Zone *zone = m_core->zones()->zoneById(m.value("zoneId").toUInt());
    const int zoneId = m.value("zoneId").toInt();
    if (target == "sceneIndex")
        detail = QString("%1 · %2").arg(zone ? zone->name() : tr("no group")).arg(m.value("index").toInt() + 1);
    else if (target == "groupMaster" || target.startsWith("group"))
        detail = zone ? zone->name() : zoneId == ALL_GROUPS && target != "groupMaster" ? tr("all groups")
                                     : zoneId == DELETED_GROUP ? tr("no group") : QString();
    if (target.startsWith("mixer"))
    {
        int mixerId = m.value("mixerId").toInt();
        for (const QVariant &v : m_core->mixer()->mixers())
            if (v.toMap().value("id").toInt() == mixerId)
                detail = v.toMap().value("name").toString();
    }
    if (target == "palette")
    {
        const QVariantMap palette = m_core->palettes()->palette(m.value("paletteId").toInt());
        detail = palette.value("name").toString();
        if (FixtureGroup *group = m_core->doc()->fixtureGroup(quint32(m.value("groupId", -1).toInt())))
            detail += " · " + group->name();
    }
    if (target == "surfaceFader")
    {
        const int page = surfacePageOf(m);
        QVariantMap item = surfaceFaderItem(m);
        detail = item.value("label").toString().isEmpty()
                ? tr("page %1, fader %2").arg((page >= 0 ? page : m.value("page").toInt()) + 1).arg(m.value("cell").toInt() + 1)
                : item.value("label").toString();
    }
    if (target.startsWith("playback"))
    {
        // "current page · 3 · Song 1", "page 2 · 3", or the function it drives
        const int pbPage = m.value("pbPage", -1).toInt();
        const int slot = m.value("slot").toInt();
        if (target == "playbackPage")
        {
            detail = pbPage < 0 ? tr("current page") : tr("page %1").arg(pbPage + 1);
        }
        else if (isSlotTarget(target))
        {
            const int fid = isFunctionTarget(target) ? m.value("functionId", -1).toInt() : -1;
            Function *f = fid >= 0 ? m_core->doc()->function(quint32(fid)) : nullptr;
            if (f != nullptr)
            {
                detail = f->name();
            }
            else if (fid >= 0)
            {
                // (it drives that function only: nothing now)
                detail = tr("deleted function");
            }
            else if (slot < 0)
            {
                // (a SHOW cuelist button without its cuelist)
                detail = tr("no cuelist");
            }
            else
            {
                detail = (pbPage < 0 ? tr("current page") : tr("page %1").arg(pbPage + 1)) + " · " + QString::number(slot + 1);
                const QString loaded = m_core->playbacks()->slotInfo(pbPage, slot).value("name").toString();
                if (loaded.isEmpty() == false)
                    detail += " · " + loaded;
            }
            // a cue of a cuelist by its number (it is bound to that cue), a
            // cue of a slot by its place
            const Cuelist *cuelist = qobject_cast<const Cuelist *>(f);
            if (target == "playbackGoTo" && cuelist != nullptr && m.value("cueUid", 0).toUInt() != 0)
            {
                const int index = cueIndexOf(m);
                detail += " · " + tr("cue %1").arg(index >= 0 ? Cuelist::numberToString(cuelist->cueAt(index).number)
                                                              : QStringLiteral("?"));
            }
            else if (target == "playbackGoTo")
            {
                detail += " · " + tr("cue %1").arg(m.value("cue").toInt() + 1);
            }
        }
    }
    return detail.isEmpty() ? name : name + " — " + detail;
}

/*********************************************************************
 * Learn
 *********************************************************************/

bool ControlMapper::learnMode() const
{
    return m_learnMode;
}

void ControlMapper::setLearnMode(bool enable)
{
    if (enable == m_learnMode)
        return;
    m_learnMode = enable;
    if (enable == false)
    {
        m_pendingTarget.clear();
        m_learnSourceOf = -1;
    }
    emit learnChanged();
}

QVariantMap ControlMapper::pendingTarget() const
{
    return m_pendingTarget;
}

void ControlMapper::startLearn(const QVariantMap &target)
{
    m_pendingTarget = target;
    m_learnSourceOf = -1;
    emit learnChanged();
}

void ControlMapper::learnSource(int id)
{
    QVariantMap m = mapping(id);
    if (m.isEmpty())
        return;
    m_learnSourceOf = id;
    m_pendingTarget = m;
    emit learnChanged();
}

void ControlMapper::cancelLearn()
{
    m_pendingTarget.clear();
    m_learnSourceOf = -1;
    emit learnChanged();
}

void ControlMapper::dropPendingLearn()
{
    if (m_pendingTarget.isEmpty() && m_learnSourceOf < 0)
        return;
    cancelLearn();
}

/*********************************************************************
 * Input
 *********************************************************************/

void ControlMapper::log(const QString &direction, const QString &device, const QString &text, int value)
{
    QVariantMap entry;
    entry.insert("time", QTime::currentTime().toString("HH:mm:ss.zzz"));
    entry.insert("direction", direction);
    entry.insert("device", device.section('|', 1));
    entry.insert("text", text);
    entry.insert("value", value);
    m_monitor.prepend(entry);
    while (m_monitor.count() > MONITOR_SIZE)
        m_monitor.removeLast();
    emit monitorChanged();
}

QVariantList ControlMapper::monitor() const
{
    QVariantList list;
    for (const QVariantMap &m : m_monitor)
        list.append(m);
    return list;
}

void ControlMapper::clearMonitor()
{
    m_monitor.clear();
    emit monitorChanged();
}

int ControlMapper::activity() const
{
    return m_activity;
}

void ControlMapper::slotInputValueChanged(quint32 universe, quint32 channel, uchar value, const QString &key)
{
    QString kind;
    QString device = deviceKeyOf(universe, &kind);
    if (device.isEmpty())
        return;
    // MIDI beat clock: far too many messages, and not a control
    if (kind == "midi" && (channel & 0x0FFF) >= 529)
        return;
    handleInput(kind, device, channel, kind == "osc" ? key : QString(), value);
}

bool ControlMapper::keyEvent(const QString &key, bool pressed)
{
    if (key.isEmpty())
        return false;
    bool used = m_pendingTarget.isEmpty() == false;
    if (used == false)
    {
        for (const QVariantMap &m : m_mappings)
            if (m.value("source") == "key" && m.value("key").toString().compare(key, Qt::CaseInsensitive) == 0)
                used = true;
    }
    if (used)
        handleInput("key", QString(), 0, key.toLower(), pressed ? 255 : 0);
    return used;
}

QString ControlMapper::keyName(int key)
{
    switch (key)
    {
        case Qt::Key_unknown:
        case Qt::Key_Escape:
        case Qt::Key_Shift:
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_AltGr:
        case Qt::Key_Meta:
        case Qt::Key_CapsLock:
        case Qt::Key_NumLock:
        case Qt::Key_ScrollLock:
            return QString();
        default:
            return QKeySequence(key).toString(QKeySequence::PortableText);
    }
}

bool ControlMapper::eventFilter(QObject *obj, QEvent *event)
{
    QEvent::Type type = event->type();
    if (type != QEvent::KeyPress && type != QEvent::KeyRelease && type != QEvent::ShortcutOverride)
        return false;

    QKeyEvent *ke = static_cast<QKeyEvent *>(event);
    if (ke->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))
        return false;
    // typing in a text field
    QObject *focus = QGuiApplication::focusObject();
    if (focus && (focus->inherits("QQuickTextInput") || focus->inherits("QQuickTextEdit")))
        return false;

    QString key = keyName(ke->key());
    if (key.isEmpty())
        return false;
    bool used = m_pendingTarget.isEmpty() == false;
    for (int i = 0; used == false && i < m_mappings.count(); i++)
        used = m_mappings.at(i).value("source") == "key" &&
               m_mappings.at(i).value("key").toString().compare(key, Qt::CaseInsensitive) == 0;
    if (used == false)
        return false;

    // a mapped key isn't a shortcut of the app
    if (type == QEvent::ShortcutOverride)
    {
        event->accept();
        return true;
    }
    // the window gets the key first, then its items: use it once
    if (obj->isWindowType() == false)
        return true;
    if (ke->isAutoRepeat() == false)
        keyEvent(key, type == QEvent::KeyPress);
    return true;
}

void ControlMapper::handleInput(const QString &source, const QString &device, quint32 channel,
                                const QString &key, uchar value)
{
    QString text = source == "midi" ? describeMidi(channel)
                 : source == "osc" ? key
                 : source == "key" ? tr("Key %1").arg(key.toUpper())
                 : QString("DMX %1").arg(channel + 1);
    log("in", device, text, source == "midi" ? midiValue(value) : int(value));
    m_activity++;
    emit activityChanged();

    // learning: the first control that moves (a press, for a button)
    if (m_pendingTarget.isEmpty() == false)
    {
        bool button = isButton(m_pendingTarget.value("target").toString());
        if (button && value == 0)
            return;

        QVariantMap props;
        props.insert("source", source);
        props.insert("device", device);
        props.insert("channel", channel);
        props.insert("key", key);

        // a known controller: its LED colours (green when on, blinking when paused)
        QString controller = source == "midi" ? controllerOf(device.section('|', 1)) : QString();
        if (controller.isEmpty() == false)
        {
            props.insert("controller", controller);
            if (button && m_learnSourceOf < 0)
            {
                // RGB pads take a colour (the velocity) and a behaviour (the
                // MIDI channel); the other buttons have one LED: 1 on, 2 blinking
                const int n = int(channel & 0x0FFF) - 128;     // the note (< 0: a CC)
                const int midiChannel = int(channel >> 12);
                int on = 1, onChannel = 0, paused = 2, pausedChannel = 0;
                if (controller == "apcmini2" && n >= 0 && n <= 63)
                {
                    // green at 100 %, pulsing
                    on = 21; onChannel = 7; paused = 21; pausedChannel = 10;
                }
                else if (controller == "apc40mk2" && midiChannel == 0 && ((n >= 0 && n <= 39) || (n >= 82 && n <= 86)))
                {
                    // the clip grid and the scene buttons: solid green, solid yellow
                    on = 21; onChannel = 1; paused = 13; pausedChannel = 1;
                }
                props.insert("onValue", on);
                props.insert("onChannel", onChannel);
                props.insert("pausedValue", paused);
                props.insert("pausedChannel", pausedChannel);
                props.insert("offValue", 0);
                props.insert("offChannel", 0);
            }
        }

        int id = m_learnSourceOf;
        if (id >= 0)
        {
            updateMapping(id, props);
        }
        else
        {
            QVariantMap m = m_pendingTarget;
            for (auto it = props.cbegin(); it != props.cend(); ++it)
                m.insert(it.key(), it.value());
            id = addMapping(m);
        }
        QVariantMap made = mapping(id);
        m_pressed.insert(id, value > 0);
        m_pendingTarget.clear();
        m_learnSourceOf = -1;
        emit learnChanged();
        emit learnt(describeSource(made) + "  →  " + describeTarget(made));
        return;
    }

    for (QVariantMap &m : m_mappings)
    {
        if (m.value("source").toString() != source)
            continue;
        QString mDevice = m.value("device").toString();
        // (keys come from no device)
        if (source != "key" && mDevice.isEmpty() == false && mDevice != device)
            continue;
        if (source == "osc" || source == "key")
        {
            if (m.value("key").toString().compare(key, Qt::CaseInsensitive) != 0)
                continue;
        }
        else if (m.value("channel").toUInt() != channel)
        {
            continue;
        }
        apply(m, value);
    }
}

/*********************************************************************
 * Driving the targets
 *********************************************************************/

Zone *ControlMapper::zoneOwning(quint32 functionId) const
{
    ZoneManager *zones = m_core->zones();
    for (int i = 0; i < zones->count(); i++)
        if (zones->zone(i)->functionIDs().contains(functionId))
            return zones->zone(i);
    return nullptr;
}

quint32 ControlMapper::functionOf(const QVariantMap &m) const
{
    if (m.value("target").toString() == "sceneIndex")
    {
        Zone *zone = m_core->zones()->zoneById(m.value("zoneId").toUInt());
        if (zone == nullptr)
            return Function::invalidId();
        return zone->functionIDs().value(m.value("index").toInt(), Function::invalidId());
    }
    return m.value("functionId").toUInt();
}

void ControlMapper::apply(QVariantMap &m, uchar value)
{
    int id = m.value("id").toInt();
    QString target = m.value("target").toString();

    if (isButton(target))
    {
        bool pressed = value > 0;
        bool was = m_pressed.value(id, false);
        m_pressed.insert(id, pressed);
        if (pressed != was)
            applyButton(m, pressed);
        return;
    }

    QString mode = m.value("mode").toString();
    qreal min, max;
    bool invert;
    rangeOf(m, min, max, invert);
    qreal step = qBound(1, m.value("step").toInt(), 100) / 100.0;
    bool loop = m.value("loop").toBool();

    if (mode == "absolute")
    {
        qreal f = value / 255.0;
        if (invert)
            f = 1.0 - f;
        const qreal wanted = min + (max - min) * f;
        if (m.value("pickup").toBool())
        {
            // pickup (a fader without a motor): it drives its target while
            // they agree; when something else moved the target (another
            // page, the screen) it must meet the target first
            const qreal current = faderValue(m);
            const qreal applied = m_applied.value(id, -1.0);
            const qreal previous = m_physical.value(id, -1.0);
            m_physical.insert(id, wanted);
            const bool inSync = applied >= 0.0 && qAbs(current - applied) < 0.01;
            const bool meets = qAbs(wanted - current) < 0.03 ||
                               (previous >= 0.0 && (previous - current) * (wanted - current) <= 0.0);
            if (inSync == false && meets == false)
            {
                // the fader moved away from what it last gave: a target
                // that shows that value again (paging back, the screen)
                // is not in sync with it any more
                m_applied.remove(id);
                return;
            }
            m_applied.insert(id, wanted);
        }
        applyFader(m, wanted);
        return;
    }

    qreal current = faderValue(m);
    qreal delta = 0;
    if (mode == "incremental")
    {
        // each press moves one step
        bool pressed = value > 0;
        bool was = m_pressed.value(id, false);
        m_pressed.insert(id, pressed);
        if (pressed == false || was)
            return;
        delta = invert ? -step : step;
    }
    else if (mode == "relative2c")
    {
        // relative encoders in two's complement (APC40 mkII): 1..63 up, 127..64 down
        int midi = midiValue(value);
        if (midi == 0)
            return;
        delta = (midi < 64 ? midi : midi - 128) * step / 5.0;
        if (invert)
            delta = -delta;
    }
    else
    {
        // relative encoders: 1..63 down, 65..127 up
        int midi = midiValue(value);
        if (midi == 64 || midi == 0)
            return;
        delta = (midi < 64 ? -(64 - midi) : midi - 64) * step / 5.0;
        if (invert)
            delta = -delta;
    }

    qreal next = current + delta;
    if (loop)
    {
        if (next > max)
            next = min;
        else if (next < min)
            next = max;
    }
    applyFader(m, qBound(min, next, max));
}

void ControlMapper::applyButton(const QVariantMap &m, bool pressed)
{
    QString target = m.value("target").toString();
    QString mode = m.value("mode").toString();
    ZoneManager *zones = m_core->zones();
    LiveMixer *mixer = m_core->mixer();
    Zone *zone = zones->zoneById(m.value("zoneId").toUInt());
    // (a deleted group is no group, not all of them)
    bool allZones = m.value("zoneId").toInt() == ALL_GROUPS;

    if (target.startsWith("playback"))
    {
        applyPlaybackButton(m, pressed);
        return;
    }

    // momentary actions: on the press
    if (target == "stopAll" || target == "tap" || target == "beatgo" ||
        target == "groupRelease" || target == "groupNext" || target == "groupPrevious")
    {
        if (pressed == false)
            return;
        if (target == "stopAll")
            m_core->stopAll();
        else if (target == "tap")
            m_core->tapTempo();
        else if (target == "beatgo")
            m_core->drive()->beatGo();
        else if (target == "groupRelease")
            allZones ? zones->stopAll() : (zone ? zone->stopAll() : void());
        else if (target == "groupNext")
            allZones ? zones->nextAll() : (zone ? zone->next() : void());
        else if (target == "groupPrevious")
            allZones ? zones->previousAll() : (zone ? zone->previous() : void());
        return;
    }

    // on/off actions: toggle, flash (while pressed), on, off
    bool state = buttonState(m) == 1;
    bool next = state;
    if (mode == "flash")
        next = pressed;
    else if (pressed == false)
        return;
    else if (mode == "on")
        next = true;
    else if (mode == "off")
        next = false;
    else
        next = !state;

    if (target == "scene" || target == "sceneIndex")
    {
        quint32 fid = functionOf(m);
        Function *f = m_core->doc()->function(fid);
        if (f == nullptr)
            return;
        Zone *owner = zoneOwning(fid);
        if (owner != nullptr)
        {
            // toggle uses the group, which knows the scenes that start paused
            if (mode == "toggle")
                owner->trigger(fid);
            else if (next && owner->isActive(fid) == false)
                owner->start(fid);
            else if (next == false && owner->isActive(fid))
                owner->stop(fid);
        }
        else if (next && f->isRunning() == false)
        {
            f->start(m_core->doc()->masterTimer(), FunctionParent::master());
        }
        else if (next == false && f->isRunning())
        {
            f->stop(FunctionParent::master());
        }
    }
    else if (target == "blackout")
    {
        m_core->setBlackout(next);
    }
    else if (target == "palette")
    {
        m_core->palettes()->setLive(m.value("paletteId").toInt(), m.value("groupId", -1).toInt(), next);
    }
    else if (target == "pauseAll")
    {
        zones->setPaused(next);
    }
    else if (target.startsWith("mixer"))
    {
        static const QHash<QString, QString> keys = {
            { "mixerFull", "full" }, { "mixerBlackout", "blackout" }, { "mixerFlash", "flash" },
            { "mixerFlashExclusive", "flashExclusive" }, { "mixerFullExclusive", "fullExclusive" },
            { "mixerStrobe", "strobe" } };
        mixer->set(m.value("mixerId").toInt(), keys.value(target), next);
    }
}

void ControlMapper::applyPlaybackButton(const QVariantMap &m, bool pressed)
{
    PlaybackManager *pb = m_core->playbacks();
    const QString target = m.value("target").toString();
    const int pbPage = m.value("pbPage", -1).toInt();
    const int slot = m.value("slot").toInt();
    // (a SHOW button on a cue of a cuelist: that cue, wherever it went)
    const int cue = cueIndexOf(m);
    // a function drives that function wherever it is (SHOW "GO of a cuelist")
    const int fid = isFunctionTarget(target) ? m.value("functionId", -1).toInt() : -1;

    if (target == "playbackFlash")
    {
        // on/off: flash (while pressed), toggle, on, off
        const QString mode = m.value("mode").toString();
        const bool flashing = playbackState(m) == 1;
        bool down = flashing;
        if (mode == "flash")
            down = pressed;
        else if (pressed == false)
            return;
        else if (mode == "on")
            down = true;
        else if (mode == "off")
            down = false;
        else
            down = !flashing;
        // (a press that changes nothing doesn't hold it once more)
        if (mode != "flash" && down == flashing)
            return;
        if (fid >= 0)
            pb->flashFunction(fid, down);
        else
            pb->flash(pbPage, slot, down);
        return;
    }

    // the rest are momentary: on the press
    if (pressed == false)
        return;
    if (target == "playbackGo")
        fid >= 0 ? pb->goFunction(fid) : pb->go(pbPage, slot);
    else if (target == "playbackBack")
        fid >= 0 ? pb->backFunction(fid) : pb->back(pbPage, slot);
    else if (target == "playbackPauseBack")
        fid >= 0 ? pb->pauseBackFunction(fid) : pb->pauseBack(pbPage, slot);
    else if (target == "playbackRelease")
        fid >= 0 ? pb->releaseFunction(fid) : pb->release(pbPage, slot);
    else if (target == "playbackGoTo")
        fid >= 0 ? pb->goToFunction(fid, cue) : pb->goTo(pbPage, slot, cue);
    else if (target == "playbackSelect")
        pb->select(pbPage, slot);
    else if (target == "playbackMainGo")
        pb->mainGo();
    else if (target == "playbackMainBack")
        pb->mainBack();
    else if (target == "playbackPage")
    {
        // (no such page: nothing)
        const int page = pb->resolvePage(pbPage);
        if (page >= 0)
            pb->setCurrentPage(page);
    }
    else if (target == "playbackPageNext")
        pb->nextPage();
    else if (target == "playbackPagePrevious")
        pb->previousPage();
    else if (target == "playbackReleaseAll")
        pb->releaseAll();
}

static QVariantMap mixerState(LiveMixer *mixer, int id)
{
    for (const QVariant &v : mixer->mixers())
        if (v.toMap().value("id").toInt() == id)
            return v.toMap();
    return QVariantMap();
}

int ControlMapper::buttonState(const QVariantMap &m) const
{
    QString target = m.value("target").toString();
    if (target.startsWith("playback"))
        return playbackState(m);
    if (target == "scene" || target == "sceneIndex")
    {
        quint32 fid = functionOf(m);
        Function *f = m_core->doc()->function(fid);
        if (f == nullptr)
            return -1;
        Zone *owner = zoneOwning(fid);
        bool active = owner != nullptr ? owner->isActive(fid) : f->isRunning();
        if (active && (f->isPaused() || (owner != nullptr && owner->isWaiting(fid))))
            return 2;
        return active ? 1 : 0;
    }
    if (target == "palette")
        return m_core->palettes()->isLive(m.value("paletteId").toInt(), m.value("groupId", -1).toInt()) ? 1 : 0;
    if (target == "blackout")
        return m_core->blackout() ? 1 : 0;
    if (target == "pauseAll")
        return m_core->zones()->paused() ? 1 : 0;
    if (target.startsWith("mixer"))
    {
        QVariantMap s = mixerState(m_core->mixer(), m.value("mixerId").toInt());
        static const QHash<QString, QString> keys = {
            { "mixerFull", "full" }, { "mixerBlackout", "blackout" }, { "mixerFlash", "flash" },
            { "mixerFlashExclusive", "flashExclusive" }, { "mixerFullExclusive", "fullExclusive" },
            { "mixerStrobe", "strobe" } };
        return s.value(keys.value(target)).toBool() ? 1 : 0;
    }
    if (target == "groupRelease" || target == "groupNext" || target == "groupPrevious")
    {
        Zone *zone = m_core->zones()->zoneById(m.value("zoneId").toUInt());
        return zone != nullptr && zone->activeCount() > 0 ? 1 : 0;
    }
    return 0;
}

int ControlMapper::playbackState(const QVariantMap &m) const
{
    PlaybackManager *pb = m_core->playbacks();
    const QString target = m.value("target").toString();

    // the pages (paging changes what page -1 is: the LEDs follow on the next tick)
    if (target == "playbackPage")
    {
        const int page = pb->resolvePage(m.value("pbPage").toInt());
        return page < 0 ? -1 : (page == pb->currentPage() ? 1 : 0);
    }
    if (target == "playbackPageNext")
        return pb->currentPage() < pb->pageCount() - 1 ? 1 : 0;
    if (target == "playbackPagePrevious")
        return pb->currentPage() > 0 ? 1 : 0;
    if (target == "playbackReleaseAll")
    {
        // something to release: a playback that plays, or any cuelist
        for (int page = 0; page < pb->pageCount(); page++)
            for (int slot = 0; slot < PlaybackManager::maxSlots(); slot++)
                if (pb->state(page, slot) >= 1)
                    return 1;
        for (Function *f : m_core->doc()->functionsByType(Function::CuelistType))
            if (f->stopped() == false)
                return 1;
        return 0;
    }
    if (isSlotTarget(target) == false && target != "playbackMainGo" && target != "playbackMainBack")
        return 0;

    // the slot of the mapping, the selected one (Main GO / Back), or a function
    int pbPage = m.value("pbPage", -1).toInt();
    int slot = m.value("slot").toInt();
    if (target == "playbackMainGo" || target == "playbackMainBack")
    {
        const QVariantMap selected = pb->selected();
        if (selected.isEmpty())
            return -1;
        pbPage = selected.value("page").toInt();
        slot = selected.value("slot").toInt();
    }
    const int fid = isFunctionTarget(target) ? m.value("functionId", -1).toInt() : -1;
    const QVariantMap status = fid >= 0 ? pb->functionStatus(fid) : QVariantMap();

    // -1 empty, 0 loaded, 1 on, 2 fading or waiting, 3 paused
    const int state = fid >= 0 ? status.value("state").toInt() : pb->state(pbPage, slot);
    if (state < 0)
        return -1;
    const bool running = state >= 1;
    auto currentCue = [&]() { return fid >= 0 ? status.value("cue").toInt() : pb->currentCue(pbPage, slot); };

    if (target == "playbackGo" || target == "playbackMainGo")
        return state == 3 ? 2 : state;
    if (target == "playbackBack" || target == "playbackMainBack")
        return running && currentCue() > 0 ? 1 : 0;
    if (target == "playbackPauseBack")
        return state == 3 ? 2 : (running ? 1 : 0);
    if (target == "playbackRelease")
        return running ? (state == 2 ? 2 : 1) : 0;
    if (target == "playbackFlash")
        return (fid >= 0 ? status.value("flashing").toBool() : pb->isFlashing(pbPage, slot)) ? 1 : 0;
    if (target == "playbackSelect")
        return pb->isSelected(pbPage, slot) ? 1 : 0;
    if (target == "playbackGoTo")
    {
        // on: the cue it plays, paused: the one the next GO plays, off: the
        // other cues, dark: no such cue (or not a cuelist)
        const int index = cueIndexOf(m);
        const int count = fid >= 0 ? status.value("cueCount").toInt() : pb->cueCount(pbPage, slot);
        if (index < 0 || index >= count)
            return -1;
        if (running && index == currentCue())
            return 1;
        const int next = fid >= 0 ? status.value("next").toInt() : pb->nextCue(pbPage, slot);
        return index == next ? 2 : 0;
    }
    // (the fader has no LED state)
    return 0;
}

qreal ControlMapper::faderValue(const QVariantMap &m) const
{
    QString target = m.value("target").toString();
    Zone *zone = m_core->zones()->zoneById(m.value("zoneId").toUInt());
    quint32 fid = m.value("functionId").toUInt();

    if (target == "grandMaster")
        return m_core->grandMaster() / 255.0;
    if (target == "groupMaster")
        return zone != nullptr ? zone->master() : 0.0;
    if (target == "sceneDimmer" || target == "sceneSpeed")
    {
        Zone *owner = zoneOwning(fid);
        if (owner == nullptr)
            return target == "sceneDimmer" ? 1.0 : 0.5;
        QVariantMap opts = owner->options(fid);
        if (target == "sceneDimmer")
            return opts.value("dimmer", 1.0).toDouble();
        // speed x0.1 .. x10 on a logarithmic fader, x1 in the middle
        return (std::log10(opts.value("speed", 1.0).toDouble()) + 1.0) / 2.0;
    }
    if (target == "bpm")
        return (m_core->bpm() - 20) / 280.0;
    if (target == "playbackFader")
        return m_core->playbacks()->level(m.value("pbPage", -1).toInt(), m.value("slot").toInt());
    if (target == "surfaceFader")
    {
        QVariantMap item = surfaceFaderItem(m);
        return item.isEmpty() ? 0.0 : m_core->surface()->level(item);
    }
    if (target.startsWith("mixer"))
    {
        QVariantMap s = mixerState(m_core->mixer(), m.value("mixerId").toInt());
        if (target == "mixerDimmer")
            return s.value("dimmer", 1.0).toDouble();
        if (target == "mixerHue")
            return s.value("hue").toDouble();
        if (target == "mixerSaturation")
            return s.value("saturation", 1.0).toDouble();
        if (target == "mixerStrobeHz")
            return (s.value("strobeHz", 8.0).toDouble() - 0.5) / 19.5;
    }
    return 0.0;
}

void ControlMapper::applyFader(const QVariantMap &m, qreal value)
{
    value = qBound(0.0, value, 1.0);
    QString target = m.value("target").toString();
    Zone *zone = m_core->zones()->zoneById(m.value("zoneId").toUInt());
    quint32 fid = m.value("functionId").toUInt();
    int mixerId = m.value("mixerId").toInt();

    if (target == "grandMaster")
        m_core->setGrandMaster(qRound(value * 255));
    else if (target == "groupMaster" && zone != nullptr)
        zone->setMaster(value);
    else if (target == "sceneDimmer" || target == "sceneSpeed")
    {
        if (Zone *owner = zoneOwning(fid))
        {
            if (target == "sceneDimmer")
                owner->setOption(fid, "dimmer", value);
            else
                owner->setOption(fid, "speed", std::pow(10.0, value * 2.0 - 1.0));
        }
    }
    else if (target == "bpm")
        m_core->setBpm(qRound(20 + value * 280));
    else if (target == "playbackFader")
        m_core->playbacks()->setLevel(m.value("pbPage", -1).toInt(), m.value("slot").toInt(), value);
    else if (target == "mixerDimmer")
        m_core->mixer()->set(mixerId, "dimmer", value);
    else if (target == "mixerHue")
        m_core->mixer()->set(mixerId, "hue", value);
    else if (target == "mixerSaturation")
        m_core->mixer()->set(mixerId, "saturation", value);
    else if (target == "mixerStrobeHz")
        m_core->mixer()->set(mixerId, "strobeHz", 0.5 + value * 19.5);
    else if (target == "surfaceFader")
    {
        QVariantMap item = surfaceFaderItem(m);
        if (item.isEmpty() == false)
            m_core->surface()->setLevel(item, value);
    }
}

/*********************************************************************
 * Feedback
 *********************************************************************/

void ControlMapper::slotFeedback()
{
    for (const QVariantMap &m : std::as_const(m_mappings))
    {
        QString source = m.value("source").toString();
        if (m.value("feedback").toBool() == false || (source != "midi" && source != "osc"))
            continue;

        // what it shows: the state of a button (with the MIDI channel of the
        // state, 0 for the one of the control), the position of a fader
        int value = 0;
        int stateChannel = 0;
        if (isButton(m.value("target").toString()))
        {
            int state = buttonState(m);
            QString key = state == 1 ? "on" : state == 2 ? "paused" : "off";
            // no scene there: the LED is off
            int midi = state < 0 ? 0 : m.value(key + "Value").toInt();
            stateChannel = state < 0 ? 0 : qBound(0, m.value(key + "Channel").toInt(), 16);
            value = dmxFromMidi(midi);
        }
        else
        {
            // (From above To: inverted, as the fader drives it)
            qreal min, max;
            bool invert;
            rangeOf(m, min, max, invert);
            qreal f = max > min ? (faderValue(m) - min) / (max - min) : 0.0;
            if (invert)
                f = 1.0 - f;
            value = qRound(qBound(0.0, f, 1.0) * 255);
        }

        int id = m.value("id").toInt();
        int sent = value | (stateChannel << 16);
        if (m_sentFeedback.value(id, -1) == sent)
            continue;
        m_sentFeedback.insert(id, sent);

        // the MIDI channel is in the top bits of the QLC+ channel
        quint32 channel = m.value("channel").toUInt();
        if (source == "midi" && stateChannel > 0)
            channel = (quint32(stateChannel - 1) << 12) | (channel & 0x0FFF);

        sendToDevices(m, channel, value);
    }
}

void ControlMapper::sendToDevices(const QVariantMap &m, quint32 channel, int value, const QString &onlyDevice)
{
    InputOutputMap *io = m_core->doc()->inputOutputMap();
    QString source = m.value("source").toString();

    // to its device, or to every device of its kind
    QString device = m.value("device").toString();
    for (const QVariant &v : devices())
    {
        QVariantMap d = v.toMap();
        QString key = d.value("key").toString();
        if (d.value("feedback").toBool() == false || d.value("kind").toString() != source)
            continue;
        if ((device.isEmpty() == false && key != device) || (onlyDevice.isEmpty() == false && key != onlyDevice))
            continue;
        quint32 universe = d.value("universe").toUInt();
        QVariant params = source == "osc" ? QVariant(m.value("key").toString()) : QVariant();
        io->sendFeedBack(universe, channel, uchar(value), params);
        log("out", key, source == "midi" ? describeMidi(channel) : m.value("key").toString(),
            source == "midi" ? midiValue(uchar(value)) : value);
    }
}

void ControlMapper::lightOff(const QVariantMap &m, const QString &onlyDevice)
{
    QString source = m.value("source").toString();
    if (m.value("feedback").toBool() == false || (source != "midi" && source != "osc"))
        return;
    // a fader with a motor stays where it is
    if (isButton(m.value("target").toString()) == false)
        return;

    // 0 on the channel of the control: a note off, the LED goes dark
    quint32 channel = m.value("channel").toUInt();
    sendToDevices(m, channel, 0, onlyDevice);
    // and on the MIDI channel of the last state, if it had its own (APC mini mk2)
    int id = m.value("id").toInt();
    int stateChannel = m_sentFeedback.value(id, 0) >> 16;
    if (source == "midi" && stateChannel > 0)
    {
        quint32 lit = (quint32(stateChannel - 1) << 12) | (channel & 0x0FFF);
        if (lit != channel)
            sendToDevices(m, lit, 0, onlyDevice);
    }
    m_sentFeedback.remove(id);
}

void ControlMapper::lightsOff()
{
    for (const QVariantMap &m : std::as_const(m_mappings))
        lightOff(m);
    m_sentFeedback.clear();
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool ControlMapper::saveXML(QXmlStreamWriter *doc) const
{
    doc->writeStartElement(KXMLDTCMappings);
    for (const QVariantMap &m : m_mappings)
    {
        doc->writeStartElement(KXMLDTCMapping);
        for (auto it = m.cbegin(); it != m.cend(); ++it)
        {
            QVariant v = it.value();
            doc->writeAttribute(it.key(), v.typeId() == QMetaType::Bool ? (v.toBool() ? "true" : "false")
                                                                        : v.toString());
        }
        doc->writeEndElement();
    }
    doc->writeEndElement();
    return true;
}

bool ControlMapper::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLDTCMappings)
        return false;

    // (a control being learnt was for the show before: its IDs are reused)
    dropPendingLearn();
    m_mappings.clear();
    m_pressed.clear();
    m_applied.clear();
    m_physical.clear();
    m_lastId = 0;
    QSet<int> ids;
    QList<int> renumbered;
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLDTCMapping)
        {
            const QVariantMap defaults = defaultsFor(root.attributes().value("target").toString());
            QVariantMap m = defaults;
            for (const QXmlStreamAttribute &attr : root.attributes())
            {
                QString key = attr.name().toString();
                QString text = attr.value().toString();
                bool ok = false;
                int number = text.toInt(&ok);
                if (key == "device" || key == "key" || key == "source" || key == "target" || key == "mode")
                    m.insert(key, text);
                else if (text == "true" || text == "false")
                    m.insert(key, text == "true");
                else if (key == "channel")
                    m.insert(key, text.toUInt());
                else if (ok)
                    m.insert(key, number);
                // a key that is a number, with a wrong one (a damaged file),
                // keeps its default; what this version doesn't know stays as it is
                else if (defaults.contains(key) == false || defaults.value(key).typeId() == QMetaType::QString)
                    m.insert(key, text);
            }
            normalizeMapping(m);
            // each mapping is edited and deleted by its ID: one without a
            // valid one, or with the ID of another (a damaged file), gets a
            // new one once they are all read
            bool idOk = false;
            const int id = m.value("id").toInt(&idOk);
            if (idOk && id > 0 && id < INT_MAX / 2 && ids.contains(id) == false)
            {
                ids.insert(id);
                m_lastId = qMax(m_lastId, id);
            }
            else
            {
                renumbered.append(int(m_mappings.count()));
            }
            m_mappings.append(m);
        }
        root.skipCurrentElement();
    }
    for (int index : std::as_const(renumbered))
        m_mappings[index].insert("id", ++m_lastId);

    // a mapping on a function that isn't there drives nothing, as when it
    // is deleted (a function made later gets its ID)
    QSet<quint32> missing;
    for (const QVariantMap &m : std::as_const(m_mappings))
    {
        const int fid = m.value("functionId", -1).toInt();
        if (fid >= 0 && m_core->doc()->function(quint32(fid)) == nullptr)
            missing.insert(quint32(fid));
    }
    for (quint32 fid : std::as_const(missing))
        slotFunctionRemoved(fid);

    m_sentFeedback.clear();
    emit mappingsChanged();
    return true;
}

/*********************************************************************
 * Targets without a mapping
 *********************************************************************/

void ControlMapper::pressTarget(const QVariantMap &target, bool pressed)
{
    QVariantMap m = defaultsFor(target.value("target").toString());
    for (auto it = target.cbegin(); it != target.cend(); ++it)
        m.insert(it.key(), it.value());
    applyButton(m, pressed);
}

void ControlMapper::setTargetLevel(const QVariantMap &target, qreal level)
{
    QVariantMap m = defaultsFor(target.value("target").toString());
    for (auto it = target.cbegin(); it != target.cend(); ++it)
        m.insert(it.key(), it.value());
    applyFader(m, qBound(0.0, level, 1.0));
}

int ControlMapper::targetState(const QVariantMap &target) const
{
    QVariantMap m = defaultsFor(target.value("target").toString());
    for (auto it = target.cbegin(); it != target.cend(); ++it)
        m.insert(it.key(), it.value());
    return buttonState(m);
}

qreal ControlMapper::targetLevel(const QVariantMap &target) const
{
    QVariantMap m = defaultsFor(target.value("target").toString());
    for (auto it = target.cbegin(); it != target.cend(); ++it)
        m.insert(it.key(), it.value());
    return faderValue(m);
}

int ControlMapper::cueIndexOf(const QVariantMap &target) const
{
    // a cue of a cuelist is bound to that cue (uids stay when cues are
    // recorded, deleted or renumbered around it); a slot or an older
    // button without it, to its place in the list
    const quint32 uid = target.value("cueUid", 0).toUInt();
    const int fid = isFunctionTarget(target.value("target").toString()) ? target.value("functionId", -1).toInt() : -1;
    if (uid == 0 || fid < 0)
        return target.value("cue").toInt();
    const Cuelist *cuelist = qobject_cast<const Cuelist *>(m_core->doc()->function(quint32(fid)));
    return cuelist != nullptr ? cuelist->indexOfUid(uid) : -1;
}
