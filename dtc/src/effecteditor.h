/*
  DMX Total Control
  effecteditor.h

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

#ifndef EFFECTEDITOR_H
#define EFFECTEDITOR_H

#include <QObject>
#include <QVariant>
#include <QUrl>

class ZoneManager;
class Programmer;
class AppSettings;
class Function;
class Doc;

/**
 * Creation and editing of the dynamic scene types, on top of
 * the engine functions:
 *   Steps       -> Sequence (a hidden Scene with the channels, plus steps)
 *   Move FX     -> EFX
 *   Colour FX   -> RGBMatrix (scripts, plain colour, text...)
 *   Effect rack -> Collection
 * New effects take the fixtures (and values) of the programmer.
 * Times exchanged with QML are in milliseconds, or in beats x 1000 when
 * the function follows the BPM.
 */
class EffectEditor : public QObject
{
    Q_OBJECT

public:
    EffectEditor(Doc *doc, Programmer *programmer, ZoneManager *zones, QObject *parent = nullptr);

    /** The settings with the default priority of new scenes (none: the engine default) */
    void setSettings(AppSettings *settings);

    /*********************************************************************
     * Common
     *********************************************************************/
    /** Everything the editor shows about a function, by type */
    Q_INVOKABLE QVariantMap info(quint32 id) const;

    Q_INVOKABLE void setTiming(quint32 id, int fadeIn, int fadeOut, int duration);
    /** Durations in beats, following the project BPM, instead of time */
    Q_INVOKABLE void setFollowBpm(quint32 id, bool follow);
    /** Number of times to play before stopping; 0 loops forever */
    Q_INVOKABLE void setSingleShot(quint32 id, bool singleShot);

    /** Start or stop a function to try it, outside of any group */
    Q_INVOKABLE void togglePreview(quint32 id);
    Q_INVOKABLE bool isRunning(quint32 id) const;

    Q_INVOKABLE quint32 duplicate(quint32 id);

    /*********************************************************************
     * Steps
     *********************************************************************/
    /** New step scene, with the programmer as first step. @return the ID or -1 */
    Q_INVOKABLE int createSteps(const QString &name, int zoneIndex = -1);
    /** [{ index, fadeIn, hold, fadeOut, channels, waits }]: $waits is a step
     *  that holds until GO (an infinite hold, QLC+ manual chasers), its hold 0 */
    Q_INVOKABLE QVariantList steps(quint32 id) const;
    /** Add the programmer as a new step after $after (-1: at the end) */
    Q_INVOKABLE bool addStep(quint32 id, int after = -1);
    /** Store the programmer in an existing step */
    Q_INVOKABLE bool updateStep(quint32 id, int index);
    /** Load a step in the programmer, to see and edit it */
    Q_INVOKABLE bool loadStep(quint32 id, int index);
    Q_INVOKABLE bool duplicateStep(quint32 id, int index);
    Q_INVOKABLE bool removeStep(quint32 id, int index);
    Q_INVOKABLE bool moveStep(quint32 id, int from, int to);
    /** Times of several steps at once; negative values are left unchanged */
    Q_INVOKABLE void setStepTimes(quint32 id, const QVariantList &indexes, int fadeIn, int hold, int fadeOut);
    /** Steps that hold until GO (an infinite hold), or that hold for the
     *  default time again (the ones that waited) */
    Q_INVOKABLE void setStepsWait(quint32 id, const QVariantList &indexes, bool wait);
    /** The channels scene of steps that are gone (deleted or converted),
     *  stopped and deleted when it was only theirs: a visible scene (QLC+
     *  binds steps to ordinary scenes) or one that other functions use
     *  (copies of steps share it) stays. @return whether it was deleted */
    static bool deleteBoundScene(Doc *doc, quint32 sceneId);

    /*********************************************************************
     * Move FX
     *********************************************************************/
    Q_INVOKABLE QStringList moveShapes() const;
    /** New movement effect on the selected fixtures that have pan/tilt */
    Q_INVOKABLE int createMoveFx(const QString &name, int zoneIndex = -1);
    /** keys: shape, width, height, rotation, xOffset, yOffset, phase,
     *  spread (0 parallel, 1 serial, 2 offset), relative, xFrequency, yFrequency, dimmer */
    Q_INVOKABLE void setMoveFx(quint32 id, const QString &key, const QVariant &value);
    /** Points of the shape, normalised to 0-1, for the preview */
    Q_INVOKABLE QVariantList movePreview(quint32 id) const;
    /** What the effect sends right now, fixture by fixture in its order:
     *  { running, items: [{ index, name, pan, tilt, startX, startY } (Move FX),
     *  { index, name, x, y, color } (Colour FX, with cols and rows), { index, name, level } (Curve FX)] } */
    Q_INVOKABLE QVariantMap liveView(quint32 id) const;

    /*********************************************************************
     * Colour FX
     *********************************************************************/
    Q_INVOKABLE QStringList colourEffects() const;
    /** New colour/pixel effect on the selected fixtures, in selection order */
    Q_INVOKABLE int createColourFx(const QString &name, int zoneIndex = -1);
    /** keys: effect, color0..4, mode (0 RGB, 1 white, 2 amber, 3 UV, 4 dimmer, 5 shutter), text, dimmerControl */
    Q_INVOKABLE void setColourFx(quint32 id, const QString &key, const QVariant &value);
    Q_INVOKABLE void setColourFxProperty(quint32 id, const QString &name, const QString &value);
    /** New mapping: a colour effect laid on the 2D plan */
    Q_INVOKABLE int createMapping(const QString &name, int zoneIndex = -1);

    /** Apply the programmer selection to an effect */
    Q_INVOKABLE bool useSelection(quint32 id);

    /*********************************************************************
     * Curve FX
     *********************************************************************/
    Q_INVOKABLE QStringList curveWaveforms() const;
    /** New Curve FX on the selected fixtures, driving their dimmer */
    Q_INVOKABLE int createCurveFx(const QString &name, int zoneIndex = -1);
    /** A strobe effect: the dimmer of the selected fixtures flashes,
     *  all together or at random (a Curve FX with a strobe wave) */
    Q_INVOKABLE int createStrobe(const QString &name, int zoneIndex = -1);
    /** A super scene: a timeline of scenes and audio, with two tracks */
    Q_INVOKABLE int createSuperScene(const QString &name, int zoneIndex = -1);
    /** keys: wave, low, high, phase, spread, relative, features (list of channel keys) */
    Q_INVOKABLE void setCurveFx(quint32 id, const QString &key, const QVariant &value);
    /** The channels of the effect's fixtures: [{ key, name, selected }] */
    Q_INVOKABLE QVariantList curveFeatures(quint32 id) const;
    /** One line per fixture (up to 8), 64 points each, values 0..255 */
    Q_INVOKABLE QVariantList curvePreview(quint32 id) const;

    /*********************************************************************
     * Playback settings
     *********************************************************************/
    /** keys: loops (0 = always), endAction ("release"/"pause"), jumpTo (-1 none,
     *  -2 next, -3 previous, or a scene ID), drive ("off"/"bpm"/"beatgo"/"pulse"),
     *  divisions, startMode ("play"/"paused"), stagger (ms), tap (ms a step of
     *  its own rhythm, 0: its own times) */
    Q_INVOKABLE void setPlayback(quint32 id, const QString &key, const QVariant &value);

    /*********************************************************************
     * Effect rack
     *********************************************************************/
    Q_INVOKABLE int createRack(const QString &name, const QVariantList &functionIds, int zoneIndex = -1);
    Q_INVOKABLE void addToRack(quint32 id, quint32 functionId);
    Q_INVOKABLE void removeFromRack(quint32 id, quint32 functionId);
    /** The functions that play $id (racks, steps, super scenes, directly or
     *  through others): it can't play them, it would play itself */
    Q_INVOKABLE QVariantList playersOf(quint32 id) const;
    /** Save a scene or a rack, with what it plays, to a file */
    Q_INVOKABLE bool exportEffect(quint32 id, const QUrl &url);
    /** Load an exported file into the show. @return the new scene ID, or -1 */
    Q_INVOKABLE int importEffect(const QUrl &url, int zoneIndex = -1);
    /** Record one cycle of an effect into a Steps scene. @return its ID, or -1 */
    Q_INVOKABLE int convertToSteps(quint32 id, int stepMs = 100);

    /*********************************************************************
     * FX
     *********************************************************************/
    /** Change a layer of a scene with FX (the linked ones share the timing) */
    Q_INVOKABLE void setFxLayer(quint32 id, int index, const QVariantMap &props);
    Q_INVOKABLE void removeFxLayer(quint32 id, int index);
    /** Each fixture of a layer at $cyclePos, for a preview */
    Q_INVOKABLE QVariantList fxPreview(quint32 id, int index, qreal cyclePos) const;
    /** The shapes, to draw them */
    Q_INVOKABLE QVariantList fxModeCurve(int mode, int samples = 48) const;
    Q_INVOKABLE QVariantList fxFigurePath(int figure, int samples = 64) const;
    /** A Move FX or a Curve FX becomes a scene with FX, with the same ID
     *  (its groups, buttons and mappings follow) */
    Q_INVOKABLE bool convertToFx(quint32 id);

signals:
    /** A function was changed by the editor; QML refreshes its view */
    void edited(quint32 id);
    /** What QML shows to the user: a problem ($error), or what was done
     *  (saved to a file, the programmer let the effect be seen...) */
    void message(const QString &text, bool error);

private:
    int addNew(Function *function, const QString &name, int zoneIndex);
    /** $pattern with the first number (from 1) that no function has as its name */
    QString uniqueName(const QString &pattern) const;
    /** The priority of new scenes, from the settings */
    void applyDefaultPriority(Function *function) const;
    QList<quint32> selectedWithPanTilt() const;
    /** Whether any of $fixtures has its dimmer open (or has no dimmer) */
    bool isLit(const QList<quint32> &fixtures) const;
    quint32 makeGroup(const QString &name, const QList<quint32> &fixtures);
    /** A group laid out like the 2D plan: each fixture where it stands */
    quint32 makePlanGroup(const QString &name, const QList<quint32> &fixtures);
    /** The fixtures of a group, in its row order */
    QList<quint32> groupFixtures(quint32 groupId) const;
    void replaceGroup(quint32 matrixId, quint32 newGroup);
    void touch(quint32 id);

private:
    Doc *m_doc;
    Programmer *m_programmer;
    ZoneManager *m_zones;
    AppSettings *m_settings;
};

#endif
