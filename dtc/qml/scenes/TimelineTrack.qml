/*
  DMX Total Control
  TimelineTrack.qml

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
 * A track of a super scene: its header (name, mute, lock, hide, lane of
 * automation), its blocks, and the lane of dimmer (volume) or phase with
 * its points: click to add one, drag it, double click to remove it, right
 * click for a smooth curve.
 */
Item {
    id: trackItem

    property var editor
    property var track: ({})
    property int index: 0

    readonly property int trackId: track.id !== undefined ? track.id : -1
    readonly property bool hidden: !!track.hidden
    readonly property bool locked: !!track.locked
    readonly property bool selected: editor.selectedTrack === trackId
    // which lane is open: "", "dimmer" or "phase" (kept by the editor)
    readonly property string lane: editor.lanes[trackId] !== undefined ? editor.lanes[trackId]
                                 : (track.dimmer || []).length ? "dimmer" : (track.phase || []).length ? "phase" : ""
    readonly property int rowHeight: hidden ? 24 : 58
    readonly property int laneHeight: lane !== "" && !hidden ? 60 : 0

    width: parent ? parent.width : 0
    height: rowHeight + laneHeight

    component HeaderButton: AppButton {
        implicitWidth: 24
        implicitHeight: 22
        flat: !highlighted
        font.pixelSize: 10
    }

    // ------------------------------------------------------------ header
    Rectangle {
        id: header
        width: editor.headerWidth
        height: parent.height
        color: trackItem.selected ? Qt.alpha(Theme.accent, 0.15) : Theme.surfaceRaised
        border.color: Theme.border
        MouseArea { anchors.fill: parent; onClicked: editor.selectedTrack = trackItem.trackId }

        Column {
            anchors { fill: parent; margins: 4 }
            spacing: 2
            Text {
                width: parent.width
                text: trackItem.track.name || ""
                color: trackItem.track.mute ? Theme.textFaint : Theme.text
                font.pixelSize: Theme.fontSmall
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                MouseArea {
                    anchors.fill: parent
                    onClicked: editor.selectedTrack = trackItem.trackId
                    onDoubleClicked: editor.renameTrack(trackItem.trackId, trackItem.track.name)
                }
            }
            Row {
                visible: !trackItem.hidden
                spacing: 1
                HeaderButton {
                    text: "M"
                    highlighted: !!trackItem.track.mute
                    accentColor: Theme.warning
                    ToolTip.text: qsTr("Mute: the track doesn't play"); ToolTip.visible: hovered; ToolTip.delay: 500
                    onClicked: Core.timeline.setTrack(editor.showId, trackItem.trackId, "mute", !trackItem.track.mute)
                }
                HeaderButton {
                    iconGlyph: trackItem.locked ? Theme.iconLock : Theme.iconUnlock
                    highlighted: trackItem.locked
                    ToolTip.text: qsTr("Lock: its blocks don't move"); ToolTip.visible: hovered; ToolTip.delay: 500
                    onClicked: Core.timeline.setTrack(editor.showId, trackItem.trackId, "locked", !trackItem.locked)
                }
                HeaderButton {
                    text: "D"
                    highlighted: trackItem.lane === "dimmer"
                    accentColor: Theme.category("#f5c524")
                    ToolTip.text: qsTr("Automation of the dimmer (the volume of the audio)"); ToolTip.visible: hovered; ToolTip.delay: 500
                    onClicked: editor.setLane(trackItem.trackId, trackItem.lane === "dimmer" ? "" : "dimmer")
                }
                HeaderButton {
                    text: "φ"
                    highlighted: trackItem.lane === "phase"
                    accentColor: Theme.category("#ae3ec9")
                    ToolTip.text: qsTr("Automation of the phase of its effects"); ToolTip.visible: hovered; ToolTip.delay: 500
                    onClicked: editor.setLane(trackItem.trackId, trackItem.lane === "phase" ? "" : "phase")
                }
                HeaderButton {
                    text: "⋯"
                    onClicked: trackMenu.popup()
                }
            }
        }
        // hide / show: fold the track
        AppButton {
            anchors { right: parent.right; top: parent.top; margins: 2 }
            implicitWidth: 22
            implicitHeight: 20
            flat: true
            iconGlyph: trackItem.hidden ? Theme.iconChevronRight : Theme.iconChevronDown
            ToolTip.text: trackItem.hidden ? qsTr("Show the track") : qsTr("Hide the track")
            ToolTip.visible: hovered
            ToolTip.delay: 500
            onClicked: Core.timeline.setTrack(editor.showId, trackItem.trackId, "hidden", !trackItem.hidden)
        }

        AppMenu {
            id: trackMenu
            MenuItem { text: qsTr("Rename"); onTriggered: editor.renameTrack(trackItem.trackId, trackItem.track.name) }
            MenuItem { text: qsTr("Move up"); onTriggered: Core.timeline.moveTrack(editor.showId, trackItem.trackId, -1) }
            MenuItem { text: qsTr("Move down"); onTriggered: Core.timeline.moveTrack(editor.showId, trackItem.trackId, 1) }
            MenuItem {
                text: qsTr("Clear the automation of this lane")
                enabled: trackItem.lane !== "" && (trackItem.track[trackItem.lane] || []).length > 0
                onTriggered: Core.timeline.setAutomation(editor.showId, trackItem.trackId, trackItem.lane, [])
            }
            MenuSeparator {}
            MenuItem { text: qsTr("Delete the track"); onTriggered: Core.timeline.removeTrack(editor.showId, trackItem.trackId) }
        }
    }

    // ------------------------------------------------------------ blocks
    Item {
        id: content
        x: editor.headerWidth
        width: parent.width - editor.headerWidth
        height: trackItem.rowHeight
        clip: true

        Rectangle {
            anchors.fill: parent
            color: trackItem.track.mute ? Theme.shadow : "transparent"
            border.color: Theme.border
            border.width: 1
        }
        // a click on the empty track: the cursor goes there
        MouseArea {
            anchors.fill: parent
            onClicked: (ev) => {
                editor.selectedTrack = trackItem.trackId
                editor.selectBlock(-1, trackItem.trackId)
                editor.cursor = editor.snap(editor.msAt(ev.x))
            }
        }

        Repeater {
            model: trackItem.hidden ? [] : (trackItem.track.blocks || [])
            delegate: TimelineBlock {
                required property var modelData
                editor: trackItem.editor
                block: modelData
                trackId: trackItem.trackId
                trackLocked: trackItem.locked
            }
        }

        // scenes from the list
        DropArea {
            anchors.fill: parent
            keys: ["dtc-timeline-scene"]
            enabled: !trackItem.locked
            onDropped: (drop) => {
                let ms = editor.snap(editor.msAt(drop.x))
                let id = Core.timeline.addBlock(editor.showId, trackItem.trackId, drop.source.functionId, ms)
                editor.selectBlock(id, trackItem.trackId)
                drop.accept()
            }
            Rectangle {
                anchors.fill: parent
                visible: parent.containsDrag
                color: Qt.alpha(Theme.accent, 0.12)
                border.color: Theme.accent
            }
        }
    }

    // ------------------------------------------------------------ automation
    Item {
        id: laneArea
        visible: trackItem.laneHeight > 0
        x: editor.headerWidth
        y: trackItem.rowHeight
        width: parent.width - editor.headerWidth
        height: trackItem.laneHeight
        clip: true

        readonly property color tint: Theme.category(trackItem.lane === "phase" ? "#ae3ec9" : "#f5c524")
        // the points, edited here before they are saved
        property var points: trackItem.track[trackItem.lane] || []
        property int dragging: -1

        function xOf(ms) { return ms * editor.pxPerMs - editor.scrollX }
        function yOf(v) { return 4 + (1 - v) * (height - 8) }
        function valueAt(y) { return Math.max(0, Math.min(1, 1 - (y - 4) / (height - 8))) }
        function pointAt(mx, my) {
            for (let i = 0; i < points.length; i++)
                if (Math.abs(xOf(points[i].time) - mx) < 7 && Math.abs(yOf(points[i].value) - my) < 7)
                    return i
            return -1
        }
        function save() { Core.timeline.setAutomation(editor.showId, trackItem.trackId, trackItem.lane, points) }

        onPointsChanged: curve.requestPaint()
        Connections {
            target: editor
            function onScrollXChanged() { curve.requestPaint() }
            function onPxPerMsChanged() { curve.requestPaint() }
        }

        Rectangle {
            anchors.fill: parent
            color: Qt.alpha(Theme.shadow, Theme.shadow.a * 0.7)
            border.color: Theme.border
        }
        Text {
            anchors { left: parent.left; top: parent.top; margins: 3 }
            text: trackItem.lane === "phase" ? qsTr("Phase") : qsTr("Dimmer / volume")
            color: Theme.readable(laneArea.tint)
            font.pixelSize: 9
            opacity: 0.8
        }

        Canvas {
            id: curve
            // the colours of the theme
            readonly property string themeName: Theme.name
            onThemeNameChanged: requestPaint()
            anchors.fill: parent
            onPaint: {
                let ctx = getContext("2d")
                ctx.reset()
                let pts = laneArea.points
                ctx.strokeStyle = laneArea.tint
                ctx.fillStyle = laneArea.tint
                ctx.lineWidth = 2
                if (pts.length === 0) {
                    // no automation: full dimmer, phase at 0
                    let y = laneArea.yOf(trackItem.lane === "phase" ? 0 : 1)
                    ctx.globalAlpha = 0.35
                    ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(width, y); ctx.stroke()
                    return
                }
                ctx.beginPath()
                ctx.moveTo(0, laneArea.yOf(pts[0].value))
                for (let i = 0; i < pts.length; i++) {
                    let x = laneArea.xOf(pts[i].time), y = laneArea.yOf(pts[i].value)
                    if (i > 0 && pts[i - 1].smooth) {
                        // the smooth curve of the previous point, in steps
                        let x0 = laneArea.xOf(pts[i - 1].time), v0 = pts[i - 1].value
                        for (let s = 1; s <= 16; s++) {
                            let f = s / 16
                            let e = (1 - Math.cos(f * Math.PI)) / 2
                            ctx.lineTo(x0 + (x - x0) * f, laneArea.yOf(v0 + (pts[i].value - v0) * e))
                        }
                    } else {
                        ctx.lineTo(x, y)
                    }
                }
                ctx.lineTo(width, laneArea.yOf(pts[pts.length - 1].value))
                ctx.stroke()
                for (let p of pts) {
                    ctx.beginPath()
                    ctx.arc(laneArea.xOf(p.time), laneArea.yOf(p.value), p.smooth ? 5 : 4, 0, 2 * Math.PI)
                    if (p.smooth) ctx.stroke(); else ctx.fill()
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            enabled: !trackItem.locked
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            preventStealing: true
            onPressed: (ev) => {
                let i = laneArea.pointAt(ev.x, ev.y)
                if (ev.button === Qt.RightButton) {
                    // a smooth curve to the next point, or straight
                    if (i >= 0) {
                        let pts = laneArea.points.slice()
                        pts[i] = Object.assign({}, pts[i], { smooth: !pts[i].smooth })
                        laneArea.points = pts
                        laneArea.save()
                    }
                    return
                }
                if (i < 0) {
                    // the new point is the one dragged (and saved on release):
                    // snapped to the grid, it may be away from the click
                    let pts = laneArea.points.slice()
                    const added = { time: editor.snap(editor.msAt(ev.x)), value: laneArea.valueAt(ev.y), smooth: false }
                    pts.push(added)
                    pts.sort((a, b) => a.time - b.time)
                    laneArea.points = pts
                    i = pts.indexOf(added)
                }
                laneArea.dragging = i
            }
            onPositionChanged: (ev) => {
                if (laneArea.dragging < 0)
                    return
                let pts = laneArea.points.slice()
                pts[laneArea.dragging] = Object.assign({}, pts[laneArea.dragging],
                    { time: editor.snap(editor.msAt(ev.x)), value: laneArea.valueAt(ev.y) })
                laneArea.points = pts
            }
            onReleased: {
                if (laneArea.dragging >= 0) {
                    laneArea.dragging = -1
                    laneArea.points = laneArea.points.slice().sort((a, b) => a.time - b.time)
                    laneArea.save()
                }
            }
            onDoubleClicked: (ev) => {
                let i = laneArea.pointAt(ev.x, ev.y)
                if (i >= 0) {
                    let pts = laneArea.points.slice()
                    pts.splice(i, 1)
                    laneArea.points = pts
                    laneArea.save()
                }
            }
        }

        // the value under the mouse
        Text {
            visible: laneArea.dragging >= 0
            anchors { right: parent.right; top: parent.top; margins: 3 }
            text: laneArea.dragging >= 0 && laneArea.dragging < laneArea.points.length
                  ? (trackItem.lane === "phase" ? Math.round(laneArea.points[laneArea.dragging].value * 360) + "°"
                                                : Math.round(laneArea.points[laneArea.dragging].value * 100) + "%") : ""
            color: Theme.text
            font.family: Theme.monoFamily
            font.pixelSize: 10
        }
    }
}
