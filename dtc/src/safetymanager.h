/*
  DMX Total Control
  safetymanager.h

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

#ifndef SAFETYMANAGER_H
#define SAFETYMANAGER_H

#include <QObject>
#include <QVariant>
#include <QDateTime>
#include <QElapsedTimer>
#include <QSharedPointer>
#include <QHash>
#include <QList>
#include <QMap>
#include <QPair>
#include <QVector>

struct OutputLayer;
class PatchManager;
class AppSettings;
class Programmer;
class LiveMixer;
class Fixture;
class QTimer;
class QXmlStreamReader;
class QXmlStreamWriter;
class QXmlStreamAttributes;
class Doc;

/** The parked channels, in the section of DMX Total Control of the show */
#define KXMLDTCSafety QStringLiteral("Safety")
#define KXMLDTCSafetyPark QStringLiteral("Park")
#define KXMLDTCSafetyShowStop QStringLiteral("ShowStop")

/**
 * Safety during the show (Parte 1): the protected special effects. What the
 * output of each universe does above every source is a layer of its own
 * (OutputLayer), built here from the patch and given to the universes: the
 * channels of the protected fixtures (PatchManager::effectRule) go out only
 * while the effects are armed, from scenes, cuelists, chasers and super
 * scenes, each fire cut at its maximum time.
 *
 * The effects are armed by hand (arm), never saved armed, and disarm by
 * themselves: Stop all, Blackout, another show, the end of the program, and
 * some minutes without a fire (Options). Every fire is written to a log of
 * the session, in the data folder of the application.
 *
 * Parked channels (safetyparks.cpp): a fixed value per channel at the
 * output, above the scenes, the FX, the programmer, the mixer, the grand
 * master and Stop all. Blackout still darkens a parked intensity, unless it
 * is parked also in blackout. A protected special effect can only be parked
 * off: parked, it never fires. Saved with the show.
 *
 * The show stop (safetyshowstop.cpp): a safety light above everything but
 * the gate (the programmer, the grand master, Blackout, the parks), put and
 * taken away by holding a control for a second: the special effects
 * disarmed (and not armed again while it lasts), the lights chosen at open
 * white, strobes and lasers dark, every shutter open and every head where it
 * is. The functions go on underneath. Its fixtures and level are saved with
 * the show; it is never saved on.
 *
 * The fade to black (safetyfade.cpp): the light goes down at the output in
 * its time (the intensities, not the special effects nor the parks that stay
 * in blackout), then Blackout comes and the light is full again under it:
 * Blackout off brings it back. It disarms the special effects when it
 * starts; pressed again, or Blackout, ends it at once.
 */
class SafetyManager : public QObject
{
    Q_OBJECT

    /** Some fixture of the show is a protected special effect */
    Q_PROPERTY(bool hasProtected READ hasProtected NOTIFY protectionChanged)
    Q_PROPERTY(bool armed READ armed NOTIFY armedChanged)
    /** Seconds until the effects disarm by themselves (no fire), -1: never
     *  (or not armed) */
    Q_PROPERTY(int disarmIn READ disarmIn NOTIFY disarmInChanged)
    /** The fixtures that fire now: [{ id, name, number }] */
    Q_PROPERTY(QVariantList firing READ firing NOTIFY firingChanged)
    /** The ones that wait for their source to let go before they can fire
     *  again: [{ id, name, number, reason: "release" (cut, disarmed, held
     *  when armed) or "wait" (pressed during its minimum wait) }] */
    Q_PROPERTY(QVariantList waiting READ waiting NOTIFY waitingChanged)
    /** The last fires, newest first (10 at most): [{ id, name, number,
     *  time ("HH:mm:ss"), ms, reason ("released", "time", "disarmed",
     *  "showstop", "removed", "notallowed", "parked"), reasonText }] */
    Q_PROPERTY(QVariantList recentFires READ recentFires NOTIFY recentFiresChanged)
    /** The log of this session, once it has something (empty before) */
    Q_PROPERTY(QString logFile READ logFile NOTIFY recentFiresChanged)
    /** Changes with each new layer and each fire: what shows them reads again */
    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged)
    /** The fixtures with parked channels: { "id": how many } */
    Q_PROPERTY(QVariantMap parkedFixtures READ parkedFixtures NOTIFY parksChanged)
    /** How many fixtures have parked channels */
    Q_PROPERTY(int parkedCount READ parkedCount NOTIFY parksChanged)
    /** The show stop is on */
    Q_PROPERTY(bool showStop READ showStop NOTIFY showStopChanged)
    /** A control holds it down now (it toggles after holdMs) */
    Q_PROPERTY(bool holding READ holding NOTIFY holdingChanged)
    /** How long it must be held, ms */
    Q_PROPERTY(int holdMs READ holdMs CONSTANT)
    /** The Pause key was pressed once: pressed again within keyWindowMs it
     *  puts (or takes away) the show stop */
    Q_PROPERTY(bool keyPending READ keyPending NOTIFY keyPendingChanged)
    /** How long the second press of Pause is waited for, ms */
    Q_PROPERTY(int keyWindowMs READ keyWindowMs CONSTANT)
    /** The level of its light, 0-100 % (100) */
    Q_PROPERTY(int showStopLevel READ showStopLevel WRITE setShowStopLevel NOTIFY showStopSettingsChanged)
    /** The fixture groups it lights (IDs; empty: every light) */
    Q_PROPERTY(QVariantList showStopGroups READ showStopGroups WRITE setShowStopGroups NOTIFY showStopSettingsChanged)
    /** A fade to black runs (Blackout comes at its end) */
    Q_PROPERTY(bool fadingToBlack READ fadingToBlack NOTIFY fadeToBlackChanged)
    /** The time of the one that runs, ms (0: none) */
    Q_PROPERTY(int fadeToBlackMs READ fadeToBlackMs NOTIFY fadeToBlackChanged)
    // (the time a new fade to black has: a SHOW button, a mapping)
    Q_PROPERTY(int defaultFadeToBlackMs READ defaultFadeToBlackMs CONSTANT)

public:
    SafetyManager(Doc *doc, PatchManager *patch, AppSettings *settings, QObject *parent = nullptr);
    ~SafetyManager();

    bool hasProtected() const;
    bool armed() const;
    int disarmIn() const;
    QVariantList firing() const;
    QVariantList waiting() const;
    QVariantList recentFires() const;
    QString logFile() const;
    int revision() const;

    /** Arm the special effects: refused (false, with a message) when no
     *  fixture is protected, Blackout or the show stop is on; refused
     *  (false) once the program closes (disarmAndFlush("exit")). Each arming
     *  is a new one: a source that holds an effect already has to let it go
     *  first */
    Q_INVOKABLE bool arm();
    Q_INVOKABLE void toggleArmed();
    /** $reason: "stopAll", "blackout", "show", "idle", "showStop",
     *  "fadeToBlack", "recall" (what Stop all stopped, brought back), "hand",
     *  "exit", "none" (nothing protected any more) */
    Q_INVOKABLE void disarm(const QString &reason = QStringLiteral("hand"));
    /** The words of a reason of disarm() */
    Q_INVOKABLE QString reasonText(const QString &reason) const;
    /** The folder of the logs, in the file manager of the system */
    Q_INVOKABLE void openLog();

    /** A new layer for every universe, from the patch as it is now, given
     *  to them at once (their next frame) */
    void push();
    /** Wait (at most $maxMs, the events not processed) until every
     *  universe sent a frame with the last layer: false when one didn't */
    bool flush(int maxMs = 150);
    /** disarm($reason), then flush() when anything could have fired: before
     *  the frames stop (another show, the end of the program). "exit": from
     *  then on nothing arms them again, and the parks let go (kept, not sent) */
    void disarmAndFlush(const QString &reason);

    /** The serial of the last layers given (Universe::sentLayerSerial) */
    quint32 serial() const { return m_serial; }
    /** The arm token of the layers (0: disarmed) */
    quint32 armToken() const { return m_armed ? m_token : 0; }
    /** (tests) the effects disarm after $ms without a fire; -1: the option */
    void setAutoDisarmMsForTest(int ms);

    struct ParkEntry
    {
        quint32 fixture;
        quint32 channel;
        uchar value;            // the byte that goes out (the limits already in it)
        bool inBlackout;        // an intensity stays at it in blackout too
        QString mode;           // the mode of the fixture when it was parked
    };

    /** A value the show stop puts on a channel ($hold: the one it sends
     *  when the show stop comes, kept) */
    struct StopValue
    {
        uchar value;
        bool hold;
    };
    /** The layers push() gives the universes of $doc (one each, in their
     *  order) with $serial and $armToken: a unit for each part of a
     *  protected fixture in a universe, and the $parks (a protected fixture
     *  with any is parked off: it never fires, its channels are the gate's).
     *  With $showStop, the show stop on and its values (by universe address,
     *  never on a protected channel), through the limits of their channels.
     *  A $rampId (not 0) is a fade to black of $rampMs */
    static QVector<QSharedPointer<const OutputLayer>> buildLayers(Doc *doc, PatchManager *patch,
                                                                  quint32 serial, quint32 armToken,
                                                                  const QList<ParkEntry> &parks = QList<ParkEntry>(),
                                                                  const QMap<quint32, StopValue> *showStop = nullptr,
                                                                  quint32 rampId = 0, quint32 rampMs = 0);
    /** What the show stop puts on the fixtures of $doc, by universe address,
     *  before the limits: every shutter open and every pan and tilt held
     *  where it is; strobes and lasers dark; the lights (moving heads,
     *  scanners, washes, LED bars, blinders, dimmers) of $groups (empty:
     *  all of them) at open white at $level (0-100 %). Smoke, hazers, fans
     *  and the other effects stay as they are; a protected special effect is
     *  never in it (the gate gives it 0). A generic dimmer stands in for
     *  anything (a video trigger, a relay, a motor): lit only when its icon
     *  or its name says it is a light (genericDimmerLight) */
    static QMap<quint32, StopValue> showStopValues(Doc *doc, PatchManager *patch, int level,
                                                   const QList<quint32> &groups);
    /** The kinds the show stop lights (PatchManager::fixtureKinds) */
    static bool showStopLights(const QString &kind);
    /** A generic dimmer ("Generic - Generic", the fixture of a channel)
     *  the show stop lights: its icon in the plan is a light, or its name
     *  says a light (PAR, FRESNEL, DIMMER, BLINDER, FOCO, FRONTAL, LIGHT…)
     *  and nothing else (VIDEO, TRIGGER, RELÉ, MOTOR, PANTALLA…) */
    static bool genericDimmerLight(const QString &name, const QString &icon);

    /*********************************************************************
     * Parked channels
     *********************************************************************/
    /** The programmer: «Park» takes its values (Program) */
    void setProgrammer(Programmer *programmer);

    QVariantMap parkedFixtures() const;
    int parkedCount() const;
    Q_INVOKABLE bool isParked(quint32 id) const;
    /** Every channel of $ids parked at what goes out now (a protected
     *  special effect: at 0). Refused (0, with a message) while the show
     *  stop or a fade to black is on: what goes out then is theirs, not the
     *  show's. @return how many fixtures */
    Q_INVOKABLE int parkCurrent(const QVariantList &ids);
    /** The intensity channels of $ids (dimmers, colours) parked at 0, the
     *  others at what goes out now (during the show stop or a fade to black:
     *  what the show sends under them); one without intensities, all at 0 */
    Q_INVOKABLE int parkOff(const QVariantList &ids);
    /** The channels the programmer holds for its selection, parked at its
     *  values (through their limits), then released from it */
    Q_INVOKABLE int parkProgrammer();
    /** The selection of the programmer has values to park */
    Q_INVOKABLE bool canParkProgrammer() const;
    Q_INVOKABLE void unpark(const QVariantList &ids);
    Q_INVOKABLE void unparkAll();
    /** The parked intensities of $ids stay at their value in blackout too */
    Q_INVOKABLE void setParkInBlackout(const QVariantList &ids, bool on);
    /** Every parked fixture of $ids stays in blackout (false: none parked) */
    Q_INVOKABLE bool parkInBlackout(const QVariantList &ids) const;
    /** What Niveles DMX marks in $universe: { "address" (0-511): { kind:
     *  "park", value, inBlackout, showStop (the show stop puts its own value
     *  there now), ramp (a fade to black takes it down now) } or { kind:
     *  "effect", parked, armed } or { kind: "showStop", hold } (a channel
     *  the show stop puts now, not parked) } */
    Q_INVOKABLE QVariantMap channelMarks(int universe) const;
    /** The parks, by fixture and channel (tests) */
    QList<ParkEntry> parks() const;

    /** The parks into the section of DMX Total Control (nothing without any) */
    void saveXML(QXmlStreamWriter *doc) const;
    /** The Safety element of a show: its parks (the bad ones skipped) */
    void loadXML(QXmlStreamReader &reader);
    /** A show opened: its parks checked against its fixtures, and said */
    void afterLoad();

    /*********************************************************************
     * The show stop
     *********************************************************************/
    /** The live mixer: no strobe during a show stop */
    void setMixer(LiveMixer *mixer);

    bool showStop() const { return m_showStop; }
    bool holding() const { return m_holdCount > 0; }
    int holdMs() const { return 1000; }
    /** A control that puts and takes away the show stop is down ($down) or
     *  up: held holdMs() it toggles it. Several at once count as one; an up
     *  of nothing held (a note off lost, the second control) does nothing */
    Q_INVOKABLE void holdShowStop(bool down);
    bool keyPending() const;
    int keyWindowMs() const { return 2000; }
    /** The Pause key was pressed. It can't be held: on a PC keyboard it has
     *  no release of its own (Windows sends its release with its press). A
     *  first press waits keyWindowMs() for a second one, which toggles the
     *  show stop at once; one press alone does nothing */
    Q_INVOKABLE void pressShowStopKey();
    /** On or off at once (tests, --show-stop) */
    void setShowStop(bool on);

    int showStopLevel() const { return m_showStopLevel; }
    void setShowStopLevel(int level);
    QVariantList showStopGroups() const;
    void setShowStopGroups(const QVariantList &groups);

    /*********************************************************************
     * The fade to black
     *********************************************************************/
    bool fadingToBlack() const { return m_rampId != 0; }
    int fadeToBlackMs() const { return m_rampId != 0 ? int(m_rampMs) : 0; }
    /** The time of a fade to black: by default, and its range (ms) */
    static constexpr int fadeToBlackDefaultMs() { return 3000; }
    static constexpr int fadeToBlackMinMs() { return 100; }
    static constexpr int fadeToBlackMaxMs() { return 60000; }
    int defaultFadeToBlackMs() const { return fadeToBlackDefaultMs(); }
    /** The time a mapping or a SHOW button $m gives its fade to black
     *  ("time"; none: the default), in its range */
    static int fadeToBlackTime(const QVariantMap &m);
    /** The light goes down in $ms (in its range), then Blackout (the light
     *  is full again under it). The special effects are disarmed. During
     *  one: Blackout at once. Blackout on already: nothing */
    Q_INVOKABLE void fadeToBlack(int ms = fadeToBlackDefaultMs());

signals:
    void protectionChanged();
    void armedChanged();
    void disarmInChanged();
    /** Only when the fixtures that fire changed (not for a refusal or a
     *  release: what shows them isn't read again for nothing) */
    void firingChanged();
    void waitingChanged();
    void recentFiresChanged();
    void revisionChanged();
    /** The effects were disarmed (not by arming again), see disarm() */
    void disarmed(const QString &reason);
    /** Something to tell the user ($error: a refusal, a failure) */
    void message(const QString &text, bool error);
    void parksChanged();
    void showStopChanged();
    void holdingChanged();
    void keyPendingChanged();
    void showStopSettingsChanged();
    void fadeToBlackChanged();

private slots:
    /** The patch, the universes or the protection changed: a new layer
     *  (not while a show loads: DtcCore gives it before the output starts) */
    void slotPatchChanged();
    void slotProtectionChanged();
    void slotDocCleared();
    void slotBlackoutChanged(bool on);
    void slotEffectEvent(quint32 universe, quint32 fixture, int kind, int reason, uint durationMs);
    void slotIdleTick();
    /** A fixture gone: its parks go. Of another mode: its parks go (said);
     *  a protected one stays parked off */
    void slotFixtureRemoved(quint32 id);
    void slotFixtureChanged(quint32 id);
    /** Held long enough: the show stop toggles */
    void slotHoldDone();
    /** A group of the show stop gone (its light: the other groups) */
    void slotGroupRemoved(quint32 id);
    /** The fixtures of a group changed: the light of the show stop follows */
    void slotGroupChanged(quint32 id);
    /** The fade to black is down: Blackout, the light full under it */
    void slotRampDone();

private:
    /** push(); $notify false: nothing told (the universes are held by the
     *  caller: what hears of it could ask for them) */
    void pushLayers(bool notify);
    int autoDisarmMs() const;
    void restartIdle();
    void setDisarmIn(int seconds);
    /** A line for the log: when, the number and name of the fixture, how
     *  long it fired, what ended it */
    void writeLog(const QString &line);
    QString fixtureName(quint32 id) const;
    int fixtureNumber(quint32 id) const;
    /** A fire: when it started, and the name and number of its fixture
     *  then (its end may come once the fixture is deleted) */
    struct Firing
    {
        QDateTime start;
        QString name;
        int number = 0;
    };
    /** One fire ended: the log and the last fires (the fixture as it is
     *  now, or as it was when it started if it is gone) */
    void logFire(quint32 fixture, const Firing &firing, uint ms, int reason);
    static QString reasonKey(int reason);
    QString fireReasonText(int reason) const;

    /** The mode a park keeps of its fixture (its name) */
    static QString modeOf(const Fixture *fxi);
    /** A protected special effect now (what the gate gives it) */
    bool protectedNow(quint32 id) const;
    /** parkCurrent(), parkOff() */
    int parkNow(const QVariantList &ids, bool off);
    /** The parks of $id gone. @return whether it had any */
    bool dropParks(quint32 id);
    /** Every channel of $id parked at 0 (a protected special effect) */
    void parkAllOff(quint32 id, bool inBlackout);
    /** Whether $id has parks that stay in blackout (parked again, it keeps it) */
    bool keepsBlackout(quint32 id) const;
    /** The parks changed: the layers, the show, the screens */
    void parksEdited();
    /** The park of some fixtures said: $text, and when $protectedCount of
     *  them are protected special effects, that they are parked off */
    void tellParked(const QString &text, int protectedCount);
    /** A show that goes: the show stop off, its settings as a new show has them */
    void clearShowStop();
    /** The show stop has settings of its own to save (not every light at 100 %) */
    bool showStopSaved() const;
    /** Its element of the show file (<ShowStop Level Groups/>), when it has settings */
    void saveShowStopXML(QXmlStreamWriter *doc) const;
    /** Its settings from that element (the bad ones: as a new show has them) */
    void loadShowStopXML(const QXmlStreamAttributes &attrs);
    /** Its groups, but those that aren't in the show */
    void checkShowStopGroups();
    /** No fade to black any more (the light full again, under the Blackout
     *  if it came); $push: the layers without it */
    void endRamp(bool push = true);

private:
    Doc *m_doc;
    PatchManager *m_patch;
    AppSettings *m_settings;

    bool m_armed = false;
    /** The program closes (disarmAndFlush("exit")): arm() is refused */
    bool m_closing = false;
    quint32 m_token = 0;
    quint32 m_tokens = 0;
    quint32 m_serial = 0;
    int m_revision = 0;

    /** Auto-disarm: the time since the last fire (or the arming) */
    QElapsedTimer m_idle;
    QTimer *m_idleTimer;
    int m_disarmIn = -1;
    int m_testAutoDisarmMs = -1;

    /** The fixtures that fire, the ones that wait */
    QMap<quint32, Firing> m_firing;
    QMap<quint32, QString> m_waiting;

    struct Fire
    {
        quint32 fixture;
        QString name;
        int number;
        QDateTime start;
        uint ms;
        int reason;
    };
    QList<Fire> m_recent;

    /** The log of the session: its file once written */
    QDateTime m_sessionStart;
    QString m_logFile;
    bool m_logFailed = false;

    /** The parked channels, by fixture and channel */
    QMap<QPair<quint32, quint32>, ParkEntry> m_parks;
    Programmer *m_programmer = nullptr;

    /** The show stop: on, the controls that hold it down, its settings */
    bool m_showStop = false;
    int m_holdCount = 0;
    QTimer *m_holdTimer;
    /** The second press of the Pause key is waited for while it runs */
    QTimer *m_keyTimer;
    int m_showStopLevel = 100;
    QList<quint32> m_showStopGroups;
    LiveMixer *m_mixer = nullptr;

    /** The fade to black: its id (0: none; a new one each time), its time */
    quint32 m_rampId = 0;
    quint32 m_ramps = 0;
    quint32 m_rampMs = 0;
    QTimer *m_rampTimer;
};

#endif
