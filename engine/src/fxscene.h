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
class FXScene final : public Function
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

        bool isEmpty() const;
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
        /** linked layers share the speed and the timing (the UI keeps them equal) */
        bool linked = false;
        /** its fixtures in their order; empty: all the fixtures of the scene */
        QList<quint32> fixtures;
        /** DMX Total Control: the fixtures that do something their own way */
        QMap<quint32, Own> own;

        bool isPanTilt() const { return param == QLatin1String("pantilt"); }
        /** The shape of the layer, and the one of $fixture with what it does its own way */
        Shape shape() const;
        Shape shapeOf(quint32 fixture) const;
        /** $own without what the layer doesn't use, what is the same as the
         *  layer and the fixtures it doesn't have (with fixtures of its own) */
        QMap<quint32, Own> tidyOwn() const;
        /** DMX Total Control: the fields a layer doesn't use (and doesn't save) don't count:
         *  figure and swingTilt without pan/tilt, groupSize without grouping, seed
         *  unless the order is random; neither does what $own has in vain (tidyOwn) */
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

    void setLayers(const QList<Layer> &layers);
    QList<Layer> layers() const;

    /** The duration of one run of the slowest layer, in ms */
    uint cycleDuration() const;

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
    /** The head (0..) channel $channel of $fixture belongs to, on fixtures with
     *  several heads; -1 for the channels of the whole fixture */
    static int headOf(const Fixture *fixture, quint32 channel);

    /** The set (0..sets-1) of each fixture of $order once grouped, and how many sets */
    static QList<int> groupSets(const Doc *doc, const QList<quint32> &order, int grouping, int groupSize, int *sets);
    /** The fixtures of a layer in the order of the effect */
    static QList<quint32> orderedFixtures(const Layer &layer, const QList<quint32> &fixtures);

    /** Where in the wave (0..1) the set $set of $sets is at $cyclePos (cycles since the start),
     *  or -1 when it rests at its base (step, wait) */
    static qreal positionOf(const Layer &layer, qreal cyclePos, int set, int sets);
    /** The same for a fixture of the layer with $shape (its phase, its mode) */
    static qreal positionOf(const Layer &layer, const Shape &shape, qreal cyclePos, int set, int sets);
    /** The point (each axis -1..1) a pan/tilt layer with $shape is at, at $pos:
     *  the figure, its direction, rotation and inversions */
    static QPointF panTiltPoint(const Shape &shape, qreal pos);
    /** Whether $shape draws its figure mirrored: backwards, or with one axis inverted */
    static bool isFlipped(const Shape &shape);
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

private slots:
    void slotFixtureRemoved(quint32 fxi_id) override;

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

    /** DMX Total Control: a pan/tilt layer ("pantilt", "pan" or "tilt") */
    static bool isMotion(const QString &param);

protected:
    /** The motion speed goes to the pan/tilt layers only */
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
    };

    void buildTargets();
    QSharedPointer<GenericFader> getFader(Universe *universe);

    QList<Target> m_targets;
    /** DMX Total Control: the fixtures of each layer in the order of the effect */
    QList<QList<quint32>> m_layerOrders;
    QList<qreal> m_layerPos;
    bool m_valuesStarted;
    /** DMX Total Control: real ms since the start (elapsed() counts at the
     *  live speed): the waves grow in with the fades of the bases, which
     *  FadeChannels run in real ms */
    quint32 m_fadeElapsed;
    /** DMX Total Control: the pause the faders hold (setFadersPaused) */
    bool m_fadersPaused;
    /** DMX Total Control: the faders of its last stops, while they fade out */
    QList<QSharedPointer<GenericFader>> m_fadingOut;
};

/** @} */

#endif
