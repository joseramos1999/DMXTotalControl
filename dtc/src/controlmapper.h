/*
  DMX Total Control
  controlmapper.h

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

#ifndef CONTROLMAPPER_H
#define CONTROLMAPPER_H

#include <QObject>
#include <QVariant>
#include <QHash>
#include <QList>

#include <QModelIndex>

class QXmlStreamReader;
class QXmlStreamWriter;
class QTimer;
class DtcCore;
class Zone;

#define KXMLDTCMappings QStringLiteral("Mappings")

/**
 * External control through mappings: MIDI, OSC, DMX input and
 * the keyboard drive scenes, groups, masters, the mixer, blackout, tap...
 *
 * Devices are the input lines of the QLC+ plugins: enabling one patches it
 * as the input of a universe, and its MIDI output as the feedback of the same
 * universe (LEDs of controllers like the Akai APC mini).
 *
 * A mapping is a map:
 *   source:   "midi", "osc", "dmx", "key" (keyboard)
 *   device:   the device key ("plugin|line name"), "" for any device
 *   channel:  the QLC+ input channel (MIDI: CC 0-127, notes 128-255, MIDI
 *             channel in the 4 top bits), or key: the OSC path / keyboard key
 *   target:   what it drives (see targets()), with functionId, zoneId,
 *             index, mixerId when it needs them; the playbacks: pbPage (-1
 *             follows the current playback page), slot and cue (0-based), or
 *             a functionId (>= 0) that acts on that function wherever it is
 *             (with cueUid, a go to a cue is bound to that cue of the list).
 *             zoneId -1 is "all groups" (release, next, previous), -2 a
 *             deleted group (nothing); a deleted function leaves functionId
 *             -1 (and slot -1 on the playbacks: nothing); a SHOW fader
 *             (surfaceFader) is found by its section, wherever its page went
 *   mode:     buttons "toggle", "flash", "on", "off";
 *             faders "absolute", "incremental", "relative" (encoders, 64 is
 *             still), "relative2c" (encoders, two's complement: 1 up, 127 down)
 *   min, max: range in % (faders; min above max is an inverted range), invert,
 *             step (%), loop
 *   pickup:   an absolute fader without a motor takes over only when it meets
 *             the level of its target (after a page change, or a change from
 *             the screen), so nothing jumps
 *   feedback: send the state back; onValue, offValue, pausedValue (0-127)
 */
class ControlMapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(QVariantList mappings READ mappings NOTIFY mappingsChanged)
    /** Changes with the mappings, without describing them all as mappings
     *  does (the badges of learn mode, on hundreds of controls, count them) */
    Q_PROPERTY(int revision READ revision NOTIFY mappingsChanged)
    /** The last messages in and out: { time, direction, device, text, value } */
    Q_PROPERTY(QVariantList monitor READ monitor NOTIFY monitorChanged)
    /** Learn mode: the mappable controls of the UI can be picked */
    Q_PROPERTY(bool learnMode READ learnMode WRITE setLearnMode NOTIFY learnChanged)
    /** The target waiting for a control to move, or empty */
    Q_PROPERTY(QVariantMap pendingTarget READ pendingTarget NOTIFY learnChanged)
    /** Changes on every input message (activity indicator) */
    Q_PROPERTY(int activity READ activity NOTIFY activityChanged)

public:
    ControlMapper(DtcCore *core, QObject *parent = nullptr);
    ~ControlMapper();

    /*********************************************************************
     * Devices
     *********************************************************************/
    /** [{ key, plugin, name, kind, enabled, feedback, canFeedback, universe, group }] */
    QVariantList devices() const;
    /** The devices in the order of the page: first the controllers (MIDI,
     *  OSC) and every device enabled (group "controllers"), then the other
     *  DMX inputs, a line for each network card (group "dmx"). The rest of
     *  the order is kept */
    static QVariantList sortDevices(const QVariantList &devices);
    Q_INVOKABLE void setDeviceEnabled(const QString &key, bool enabled);
    Q_INVOKABLE void setDeviceFeedback(const QString &key, bool enabled);
    Q_INVOKABLE void refreshDevices();

    /** The kind of controller of a device, from its name: "apcmini" (Akai
     *  APC mini), "apcmini2" (APC mini mk2), "apc40mk2" (APC40 mkII) or "".
     *  The second port of the mk2 (Note mode) and the first APC40 (another
     *  protocol) are not controllers */
    static QString controllerOf(const QString &deviceName);

    /**
     * Map a whole controller in one go, replacing the mappings of the device.
     *
     * Layout "groups", Akai APC mini and mk2: a column of pads per group (its
     * scenes from the top, green when playing, yellow when ready), the faders
     * on the group masters and the last one on the grand master, the buttons
     * under the pads release the groups, the buttons on the right: blackout,
     * stop all, pause, tap, BeatGO, previous, next and flash.
     *
     * Layout "playbacks", the playbacks 1-8 of the current playback page:
     * - APC mini and mk2: a column per playback, its fader (pickup), from the
     *   bottom GO, Pause/Back, Flash and its first 5 cues (cue 1 on top), the
     *   button under the pads releases it; the last fader is the grand master,
     *   the buttons on the right: previous and next page, GO and Back of the
     *   selected playback, blackout, tap, BeatGO and stop all.
     * - APC40 mkII: a track per playback (fader, activator GO, solo
     *   Pause/Back, record arm Flash, track select, clip stop release), the
     *   clip grid on their first 5 cues, the scene buttons on pages 1-5, the
     *   bank buttons on the pages and the groups, Play / Record GO and Back of
     *   the selected playback, the device knobs on playbacks 9-16 and the
     *   track knobs on the masters of the first 8 groups.
     * @return the number of mappings made (0: no such layout for that controller)
     */
    Q_INVOKABLE int applyTemplate(const QString &deviceKey, const QString &controller,
                                  const QString &layout = QStringLiteral("groups"));

public slots:
    /**
     * Get the known controllers with LEDs ready: the APCs ignore note offs
     * (no MIDISendNoteOff), the APC40 mkII goes into Ableton mode (the QLC+
     * MIDI template, sent when its output opens again) and its knob rings show
     * a single dot. At the start, after opening a show (it patches the
     * universes again), when the LEDs of a device are switched on and after a
     * template; the LEDs are all sent again.
     */
    void initControllers();

public:
    /*********************************************************************
     * Mappings
     *********************************************************************/
    QVariantList mappings() const;
    int revision() const;
    /** The targets that can be mapped: [{ target, kind: "button"|"fader", name, needs }] */
    Q_INVOKABLE QVariantList targets() const;

    Q_INVOKABLE int addMapping(const QVariantMap &mapping);
    Q_INVOKABLE void updateMapping(int id, const QVariantMap &props);
    Q_INVOKABLE void removeMapping(int id);
    Q_INVOKABLE int duplicateMapping(int id);
    Q_INVOKABLE QVariantMap mapping(int id) const;
    /** Human readable source and target of a mapping */
    Q_INVOKABLE QString describeSource(const QVariantMap &mapping) const;
    Q_INVOKABLE QString describeTarget(const QVariantMap &mapping) const;
    /** The mappings driving a target (badges in learn mode) */
    Q_INVOKABLE int countFor(const QVariantMap &target) const;
    /** Whether a target is a button (else a fader) */
    Q_INVOKABLE bool isButton(const QString &target) const;

    /** Drive a target directly, without a mapping (the SHOW): a button
     *  target is pressed or released (its "mode" as in a mapping: toggle
     *  by default), a fader target gets a level 0..1 */
    Q_INVOKABLE void pressTarget(const QVariantMap &target, bool pressed);
    Q_INVOKABLE void setTargetLevel(const QVariantMap &target, qreal level);
    /** -1: nothing there, 0 off, 1 on, 2 paused */
    Q_INVOKABLE int targetState(const QVariantMap &target) const;
    Q_INVOKABLE qreal targetLevel(const QVariantMap &target) const;
    /** The cue index a "go to a cue" target plays: by "cueUid" when it has
     *  one (a SHOW button on a cue of a cuelist: cues recorded, deleted or
     *  renumbered before it don't move it to another cue; -1 once that cue
     *  is deleted), else "cue" (positional, the cue grid of the APCs) */
    Q_INVOKABLE int cueIndexOf(const QVariantMap &target) const;

    /** Playback pages moved or one was deleted (PlaybackManager::pagesRenumbered):
     *  a target bound to a slot of a page (pbPage >= 0) follows its page; when
     *  the page is deleted, a GO, Back, Pause/Back, Release, go to a cue or
     *  Flash drives the function that was there (it keeps playing), anything
     *  else is bound to nothing (slot -1). Page targets ("page 2", the APC
     *  scene buttons) stay where they are. @return whether $target changed */
    static bool followPlaybackPages(QVariantMap &target, const QList<int> &newIndexes,
                                    const QHash<int, quint32> &removedSlots);

    /** The range of a fader ("min", "max": 0-100 %, "invert") as 0..1, the
     *  whole range when they are missing. From above To is an inverted range,
     *  as with Invert. The faders of the SHOW have one too */
    static void rangeOf(const QVariantMap &m, qreal &min, qreal &max, bool &invert);

    /** The MIDI channel (1-based) of a state of the LED of a button ("on",
     *  "off", "paused") when the mapping has none of its own (its
     *  "<state>Channel" is 0). 0: the channel of the control. The RGB pads of
     *  the APC mini mk2 light "on" full (7) and "off" and "paused" dim (1) */
    Q_INVOKABLE static int defaultLedChannel(const QVariantMap &mapping, const QString &state);
    /** What the feedback of mapping $m sends now (also used by the tests):
     *  $value 0-255 on the QLC+ $channel (the MIDI channel of the state of
     *  an LED in its top bits), $stateChannel that MIDI channel (1-based,
     *  0: the one of the control) */
    void feedbackOf(const QVariantMap &m, int &value, quint32 &channel, int &stateChannel) const;

    /** Forget every mapping (their LEDs go off) */
    void clear();

    /** Turn off the LEDs of every button mapping, e.g. when the
     *  application closes: the controller doesn't stay lit */
    void lightsOff();
    /** No more feedback to the controllers (the application closes: their
     *  LEDs go off after, nothing else is sent to them) */
    void stopFeedback();

    /*********************************************************************
     * Learn
     *********************************************************************/
    bool learnMode() const;
    void setLearnMode(bool enable);
    QVariantMap pendingTarget() const;

    /** The next control that moves drives $target (a new mapping) */
    Q_INVOKABLE void startLearn(const QVariantMap &target);
    /** The next control that moves becomes the source of mapping $id */
    Q_INVOKABLE void learnSource(int id);
    Q_INVOKABLE void cancelLearn();

    /*********************************************************************
     * Input
     *********************************************************************/
    /** A keyboard key from the UI. @return true when a mapping used it */
    Q_INVOKABLE bool keyEvent(const QString &key, bool pressed);

    /** The keyboard of the whole app: install it on the application. Mapped
     *  keys (and any key while learning) don't reach the shortcuts. Keys go
     *  to text fields as usual, and Ctrl/Alt and Esc are never used */
    bool eventFilter(QObject *obj, QEvent *event) override;
    /** The name of a key for the mappings: "Q", "1", "Space", "F10" */
    static QString keyName(int key);

    /** An input message (also used by the tests) */
    void handleInput(const QString &source, const QString &device, quint32 channel,
                     const QString &key, uchar value);

    /** The last messages in and out, newest first: { time, direction, device,
     *  text, value, serial } (serial: a number that grows with each message) */
    QVariantList monitor() const;
    Q_INVOKABLE void clearMonitor();
    int activity() const;

    /** Describe a QLC+ MIDI input channel: "CC 7 · ch 1", "Note 60 · ch 10" */
    static QString describeMidi(quint32 channel);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc) const;

signals:
    void devicesChanged();
    void mappingsChanged();
    void monitorChanged();
    void learnChanged();
    void activityChanged();
    /** A mapping was learnt (for a message in the UI) */
    void learnt(const QString &description);

private slots:
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value, const QString &key);
    /** A universe added or removed: the devices */
    void slotUniversesChanged();
    void slotFeedback();
    void slotPlaybackPagesRenumbered(const QList<int> &newIndexes, const QHash<int, quint32> &removedSlots);
    /** The LEDs of the device of a universe that goes are turned off first */
    void slotUniverseAboutToBeRemoved(int universe);
    /** The mappings of a deleted function or group drive nothing */
    void slotFunctionRemoved(quint32 functionId);
    void slotZonesAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    /** The SHOW faders mapped follow their pages */
    void slotSurfacePagesChanged();
    /** What the mappings drive was renamed (a scene, group, palette...):
     *  their descriptions (targetText) are read again, once for a burst */
    void slotNamesChanged();
    /** New groups: their names are followed too */
    void slotZonesChanged();

private:
    QString deviceKeyOf(quint32 universe, QString *kind = nullptr) const;
    int universeOf(const QString &deviceKey) const;
    void log(const QString &direction, const QString &device, const QString &text, int value);
    /** Forget the control being learnt, if any (its mapping or its show went) */
    void dropPendingLearn();
    /** The page of the SHOW fader of a "surfaceFader" mapping (-1: gone) and the fader */
    int surfacePageOf(const QVariantMap &m) const;
    QVariantMap surfaceFaderItem(const QVariantMap &m) const;

    /** Apply a message to a mapping */
    void apply(QVariantMap &m, uchar value);
    void applyButton(const QVariantMap &m, bool pressed);
    void applyFader(const QVariantMap &m, qreal value);
    /** The current value of a fader target, 0..1 */
    qreal faderValue(const QVariantMap &m) const;
    /** The feedback state of a button target: 0 off, 1 on, 2 paused */
    int buttonState(const QVariantMap &m) const;
    /** The same for the playback targets (-1 dark: an empty slot, no such cue or page) */
    int playbackState(const QVariantMap &m) const;
    void applyPlaybackButton(const QVariantMap &m, bool pressed);
    quint32 functionOf(const QVariantMap &m) const;
    Zone *zoneOwning(quint32 functionId) const;
    /** A scene of no group a fader of the SHOW holds up: only the fader stops it */
    bool heldByFader(quint32 functionId) const;
    QVariantMap defaultsFor(const QString &target) const;

    /** Send a feedback value of a mapping to its device(s), or only to $onlyDevice */
    void sendToDevices(const QVariantMap &m, quint32 channel, int value, const QString &onlyDevice = QString());
    /** Turn off the LED of a button mapping (motorised faders stay where they are) */
    void lightOff(const QVariantMap &m, const QString &onlyDevice = QString());

private:
    DtcCore *m_core;
    QList<QVariantMap> m_mappings;
    int m_lastId;

    bool m_learnMode;
    QVariantMap m_pendingTarget;
    int m_learnSourceOf;

    QList<QVariantMap> m_monitor;
    /** Numbers the messages of the monitor ("serial"): the list adds the new ones only */
    int m_monitorSerial;
    int m_activity;

    QTimer *m_feedbackTimer;
    /** Last value sent back, by mapping ID */
    QHash<int, int> m_sentFeedback;
    /** Last state of the buttons (edges), by mapping ID */
    QHash<int, bool> m_pressed;
    /** Pickup, by mapping ID: the last level a fader gave its target (only
     *  while the fader is still there), and where the fader was (even when
     *  it didn't drive it) */
    QHash<int, qreal> m_applied;
    QHash<int, qreal> m_physical;
    /** Whether each target is a button (isButton runs for every mapping on every feedback tick) */
    mutable QHash<QString, bool> m_buttonKinds;
    /** The name of each target (describeTarget, for every mapping), in the
     *  language of the interface */
    mutable QHash<QString, QString> m_targetNames;
    /** slotNamesChanged() already told the list to read the mappings again */
    bool m_namesPending;
    /** Counts the changes of the mappings (revision) */
    int m_revision;
};

#endif
