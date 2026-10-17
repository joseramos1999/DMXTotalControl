/*
  DMX Total Control
  TimelineBlock.qml

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
 * A block of a super scene: drag it (to another track too), drag its edges
 * to resize it, drag the handles of its top corners for the fades. A scene
 * that loops shows where each cycle ends; audio shows its waveform.
 */
Item {
    id: blockItem

    property var editor
    property var block: ({})
    property int trackId: -1
    property bool trackLocked: false

    readonly property bool selected: editor.selectedBlock === block.id
    readonly property bool locked: !!block.locked || trackLocked
    readonly property bool audio: block.type === "Audio"
    readonly property real px: editor.pxPerMs

    // a drag in progress: "move", "left", "right", "fadeIn", "fadeOut"
    property string mode: ""
    property real pressX: 0
    property real pressY: 0
    property real deltaMs: 0
    readonly property bool dragging: mode !== "" && (Math.abs(deltaMs) > 0 || mode === "move" && Math.abs(dragY) > 4)
    property real dragY: 0

    // where it is drawn, the drag included
    readonly property real startMs: mode === "move" || mode === "left" ? Math.max(0, editor.snap(block.start + deltaMs)) : block.start
    readonly property real endMs: mode === "right" ? Math.max(block.start + 100, editor.snap(block.start + block.duration + deltaMs))
                                 : mode === "left" ? block.start + block.duration
                                 : startMs + block.duration
    readonly property real fadeInMs: mode === "fadeIn" ? Math.max(0, Math.min(block.duration, block.fadeIn + deltaMs)) : block.fadeIn
    readonly property real fadeOutMs: mode === "fadeOut" ? Math.max(0, Math.min(block.duration, block.fadeOut - deltaMs)) : block.fadeOut

    x: startMs * px - editor.scrollX
    y: 4 + (mode === "move" ? dragY : 0)
    z: dragging ? 10 : (selected ? 2 : 1)
    width: Math.max(4, (endMs - startMs) * px)
    height: parent ? parent.height - 8 : 50

    readonly property color tint: block.color && block.color.length ? Theme.category(block.color) : Theme.functionColor(block.type)

    Rectangle {
        anchors.fill: parent
        radius: 4
        color: Qt.rgba(blockItem.tint.r, blockItem.tint.g, blockItem.tint.b, blockItem.selected ? 0.6 : 0.38)
        border.color: blockItem.selected ? Qt.lighter(blockItem.tint, 1.5) : blockItem.tint
        border.width: blockItem.selected ? 2 : 1
        opacity: blockItem.dragging ? 0.8 : 1
        clip: true

        // the waveform of the audio
        Canvas {
            id: wave
            // the colours of the theme
            readonly property string themeName: Theme.name
            onThemeNameChanged: requestPaint()
            anchors { fill: parent; topMargin: 16; bottomMargin: 2 }
            visible: blockItem.audio
            property int revision: 0
            readonly property var peaks: { revision; return blockItem.audio ? Core.timeline.waveform(blockItem.block.functionId) : [] }
            onPeaksChanged: requestPaint()
            onWidthChanged: requestPaint()
            Connections {
                target: Core.timeline
                function onWaveformReady(id) { if (id === blockItem.block.functionId) wave.revision++ }
            }
            onPaint: {
                let ctx = getContext("2d")
                ctx.reset()
                let p = peaks
                if (!p || p.length === 0)
                    return
                let msPer = Core.timeline.waveformStep()
                let mid = height / 2
                ctx.strokeStyle = Qt.lighter(blockItem.tint, 1.4)
                ctx.lineWidth = 1
                ctx.beginPath()
                for (let x = 0; x < width; x++) {
                    let i = Math.floor(x / blockItem.px / msPer)
                    if (i >= p.length)
                        break
                    let h = p[i] * mid
                    ctx.moveTo(x + 0.5, mid - h)
                    ctx.lineTo(x + 0.5, mid + h)
                }
                ctx.stroke()
            }
        }

        // a scene that loops: where each cycle ends
        Repeater {
            model: !blockItem.audio && !blockItem.block.stretch && blockItem.block.cycle > 0
                   && blockItem.block.cycle * blockItem.px > 8
                   ? Math.min(200, Math.floor((blockItem.endMs - blockItem.startMs - 1) / blockItem.block.cycle)) : 0
            Rectangle {
                required property int index
                x: (index + 1) * blockItem.block.cycle * blockItem.px
                width: 1
                height: parent.height
                color: Qt.alpha(blockItem.tint, 0.6)
            }
        }

        // the fades
        Canvas {
            id: fades
            // the colours of the theme
            readonly property string themeName: Theme.name
            onThemeNameChanged: requestPaint()
            anchors.fill: parent
            property real fin: blockItem.fadeInMs * blockItem.px
            property real fout: blockItem.fadeOutMs * blockItem.px
            onFinChanged: requestPaint()
            onFoutChanged: requestPaint()
            onWidthChanged: requestPaint()
            onPaint: {
                let ctx = getContext("2d")
                ctx.reset()
                ctx.fillStyle = Theme.shadow
                ctx.strokeStyle = Qt.lighter(blockItem.tint, 1.3)
                ctx.lineWidth = 1.5
                if (fin > 0) {
                    ctx.beginPath(); ctx.moveTo(0, 0); ctx.lineTo(fin, 0); ctx.lineTo(0, height); ctx.closePath(); ctx.fill()
                    ctx.beginPath(); ctx.moveTo(0, height); ctx.lineTo(fin, 0); ctx.stroke()
                }
                if (fout > 0) {
                    ctx.beginPath(); ctx.moveTo(width, 0); ctx.lineTo(width - fout, 0); ctx.lineTo(width, height); ctx.closePath(); ctx.fill()
                    ctx.beginPath(); ctx.moveTo(width - fout, 0); ctx.lineTo(width, height); ctx.stroke()
                }
            }
        }

        Row {
            anchors { left: parent.left; top: parent.top; leftMargin: 6 + Math.min(fades.fin, 40); topMargin: 2 }
            spacing: 4
            IconLabel { visible: blockItem.locked; icon: Theme.iconLock; size: 9; color: Theme.textDim }
            Text {
                visible: !!blockItem.block.stretch
                text: "↔"
                color: Theme.text
                font.pixelSize: 11
            }
            Text {
                text: blockItem.block.name || ""
                color: Theme.text
                font.pixelSize: 11
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                width: Math.max(0, blockItem.width - 30)
            }
        }
    }

    // the handles of the fades
    Rectangle {
        visible: blockItem.selected && !blockItem.locked
        x: Math.min(blockItem.width - 10, fades.fin) - 5
        y: -5
        width: 10; height: 10; radius: 5
        color: Theme.knob
        border.color: blockItem.tint
    }
    Rectangle {
        visible: blockItem.selected && !blockItem.locked
        x: Math.max(0, blockItem.width - fades.fout) - 5
        y: -5
        width: 10; height: 10; radius: 5
        color: Theme.knob
        border.color: blockItem.tint
    }

    MouseArea {
        anchors { fill: parent; topMargin: -6 }
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        preventStealing: true

        function zoneAt(mx, my) {
            if (blockItem.locked)
                return "move"
            let y = my - 6
            if (blockItem.selected && y < 8 && Math.abs(mx - fades.fin) < 8)
                return "fadeIn"
            if (blockItem.selected && y < 8 && Math.abs(mx - (blockItem.width - fades.fout)) < 8)
                return "fadeOut"
            if (mx < 6 && blockItem.width > 16)
                return "left"
            if (mx > blockItem.width - 6 && blockItem.width > 16)
                return "right"
            return "move"
        }
        cursorShape: {
            let z = zoneAt(mouseX, mouseY)
            return z === "left" || z === "right" ? Qt.SizeHorCursor
                 : z === "fadeIn" || z === "fadeOut" ? Qt.PointingHandCursor
                 : blockItem.locked ? Qt.ArrowCursor : Qt.SizeAllCursor
        }

        onPressed: (ev) => {
            editor.selectBlock(blockItem.block.id, blockItem.trackId)
            if (ev.button === Qt.RightButton) {
                blockMenu.popup()
                return
            }
            blockItem.mode = blockItem.locked ? "" : zoneAt(ev.x, ev.y)
            let p = mapToItem(editor, ev.x, ev.y)
            blockItem.pressX = p.x
            blockItem.pressY = p.y
            blockItem.deltaMs = 0
            blockItem.dragY = 0
        }
        onPositionChanged: (ev) => {
            if (blockItem.mode === "")
                return
            let p = mapToItem(editor, ev.x, ev.y)
            blockItem.deltaMs = (p.x - blockItem.pressX) / blockItem.px
            if (blockItem.mode === "move")
                blockItem.dragY = p.y - blockItem.pressY
        }
        onReleased: (ev) => {
            let mode = blockItem.mode
            let wasDragging = blockItem.dragging
            blockItem.mode = ""
            if (mode === "" || !wasDragging && mode !== "move")
                return
            let id = blockItem.block.id
            let showId = editor.showId
            if (mode === "move") {
                // to the track under the mouse
                let p = mapToItem(editor, ev.x, ev.y)
                let target = editor.trackAt(p.y)
                if (wasDragging || (target >= 0 && target !== blockItem.trackId)) {
                    let start = Math.max(0, editor.snap(blockItem.block.start + blockItem.deltaMs))
                    Core.timeline.moveBlock(showId, id, target >= 0 ? target : blockItem.trackId, start)
                }
            } else if (mode === "left") {
                let end = blockItem.block.start + blockItem.block.duration
                let start = Math.min(end - 100, Math.max(0, editor.snap(blockItem.block.start + blockItem.deltaMs)))
                Core.timeline.resizeBlock(showId, id, start, end - start)
            } else if (mode === "right") {
                let end = Math.max(blockItem.block.start + 100, editor.snap(blockItem.block.start + blockItem.block.duration + blockItem.deltaMs))
                Core.timeline.resizeBlock(showId, id, blockItem.block.start, end - blockItem.block.start)
            } else if (mode === "fadeIn") {
                Core.timeline.setBlock(showId, id, "fadeIn", Math.round(blockItem.fadeInMs))
            } else if (mode === "fadeOut") {
                Core.timeline.setBlock(showId, id, "fadeOut", Math.round(blockItem.fadeOutMs))
            }
        }
        onCanceled: blockItem.mode = ""
        onDoubleClicked: if (!blockItem.audio) editor.openFunction(blockItem.block.functionId)
    }

    AppMenu {
        id: blockMenu
        MenuItem {
            text: blockItem.block.stretch ? qsTr("Loop the scene in the block") : qsTr("Stretch the scene to fit the block")
            enabled: !blockItem.audio && blockItem.block.cycle > 0
            onTriggered: Core.timeline.setBlock(editor.showId, blockItem.block.id, "stretch", !blockItem.block.stretch)
        }
        MenuItem {
            text: qsTr("Duplicate")
            onTriggered: editor.selectBlock(Core.timeline.duplicateBlock(editor.showId, blockItem.block.id), blockItem.trackId)
        }
        MenuItem {
            text: blockItem.block.locked ? qsTr("Unlock") : qsTr("Lock")
            onTriggered: Core.timeline.setBlock(editor.showId, blockItem.block.id, "locked", !blockItem.block.locked)
        }
        MenuItem {
            text: qsTr("Open the scene")
            enabled: !blockItem.audio
            onTriggered: editor.openFunction(blockItem.block.functionId)
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Delete")
            enabled: !blockItem.trackLocked
            onTriggered: Core.timeline.removeBlock(editor.showId, blockItem.block.id)
        }
    }
}
