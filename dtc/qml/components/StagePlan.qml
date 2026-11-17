/*
  DMX Total Control
  StagePlan.qml

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

import QtQuick
import QtQuick.Controls
import DMXTotalControl
import DMXTotalControl.Backend

/**
 * Top-down 2D stage plan. Fixtures are placed in millimeters; the plan
 * scales to fit, the wheel zooms and a right (or middle) drag moves it.
 * Click to select, Ctrl/Shift+click to add, drag on the empty stage for a
 * rectangle (or a lasso) selection, and drag fixtures to move them when
 * editable. Fixtures with several heads (bars, matrices) show their cells.
 * Each fixture shows the glyph of its kind (a moving head, a strobe, a CO2
 * jet...) in the colour of its type (beside its cells, smaller), which the
 * legend lists: a click there selects the fixtures of a type. The stage
 * fits beside or below the legend. Live, each fixture shows its colour,
 * dimmer, strobe and gobo. Editable (the Patch page), each side of the
 * stage has a «−» and a «+» outside it: a metre less or more on that side,
 * the fixtures staying where they are on the stage. Fixtures can be locked
 * (lockedIds: the Patch page, a group isolated): faded, nothing selects or
 * moves them, and a chip says so (lockText), with an × that asks to unlock
 * them.
 */
Rectangle {
    id: plan

    property var fixtures: []
    property var selection: []
    property bool editable: false
    /** Show what each fixture outputs */
    property bool live: false
    /** Free-hand selection instead of a rectangle */
    property bool lasso: false
    property real zoom: 1
    property point pan: Qt.point(0, 0)
    /** "select": click and draw around fixtures; "move": drag them (the whole
     *  selection), dragging the empty stage moves the view */
    property string tool: "select"
    /** Grid step in mm the moved fixtures land on (0: free) */
    property real snap: 0
    /** The names under the fixtures */
    property bool showNames: true
    /** The legend: the fixture types, their glyph and colour */
    property bool showLegend: true
    /** Core.patch.fixtureTypes: [{ key, manufacturer, model, kind, colorIndex, count, ids }] */
    readonly property var types: Core.patch.fixtureTypes
    /** The fixtures with parked channels (a lock on them) */
    readonly property var parked: Core.safety.parkedFixtures

    // a drag of fixtures in progress: the others of the selection follow
    property int draggingId: -1
    property real dragDX: 0
    property real dragDY: 0

    /** Follow: a click or a drag on the stage picks the point the moving
     *  heads aim at; the fixtures aren't selected nor moved */
    property bool aiming: false
    /** The point they aim at ({ x, y } in mm), or null */
    property var target: null
    /** The fixtures aiming at it: a line from each one */
    property var aimedIds: []

    /** The fixtures that can't be picked: an object used as a set (id →
     *  true). Faded; a click, the rubber band, the lasso, the legend and a
     *  drag pass them by (a click on one does nothing). None by default */
    property var lockedIds: ({})
    readonly property bool anyLocked: {
        for (const id in lockedIds)
            return true
        return false
    }
    /** What the chip over the plan says while fixtures are locked (empty: no
     *  chip). Where it doesn't fit, the chip says lockName · lockCount when
     *  they are given (the name shortened in the middle, the count whole) */
    property string lockText: ""
    property string lockName: ""
    property string lockCount: ""
    /** A locked fixture at x, y of the plan: a click there does nothing (the
     *  pointer goes through it: a rubber band starts there, a drag moves the
     *  view) */
    function lockedAt(x, y) {
        if (!anyLocked)
            return false
        for (let i = 0; i < fixtureItems.count; i++) {
            const item = fixtureItems.itemAt(i)
            if (item !== null && item.locked && item.contains(item.mapFromItem(plan, x, y)))
                return true
        }
        return false
    }
    /** $ids without the locked fixtures, in their order */
    function pickable(ids) {
        const list = Array.from(ids || [])
        return anyLocked ? list.filter(id => lockedIds[id] !== true) : list
    }

    /** ids, additive */
    signal selectRequested(var ids, bool additive)
    /** move fixtures $ids by dx, dy (mm) */
    signal movedBy(var ids, real dx, real dy)
    /** Follow: the point picked on the stage (mm) */
    signal pointPicked(real x, real y)
    /** The × of the chip: unlock every fixture */
    signal unlockRequested()
    /** A right click on fixture $id (not locked) at $x, $y of the plan,
     *  without moving the view: its menu */
    signal contextRequested(int id, real x, real y)

    /** The fixture (not locked) at $x, $y of the plan, the one on top; -1: none */
    function fixtureAt(x, y) {
        for (let i = fixtureItems.count - 1; i >= 0; i--) {
            const item = fixtureItems.itemAt(i)
            if (item !== null && !item.locked && item.contains(item.mapFromItem(plan, x, y)))
                return item.modelData.id
        }
        return -1
    }

    readonly property size stage: Core.patch.stageSize
    /** The «−» and «+» on each side of the stage: a metre less or more on
     *  that side (the Patch page) */
    property bool sideControls: editable
    /** Core.patch.stageRoom: the room free of fixtures on each side (mm),
     *  the smallest and the largest stage */
    readonly property var room: sideControls ? Core.patch.stageRoom : ({})
    /** The room the controls of a side take outside the stage (px), and the
     *  margin of the stage in the plan */
    readonly property real sideBand: sideControls ? 32 : 0
    readonly property real edge: sideControls ? 36 : 20
    /** The bottom and the right of the legend alone (0: no legend) */
    readonly property real legendBottom: legend.visible ? legend.y + legend.height + 8 : 0
    readonly property real legendEnd: legend.visible ? legend.x + legend.width + 8 : 0
    /** The bottom and the right of the legend and the chip of the locked
     *  fixtures (0: neither): the stage goes below them or beside them, not
     *  under them (the chip under the legend stays left of where the stage
     *  is beside the legend alone: it doesn't move it) */
    readonly property real legendRoom: Math.max(legendBottom, lockChip.visible ? lockChip.y + lockChip.height + 8 : 0)
    readonly property real legendRight: Math.max(legendEnd, lockChip.visible && !chipUnderLegend ? lockChip.x + lockChip.width + 8 : 0)
    // the scale (px per mm) the whole stage fits in: the whole plan, below
    // the legend, beside it (with the controls of the sides between them)
    function fitBelowAt(bottom) {
        return Math.min((width - 2 * edge) / Math.max(1, stage.width),
                        (height - bottom - sideBand - edge) / Math.max(1, stage.height))
    }
    function fitBesideAt(right) {
        return Math.min((width - right - sideBand - edge) / Math.max(1, stage.width),
                        (height - 2 * edge) / Math.max(1, stage.height))
    }
    readonly property real fitFree: Math.min((width - 2 * edge) / Math.max(1, stage.width),
                                             (height - 2 * edge) / Math.max(1, stage.height))
    readonly property real fitBelow: fitBelowAt(legendRoom)
    readonly property real fitBeside: fitBesideAt(legendRight)
    /** The chip goes under the legend when the stage goes beside the legend
     *  (else beside it, at the top): the stage stays where it was (a long
     *  legend leaves room for it) */
    readonly property bool chipUnderLegend: legend.visible && fitBesideAt(legendEnd) > fitBelowAt(legendBottom)
    /** Under the legend, the chip goes as far as the legend, or up to the
     *  controls of the left side of the stage: where they are once the plan
     *  fits, or where they are now when the stage is further left (zoomed in,
     *  or a side grown while the view stays: it never covers a fixture) */
    readonly property real chipRoomUnderLegend: Math.max(legend.width, Math.min(leftAt(fitScale), stageX) - sideBand - 8 - lockChip.x)
    /** The lowest bottom (the rightmost right) of the legend and the chip
     *  where a control of a side at x, w (y, h) would meet them, else 4 */
    function cornerBottomAt(x, w) {
        let bottom = 4
        if (legend.visible && x < legendEnd && x + w > legend.x)
            bottom = Math.max(bottom, legendBottom)
        if (lockChip.visible && x < lockChip.x + lockChip.width + 8 && x + w > lockChip.x)
            bottom = Math.max(bottom, lockChip.y + lockChip.height + 8)
        return bottom
    }
    function cornerRightAt(y, h) {
        let right = 4
        if (legend.visible && y < legendBottom && y + h > legend.y)
            right = Math.max(right, legendEnd)
        if (lockChip.visible && y < lockChip.y + lockChip.height + 8 && y + h > lockChip.y)
            right = Math.max(right, lockChip.x + lockChip.width + 8)
        return right
    }
    /** Around the legend, where it is the bigger, unless that leaves the stage
     *  less than half its size (a long legend on a small plan: it goes over
     *  the stage, and folds) */
    readonly property bool aroundLegend: (legend.visible || lockChip.visible) && Math.max(fitBelow, fitBeside) >= fitFree / 2
    readonly property bool besideLegend: aroundLegend && fitBeside > fitBelow
    readonly property real fitScale: aroundLegend ? Math.max(1e-6, Math.min(fitFree, Math.max(fitBelow, fitBeside)))
                                                  : fitFree
    readonly property real mmScale: fitScale * zoom
    readonly property real stageX: leftAt(mmScale) + pan.x
    readonly property real stageY: topAt(mmScale) + pan.y
    /** Where the left of the stage is at $scale (px per mm), the view not
     *  moved: in the middle of the plan, or of the room beside the legend */
    function leftAt(scale) {
        const w = stage.width * scale
        return besideLegend ? legendRight + sideBand + (width - legendRight - sideBand - edge - w) / 2 : (width - w) / 2
    }
    /** Where the top of the stage is at $scale, the view not moved: in the
     *  middle, or below the legend when there is room for it */
    function topAt(scale) {
        const h = stage.height * scale
        if (besideLegend)
            return (height - h) / 2
        return Math.max((height - h) / 2, Math.min(legendRoom + sideBand, height - edge - h))
    }
    // a small plan: smaller fixtures (as big as 50 cm of the stage, from 45 %
    // on: room for a glyph that reads)
    readonly property real bodyScale: Math.max(0.45, Math.min(1, mmScale * 500 / 40))
    /** The glyph of each fixture: never below a size that reads */
    readonly property int glyphSize: Math.max(12, Math.round(20 * bodyScale))

    /** What each fixture shows, by id (live) */
    property var states: ({})

    /** The room of the name of each fixture, by id (px): up to the nearest
     *  fixture beside it, so the names don't run over each other */
    readonly property var labelRooms: {
        const list = plan.fixtures || []
        const s = plan.mmScale
        let rooms = {}
        for (let i = 0; i < list.length; i++) {
            let room = 90
            for (let j = 0; j < list.length; j++) {
                if (i !== j && Math.abs(list[j].y - list[i].y) * s < 16)
                    room = Math.min(room, Math.abs(list[j].x - list[i].x) * s)
            }
            rooms[list[i].id] = room
        }
        return rooms
    }

    function resetView() { zoom = 1; pan = Qt.point(0, 0) }

    // the controls of the sides: while one is under the pointer or pressed,
    // a step on a side doesn't fit the plan again (the fixtures stay where
    // they are on the screen, the control under the pointer); once they are
    // left, it fits again if it fitted before
    property int sidesHeld: 0
    property bool fittedBeforeSides: true
    property bool sidesResized: false
    function holdSide(on) {
        if (on) {
            if (sidesHeld === 0) {
                fittedBeforeSides = zoom === 1 && pan.x === 0 && pan.y === 0
                sidesResized = false
            }
            sidesHeld++
            return
        }
        sidesHeld = Math.max(0, sidesHeld - 1)
        if (sidesHeld === 0 && sidesResized && fittedBeforeSides)
            resetView()
    }
    /** $step metres more (or less, below 0) on a $side of the stage: the
     *  scale stays and the rest of the stage stays where it is on the
     *  screen, the strip appears (or goes) on that side */
    function resizeSide(side, step) {
        const s = mmScale, x0 = stageX, y0 = stageY
        const fitted = zoom === 1 && pan.x === 0 && pan.y === 0
        if (!Core.patch.resizeStageSide(side, step * 1000))
            return false
        // (no control held, a tap on a touch screen: the plan fits again)
        if (sidesHeld === 0 && fitted)
            return true
        const d = step * 1000 * s
        zoom = s / fitScale
        pan = Qt.point((side === "left" ? x0 - d : x0) - leftAt(s),
                       (side === "top" ? y0 - d : y0) - topAt(s))
        sidesResized = true
        return true
    }

    function zoomAt(px, py, factor) {
        // (zoomed by hand: the view stays so once the sides are left)
        fittedBeforeSides = false
        let mmX = (px - stageX) / mmScale, mmY = (py - stageY) / mmScale
        // from 50 % to 10 times, never the other way than asked: a side grown
        // or shrunk while the view stayed may have left the zoom beyond them
        const z = zoom * factor
        zoom = factor > 1 ? Math.max(zoom, Math.min(10, z)) : Math.min(zoom, Math.max(0.5, z))
        let s = fitScale * zoom
        pan = Qt.point(px - mmX * s - leftAt(s),
                       py - mmY * s - topAt(s))
    }

    // a point inside a polygon (lasso)
    function inside(px, py, poly) {
        let hit = false
        for (let i = 0, j = poly.length - 1; i < poly.length; j = i++) {
            let a = poly[i], b = poly[j]
            if ((a.y > py) !== (b.y > py) && px < (b.x - a.x) * (py - a.y) / (b.y - a.y) + a.x)
                hit = !hit
        }
        return hit
    }

    color: Theme.background
    radius: Theme.radius
    clip: true

    Timer {
        interval: 100
        repeat: true
        running: plan.live && plan.visible
        triggeredOnStart: true
        onTriggered: {
            let map = {}
            for (let s of Core.patch.liveStates())
                map[s.id] = s
            plan.states = map
        }
    }
    onLiveChanged: if (!live) states = ({})

    // stage floor
    Rectangle {
        id: floor
        x: plan.stageX
        y: plan.stageY
        width: plan.stage.width * plan.mmScale
        height: plan.stage.height * plan.mmScale
        color: Theme.planGrid
        border.color: Theme.borderStrong
        radius: 4

        // one line per meter, and the grid fixtures snap to
        readonly property real gridStep: plan.snap > 0 && plan.snap * plan.mmScale >= 6 ? plan.snap : 1000
        Repeater {
            model: Math.floor(plan.stage.width / floor.gridStep)
            Rectangle {
                required property int index
                readonly property bool meter: Math.abs(((index + 1) * floor.gridStep) % 1000) < 1
                x: (index + 1) * floor.gridStep * plan.mmScale
                width: 1
                height: floor.height
                color: Theme.border
                opacity: meter ? 0.6 : 0.25
            }
        }
        Repeater {
            model: Math.floor(plan.stage.height / floor.gridStep)
            Rectangle {
                required property int index
                readonly property bool meter: Math.abs(((index + 1) * floor.gridStep) % 1000) < 1
                y: (index + 1) * floor.gridStep * plan.mmScale
                height: 1
                width: floor.width
                color: Theme.border
                opacity: meter ? 0.6 : 0.25
            }
        }

        Text {
            anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 6 }
            text: qsTr("AUDIENCE")
            color: Theme.textFaint
            font.pixelSize: 10
            font.letterSpacing: 2
        }
        Text {
            anchors { right: parent.right; top: parent.top; margins: 6 }
            text: (plan.stage.width / 1000).toLocaleString(Qt.locale(), 'f', 1) + " × "
                  + (plan.stage.height / 1000).toLocaleString(Qt.locale(), 'f', 1) + " m"
            color: Theme.textFaint
            font.pixelSize: 10
        }
    }

    // background: clear the selection, rubber band or lasso, move the view
    MouseArea {
        id: bandArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
        property point start
        property point startPan
        property bool banding: false
        property bool panning: false
        property var path: []

        onPressed: (ev) => {
            start = Qt.point(ev.x, ev.y)
            banding = false
            // the move tool moves the view with the left button too
            panning = ev.button !== Qt.LeftButton || plan.tool === "move"
            startPan = plan.pan
            path = [Qt.point(ev.x, ev.y)]
        }
        onPositionChanged: (ev) => {
            if (panning) {
                plan.pan = Qt.point(startPan.x + ev.x - start.x, startPan.y + ev.y - start.y)
                return
            }
            if (!banding && (Math.abs(ev.x - start.x) > 4 || Math.abs(ev.y - start.y) > 4))
                banding = true
            if (!banding)
                return
            if (plan.lasso) {
                let p = path.slice()
                p.push(Qt.point(ev.x, ev.y))
                path = p
                lassoCanvas.requestPaint()
            } else {
                band.x = Math.min(start.x, ev.x); band.y = Math.min(start.y, ev.y)
                band.width = Math.abs(ev.x - start.x); band.height = Math.abs(ev.y - start.y)
            }
        }
        onReleased: (ev) => {
            if (panning) {
                panning = false
                const still = Math.abs(ev.x - start.x) < 4 && Math.abs(ev.y - start.y) < 4
                // a right click on a fixture (the view didn't move): its menu
                if (ev.button === Qt.RightButton && still) {
                    const id = plan.fixtureAt(ev.x, ev.y)
                    if (id >= 0) {
                        plan.pan = startPan
                        plan.contextRequested(id, ev.x, ev.y)
                    }
                    return
                }
                // a click on the empty stage with the move tool: nothing
                // selected (on a locked fixture: nothing happens)
                if (ev.button === Qt.LeftButton && still && !plan.lockedAt(ev.x, ev.y))
                    plan.selectRequested([], false)
                return
            }
            let additive = (ev.modifiers & (Qt.ControlModifier | Qt.ShiftModifier)) !== 0
            if (banding) {
                // (the locked fixtures aren't caught)
                let ids = []
                for (let f of plan.fixtures) {
                    if (plan.lockedIds[f.id] === true)
                        continue
                    let fx = plan.stageX + f.x * plan.mmScale
                    let fy = plan.stageY + f.y * plan.mmScale
                    let hit = plan.lasso ? plan.inside(fx, fy, path)
                                         : fx >= band.x && fx <= band.x + band.width && fy >= band.y && fy <= band.y + band.height
                    if (hit)
                        ids.push(f.id)
                }
                plan.selectRequested(ids, additive)
            } else if (!additive && !plan.lockedAt(ev.x, ev.y)) {
                plan.selectRequested([], false)
            }
            banding = false
            path = []
            lassoCanvas.requestPaint()
        }
        onDoubleClicked: (ev) => { if (ev.button === Qt.RightButton || ev.button === Qt.MiddleButton) plan.resetView() }
        onWheel: (ev) => plan.zoomAt(ev.x, ev.y, ev.angleDelta.y > 0 ? 1.15 : 1 / 1.15)
    }

    Rectangle {
        id: band
        visible: bandArea.banding && !plan.lasso
        color: Qt.alpha(Theme.accent, 0.15)
        border.color: Theme.accent
    }

    // a «−» or a «+» of a side of the stage: a click, a metre; held, it goes
    // on (a metre a tenth of a second). Its tooltip says what it does, or
    // why it can't
    component SideButton: Item {
        id: sideButton
        /** The plan (an inline component doesn't see its ids) */
        property Item view
        /** "left", "right", "top" (the back) or "bottom" (the front) */
        property string side
        /** 1: a metre more on that side, -1: a metre less */
        property int step: 1
        property bool can: true
        /** The most metres it can take (or give) now */
        property int most: 1
        property string tip
        readonly property bool pressed: button.pressed
        width: 22
        height: 22

        AppButton {
            id: button
            anchors.fill: parent
            flat: true
            implicitHeight: 22
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0
            iconGlyph: sideButton.step > 0 ? Theme.iconPlus : Theme.iconMinus
            iconSize: 10
            enabled: sideButton.can
            Accessible.name: sideButton.tip
            // (held, the timer made the steps: also when the pointer slid off
            // the button and back, which stops the timer and starts it again)
            onPressed: holdSteps.stepped = false
            onClicked: if (!holdSteps.stepped) sideButton.view.resizeSide(sideButton.side, sideButton.step)
        }
        // held: from 0.3 s on, a metre a tenth of a second. A step that keeps
        // the program busy longer (the left or the back of a big rig, where
        // every fixture moves) is made up with the next one: a few metres at
        // once (up to a second of them)
        Timer {
            id: holdSteps
            interval: 100
            repeat: true
            running: button.pressed
            property double last: 0
            /** A step made since the button was pressed */
            property bool stepped: false
            onRunningChanged: if (running) last = Date.now() + 200
            onTriggered: {
                const now = Date.now()
                const due = Math.round((now - last) / 100)
                const n = Math.min(due, 10, sideButton.most)
                if (n < 1)
                    return
                last = n < due ? now : last + n * 100
                stepped = true
                sideButton.view.resizeSide(sideButton.side, sideButton.step * n)
            }
        }
        // (on the item, not the button: disabled, it says why. Its own tip:
        // what it says changes with a click, also before it shows)
        HoverHandler { id: sideHover }
        ToolTip {
            visible: sideHover.hovered
            delay: 500
            text: sideButton.tip
        }
    }

    // the «−» and «+» of a side, outside the stage in the middle of that
    // side, beyond the fixtures near it when there is room (and under them:
    // they never cover one). Under the pointer or pressed, it stays where it
    // is: the side moves away from it or under it (into the strip it adds,
    // never over a fixture), and the next click lands on it again
    component SideControl: Rectangle {
        id: control
        /** The plan (an inline component doesn't see its ids) */
        property Item view
        property string side
        /** Where it goes, outside that side of the stage */
        property real edgeX
        property real edgeY
        readonly property bool held: hover.hovered || minus.pressed || plus.pressed
        property real heldX
        property real heldY
        x: held ? heldX : edgeX
        y: held ? heldY : edgeY
        onEdgeXChanged: if (!held) heldX = edgeX
        onEdgeYChanged: if (!held) heldY = edgeY
        // (held from where it is now, never from where an earlier hold left
        // it: a hold that ends without the plan fitting again moves it, and
        // its edge may not change again)
        onHeldChanged: {
            heldX = edgeX
            heldY = edgeY
            view.holdSide(held)
        }
        Component.onCompleted: { heldX = edgeX; heldY = edgeY }
        Component.onDestruction: if (held) view.holdSide(false)
        HoverHandler { id: hover }

        readonly property bool across: side === "left" || side === "right"
        // the size of the stage that way, the room free of fixtures on that
        // side, the smallest and largest stage (mm)
        readonly property real size: across ? view.stage.width : view.stage.height
        readonly property real free: view.room[side] ?? 0
        readonly property real minimum: view.room.minimum ?? 1000
        readonly property real maximum: view.room.maximum ?? 200000
        readonly property bool canGrow: size + 1000 <= maximum + 0.5
        readonly property bool tooSmall: size - 1000 < minimum - 0.5
        // a fixture in the metre that would go, or on its inner edge
        readonly property bool blocked: free <= 1000
        // the most metres that side can take, and give: free of fixtures,
        // and the metre past the last one too (none is left on the edge)
        readonly property int growth: Math.floor((maximum - size + 0.5) / 1000)
        readonly property int shrink: Math.min(Math.floor((size - minimum + 0.5) / 1000), Math.ceil(free / 1000) - 1)
        /** How far from the stage it goes (px): beyond the fixture nearest
         *  that side (its body, and its name below it at the front) */
        readonly property real gap: {
            const reach = 24 * view.bodyScale + (side === "bottom" && view.showNames ? 16 : 0)
            return 6 + Math.max(0, reach - free * view.mmScale)
        }
        width: across ? 26 : 48
        height: across ? 48 : 26
        radius: Theme.radiusSmall
        color: Qt.alpha(Theme.surface, 0.94)
        border.color: Theme.border

        function addTip() {
            switch (side) {
            case "left": return qsTr("Add 1 m on the left")
            case "right": return qsTr("Add 1 m on the right")
            case "top": return qsTr("Add 1 m at the back of the stage")
            default: return qsTr("Add 1 m at the front, on the audience side")
            }
        }
        function removeTip() {
            switch (side) {
            case "left": return qsTr("Remove 1 m on the left")
            case "right": return qsTr("Remove 1 m on the right")
            case "top": return qsTr("Remove 1 m at the back of the stage")
            default: return qsTr("Remove 1 m at the front, on the audience side")
            }
        }

        // (a click on it, or on a button that can't, doesn't reach the plan
        // under it: the selection stays; the wheel still zooms)
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
        }

        // + above − on the left and the right; − before + at the back and the front
        SideButton {
            id: minus
            x: 2
            y: control.across ? 24 : 2
            view: control.view
            side: control.side
            step: -1
            can: !control.tooSmall && !control.blocked
            most: control.shrink
            tip: control.tooSmall ? qsTr("The stage can't be smaller than %1 m").arg(control.minimum / 1000)
               : control.blocked ? qsTr("There is a fixture within 1 m of this side: move it first")
               : control.removeTip()
        }
        SideButton {
            id: plus
            x: control.across ? 2 : 24
            y: 2
            view: control.view
            side: control.side
            step: 1
            can: control.canGrow
            most: control.growth
            tip: control.canGrow ? control.addTip() : qsTr("The stage can't be larger than %1 m").arg(control.maximum / 1000)
        }
    }

    Loader {
        anchors.fill: parent
        active: plan.sideControls
        sourceComponent: Item {
            // (where the stage leaves no room, they stay inside the plan, and
            // off the legend and the chip of the locked fixtures)
            SideControl {
                view: plan
                side: "top"
                readonly property real limit: plan.cornerBottomAt(edgeX, width)
                edgeX: plan.stageX + floor.width / 2 - width / 2
                edgeY: Math.max(Math.min(limit, plan.stageY - 6 - height), plan.stageY - gap - height)
            }
            SideControl {
                view: plan
                side: "bottom"
                edgeX: plan.stageX + floor.width / 2 - width / 2
                edgeY: Math.min(Math.max(plan.height - 4 - height, plan.stageY + floor.height + 6),
                                plan.stageY + floor.height + gap)
            }
            SideControl {
                view: plan
                side: "left"
                readonly property real limit: plan.cornerRightAt(edgeY, height)
                edgeX: Math.max(Math.min(limit, plan.stageX - 6 - width), plan.stageX - gap - width)
                edgeY: plan.stageY + floor.height / 2 - height / 2
            }
            SideControl {
                view: plan
                side: "right"
                edgeX: Math.min(Math.max(plan.width - 4 - width, plan.stageX + floor.width + 6),
                                plan.stageX + floor.width + gap)
                edgeY: plan.stageY + floor.height / 2 - height / 2
            }
        }
    }
    Canvas {
        id: lassoCanvas
        anchors.fill: parent
        z: 20
        onPaint: {
            let ctx = getContext("2d")
            ctx.reset()
            let path = bandArea.path
            if (!bandArea.banding || !plan.lasso || path.length < 2)
                return
            ctx.fillStyle = Qt.alpha(Theme.accent, 0.15)
            ctx.strokeStyle = Theme.accent
            ctx.lineWidth = 1.5
            ctx.beginPath()
            ctx.moveTo(path[0].x, path[0].y)
            for (let i = 1; i < path.length; i++)
                ctx.lineTo(path[i].x, path[i].y)
            ctx.closePath()
            ctx.fill()
            ctx.stroke()
        }
    }

    Repeater {
        id: fixtureItems
        model: plan.fixtures

        delegate: Item {
            id: fxItem
            required property var modelData
            readonly property int selectionIndex: plan.selection.indexOf(modelData.id)
            readonly property bool selected: selectionIndex >= 0
            // locked: faded (its name too) and below the others; the pointer
            // goes through it to the stage
            readonly property bool locked: plan.lockedIds[modelData.id] === true
            opacity: locked ? 0.28 : 1
            readonly property var liveState: plan.states[modelData.id]
            readonly property bool lit: plan.live && liveState !== undefined
            // live and giving light: its colours; dark, it shows its kind
            readonly property bool shining: lit && liveState.dimmer > 0.02
            // the colour of its type (the glyph, the edge of its body)
            readonly property color typeColor: Theme.fixtureTypeColor(modelData.colorIndex)
            readonly property int heads: modelData.heads || 1
            // its heads as a row of cells (a bar, a matrix; not a generic
            // dimmer standing in for one CO2 jet with two channels)
            readonly property bool bar: !!modelData.cells

            // a bar: its real length, at least a few pixels per head
            readonly property real barLength: Math.max(heads * 6, (modelData.width > 0 ? modelData.width : heads * 100) * plan.mmScale)
            readonly property real barThickness: Math.max(4, Math.min(22, barLength / heads))
            // the shape chosen in the plan: round, square or a rectangle
            readonly property string shape: modelData.shape || "round"
            readonly property real bodyWidth: bar ? barLength : (shape === "rectangle" ? 58 : 40) * plan.bodyScale
            readonly property real bodyHeight: bar ? barThickness : (shape === "rectangle" ? 30 : 40) * plan.bodyScale
            readonly property real angle: (modelData.rotation || 0) * Math.PI / 180
            // bounding box of the turned body
            width: Math.abs(bodyWidth * Math.cos(angle)) + Math.abs(bodyHeight * Math.sin(angle))
            height: Math.abs(bodyWidth * Math.sin(angle)) + Math.abs(bodyHeight * Math.cos(angle))
            // the other fixtures of the selection follow the one dragged
            readonly property bool following: plan.draggingId >= 0 && plan.draggingId !== modelData.id && selected && !locked
            x: plan.stageX + modelData.x * plan.mmScale - width / 2 + (following ? plan.dragDX : 0)
            y: plan.stageY + modelData.y * plan.mmScale - height / 2 + (following ? plan.dragDY : 0)
            z: dragArea.drag.active ? 10 : (selected ? 2 : (locked ? 0.5 : 1))

            // the light it gives, live
            Rectangle {
                anchors.centerIn: parent
                visible: fxItem.lit && !fxItem.bar && fxItem.liveState.dimmer > 0.02
                width: (40 + 26 * (fxItem.lit ? fxItem.liveState.dimmer : 0)) * plan.bodyScale
                height: width
                radius: width / 2
                color: fxItem.lit ? fxItem.liveState.colors[0] : "transparent"
                opacity: 0.35
            }

            Item {
                id: body
                anchors.centerIn: parent
                width: fxItem.bodyWidth
                height: fxItem.bodyHeight
                rotation: fxItem.modelData.rotation || 0

                // one fixture: a circle, a square or a rectangle
                Rectangle {
                    visible: !fxItem.bar
                    anchors.fill: parent
                    radius: fxItem.shape === "round" ? width / 2 : 5
                    color: fxItem.lit ? fxItem.liveState.colors[0]
                                      : (fxItem.selected ? Theme.accentFill : Theme.surfaceRaised)
                    border.color: fxItem.selected ? Qt.lighter(Theme.accent, 1.4)
                                                  : (fxItem.shining ? Theme.borderStrong : fxItem.typeColor)
                    border.width: fxItem.selected ? 3 : 2
                }
                // several heads: a row of cells, pixel 1 on the left
                Rectangle {
                    visible: fxItem.bar
                    anchors.fill: parent
                    radius: 3
                    color: Theme.surfaceRaised
                    border.color: fxItem.selected ? Qt.lighter(Theme.accent, 1.4) : fxItem.typeColor
                    border.width: fxItem.selected ? 2 : (body.height > 8 ? 1 : 0)
                    Row {
                        id: cells
                        anchors { fill: parent; margins: body.height > 8 ? 2 : 0 }
                        Repeater {
                            model: fxItem.bar ? fxItem.heads : 0
                            Rectangle {
                                required property int index
                                width: cells.width / fxItem.heads
                                height: cells.height
                                color: fxItem.lit && fxItem.liveState.colors[index] !== undefined ? fxItem.liveState.colors[index]
                                     : (fxItem.selected ? Qt.alpha(Theme.accent, 0.6) : Theme.planLine)
                                border.color: Theme.planGrid
                                border.width: width > 5 ? 1 : 0
                            }
                        }
                    }
                }
            }

            // strobe: it flashes
            Rectangle {
                id: flash
                anchors.centerIn: parent
                width: fxItem.bodyWidth + 8
                height: fxItem.bodyHeight + 8
                rotation: body.rotation
                radius: fxItem.bar || fxItem.shape !== "round" ? 5 : width / 2
                color: "white"
                opacity: 0
                visible: fxItem.lit && fxItem.liveState.strobe
                SequentialAnimation on opacity {
                    running: flash.visible
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.85; duration: 40 }
                    NumberAnimation { to: 0; duration: 110 }
                }
            }

            // what it is (its kind), in the colour of its type
            IconLabel {
                anchors.centerIn: parent
                visible: !fxItem.bar && !fxItem.selected && !fxItem.shining
                icon: Theme.kindIcon(fxItem.modelData.kind)
                size: plan.glyphSize
                color: fxItem.typeColor
            }
            // a row of cells: its glyph beside them, smaller (an LED bar's
            // cells say it, unless an icon was chosen for it)
            Rectangle {
                visible: fxItem.bar && !fxItem.selected && !fxItem.shining
                         && (fxItem.modelData.kind !== "ledbar" || !!fxItem.modelData.icon)
                anchors { right: parent.left; rightMargin: 2; verticalCenter: parent.verticalCenter }
                width: badgeGlyph.size + 6
                height: width
                radius: width / 2
                color: Theme.surfaceRaised
                IconLabel {
                    id: badgeGlyph
                    anchors.centerIn: parent
                    icon: Theme.kindIcon(fxItem.modelData.kind)
                    size: Math.max(10, Math.round(plan.glyphSize * 0.7))
                    color: fxItem.typeColor
                }
            }
            // selection index: effects and fans follow this order
            Rectangle {
                anchors.centerIn: parent
                visible: fxItem.selected
                width: indexText.implicitWidth + 10
                height: 20
                radius: 10
                color: fxItem.lit || fxItem.bar ? Qt.alpha("black", 0.6) : "transparent"
                Text {
                    id: indexText
                    anchors.centerIn: parent
                    text: fxItem.selectionIndex + 1
                    color: fxItem.lit || fxItem.bar ? Theme.textOnShade : Theme.accentText
                    font.pixelSize: fxItem.bar || plan.bodyScale < 0.8 ? Theme.fontSmall : Theme.fontLarge
                    font.weight: Font.Bold
                }
            }
            // a protected special effect: a shield at its top left, red while
            // the effects are armed, glowing while it fires
            Rectangle {
                id: shieldBadge
                visible: !!fxItem.modelData.protected
                readonly property bool armed: Core.safety.armed
                // (read only on a protected one: the others never fire)
                readonly property bool firing: {
                    if (!fxItem.modelData.protected)
                        return false
                    const list = Core.safety.firing
                    for (let i = 0; i < list.length; i++)
                        if (list[i].id === fxItem.modelData.id)
                            return true
                    return false
                }
                anchors { left: parent.left; top: parent.top; margins: -6 }
                width: 18; height: 18; radius: 9
                color: armed ? Theme.danger : Theme.plan
                border.color: firing ? Theme.warning : (armed ? Theme.danger : Theme.borderStrong)
                border.width: firing ? 2 : 1
                IconLabel {
                    anchors.centerIn: parent
                    icon: Theme.iconShield
                    size: 10
                    color: shieldBadge.armed ? Theme.textOn(Theme.danger) : Theme.textDim
                }
                // (a glow that pulses while it fires)
                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width + 8; height: width; radius: width / 2
                    color: "transparent"
                    border.color: Theme.warning
                    border.width: 2
                    visible: shieldBadge.firing
                    SequentialAnimation on opacity {
                        running: shieldBadge.firing
                        loops: Animation.Infinite
                        NumberAnimation { from: 1; to: 0.2; duration: 300 }
                        NumberAnimation { from: 0.2; to: 1; duration: 300 }
                    }
                }
            }
            // parked: a lock at its top right
            Rectangle {
                visible: plan.parked[fxItem.modelData.id] !== undefined
                anchors { right: parent.right; top: parent.top; margins: -6 }
                width: 18; height: 18; radius: 9
                color: Theme.plan
                border.color: Theme.warning
                border.width: 1
                IconLabel {
                    anchors.centerIn: parent
                    icon: Theme.iconLock
                    size: 9
                    color: Theme.warning
                }
            }
            // gobo
            Rectangle {
                visible: fxItem.lit && fxItem.liveState.gobo.length > 0 && fxItem.liveState.dimmer > 0.02
                anchors { right: parent.right; bottom: parent.bottom; margins: -6 }
                width: 20; height: 20; radius: 10
                color: Theme.plan
                border.color: Theme.borderStrong
                Image {
                    anchors { fill: parent; margins: 2 }
                    visible: fxItem.lit && fxItem.liveState.goboImage.length > 0
                    source: fxItem.lit ? fxItem.liveState.goboImage : ""
                    sourceSize: Qt.size(32, 32)
                    fillMode: Image.PreserveAspectFit
                }
                Text {
                    anchors.centerIn: parent
                    visible: fxItem.lit && fxItem.liveState.goboImage.length === 0
                    text: "G"
                    color: Theme.text
                    font.pixelSize: 10
                    font.weight: Font.Bold
                }
            }

            // a moving head: the way it faces (where its pan is in the middle),
            // on the edge of its body. Small, a notch on the edge: the chevron
            // would cover its glyph
            Item {
                id: facing
                visible: !!fxItem.modelData.aimable && !fxItem.bar
                anchors.centerIn: parent
                width: fxItem.bodyWidth
                height: fxItem.bodyHeight
                rotation: fxItem.modelData.rotation || 0
                readonly property bool small: height < 30
                IconLabel {
                    visible: !facing.small
                    anchors { bottom: parent.bottom; bottomMargin: 1; horizontalCenter: parent.horizontalCenter }
                    icon: Theme.iconChevronDown
                    size: 10
                    color: fxItem.selected ? Theme.accentText : Theme.textDim
                }
                Rectangle {
                    visible: facing.small
                    width: 6
                    height: 6
                    rotation: 45
                    x: (facing.width - width) / 2
                    y: facing.height - height / 2 - 1
                    color: fxItem.selected ? Theme.accentText : Theme.textDim
                }
            }

            // (too close to the next one: no name, zoom in to read it)
            Text {
                anchors { top: parent.bottom; topMargin: 2; horizontalCenter: parent.horizontalCenter }
                width: fxItem.bar ? Math.max(90, parent.width) : Math.min(90, (plan.labelRooms[fxItem.modelData.id] ?? 90) - 6)
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                text: fxItem.modelData.name
                color: fxItem.selected ? Theme.text : Theme.textDim
                font.pixelSize: 10
                visible: plan.showNames && (fxItem.bar ? plan.zoom >= 1.5 || fxItem.selected : width >= 26)
            }

            MouseArea {
                id: dragArea
                anchors.fill: parent
                enabled: !fxItem.locked
                readonly property bool moving: plan.editable && plan.tool === "move"
                property real baseX: 0
                property real baseY: 0
                cursorShape: moving ? Qt.SizeAllCursor : Qt.PointingHandCursor
                drag.target: moving ? fxItem : null
                drag.threshold: 3
                onPressed: {
                    baseX = fxItem.x
                    baseY = fxItem.y
                    // the move tool takes the fixture into the selection
                    if (moving && !fxItem.selected)
                        plan.selectRequested([fxItem.modelData.id], false)
                }
                onPositionChanged: {
                    if (drag.active) {
                        plan.draggingId = fxItem.modelData.id
                        plan.dragDX = fxItem.x - baseX
                        plan.dragDY = fxItem.y - baseY
                    }
                }
                onClicked: (ev) => {
                    if (!moving)
                        plan.selectRequested([fxItem.modelData.id], (ev.modifiers & (Qt.ControlModifier | Qt.ShiftModifier)) !== 0)
                }
                onReleased: {
                    let dx = (fxItem.x - baseX) / plan.mmScale
                    let dy = (fxItem.y - baseY) / plan.mmScale
                    plan.draggingId = -1
                    plan.dragDX = 0
                    plan.dragDY = 0
                    // the drag broke its place: bind it again
                    fxItem.x = Qt.binding(() => plan.stageX + fxItem.modelData.x * plan.mmScale - fxItem.width / 2
                                                + (fxItem.following ? plan.dragDX : 0))
                    fxItem.y = Qt.binding(() => plan.stageY + fxItem.modelData.y * plan.mmScale - fxItem.height / 2
                                                + (fxItem.following ? plan.dragDY : 0))
                    if (moving && (Math.abs(dx) > 0.5 || Math.abs(dy) > 0.5)) {
                        // the whole selection moves (this fixture alone if it
                        // isn't in it), never a locked fixture
                        let ids = fxItem.selected ? plan.pickable(plan.selection) : [fxItem.modelData.id]
                        plan.movedBy(ids, dx, dy)
                    }
                }
                onWheel: (ev) => {
                    let p = mapToItem(plan, ev.x, ev.y)
                    plan.zoomAt(p.x, p.y, ev.angleDelta.y > 0 ? 1.15 : 1 / 1.15)
                }
            }
        }
    }

    // Follow: a line from each head to the point, and the point
    Canvas {
        id: aimCanvas
        anchors.fill: parent
        z: 15
        visible: plan.target !== null && plan.target !== undefined
        onPaint: {
            let ctx = getContext("2d")
            ctx.reset()
            if (!visible)
                return
            let tx = plan.stageX + plan.target.x * plan.mmScale
            let ty = plan.stageY + plan.target.y * plan.mmScale
            ctx.strokeStyle = Theme.warning
            ctx.globalAlpha = 0.7
            ctx.lineWidth = 1.5
            ctx.setLineDash([4, 3])
            for (let f of plan.fixtures) {
                if (plan.aimedIds.indexOf(f.id) < 0)
                    continue
                ctx.beginPath()
                ctx.moveTo(plan.stageX + f.x * plan.mmScale, plan.stageY + f.y * plan.mmScale)
                ctx.lineTo(tx, ty)
                ctx.stroke()
            }
        }
        Connections {
            target: plan
            function onTargetChanged() { aimCanvas.requestPaint() }
            function onAimedIdsChanged() { aimCanvas.requestPaint() }
            function onFixturesChanged() { aimCanvas.requestPaint() }
            function onStageXChanged() { aimCanvas.requestPaint() }
            function onStageYChanged() { aimCanvas.requestPaint() }
            function onMmScaleChanged() { aimCanvas.requestPaint() }
        }
        onVisibleChanged: requestPaint()
    }
    IconLabel {
        visible: aimCanvas.visible
        z: 16
        icon: Theme.iconTarget
        size: 22
        color: Theme.warning
        x: plan.stageX + (visible ? plan.target.x : 0) * plan.mmScale - width / 2
        y: plan.stageY + (visible ? plan.target.y : 0) * plan.mmScale - height / 2
    }
    // picking the point: the left button, on the stage; the right one still moves the view
    MouseArea {
        anchors.fill: parent
        z: 25
        visible: plan.aiming
        acceptedButtons: Qt.LeftButton
        cursorShape: Qt.CrossCursor
        preventStealing: true
        function pick(ev) {
            let x = Math.max(0, Math.min(plan.stage.width, (ev.x - plan.stageX) / plan.mmScale))
            let y = Math.max(0, Math.min(plan.stage.height, (ev.y - plan.stageY) / plan.mmScale))
            plan.pointPicked(x, y)
        }
        onPressed: (ev) => pick(ev)
        onPositionChanged: (ev) => { if (pressed) pick(ev) }
        onWheel: (ev) => plan.zoomAt(ev.x, ev.y, ev.angleDelta.y > 0 ? 1.15 : 1 / 1.15)
    }

    // zoom
    Row {
        anchors { left: parent.left; bottom: parent.bottom; margins: 8 }
        spacing: 4
        z: 30
        AppButton { iconGlyph: Theme.iconZoomOut; flat: true; implicitHeight: 26; implicitWidth: 30; onClicked: plan.zoomAt(plan.width / 2, plan.height / 2, 1 / 1.25) }
        AppButton { iconGlyph: Theme.iconZoomIn; flat: true; implicitHeight: 26; implicitWidth: 30; onClicked: plan.zoomAt(plan.width / 2, plan.height / 2, 1.25) }
        AppButton {
            visible: plan.zoom !== 1 || plan.pan.x !== 0 || plan.pan.y !== 0
            iconGlyph: Theme.iconCompress
            flat: true
            implicitHeight: 26
            implicitWidth: 30
            onClicked: plan.resetView()
        }
    }

    // while fixtures are locked: what the plan shows (lockText), and an ×
    // that unlocks them. Under the legend when the stage is beside it, beside
    // the legend when the stage is below it: the stage stays where it was
    Rectangle {
        id: lockChip
        visible: plan.lockText.length > 0
        z: 30
        x: legend.visible && !plan.chipUnderLegend ? plan.legendEnd : 8
        y: plan.chipUnderLegend ? plan.legendBottom - 2 : 8
        // as wide as its text, up to the room under the legend (left of the
        // stage beside it, which doesn't move) or the plan beside it
        readonly property real natural: 8 + chipGlyph.implicitWidth + Math.ceil(chipMetrics.advanceWidth(plan.lockText))
                                        + unlockButton.implicitWidth + 2 * chipRow.spacing + 5
        width: plan.chipUnderLegend ? Math.min(natural, plan.chipRoomUnderLegend)
                                    : Math.max(Math.min(120, plan.width - x - 8), Math.min(natural, plan.width - x - 8))
        height: 30
        radius: Theme.radiusSmall
        color: Qt.alpha(Theme.surface, 0.94)
        border.color: Theme.accent
        border.width: 1

        FontMetrics { id: chipMetrics; font.pixelSize: Theme.fontSmall; font.weight: Font.DemiBold }

        // (nothing under it is clicked, nor zoomed)
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
            onWheel: (ev) => ev.accepted = true
        }
        HoverHandler { id: chipHover }
        // (shortened, its whole text first)
        ToolTip {
            visible: chipHover.hovered
            delay: 500
            text: (chipText.truncated || chipText.text !== plan.lockText ? plan.lockText + "\n" : "")
                  + qsTr("The other fixtures are locked on the plan: faded, they can't be selected or moved. Press × to unlock them all")
        }

        Row {
            id: chipRow
            anchors { left: parent.left; leftMargin: 8; right: parent.right; rightMargin: 3; verticalCenter: parent.verticalCenter }
            spacing: 6
            IconLabel {
                id: chipGlyph
                anchors.verticalCenter: parent.verticalCenter
                icon: Theme.iconEye
                size: 12
                color: Theme.accent
            }
            // (too long: the name and the count alone, the name shortened in
            // the middle, the count whole; without them, the middle of it all)
            Text {
                id: chipText
                readonly property real room: chipRow.width - chipGlyph.width - unlockButton.width - 2 * chipRow.spacing
                anchors.verticalCenter: parent.verticalCenter
                width: Math.min(implicitWidth, room)
                text: {
                    if (plan.lockCount.length === 0 || chipMetrics.advanceWidth(plan.lockText) <= room)
                        return plan.lockText
                    const count = " · " + plan.lockCount
                    return chipMetrics.elidedText(plan.lockName, Qt.ElideMiddle,
                                                  Math.max(0, room - chipMetrics.advanceWidth(count) - 2)) + count
                }
                color: Theme.text
                font.pixelSize: Theme.fontSmall
                font.weight: Font.DemiBold
                elide: Text.ElideMiddle
            }
            AppButton {
                id: unlockButton
                anchors.verticalCenter: parent.verticalCenter
                implicitWidth: 24
                implicitHeight: 24
                leftPadding: 0
                rightPadding: 0
                flat: true
                iconGlyph: Theme.iconClose
                iconSize: 11
                Accessible.name: qsTr("Unlock all the fixtures")
                onClicked: plan.unlockRequested()
            }
        }
    }

    // the legend: each fixture type of the show, its glyph in its colour and
    // how many; a click selects them (Ctrl/Shift+click adds them; with
    // fixtures locked, those that aren't). Folded, its title alone
    // (remembered; with one type it starts folded)
    Rectangle {
        id: legend
        readonly property bool open: Core.settings.planLegend === "open"
                                     || (Core.settings.planLegend === "" && plan.types.length > 1)
        visible: plan.showLegend && plan.types.length > 0
        z: 30
        anchors { left: parent.left; top: parent.top; margins: 8 }
        // as wide as its longest line (up to 300 px: longer names are cut)
        readonly property real linesWidth: {
            let widest = 0
            for (const t of plan.types) {
                const generic = t.model.length === 0
                widest = Math.max(widest, nameMetrics.advanceWidth(generic ? Theme.kindName(t.kind) : t.model)
                                          + makerMetrics.advanceWidth(generic ? qsTr("Generic dimmer") : t.manufacturer))
            }
            // (the glyph, the gaps and the count)
            return widest + 84
        }
        width: Math.min(plan.width - 16, Math.max(legendTitle.implicitWidth + 16, open ? Math.min(300, linesWidth) : 0))
        height: legendColumn.height + 8
        radius: Theme.radiusSmall
        color: Qt.alpha(Theme.surface, 0.94)
        border.color: Theme.border

        FontMetrics { id: nameMetrics; font.pixelSize: Theme.fontSmall; font.weight: Font.DemiBold }
        FontMetrics { id: makerMetrics; font.pixelSize: 10 }

        // (nothing under it is clicked, nor zoomed)
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
            onWheel: (ev) => ev.accepted = true
        }

        Column {
            id: legendColumn
            x: 4
            y: 4
            width: parent.width - 8

            Rectangle {
                width: parent.width
                height: 26
                radius: Theme.radiusSmall
                color: titleMouse.containsMouse ? Theme.surfaceHover : "transparent"
                Row {
                    id: legendTitle
                    anchors { left: parent.left; leftMargin: 4; verticalCenter: parent.verticalCenter }
                    spacing: 6
                    IconLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        icon: legend.open ? Theme.iconChevronDown : Theme.iconChevronRight
                        size: 10
                        color: Theme.textDim
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Fixture types (%1)").arg(plan.types.length)
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSmall
                        font.weight: Font.Bold
                    }
                    // folded: the glyphs alone
                    Repeater {
                        model: legend.open ? [] : plan.types.slice(0, 8)
                        IconLabel {
                            required property var modelData
                            anchors.verticalCenter: parent.verticalCenter
                            icon: Theme.kindIcon(modelData.kind)
                            size: 12
                            color: Theme.fixtureTypeColor(modelData.colorIndex)
                        }
                    }
                }
                MouseArea {
                    id: titleMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: Core.settings.planLegend = legend.open ? "closed" : "open"
                }
            }

            ListView {
                id: legendList
                visible: legend.open
                width: parent.width
                // (and room under it for the chip of the locked fixtures)
                height: visible ? Math.min(contentHeight, Math.max(60, plan.height - 80 - (plan.lockText.length > 0 ? 38 : 0))) : 0
                clip: true
                interactive: contentHeight > height
                boundsBehavior: Flickable.StopAtBounds
                model: plan.types
                ScrollBar.vertical: ScrollBar { policy: legendList.interactive ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff }

                delegate: Rectangle {
                    id: typeRow
                    required property var modelData
                    readonly property bool generic: modelData.model.length === 0
                    // with fixtures locked: those of its fixtures that aren't
                    // (none: dimmed, a click does nothing)
                    readonly property var pickIds: plan.pickable(modelData.ids)
                    readonly property bool usable: pickIds.length > 0
                    readonly property bool allSelected: usable && pickIds.every(id => plan.selection.indexOf(id) >= 0)
                    readonly property color tint: Theme.fixtureTypeColor(modelData.colorIndex)
                    width: ListView.view.width - (legendList.interactive ? 8 : 0)
                    height: 26
                    radius: Theme.radiusSmall
                    opacity: usable ? 1 : 0.4
                    color: allSelected ? Qt.alpha(Theme.accent, 0.2)
                                       : (rowMouse.containsMouse && usable ? Theme.surfaceHover : "transparent")

                    Item {
                        id: glyphBox
                        anchors { left: parent.left; leftMargin: 2; verticalCenter: parent.verticalCenter }
                        width: 22
                        height: 22
                        Rectangle {
                            anchors.centerIn: parent
                            width: 20
                            height: 20
                            radius: 10
                            color: Theme.surfaceRaised
                            border.color: typeRow.tint
                            border.width: 2
                        }
                        IconLabel {
                            anchors.centerIn: parent
                            icon: Theme.kindIcon(typeRow.modelData.kind)
                            size: 11
                            color: typeRow.tint
                        }
                    }
                    // the model (a generic dimmer: what it stands in for), and the maker
                    Text {
                        id: modelText
                        anchors { left: glyphBox.right; leftMargin: 6; verticalCenter: parent.verticalCenter }
                        width: Math.min(implicitWidth, countText.x - x - 6)
                        text: typeRow.generic ? Theme.kindName(typeRow.modelData.kind) : typeRow.modelData.model
                        color: Theme.text
                        font.pixelSize: Theme.fontSmall
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }
                    Text {
                        anchors { left: modelText.right; leftMargin: 5; right: countText.left; rightMargin: 6
                                  verticalCenter: parent.verticalCenter }
                        text: typeRow.generic ? qsTr("Generic dimmer") : typeRow.modelData.manufacturer
                        color: Theme.textFaint
                        font.pixelSize: 10
                        elide: Text.ElideRight
                    }
                    Text {
                        id: countText
                        anchors { right: parent.right; rightMargin: 6; verticalCenter: parent.verticalCenter }
                        text: typeRow.modelData.count
                        color: Theme.textDim
                        font.family: Theme.monoFamily
                        font.pixelSize: Theme.fontSmall
                    }
                    MouseArea {
                        id: rowMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: typeRow.usable ? Qt.PointingHandCursor : Qt.ArrowCursor
                        ToolTip.visible: containsMouse
                        ToolTip.delay: 600
                        ToolTip.text: (typeRow.generic ? qsTr("Generic dimmer")
                                                       : typeRow.modelData.manufacturer + " " + typeRow.modelData.model)
                                      + " · " + Theme.kindName(typeRow.modelData.kind)
                                      + "\n" + (!plan.anyLocked ? qsTr("Click: select them. Ctrl+click: add them to the selection")
                                                : typeRow.usable ? qsTr("Click: select those that aren't locked. Ctrl+click: add them to the selection")
                                                : qsTr("All of them are locked on the plan"))
                        // Ctrl/Shift: the selection and them, sent whole (a
                        // single fixture added alone would toggle)
                        onClicked: (ev) => {
                            const ids = typeRow.pickIds
                            if (ids.length === 0)
                                return
                            const add = (ev.modifiers & (Qt.ControlModifier | Qt.ShiftModifier)) !== 0
                            const selection = Array.from(plan.selection || [])
                            plan.selectRequested(add ? selection.concat(ids.filter(id => selection.indexOf(id) < 0)) : ids,
                                                 false)
                        }
                    }
                }
            }
        }
    }
}
