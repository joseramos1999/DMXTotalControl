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
#include <QSizeF>
#include <QPointF>
#include <QHash>
#include <QPair>
#include <QStringList>
#include <QVariant>

#include "aim.h"

class GenericDMXSource;
class ChannelModifier;
class QLCFixtureMode;
class QLCFixtureDef;
class Fixture;
class QTimer;
class QXmlStreamReader;
class QXmlStreamWriter;
class Doc;

#define KGenericDimmerModel QStringLiteral("Generic Dimmer")
#define KXMLDTCSelections QStringLiteral("Selections")
#define KXMLDTCPlan QStringLiteral("Plan")
#define KXMLDTCNumbers QStringLiteral("Numbers")

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
    Q_PROPERTY(QVariantList selections READ selections NOTIFY selectionsChanged)

public:
    PatchManager(Doc *doc, QObject *parent = nullptr);
    ~PatchManager();

    /** [{ id, name, manufacturer, model, mode, type, universe, address,
     *     channels, x, y, positioned }]
     *  Addresses are 1-based, as displayed to the user.
     *  x/y is the position in the 2D stage plan, in millimeters */
    QVariantList fixtures() const;
    int fixturesCount() const;

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

    /*********************************************************************
     * Fixture groups
     *********************************************************************/
    /** [{ id, name, fixtures: [ids] }] */
    QVariantList groups() const;

    Q_INVOKABLE int createGroup(const QString &name, const QVariantList &fixtureIds);
    Q_INVOKABLE void deleteGroup(quint32 id);
    Q_INVOKABLE void renameGroup(quint32 id, const QString &name);
    Q_INVOKABLE void setGroupFixtures(quint32 id, const QVariantList &fixtureIds);

    /*********************************************************************
     * 2D stage plan
     *********************************************************************/
    /** Stage width/height in millimeters */
    QSizeF stageSize() const;
    Q_INVOKABLE void setStageSize(qreal width, qreal height);

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
    /** Identify $ids (an empty list stops) */
    Q_INVOKABLE void identify(const QVariantList &ids);
    Q_INVOKABLE void toggleIdentify(quint32 id);
    Q_INVOKABLE bool isIdentified(quint32 id) const;
    Q_INVOKABLE void stopIdentify();

    /*********************************************************************
     * Limits and channel options
     *********************************************************************/
    /** { id, name, hasDimmer, hasPan, hasTilt, dimmerMin, dimmerMax, panMin,
     *    panMax, tiltMin, tiltMax (0-100 %), invertPan, invertTilt,
     *    swapPanTilt, channels: [{ index, name, group, canFade, dimmer }] } */
    Q_INVOKABLE QVariantMap fixtureOptions(quint32 id) const;
    /** fixtureOptions() of several fixtures: what any of them has (a dimmer,
     *  pan, tilt), each with the values of the first fixture that has it;
     *  id, name and channels of the first one */
    Q_INVOKABLE QVariantMap selectionOptions(const QVariantList &ids) const;
    /** Set some of the keys of fixtureOptions() on fixtures $ids: the
     *  limits are channel modifiers, applied to everything that goes out */
    Q_INVOKABLE void setLimits(const QVariantList &ids, const QVariantMap &limits);
    /** Whether channel $channel of fixture $id fades, and follows the dimmers */
    Q_INVOKABLE void setChannelOptions(quint32 id, int channel, bool canFade, bool followsDimmer);

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
    /** What each fixture shows now: [{ id, dimmer (0-1), colors: ["#rrggbb"
     *  per head, dimmer included], strobe, gobo, goboImage }] */
    Q_INVOKABLE QVariantList liveStates() const;

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
    bool saveXML(QXmlStreamWriter *doc) const;

signals:
    void fixturesChanged();
    void groupsChanged();
    void stageSizeChanged();
    void identifiedChanged();
    void selectionsChanged();

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
    /** The fixture numbers */
    QHash<quint32, int> m_numbers;
};

#endif
