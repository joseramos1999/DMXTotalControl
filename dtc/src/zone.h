/*
  DMX Total Control
  zone.h

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

#ifndef ZONE_H
#define ZONE_H

#include <QObject>
#include <QColor>
#include <QHash>
#include <QList>
#include <QSet>
#include <QVariant>

class QXmlStreamReader;
class QXmlStreamWriter;
class Function;
class Doc;

#define KXMLDTCZone QStringLiteral("Zone")

/**
 * A Zone groups a set of Functions (scenes, chasers, EFX...): when the
 * zone is exclusive only one of its functions can
 * run at a time, and triggering another one crossfades to it. Every zone
 * runs independently from the others and has its own intensity master.
 */
class Zone : public QObject
{
    Q_OBJECT

public:
    /** What a scene stops when it starts (release mode) */
    enum ReleaseMode
    {
        ReleaseGroupDefault = -1, ///< exclusive group: ReleaseGroup, mix group: ReleaseNone
        ReleaseNone = 0,
        ReleaseGroup,               ///< the other scenes of its group
        ReleaseAll,                 ///< every other scene
        ReleaseOthers               ///< the scenes of the other groups
    };
    Q_ENUM(ReleaseMode)

    /** Which releases a scene ignores (release protection) */
    enum Protection
    {
        ProtectNone = 0,
        ProtectAll,                 ///< only a manual stop releases it
        ProtectGroup,               ///< ignores releases from its own group
        ProtectOthers               ///< ignores releases from other groups
    };
    Q_ENUM(Protection)


    Q_PROPERTY(quint32 id READ id CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool exclusive READ exclusive WRITE setExclusive NOTIFY exclusiveChanged)
    Q_PROPERTY(int fadeTime READ fadeTime WRITE setFadeTime NOTIFY fadeTimeChanged)
    Q_PROPERTY(qreal master READ master WRITE setMaster NOTIFY masterChanged)
    Q_PROPERTY(QVariantList functions READ functionsInfo NOTIFY functionsChanged)
    Q_PROPERTY(int activeCount READ activeCount NOTIFY activeChanged)

public:
    Zone(Doc *doc, quint32 id, QObject *parent = nullptr);
    ~Zone();

    quint32 id() const;

    QString name() const;
    void setName(const QString &name);

    QColor color() const;
    void setColor(const QColor &color);

    /** When true, triggering a function stops the other running ones */
    bool exclusive() const;
    void setExclusive(bool exclusive);

    /** Crossfade time in ms. A negative value uses each function's own timings */
    int fadeTime() const;
    void setFadeTime(int ms);

    /** Intensity master applied to every function started by this zone (0.0 - 1.0) */
    qreal master() const;
    void setMaster(qreal value);

    QList<quint32> functionIDs() const;
    QVariantList functionsInfo() const;
    int activeCount() const;

    Q_INVOKABLE void addFunction(quint32 fid);
    Q_INVOKABLE void removeFunction(quint32 fid);
    Q_INVOKABLE void moveFunction(int from, int to);

    /** Start $fid, or stop it if this zone already started it */
    Q_INVOKABLE void trigger(quint32 fid);
    Q_INVOKABLE void start(quint32 fid);
    Q_INVOKABLE void stop(quint32 fid);
    Q_INVOKABLE void stopAll();
    Q_INVOKABLE bool isActive(quint32 fid) const;

    /** Flash scenes play while pressed */
    Q_INVOKABLE void press(quint32 fid);
    Q_INVOKABLE void unpress(quint32 fid);

    /** Start the next/previous scene of the group, stopping the current one */
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();

    /** Per scene options: { release, protection, flash, dimmer, speed, phase, size, fadeSpeed }.
     *  dimmer, speed, phase, size and fadeSpeed are live controls (live dials) */
    Q_INVOKABLE QVariantMap options(quint32 fid) const;
    Q_INVOKABLE void setOption(quint32 fid, const QString &key, const QVariant &value);

    /** The release mode in effect, with ReleaseGroupDefault resolved */
    ReleaseMode effectiveRelease(quint32 fid) const;
    Protection protection(quint32 fid) const;
    bool isFlash(quint32 fid) const;
    /** Live dimmer of a scene (0.0 - 1.0), on top of the group master */
    qreal sceneDimmer(quint32 fid) const;
    /** The level a fader of the SHOW gives a scene this group plays (0.0 -
     *  1.0): on top of the master and the dimmer (the group has the one
     *  intensity of the scene). It lasts while the scene plays (1.0 after) */
    void setSceneLevel(quint32 fid, qreal level);
    qreal sceneLevel(quint32 fid) const;

    /** $level (0.0 - 1.0) as the intensity of $f through the overrides $ids
     *  it was given (adjusted; those gone with a previous run, new ones). A
     *  super scene has no intensity of its own: an override per track, and
     *  its static values play at $level too. @return its overrides now */
    static QList<int> applyLevel(Doc *doc, Function *f, qreal level, const QList<int> &ids);
    /** Whether a scene waits paused for a second press (start mode "paused") */
    Q_INVOKABLE bool isWaiting(quint32 fid) const;
    QList<quint32> activeFunctions() const;

    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc) const;

signals:
    void nameChanged();
    void colorChanged();
    void exclusiveChanged();
    void fadeTimeChanged();
    void masterChanged();
    void functionsChanged();
    void activeChanged();

private slots:
    void slotFunctionRunning(quint32 fid);
    void slotFunctionStopped(quint32 fid);
    void slotFunctionRemoved(quint32 fid);
    void slotFunctionReplaced(quint32 fid);
    void slotLoopsFinished(quint32 fid);

private:
    void connectFunction(quint32 fid);
    void disconnectFunction(quint32 fid);
    uint fadeIn() const;
    uint fadeOut() const;
    void step(int direction);
    /** Apply speed, phase and size to a running scene */
    void applyLiveControls(quint32 fid);

private:
    Doc *m_doc;
    quint32 m_id;
    QString m_name;
    QColor m_color;
    bool m_exclusive;
    int m_fadeTime;
    qreal m_master;

    QList<quint32> m_functions;
    /** Functions started by this zone */
    QSet<quint32> m_active;
    /** Intensity override IDs, by function ID (a super scene has one per
     *  track: it has no intensity of its own) */
    QHash<quint32, QList<int>> m_intensityOverrides;

    /** The master and the dimmer of the scene $fid on its intensity (on
     *  every track of a super scene, and on its static values) */
    void applyIntensity(quint32 fid);

    struct Options
    {
        int release = ReleaseGroupDefault;
        int protection = ProtectNone;
        bool flash = false;
        qreal dimmer = 1.0;
        qreal speed = 1.0;
        int phase = 0;
        qreal size = 1.0;
        qreal fadeSpeed = 1.0;
    };
    QHash<quint32, Options> m_options;
    /** Overrides of the live controls, by function ID and attribute index */
    QHash<quint32, QHash<int, int>> m_liveOverrides;
    /** Scenes started paused, waiting for a second press */
    QSet<quint32> m_waiting;
    /** The levels of the SHOW faders on the scenes playing (setSceneLevel) */
    QHash<quint32, qreal> m_sceneLevels;
    /** The scene started last, for next/previous */
    quint32 m_lastStarted;
};

#endif
