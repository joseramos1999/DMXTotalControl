/*
  DMX Total Control
  fxscene.h

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

#ifndef FXSCENE_H
#define FXSCENE_H

#include <QPointF>
#include <QMutex>
#include <QList>
#include <QHash>
#include <QMap>
#include <functional>
#include <optional>

#include "function.h"
#include "dmxsource.h"

class GenericFader;
class QLCChannel;
class Fixture;
class Doc;

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLDTCFXFixture QStringLiteral("Fixture")
#define KXMLDTCFXValue   QStringLiteral("Value")
#define KXMLDTCFXLayer   QStringLiteral("Layer")
#define KXMLDTCFXOwn     QStringLiteral("Own")

/**
 * A scene with effects: base values (optional) and
 * FX layers. Each layer modulates one parameter of its fixtures (dimmer,
 * red, zoom, pan and tilt together...) around its base value or, without a
 * base, around what the scenes below send. A layer has a size (swing), a
 * speed, a shape (mode) or, for pan and tilt, a figure, and a timing that
 * spreads it over the fixtures (wave or step per X, grouping, order, wait).
 */
class FXScene final : public Function, public DMXSource
{
    Q_OBJECT
    Q_DISABLE_COPY(FXScene)

public:
    /** Live attributes: Size scales the swing of every layer, Phase adds degrees */
    enum FXAttr
    {
        Intensity = Function::Intensity,
        Size,
        Phase
    };

    /** How the fixtures are split into sets that the wave or the step treats as one */
    enum Grouping { NoGrouping = 0, Every, Block, Divide, Mirror, ByGroup };
    /** The order of the fixtures along the effect */
    enum Order { SelectionOrder = 0, Reversed, Shuffled, Sorted };

    /** The figures of the pan/tilt layers */
    enum Figure { Circle = 0, Eight, Square, Triangle, Diamond, PanLine, TiltLine, Diagonal,
                  Leaf, Lissajous, Ballyhoo, Cross, FigureCount };

    static const int ModeCount = 22;

    /** DMX Total Control: what a fixture of a layer does: its shape, size and
     *  direction (the speed and the timing are the layer's) */
    struct Shape
    {
        int mode;
        int figure;
        int swing;
        int swingTilt;
        int phase;
        int rotate;
        bool panInvert;
        bool tiltInvert;
        bool reverse;
        /** DMX Total Control: how far behind the others the fixture goes, in
         *  degrees of the wave (360: a cycle; in a step, one place), and how
         *  much later it does the FX, in ms. Always 0 for the layer itself */
        int shift = 0;
        int delay = 0;

        bool operator==(const Shape &other) const;
        bool operator!=(const Shape &other) const { return !(*this == other); }
    };

    /** DMX Total Control: what one fixture of a layer does its own way:
     *  the same FX, on the same clock and timing as the others, with
     *  another size, direction, figure... Only what differs is set */
    struct Own
    {
        std::optional<int> mode;
        std::optional<int> figure;
        std::optional<int> swing;
        std::optional<int> swingTilt;
        std::optional<int> phase;
        std::optional<int> rotate;
        std::optional<bool> panInvert;
        std::optional<bool> tiltInvert;
        std::optional<bool> reverse;
        /** DMX Total Control: its shift (degrees behind, 0-1440) and its delay
         *  (ms behind, 0-60000), set by hand. They add to the wave and the
         *  phase, and stay when the shape goes back to the layer's */
        std::optional<int> shift;
        std::optional<int> delay;

        bool isEmpty() const;
        /** Whether it does the shape, size, direction or phase its own way */
        bool hasShape() const;
        /** Whether it has a shift or a delay */
        bool hasOffset() const;
        bool operator==(const Own &other) const;
        bool operator!=(const Own &other) const { return !(*this == other); }
    };

    /** One FX layer */
    struct Layer
    {
        /** "pantilt", "pan", "tilt", "dimmer", "red", "green"... or the lowercase name of a channel */
        QString param;
        /** 1-22, the shape of the wave around the base */
        int mode = 1;
        /** pan/tilt: the figure */
        int figure = Circle;
        /** % (0-250) of half the range; with pan/tilt, of the pan */
        int swing = 50;
        int swingTilt = 50;
        /** cycles per minute; 0 freezes it where it is */
        qreal bpm = 30;
        /** the speed follows the BPM of the show */
        bool syncBpm = false;
        qreal multiplier = 1.0;
        /** the shape spreads over X sets (0: all together, -1: as many as there are) */
        int wave = 0;
        /** one set after the other, X of them (0: off, -1: all; wins over the wave) */
        int step = 0;
        int grouping = NoGrouping;
        int groupSize = 2;
        int order = SelectionOrder;
        int seed = 1;
        /** where the wave starts, in degrees */
        int phase = 0;
        /** ms at the base after each run */
        uint wait = 0;
        /** pan/tilt */
        int rotate = 0;
        bool panInvert = false;
        bool tiltInvert = false;
        bool reverse = false;
        /** DMX Total Control: its link group (0: none). The layers of a group
         *  share the speed and the timing (the UI keeps them equal) and go on
         *  one clock. Saved as Linked="1" (what older versions read: one link
         *  for all) and LinkGroup, past the first */
        int link = 0;
        bool isLinked() const { return link > 0; }
        /** its fixtures in their order; empty: all the fixtures of the scene */
        QList<quint32> fixtures;
        /** DMX Total Control: the fixtures its timing spreads over when some
         *  of them left it (a later cue took them from a layer it tracks), in
         *  their order: the ones it keeps stay in their place in the wave, the
         *  sets and the mirror. Empty: its fixtures. Not part of the FX
         *  (operator== leaves it out): it only keeps those places */
        QList<quint32> spread;
        /** DMX Total Control: the fixtures that do something their own way */
        QMap<quint32, Own> own;
        /** DMX Total Control: a dimmer layer on fixtures without a dimmer dims
         *  their colour (a virtual dimmer) from this level, 0-255; -1: auto
         *  (the base of the real dimmers of the layer, or full) */
        int virtualBase = -1;

        bool isPanTilt() const { return param == QLatin1String("pantilt"); }
        /** The shape of the layer, and the one of $fixture with what it does its own way */
        Shape shape() const;
        Shape shapeOf(quint32 fixture) const;
        /** $own without what the layer doesn't use, what is the same as the
         *  layer and the fixtures it doesn't have (with fixtures of its own) */
        QMap<quint32, Own> tidyOwn() const;
        /** DMX Total Control: the fields a layer doesn't use (and doesn't save) don't count:
         *  figure and swingTilt without pan/tilt, groupSize without grouping, seed
         *  unless the order is random, virtualBase but on the dimmer; neither does
         *  what $own has in vain (tidyOwn) */
        bool operator==(const Layer &other) const;
        bool operator!=(const Layer &other) const { return !(*this == other); }
    };

public:
    FXScene(Doc *doc);
    ~FXScene();

    QIcon getIcon() const override;
    quint32 totalDuration() override;

    Function *createCopy(Doc *doc, bool addToDoc = true) override;
    bool copyFrom(const Function *function) override;

    /*********************************************************************
     * Contents
     *********************************************************************/
public:
    /** The fixtures of the scene, in their order */
    void setFixtures(const QList<quint32> &fixtures);
    QList<quint32> fixtures() const;

    /** Base values: (fixture, channel) -> value */
    void setValues(const QHash<QPair<quint32, quint32>, uchar> &values);
    QHash<QPair<quint32, quint32>, uchar> values() const;
    void setValue(quint32 fixture, quint32 channel, uchar value);

    /** The layers. DMX Total Control: with none that follows the BPM any
     *  more, the scene has no tapped rhythm (setTapPeriod(0)). Each layer
     *  goes on from where it was; a new one on the clock of what plays (the
     *  layer its fixtures come from, or one of its parameter at the same
     *  speed), and one that joins a link group on the clock of the group */
    void setLayers(const QList<Layer> &layers);
    QList<Layer> layers() const;
    /** DMX Total Control: where each layer is, in cycles (since it started;
     *  on the beat of the show, the beat clock by its multiplier) */
    QList<qreal> layerPositions() const;

    /** DMX Total Control: whether a layer follows the BPM of the show. The
     *  scene then takes a rhythm tapped on its button, as steps do: those
     *  layers go at it (a cycle at x1 a beat, setTapPeriod) instead of the
     *  BPM of the show. The first beat of a row of presses (tapBeat) starts
     *  their cycle again; each next one puts them where they are at it (a
     *  cycle at x1 a beat from the first: a chase on its next set, x1/4 a
     *  quarter further) */
    bool hasSyncedLayer() const;
    /** DMX Total Control: a layer that follows the BPM of a scene with no
     *  tapped rhythm goes on the beat of the show: its place is the beat
     *  clock (MasterTimer::beatPosition) by its multiplier, so any scene,
     *  started at any time, is on the same beat; the show TAP puts it back on
     *  it. A press of the scene's button, a pause or another speed (live or
     *  of the movement) take it off it until then; a new multiplier puts it
     *  back. Whether every layer does (and there is one) */
    bool followsShowBeat() const;

    /** The duration of one run of the slowest layer, in ms. DMX Total
     *  Control: as they play (the layers that follow the BPM at its tapped
     *  rhythm or at the BPM of the show, the whole pass with a wait, a step
     *  over the sets of its grouping) */
    uint cycleDuration() const;
    /** DMX Total Control: how long until every layer is back where it
     *  started (ms): the shortest common length of their runs, up to
     *  $longest; else the one of the slowest */
    uint loopDuration(uint longest) const;
    /** DMX Total Control: the same in beats of the show, for a scene that
     *  follows its beat (followsShowBeat): a whole number of thousandths of a
     *  beat, up to $longest beats; 0 when there is none */
    qreal loopBeats(qreal longest) const;

    /*********************************************************************
     * The shapes
     *********************************************************************/
public:
    /** The wave of $mode (1-22) at $pos (0..1): -1..1 around the base */
    static qreal modeValue(int mode, qreal pos);
    /** Whether a mode travels from the last fixture to the first */
    static bool modeReversed(int mode);
    /** The point of $figure at $pos (0..1), each axis -1..1 */
    static QPointF figurePoint(int figure, qreal pos);

    /** What a channel does, whatever its name: "dimmer", "red", "pan", "colorwheel",
     *  "gobo", "shutter", "zoom"... (from its preset, else its group and name).
     *  Channels without a known function: their name in lower case */
    static QString attributeOf(const QLCChannel *channel);
    /** The parameter an FX on this channel modulates (its attribute), or
     *  an empty string for fine channels */
    static QString paramOf(const QLCChannel *channel);
    /** The channel of $fixture for $param ("pan" and "tilt" for "pantilt": use those),
     *  and its fine channel, or QLCChannel::invalid(). On fixtures with several heads,
     *  the channel of the whole fixture (a master dimmer) comes before the ones of a head.
     *  Shows saved with channel names as parameters still find their channel */
    static quint32 channelFor(const Fixture *fixture, const QString &param, quint32 *fine = nullptr);
    /** DMX Total Control: the channels an FX on $param moves on $fixture: the
     *  one of channelFor, and on a fixture with several heads without one of
     *  its own for it, the one of every head (each with its fine channel in
     *  $fines, or QLCChannel::invalid()). All of them do the same */
    static QList<quint32> channelsFor(const Fixture *fixture, const QString &param, QList<quint32> *fines = nullptr);
    /** The parameter an FX on channel $channel of $fixture modulates: its
     *  paramOf(), but a dimmer of the whole fixture that is not its master
     *  (the flash intensity of a strobe with a colour aura) is a parameter
     *  of its own, its name in lower case, so an FX can still move it */
    static QString fixtureParamOf(const Fixture *fixture, quint32 channel);
    /** The dimmers a dimmer level (the programmer's «Dimmer», @ FULL, a
     *  dimmer palette of another fixture type) sets on $fixture: its master
     *  dimmer, the dimmer of each head, and the other dimmers of the whole
     *  fixture that are a plain level of light (the two lamps of a twin spot,
     *  pixels, a background layer). Not the flash intensity of a strobe with
     *  an aura, an effect, the fog of a fogger with LEDs, a lamp control, nor
     *  the hue or saturation of an HSI fixture. An HSL/HSV mode without a
     *  dimmer: its lightness */
    static QList<quint32> levelDimmerChannels(const Fixture *fixture);
    /** The head (0..) channel $channel of $fixture belongs to, on fixtures with
     *  several heads; -1 for the channels of the whole fixture */
    static int headOf(const Fixture *fixture, quint32 channel);

    /** DMX Total Control: the virtual dimmer of a fixture without a dimmer (RGB
     *  only...): its emitters (red, green, blue, white, amber, UV, lime,
     *  indigo) that the grand master scales, each with its fine channel or
     *  QLCChannel::invalid(). A dimmer FX dims them all together. Empty when
     *  the fixture has a dimmer (channelFor finds one) */
    static QList<QPair<quint32, quint32>> virtualDimmerChannels(const Fixture *fixture);
    /** DMX Total Control: whether an FX on $param moves something of $fixture:
     *  pan and tilt for "pantilt", its channel, or a virtual dimmer for "dimmer" */
    static bool hasParam(const Fixture *fixture, const QString &param);
    /** DMX Total Control: the channels (and their fine channels, or
     *  QLCChannel::invalid()) a layer on $param plays on $fixture, the
     *  emitters of a virtual dimmer included */
    static QList<QPair<quint32, quint32>> layerChannels(const Fixture *fixture, const QString &param);
    /** DMX Total Control: the level the virtual dimmer of a dimmer layer starts
     *  from when it is auto: the base in $values of the first fixture of
     *  $order with a real dimmer that has one (a mixed rig goes together),
     *  else full (255) */
    static int autoVirtualBase(const Doc *doc, const QList<quint32> &order,
                               const QHash<QPair<quint32, quint32>, uchar> &values);

    /** The set (0..sets-1) of each fixture of $order once grouped, and how many sets.
     *  DMX Total Control: «Divide» makes min(X, fixtures) sets, one apart in size
     *  at most. «By group» takes the fixture groups of the patch that split the
     *  fixtures into sets that don't overlap, a group with all of them left
     *  out: the finest split, then the one that leaves fewer alone, then the
     *  smaller groups, then the order of the patch. A fixture in none of them
     *  is a set of its own. $setGroups: the group of each set
     *  (FixtureGroup::invalidId() for a fixture alone, or another grouping) */
    static QList<int> groupSets(const Doc *doc, const QList<quint32> &order, int grouping, int groupSize, int *sets,
                                QList<quint32> *setGroups = nullptr);
    /** The fixtures of a layer in the order of the effect. DMX Total Control:
     *  «By number» by the numbers of the patch of $doc (setPatchOrder; the
     *  ones without a number after them, by ID) */
    static QList<quint32> orderedFixtures(const Layer &layer, const QList<quint32> &fixtures, const Doc *doc = nullptr);
    /** DMX Total Control: the fixtures the timing of a layer spreads over, in
     *  the order of the effect: its spread (some of them left it), then its
     *  fixtures that are not in it; else orderedFixtures() */
    static QList<quint32> timingOrder(const Layer &layer, const QList<quint32> &fixtures, const Doc *doc = nullptr);

    /** DMX Total Control: what the patch of $doc says that the FX go by: the
     *  number of each fixture and the order of its groups. The app tells it
     *  when they change; a copy, so the timer thread reads it too */
    static void setPatchOrder(const Doc *doc, const QHash<quint32, int> &numbers, const QList<quint32> &groupOrder);
    static void clearPatchOrder(const Doc *doc);
    /** DMX Total Control: the patch order of $from for $to too (a copy of
     *  its fixtures, under their IDs: an FX rendered offline) */
    static void copyPatchOrder(const Doc *from, const Doc *to);
    /** DMX Total Control: the number of $fixture in the patch of $doc (0: none) */
    static int fixtureNumber(const Doc *doc, quint32 fixture);
    /** DMX Total Control: $fixtures by their numbers in the patch of $doc (the
     *  ones without one after them, by ID) */
    static void sortByNumber(const Doc *doc, QList<quint32> &fixtures);
    /** DMX Total Control: where $fixture goes in $order by its number: before
     *  the first one with a higher number, or a lower one when $order goes
     *  down; at the end when there is none */
    static int placeByNumber(const Doc *doc, const QList<quint32> &order, quint32 fixture);

    /** Where in the wave (0..1) the set $set of $sets is at $cyclePos (cycles since the start),
     *  or -1 when it rests at its base (step, wait); at the BPM of the layer */
    static qreal positionOf(const Layer &layer, qreal cyclePos, int set, int sets);
    /** The same for a fixture of the layer with $shape (its phase, its mode). DMX Total
     *  Control: the layer going at $bpm (x1), which turns its wait (ms) into cycles: for
     *  a layer that follows the BPM, the BPM of the show or its tapped rhythm (syncedBpm) */
    static qreal positionOf(const Layer &layer, const Shape &shape, qreal cyclePos, int set, int sets, qreal bpm);
    /** DMX Total Control: the delay of $shape in cycles of $layer going at
     *  $bpm (x1), as its wait: when the FX starts, the fixture waits at its
     *  base until the layer has gone that far */
    static qreal delayCycles(const Layer &layer, const Shape &shape, qreal bpm);
    /** DMX Total Control: the length of a pass of $layer (with $sets sets)
     *  going at $bpm (x1), in cycles: a step over its sets, or a wave with a
     *  wait, and the wait; 0 for a wave without a wait. One that follows the
     *  BPM with a wait lasts a whole number of beats (at least the run), so
     *  each pass starts on a beat */
    static qreal passCycles(const Layer &layer, int sets, qreal bpm);
    /** DMX Total Control: the longest delay of a fixture of its layers, in ms */
    uint longestDelay() const;
    /** DMX Total Control: the BPM the layers that follow it go at: its tapped rhythm,
     *  or else the BPM of the show */
    qreal syncedBpm() const;
    /** The point (each axis -1..1) a pan/tilt layer with $shape is at, at $pos:
     *  the figure, its direction, rotation and inversions */
    static QPointF panTiltPoint(const Shape &shape, qreal pos);
    /** Whether $shape draws its figure mirrored: backwards, or with one axis inverted */
    static bool isFlipped(const Shape &shape);
    /** DMX Total Control: whether $figure going backwards is its mirror in pan
     *  (a circle, a square, a pan line...: not an eight, a diagonal...) */
    static bool reverseMirrorsPan(int figure);
    /** DMX Total Control: a pan/tilt layer whose fixtures that go the other way
     *  round (backwards, a pan or tilt inverted of their own) are all on one side
     *  of the effect $order is a mirror: each of them takes the place in the wave
     *  of its twin, the fixture in the mirror place on the other side (the 3
     *  last of 6 go with the 3rd, the 2nd and the 1st). For each place of
     *  $order, whether it does; empty when the layer is not a mirror */
    static QList<bool> mirrorTwins(const Layer &layer, const QList<quint32> &order);

    /** For the previews: the value of each fixture of a layer (-1..1, or the x/y
     *  pairs of pan/tilt) at $cyclePos */
    QVariantList previewLayer(int index, qreal cyclePos) const;
    /** DMX Total Control: the same while it plays where the layer is, as it
     *  goes out (its speed, the speed of the movement, its delays); stopped,
     *  at $cyclePos */
    QVariantList livePreviewLayer(int index, qreal cyclePos) const;

private slots:
    void slotFixtureRemoved(quint32 fxi_id) override;
    /** DMX Total Control: a fixture of the FX moved or changed its mode */
    void slotFixtureChanged(quint32 fxi_id);

private:
    /** Take out what is on the fixtures $remove says (values, layers that
     *  lose their last fixture). @return whether anything went */
    bool removeFixturesLocked(const std::function<bool(quint32)> &remove);

    QList<quint32> m_fixtures;
    QHash<QPair<quint32, quint32>, uchar> m_values;
    QList<Layer> m_layers;
    /** layers and values change from the UI while the effect runs */
    mutable QMutex m_mutex;
    bool m_targetsDirty;

    /*********************************************************************
     * Save & Load
     *********************************************************************/
public:
    bool saveXML(QXmlStreamWriter *doc) const override;
    bool loadXML(QXmlStreamReader &root) override;
    /** DMX Total Control: the fixtures that are not in the show leave it (a
     *  fixture patched later would get their IDs) */
    void postLoad() override;

    static QString groupingToString(int grouping);
    static int stringToGrouping(const QString &str);
    static QString orderToString(int order);
    static int stringToOrder(const QString &str);

    /** A <Layer> element: shared with the cuelists, which keep FX layers in their cues */
    static void saveLayerXML(QXmlStreamWriter *doc, const Layer &layer);
    /** The layer of a <Layer> element's attributes (its param is empty when it has none) */
    static Layer loadLayerXML(const QXmlStreamAttributes &attrs);
    /** The layer of the <Layer> element $root is at, with what its fixtures
     *  do their own way: it reads the whole element */
    static Layer loadLayerXML(QXmlStreamReader &root);

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    void preRun(MasterTimer *timer) override;
    void write(MasterTimer *timer, QList<Universe *> universes) override;
    void postRun(MasterTimer *timer, QList<Universe *> universes) override;

    int adjustAttribute(qreal fraction, int attributeId) override;

    /** DMX Total Control: holds or resumes its faders (a base fading in, a
     *  static value) and the ones its last stops left fading out, in the timer
     *  thread. write() makes them follow isPaused(); a function that runs the
     *  scene inside its own frames and doesn't write it while paused (the FX
     *  player of a cuelist) calls it itself */
    void setFadersPaused(bool paused);

    /** DMX Total Control: the next change of what it plays (setFixtures,
     *  setValues, setLayers before its next frame) fades instead of snapping
     *  as a live edit does: a base goes to its new value, a layer grows in,
     *  takes its new size and shape, or leaves, moving as it fades to what is
     *  below, over $ms after $delayMs, on $curve (FadeChannel::Curve). The
     *  channels (fixture, channel) in $times take their own (ms, delay).
     *  Stopped, its start fades so. A cuelist gives its FX player the times of
     *  each cue */
    void setChangeFade(uint ms, uint delayMs, int curve = 0,
                       const QHash<QPair<quint32, quint32>, QPair<uint, uint>> &times = {});
    /** DMX Total Control: the fades that run (its fade in, a change, what
     *  leaves or fades out) take $factor times as long from where they are;
     *  0 or less ends them now (the Live Time of a cuelist) */
    void scaleFades(qreal factor);
    /** DMX Total Control: the base channel $channel (or its fine channel) of
     *  $fixture holds now under its FX, where its fade is (8 bit); -1 when it
     *  has none (an FX on what is below, or none) */
    int heldBase(quint32 fixture, quint32 channel) const;

    /** DMX Total Control: what its stops left fading out goes on moving
     *  (a DMX source of the timer while they last) */
    void writeDMX(MasterTimer *timer, QList<Universe *> universes) override;

    /** DMX Total Control: a pan/tilt layer ("pantilt", "pan" or "tilt") */
    static bool isMotion(const QString &param);

    /** DMX Total Control: the beats since it started: a cycle at x1 of the
     *  layers that follow the BPM, at its tapped rhythm or else the beats of
     *  the show, and each tapped beat but the next one of a row less than half
     *  a beat after a beat of that clock, which only times the next ones (the
     *  rhythm bars of its buttons). Counts round past the largest int, never
     *  below 0 */
    int beats() const;

protected:
    /** The motion speed goes to the pan/tilt layers only (DMX Total Control:
     *  and to the layers linked to one) */
    qreal frameMotion() const override;

private:
    /** A channel the scene plays */
    struct Target
    {
        quint32 fixture;
        quint32 universe;
        quint32 channel;
        /** the fine channel, or QLCChannel::invalid() */
        quint32 fine;
        /** the layer that modulates it, -1 for a static value */
        int layer;
        /** pan/tilt layers: the tilt axis */
        bool tilt;
        int set;
        int sets;
        /** DMX Total Control: its place in the order of the layer, and the set
         *  of its twin (the fixture in the mirror place: mirrorTwins) */
        int index;
        int twinSet;
        bool hasBase;
        quint32 base;
        /** DMX Total Control: the virtual dimmer that dims it (m_scalers), or -1 */
        int scaler = -1;
        /** DMX Total Control: an FX without a base, on what is below: in the
         *  faders after the ones of its priority (getFader) */
        bool relative = false;
    };

    /** DMX Total Control: where a fixture of the FX is in the patch, when
     *  its targets were found */
    struct PatchStamp
    {
        quint32 universe = 0;
        quint32 address = 0;
        quint32 channels = 0;
        const void *mode = nullptr;
        static PatchStamp of(const Fixture *fixture);
        bool operator!=(const PatchStamp &other) const
        {
            return universe != other.universe || address != other.address || channels != other.channels ||
                   mode != other.mode;
        }
    };

    /** DMX Total Control: the virtual dimmer of a fixture without a dimmer: a
     *  dimmer layer on it scales its emitters (the targets with its index) */
    struct Scaler
    {
        quint32 fixture;
        int layer;
        int set;
        int sets;
        int index;
        int twinSet;
        /** the level it dims from when the layer's is auto (autoVirtualBase) */
        int autoBase;
    };

    typedef QPair<quint32, quint32> Key;

    /** DMX Total Control: a change that fades (setChangeFade): the time and
     *  the delay of each channel, ms */
    struct Change
    {
        uint fade = 0;
        uint delay = 0;
        int curve = 0;
        QHash<Key, QPair<uint, uint>> times;
        QPair<uint, uint> timeOf(const Key &key) const { return times.value(key, qMakePair(fade, delay)); }
    };

    /** DMX Total Control: what the layers played, going on as it was: under
     *  a change that fades until the new look is in (each channel mixes from
     *  it to the new one), or what a stop leaves fading out */
    struct Look
    {
        QList<Layer> layers;
        QList<Target> targets;
        QHash<Key, int> index;
        QList<Scaler> scalers;
        QList<QList<bool>> twins;
        QList<qreal> pos;
        QList<qreal> age;
        /** the cycles each layer goes in a real ms */
        QList<qreal> rate;
        /** each layer: the layer of the next look it goes on as (its place), -1: none */
        QList<int> next;
        /** the change to the next look, and the real ms since it came */
        Change change;
        quint64 elapsed = 0;
        /** how far (0..1) $key is on its way to the next look */
        qreal progress(const Key &key) const;
        /** whether every channel is there */
        bool done() const;
    };

    /** DMX Total Control: what a stop with a fade out left: it goes on moving
     *  in its faders while they fade out (writeDMX) */
    struct Tail
    {
        QList<Look> looks;
        QList<QSharedPointer<GenericFader>> faders;
        qreal phaseShift = 0;
        qreal size = 1;
        qreal fadeFactor = 1;
        qreal syncedBpm = 120;
    };

    /** DMX Total Control: what a target sends besides its base: its wave (a
     *  fraction of its range, before the size) and its virtual dimmer */
    struct Wave
    {
        qreal wave = 0.0;
        qreal scale = 1.0;
    };

    void buildTargets();
    /** DMX Total Control: previewLayer, under the lock; $live: how far it went
     *  since it started, -1: $cyclePos */
    QVariantList previewLocked(int index, qreal cyclePos, qreal live) const;
    /** DMX Total Control: the scale (0..1) of each virtual dimmer of $look at
     *  this frame, as the dimmer layer would set a dimmer */
    static QList<qreal> scalerLevels(const Look &look, qreal phaseShift, qreal syncedBpm,
                                     qreal size, qreal fadeFactor);
    /** DMX Total Control: the wave of target $index of $look */
    static Wave waveIn(const Look &look, const QList<qreal> &scales, int index, qreal phaseShift, qreal syncedBpm);
    /** DMX Total Control: the wave of $key through the looks of $chain up to
     *  $upTo, each one on its way to the next one ($scales: of each look) */
    static Wave chainWave(const QList<const Look *> &chain, const QList<QList<qreal>> &scales, const Key &key,
                          int upTo, qreal phaseShift, qreal syncedBpm);
    /** DMX Total Control: the waves of the channels of $chain that are not in
     *  its last look, each one as the last look it is in has it */
    static QHash<Key, Wave> leavingWaves(const QList<const Look *> &chain, const QList<QList<qreal>> &scales,
                                         qreal phaseShift, qreal syncedBpm);
    /** DMX Total Control: the looks of $looks go on for $ms real ms (the
     *  layers that go on in the next one, or $last after the last one, at
     *  its place) */
    static void advanceLooks(QList<Look> &looks, const QList<qreal> &lastPos, const QList<qreal> &lastAge, uint ms);
    /** DMX Total Control: the waves of $waves into the channels of $faders
     *  that leave their FX */
    static void writeLeaving(const QList<QSharedPointer<GenericFader>> &faders, const QHash<Key, Wave> &waves,
                             qreal size, qreal fadeFactor);
    /** DMX Total Control: what plays now, as a look */
    Look currentLookLocked() const;
    /** DMX Total Control: its fade in, ms (in beats when it was given in
     *  beats), by the Live Time */
    uint fadeInTime(MasterTimer *timer) const;
    /** DMX Total Control: what plays now goes on under the change that comes
     *  (once per change) */
    void keepLookLocked();
    /** DMX Total Control: its fader of $universe for the FX without a base
     *  ($relative: after the faders of the same priority), or for the rest */
    QSharedPointer<GenericFader> getFader(Universe *universe, bool relative = false);
    /** DMX Total Control: the key of that fader in m_fadersMap: the ID of the
     *  universe, past RelativeFaders for the FX without a base */
    static quint32 faderKey(quint32 universe, bool relative);
    static const quint32 RelativeFaders = 0x10000;

    QList<Target> m_targets;
    /** DMX Total Control: the target of each channel (fixture, channel) */
    QHash<Key, int> m_targetIndex;
    /** DMX Total Control: where its fixtures were in the patch (buildTargets) */
    QHash<quint32, PatchStamp> m_patchStamps;
    /** DMX Total Control: the looks under a change that fades, oldest first */
    QList<Look> m_looks;
    /** DMX Total Control: the change the next frame fades (setChangeFade),
     *  and whether what played went on under it already */
    Change m_change;
    bool m_changeSet = false;
    bool m_lookKept = false;
    /** DMX Total Control: what its stops with a fade out left, and whether
     *  the timer runs it (a DMX source) */
    QList<Tail> m_tails;
    bool m_tailSource = false;
    /** DMX Total Control: the cycles each layer goes in a real ms */
    QList<qreal> m_layerRate;
    /** DMX Total Control: the length of its fade in, by the Live Time */
    qreal m_fadeScale = 1.0;
    /** DMX Total Control: the virtual dimmers of the targets */
    QList<Scaler> m_scalers;
    /** DMX Total Control: the fixtures of each layer in the order of the effect */
    QList<QList<quint32>> m_layerOrders;
    QList<qreal> m_layerPos;
    /** DMX Total Control: for each layer, the cycles since it started (its
     *  delays wait on them), the length of its pass at the last frame (cycles,
     *  0: none: passCycles) and the multiplier it went at on the beat of the
     *  show (-1: it didn't, it goes back to that beat) */
    QList<qreal> m_layerAge;
    QList<qreal> m_layerPass;
    QList<qreal> m_layerOnBeat;
    /** DMX Total Control: the sets of each layer, as its targets have them */
    QList<int> m_layerSets;
    /** DMX Total Control: the beat clock at the last frame (-1: none yet),
     *  and the beats it was moved by (MasterTimer::beatAnchors) */
    qreal m_lastBeat = -1.0;
    int m_lastAnchors = 0;
    bool m_valuesStarted;
    /** DMX Total Control: real ms since the start (elapsed() counts at the
     *  live speed): the waves grow in with the fades of the bases, which
     *  FadeChannels run in real ms */
    quint32 m_fadeElapsed;
    /** DMX Total Control: the pause the faders hold (setFadersPaused) */
    bool m_fadersPaused;
    /** DMX Total Control: where in its beat it is (timer thread), whether
     *  its last beat was one of that clock (not tapped), and the beats so far
     *  (beats(), read by the UI) */
    qreal m_beatPos = 0.0;
    bool m_beatTimed = false;
    QAtomicInt m_beats;
};

/** @} */

#endif
