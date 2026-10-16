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
#include <QList>
#include <memory>
#include <vector>

class QXmlStreamReader;
class QXmlStreamWriter;
class ZoneManager;
class GenericDMXSource;
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
 *  "bpm"      BPM, tap, BeatGO and the pulse meter
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
 * intensity), "sceneSpeed" (functionId), "mixerDimmer" (mixerId), "moveSpeed"
 * (groupId: the pan/tilt speed channel of a fixture group, -1 all the moving
 * heads), "bpm", "map" (map: a fader target of the mappings). Older items have
 * a single "target" with its keys. "flash":
 * a button under the fader puts it at full while pressed (default true).
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

    /** Faders, 0..1: every target of the item follows */
    Q_INVOKABLE void setLevel(const QVariantMap &item, qreal level);
    /** The level of a fader: the one of its first target */
    Q_INVOKABLE qreal level(const QVariantMap &item) const;
    /** The targets of a fader item (older items: their single target) */
    Q_INVOKABLE static QVariantList faderTargets(const QVariantMap &item);

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
    /** The patch changed: the movement speed faders on the channels of now */
    void slotFixturesChanged();

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

    void setTargetLevel(const QVariantMap &target, qreal level);
    qreal targetLevel(const QVariantMap &target) const;
    /** The pan/tilt speed channels of a fixture group (-1: every moving head):
     *  fixture, channel and whether 0 is the fast end */
    struct SpeedChannel { quint32 fixture; quint32 channel; bool fastFirst; };
    QList<SpeedChannel> speedChannels(int groupId) const;
    /** A movement speed fader on the speed channels of its group */
    void applySpeed(int groupId, qreal level);

    LiveMixer *m_mixer;
    DtcCore *m_core;
    int m_currentPage;
    /** Movement speed faders write the speed channels of the moving heads */
    GenericDMXSource *m_speedSource;
    QHash<int, qreal> m_speedLevels;
    /** The fixture groups of the speed faders moved, the last one moved last */
    QList<int> m_speedOrder;
};

#endif
