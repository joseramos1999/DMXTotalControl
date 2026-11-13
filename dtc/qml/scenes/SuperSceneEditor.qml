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
 * the bar below) moves along the time. Ctrl+Z undoes, Ctrl+Y redoes.
 * Several blocks can be selected (Ctrl+click, Shift+click, a rectangle on
 * the empty tracks, Ctrl+A): they move, duplicate (Ctrl+D) and go (Del)
 * together. Ctrl+C and Ctrl+X copy and cut them, Ctrl+V pastes them at the
 * cursor, in this super scene or another one of the show.
 */
Item {
    id: editor

    property var info: ({})
    readonly property int showId: info.id !== undefined ? info.id : -1
    readonly property var timeline: { Core.timeline.revision; return showId >= 0 ? Core.timeline.show(showId) : ({}) }
    readonly property var tracks: timeline.tracks || []
    // what undo and redo would do: { canUndo, canRedo, undoText, redoText }
    readonly property var history: { Core.timeline.historyRevision; return showId >= 0 ? Core.timeline.history(showId) : ({}) }

    signal openFunction(int functionId)

    // the view
    property real pxPerMs: 0.05
    property real scrollX: 0
    readonly property int headerWidth: 170
    readonly property real viewWidth: Math.max(1, timeArea.width - headerWidth)
    readonly property real contentWidth: Math.max(((timeline.length || 0) + 30000) * pxPerMs, viewWidth)

    // where it plays from, what is selected: the blocks (by id), the last one
    // clicked (Shift+click selects from it), the track
    property int cursor: 0
    property var selectedBlocks: []
    property int anchorBlock: -1
    readonly property int selectedBlock: selectedBlocks.length === 1 ? selectedBlocks[0] : -1
    property int selectedTrack: -1
    // the selection dragged by one of its blocks: that block, how far they go
    // in time and in tracks, the tracks they land on (lit)
    property int dragBlock: -1
    property var dragIds: []
    property real dragDeltaMs: 0
    property real dragY: 0
    property int dragRows: 0
    property var dragTargets: []
    property real groupMinStart: 0
    property int groupRowMin: 0
    property int groupRowMax: 0
    readonly property bool groupMoving: dragBlock >= 0 && (dragDeltaMs !== 0 || Math.abs(dragY) > 4)
    // the rectangle drawn on the empty tracks (in timeArea)
    property bool banding: false
    property rect band: Qt.rect(0, 0, 0, 0)
    property point bandFrom: Qt.point(0, 0)
    property bool snapOn: true
    // the open automation lane of each track (by id)
    property var lanes: ({})
    // a block, an edge, a fade or a point being dragged: undo waits
    property bool dragging: false
    // typing in a field: there Ctrl+Z undoes the typing
    readonly property bool typing: Window.activeFocusItem !== null && Window.activeFocusItem.cursorPosition !== undefined
    readonly property bool keysOn: visible && !typing && !dragging

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
    /*****************************************************************
     * The selection
     *****************************************************************/
    function rowOf(trackId) { return tracks.findIndex(t => t.id === trackId) }
    // { block, trackId, row } of every block by its id, built once per change
    readonly property var blockIndex: {
        let index = {}
        for (let row = 0; row < tracks.length; row++)
            for (let b of tracks[row].blocks)
                index[b.id] = { block: b, trackId: tracks[row].id, row: row }
        return index
    }
    // { block, trackId, row } of a block, null when it is gone
    function blockInfo(id) { return blockIndex[id] || null }
    function isSelected(id) { return selectedBlocks.indexOf(id) >= 0 }
    // a click: only it (-1: none), and its track
    function selectOnly(id, trackId) {
        selectedBlocks = id >= 0 ? [id] : []
        anchorBlock = id
        if (trackId !== undefined && trackId >= 0)
            selectedTrack = trackId
    }
    function selectBlock(id, trackId) { selectOnly(id, trackId) }
    // Ctrl+click: in or out
    function toggleBlock(id, trackId) {
        let s = selectedBlocks.slice()
        let i = s.indexOf(id)
        if (i >= 0)
            s.splice(i, 1)
        else
            s.push(id)
        selectedBlocks = s
        anchorBlock = id
        selectedTrack = trackId
    }
    // Shift+click: on the tracks from the last block clicked to this one,
    // the blocks from the earlier start to the later end
    function selectRange(id, trackId) {
        let a = blockInfo(anchorBlock), b = blockInfo(id)
        if (!a || !b) {
            selectOnly(id, trackId)
            return
        }
        let start = Math.min(a.block.start, b.block.start)
        let end = Math.max(a.block.start + a.block.duration, b.block.start + b.block.duration)
        let s = []
        for (let row = Math.min(a.row, b.row); row <= Math.max(a.row, b.row); row++) {
            if (tracks[row].hidden)
                continue
            for (let blk of tracks[row].blocks)
                if (blk.start < end && blk.start + blk.duration > start)
                    s.push(blk.id)
        }
        selectedBlocks = s
        selectedTrack = trackId
    }
    // Ctrl+A: the blocks of the tracks that are not folded
    function selectAll() {
        let s = []
        for (let t of tracks)
            if (!t.hidden)
                for (let b of t.blocks)
                    s.push(b.id)
        selectedBlocks = s
    }
    // the blocks a rectangle of timeArea touches (added to the selection
    // with Ctrl or Shift)
    function selectBand(rect, additive) {
        let top = timeArea.mapToItem(trackColumn, rect.x, rect.y).y
        let from = msAt(rect.x - headerWidth), to = msAt(rect.x + rect.width - headerWidth)
        let s = additive ? selectedBlocks.slice() : []
        for (let i = 0; i < trackColumn.children.length; i++) {
            let row = trackColumn.children[i]
            if (row.trackId === undefined || row.hidden || row.y + row.rowHeight < top || row.y > top + rect.height)
                continue
            for (let b of row.track.blocks || [])
                if (b.start < to && b.start + b.duration > from && s.indexOf(b.id) < 0)
                    s.push(b.id)
        }
        selectedBlocks = s
    }
    // what a change on a block applies to: the selection when it is in it
    function targets(id) { return isSelected(id) ? selectedBlocks : [id] }
    // the ones that can loop or stretch, the ones on tracks that aren't locked
    function stretchable(ids) {
        return ids.filter(id => { let i = blockInfo(id); return i !== null && i.block.type !== "Audio" && i.block.cycle > 0 })
    }
    function deletable(ids) {
        return ids.filter(id => { let i = blockInfo(id); return i !== null && !tracks[i.row].locked })
    }
    // what was selected may be gone (undo, redo), or folded away
    function pruneSelection() {
        let shown = []
        for (let t of tracks)
            if (!t.hidden)
                for (let b of t.blocks)
                    shown.push(b.id)
        let s = selectedBlocks.filter(id => shown.indexOf(id) >= 0)
        if (s.length !== selectedBlocks.length)
            selectedBlocks = s
        if (selectedTrack >= 0 && !tracks.some(t => t.id === selectedTrack))
            selectedTrack = -1
    }
    function deleteSelection() {
        if (selectedBlocks.length === 0)
            return
        Core.timeline.removeBlocks(showId, selectedBlocks)
        selectedBlocks = []
    }
    // the copies are the selection
    function duplicate(ids) {
        if (ids.length === 0)
            return
        let copies = Core.timeline.duplicateBlocks(showId, ids)
        if (copies.length > 0) {
            selectedBlocks = copies
            anchorBlock = copies[0]
        }
    }
    function duplicateSelection() { duplicate(selectedBlocks) }

    /*****************************************************************
     * Copy and paste (also into another super scene of the show)
     *****************************************************************/
    function copy(ids) {
        if (ids.length > 0)
            Core.timeline.copyBlocks(showId, ids)
    }
    // (the ones on locked tracks are copied, and stay)
    function cut(ids) {
        if (ids.length === 0)
            return
        Core.timeline.cutBlocks(showId, ids)
        selectedBlocks = []
    }
    function copySelection() { copy(selectedBlocks) }
    function cutSelection() { cut(selectedBlocks) }
    // the earliest block at $ms, the topmost on $trackId. The pasted blocks
    // are the selection, and the cursor goes to their end: pasted again,
    // the next copy lies right after
    function paste(ms, trackId) {
        let ids = Core.timeline.pasteBlocks(showId, trackId, ms)
        if (ids.length === 0)
            return
        let end = ms
        for (let id of ids) {
            let i = blockInfo(id)
            if (i)
                end = Math.max(end, i.block.start + i.block.duration)
        }
        selectedBlocks = ids
        anchorBlock = ids[0]
        cursor = end
        // (the cursor stays in view)
        if (xOf(cursor) > viewWidth - 30)
            scrollX = Math.max(0, cursor * pxPerMs - viewWidth * 0.8)
    }
    function pasteAtCursor() { paste(cursor, targetTrack) }

    /*****************************************************************
     * A group dragged, the rubber band
     *****************************************************************/
    // the row of the track under a point of the editor: the first one above
    // the tracks, the last one below them
    function rowAt(y) {
        let p = mapToItem(trackColumn, 0, y)
        if (p.y < 0)
            return 0
        if (p.y >= trackColumn.height)
            return tracks.length - 1
        return rowOf(trackAt(y))
    }
    // the block $id pressed: the selection goes with it, but for the locked
    // blocks and the blocks on locked tracks
    function startGroupDrag(id) {
        let ids = [], start = Infinity, rowMin = tracks.length, rowMax = -1
        for (let m of targets(id)) {
            let i = blockInfo(m)
            if (!i || i.block.locked || tracks[i.row].locked)
                continue
            ids.push(m)
            start = Math.min(start, i.block.start)
            rowMin = Math.min(rowMin, i.row)
            rowMax = Math.max(rowMax, i.row)
        }
        dragIds = ids
        groupMinStart = ids.length ? start : 0
        groupRowMin = ids.length ? rowMin : 0
        groupRowMax = ids.length ? rowMax : 0
        dragDeltaMs = 0
        dragY = 0
        dragRows = 0
        dragTargets = []
        dragBlock = id
    }
    // the mouse at $y (of the editor) over the tracks: as many tracks down or
    // up, never off them. The tracks they land on are lit, none when one of
    // them is locked (then they move only in time)
    function dragGroupTo(trackId, y) {
        let over = rowAt(y)
        let rows = over < 0 ? dragRows : Math.max(-groupRowMin, Math.min(tracks.length - 1 - groupRowMax, over - rowOf(trackId)))
        let lit = []
        if (rows !== 0) {
            for (let m of dragIds) {
                let i = blockInfo(m)
                let t = i ? tracks[i.row + rows] : null
                if (!t || t.locked) {
                    lit = []
                    break
                }
                if (lit.indexOf(t.id) < 0)
                    lit.push(t.id)
            }
        }
        dragRows = rows
        dragTargets = lit
    }
    function endGroupDrag() {
        dragBlock = -1
        dragIds = []
        dragDeltaMs = 0
        dragY = 0
        dragRows = 0
        dragTargets = []
    }
    // a click on the empty part of a track, at $x of its blocks: the cursor
    // goes there, and no block is selected (but with Ctrl or Shift)
    function clickTrack(trackId, x, modifiers) {
        selectedTrack = trackId
        if (!(modifiers & (Qt.ControlModifier | Qt.ShiftModifier)))
            selectOnly(-1, trackId)
        cursor = snap(msAt(x))
    }
    // the empty part of a track pressed at $x, $y of $item; a drag of more
    // than 4 px draws the rectangle
    function pressBand(item, x, y) {
        bandFrom = item.mapToItem(timeArea, x, y)
        banding = false
    }
    function dragBand(item, x, y) {
        let p = item.mapToItem(timeArea, x, y)
        if (!banding && Math.abs(p.x - bandFrom.x) <= 4 && Math.abs(p.y - bandFrom.y) <= 4)
            return
        banding = true
        dragging = true
        band = Qt.rect(Math.min(p.x, bandFrom.x), Math.min(p.y, bandFrom.y), Math.abs(p.x - bandFrom.x), Math.abs(p.y - bandFrom.y))
    }
    function endBand(additive) {
        if (!banding)
            return
        banding = false
        dragging = false
        if (additive !== undefined)
            selectBand(band, additive)
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
    onShowIdChanged: { cursor = 0; scrollX = 0; selectedBlocks = []; anchorBlock = -1; selectedTrack = -1 }
    onTracksChanged: pruneSelection()

    function undo() { Core.timeline.undo(showId) }
    function redo() { Core.timeline.redo(showId) }
    // (on Windows also Ctrl+Shift+Z; the keyboard mappings never take Ctrl)
    Shortcut { sequences: [StandardKey.Undo]; enabled: editor.keysOn; onActivated: editor.undo() }
    Shortcut { sequences: [StandardKey.Redo]; enabled: editor.keysOn; onActivated: editor.redo() }
    Shortcut { sequences: [StandardKey.SelectAll]; enabled: editor.keysOn; onActivated: editor.selectAll() }
    Shortcut { sequence: "Ctrl+D"; enabled: editor.keysOn; onActivated: editor.duplicateSelection() }
    // (on Windows Cut is also Shift+Del, Paste also Shift+Ins)
    Shortcut {
        sequences: [StandardKey.Copy]
        enabled: editor.keysOn && editor.selectedBlocks.length > 0
        onActivated: editor.copySelection()
    }
    Shortcut {
        sequences: [StandardKey.Cut]
        enabled: editor.keysOn && editor.selectedBlocks.length > 0
        onActivated: editor.cutSelection()
    }
    Shortcut {
        sequences: [StandardKey.Paste]
        enabled: editor.keysOn && Core.timeline.clipboardCount > 0
        onActivated: editor.pasteAtCursor()
    }

    // (a press on a block, a track or the ruler brings the focus here)
    focus: true
    Keys.onPressed: (ev) => {
        if ((ev.key === Qt.Key_Delete || ev.key === Qt.Key_Backspace) && selectedBlocks.length > 0 && !typing) {
            deleteSelection()
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
            // (a value still typed in a field counts first)
            AppButton {
                iconGlyph: Theme.iconUndo
                implicitHeight: 30
                flat: true
                enabled: !!editor.history.canUndo
                ToolTip.text: qsTr("Undo: %1 (Ctrl+Z)").arg(editor.history.undoText || "")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: { commitEdits(); editor.undo() }
            }
            AppButton {
                iconGlyph: Theme.iconRedo
                implicitHeight: 30
                flat: true
                enabled: !!editor.history.canRedo
                ToolTip.text: qsTr("Redo: %1 (Ctrl+Y)").arg(editor.history.redoText || "")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: { commitEdits(); editor.redo() }
            }
            // the blocks copied, in this super scene or another one
            AppButton {
                text: qsTr("Paste")
                iconGlyph: Theme.iconPaste
                implicitHeight: 30
                enabled: Core.timeline.clipboardCount > 0
                ToolTip.text: qsTr("Paste %n block(s) at the cursor, from the selected track (Ctrl+V)", "", Core.timeline.clipboardCount)
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: { commitEdits(); editor.pasteAtCursor() }
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
                onValueModified: Core.timeline.setSettings(editor.showId, { bpm: value }, true)
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
                        onPressed: (ev) => { editor.forceActiveFocus(); editor.cursor = editor.snap(editor.msAt(ev.x)) }
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

                // the rectangle that selects blocks
                Rectangle {
                    visible: editor.banding
                    x: editor.band.x
                    y: editor.band.y
                    width: editor.band.width
                    height: editor.band.height
                    color: Qt.alpha(Theme.accent, 0.15)
                    border.color: Theme.accent
                    z: 30
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

        // ------------------------------------------------------------ the selected blocks
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: blockBar.implicitHeight + 4
            visible: editor.selectedBlocks.length > 0
            radius: Theme.radius
            color: Theme.surface
            border.color: Theme.border

            // one line; in a narrow window the buttons go under the times
            GridLayout {
                id: blockBar
                anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; leftMargin: 10; rightMargin: 10 }
                columns: blockTimes.implicitWidth + blockActions.implicitWidth + columnSpacing <= width ? 2 : 1
                columnSpacing: 8
                rowSpacing: 0

                RowLayout {
                    id: blockTimes
                    Layout.fillWidth: true
                    spacing: 8
                    // several blocks: how many (their times are set on each one)
                    Text {
                        visible: editor.selected === null
                        text: qsTr("%n block(s) selected", "", editor.selectedBlocks.length)
                        color: Theme.text
                        font.weight: Font.DemiBold
                    }
                    // one block: its times (its name gives way first)
                    RowLayout {
                        visible: editor.selected !== null
                        Layout.fillWidth: true
                        spacing: 8
                        Rectangle { width: 6; height: 24; radius: 3; color: editor.selected ? Theme.functionColor(editor.selected.type) : "transparent" }
                        Text {
                            Layout.fillWidth: true
                            Layout.preferredWidth: 150
                            Layout.maximumWidth: 150
                            Layout.minimumWidth: 60
                            text: editor.selected ? editor.selected.name : ""
                            color: Theme.text
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }
                        Text { text: qsTr("Start"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                        TimeSpin {
                            compact: true
                            implicitWidth: fitWidth
                            engineValue: editor.selected ? editor.selected.start : 0
                            to: 360000
                            // (the changes of a field within a second are one undo step)
                            onEdited: (v) => Core.timeline.moveBlock(editor.showId, editor.selectedBlock, -1, v, true)
                        }
                        Text { text: qsTr("Length"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                        TimeSpin {
                            compact: true
                            implicitWidth: fitWidth
                            engineValue: editor.selected ? editor.selected.duration : 0
                            to: 360000
                            onEdited: (v) => Core.timeline.resizeBlock(editor.showId, editor.selectedBlock, editor.selected.start, v, true)
                        }
                        Text { text: qsTr("Fade in"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                        TimeSpin {
                            compact: true
                            implicitWidth: fitWidth
                            engineValue: editor.selected ? editor.selected.fadeIn : 0
                            onEdited: (v) => Core.timeline.setBlock(editor.showId, editor.selectedBlock, "fadeIn", v, true)
                        }
                        Text { text: qsTr("Fade out"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                        TimeSpin {
                            compact: true
                            implicitWidth: fitWidth
                            engineValue: editor.selected ? editor.selected.fadeOut : 0
                            onEdited: (v) => Core.timeline.setBlock(editor.showId, editor.selectedBlock, "fadeOut", v, true)
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
                    }
                }
                RowLayout {
                    id: blockActions
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    spacing: 8
                    // (a value still typed in a field goes to its block first.
                    // Copy is a word: the icon of copies is Duplicate)
                    AppButton {
                        text: qsTr("Copy")
                        flat: true
                        implicitHeight: 28
                        ToolTip.text: qsTr("Ctrl+C")
                        ToolTip.visible: hovered
                        onClicked: { commitEdits(); editor.copySelection() }
                    }
                    AppButton {
                        text: qsTr("Cut")
                        iconGlyph: Theme.iconCut
                        flat: true
                        implicitHeight: 28
                        ToolTip.text: qsTr("Ctrl+X")
                        ToolTip.visible: hovered
                        onClicked: { commitEdits(); editor.cutSelection() }
                    }
                    AppButton {
                        iconGlyph: Theme.iconCopy
                        flat: true
                        implicitHeight: 28
                        ToolTip.text: qsTr("Duplicate (Ctrl+D)")
                        ToolTip.visible: hovered
                        onClicked: { commitEdits(); editor.duplicateSelection() }
                    }
                    AppButton {
                        iconGlyph: Theme.iconTrash
                        flat: true
                        implicitHeight: 28
                        accentColor: Theme.danger
                        ToolTip.text: qsTr("Delete (Del)")
                        ToolTip.visible: hovered
                        onClicked: { commitEdits(); editor.deleteSelection() }
                    }
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
