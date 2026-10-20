/*
  DMX Total Control
  SectionView.qml

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
import QtQuick.Layouts
import DMXTotalControl
import DMXTotalControl.Backend

/**
 * One section of a SHOW page: a table of buttons or faders
 * that fill its cells, a group, or a utility like a clock or a timer.
 */
Rectangle {
    id: view

    property var section: ({})
    /** The ShowPage: edit mode, selection, timers, drag item */
    property var surface: null

    readonly property string sid: section.id || ""
    readonly property string type: section.type || "empty"
    readonly property bool editing: surface !== null && surface.editing
    readonly property bool selected: editing && surface.selectedSection === sid
    readonly property var items: section.items || ({})
    /** A row of playbacks that follows the current playback page: its header pages */
    readonly property bool followsPlaybackPage: type === "playbacks" && (section.playbackPage === undefined || section.playbackPage < 0)
    readonly property bool hasHeader: (section.title || "").length > 0 || editing || followsPlaybackPage

    radius: Theme.radius
    color: type === "empty" && !editing ? "transparent" : Theme.surface
    border.color: selected ? Theme.accent : (editing ? Theme.borderStrong : Theme.border)
    border.width: selected ? 2 : 1
    clip: true

    // ------------------------------------------------------------ header
    Rectangle {
        id: header
        visible: view.hasHeader
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 1 }
        height: visible ? 26 : 0
        radius: Theme.radius
        color: view.selected ? Qt.alpha(Theme.accent, 0.18) : "transparent"

        Text {
            anchors { left: parent.left; leftMargin: 10; verticalCenter: parent.verticalCenter; right: pager.visible ? pager.left : tools.left; rightMargin: 6 }
            text: (view.section.title || "").length ? view.section.title : view.surface ? view.surface.typeName(view.type) : ""
            color: (view.section.title || "").length ? Theme.text : Theme.textDim
            font.pixelSize: Theme.fontSmall
            font.weight: Font.Bold
            font.capitalization: Font.AllUppercase
            elide: Text.ElideRight
        }
        MouseArea {
            anchors.fill: parent
            enabled: view.editing
            onClicked: view.surface.selectSection(view.sid)
        }
        // playbacks that follow the playback page: ◀ the page ▶
        Row {
            id: pager
            visible: view.followsPlaybackPage
            anchors { right: tools.visible ? tools.left : parent.right; rightMargin: 4; verticalCenter: parent.verticalCenter }
            spacing: 4
            AppButton {
                implicitHeight: 22
                implicitWidth: 26
                flat: true
                iconGlyph: Theme.iconStepBackward
                enabled: Core.playbacks.currentPage > 0
                onClicked: Core.playbacks.previousPage()
                ToolTip.text: qsTr("Previous playback page")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                MapTarget { target: ({ target: "playbackPagePrevious" }) }
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                width: Math.min(implicitWidth, Math.max(40, header.width / 3))
                text: Core.playbacks.pageNames[Core.playbacks.currentPage] || ""
                color: Theme.text
                font.pixelSize: Theme.fontSmall
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }
            AppButton {
                implicitHeight: 22
                implicitWidth: 26
                flat: true
                iconGlyph: Theme.iconStepForward
                enabled: Core.playbacks.currentPage < Core.playbacks.pageCount - 1
                onClicked: Core.playbacks.nextPage()
                ToolTip.text: qsTr("Next playback page")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                MapTarget { target: ({ target: "playbackPageNext" }) }
            }
        }
        Row {
            id: tools
            visible: view.editing
            anchors { right: parent.right; rightMargin: 4; verticalCenter: parent.verticalCenter }
            spacing: 2
            component Tool: AppButton {
                implicitHeight: 22
                implicitWidth: 26
                flat: true
                ToolTip.visible: hovered
                ToolTip.delay: 400
            }
            Tool {
                iconGlyph: Theme.iconArrowsH
                ToolTip.text: qsTr("Split in two halves, side by side")
                onClicked: view.surface.split(view.sid, "h")
            }
            Tool {
                iconGlyph: Theme.iconArrowsV
                ToolTip.text: qsTr("Split in two halves, one above the other")
                onClicked: view.surface.split(view.sid, "v")
            }
            Tool {
                iconGlyph: Theme.iconClose
                ToolTip.text: qsTr("Remove this section")
                onClicked: view.surface.removeSection(view.sid)
            }
        }
    }

    // ------------------------------------------------------------ content
    Loader {
        id: content
        anchors { left: parent.left; right: parent.right; top: header.bottom; bottom: parent.bottom; margins: 4 }
        sourceComponent: {
            switch (view.type) {
            case "buttons": return buttonsContent
            case "faders": return fadersContent
            case "group": return groupContent
            case "groups": return groupsContent
            case "mixer": return mixerContent
            case "clock": return clockContent
            case "timer": return timerContent
            case "bpm": return bpmContent
            case "notes": return notesContent
            case "running": return runningContent
            case "monitor": return monitorContent
            case "dials": return dialsContent
            case "colour": return colourContent
            case "xy": return xyContent
            case "image": return imageContent
            case "playbacks": return playbacksContent
            default: return emptyContent
            }
        }
    }

    // in edit mode, sections without cells are selected with a click anywhere
    MouseArea {
        anchors.fill: content
        visible: view.editing && view.type !== "buttons" && view.type !== "faders" && view.type !== "dials" && view.type !== "empty"
        onClicked: view.surface.selectSection(view.sid)
    }

    // ------------------------------------------------------------ tables
    // a cell of a table: the control, or an empty place to fill in edit mode
    component Cell: Item {
        id: cell
        required property int index
        readonly property var item: view.items[String(index)]
        readonly property bool cellSelected: view.selected && view.surface.selectedCell === index

        // drop a scene from the list, or a control from another cell
        DropArea {
            id: drop
            anchors.fill: parent
            enabled: view.editing
            keys: ["dtc-show-scene", "dtc-show-cell"]
            onDropped: (ev) => {
                let d = view.surface.dragItem
                if (d.kind === "scene")
                    view.surface.dropScene(view.sid, cell.index, d.functionId)
                else if (d.sectionId === view.sid)
                    view.surface.moveCell(view.sid, d.cell, cell.index)
                ev.accept()
            }
        }
        Rectangle {
            anchors.fill: parent
            visible: view.editing && (cell.item === undefined || drop.containsDrag)
            radius: Theme.radiusSmall
            color: drop.containsDrag ? Qt.alpha(Theme.accent, 0.25) : "transparent"
            border.color: cell.cellSelected ? Theme.accent : Theme.border
            border.width: cell.cellSelected ? 2 : 1
            Text {
                anchors.centerIn: parent
                visible: cell.item === undefined
                text: "+"
                color: Theme.textFaint
                font.pixelSize: Math.min(22, parent.height / 3)
            }
        }
        Rectangle {
            anchors.fill: parent
            anchors.margins: -2
            visible: cell.cellSelected && cell.item !== undefined
            radius: Theme.radiusSmall + 2
            color: "transparent"
            border.color: Theme.accent
            border.width: 2
            z: 2
        }
        // edit: select, or drag to another cell
        MouseArea {
            id: cellMouse
            anchors.fill: parent
            z: 3
            enabled: view.editing
            drag.target: cell.item !== undefined ? view.surface.dragItem : null
            drag.threshold: 6
            preventStealing: true
            onPressed: (ev) => {
                view.surface.selectCell(view.sid, cell.index)
                if (cell.item !== undefined)
                    view.surface.beginDrag(cellMouse, ev, "cell", view.sid, cell.index, -1, qsTr("Move"))
            }
            onPositionChanged: if (drag.active) view.surface.dragItem.dragging = true
            onReleased: view.surface.dragItem.drop()
        }
    }

    Component {
        id: buttonsContent
        Grid {
            id: grid
            readonly property int cols: Math.max(1, view.section.cols || 1)
            readonly property int rowCount: Math.max(1, view.section.rows || 1)
            columns: cols
            spacing: 3
            Repeater {
                model: grid.cols * grid.rowCount
                Cell {
                    width: (grid.width - (grid.cols - 1) * grid.spacing) / grid.cols
                    height: (grid.height - (grid.rowCount - 1) * grid.spacing) / grid.rowCount
                    SurfaceButton {
                        anchors.fill: parent
                        visible: parent.item !== undefined
                        item: parent.item || ({})
                        editing: view.editing
                    }
                }
            }
        }
    }

    Component {
        id: fadersContent
        Row {
            id: row
            readonly property int cols: Math.max(1, view.section.cols || 1)
            spacing: 3
            Repeater {
                model: row.cols
                Cell {
                    width: (row.width - (row.cols - 1) * row.spacing) / row.cols
                    height: row.height
                    SurfaceFader {
                        anchors.fill: parent
                        visible: parent.item !== undefined
                        item: parent.item || ({})
                        editing: view.editing
                        page: view.surface ? view.surface.currentPage : 0
                        sectionId: view.sid
                        cellIndex: parent.index
                    }
                }
            }
        }
    }

    Component {
        id: dialsContent
        Grid {
            id: dialGrid
            readonly property int cols: Math.max(1, view.section.cols || 1)
            readonly property int rowCount: Math.max(1, view.section.rows || 1)
            columns: cols
            spacing: 3
            Repeater {
                model: dialGrid.cols * dialGrid.rowCount
                Cell {
                    width: (dialGrid.width - (dialGrid.cols - 1) * dialGrid.spacing) / dialGrid.cols
                    height: (dialGrid.height - (dialGrid.rowCount - 1) * dialGrid.spacing) / dialGrid.rowCount
                    SurfaceDial {
                        anchors.fill: parent
                        visible: parent.item !== undefined
                        item: parent.item || ({})
                        editing: view.editing
                    }
                }
            }
        }
    }

    // ------------------------------------------------------------ playbacks (playback bar)
    // the strips of the Playbacks page: of the current playback page or of a
    // fixed one, from a first playback. Editing, a scene dropped on a strip
    // puts it there
    Component {
        id: playbacksContent
        Row {
            id: pbRow
            readonly property int first: Math.max(0, view.section.first || 0)
            // (a page has 20 playbacks at most)
            readonly property int cols: Math.max(1, Math.min(view.section.cols || 1, 20 - first))
            spacing: 3
            Repeater {
                model: pbRow.cols
                PlaybackStrip {
                    required property int index
                    width: (pbRow.width - (pbRow.cols - 1) * pbRow.spacing) / pbRow.cols
                    height: pbRow.height
                    compact: height < 260
                    page: view.followsPlaybackPage ? -1 : view.section.playbackPage
                    slot: pbRow.first + index
                    editing: view.editing
                    locked: view.surface !== null && (view.surface.locked || view.surface.editLocked)
                    showBack: view.section.showBack !== false
                    showFlash: view.section.showFlash !== false
                    showCue: view.section.showCue !== false
                    dragSource: view.surface ? view.surface.dragItem : null
                    onOpenScene: (id) => view.surface.openScene(id)
                }
            }
        }
    }

    // ------------------------------------------------------------ colour wheel
    Component {
        id: colourContent
        Item {
            id: colourBox
            property int tick: 0
            readonly property int pageIndex: view.surface ? view.surface.currentPage : 0
            readonly property color current: { tick; Core.surface.revision; return Core.surface.colour(pageIndex, view.sid) }
            readonly property var swatches: ["#ffffff", "#ff0000", "#ff7f00", "#ffff00", "#00ff00", "#00ffff", "#0000ff", "#ff00ff"]
            function pick(c) { Core.surface.setColour(pageIndex, view.sid, c) }
            Connections {
                target: Core.programmer
                function onValuesChanged() { colourBox.tick++ }
            }

            Canvas {
                id: wheel
                readonly property real side: Math.min(parent.width, parent.height - 48)
                width: side
                height: side
                anchors.horizontalCenter: parent.horizontalCenter
                onWidthChanged: requestPaint()
                onPaint: {
                    let ctx = getContext("2d")
                    ctx.reset()
                    let r = width / 2 - 2, c = width / 2
                    if (r <= 4)
                        return
                    // hue all around (0° to the right, counter clockwise), white in the middle
                    for (let i = 0; i < 360; i += 2) {
                        ctx.fillStyle = Qt.hsva(i / 360, 1, 1, 1)
                        ctx.beginPath()
                        ctx.moveTo(c, c)
                        ctx.arc(c, c, r, -(i + 2.5) * Math.PI / 180, -i * Math.PI / 180, false)
                        ctx.closePath()
                        ctx.fill()
                    }
                    let g = ctx.createRadialGradient(c, c, 0, c, c, r)
                    g.addColorStop(0, "#ffffffff")
                    g.addColorStop(1, "#00ffffff")
                    ctx.fillStyle = g
                    ctx.beginPath(); ctx.arc(c, c, r, 0, Math.PI * 2); ctx.fill()
                }
                // where the colour is now
                Rectangle {
                    readonly property real r: wheel.width / 2 - 2
                    readonly property real hue: Math.max(0, colourBox.current.hsvHue)
                    readonly property real sat: colourBox.current.hsvSaturation
                    visible: colourBox.current.valid
                    width: 16; height: 16; radius: 8
                    color: colourBox.current
                    border.color: "white"
                    border.width: 2
                    x: wheel.width / 2 + Math.cos(hue * Math.PI * 2) * sat * r - width / 2
                    y: wheel.height / 2 - Math.sin(hue * Math.PI * 2) * sat * r - height / 2
                }
                MouseArea {
                    anchors.fill: parent
                    enabled: !view.editing
                    function apply(x, y) {
                        let c = wheel.width / 2
                        let dx = x - c, dy = c - y
                        let hue = Math.atan2(dy, dx) / (Math.PI * 2)
                        if (hue < 0) hue += 1
                        let sat = Math.min(1, Math.hypot(dx, dy) / (c - 2))
                        colourBox.pick(Qt.hsva(hue, sat, 1, 1))
                    }
                    onPressed: (ev) => apply(ev.x, ev.y)
                    onPositionChanged: (ev) => { if (pressed) apply(ev.x, ev.y) }
                }
            }

            Row {
                anchors { bottom: parent.bottom; bottomMargin: 6; horizontalCenter: parent.horizontalCenter }
                spacing: 4
                Repeater {
                    model: colourBox.swatches
                    Rectangle {
                        required property string modelData
                        width: 28; height: 28; radius: 14
                        color: modelData
                        opacity: Theme.lightLevel
                        border.color: Theme.border
                        MouseArea { anchors.fill: parent; enabled: !view.editing; onClicked: colourBox.pick(parent.modelData) }
                    }
                }
                AppButton {
                    text: qsTr("Release")
                    implicitHeight: 28
                    enabled: !view.editing
                    ToolTip.text: qsTr("Give the colour back to the scenes"); ToolTip.visible: hovered
                    onClicked: Core.surface.releaseSection(colourBox.pageIndex, view.sid)
                }
            }
        }
    }

    // ------------------------------------------------------------ XY grid (pan and tilt)
    Component {
        id: xyContent
        Item {
            id: xyBox
            property int tick: 0
            readonly property int pageIndex: view.surface ? view.surface.currentPage : 0
            readonly property var pos: { tick; Core.surface.revision; return Core.surface.xy(pageIndex, view.sid) }
            Connections {
                target: Core.programmer
                function onValuesChanged() { xyBox.tick++ }
            }

            Rectangle {
                id: pad
                readonly property real side: Math.min(parent.width, parent.height - 48)
                width: side
                height: side
                anchors.horizontalCenter: parent.horizontalCenter
                radius: Theme.radiusSmall
                color: Theme.background
                border.color: Theme.border
                Repeater {
                    model: 3
                    Rectangle { required property int index; x: pad.width * (index + 1) / 4; width: 1; height: pad.height; color: Theme.border }
                }
                Repeater {
                    model: 3
                    Rectangle { required property int index; y: pad.height * (index + 1) / 4; height: 1; width: pad.width; color: Theme.border }
                }
                Rectangle {
                    width: 18; height: 18; radius: 9
                    color: Theme.functionColor("EFX")
                    border.color: Theme.textOn(color)
                    border.width: 2
                    opacity: xyBox.pos.available ? 1 : 0.4
                    x: (xyBox.pos.x !== undefined ? xyBox.pos.x : 0.5) * pad.width - width / 2
                    y: (1 - (xyBox.pos.y !== undefined ? xyBox.pos.y : 0.5)) * pad.height - height / 2
                }
                MouseArea {
                    anchors.fill: parent
                    enabled: !view.editing
                    function apply(x, y) {
                        Core.surface.setXY(xyBox.pageIndex, view.sid, Math.max(0, Math.min(1, x / pad.width)),
                                           Math.max(0, Math.min(1, 1 - y / pad.height)))
                    }
                    onPressed: (ev) => apply(ev.x, ev.y)
                    onPositionChanged: (ev) => { if (pressed) apply(ev.x, ev.y) }
                }
            }
            Row {
                anchors { bottom: parent.bottom; bottomMargin: 6; horizontalCenter: parent.horizontalCenter }
                spacing: 6
                AppButton {
                    text: qsTr("Centre")
                    implicitHeight: 28
                    enabled: !view.editing
                    onClicked: Core.surface.setXY(xyBox.pageIndex, view.sid, 0.5, 0.5)
                }
                AppButton {
                    text: qsTr("Release")
                    implicitHeight: 28
                    enabled: !view.editing
                    ToolTip.text: qsTr("Give the position back to the scenes"); ToolTip.visible: hovered
                    onClicked: Core.surface.releaseSection(xyBox.pageIndex, view.sid)
                }
            }
        }
    }

    // ------------------------------------------------------------ image
    Component {
        id: imageContent
        Item {
            Image {
                anchors.fill: parent
                source: view.section.image ? (view.section.image.indexOf("file:") === 0 ? view.section.image
                                                                                       : "file:///" + view.section.image) : ""
                fillMode: view.section.fit === "fill" ? Image.PreserveAspectCrop : Image.PreserveAspectFit
                asynchronous: true
                clip: true
                smooth: true
            }
            Text {
                anchors.centerIn: parent
                visible: !view.section.image
                text: qsTr("Choose an image in the properties")
                color: Theme.textFaint
            }
        }
    }

    // ------------------------------------------------------------ groups
    Component {
        id: groupContent
        Item {
            readonly property Zone zone: { Core.zones.count; return Core.zones.zoneById(view.section.zoneId) }
            ZoneColumn {
                anchors.fill: parent
                visible: parent.zone !== null
                zoneObject: parent.zone
                operatorMode: true
                border.width: 0
                color: "transparent"
            }
            Text {
                anchors.centerIn: parent
                width: parent.width - 20
                visible: parent.zone === null
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: qsTr("Choose a group in the properties")
                color: Theme.textFaint
            }
        }
    }

    Component {
        id: mixerContent
        MixerRow {
            fit: true
            interactive: !view.editing
        }
    }

    Component {
        id: groupsContent
        Row {
            id: groupsRow
            spacing: 6
            Repeater {
                model: Core.zones
                delegate: ZoneColumn {
                    required property var zone
                    required property int index
                    zoneObject: zone
                    zoneIndex: index
                    operatorMode: true
                    width: (groupsRow.width - (Core.zones.count - 1) * groupsRow.spacing) / Math.max(1, Core.zones.count)
                    height: groupsRow.height
                }
            }
            Text {
                visible: Core.zones.count === 0
                text: qsTr("No groups yet. Create them on the Live page.")
                color: Theme.textFaint
            }
        }
    }

    // ------------------------------------------------------------ utilities
    Component {
        id: clockContent
        Item {
            id: clockItem
            property date now: new Date()
            Timer { interval: 250; running: view.visible; repeat: true; triggeredOnStart: true; onTriggered: parent.now = new Date() }
            Column {
                anchors.centerIn: parent
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: Qt.formatTime(clockItem.now, view.section.seconds === false ? "HH:mm" : "HH:mm:ss")
                    color: Theme.text
                    font.family: Theme.monoFamily
                    font.pixelSize: Math.max(14, Math.min(view.height / 2.6, view.width / (view.section.seconds === false ? 3.4 : 5.2)))
                }
                // the long date gets smaller in a narrow section (it was cut
                // on both sides next to the properties of the edit mode)
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: clockItem.width - 12
                    visible: view.section.date !== false
                    text: clockItem.now.toLocaleDateString(Qt.locale(), Locale.LongFormat)
                    horizontalAlignment: Text.AlignHCenter
                    fontSizeMode: Text.HorizontalFit
                    minimumPixelSize: 8
                    elide: Text.ElideRight
                    color: Theme.textDim
                    font.pixelSize: Math.max(10, Math.min(18, view.height / 10))
                }
            }
        }
    }

    Component {
        id: timerContent
        Item {
            id: timerItem
            readonly property bool countdown: view.section.mode === "countdown"
            readonly property int total: (view.section.minutes || 5) * 60000
            property real elapsed: 0
            readonly property bool running: { view.surface.timerRevision; return view.surface.timerRunning(view.sid) }
            readonly property real shown: countdown ? Math.max(0, total - elapsed) : elapsed
            Timer {
                interval: 100; running: view.visible; repeat: true; triggeredOnStart: true
                onTriggered: timerItem.elapsed = view.surface.timerElapsed(view.sid)
            }
            function format(ms) {
                let s = Math.floor(ms / 1000)
                let h = Math.floor(s / 3600), m = Math.floor((s % 3600) / 60), sec = s % 60
                let pad = (n) => (n < 10 ? "0" : "") + n
                return (h > 0 ? h + ":" + pad(m) : pad(m)) + ":" + pad(sec) + (countdown ? "" : "." + Math.floor((ms % 1000) / 100))
            }
            ColumnLayout {
                anchors.fill: parent
                spacing: 4
                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: timerItem.format(timerItem.shown)
                    color: timerItem.countdown && timerItem.shown <= 0 ? Theme.danger
                         : timerItem.countdown && timerItem.shown < 60000 ? Theme.warning : Theme.text
                    font.family: Theme.monoFamily
                    font.pixelSize: Math.max(14, Math.min(view.height / 3, view.width / 6))
                }
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    AppButton {
                        text: timerItem.running ? qsTr("Pause") : qsTr("Start")
                        iconGlyph: timerItem.running ? Theme.iconPause : Theme.iconPlay
                        highlighted: timerItem.running
                        accentColor: Theme.success
                        enabled: !view.editing
                        onClicked: view.surface.timerToggle(view.sid)
                    }
                    AppButton {
                        text: qsTr("Reset")
                        iconGlyph: Theme.iconRefresh
                        enabled: !view.editing
                        onClicked: view.surface.timerReset(view.sid)
                    }
                }
            }
        }
    }

    // the rhythm: GO (the scene buttons start their scenes and mark their
    // rhythm) and Stop (they stop them), as in the top bar (each button
    // shows the rhythm of its scene with steps), and BeatGO
    Component {
        id: bpmContent
        ColumnLayout {
            spacing: 6
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 6
                AppButton {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredWidth: 1
                    text: "GO"
                    highlighted: Core.pressMode === "go"
                    accentColor: Theme.success
                    enabled: !view.editing
                    onClicked: Core.pressMode = Core.pressMode === "go" ? "toggle" : "go"
                }
                AppButton {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredWidth: 1
                    text: qsTr("Stop")
                    highlighted: Core.pressMode === "stop"
                    accentColor: Theme.danger
                    enabled: !view.editing
                    onClicked: Core.pressMode = Core.pressMode === "stop" ? "toggle" : "stop"
                }
            }
            AppButton {
                Layout.fillWidth: true
                text: "BeatGO"
                implicitHeight: 30
                accentColor: Theme.warning
                enabled: !view.editing
                onPressed: Core.drive.beatGo()
            }
        }
    }

    Component {
        id: notesContent
        Flickable {
            clip: true
            contentHeight: notesText.implicitHeight
            boundsBehavior: Flickable.StopAtBounds
            Text {
                id: notesText
                width: parent.width
                text: (view.section.text || "").length ? view.section.text
                      : view.editing ? qsTr("Write the notes in the properties") : ""
                color: (view.section.text || "").length ? Theme.text : Theme.textFaint
                font.pixelSize: view.section.fontSize || 20
                wrapMode: Text.Wrap
                padding: 6
            }
        }
    }

    Component {
        id: runningContent
        ListView {
            id: runningList
            clip: true
            spacing: 3
            property var running: []
            model: running
            Timer {
                interval: 300; running: view.visible; repeat: true; triggeredOnStart: true
                onTriggered: runningList.running = Core.functions.filter(f => Core.effects.isRunning(f.id))
            }
            delegate: Rectangle {
                required property var modelData
                width: runningList.width
                height: 32
                radius: Theme.radiusSmall
                color: Theme.surfaceRaised
                Rectangle { width: 4; height: parent.height - 8; anchors.verticalCenter: parent.verticalCenter; x: 4; radius: 2; color: Theme.functionColor(parent.modelData.type) }
                Text {
                    anchors { left: parent.left; leftMargin: 14; right: stopButton.left; verticalCenter: parent.verticalCenter }
                    text: parent.modelData.name
                    color: Theme.text
                    elide: Text.ElideRight
                }
                AppButton {
                    id: stopButton
                    anchors { right: parent.right; rightMargin: 4; verticalCenter: parent.verticalCenter }
                    implicitHeight: 24
                    iconGlyph: Theme.iconStop
                    flat: true
                    enabled: !view.editing
                    onClicked: view.surface.stopScene(parent.modelData.id)
                }
            }
            Text {
                anchors.centerIn: parent
                visible: runningList.count === 0
                text: qsTr("Nothing is playing")
                color: Theme.textFaint
            }
        }
    }

    Component {
        id: monitorContent
        Item {
            id: monitor
            property var values: []
            readonly property int universe: view.section.universe || 0
            Timer {
                interval: 120; running: view.visible; repeat: true; triggeredOnStart: true
                onTriggered: monitor.values = Core.io.channelValues(monitor.universe)
            }
            Text {
                id: monitorTitle
                text: qsTr("Universe %1").arg(monitor.universe + 1)
                color: Theme.textDim
                font.pixelSize: 10
            }
            Grid {
                id: levels
                anchors { left: parent.left; right: parent.right; top: monitorTitle.bottom; bottom: parent.bottom; topMargin: 2 }
                columns: 32
                spacing: 1
                readonly property real cw: (width - 31) / 32
                readonly property real ch: (height - 15) / 16
                Repeater {
                    model: 512
                    Rectangle {
                        required property int index
                        width: levels.cw
                        height: levels.ch
                        color: Qt.alpha(Theme.accent, 0.06 + 0.9 * (monitor.values[index] || 0) / 255)
                    }
                }
            }
        }
    }

    Component {
        id: emptyContent
        Item {
            // edit mode: what goes in this section
            Flow {
                anchors.centerIn: parent
                width: Math.min(parent.width - 10, 460)
                visible: view.editing
                spacing: 6
                Repeater {
                    model: view.surface ? view.surface.sectionTypes : []
                    AppButton {
                        required property var modelData
                        text: modelData.name
                        iconGlyph: modelData.icon
                        implicitHeight: 32
                        onClicked: {
                            view.surface.selectSection(view.sid)
                            Core.surface.updateSection(view.surface.currentPage, view.sid, { type: modelData.type })
                        }
                    }
                }
            }
        }
    }
}
