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
import DMXTotalControl
import DMXTotalControl.Backend

/**
 * Top-down 2D stage plan. Fixtures are placed in millimeters; the plan
 * scales to fit, the wheel zooms and a right (or middle) drag moves it.
 * Click to select, Ctrl/Shift+click to add, drag on the empty stage for a
 * rectangle (or a lasso) selection, and drag fixtures to move them when
 * editable. Fixtures with several heads (bars, matrices) show their cells.
 * Live, each fixture shows its colour, dimmer, strobe and gobo.
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

    /** ids, additive */
    signal selectRequested(var ids, bool additive)
    /** move fixtures $ids by dx, dy (mm) */
    signal movedBy(var ids, real dx, real dy)
    /** Follow: the point picked on the stage (mm) */
    signal pointPicked(real x, real y)

    readonly property size stage: Core.patch.stageSize
    readonly property real fitScale: Math.min((width - 40) / Math.max(1, stage.width),
                                              (height - 40) / Math.max(1, stage.height))
    readonly property real mmScale: fitScale * zoom
    readonly property real stageX: (width - stage.width * mmScale) / 2 + pan.x
    readonly property real stageY: (height - stage.height * mmScale) / 2 + pan.y
    // a small plan: smaller fixtures (as big as 50 cm of the stage, from 40 % on)
    readonly property real bodyScale: Math.max(0.4, Math.min(1, mmScale * 500 / 40))

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
    function zoomAt(px, py, factor) {
        let mmX = (px - stageX) / mmScale, mmY = (py - stageY) / mmScale
        zoom = Math.max(0.5, Math.min(10, zoom * factor))
        let s = fitScale * zoom
        pan = Qt.point(px - mmX * s - (width - stage.width * s) / 2,
                       py - mmY * s - (height - stage.height * s) / 2)
    }

    function typeIcon(type) {
        switch (type) {
        case "Moving Head": return Theme.iconTarget
        case "Scanner": return Theme.iconLocate
        case "Color Changer": case "LED Bar (Pixels)": case "LED Bar (Beams)": return Theme.iconSun
        case "Strobe": return Theme.iconBolt
        case "Smoke": case "Hazer": return Theme.iconCloud
        case "Laser": return Theme.iconWand
        case "Dimmer": return Theme.iconLight
        default: return Theme.iconCircle
        }
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
                // a click on the empty stage with the move tool: nothing selected
                if (ev.button === Qt.LeftButton && Math.abs(ev.x - start.x) < 4 && Math.abs(ev.y - start.y) < 4)
                    plan.selectRequested([], false)
                return
            }
            let additive = (ev.modifiers & (Qt.ControlModifier | Qt.ShiftModifier)) !== 0
            if (banding) {
                let ids = []
                for (let f of plan.fixtures) {
                    let fx = plan.stageX + f.x * plan.mmScale
                    let fy = plan.stageY + f.y * plan.mmScale
                    let hit = plan.lasso ? plan.inside(fx, fy, path)
                                         : fx >= band.x && fx <= band.x + band.width && fy >= band.y && fy <= band.y + band.height
                    if (hit)
                        ids.push(f.id)
                }
                plan.selectRequested(ids, additive)
            } else if (!additive) {
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
        model: plan.fixtures

        delegate: Item {
            id: fxItem
            required property var modelData
            readonly property int selectionIndex: plan.selection.indexOf(modelData.id)
            readonly property bool selected: selectionIndex >= 0
            readonly property var liveState: plan.states[modelData.id]
            readonly property bool lit: plan.live && liveState !== undefined
            readonly property int heads: modelData.heads || 1
            readonly property bool bar: heads > 1

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
            readonly property bool following: plan.draggingId >= 0 && plan.draggingId !== modelData.id && selected
            x: plan.stageX + modelData.x * plan.mmScale - width / 2 + (following ? plan.dragDX : 0)
            y: plan.stageY + modelData.y * plan.mmScale - height / 2 + (following ? plan.dragDY : 0)
            z: dragArea.drag.active ? 10 : (selected ? 2 : 1)

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
                    border.color: fxItem.selected ? Qt.lighter(Theme.accent, 1.4) : Theme.borderStrong
                    border.width: fxItem.selected ? 3 : 2
                }
                // several heads: a row of cells, pixel 1 on the left
                Rectangle {
                    visible: fxItem.bar
                    anchors.fill: parent
                    radius: 3
                    color: Theme.surfaceRaised
                    border.color: fxItem.selected ? Qt.lighter(Theme.accent, 1.4) : Theme.borderStrong
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

            IconLabel {
                anchors.centerIn: parent
                visible: !fxItem.bar && !fxItem.selected && !fxItem.lit
                icon: plan.typeIcon(fxItem.modelData.type)
                size: Math.max(8, Math.round(15 * plan.bodyScale))
                color: Theme.textDim
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
            // on the edge of its body
            Item {
                visible: !!fxItem.modelData.aimable && !fxItem.bar
                anchors.centerIn: parent
                width: fxItem.bodyWidth
                height: fxItem.bodyHeight
                rotation: fxItem.modelData.rotation || 0
                IconLabel {
                    anchors { bottom: parent.bottom; bottomMargin: 1; horizontalCenter: parent.horizontalCenter }
                    icon: Theme.iconChevronDown
                    size: 10
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
                        // the whole selection moves (this fixture alone if it isn't in it)
                        let ids = fxItem.selected ? plan.selection : [fxItem.modelData.id]
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
}
