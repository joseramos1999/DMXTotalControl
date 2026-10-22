/*
  DMX Total Control
  SuperSceneEditor.qml

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
import QtQuick.Dialogs
import DMXTotalControl
import DMXTotalControl.Backend

/**
 * Super scene: tracks with blocks of scenes and of audio along
 * a timeline. Drag scenes from the list onto a track; the grid is in time
 * or in bars, blocks snap to it. Ctrl+wheel zooms, the wheel with Shift (or
 * the bar below) moves along the time.
 */
Item {
    id: editor

    property var info: ({})
    readonly property int showId: info.id !== undefined ? info.id : -1
    readonly property var timeline: { Core.timeline.revision; return showId >= 0 ? Core.timeline.show(showId) : ({}) }
    readonly property var tracks: timeline.tracks || []

    signal openFunction(int functionId)

    // the view
    property real pxPerMs: 0.05
    property real scrollX: 0
    readonly property int headerWidth: 170
    readonly property real viewWidth: Math.max(1, timeArea.width - headerWidth)
    readonly property real contentWidth: Math.max(((timeline.length || 0) + 30000) * pxPerMs, viewWidth)

    // where it plays from, what is selected
    property int cursor: 0
    property int selectedBlock: -1
    property int selectedTrack: -1
    property bool snapOn: true
    // the open automation lane of each track (by id)
    property var lanes: ({})

    readonly property bool playing: Core.timeline.playingId === showId && showId >= 0
    readonly property int playhead: playing ? Core.timeline.position : cursor
    readonly property bool bars: timeline.grid === "bars"
    readonly property real beatMs: 60000 / Math.max(20, timeline.bpm || 120)
    readonly property int beatsPerBar: timeline.beatsPerBar || 4

    function xOf(ms) { return ms * pxPerMs - scrollX }
    function msAt(x) { return Math.max(0, (x + scrollX) / pxPerMs) }
    // the step of the grid: a beat, or a time that leaves some pixels
    function gridStep() {
        if (bars)
            return beatMs
        for (let s of [50, 100, 250, 500, 1000, 2000, 5000, 10000, 30000, 60000])
            if (s * pxPerMs >= 14)
                return s
        return 60000
    }
    function snap(ms) {
        if (!snapOn)
            return Math.max(0, Math.round(ms))
        let step = gridStep()
        let offset = bars ? (timeline.gridOffset || 0) : 0
        return Math.max(0, Math.round(Math.round((ms - offset) / step) * step + offset))
    }
    function zoomAt(x, factor) {
        let ms = msAt(x)
        pxPerMs = Math.max(0.002, Math.min(1.0, pxPerMs * factor))
        scrollX = Math.max(0, Math.min(contentWidth - viewWidth, ms * pxPerMs - x))
    }
    function scrollBy(px) { scrollX = Math.max(0, Math.min(Math.max(0, contentWidth - viewWidth), scrollX + px)) }
    function formatTime(ms) {
        let s = ms / 1000
        let m = Math.floor(s / 60)
        let rest = s - m * 60
        return m + ":" + (rest < 10 ? "0" : "") + rest.toFixed(1)
    }
    function formatBar(ms) {
        let beats = (ms - (timeline.gridOffset || 0)) / beatMs
        if (beats < 0)
            return "0.0"
        return (Math.floor(beats / beatsPerBar) + 1) + "." + (Math.floor(beats % beatsPerBar) + 1)
    }
    function selectBlock(id, trackId) {
        selectedBlock = id
        if (trackId !== undefined && trackId >= 0)
            selectedTrack = trackId
    }
    function setLane(trackId, lane) {
        let l = Object.assign({}, lanes)
        l[trackId] = lane
        lanes = l
    }
    function renameTrack(trackId, name) {
        prompt.ask(qsTr("Rename track"), name, (text) => Core.timeline.setTrack(showId, trackId, "name", text))
    }
    // the track under a point of the editor
    function trackAt(y) {
        let p = mapToItem(trackColumn, 0, y)
        let row = trackColumn.childAt(10, p.y)
        return row && row.trackId !== undefined ? row.trackId : -1
    }
    readonly property var selected: {
        for (let t of tracks)
            for (let b of t.blocks)
                if (b.id === selectedBlock)
                    return b
        return null
    }
    readonly property int targetTrack: selectedTrack >= 0 ? selectedTrack : (tracks.length ? tracks[0].id : -1)

    // the playhead stays in view while it plays
    onPlayheadChanged: {
        if (!playing)
            return
        let x = xOf(playhead)
        if (x > viewWidth - 30 || x < 0)
            scrollX = Math.max(0, playhead * pxPerMs - viewWidth * 0.2)
    }
    onShowIdChanged: { cursor = 0; scrollX = 0; selectedBlock = -1; selectedTrack = -1 }

    focus: true
    Keys.onPressed: (ev) => {
        if (ev.key === Qt.Key_Delete && selectedBlock >= 0) {
            Core.timeline.removeBlock(showId, selectedBlock)
            selectedBlock = -1
            ev.accepted = true
        } else if (ev.key === Qt.Key_Space) {
            if (playing) Core.timeline.stop(); else Core.timeline.play(showId, cursor)
            ev.accepted = true
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        // ------------------------------------------------------------ toolbar
        Flow {
            Layout.fillWidth: true
            spacing: 6

            AppButton {
                text: editor.playing ? qsTr("Stop") : qsTr("Play")
                iconGlyph: editor.playing ? Theme.iconStop : Theme.iconPlay
                primary: !editor.playing
                accentColor: editor.playing ? Theme.danger : Theme.accent
                implicitHeight: 30
                ToolTip.text: qsTr("From the cursor (Space)")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: editor.playing ? Core.timeline.stop() : Core.timeline.play(editor.showId, editor.cursor)
            }
            AppButton {
                text: "⏮"
                implicitHeight: 30
                ToolTip.text: qsTr("To the start")
                ToolTip.visible: hovered
                onClicked: { editor.cursor = 0; editor.scrollX = 0; if (editor.playing) Core.timeline.play(editor.showId, 0) }
            }
            Rectangle {
                width: timeText.implicitWidth + 20
                height: 30
                radius: Theme.radiusSmall
                color: Theme.background
                border.color: Theme.border
                Text {
                    id: timeText
                    anchors.centerIn: parent
                    text: editor.formatTime(editor.playhead) + (editor.bars ? "   " + qsTr("bar %1").arg(editor.formatBar(editor.playhead)) : "")
                    color: editor.playing ? Theme.success : Theme.text
                    font.family: Theme.monoFamily
                }
            }
            Rectangle { width: 1; height: 30; color: Theme.border }

            // the grid: time, or bars of a BPM
            ComboBox {
                implicitHeight: 30
                implicitWidth: 120
                model: [qsTr("Time"), qsTr("Bars")]
                currentIndex: editor.bars ? 1 : 0
                onActivated: (i) => Core.timeline.setSettings(editor.showId, { grid: i === 1 ? "bars" : "time" })
            }
            SpinBox {
                visible: editor.bars
                implicitHeight: 30
                implicitWidth: 140
                from: 20; to: 300; editable: true
                value: editor.timeline.bpm || 120
                textFromValue: (v) => v + " BPM"
                valueFromText: (t) => parseInt(t)
                onValueModified: Core.timeline.setSettings(editor.showId, { bpm: value })
            }
            ComboBox {
                visible: editor.bars
                implicitHeight: 30
                implicitWidth: 80
                model: ["4/4", "3/4", "2/4"]
                currentIndex: Math.max(0, [4, 3, 2].indexOf(editor.beatsPerBar))
                onActivated: (i) => Core.timeline.setSettings(editor.showId, { beatsPerBar: [4, 3, 2][i] })
            }
            AppButton {
                visible: editor.bars
                text: qsTr("Bar 1 here")
                implicitHeight: 30
                ToolTip.text: qsTr("The first bar starts at the cursor")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: Core.timeline.setSettings(editor.showId, { gridOffset: editor.cursor })
            }
            AppButton {
                text: qsTr("Snap")
                iconGlyph: Theme.iconGrid
                implicitHeight: 30
                highlighted: editor.snapOn
                onClicked: editor.snapOn = !editor.snapOn
            }
            Rectangle { width: 1; height: 30; color: Theme.border }
            AppButton {
                text: qsTr("End here")
                implicitHeight: 30
                ToolTip.text: qsTr("The super scene ends at the cursor")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: Core.timeline.setSettings(editor.showId, { endTime: Math.max(100, editor.cursor) })
            }
            AppButton {
                visible: (editor.timeline.endTime || 0) > 0
                text: "×"
                implicitHeight: 30
                ToolTip.text: qsTr("No end marker: it ends with its last block")
                ToolTip.visible: hovered
                onClicked: Core.timeline.setSettings(editor.showId, { endTime: 0 })
            }
            AppButton {
                text: qsTr("Loop")
                iconGlyph: Theme.iconRefresh
                implicitHeight: 30
                highlighted: !!editor.timeline.loop
                onClicked: Core.timeline.setSettings(editor.showId, { loop: !editor.timeline.loop })
            }
            Rectangle { width: 1; height: 30; color: Theme.border }
            AppButton {
                text: qsTr("Track")
                iconGlyph: Theme.iconPlus
                implicitHeight: 30
                onClicked: editor.selectedTrack = Core.timeline.addTrack(editor.showId, "")
            }
            AppButton {
                text: qsTr("Audio")
                iconGlyph: Theme.iconPlus
                implicitHeight: 30
                ToolTip.text: qsTr("An audio file on the selected track, at the cursor")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: audioDialog.start()
            }
            AppButton {
                text: (editor.timeline.staticValues || 0) > 0 ? qsTr("Static values (%1)").arg(editor.timeline.staticValues) : qsTr("Static values")
                iconGlyph: Theme.iconSliders
                implicitHeight: 30
                highlighted: (editor.timeline.staticValues || 0) > 0
                onClicked: staticMenu.popup()
                AppMenu {
                    id: staticMenu
                    MenuItem { text: qsTr("Take the values of the programmer"); onTriggered: Core.timeline.captureStatic(editor.showId) }
                    MenuItem {
                        text: qsTr("Clear the static values")
                        enabled: (editor.timeline.staticValues || 0) > 0
                        onTriggered: Core.timeline.clearStatic(editor.showId)
                    }
                }
            }
            Rectangle { width: 1; height: 30; color: Theme.border }
            AppButton { iconGlyph: Theme.iconZoomOut; implicitHeight: 30; flat: true; onClicked: editor.zoomAt(editor.xOf(editor.playhead), 1 / 1.5) }
            AppButton { iconGlyph: Theme.iconZoomIn; implicitHeight: 30; flat: true; onClicked: editor.zoomAt(editor.xOf(editor.playhead), 1.5) }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            // ------------------------------------------------------------ scenes to drag
            Panel {
                Layout.preferredWidth: 230
                Layout.fillHeight: true
                title: qsTr("Scenes")

                SceneBrowser {
                    id: sceneList
                    anchors { fill: parent; margins: 6 }
                    compact: true
                    // (not a rack that plays this super scene: it would play itself)
                    functions: {
                        const players = Core.effects.playersOf(editor.showId)
                        return Core.functions.filter(f => f.type !== "Show" && f.id !== editor.showId && players.indexOf(f.id) < 0)
                    }
                    dragTarget: dragProxy
                    onDragPressed: (area, ev, fn) => {
                        let p = area.mapToItem(editor, ev.x, ev.y)
                        dragProxy.functionId = fn.id
                        dragProxy.label = fn.name
                        dragProxy.x = p.x - 12
                        dragProxy.y = p.y - 14
                    }
                    onDragMoved: dragProxy.Drag.active = true
                    onDragReleased: { dragProxy.Drag.drop(); dragProxy.Drag.active = false }
                    // double click: at the cursor, on the selected track
                    onSceneDoubleClicked: (id) => editor.selectBlock(Core.timeline.addBlock(editor.showId, editor.targetTrack,
                                                                                            id, editor.cursor), editor.targetTrack)
                }
            }

            // ------------------------------------------------------------ timeline
            Rectangle {
                id: timeArea
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Theme.background
                border.color: Theme.border
                radius: Theme.radius
                clip: true

                // the ruler: click or drag to move the cursor
                Item {
                    id: ruler
                    x: editor.headerWidth
                    width: parent.width - editor.headerWidth
                    height: 30

                    Canvas {
                        id: rulerCanvas
                        // the colours of the theme
                        readonly property string themeName: Theme.name
                        onThemeNameChanged: requestPaint()
                        anchors.fill: parent
                        Connections {
                            target: editor
                            function onScrollXChanged() { rulerCanvas.requestPaint() }
                            function onPxPerMsChanged() { rulerCanvas.requestPaint() }
                            function onTimelineChanged() { rulerCanvas.requestPaint() }
                        }
                        onWidthChanged: requestPaint()
                        onPaint: {
                            let ctx = getContext("2d")
                            ctx.reset()
                            ctx.fillStyle = Theme.surface
                            ctx.fillRect(0, 0, width, height)
                            ctx.font = "10px sans-serif"
                            ctx.textBaseline = "top"
                            let step = editor.gridStep()
                            let offset = editor.bars ? (editor.timeline.gridOffset || 0) : 0
                            // a label every so many steps
                            let every = editor.bars ? editor.beatsPerBar : 1
                            while (step * every * editor.pxPerMs < 60)
                                every *= 2
                            let first = Math.floor((editor.msAt(0) - offset) / step)
                            let last = Math.ceil((editor.msAt(width) - offset) / step)
                            for (let i = Math.max(first, editor.bars ? 0 : 0); i <= last; i++) {
                                let ms = offset + i * step
                                if (ms < 0)
                                    continue
                                let x = editor.xOf(ms)
                                let major = i % every === 0
                                ctx.strokeStyle = major ? Theme.mix(Theme.textFaint, Theme.textDim, 0.85) : Theme.borderStrong
                                ctx.beginPath(); ctx.moveTo(x + 0.5, major ? 12 : 20); ctx.lineTo(x + 0.5, height); ctx.stroke()
                                if (major) {
                                    ctx.fillStyle = Theme.mix(Theme.textDim, Theme.text, 0.6)
                                    ctx.fillText(editor.bars ? String(Math.floor(i / editor.beatsPerBar) + 1) : editor.formatTime(ms), x + 3, 2)
                                }
                            }
                            // the end marker and the loop
                            let end = editor.timeline.endTime || 0
                            if (end > 0) {
                                let x = editor.xOf(end)
                                ctx.fillStyle = editor.timeline.loop ? Theme.success : Theme.danger
                                ctx.fillRect(x - 1, 0, 3, height)
                            }
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onPressed: (ev) => editor.cursor = editor.snap(editor.msAt(ev.x))
                        onPositionChanged: (ev) => { if (pressed) editor.cursor = editor.snap(editor.msAt(ev.x)) }
                        onDoubleClicked: if (!editor.playing) Core.timeline.play(editor.showId, editor.cursor)
                        onWheel: (ev) => editor.zoomAt(ev.x, ev.angleDelta.y > 0 ? 1.2 : 1 / 1.2)
                    }
                }

                // the grid behind the tracks
                Canvas {
                    id: gridCanvas
                    // the colours of the theme
                    readonly property string themeName: Theme.name
                    onThemeNameChanged: requestPaint()
                    x: editor.headerWidth
                    y: ruler.height
                    width: parent.width - editor.headerWidth
                    height: parent.height - ruler.height - hbar.height
                    Connections {
                        target: editor
                        function onScrollXChanged() { gridCanvas.requestPaint() }
                        function onPxPerMsChanged() { gridCanvas.requestPaint() }
                        function onTimelineChanged() { gridCanvas.requestPaint() }
                    }
                    onWidthChanged: requestPaint()
                    onHeightChanged: requestPaint()
                    onPaint: {
                        let ctx = getContext("2d")
                        ctx.reset()
                        let step = editor.gridStep()
                        let offset = editor.bars ? (editor.timeline.gridOffset || 0) : 0
                        let first = Math.floor((editor.msAt(0) - offset) / step)
                        let last = Math.ceil((editor.msAt(width) - offset) / step)
                        for (let i = first; i <= last; i++) {
                            let ms = offset + i * step
                            if (ms < 0)
                                continue
                            let x = editor.xOf(ms)
                            let bar = editor.bars && i % editor.beatsPerBar === 0
                            ctx.strokeStyle = bar ? Theme.mix(Theme.border, Theme.borderStrong, 0.6) : Theme.mix(Theme.background, Theme.border, 0.62)
                            ctx.beginPath(); ctx.moveTo(x + 0.5, 0); ctx.lineTo(x + 0.5, height); ctx.stroke()
                        }
                        // after the end
                        let end = editor.timeline.endTime || 0
                        if (end > 0) {
                            ctx.fillStyle = Theme.shadow
                            let x = Math.max(0, editor.xOf(end))
                            ctx.fillRect(x, 0, width - x, height)
                        }
                    }
                }

                // the tracks
                Flickable {
                    id: tracksFlick
                    y: ruler.height
                    width: parent.width
                    height: parent.height - ruler.height - hbar.height
                    contentHeight: trackColumn.height + 60
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    interactive: false
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                    Column {
                        id: trackColumn
                        // for the delegates, where "editor" is their own property
                        readonly property var owner: editor
                        width: tracksFlick.width
                        Repeater {
                            model: editor.tracks
                            delegate: TimelineTrack {
                                required property var modelData
                                required property int index
                                editor: trackColumn.owner
                                track: modelData
                            }
                        }
                    }

                    // Ctrl: zoom; Shift: along the time; else up and down
                    WheelHandler {
                        acceptedModifiers: Qt.ControlModifier
                        onWheel: (ev) => editor.zoomAt(point.position.x - editor.headerWidth, ev.angleDelta.y > 0 ? 1.2 : 1 / 1.2)
                    }
                    WheelHandler {
                        acceptedModifiers: Qt.ShiftModifier
                        onWheel: (ev) => editor.scrollBy(-(ev.angleDelta.y || ev.angleDelta.x) / 2)
                    }
                    WheelHandler {
                        acceptedModifiers: Qt.NoModifier
                        onWheel: (ev) => {
                            if (ev.angleDelta.x !== 0)
                                editor.scrollBy(-ev.angleDelta.x / 2)
                            else
                                tracksFlick.contentY = Math.max(0, Math.min(tracksFlick.contentHeight - tracksFlick.height,
                                                                            tracksFlick.contentY - ev.angleDelta.y / 2))
                        }
                    }
                }

                // the cursor, or where it plays
                Rectangle {
                    x: editor.headerWidth + editor.xOf(editor.playhead)
                    visible: x >= editor.headerWidth && x <= parent.width
                    width: 2
                    height: parent.height - hbar.height
                    color: editor.playing ? Theme.success : Theme.warning
                    z: 20
                }

                // along the time
                ScrollBar {
                    id: hbar
                    anchors { left: parent.left; right: parent.right; bottom: parent.bottom; leftMargin: editor.headerWidth }
                    orientation: Qt.Horizontal
                    policy: ScrollBar.AlwaysOn
                    size: Math.min(1, editor.viewWidth / editor.contentWidth)
                    position: editor.scrollX / editor.contentWidth
                    onPositionChanged: if (active) editor.scrollX = position * editor.contentWidth
                }

                Text {
                    anchors.centerIn: parent
                    visible: editor.tracks.length > 0 && editor.tracks.every(t => t.blocks.length === 0)
                    text: qsTr("Drag scenes from the list onto a track")
                    color: Theme.textFaint
                }
            }
        }

        // ------------------------------------------------------------ the selected block
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            visible: editor.selected !== null
            radius: Theme.radius
            color: Theme.surface
            border.color: Theme.border

            RowLayout {
                anchors { fill: parent; leftMargin: 10; rightMargin: 10 }
                spacing: 8
                Rectangle { width: 6; height: 24; radius: 3; color: editor.selected ? Theme.functionColor(editor.selected.type) : "transparent" }
                Text {
                    Layout.preferredWidth: 150
                    text: editor.selected ? editor.selected.name : ""
                    color: Theme.text
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }
                Text { text: qsTr("Start"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                TimeSpin {
                    implicitWidth: 115
                    engineValue: editor.selected ? editor.selected.start : 0
                    to: 360000
                    onEdited: (v) => Core.timeline.moveBlock(editor.showId, editor.selectedBlock, -1, v)
                }
                Text { text: qsTr("Length"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                TimeSpin {
                    implicitWidth: 115
                    engineValue: editor.selected ? editor.selected.duration : 0
                    to: 360000
                    onEdited: (v) => Core.timeline.resizeBlock(editor.showId, editor.selectedBlock, editor.selected.start, v)
                }
                Text { text: qsTr("Fade in"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                TimeSpin {
                    implicitWidth: 105
                    engineValue: editor.selected ? editor.selected.fadeIn : 0
                    onEdited: (v) => Core.timeline.setBlock(editor.showId, editor.selectedBlock, "fadeIn", v)
                }
                Text { text: qsTr("Fade out"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                TimeSpin {
                    implicitWidth: 105
                    engineValue: editor.selected ? editor.selected.fadeOut : 0
                    onEdited: (v) => Core.timeline.setBlock(editor.showId, editor.selectedBlock, "fadeOut", v)
                }
                AppButton {
                    visible: editor.selected !== null && editor.selected.type !== "Audio" && editor.selected.cycle > 0
                    text: editor.selected && editor.selected.stretch ? qsTr("Stretched") : qsTr("Loops")
                    implicitHeight: 28
                    highlighted: editor.selected !== null && !!editor.selected.stretch
                    ToolTip.text: qsTr("Loops: the scene plays again and again in the block. Stretched: one cycle lasts the whole block")
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                    onClicked: Core.timeline.setBlock(editor.showId, editor.selectedBlock, "stretch", !editor.selected.stretch)
                }
                Item { Layout.fillWidth: true }
                AppButton {
                    iconGlyph: Theme.iconCopy
                    flat: true
                    implicitHeight: 28
                    ToolTip.text: qsTr("Duplicate")
                    ToolTip.visible: hovered
                    onClicked: editor.selectBlock(Core.timeline.duplicateBlock(editor.showId, editor.selectedBlock))
                }
                AppButton {
                    iconGlyph: Theme.iconTrash
                    flat: true
                    implicitHeight: 28
                    accentColor: Theme.danger
                    ToolTip.text: qsTr("Delete (Del)")
                    ToolTip.visible: hovered
                    onClicked: { Core.timeline.removeBlock(editor.showId, editor.selectedBlock); editor.selectedBlock = -1 }
                }
            }
        }
    }

    // a scene being dragged from the list
    Rectangle {
        id: dragProxy
        property int functionId: -1
        property string label: ""
        visible: Drag.active
        width: 150
        height: 28
        radius: 4
        color: Qt.alpha(Theme.accentFill, 0.9)
        z: 100
        Drag.keys: ["dtc-timeline-scene"]
        Drag.hotSpot: Qt.point(12, 14)
        Text {
            anchors { fill: parent; leftMargin: 8; rightMargin: 6 }
            verticalAlignment: Text.AlignVCenter
            text: dragProxy.label
            color: Theme.accentText
            elide: Text.ElideRight
            font.pixelSize: Theme.fontSmall
        }
    }

    AppFileDialog {
        id: audioDialog
        kind: "audio"
        title: qsTr("Add audio")
        nameFilters: [qsTr("Audio files (*.wav *.mp3 *.flac *.ogg *.aif *.aiff)"), qsTr("All files (*)")]
        onAccepted: editor.selectBlock(Core.timeline.addAudio(editor.showId, editor.targetTrack, selectedFile, editor.cursor), editor.targetTrack)
    }

    TextPrompt { id: prompt }
}
