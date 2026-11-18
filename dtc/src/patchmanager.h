/*
  DMX Total Control
  patchmanager.h

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

#ifndef PATCHMANAGER_H
#define PATCHMANAGER_H

#include <QObject>
#include <QUrl>
#include <QSizeF>
#include <QPointF>
#include <QHash>
#include <QPair>
#include <QStringList>
#include <QVariant>
#include <functional>

#include "aim.h"

class GenericDMXSource;
class ChannelModifier;
class QLCFixtureMode;
class QLCFixtureDef;
class Fixture;
class QTimer;
class QXmlStreamReader;
class QXmlStreamWriter;
class QXmlStreamAttributes;
class Doc;

#define KGenericDimmerModel QStringLiteral("Generic Dimmer")
#define KXMLDTCSelections QStringLiteral("Selections")
#define KXMLDTCPlan QStringLiteral("Plan")
#define KXMLDTCNumbers QStringLiteral("Numbers")
#define KXMLDTCGroupOrder QStringLiteral("GroupOrder")

/**
 * Fixture library browsing and fixture patching
 */
class PatchManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList fixtures READ fixtures NOTIFY fixturesChanged)
    Q_PROPERTY(int fixturesCount READ fixturesCount NOTIFY fixturesChanged)
    Q_PROPERTY(QVariantList identified READ identified NOTIFY identifiedChanged)
    Q_PROPERTY(QVariantList groups READ groups NOTIFY groupsChanged)
    Q_PROPERTY(QSizeF stageSize READ stageSize NOTIFY stageSizeChanged)
    Q_PROPERTY(QVariantMap stageRoom READ stageRoom NOTIFY stageRoomChanged)
    Q_PROPERTY(QVariantList selections READ selections NOTIFY selectionsChanged)
    Q_PROPERTY(QVariantList fixtureTypes READ fixtureTypes NOTIFY fixturesChanged)
    Q_PROPERTY(QStringList fixtureKinds READ fixtureKindList CONSTANT)
    /** Some fixture of the show is a protected special effect */
    Q_PROPERTY(bool hasProtected READ hasProtected NOTIFY protectionChanged)
    /** How many fixtures the grand master doesn't lower */
    Q_PROPERTY(int grandMasterProtectedCount READ grandMasterProtectedCount NOTIFY fixturesChanged)

public:
    PatchManager(Doc *doc, QObject *parent = nullptr);
    ~PatchManager();

    /** [{ id, name, manufacturer, model, mode, type, universe, address,
     *     channels, x, y, positioned, heads, cells, kind, icon, typeKey,
     *     colorIndex, protected, gmProtected, crossUniverse }]
     *  Addresses are 1-based, as displayed to the user.
     *  x/y is the position in the 2D stage plan, in millimeters.
     *  cells: the plan draws it as a row of cells, one per head (a bar, a
     *  matrix, a dimmer pack; not a generic dimmer standing in for one
     *  effect, a CO2 jet with two channels); kind: what its glyph on the
     *  plan shows (fixtureKind(), with the icon chosen by hand); icon: the
     *  one chosen by hand ("" when automatic); typeKey and colorIndex: its
     *  type in fixtureTypes(); protected: a protected special effect
     *  (isProtected); gmProtected: the grand master doesn't lower it
     *  (setProtectedFromGrandMaster); crossUniverse: its channels go on in
     *  the next universe (a show from QLC+) */
    QVariantList fixtures() const;
    int fixturesCount() const;
    int grandMasterProtectedCount() const;

    Q_INVOKABLE QStringList manufacturers() const;
    Q_INVOKABLE QStringList models(const QString &manufacturer) const;
    /** [{ name, channels }] */
    Q_INVOKABLE QVariantList modes(const QString &manufacturer, const QString &model) const;

    /** Find the first free 1-based address in $universe for $channels channels,
     *  or -1 if there isn't one */
    Q_INVOKABLE int nextFreeAddress(int universe, int channels) const;

    /**
     * Patch $quantity fixtures, starting at the 1-based $address.
     * For "Generic Dimmer", $channels gives the number of dimmer channels.
     * With a $groupName, the new fixtures also make a fixture group.
     * All of them or none: when one doesn't fit, nothing is patched and
     * lastError() says why.
     * @return the number of fixtures added
     */
    Q_INVOKABLE int addFixtures(const QString &manufacturer, const QString &model,
                                const QString &mode, const QString &name,
                                int universe, int address, int quantity,
                                int gap = 0, int channels = 1,
                                const QString &groupName = QString());
    /** Why the last addFixtures() patched nothing (empty when it did) */
    Q_INVOKABLE QString lastError() const;

    /**
     * Give a patched fixture another definition and mode ($def, $mode: null
     * to keep them), universe and 0-based address in one step. The timer
     * thread, which reads the fixture while the functions play, doesn't tick
     * in the middle, and Doc hears of the fixture once everything is set: in
     * between, it could overlap another fixture. The range must be free.
     * The capabilities of the universes are not set again.
     */
    static void applyPatch(Doc *doc, Fixture *fxi, QLCFixtureDef *def, QLCFixtureMode *mode,
                           quint32 universe, quint32 address);

    Q_INVOKABLE void removeFixture(quint32 id);
    Q_INVOKABLE void removeFixtures(const QVariantList &ids);
    Q_INVOKABLE void renameFixture(quint32 id, const QString &name);
    /** Move a fixture to a new 1-based address */
    Q_INVOKABLE bool setFixtureAddress(quint32 id, int universe, int address);

    /**
     * Edit a patched fixture: name, mode, universe and 1-based address.
     * The change is applied only if the new channel range is free.
     * @return an empty string on success, otherwise the reason of the failure
     */
    Q_INVOKABLE QString updateFixture(quint32 id, const QString &name, const QString &mode,
                                      int universe, int address);

    /** Re-address $ids one after the other from the 1-based $address */
    Q_INVOKABLE QString readdressFixtures(const QVariantList &ids, int universe, int address, int gap = 0);

    /*********************************************************************
     * Fixture numbers: what the command line uses (1 THRU 8 @ FULL)
     *********************************************************************/
    /** The number of a fixture (1...), 0 if it has none */
    Q_INVOKABLE int fixtureNumber(quint32 id) const;
    /** The fixture with $number, or -1 */
    Q_INVOKABLE int fixtureByNumber(int number) const;
    /** Give a fixture another number. @return an error, empty when done */
    Q_INVOKABLE QString setFixtureNumber(quint32 id, int number);
    /** Number $ids one after the other, in that order, from $start */
    Q_INVOKABLE QString renumberFixtures(const QVariantList &ids, int start);
    /** The fixtures sorted by number */
    QList<quint32> fixturesByNumber() const;

    /** The patch of another show (read with EffectExchange::loadShow): its
     *  fixtures at their address when it is free (else at the next free
     *  one), their fixture groups and their place in the 2D plan.
     *  @param copies set to the ID of the copy of each fixture, by its ID in $from
     *  @return the number of fixtures added */
    int importFixtures(Doc *from, QHash<quint32, quint32> *copies = nullptr);
    /** What the plan of that show says of the fixtures copied ($copies, as
     *  importFixtures gives them; $plan, as EffectExchange::loadShow reads
     *  it): the icon chosen for each one, and a protected special effect
     *  with its times and safety channel. Protection only ever comes with
     *  them: a copy whose kind is a protected one stays protected, whatever
     *  that show said */
    void importPlanSettings(const QHash<quint32, quint32> &copies, const QHash<quint32, QXmlStreamAttributes> &plan);

    /*********************************************************************
     * Patch import: the fixture table (CSV) a visualizer exports
     * (patchimport.cpp)
     *********************************************************************/
    /**
     * What importing the table in $file would do, with $options: { choices:
     * { typeKey: { manufacturer, model, mode } | { generic: true } |
     * { skip: true } | { auto: true } }, place, view ("top", "front"; empty:
     * the last one used, else the one that suits the rig), groupsByType,
     * groupsByLayer, groupsFromTable, removeMissing }. Nothing changes.
     * @return { ok, error, fileName, rows, universeFrom, universeTo,
     *  types: [{ key, fixture, mode, channels, count, status ("exact",
     *  "similar", "channels", "remembered", "manual", "generic", "skip"),
     *  generic, manufacturer, model, modeName, modeChannels, note,
     *  foundManufacturer, foundModel (not found: where to look by hand) }],
     *  newCount, updateCount, skippedRows, notImported, removeCount,
     *  removeNames (of the ones removeMissing takes), problems: [text],
     *  layers, hasGroups, hasPositions, view,
     *  suggestedView, stage: { width, height }, stageNow, newUniverses,
     *  options: the options in effect }
     */
    Q_INVOKABLE QVariantMap tablePreview(const QUrl &file, const QVariantMap &options = QVariantMap());
    QVariantMap tablePreviewData(const QByteArray &data, const QVariantMap &options = QVariantMap());
    /**
     * Import the table in $file as tablePreview() shows it, in one step:
     * the fixtures imported earlier from it (by their identifier, or by
     * their name and fixture type) are updated, the others added; the
     * choices of fixtures made by hand and the view are remembered.
     * @return what was done (for a message), empty when nothing could be
     *  (lastError() says why)
     */
    Q_INVOKABLE QString importTable(const QUrl &file, const QVariantMap &options = QVariantMap());
    QString importTableData(const QByteArray &data, const QVariantMap &options = QVariantMap());
    /** The identifier of the table fixture $id was imported from (empty: none) */
    QString importIdentifier(quint32 id) const;

    /*********************************************************************
     * Fixture groups
     *********************************************************************/
    /** [{ id, name, fixtures: [ids] }], in the order the user gives them
     *  (new ones at the end). The place is also the number of GROUP in the
     *  command line */
    QVariantList groups() const;

    Q_INVOKABLE int createGroup(const QString &name, const QVariantList &fixtureIds);
    Q_INVOKABLE void deleteGroup(quint32 id);
    Q_INVOKABLE void renameGroup(quint32 id, const QString &name);
    Q_INVOKABLE void setGroupFixtures(quint32 id, const QVariantList &fixtureIds);
    /** The group at $from (in groups()) goes to $to */
    Q_INVOKABLE void moveGroup(int from, int to);
    /** In alphabetical order (numbers as numbers: "Truss 2" before "Truss 10") */
    Q_INVOKABLE void sortGroups();

    /*********************************************************************
     * 2D stage plan
     *********************************************************************/
    /** Stage width/height in millimeters */
    QSizeF stageSize() const;
    /** The size of the stage, each way between stageMinimum() and
     *  stageMaximum(): it grows or shrinks on the right and at the front.
     *  Nothing moves: a fixture past a new, smaller edge stays outside it */
    Q_INVOKABLE void setStageSize(qreal width, qreal height);
    /** The smallest and the largest stage, each way (mm) */
    static constexpr qreal stageMinimum() { return 1000.0; }
    static constexpr qreal stageMaximum() { return 200000.0; }
    /**
     * Grow ($deltaMm > 0) or shrink the stage on one $side of the plan:
     * "left", "right", "top" (the back of the stage) or "bottom" (the
     * front, the audience side). On the left or at the top, everything on
     * the plan moves with that side: each fixture with its heads, the items
     * of QLC+'s 3D view and the point Follow aims at, so they stay where
     * they are on the real stage. Nothing changes, and it returns false,
     * when growing would make the stage larger than stageMaximum(), when
     * shrinking would make it smaller than stageMinimum() (a stage already
     * larger than stageMaximum(), from QLC+, still shrinks), or when
     * shrinking would leave a positioned fixture on the new edge or past it
     * (see stageRoom()). One stageSizeChanged, and one fixturesChanged when
     * they moved (the left or the top)
     */
    Q_INVOKABLE bool resizeStageSide(const QString &side, qreal deltaMm);
    /** The room free of fixtures between each side of the stage and the
     *  nearest positioned fixture, in mm: { left, right, top, bottom } (the
     *  whole width or depth when none is positioned; below 0 when one is
     *  outside the stage); and { minimum, maximum }: the smallest and the
     *  largest stage. A side shrinks by 1 m when its room is more than that */
    QVariantMap stageRoom() const;
    /** While resizeStageSide tells that the fixtures moved with a side of the
     *  stage: the whole plan moved, nothing moved on the real stage */
    bool shiftingStage() const { return m_shiftingStage; }

    Q_INVOKABLE void setFixturePosition(quint32 id, qreal x, qreal y);
    /** Place the fixtures without a position in rows, by universe/address */
    Q_INVOKABLE void autoLayout(bool all = false);

    /*********************************************************************
     * Pan/tilt invert, identify
     *********************************************************************/
    /** Invert the pan or tilt ($axis "pan" or "tilt") of fixtures. It is a channel
     *  modifier, so it applies to everything that moves them, and it is saved in the show */
    Q_INVOKABLE void setInverted(const QVariantList &ids, const QString &axis, bool inverted);

    /** Fixtures lit and blinking to find them on stage */
    QVariantList identified() const;
    /** Identify $ids (an empty list stops). DMX Total Control: never a
     *  protected special effect (a CO2 that blinks fires). @return how many
     *  of them were left out */
    Q_INVOKABLE int identify(const QVariantList &ids);
    /** @return how many protected special effects were left out (identify) */
    Q_INVOKABLE int toggleIdentify(quint32 id);
    Q_INVOKABLE bool isIdentified(quint32 id) const;
    Q_INVOKABLE void stopIdentify();

    /*********************************************************************
     * Limits and channel options
     *********************************************************************/
    /** { id, name, hasDimmer, hasPan, hasTilt, dimmerMin, dimmerMax, panMin,
     *    panMax, tiltMin, tiltMax (0-100 %), invertPan, invertTilt,
     *    swapPanTilt, gmProtected, channels: [{ index, name, group, canFade,
     *    dimmer }] } */
    Q_INVOKABLE QVariantMap fixtureOptions(quint32 id) const;
    /** fixtureOptions() of several fixtures: what any of them has (a dimmer,
     *  pan, tilt), each with the values of the first fixture that has it;
     *  id, name and channels of the first one. gmProtected: all of them are
     *  protected from the grand master; gmProtectedMixed: some, not all */
    Q_INVOKABLE QVariantMap selectionOptions(const QVariantList &ids) const;
    /** Set some of the keys of fixtureOptions() on fixtures $ids: the
     *  limits are channel modifiers, applied to everything that goes out */
    Q_INVOKABLE void setLimits(const QVariantList &ids, const QVariantMap &limits);
    /** Whether channel $channel of fixture $id fades, and follows the dimmers */
    Q_INVOKABLE void setChannelOptions(quint32 id, int channel, bool canFade, bool followsDimmer);
    /** The grand master never lowers fixtures $ids (all their channels), or
     *  lowers them again: at once, also what plays now. The masters of the
     *  groups, the scene dimmers, Blackout and the fade to black still do.
     *  Saved with the show, in each fixture */
    Q_INVOKABLE void setProtectedFromGrandMaster(const QVariantList &ids, bool protect);

    /*********************************************************************
     * LED matrices and strips
     *********************************************************************/
    /**
     * Patch a matrix of $columns x $rows RGB pixels ($rows 1: a strip), one
     * fixture per line of pixels, with a fixture group that knows where each
     * pixel is (for colour effects) and a place in the 2D plan.
     * $components: "RGB", "GRB", "BGR", "RGBW"...
     * $wiring: "rows", "snake" (every other row comes back), "columns",
     * "snakeColumns"; $start: the corner of pixel 1: "topLeft", "topRight",
     * "bottomLeft", "bottomRight".
     * @return an empty string, or what went wrong
     */
    Q_INVOKABLE QString addMatrix(const QString &name, int columns, int rows, const QString &components,
                                  int universe, int address, const QString &wiring, const QString &start);

    /*********************************************************************
     * 2D plan tools
     *********************************************************************/
    /** Place fixtures $ids, in this order, around their centre: $shape "line"
     *  (option vertical), "circle" (radius), "grid" (columns, order: "rows",
     *  "snake", "columns", "snakeColumns"). Option spacing: in mm */
    Q_INVOKABLE void arrange(const QVariantList &ids, const QString &shape, const QVariantMap &options);
    /** $how: "left", "right", "centerX", "top", "bottom", "centerY" */
    Q_INVOKABLE void align(const QVariantList &ids, const QString &how);
    /** Same space between fixtures, $axis "horizontal" or "vertical" */
    Q_INVOKABLE void distribute(const QVariantList &ids, const QString &axis);
    /** Turn fixtures around their centre (and each one on itself) */
    Q_INVOKABLE void rotate(const QVariantList &ids, qreal degrees);
    /** Move fixtures by $dx, $dy (mm); with a $snap they land on a grid of that step */
    Q_INVOKABLE void moveFixtures(const QVariantList &ids, qreal dx, qreal dy, qreal snap = 0);
    /** How fixtures look in the plan: "round", "square" or "rectangle" */
    Q_INVOKABLE void setFixtureShape(const QVariantList &ids, const QString &shape);
    void clearPlan();

    /*********************************************************************
     * Kinds of fixture: the glyph of each fixture on the plan, and the
     * types of the show its colours and legend follow (patchkinds.cpp)
     *********************************************************************/
    /** The kinds, in the order of the Look menu of the plan: movinghead,
     *  scanner, wash, ledbar, strobe, blinder, dimmer, laser, smoke, hazer,
     *  co2, flame, spark, confetti, bubbles, snow, fan, effect, other */
    static QStringList fixtureKinds();
    QStringList fixtureKindList() const { return fixtureKinds(); }
    /**
     * The kind of a fixture: the $icon chosen for it by hand, when it is
     * one; else what the words of its definition's $manufacturer and $model
     * say where they say more than its QLC+ $type (an "Effect" called "CO2
     * Jet": co2; a "Dimmer" called "Blinder": blinder; a "Moving Head"
     * called "Beam" or "Wash" stays a moving head); else its type. A Generic
     * definition looks at the words of the fixture's own $name first, by
     * the same rules; a generic dimmer (Generic › Generic, what stands in
     * for a fixture the library doesn't have) takes any kind its name says
     * ("CO2 1", "FLAME 3", "BLINDER1")
     */
    static QString fixtureKind(const QString &manufacturer, const QString &model, const QString &type,
                               const QString &name = QString(), const QString &icon = QString());
    /** The kind the words of $text say, whatever the type ("" when none):
     *  whole words or their start, in several languages, without caring
     *  for case or accents. The special effects first ("Strobe Flame" is a
     *  flame), then blinder, strobe... */
    static QString kindFromWords(const QString &text);
    /** $text as the kind words are looked for in it: the words of a name
     *  written together apart ("FrontVideo" is "front video"), in lower
     *  case, without accents */
    static QString nameWords(const QString &text);
    /** The kind of a QLC+ fixture type ("Moving Head": movinghead) */
    static QString kindOfType(const QString &type);
    /** fixtureKind() of a patched fixture (its icon chosen by hand too) */
    QString kindOf(const Fixture *fxi) const;
    /** The fixture types of the show, as the plan colours them and its
     *  legend lists them, in the order of their lowest fixture ID: [{ key,
     *  manufacturer, model (both empty: a generic dimmer), kind,
     *  colorIndex, count, ids }]. One type per definition, its kind that
     *  of its first fixture without an icon chosen by hand (an icon chosen
     *  on some of them changes their glyph, not their type nor colour);
     *  the generic dimmers, a type for each kind they stand in for
     *  (blinders, CO2 jets) */
    QVariantList fixtureTypes() const;
    /** The icon of fixtures $ids on the plan: one of fixtureKinds(); empty
     *  (or "auto") goes back to the one fixtureKind() finds */
    Q_INVOKABLE void setFixtureIcon(const QVariantList &ids, const QString &kind);
    /** The icon chosen for fixture $id ("" when automatic) */
    QString fixtureIcon(quint32 id) const;
    /** What each fixture shows now: [{ id, dimmer (0-1), colors: ["#rrggbb"
     *  per head, dimmer included], strobe, gobo, goboImage }] */
    Q_INVOKABLE QVariantList liveStates() const;

    /*********************************************************************
     * Protected special effects (patcheffects.cpp): CO2, flames, sparks and
     * confetti only go out while the effects are armed (SafetyManager), and
     * each fire has a maximum time. Each fixture keeps what was decided for
     * it, saved with the show; the automatic rules only ever protect
     *********************************************************************/
    /** The kinds protected when nothing was decided: co2, flame, spark,
     *  confetti (not bubbles nor snow: harmless) */
    static bool protectedByDefault(const QString &kind);
    /** The maximum time of a fire by default (ms): co2 3000, flame 2000,
     *  spark 5000, confetti 2000, any other 3000 */
    static int defaultFireMs(const QString &kind);
    /** The minimum time between two fires by default (ms) */
    static int defaultWaitMs() { return 1000; }
    /** The ranges of the times (ms) */
    static constexpr int minFireMs() { return 100; }
    static constexpr int maxFireMs() { return 30000; }
    static constexpr int maxWaitMs() { return 60000; }

    bool hasProtected() const;
    Q_INVOKABLE bool isProtected(quint32 id) const;
    /** What fixtures $ids have: { count, protected, kind, byKind (as their
     *  kind says: protection, times, no safety channel), defaultFireMs (of
     *  the kind), maxFireMs, minWaitMs, safetyChannel (-1: none),
     *  safetyFrom, safetyTo, sameMode, channels: [{ index, name }] (of the
     *  first one, when they all have the same channels), crossUniverse (one
     *  of them never fires: across two universes), mixed: [the keys that
     *  differ between them] } */
    Q_INVOKABLE QVariantMap effectSettings(const QVariantList &ids) const;
    /** One of them on fixtures $ids: "protected", "maxFireMs" (100-30000),
     *  "minWaitMs" (0-60000), "safetyChannel" (-1, or a channel each of them
     *  has), "safetyFrom", "safetyTo" (0-255, from not above to). A value
     *  out of its range goes to the nearest one; a safety channel or a
     *  window that can't be is refused (false, lastError() says why) */
    Q_INVOKABLE bool setEffectSetting(const QVariantList &ids, const QString &key, const QVariant &value);
    /** Back to what their kind says (protected or not, its times, no safety
     *  channel) */
    Q_INVOKABLE void resetEffectSettings(const QVariantList &ids);

    /** What the output does with a fixture: protected or not (as
     *  isProtected() says), its times and its safety channel (with its
     *  window), the defaults resolved */
    struct EffectRule
    {
        bool protect = false;
        quint32 maxFireMs = 3000;
        quint32 minWaitMs = 1000;
        int safety = -1;            // a channel of the fixture, -1: none
        uchar from = 1;
        uchar to = 255;
    };
    EffectRule effectRule(quint32 id) const;
    /** The fixtures nothing was decided for take what their kind says (a
     *  show loaded: the end of its plan, the end of the loading) */
    void settleProtection();
    /** Between beginLoading() and endLoading() */
    bool loading() const { return m_loading; }

    /** Re-addressing fixtures (applyPatch): $hook runs once their new
     *  addresses are set, before the universes are let go (no frame in
     *  between): the output protects the new ones at once
     *  (SafetyManager). One per Doc; null removes it */
    static void setPatchHook(Doc *doc, const std::function<void()> &hook);
    /** Its channels go on in the next universe (a show from QLC+): it
     *  never fires */
    static bool crossesUniverses(const Fixture *fxi);

    /*********************************************************************
     * Follow: where the moving heads hang and how they move, so they can
     * all aim at a point of the plan (Programmer::aimAt)
     *********************************************************************/
    /** What fixtures $ids have (the ones with pan or tilt): { count, height
     *  (mm), standing, facing (degrees, clockwise; 0: the audience),
     *  panRange, tiltRange (their own, degrees; 0: the definition's),
     *  panDefinition, tiltDefinition (0: it says none), panInverted,
     *  tiltInverted, mixed: [the keys that differ between them] } */
    Q_INVOKABLE QVariantMap aimSettings(const QVariantList &ids) const;
    /** One of them on fixtures $ids: "height" (mm), "standing", "facing",
     *  "panRange", "tiltRange", "panInverted", "tiltInverted" */
    Q_INVOKABLE void setAimSetting(const QVariantList &ids, const QString &key, const QVariant &value);
    /** Where fixture $id is and how it moves; $guessed: its pan or tilt
     *  travel comes from nowhere (the usual one of a moving head) */
    AimMount aimMount(quint32 id, bool *guessed = nullptr) const;
    /** The height of a hung head without one (mm) */
    static int defaultHeight() { return 4000; }

    /*********************************************************************
     * Selections: ordered parts of a fixture group
     *********************************************************************/
    /** [{ id, name, groupId, fixtures: [ids, in order] }] */
    QVariantList selections() const;
    Q_INVOKABLE int addSelection(const QString &name, quint32 groupId, const QVariantList &fixtureIds);
    Q_INVOKABLE void renameSelection(int id, const QString &name);
    Q_INVOKABLE void removeSelection(int id);
    /** A pattern of $fixtureIds: "odd", "even", "reverse", "firstHalf",
     *  "secondHalf", "outsideIn", "insideOut" */
    Q_INVOKABLE QVariantList patternSelection(const QVariantList &fixtureIds, const QString &pattern) const;

    void clearSelections();
    bool loadXML(QXmlStreamReader &root);
    /** A show loads, from its Doc to its last section (the plan, the
     *  numbers, the order of the groups): the fixtures and the groups are
     *  told once, by endLoading(), not by each part (each one made the
     *  lists and plans of the UI again) */
    void beginLoading();
    void endLoading();
    bool saveXML(QXmlStreamWriter *doc) const;

signals:
    void fixturesChanged();
    void groupsChanged();
    void stageSizeChanged();
    /** stageRoom() changed: the stage or the fixtures */
    void stageRoomChanged();
    /** Every item of the plan moved by $dx, $dy (mm) with a side of the
     *  stage (resizeStageSide), before fixturesChanged */
    void stageShifted(qreal dx, qreal dy);
    void identifiedChanged();
    void selectionsChanged();
    /** A fixture became a protected special effect, or stopped being one,
     *  or its times or safety channel changed */
    void protectionChanged();
    /** Something to tell the user (a fixture protected by itself) */
    void protectionMessage(const QString &text);

private slots:
    void slotBlink();
    void slotFixtureRemoved();
    void slotDocCleared();
    void slotGroupRemoved(quint32 id);
    void slotFixtureAdded(quint32 id);
    void slotFixtureGone(quint32 id);
    void slotFixtureChanged(quint32 id);
    /** The fixtures of Doc changed: the list is told once, a moment later */
    void slotFixturesDirty();
    /** Tell the list now (and not again later) */
    void notifyFixtures();
    /** A group of Doc changed: the groups are told (a show cleared or
     *  loaded: once, at the end) */
    void slotGroupsChanged();
    /** Tell the groups now (or at endLoading) */
    void notifyGroups();
    /** A show loaded without DtcCore around (the tests): the fixtures take
     *  what their kind says */
    void slotDocLoaded();

private:
    bool isRangeFree(int universe, int address, int channels, quint32 ignoreId) const;
    bool isRangeFree(int universe, int address, int channels, const QList<quint32> &ignoreIds) const;

private:
    /** The "Invert" modifier of the cache, created if the templates are missing */
    ChannelModifier *invertModifier() const;
    /** The output range of the first channel of a group (0-255, from > to:
     *  inverted) */
    QPair<int, int> channelRange(Fixture *fxi, int group, int colour = -1) const;
    /** Set the range modifier on the channels of a group */
    void setChannelRange(Fixture *fxi, int group, int from, int to, int colour = -1);
    bool isInverted(Fixture *fxi, int group) const;
    void setPositions(const QHash<quint32, QPointF> &positions);
    QPointF position(quint32 id) const;
    void writeIdentify();
    /** A fixture type of fixtureTypes() */
    struct TypeEntry
    {
        QString key;
        QString manufacturer;
        QString model;
        QString kind;
        QList<quint32> ids;
    };
    /** The types of the show, by their lowest fixture ID (each list sorted) */
    QList<TypeEntry> typeEntries() const;
    /** Protection: what was decided for a fixture (patcheffects.cpp) */
    struct EffectSetup
    {
        bool protect = false;
        int maxFireMs = -1;         // -1: the default of its kind
        int minWaitMs = -1;         // -1: defaultWaitMs()
        int safetyChannel = -1;
        QString safetyName;         // the name of that channel when it was chosen
        int safetyFrom = 1;
        int safetyTo = 255;
    };
    /** The safety channel of $setup is still the one chosen on $fxi: the
     *  same name at its place (another mode, an updated definition may put
     *  another channel there: the fire itself), and another one to fire */
    static bool safetyStillThere(const Fixture *fxi, const EffectSetup &setup);
    /** A fixture whose safety channel isn't there any more: none, said */
    void dropLostSafety(quint32 id);
    /** The hook of $doc (setPatchHook), if it has one */
    static void runPatchHook(Doc *doc);
    /** While a show loads (or Doc does): protection waits for settleProtection() */
    bool deferProtection() const;
    /** $id takes what its kind says, if nothing was decided for it. True
     *  when that protected it */
    bool decideProtection(quint32 id);
    /** The kind of $ids is looked at again: one that became a protected kind
     *  is protected, told with a message when $tell (nothing is ever
     *  unprotected by it) */
    void recheckKinds(const QList<quint32> &ids, bool tell = true);
    /** The attributes of protection of fixture $id in the plan of the show */
    void writeEffectAttributes(QXmlStreamWriter *doc, quint32 id) const;
    void readEffectAttributes(quint32 id, const QXmlStreamAttributes &attrs);
    /** What $attrs (of the plan of a show) say of $fxi, of kind $kind, in
     *  $setup: false when they say nothing of it. $lostSafety: a safety
     *  channel it names is another channel of $fxi now (dropped) */
    bool effectSetupFrom(const Fixture *fxi, const QString &kind, const QXmlStreamAttributes &attrs,
                         EffectSetup *setup, bool *lostSafety = nullptr) const;
    /** Something to write in the plan for $id */
    bool hasEffectAttributes(quint32 id) const;

    /** What a table import does (patchimport.cpp) */
    struct TablePlan;
    void planTable(const QByteArray &data, const QVariantMap &options, TablePlan *plan) const;
    QVariantMap previewMap(const TablePlan &plan) const;

private:
    Doc *m_doc;
    GenericDMXSource *m_identifySource;
    QTimer *m_blinkTimer;
    /** fixturesChanged once for many changes of Doc (a show loading, a
     *  patch of many fixtures): the UI reads the whole list every time */
    QTimer *m_fixturesTimer;
    QString m_lastError;
    QList<quint32> m_identified;
    bool m_blinkOn;
    /** The order of the groups the user gave (IDs; saved with the show) */
    QList<quint32> m_groupOrder;
    void setGroupOrder(const QList<quint32> &ids);

    struct Selection
    {
        int id;
        QString name;
        quint32 groupId;
        QList<quint32> fixtures;
    };
    QList<Selection> m_selections;
    int m_lastSelectionId;
    /** The shape of the fixtures in the plan, when not round */
    QHash<quint32, QString> m_shapes;
    /** The icon of the fixtures in the plan, when chosen (by hand, or by a
     *  patch import for a generic dimmer standing in for another fixture) */
    QHash<quint32, QString> m_icons;
    /** The icon the last patch import gave each fixture, when it gave one:
     *  a new import keeps the one chosen by hand since, unless the type's
     *  match changed */
    QHash<quint32, QString> m_importIcons;
    /** Follow: how each moving head hangs, when not the default */
    struct AimSetup
    {
        int height = -1;            // mm, -1: the default
        bool standing = false;
        int panRange = 0;           // degrees, 0: the definition's
        int tiltRange = 0;
        bool panInverted = false;
        bool tiltInverted = false;
        bool isDefault() const
        {
            return height < 0 && standing == false && panRange == 0 && tiltRange == 0 &&
                   panInverted == false && tiltInverted == false;
        }
    };
    QHash<quint32, AimSetup> m_aims;
    /** The identifier of the table row each fixture was imported from
     *  (saved in the plan: a new import updates the same fixtures) */
    QHash<quint32, QString> m_importIds;
    /** The fixture numbers */
    QHash<quint32, int> m_numbers;
    /** Protection decided for each fixture (saved in the plan), and the
     *  kind each one had when last looked at */
    QHash<quint32, EffectSetup> m_effects;
    QHash<quint32, QString> m_effectKinds;
    /** The fixtures of a show opening whose safety channel isn't there any
     *  more (said once it is open) */
    QStringList m_lostSafety;
    /** Between beginLoading and endLoading, and what to tell then */
    bool m_loading = false;
    bool m_fixturesHeld = false;
    bool m_groupsHeld = false;
    /** See shiftingStage() */
    bool m_shiftingStage = false;
};

#endif
