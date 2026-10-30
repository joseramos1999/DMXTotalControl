/*
  DMX Total Control
  showsurface.h

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

#ifndef SHOWSURFACE_H
#define SHOWSURFACE_H

#include <QObject>
#include <QVariant>
#include <QColor>
#include <QHash>
#include <QSet>
#include <QList>
#include <memory>
#include <vector>

class QXmlStreamReader;
class QXmlStreamWriter;
class ZoneManager;
class LiveMixer;
class DtcCore;
class Function;
class Zone;
class Doc;

#define KXMLDTCSurface QStringLiteral("Surface")

/**
 * The SHOW screen: each page is made of
 * sections that tile the screen. A section can be split in two halves, side
 * by side or one above the other, and so on. Each section shows one thing:
 *
 *  "buttons"  a table of buttons (cols x rows), one button per cell
 *  "faders"   a row of faders (cols), one fader per cell
 *  "group"    the panel of a group of scenes (zoneId)
 *  "groups"   every group side by side
 *  "clock"    time and date
 *  "timer"    a stopwatch or a countdown (mode, minutes)
 *  "bpm"      the rhythm: GO and Stop of the scene buttons, BeatGO
 *  "notes"    text for the show (text, fontSize)
 *  "running"  the scenes playing, with their stop buttons
 *  "monitor"  the DMX levels of a universe (universe)
 *  "dials"    a table of dials (cols x rows): fader items, or incremental
 *             wheels ("encoder": true, "step" in % per notch)
 *  "colour"   a colour wheel for a fixture group (groupId, -1 all)
 *  "xy"       an XY grid for the pan and tilt of a fixture group (groupId, -1 all)
 *  "image"    a picture (image: a file, fit: "fit" | "fill")
 *  "playbacks" a row of playbacks (cols of them, from the playback "first",
 *             0-based) of a playback page (playbackPage, -1 follows the
 *             current page), with their Back and Flash buttons and the cue
 *             lines (showBack, showFlash, showCue)
 *  "empty"    nothing yet
 *
 * Any section can have a "background", a colour "#rrggbb" (none or empty: the
 * one of the theme): the SHOW and the phone tint the section with it.
 *
 * Pages are given to QML as trees of maps: a split is
 * { id, split: "h"|"v", ratio, a, b }, a section is { id, split: "none",
 * type, title, ..., items: { "<cell>": item } }.
 *
 * A button item: action "scene" (functionId, mode "toggle"/"flash"; played in
 * its group when it has one), "next", "previous", "release" (zoneId, -1 = all
 * groups), "blackout", "stopAll", "tap", "beatgo", "map" (map: any target of
 * the mappings, ControlMapper; the GO, Back, go to a cue and Release of a
 * cuelist are playback targets with its functionId, and cue for the go to);
 * label, color, key, icon (a glyph), image (a file).
 * A fader item drives one or more targets: "targets" is a list of
 * { target, ... }: "grandMaster", "group" (zoneId: its master), "scene"
 * (functionId: up from zero starts it, down to zero stops it, in between its
 * intensity), "sceneDimmer" (functionId: the dimmer of the scene, whoever
 * plays it; it never starts or stops it), "sceneSpeed" (functionId),
 * "sceneRun" (functionId: a scene and its speed: up from the end that stops
 * it starts it, as a scene fader does, and the travel is its speed, x0.1 to
 * x10, for that run only: stopped, it plays at its own speed again),
 * "mixerDimmer" (mixerId), "moveSpeed" (groupId: the speed of the movement
 * of a fixture group, -1 all the moving heads: their pan/tilt FX and the
 * steps that move them, x0.1 to x10), "bpm", "map" (map: a fader
 * target of the mappings). Older items have a single "target" with its keys.
 * "min", "max" (0-100 %) and "invert": the part of what it drives that the
 * travel covers (faders and dials). "flash": a button under the fader puts it
 * at the top of its travel while pressed (default true).
 * Everything runs here (the remote control uses it too).
 */
class ShowSurface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList pages READ pages NOTIFY pagesChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pagesChanged)
    /** Changes whenever something the controls show may have changed (running scenes, masters) */
    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged)
    /** The page the SHOW shows (where Alt+click puts new controls) */
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)

public:
    ShowSurface(Doc *doc, ZoneManager *zones, QObject *parent = nullptr);
    ~ShowSurface();

    /*********************************************************************
     * Pages and sections
     *********************************************************************/
    /** [{ name, root }] */
    QVariantList pages() const;
    int pageCount() const;

    /** A page with a single section of $type ("buttons" by default) */
    Q_INVOKABLE int addPage(const QString &name = QString(), const QString &type = QString());
    Q_INVOKABLE void renamePage(int page, const QString &name);
    Q_INVOKABLE void removePage(int page);
    Q_INVOKABLE void movePage(int from, int to);

    /** Split a section in two halves: "h" side by side, "v" one above the other.
     *  @return the ID of the new (empty) half */
    Q_INVOKABLE QString splitSection(int page, const QString &sectionId, const QString &orientation);
    /** Remove a section: the other half of its split takes the space */
    Q_INVOKABLE void removeSection(int page, const QString &sectionId);
    /** Where the divider of a split is, 0.1 .. 0.9 */
    Q_INVOKABLE void setSplitRatio(int page, const QString &splitId, qreal ratio);
    /** Merge $props into a section (type, title, cols, rows, zoneId...).
     *  Changing the type keeps the items only between buttons, faders and
     *  dials, with what they drive where the new kind can drive it: a scene,
     *  and the group of a next, previous or release button as its master
     *  (faders and dials keep everything) */
    Q_INVOKABLE void updateSection(int page, const QString &sectionId, const QVariantMap &props);
    Q_INVOKABLE QVariantMap section(int page, const QString &sectionId) const;

    /** Merge $props into the item of a cell, creating it with the defaults of its section */
    Q_INVOKABLE void setItem(int page, const QString &sectionId, int cell, const QVariantMap &props);
    Q_INVOKABLE void clearItem(int page, const QString &sectionId, int cell);
    /** Swap the items of two cells of a section */
    Q_INVOKABLE void moveItem(int page, const QString &sectionId, int from, int to);
    Q_INVOKABLE QVariantMap item(int page, const QString &sectionId, int cell) const;
    /** One colour for every button, fader or dial of a table section, in one
     *  change (its empty cells stay empty). @return how many items changed */
    Q_INVOKABLE int setItemsColour(int page, const QString &sectionId, const QString &color);

    /** Fill a buttons section with the scenes of a group, in order. @return how many */
    Q_INVOKABLE int fillWithGroup(int page, const QString &sectionId, quint32 zoneId);

    /** A first page from the groups of the show: the groups, masters and the clock */
    Q_INVOKABLE int buildFromGroups();

    /** Section types and their default settings */
    Q_INVOKABLE static QVariantMap sectionDefaults(const QString &type);
    Q_INVOKABLE static QVariantMap itemDefaults(const QString &sectionType);

    void clear();

    /*********************************************************************
     * Playback
     *********************************************************************/
    Q_INVOKABLE void press(const QVariantMap &item);
    Q_INVOKABLE void release(const QVariantMap &item);
    /** Whether a button is lit: its scene runs */
    Q_INVOKABLE bool isActive(const QVariantMap &item) const;
    /** The automatic label of a "map" button: "GO · Song 1", "Song 1 · cue
     *  2.5" for the cuelist actions, else what the mappings call the target */
    Q_INVOKABLE QString mapLabel(const QVariantMap &target) const;

    /** Faders and dials: their position, 0..1. Every target of the item
     *  follows, within the range of the item ("min", "max" 0-100 %, "invert",
     *  as the mappings: ControlMapper::rangeOf); at the end of the travel
     *  where the value is lowest (the bottom, the top when inverted) a scene
     *  or playback fader stops it, whatever the minimum */
    Q_INVOKABLE void setLevel(const QVariantMap &item, qreal level);
    /** The position of a fader, 0..1, from the value of its first target (a
     *  value out of the range is at the end of it) */
    Q_INVOKABLE qreal level(const QVariantMap &item) const;
    /** The value of the first target of a fader, 0..1, as it is: its text */
    Q_INVOKABLE qreal value(const QVariantMap &item) const;
    /** Where $value (0..1, a value of the targets) is on the fader, 0..1 */
    Q_INVOKABLE qreal positionOf(const QVariantMap &item, qreal value) const;
    /** An encoder: $notches steps of the item's "step" (% of the value,
     *  default 2) up or down its range; turned up, a stopped scene starts at
     *  the minimum */
    Q_INVOKABLE void stepLevel(const QVariantMap &item, int notches);
    /** Flash: the values of the targets as they are, and back to them
     *  exactly (also out of the range of the fader) */
    Q_INVOKABLE QVariantList holdLevels(const QVariantMap &item) const;
    Q_INVOKABLE void restoreLevels(const QVariantMap &item, const QVariantList &levels);
    /** The targets of a fader item (older items: their single target) */
    Q_INVOKABLE static QVariantList faderTargets(const QVariantMap &item);
    /** A fader or dial whose targets are all speeds (of the movement, of
     *  scenes, scenes and their speed): x0.1 to x10, x1 in the middle */
    Q_INVOKABLE static bool isSpeed(const QVariantMap &item);
    /** Where a double click puts a fader or a dial (0..1): the BPM at 90, a
     *  speed at x1 (or the nearest end of its range; one that plays a scene
     *  plays it, off the end that stops it), anything else at the bottom */
    Q_INVOKABLE qreal resetLevel(const QVariantMap &item) const;

    /** The dimmer of a scene ("sceneDimmer" targets), 0..1: it scales the
     *  scene whoever plays it (its group, a playback, a button, a super
     *  scene) and never starts or stops it. Saved with the show; it lives
     *  while a fader or dial is on it (pruneSceneDimmers) */
    void setSceneDimmer(quint32 functionId, qreal dimmer);
    qreal sceneDimmer(quint32 functionId) const;

    /** Whether a fader or dial of the SHOW holds $fid up ("scene",
     *  "sceneRun"): only that fader, at the bottom, stops it (not its group,
     *  its button, releasing the groups or Stop all) */
    Q_INVOKABLE bool holds(quint32 fid) const;
    /** The scenes faders hold up, in their groups or on their own */
    QSet<quint32> heldScenes() const;

    /** Something the controls show changed with no signal of its own (the
     *  speed of a scene in no group, from a mapping): they read it again */
    void refresh();

    /** The mixer the "mixerDimmer" targets drive */
    void setMixer(LiveMixer *mixer);
    /** The core: blackout, tap, the programmer (colour wheels, XY grids), the mappings */
    void setCore(DtcCore *core);

    int currentPage() const;
    void setCurrentPage(int page);

    /** Colour wheels and XY grids: live edits of the programmer on the fixtures of the section */
    Q_INVOKABLE void setColour(int page, const QString &sectionId, const QColor &colour);
    Q_INVOKABLE QColor colour(int page, const QString &sectionId) const;
    Q_INVOKABLE void setXY(int page, const QString &sectionId, qreal x, qreal y);
    /** { x, y, available } */
    Q_INVOKABLE QVariantMap xy(int page, const QString &sectionId) const;
    /** Release the colour or the position the section gave (the scenes show again) */
    Q_INVOKABLE void releaseSection(int page, const QString &sectionId);

    /** Alt+click: a control on the SHOW page for a target of the mappings, in the
     *  first free cell of a section of its kind (a new one if there is none).
     *  A playback of the current page (pbPage -1) is stored with its page.
     *  @return { page, section, cell }, or an empty map */
    Q_INVOKABLE QVariantMap addFromTarget(const QVariantMap &target, int page = -1);

    int revision() const;

    /** The pictures of the SHOW (picture sections and buttons), as saved:
     *  paths or file URLs */
    QStringList images() const;
    /** The picture saved as $from is $to now. @return how many changed */
    int replaceImage(const QString &from, const QString &to);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc) const;

signals:
    void pagesChanged();
    void revisionChanged();
    void currentPageChanged();
    /** Alt+click put a control on the SHOW: { page, section, cell, pageName } */
    void controlAdded(const QVariantMap &where);
    /** The show was closed (another one opens): what the controls held is gone */
    void cleared();

private slots:
    void slotChanged();
    void slotZonesChanged();
    /** Playback pages moved or one was deleted: the rows of playbacks fixed
     *  on a page and the controls bound to a slot of a page follow it (a
     *  deleted page: the row follows the current page, the controls
     *  ControlMapper::followPlaybackPages) */
    void slotPlaybackPagesRenumbered(const QList<int> &newIndexes, const QHash<int, quint32> &removedSlots);
    /** A scene of no group a fader drives runs or stops (engine signals) */
    void slotFunctionRunning(quint32 fid);
    void slotFunctionStopped(quint32 fid);
    void slotFunctionGone(quint32 fid);
    /** A function deleted: the buttons and faders on it drive nothing (a
     *  function made later may get its ID) */
    void slotFunctionRemoved(quint32 fid);
    /** A new function has no dimmer (a copy doesn't bring one); one that
     *  takes the place of another (an effect made an FX) keeps the dimmer, and
     *  a super scene gives it to its new tracks and static values */
    void slotFunctionAdded(quint32 fid);
    void slotFunctionChanged(quint32 fid);
    /** The patch changed: the movement speed faders on the fixtures of now */
    void slotFixturesChanged();
    /** What moves changed (a function, the FX of the programmer): the speed
     *  of the movement goes there again */
    void slotMotionChanged();

private:
    struct Node
    {
        QString id;
        QString split;          // "none", "h" or "v"
        qreal ratio = 0.5;
        QVariantMap props;      // sections: type, title... and "items"
        std::unique_ptr<Node> a;
        std::unique_ptr<Node> b;
    };
    struct Page
    {
        QString name;
        std::unique_ptr<Node> root;
    };

    Node *find(Node *node, const QString &id, Node **parent = nullptr) const;
    Node *findSection(int page, const QString &id, Node **parent = nullptr) const;
    std::unique_ptr<Node> newSection(const QString &type);
    QString nextId(const QString &prefix);
    QVariantMap toMap(const Node *node) const;
    void saveNode(QXmlStreamWriter *doc, const Node *node) const;
    std::unique_ptr<Node> loadNode(QXmlStreamReader &root);
    void loadVersion1Page(QXmlStreamReader &root, Page &page);
    /** "Go to a cue" buttons saved before they kept their cue (by place
     *  only): bound to the cue at that place, as the show was saved */
    void bindGoToCues(Node *node);
    /** A loaded show (a damaged or edited file): a section without an ID or
     *  with the ID of another one gets a new one; the IDs of the functions
     *  that aren't there are let go */
    void checkLoaded();
    /** The buttons and faders (their targets) on the functions $gone says
     *  (by their functionId) drive nothing. @return whether any changed */
    bool forgetFunctions(const std::function<bool(const QVariant &)> &gone);

    /** An item of a buttons, faders or dials section in a section of $type
     *  (not between faders and dials): empty when it can't drive anything there */
    static QVariantMap convertItem(const QVariantMap &item, const QString &type);

    /** The group that plays a scene, if any */
    Zone *zoneFor(quint32 functionId) const;
    /** The fixtures a colour wheel or an XY grid acts on */
    QList<quint32> sectionFixtures(const Node *node) const;
    void changed();

private:
    Doc *m_doc;
    ZoneManager *m_zones;
    std::vector<Page> m_pages;
    int m_lastId;
    int m_revision;

    /** Scene faders on scenes of no group (a group has the level of its
     *  scenes): the level of each scene that plays, and its overrides */
    QHash<quint32, QList<int>> m_overrides;
    QHash<quint32, qreal> m_levels;
    /** The level of the fader on a scene of no group as its intensity */
    void applySceneLevel(Function *f);
    /** "sceneRun" faders on scenes of no group (a group keeps the speed of
     *  the run, Zone::setRunSpeed): the speed each had before the fader gave
     *  it one, and the one it gave. Given back when it stops, whoever stops
     *  it, unless something else changed its speed meanwhile */
    struct RunSpeed
    {
        qreal before = 1.0;
        qreal given = 1.0;
    };
    QHash<quint32, RunSpeed> m_runSpeeds;
    void restoreRunSpeed(quint32 fid);

    /** The dimmers of the scenes (setSceneDimmer), below 1 only */
    QHash<quint32, qreal> m_sceneDimmers;
    /** A dimmer is the base of the intensity of the function (the levels of
     *  the groups, playbacks and faders are overrides that multiply it); a
     *  super scene has one per track, and its static values */
    void applySceneDimmer(Function *f, qreal dimmer);
    /** No dimmers (another show): the scenes still there back to full */
    void clearSceneDimmers();
    /** The dimmers no "sceneDimmer" fader or dial is on any more go (their
     *  scenes back to full): a dimmer lives with its controls */
    void pruneSceneDimmers();
    /** The scenes no "scene" or "sceneRun" fader or dial is on any more are
     *  let go (they play on): a hold lives with its controls */
    void pruneHolds();
    /** The scenes the faders and dials of every page have as targets of
     *  the $kinds ("sceneDimmer", "scene"…); $shownOnly: only those of
     *  faders and dials drawn (not out of their table, made smaller) */
    QSet<quint32> controlledScenes(const QStringList &kinds, bool shownOnly = false) const;
    /** At the bottom of the travel, a scene or a playback fader stops it */
    static bool bottomStops(const QVariantMap &target);
    /** The scenes the targets of a control start ("scene", "sceneRun") */
    static QList<quint32> startedScenes(const QVariantList &targets);

    void setTargetLevel(const QVariantMap &target, qreal level);
    qreal targetLevel(const QVariantMap &target) const;
    /** The fixtures of a movement speed fader: of its group, or every one
     *  with pan or tilt (-1) */
    QSet<quint32> motionFixtures(int groupId) const;
    /** $f moves pan or tilt of one of $fixtures: an FX of pan/tilt, a Move
     *  FX, steps with positions, a cuelist with an FX of pan/tilt */
    bool movesHeads(Function *f, const QSet<quint32> &fixtures) const;
    /** The speed of the movement of every fader on what moves its heads (the
     *  last fader moved last); the rest at its own speed */
    void applyMotion();

    LiveMixer *m_mixer;
    DtcCore *m_core;
    int m_currentPage;
    /** Movement speed faders: the level of each group, x0.1 to x10 */
    QHash<int, qreal> m_speedLevels;
    /** The fixture groups of the speed faders moved, the last one moved last */
    QList<int> m_speedOrder;
};

#endif
