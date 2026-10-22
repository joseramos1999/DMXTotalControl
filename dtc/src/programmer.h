/*
  DMX Total Control
  programmer.h

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

#ifndef PROGRAMMER_H
#define PROGRAMMER_H

#include <QObject>
#include <QColor>
#include <QList>
#include <QMap>
#include <QPair>
#include <QVariant>

#include <functional>
#include <QSet>
#include <QHash>
#include <QPointF>

#include "fxscene.h"

class GenericDMXSource;
class SceneValue;
class QLCChannel;
class Cuelist;
class Fixture;
class ZoneManager;
class PaletteManager;
class PatchManager;
class Doc;

/**
 * The programmer, the way lighting consoles have it:
 * select fixtures, set their attributes live, then record the result as a
 * scene. Programmer values have priority over running playbacks.
 *
 * Channels of the selected fixtures are merged by what they do (their
 * attribute), so moving the "Pan" fader moves the pan of every selected
 * fixture that has one, whatever the name of its channel.
 */
class Programmer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList selection READ selection NOTIFY selectionChanged)
    Q_PROPERTY(QVariantList channels READ channels NOTIFY channelsChanged)
    Q_PROPERTY(bool hasValues READ hasValues NOTIFY valuesChanged)
    Q_PROPERTY(int valuesCount READ valuesCount NOTIFY valuesChanged)
    /** Blind: edit the programmer without sending it to the output */
    Q_PROPERTY(bool blind READ blind WRITE setBlind NOTIFY blindChanged)
    /** Pan and tilt of the selection, 0..1: { x, y, available } */
    Q_PROPERTY(QVariantMap position READ position NOTIFY channelsChanged)
    /** The fan of the position of the selection: { pan, tilt (-1..1), mode } */
    Q_PROPERTY(QVariantMap positionFan READ positionFan NOTIFY channelsChanged)
    Q_PROPERTY(QVariantMap aim READ aim NOTIFY aimChanged)
    Q_PROPERTY(QVariantList fxLayers READ fxLayers NOTIFY fxChanged)
    Q_PROPERTY(QVariantList fxParams READ fxParams NOTIFY fxParamsChanged)
    Q_PROPERTY(QString lastParam READ lastParam WRITE setLastParam NOTIFY lastParamChanged)
    /** Highlight: the selection at full in open white, over everything, not recorded */
    Q_PROPERTY(bool highlight READ highlight WRITE setHighlight NOTIFY highlightChanged)
    /** Going through the selection one fixture at a time: { active, index, count } */
    Q_PROPERTY(QVariantMap stepping READ stepping NOTIFY selectionChanged)

public:
    Programmer(Doc *doc, ZoneManager *zones, QObject *parent = nullptr);
    ~Programmer();

    /*********************************************************************
     * Selection
     *********************************************************************/
    QVariantList selection() const;

    Q_INVOKABLE bool isSelected(quint32 fixtureId) const;
    Q_INVOKABLE void select(quint32 fixtureId, bool additive = false);
    Q_INVOKABLE void toggle(quint32 fixtureId);
    Q_INVOKABLE void selectFixtures(const QVariantList &fixtureIds, bool additive = false);
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void clearSelection();

    /** Selection tools. Order follows the 2D plan, left to right */
    Q_INVOKABLE void invertSelection();
    /** Keep one fixture every n (1/2, 1/3, 1/4...) */
    Q_INVOKABLE void selectEvery(int n);
    /** Move the selection to the next (1) or previous (-1) fixtures */
    Q_INVOKABLE void shiftSelection(int direction);

    /*********************************************************************
     * Values
     *********************************************************************/
    /** The merged channels of the selection:
     *  [{ key, name, group, colour, value, active, capabilities }] */
    QVariantList channels() const;

    bool hasValues() const;
    int valuesCount() const;

    Q_INVOKABLE void setChannelValue(const QString &key, int value);
    Q_INVOKABLE void releaseChannel(const QString &key);
    /** The attribute of a channel of a fixture: the key of its fader ("red", "pan", "gobo"...) */
    QString attributeKey(quint32 fixtureId, quint32 channel) const;
    /** Release the values of $fixtures whose channel matches $match (an effect that is
     *  tried needs them: the programmer would override it). @return how many */
    int releaseChannels(const QList<quint32> &fixtures, const std::function<bool(const QLCChannel *)> &match);
    /** Set every intensity channel without a colour (dimmers) of the selection.
     *  @return how many fixtures have one (RGB-only fixtures don't) */
    Q_INVOKABLE int setDimmer(int value);
    /** Mix $color using the RGB/CMY/W channels of the selection */
    Q_INVOKABLE void setColor(const QColor &color);
    /** Release every programmer value */
    Q_INVOKABLE void clear();
    /** Release the values of the selected fixtures only */
    Q_INVOKABLE void resetSelected();

    /** Pan and tilt of the selection from an XY grid, 0..1, with the fine
     *  channels (only those: not a rotation or a speed of pan and tilt). With
     *  a fan, the fixtures go there and keep their fan around it */
    Q_INVOKABLE void setPosition(qreal x, qreal y);
    /** { available, x, y, points: [{ x, y }] the position of each fixture
     *  of the selection with pan or tilt }; with a fan, x and y are its centre */
    QVariantMap position() const;

    /**
     * A fan of the position: each fixture of the selection (in its order)
     * moves from where it was by up to half the travel times  and 
     * (-1..1): "spread" from one end to the other (-1 to 1), "v" the ends
     * out and the middle still (1 0 1), "first" from the first one (0 to
     * 1). 0 and 0: back where they were. Moved by something else (a
     * palette, released, another selection) the fan starts again from there
     */
    Q_INVOKABLE void setPositionFan(qreal pan, qreal tilt, const QString &mode);
    QVariantMap positionFan() const;

    /** A channel of the position: pan or tilt, coarse or fine (a preset of
     *  them, or one of their groups that isn't a rotation, a speed...) */
    static bool isPositionChannel(const QLCChannel *ch, bool *pan = nullptr, bool *fine = nullptr);

    /** The same on other fixtures than the selection (the SHOW colour wheel and XY grid) */
    void setColorFor(const QList<quint32> &fixtures, const QColor &color);
    void setPositionFor(const QList<quint32> &fixtures, qreal x, qreal y);
    QVariantMap positionFor(const QList<quint32> &fixtures) const;
    /** The colour the programmer gives the first of $fixtures that has one (invalid: none) */
    QColor colorFor(const QList<quint32> &fixtures) const;

    /** Live edits beat the scenes of this priority (0-10) and below */
    void setLivePriority(int priority);
    /** When a scene starts (or a cuelist goes to a cue): "keep" the edits,
     *  release the "channels" it uses (a cue: the ones it changes), or "all" */
    void setReleaseMode(const QString &mode);
    /** Release the edits on the channels a function plays (a cuelist: the
     *  ones its GOs changed since the last time) */
    void releaseUsedBy(quint32 functionId);
    /** Release the edits on these channels. @return true if any was released */
    bool releaseKeys(const QSet<QPair<quint32, quint32>> &keys);

    /** Open the beam: dimmer full and shutter open (white for fixtures without dimmer) */
    Q_INVOKABLE void beamOn();
    /** Close the beam: dimmer at zero and shutter closed */
    Q_INVOKABLE void beamOff();
    /** Pan and tilt to the centre */
    Q_INVOKABLE void centreBeam();

    /*********************************************************************
     * Follow: the moving heads of the selection aim at a point of the plan
     *********************************************************************/
    /** The patch: where the fixtures are and how they hang */
    void setPatch(PatchManager *patch);
    /** The moving heads of the selection aim at the point $x, $y of the
     *  stage plan (mm) at height $z (mm over the floor): their pan and tilt,
     *  with the inversions, limits and swap of the patch undone. Dragged,
     *  each one goes the short way from where it aimed. @return aim() */
    Q_INVOKABLE QVariantMap aimAt(qreal x, qreal y, qreal z);
    /** { active, x, y, z (mm), aimed: [ids], unreachable: [ids out of their
     *  reach], guessed: [ids whose travel is a guess], skipped: [ids of
     *  scanners] }; active until the position is set another way, the
     *  selection changes or the programmer is cleared */
    QVariantMap aim() const;

    /**
     * Spread a channel over the selection, in selection order: from $base
     * to $base + $spread from the first to the last fixture, or from the
     * centre outwards when $fromCentre. A fixture without the channel
     * but with it on each head ("dimmer 1", "dimmer 2"...) sets them all.
     * @return how many fixtures it set
     */
    Q_INVOKABLE int setChannelFan(const QString &key, int base, int spread, bool fromCentre);

    bool blind() const;
    void setBlind(bool blind);

    /*********************************************************************
     * Record
     *********************************************************************/
    /** Store the programmer in a new scene, optionally adding it to a zone.
     *  @return the new scene ID, or -1 on error */
    Q_INVOKABLE int recordScene(const QString &name, int zoneIndex = -1, int priority = -1);
    /** Overwrite the values of an existing scene with the programmer (with
     *  FX a static scene becomes an FX scene with its ID, except the scene
     *  steps play their channels through: false) */
    Q_INVOKABLE bool updateScene(quint32 sceneId);
    /** Snapshot: the live edits into an existing scene, over its values on the same channels */
    Q_INVOKABLE bool mergeIntoScene(quint32 sceneId);
    /** Snapshot: everything that leaves the software, on every fixture with light or values,
     *  as a new scene. Not while highlight is on (it isn't part of the look): -1 */
    Q_INVOKABLE int snapshotAll(const QString &name, int zoneIndex = -1, int priority = -1);
    /** Load the values of a scene into the programmer for editing */
    Q_INVOKABLE bool loadScene(quint32 sceneId);

    /** The programmer values, as scene values */
    QList<SceneValue> sceneValues() const;
    /** Replace the programmer with values, selecting their fixtures (no FX) */
    void loadValues(const QList<SceneValue> &values);
    /**
     * Replace the programmer with a look to edit (a scene, a step, a cue):
     * its values, its FX (with none, the FX of before stop: the next update
     * would record them) and the palettes of its values. $fixtures are
     * selected in that order, then the other fixtures with values or FX.
     * It is not a clear(): cleared() is not emitted
     */
    void loadState(const QMap<QPair<quint32, quint32>, uchar> &values, const QList<quint32> &fixtures,
                   const QList<FXScene::Layer> &fx, const QHash<QPair<quint32, quint32>, int> &refs);
    QList<quint32> selectionIds() const;
    /** The fixtures with values: the selected ones in selection order, then the others */
    QList<quint32> valueFixturesInOrder() const;

    /*********************************************************************
     * FX: waves on the parameters of the selection, played live
     * on top of the programmer values (their base) and recorded with them
     *********************************************************************/
    /** The FX of the programmer: [{ index, param, count, ...layer }] */
    QVariantList fxLayers() const;
    /** The scene that plays them (the speed of the movement of the SHOW goes there too) */
    FXScene *fxScene() const;
    /** The parameters of the selection an FX can move:
     *  [{ key, name, group, colour, active }], pan/tilt together first */
    QVariantList fxParams() const;
    /** The parameter last touched (where the FX panel opens) */
    QString lastParam() const;
    Q_INVOKABLE void setLastParam(const QString &param);

    /** The FX of $param on the selection, with "active", "count" and "sets". Its
     *  shape is the one of the first selected fixture; "partial": the FX has
     *  other fixtures; "ownCount", "ownSelected": the fixtures (selected ones)
     *  that do something their own way */
    Q_INVOKABLE QVariantMap fxOf(const QString &param) const;
    /** Change (or start) the FX of $param on the selection. An FX stays one, on
     *  one clock: the selected fixtures join it; the shape, size and direction
     *  (FxTools::ownKeys) go to the selected fixtures only, the speed and the
     *  timing to the whole FX */
    Q_INVOKABLE void setFx(const QString &param, const QVariantMap &props);
    /** The selected fixtures of the FX of $param do what the FX does again */
    Q_INVOKABLE void resetFxOwn(const QString &param);
    /** Stop the FX of $param on the selection (and of what is linked to it) */
    Q_INVOKABLE void stopFx(const QString &param);
    Q_INVOKABLE void stopAllFx();
    /** Select the fixtures of an FX of the programmer and open its parameter */
    Q_INVOKABLE void selectFx(int index);
    /** How many sets the selection makes with the grouping of the FX of $param */
    Q_INVOKABLE int fxSets(const QString &param) const;
    /** Each fixture of the FX of $param at $cyclePos, for a preview (blind too) */
    Q_INVOKABLE QVariantList fxPreview(const QString &param, qreal cyclePos) const;

    QList<FXScene::Layer> fxLayerList() const;
    /** A macro on the selection: its layers where the fixtures have the parameter,
     *  and its base values. @return how many layers went on */
    int applyFxMacro(const QList<FXScene::Layer> &layers, const QMap<QString, int> &bases);
    /** The base values of the parameters of $layers, on the last selected fixture */
    QMap<QString, int> fxBaseValues(const QList<FXScene::Layer> &layers) const;
    /** Start the FX again after everything was stopped */
    void restartFx();
    bool isFxRunning() const;

    /*********************************************************************
     * Console tools: highlight, locate, one fixture at a time
     *********************************************************************/
    bool highlight() const;
    void setHighlight(bool on);
    /** Locate: the selection at full, open white, centred and with an open beam */
    Q_INVOKABLE void locate();
    /** One fixture of the selection at a time */
    Q_INVOKABLE void nextFixture();
    Q_INVOKABLE void previousFixture();
    /** Back to the whole selection after going through it */
    Q_INVOKABLE void allFixtures();
    QVariantMap stepping() const;
    /** Add $delta (DMX) to the dimmer of the selection, from its value now.
     *  @return how many fixtures have one */
    Q_INVOKABLE int nudgeDimmer(int delta);

    /*********************************************************************
     * Palettes: the values that come from one remember it, so a change of
     * the palette reaches the programmer and the scenes recorded with it
     *********************************************************************/
    void setPalettes(PaletteManager *palettes);
    PaletteManager *palettes() const;
    /** The programmer values, by (fixture, channel) */
    const QMap<QPair<quint32, quint32>, uchar> &valueMap() const;
    /** The palette of each programmer value that comes from one */
    QHash<QPair<quint32, quint32>, int> paletteRefs() const;
    void setPaletteRefs(const QHash<QPair<quint32, quint32>, int> &refs);
    /** Set values that come from palette $paletteId: fixture -> (channel -> value) */
    void applyPaletteValues(const QHash<quint32, QMap<quint32, uchar>> &values, int paletteId);
    /** Release the values of $fixtures that come from palette $paletteId */
    void releasePaletteValues(const QList<quint32> &fixtures, int paletteId);
    /** A value of a palette that changed: it keeps coming from it */
    void setPaletteValue(quint32 fixtureId, quint32 channel, uchar value);
    /** A palette was deleted: its values stay, as plain values */
    void forgetPalette(int paletteId);
    /** The open white of a fixture: dimmer full, shutter open, white (DMX values) */
    QMap<quint32, uchar> openWhite(const Fixture *fixture) const;
    /** The values that mix $color on $fixture (RGB, CMY, white) */
    static QMap<quint32, uchar> colourMix(const Fixture *fixture, const QColor &color);

signals:
    void selectionChanged();
    void channelsChanged();
    void valuesChanged();
    void blindChanged();
    void fxChanged();
    void fxParamsChanged();
    void lastParamChanged();
    void highlightChanged();
    void aimChanged();
    /** clear() released everything (loading a look into the programmer doesn't emit it) */
    void cleared();
    /** A scene or a step was loaded to edit it (loadScene, loadValues): the
     *  programmer doesn't hold a cue any more. loadState() alone doesn't
     *  emit it (a cue that is loaded sets itself as the one edited) */
    void lookLoaded();
    /** updateScene() saved the programmer in that scene: what shows it
     *  (channels, fixtures) reads it again. Doc says nothing of it */
    void sceneUpdated(quint32 id);

private slots:
    void slotFixtureRemoved(quint32 fixtureId);
    /** A fixture changed in the patch (address, universe, mode, options):
     *  its values go where it is now, on the same attributes */
    void slotFixtureChanged(quint32 fixtureId);
    void slotDocCleared();
    void slotFunctionStarted(quint32 id);
    /** A cuelist went to a cue (MasterTimer::cueStarted): the live edits it takes back */
    void slotCueStarted(quint32 functionId, quint32 cueUid);
    void slotSelectionChanged();

private:
    QString channelKey(quint32 fixtureId, quint32 channel) const;
    /** The attribute of each channel of a fixture, by function (cached) */
    QStringList keysOf(const Fixture *fixture) const;
    QList<quint32> spatialOrder() const;
    void setBeam(bool on);
    void notifyValues();
    void setValue(quint32 fixtureId, quint32 channel, uchar value);
    void updateHighlight();
    void setStep(int index);

    /** The FX parameter of a programmer channel key (pan and tilt: "pantilt") */
    QString paramOfKey(const QString &key) const;
    /** The selected fixtures that have $param */
    QList<quint32> selectionWith(const QString &param) const;
    /** The FX of $param on exactly the selection, or (not $exact) on part of it; -1 */
    int fxLayerFor(const QString &param, bool exact) const;
    /** $fixtures leave the FX of $param (FX left without fixtures go) */
    void removeFromFx(const QString &param, const QList<quint32> &fixtures);
    /** The fixtures whose FX channels (or their fine channels) are in $keys
     *  leave the FX of the programmer. @return true if any did */
    bool releaseFxOn(const QSet<QPair<quint32, quint32>> &keys);
    /** A cuelist went to a cue: the edits the release mode takes back. The
     *  changed channels are always drained, so that a GO with nothing to
     *  release never releases a later edit */
    void releaseForCue(Cuelist *cuelist);
    /** The FX play: the source leaves them their channels, the FX scene gets the layers */
    void syncFx();

private:
    Doc *m_doc;
    ZoneManager *m_zones;
    GenericDMXSource *m_source;

    QList<quint32> m_selection;
    /** Programmer values by (fixture, channel) */
    QMap<QPair<quint32, quint32>, uchar> m_values;
    bool m_blind;
    QString m_releaseMode;

    /** The FX of the programmer: a scene that is not in the show */
    FXScene *m_fx;
    QList<FXScene::Layer> m_fxLayers;
    /** The channels the FX move (the source doesn't send them) */
    QSet<QPair<quint32, quint32>> m_fxChannels;
    QString m_lastParam;

    PaletteManager *m_palettes;
    /** The palette each value comes from */
    QHash<QPair<quint32, quint32>, int> m_paletteRefs;

    /** Highlight: a source over everything (Override) */
    GenericDMXSource *m_highlightSource;
    bool m_highlight;
    /** Going through the selection: the whole one, and where we are */
    QList<quint32> m_stepBase;
    int m_stepIndex;
    bool m_inStep;
    /** keysOf() by fixture, with the mode they were made for */
    mutable QHash<quint32, QPair<const void *, QStringList>> m_keyCache;
    /** The attribute of each channel of the fixtures with values, when their
     *  values were set: a fixture that changes mode keeps them on the same
     *  attributes (its dimmer may be on another channel now) */
    QHash<quint32, QStringList> m_valueKeys;

    /** The fan of the position: the fixtures it moves (in order), where
     *  each one was (its centre), what it wrote (moved since: it's gone) */
    qreal m_fanPan = 0.0;
    qreal m_fanTilt = 0.0;
    QString m_fanMode = QStringLiteral("spread");
    QList<quint32> m_fanFixtures;
    QHash<quint32, QPointF> m_fanBases;
    QHash<quint32, QPointF> m_fanWritten;
    /** The fixtures of the selection with a position, in its order */
    QList<quint32> positionedSelection() const;
    /** Whether the fan is still what the fixtures have */
    bool fanHolds() const;
    /** The fan on the fixtures, from their centres */
    void applyPositionFan();
    /** The position of one fixture, without telling (the fan writes them all);
     *  with onlyMoves, only the axes that change */
    void writePosition(quint32 fixtureId, qreal x, qreal y, bool onlyMoves = false);

    /** Follow: the patch, what aim() gives, where each head aimed (degrees) */
    PatchManager *m_patch = nullptr;
    QVariantMap m_aim;
    QHash<quint32, QPointF> m_aimAngles;
    /** What writeAim wrote: changed another way (a palette, a fader), the heads aim no more */
    QHash<QPair<quint32, quint32>, uchar> m_aimWritten;
    bool aimHolds() const;
    /** The position was set another way: no aim */
    void dropAim();
    /** Pan and tilt of one fixture as they leave it (0..1 of their travel):
     *  through the inversions, limits and swap of the patch */
    void writeAim(quint32 fixtureId, qreal pan, qreal tilt);
};

#endif
