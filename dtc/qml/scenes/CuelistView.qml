/*
  DMX Total Control
  CuelistView.qml

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
 * The cues of a cuelist: a cue holds only what it changes, the rest
 * tracks from the cues before it. Cues are recorded from the programmer,
 * updated (the change goes on to the next cues, or only into this one) and
 * loaded back to edit them. Ctrl/Shift+click selects several cues to change
 * their times, trigger or flags together. The tracking sheet shows what the
 * stage shows on the selected cue.
 *
 * (Not CuelistEditor.qml: that name is the C++ type of Core.cuelists.)
 */
Item {
    id: view

    property var info: ({})
    readonly property int fid: info.id !== undefined ? info.id : -1

    /** A problem to show (the page shows the messages of Core.cuelists itself) */
    signal message(string text)

    /** The cues, as Core.cuelists.cues() gives them */
    property var cues: []
    property var selected: []
    readonly property int current: selected.length ? selected[selected.length - 1] : -1
    readonly property var currentCue: current >= 0 && current < cues.length ? cues[current] : ({})
    readonly property bool hasProgrammer: Core.programmer.hasValues || Core.programmer.fxLayers.length > 0

    /** Where the list is: { running, cue, next, cueNumber, ... } */
    readonly property var status: {
        Core.playbacks.revision
        return fid >= 0 ? Core.playbacks.functionStatus(fid) : ({})
    }
    readonly property real progress: {
        Core.playbacks.progressRevision
        Core.playbacks.revision
        return fid >= 0 && status.running ? Core.playbacks.functionStatus(fid).progress : 0
    }
    /** The number a new cue gets after the selected one */
    readonly property string recordNumber: {
        cues
        return fid >= 0 ? Core.cuelists.nextNumber(fid, current) : ""
    }
    /** The cue the programmer holds, when it is one of these */
    readonly property int editingUid: Core.cuelists.editing.id === fid ? Core.cuelists.editing.uid : -1
    /** Room for the tracking sheet beside the cues */
    readonly property bool sideBySide: width >= 1150
    /** The tracking sheet: -1 automatic (shown when there is room beside the cues), 0 hidden, 1 shown */
    property int sheetMode: -1
    readonly property bool sheetShown: sheetMode < 0 ? sideBySide : sheetMode === 1

    function refresh() {
        // the selection follows its cues (by uid): a cue recorded, deleted or
        // renumbered elsewhere moves the rows, not what is selected
        const before = cues
        const keep = selected.filter(i => i >= 0 && i < before.length).map(i => before[i].uid)
        cues = fid >= 0 ? Core.cuelists.cues(fid) : []
        // the model keeps its rows: a change updates them in place
        for (let i = 0; i < cues.length; i++) {
            const c = cues[i]
            const row = { uid: c.uid, number: c.number, name: c.name, note: c.note, trigger: c.trigger,
                          triggerTime: c.triggerTime, fade: c.fade, downFade: c.downFade, delay: c.delay,
                          skip: c.skip, block: c.block, changes: c.changes, releases: c.releases + c.fxReleases,
                          fx: c.fx, palettesText: c.palettes.join(", ") }
            if (i < cueModel.count)
                cueModel.set(i, row)
            else
                cueModel.append(row)
        }
        if (cueModel.count > cues.length)
            cueModel.remove(cues.length, cueModel.count - cues.length)
        // the selection stays on the cues it held, where they are now
        let indexOfUid = {}
        for (let i = 0; i < cues.length; i++)
            indexOfUid[cues[i].uid] = i
        let s = keep.map(uid => indexOfUid[uid]).filter(i => i !== undefined)
        if (s.length === 0 && cues.length > 0) {
            // the cue in the programmer, else where it was
            const e = Core.cuelists.editing
            s = [e.id === fid && e.index !== undefined && e.index < cues.length ? e.index
                                                                              : Math.min(cues.length - 1, Math.max(0, current))]
        }
        if (JSON.stringify(s) !== JSON.stringify(selected))
            selected = s
    }

    function select(index, modifiers) {
        if (modifiers & Qt.ShiftModifier && selected.length) {
            let from = Math.min(current, index), to = Math.max(current, index)
            let s = []
            for (let i = from; i <= to; i++) s.push(i)
            selected = s
        } else if (modifiers & Qt.ControlModifier) {
            let s = selected.slice()
            let i = s.indexOf(index)
            if (i >= 0 && s.length > 1) s.splice(i, 1); else if (i < 0) s.push(index)
            selected = s
        } else {
            selected = [index]
        }
    }

    /** The cues a change on row $index applies to: the selection when it holds the row */
    function targets(index) { return selected.indexOf(index) >= 0 ? selected : [index] }

    function cueLabel(cue) {
        if (!cue || cue.number === undefined)
            return ""
        return cue.name && cue.name.length > 0 ? cue.number + " · " + cue.name : cue.number
    }

    function recordCue() {
        const number = recordNumber
        const index = Core.cuelists.recordCue(fid, number, { existing: "ask" })
        if (index === -2)
            existsDialog.ask(fid, number, {}, (i) => view.recorded(i))
        else if (index >= 0)
            recorded(index)
    }
    function recorded(index) {
        selected = [index]
        if (Core.settings.clearAfterRecord)
            Core.programmer.clear()
    }
    /** Update: the programmer into the selected cue; then cleared, as every Record and Update */
    function updateCue(cueOnly) {
        if (Core.cuelists.updateCue(fid, current, { cueOnly: cueOnly }) && Core.settings.clearAfterRecord)
            Core.programmer.clear()
    }
    function loadCue(index, tracked) {
        if (Core.cuelists.loadCue(fid, index, tracked))
            selected = [index]
    }

    onFidChanged: { cueModel.clear(); selected = []; refresh() }
    Component.onCompleted: refresh()

    Connections {
        target: Core.cuelists
        function onEdited(id) { if (id === view.fid) view.refresh() }
        /* a cue loaded from elsewhere (command line, playbacks): it is the
         * selected one. (Not editingChanged: every edit of the cuelist says
         * it too, and the selection would jump back to the loaded cue.) */
        function onCueLoaded(id, index) {
            if (id === view.fid && index < view.cues.length && view.current !== index)
                view.selected = [index]
        }
    }
    // the palettes give the cues their names
    Connections {
        target: Core.palettes
        function onPalettesChanged() { view.refresh() }
    }

    ListModel { id: cueModel }

    /* A time in a cell, in seconds: typed, or the mouse wheel once clicked
     * (0.1 s, with Shift 1 s). $allowSame: empty or "=" is the same as the fade */
    component TimeField: TextField {
        id: field
        property int ms: 0
        property bool same: false
        property bool allowSame: false
        signal committed(int ms)

        // to the ms (a time typed with 3 decimals, or from QLC+, shows as it is)
        function format(value) {
            const s = value / 1000
            return s.toLocaleString(Qt.locale(), 'f', value % 100 === 0 ? 1 : (value % 10 === 0 ? 2 : 3))
        }
        readonly property string shown: allowSame && same ? "=" : format(ms)
        /** Something was typed: only then a focus that leaves sets the time
         *  (of every cue selected), not a click in and out */
        property bool typed: false
        onTextEdited: typed = true
        function commit() {
            const t = text.trim().replace(",", ".").replace(/s$/, "").trim()
            if (typed) {
                // ("=" is no time: 0 typed on it is one, an instant down fade)
                const current = allowSame && same ? -1 : ms
                if (allowSame && (t === "" || t === "=")) {
                    if (!same)
                        committed(-2)
                } else {
                    const v = Number(t)
                    if (t.length > 0 && !isNaN(v) && v >= 0 && Math.round(v * 1000) !== current)
                        committed(Math.min(3600000, Math.round(v * 1000)))
                }
            }
            typed = false
            text = Qt.binding(() => field.shown)
        }

        text: shown
        horizontalAlignment: TextInput.AlignRight
        font.family: Theme.monoFamily
        font.pixelSize: Theme.fontSmall + 1
        color: (allowSame && same) || ms === 0 ? Theme.textFaint : Theme.text
        selectByMouse: true
        leftPadding: 6
        rightPadding: 6
        validator: RegularExpressionValidator { regularExpression: /=?|[0-9]{0,4}([.,][0-9]{0,3})?\s*s?/ }
        background: Rectangle {
            radius: Theme.radiusSmall
            color: field.activeFocus ? Theme.background : (fieldHover.hovered ? Theme.surfaceHover : "transparent")
            border.color: field.activeFocus ? Theme.accent : "transparent"
        }
        HoverHandler { id: fieldHover }
        onEditingFinished: commit()
        onActiveFocusChanged: { wheelDelta = 0; if (activeFocus) selectAll() }
        Keys.onShortcutOverride: (event) => event.accepted = event.key === Qt.Key_Escape
        Keys.onEscapePressed: { typed = false; text = Qt.binding(() => field.shown); focus = false }
        /* The wheel changes the time only while the cell is being edited
         * (clicked): over the other cells it scrolls the list. A notch
         * (120) is a step; a touchpad adds its small deltas up to one. */
        property real wheelDelta: 0
        WheelHandler {
            enabled: field.activeFocus
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: (event) => {
                const shift = (event.modifiers & Qt.ShiftModifier) !== 0
                // (Shift+wheel can come horizontal from the system)
                const delta = event.angleDelta.y !== 0 ? event.angleDelta.y : (shift ? event.angleDelta.x : 0)
                if (delta === 0)
                    return
                field.wheelDelta += delta
                const notches = Math.trunc(field.wheelDelta / 120)
                if (notches === 0)
                    return
                field.wheelDelta -= notches * 120
                const base = field.allowSame && field.same ? field.sameBase : field.ms
                field.committed(Math.min(3600000, Math.max(0, base + notches * (shift ? 1000 : 100))))
                // (what was typed and not entered gives way to the new time)
                field.typed = false
                field.text = Qt.binding(() => field.shown)
            }
        }
        /** The value the wheel starts from when it is "=" */
        property int sameBase: ms
    }

    // a small on/off box: Skip and Block
    component FlagBox: Rectangle {
        id: box
        property bool checked: false
        property string letter: ""
        property color tint: Theme.accent
        signal toggled()
        implicitWidth: 24
        implicitHeight: 24
        radius: 5
        color: checked ? tint : (boxMouse.containsMouse ? Theme.surfaceHover : "transparent")
        border.color: checked ? tint : Theme.borderStrong
        Text {
            anchors.centerIn: parent
            text: box.letter
            color: box.checked ? Theme.textOn(box.tint) : Theme.textFaint
            font.pixelSize: Theme.fontSmall
            font.weight: Font.Bold
        }
        MouseArea {
            id: boxMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: box.toggled()
        }
    }

    // a column title
    component HeaderText: Text {
        color: Theme.textFaint
        font.weight: Font.Bold
        font.pixelSize: Theme.fontSmall
        elide: Text.ElideRight
    }

    // the widths of the columns, shared by the titles and the rows
    readonly property int wCue: 64
    readonly property int wTrigger: 156
    readonly property int wTime: 62
    readonly property int wChanges: 96
    readonly property int wFlag: 24

    GridLayout {
        anchors.fill: parent
        columns: view.sideBySide ? 2 : 1
        columnSpacing: 12
        rowSpacing: 12

        Panel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 1000
            title: qsTr("Cues (%1)").arg(view.cues.length)
            headerItems: [
                // where it is, and its own transport (as a playback: the same state)
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    width: Math.min(implicitWidth, 300)
                    elide: Text.ElideRight
                    text: view.status.running
                          ? (view.status.paused ? qsTr("Paused") + " · " : "") + "Q " + view.status.cueNumber
                            + (view.status.cueName ? " · " + view.status.cueName : "")
                            + (view.status.nextNumber ? "   ▸ " + view.status.nextNumber : "")
                          : (view.status.nextNumber ? qsTr("Released · GO plays %1").arg(view.status.nextNumber)
                                                    : qsTr("Released"))
                    color: view.status.running ? Theme.success : Theme.textDim
                    font.pixelSize: Theme.fontSmall
                    font.weight: Font.DemiBold
                },
                AppButton {
                    iconGlyph: Theme.iconStepBackward
                    implicitHeight: 28
                    enabled: !!view.status.running
                    ToolTip.text: qsTr("Back: the cue before")
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                    onClicked: Core.playbacks.backFunction(view.fid)
                },
                AppButton {
                    text: qsTr("GO")
                    iconGlyph: Theme.iconPlay
                    implicitHeight: 28
                    primary: true
                    accentColor: Theme.success
                    enabled: view.cues.length > 0
                    ToolTip.text: qsTr("The next cue (the same GO as its playback)")
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                    onClicked: Core.playbacks.goFunction(view.fid)
                },
                AppButton {
                    iconGlyph: Theme.iconStop
                    implicitHeight: 28
                    enabled: !!view.status.running
                    ToolTip.text: qsTr("Release the cuelist")
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                    onClicked: Core.playbacks.releaseFunction(view.fid)
                }
            ]

            ColumnLayout {
                anchors { fill: parent; margins: 8 }
                spacing: 6

                // record and edit, from the programmer
                Flow {
                    Layout.fillWidth: true
                    spacing: 6

                    AppButton {
                        text: view.recordNumber.length ? qsTr("Record cue %1").arg(view.recordNumber) : qsTr("Record cue")
                        iconGlyph: Theme.iconRecord
                        accentColor: Theme.danger
                        primary: true
                        implicitHeight: 30
                        enabled: view.hasProgrammer && view.recordNumber.length > 0
                        ToolTip.text: qsTr("The programmer becomes a new cue after the selected one: only what changes from what it tracks")
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        onClicked: view.recordCue()
                    }
                    AppButton {
                        text: qsTr("Update") + " ▾"
                        iconGlyph: Theme.iconSave
                        implicitHeight: 30
                        enabled: view.hasProgrammer && view.current >= 0
                        ToolTip.text: qsTr("Merge the programmer into the selected cue")
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        onClicked: updateMenu.popup()
                        AppMenu {
                            id: updateMenu
                            MenuItem {
                                text: qsTr("Update cue %1 (the next cues follow)").arg(view.currentCue.number || "")
                                onTriggered: view.updateCue(false)
                            }
                            MenuItem {
                                text: qsTr("Only this cue (the next cues keep their look)")
                                onTriggered: view.updateCue(true)
                            }
                        }
                    }
                    AppButton {
                        text: qsTr("Load") + " ▾"
                        iconGlyph: Theme.iconSliders
                        implicitHeight: 30
                        enabled: view.current >= 0
                        ToolTip.text: qsTr("Put the selected cue in the programmer, to see and change it (double click a cue)")
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        onClicked: loadMenu.popup()
                        AppMenu {
                            id: loadMenu
                            MenuItem {
                                text: qsTr("What the stage shows on cue %1").arg(view.currentCue.number || "")
                                onTriggered: view.loadCue(view.current, true)
                            }
                            MenuItem {
                                text: qsTr("Only what cue %1 changes").arg(view.currentCue.number || "")
                                onTriggered: view.loadCue(view.current, false)
                            }
                        }
                    }
                    AppButton {
                        text: qsTr("Take out") + " ▾"
                        iconGlyph: Theme.iconEraser
                        implicitHeight: 30
                        enabled: view.hasProgrammer && view.current >= 0
                        ToolTip.text: qsTr("The channels of the programmer in the selected cue: take them out, or release them")
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        onClicked: takeOutMenu.popup()
                        AppMenu {
                            id: takeOutMenu
                            MenuItem {
                                text: qsTr("Remove the programmer's channels from cue %1 (they track again)").arg(view.currentCue.number || "")
                                onTriggered: Core.cuelists.removeFromCue(view.fid, view.current)
                            }
                            MenuItem {
                                text: qsTr("[Release] the programmer's channels in cue %1").arg(view.currentCue.number || "")
                                onTriggered: Core.cuelists.releaseInCue(view.fid, view.current)
                            }
                        }
                    }
                    Rectangle { width: 1; height: 30; color: Theme.border }
                    AppButton {
                        iconGlyph: Theme.iconCopy
                        flat: true
                        implicitHeight: 30
                        enabled: view.current >= 0
                        ToolTip.text: qsTr("Copy the selected cue to the end")
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        onClicked: copyMenu.popup()
                        AppMenu {
                            id: copyMenu
                            MenuItem {
                                text: qsTr("Copy what cue %1 changes").arg(view.currentCue.number || "")
                                onTriggered: { const i = Core.cuelists.copyCue(view.fid, view.current, "", false); if (i >= 0) view.selected = [i] }
                            }
                            MenuItem {
                                text: qsTr("Copy the whole look of cue %1 (block cue)").arg(view.currentCue.number || "")
                                onTriggered: { const i = Core.cuelists.copyCue(view.fid, view.current, "", true); if (i >= 0) view.selected = [i] }
                            }
                        }
                    }
                    AppButton {
                        text: qsTr("Number…")
                        flat: true
                        implicitHeight: 30
                        enabled: view.current >= 0
                        ToolTip.text: qsTr("Give the selected cue another number (also a point cue: 2.5)")
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        onClicked: view.askNumber(view.current)
                    }
                    AppButton {
                        iconGlyph: Theme.iconTrash
                        flat: true
                        implicitHeight: 30
                        enabled: view.selected.length > 0
                        ToolTip.text: qsTr("Delete the selected cues")
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        onClicked: deleteConfirm.open()
                    }
                    Rectangle { width: 1; height: 30; color: Theme.border }
                    AppButton {
                        text: qsTr("Tracking sheet")
                        iconGlyph: Theme.iconTable
                        flat: !view.sheetShown
                        highlighted: view.sheetShown
                        implicitHeight: 30
                        ToolTip.text: qsTr("What the stage shows on the selected cue, channel by channel")
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        onClicked: view.sheetMode = view.sheetShown ? 0 : 1
                    }
                }

                // column titles
                RowLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    spacing: 8
                    HeaderText { Layout.preferredWidth: view.wCue; text: qsTr("CUE") }
                    HeaderText { Layout.fillWidth: true; Layout.minimumWidth: 80; Layout.preferredWidth: 150; text: qsTr("NAME") }
                    HeaderText { Layout.preferredWidth: view.wTrigger; text: qsTr("TRIGGER") }
                    HeaderText { Layout.preferredWidth: view.wTime; text: qsTr("DELAY"); horizontalAlignment: Text.AlignRight }
                    HeaderText { Layout.preferredWidth: view.wTime; text: qsTr("FADE"); horizontalAlignment: Text.AlignRight }
                    HeaderText {
                        Layout.preferredWidth: view.wTime
                        text: qsTr("DOWN")
                        horizontalAlignment: Text.AlignRight
                        ToolTip.text: qsTr("Fade of the dimmers that go down; = is the same as the fade")
                        ToolTip.visible: downMouse.containsMouse
                        ToolTip.delay: 400
                        MouseArea { id: downMouse; anchors.fill: parent; hoverEnabled: true }
                    }
                    HeaderText { Layout.preferredWidth: view.wChanges; text: qsTr("CHANGES") }
                    HeaderText {
                        Layout.preferredWidth: view.wFlag
                        text: "S"
                        horizontalAlignment: Text.AlignHCenter
                        ToolTip.text: qsTr("Skip (cue only): what the cue changes doesn't go on to the next cues")
                        ToolTip.visible: skipMouse.containsMouse
                        ToolTip.delay: 400
                        MouseArea { id: skipMouse; anchors.fill: parent; hoverEnabled: true }
                    }
                    HeaderText {
                        Layout.preferredWidth: view.wFlag
                        text: "B"
                        horizontalAlignment: Text.AlignHCenter
                        ToolTip.text: qsTr("Block: Record and Update also store the values equal to the ones it tracks")
                        ToolTip.visible: blockMouse.containsMouse
                        ToolTip.delay: 400
                        MouseArea { id: blockMouse; anchors.fill: parent; hoverEnabled: true }
                    }
                    HeaderText { Layout.fillWidth: true; Layout.minimumWidth: 60; Layout.preferredWidth: 150; text: qsTr("NOTE") }
                }

                ListView {
                    id: list
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 3
                    boundsBehavior: Flickable.StopAtBounds
                    model: cueModel
                    ScrollBar.vertical: ScrollBar {}

                    delegate: Rectangle {
                        id: row
                        required property int index
                        required property var model
                        readonly property bool isSelected: view.selected.indexOf(index) >= 0
                        readonly property bool isLive: !!view.status.running && view.status.cue === index
                        readonly property bool isNext: view.status.next === index && !isLive
                        width: list.width
                        height: 38
                        radius: Theme.radiusSmall
                        color: isLive ? Qt.alpha(Theme.success, isSelected ? 0.32 : 0.2)
                               : isSelected ? Qt.alpha(Theme.accent, 0.18) : Theme.surfaceRaised
                        border.color: isSelected ? Theme.accent : isNext ? Theme.success : Theme.border
                        border.width: isNext ? 2 : 1

                        // the live cue: a green bar and how far its fade went
                        Rectangle {
                            visible: row.isLive
                            anchors { left: parent.left; top: parent.top; bottom: parent.bottom; margins: 1 }
                            width: 4
                            radius: 2
                            color: Theme.success
                        }
                        Rectangle {
                            visible: row.isLive && view.progress < 1
                            anchors { left: parent.left; bottom: parent.bottom; leftMargin: 6; bottomMargin: 2 }
                            width: (parent.width - 12) * view.progress
                            height: 2
                            color: Theme.success
                        }

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                            onClicked: (ev) => {
                                if (ev.button === Qt.RightButton) {
                                    if (!row.isSelected)
                                        view.select(row.index, 0)
                                    rowMenu.popup()
                                } else {
                                    view.select(row.index, ev.modifiers)
                                }
                            }
                            onDoubleClicked: (ev) => { if (ev.button === Qt.LeftButton) view.loadCue(row.index, true) }
                        }

                        RowLayout {
                            anchors { fill: parent; leftMargin: 10; rightMargin: 10 }
                            spacing: 8

                            // number: double click gives it another one; a pen: it is in the programmer
                            Item {
                                Layout.preferredWidth: view.wCue
                                Layout.fillHeight: true
                                Text {
                                    id: numberText
                                    anchors { left: parent.left; verticalCenter: parent.verticalCenter }
                                    width: Math.min(implicitWidth, parent.width - (penIcon.visible ? 16 : 0))
                                    text: row.model.number
                                    color: row.model.skip ? Theme.warning : Theme.text
                                    font.pixelSize: Theme.fontLarge
                                    font.weight: Font.DemiBold
                                    elide: Text.ElideRight
                                }
                                IconLabel {
                                    id: penIcon
                                    anchors { left: numberText.right; leftMargin: 5; verticalCenter: parent.verticalCenter }
                                    visible: view.editingUid === row.model.uid
                                    icon: Theme.iconPen
                                    size: 10
                                    color: Theme.functionColor("Cuelist")
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: (ev) => view.select(row.index, ev.modifiers)
                                    onDoubleClicked: view.askNumber(row.index)
                                    ToolTip.text: qsTr("In the programmer: Update stores the changes")
                                    ToolTip.visible: penIcon.visible && containsMouse
                                    ToolTip.delay: 400
                                    hoverEnabled: true
                                }
                            }
                            TextField {
                                id: nameField
                                Layout.fillWidth: true
                                Layout.minimumWidth: 80
                                Layout.preferredWidth: 150
                                text: row.model.name
                                // out of focus a long text shows its start
                                onTextChanged: if (!activeFocus) cursorPosition = 0
                                Component.onCompleted: cursorPosition = 0
                                placeholderText: qsTr("Name")
                                placeholderTextColor: Theme.textFaint
                                color: Theme.text
                                selectByMouse: true
                                background: Rectangle {
                                    radius: Theme.radiusSmall
                                    color: nameField.activeFocus ? Theme.background : "transparent"
                                    border.color: nameField.activeFocus ? Theme.accent : "transparent"
                                }
                                onActiveFocusChanged: {
                                    if (activeFocus && !row.isSelected)
                                        view.select(row.index, 0)
                                    if (!activeFocus)
                                        cursorPosition = 0
                                }
                                onEditingFinished: if (text !== row.model.name) Core.cuelists.setCueName(view.fid, row.index, text)
                                Keys.onShortcutOverride: (event) => event.accepted = event.key === Qt.Key_Escape
                                Keys.onEscapePressed: { text = Qt.binding(() => row.model.name); focus = false }
                            }
                            // trigger: GO, Follow (after the cue before ends) or Wait (after its GO)
                            RowLayout {
                                Layout.preferredWidth: view.wTrigger
                                spacing: 4
                                AppButton {
                                    Layout.preferredWidth: 86
                                    implicitHeight: 28
                                    flat: true
                                    text: (row.model.trigger === "follow" ? qsTr("Follow")
                                           : row.model.trigger === "wait" ? qsTr("Wait") : qsTr("GO")) + " ▾"
                                    highlighted: row.model.trigger !== "go"
                                    accentColor: Qt.alpha(Theme.warning, 0.75)
                                    onClicked: triggerMenu.popup()
                                    AppMenu {
                                        id: triggerMenu
                                        MenuItem {
                                            text: qsTr("GO: waits for a GO")
                                            onTriggered: Core.cuelists.setCueTrigger(view.fid, view.targets(row.index), "go", -1)
                                        }
                                        MenuItem {
                                            text: qsTr("Follow: when the cue before has finished, plus a time")
                                            onTriggered: Core.cuelists.setCueTrigger(view.fid, view.targets(row.index), "follow", -1)
                                        }
                                        MenuItem {
                                            text: qsTr("Wait: a time after the GO of the cue before")
                                            onTriggered: Core.cuelists.setCueTrigger(view.fid, view.targets(row.index), "wait", -1)
                                        }
                                    }
                                }
                                TimeField {
                                    Layout.fillWidth: true
                                    visible: row.model.trigger !== "go"
                                    ms: row.model.triggerTime
                                    onActiveFocusChanged: if (activeFocus && !row.isSelected) view.select(row.index, 0)
                                    onCommitted: (v) => Core.cuelists.setCueTrigger(view.fid, view.targets(row.index), "", v)
                                }
                                Item { Layout.fillWidth: true; visible: row.model.trigger === "go" }
                            }
                            TimeField {
                                Layout.preferredWidth: view.wTime
                                ms: row.model.delay
                                onActiveFocusChanged: if (activeFocus && !row.isSelected) view.select(row.index, 0)
                                onCommitted: (v) => Core.cuelists.setCueTimes(view.fid, view.targets(row.index), -1, -1, v)
                            }
                            TimeField {
                                Layout.preferredWidth: view.wTime
                                ms: row.model.fade
                                onActiveFocusChanged: if (activeFocus && !row.isSelected) view.select(row.index, 0)
                                onCommitted: (v) => Core.cuelists.setCueTimes(view.fid, view.targets(row.index), v, -1, -1)
                            }
                            TimeField {
                                Layout.preferredWidth: view.wTime
                                allowSame: true
                                same: row.model.downFade < 0
                                ms: Math.max(0, row.model.downFade)
                                sameBase: row.model.fade
                                onActiveFocusChanged: if (activeFocus && !row.isSelected) view.select(row.index, 0)
                                onCommitted: (v) => Core.cuelists.setCueTimes(view.fid, view.targets(row.index), -1, v, -1)
                            }
                            // what it changes: channels, FX, releases, palettes
                            Row {
                                Layout.preferredWidth: view.wChanges
                                spacing: 4
                                clip: true
                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: row.model.changes
                                    color: row.model.changes > 0 ? Theme.text : Theme.textFaint
                                    font.family: Theme.monoFamily
                                    font.pixelSize: Theme.fontSmall + 1
                                    width: 26
                                }
                                Rectangle {
                                    anchors.verticalCenter: parent.verticalCenter
                                    visible: row.model.fx > 0
                                    width: fxBadge.implicitWidth + 8
                                    height: 18
                                    radius: 4
                                    color: Qt.alpha(Theme.functionColor("FXScene"), 0.3)
                                    border.color: Theme.functionColor("FXScene")
                                    Text { id: fxBadge; anchors.centerIn: parent; text: "FX"; color: Theme.text; font.pixelSize: 10; font.weight: Font.Bold }
                                }
                                Rectangle {
                                    anchors.verticalCenter: parent.verticalCenter
                                    visible: row.model.releases > 0
                                    width: relBadge.implicitWidth + 8
                                    height: 18
                                    radius: 4
                                    color: Qt.alpha(Theme.danger, 0.2)
                                    border.color: Theme.danger
                                    Text { id: relBadge; anchors.centerIn: parent; text: "[R]"; color: Theme.text; font.pixelSize: 10; font.weight: Font.Bold }
                                }
                                IconLabel {
                                    anchors.verticalCenter: parent.verticalCenter
                                    visible: row.model.palettesText.length > 0
                                    icon: Theme.iconPalette
                                    size: 11
                                    color: Theme.textDim
                                    ToolTip.text: row.model.palettesText
                                    ToolTip.visible: paletteHover.hovered
                                    HoverHandler { id: paletteHover }
                                }
                            }
                            FlagBox {
                                Layout.preferredWidth: view.wFlag
                                Layout.preferredHeight: 24
                                letter: "S"
                                tint: Theme.warning
                                checked: row.model.skip
                                onToggled: Core.cuelists.setCueFlags(view.fid, view.targets(row.index), checked ? 0 : 1, -1)
                            }
                            FlagBox {
                                Layout.preferredWidth: view.wFlag
                                Layout.preferredHeight: 24
                                letter: "B"
                                tint: Theme.category("#f08c00")
                                checked: row.model.block
                                onToggled: Core.cuelists.setCueFlags(view.fid, view.targets(row.index), -1, checked ? 0 : 1)
                            }
                            TextField {
                                id: noteField
                                Layout.fillWidth: true
                                Layout.minimumWidth: 60
                                Layout.preferredWidth: 150
                                text: row.model.note
                                // out of focus a long text shows its start
                                onTextChanged: if (!activeFocus) cursorPosition = 0
                                Component.onCompleted: cursorPosition = 0
                                placeholderText: "—"
                                placeholderTextColor: Theme.textFaint
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSmall + 1
                                selectByMouse: true
                                background: Rectangle {
                                    radius: Theme.radiusSmall
                                    color: noteField.activeFocus ? Theme.background : "transparent"
                                    border.color: noteField.activeFocus ? Theme.accent : "transparent"
                                }
                                onActiveFocusChanged: {
                                    if (activeFocus && !row.isSelected)
                                        view.select(row.index, 0)
                                    if (!activeFocus)
                                        cursorPosition = 0
                                }
                                onEditingFinished: if (text !== row.model.note) Core.cuelists.setCueNote(view.fid, row.index, text)
                                Keys.onShortcutOverride: (event) => event.accepted = event.key === Qt.Key_Escape
                                Keys.onEscapePressed: { text = Qt.binding(() => row.model.note); focus = false }
                            }
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        width: parent.width - 40
                        visible: list.count === 0
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        text: qsTr("No cues yet: set a look in the programmer and record it as cue 1.")
                        color: Theme.textDim
                    }
                }

                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    text: qsTr("Double click a cue to load it in the programmer, its number to change it. Ctrl/Shift+click selects several cues to change their times together. Wheel on a clicked time: ±0.1 s (Shift: 1 s).")
                    color: Theme.textFaint
                    font.pixelSize: Theme.fontSmall
                }
            }
        }

        // what the stage shows on the selected cue
        Panel {
            visible: view.sheetShown
            Layout.fillWidth: !view.sideBySide
            Layout.fillHeight: true
            Layout.preferredWidth: 380
            Layout.preferredHeight: 1000
            title: view.current >= 0 ? qsTr("Cue %1 on stage").arg(view.cueLabel(view.currentCue)) : qsTr("Tracking")

            TrackingSheet {
                anchors { fill: parent; margins: 8 }
                cuelistId: view.fid
                cueIndex: view.current
            }
        }
    }

    function askNumber(index) {
        if (index < 0 || index >= cues.length)
            return
        numberPrompt.ask(qsTr("New number for cue %1").arg(cues[index].number), cues[index].number, (text) => {
            const error = Core.cuelists.renumber(view.fid, index, text)
            if (error.length > 0) {
                view.message(error)
                return
            }
            // the cue may have moved: select it where it is now
            const moved = view.cues.findIndex(c => c.number === text.replace(",", ".") || Number(c.number) === Number(text.replace(",", ".")))
            if (moved >= 0)
                view.selected = [moved]
        })
    }

    AppMenu {
        id: rowMenu
        MenuItem {
            text: qsTr("Go to cue %1").arg(view.currentCue.number || "")
            onTriggered: Core.playbacks.goToFunction(view.fid, view.current)
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Load what the stage shows")
            onTriggered: view.loadCue(view.current, true)
        }
        MenuItem {
            text: qsTr("Load only what it changes")
            onTriggered: view.loadCue(view.current, false)
        }
        MenuItem {
            text: qsTr("Update with the programmer")
            enabled: view.hasProgrammer
            onTriggered: view.updateCue(false)
        }
        MenuSeparator {}
        MenuItem { text: qsTr("Number…"); onTriggered: view.askNumber(view.current) }
        MenuItem { text: qsTr("Delete…"); onTriggered: deleteConfirm.open() }
    }

    TextPrompt { id: numberPrompt }
    CueExistsDialog { id: existsDialog }

    Popup {
        id: deleteConfirm
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 440
        modal: true
        padding: 18
        onOpened: keepLook.checked = true
        background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }
        contentItem: ColumnLayout {
            spacing: 12
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: view.selected.length > 1 ? qsTr("Delete %n cues?", "", view.selected.length)
                                               : qsTr("Delete cue %1?").arg(view.cueLabel(view.currentCue))
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
            }
            CheckBox {
                id: keepLook
                text: qsTr("Keep the look of the next cues")
                checked: true
            }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: keepLook.checked
                      ? qsTr("What it changes moves into the next cue, so the cues after it look the same.")
                      : qsTr("The next cues track from the cue before it, so they may look different.")
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton { text: qsTr("Cancel"); onClicked: deleteConfirm.close() }
                AppButton {
                    text: qsTr("Delete")
                    primary: true
                    accentColor: Theme.danger
                    onClicked: {
                        // the cue that takes the place of the first one deleted is selected
                        const first = Math.min(...view.selected)
                        Core.cuelists.removeCues(view.fid, view.selected, keepLook.checked)
                        view.selected = view.cues.length > 0 ? [Math.min(first, view.cues.length - 1)] : []
                        deleteConfirm.close()
                    }
                }
            }
        }
    }
}
