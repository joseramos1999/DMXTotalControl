/*
  DMX Total Control
  CueActionsView.qml

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
 * The link and the actions of a cue: the cue a GO plays after it (a number
 * of times, or always) and what it does when it starts (a GO of another
 * cuelist, a scene, a group, the BPM...). It shows and edits the cue
 * $cueIndex (with several cues selected, the current one).
 */
Item {
    id: links

    property int cuelistId: -1
    property int cueIndex: -1

    property int revision: 0
    Connections {
        target: Core.cuelists
        function onEdited(id) { if (id === links.cuelistId) links.revision++ }
    }
    // the names of the scenes and cuelists the actions name
    Connections {
        target: Core
        function onFunctionsChanged() { links.revision++ }
    }

    readonly property var cues: {
        revision
        return cuelistId >= 0 ? Core.cuelists.cues(cuelistId) : []
    }
    readonly property var cue: cueIndex >= 0 && cueIndex < cues.length ? cues[cueIndex] : ({})
    readonly property var actions: {
        revision
        return cuelistId >= 0 && cueIndex >= 0 ? Core.cuelists.cueActions(cuelistId, cueIndex) : []
    }
    readonly property string atEnd: {
        revision
        return cuelistId >= 0 ? (Core.cuelists.options(cuelistId).atEnd || "loop") : "loop"
    }
    /** The links count: not in a Chase nor in a Timecode list (a link set
     *  there is kept, for when it is Normal again) */
    readonly property bool linksCount: {
        revision
        const mode = cuelistId >= 0 ? (Core.cuelists.options(cuelistId).mode || "normal") : "normal"
        return mode !== "chase" && mode !== "timecode"
    }

    /* The rows of the actions, changed in place: an edit keeps its row (a
       held arrow of its BPM goes on, the field keeps the focus) */
    function syncActions() {
        for (let i = 0; i < actions.length; i++) {
            const row = { action: actions[i] }
            if (i < actionModel.count)
                actionModel.set(i, row)
            else
                actionModel.append(row)
        }
        if (actionModel.count > actions.length)
            actionModel.remove(actions.length, actionModel.count - actions.length)
    }
    onActionsChanged: syncActions()
    Component.onCompleted: syncActions()
    ListModel { id: actionModel }

    /* The link */
    readonly property int linkIndex: cue.link ? cues.findIndex(c => c.uid === cue.link) : -1
    readonly property int linkCount: cue.linkCount || 0
    readonly property var linkChoices: [{ uid: 0, name: qsTr("The next cue") }].concat(
        cues.map(c => ({ uid: c.uid, name: c.number + (c.name ? " · " + c.name : "") })))
    function numberAt(i) { return i >= 0 && i < cues.length ? cues[i].number : "" }
    /** What the link does, in words */
    readonly property string linkText: {
        if (linkIndex < 0 || cueIndex < 0)
            return ""
        const from = numberAt(linkIndex), to = numberAt(cueIndex)
        // the cue after it once the link is done (past the end: as the list says)
        const last = cueIndex + 1 >= cues.length
        const then = !last ? numberAt(cueIndex + 1) : atEnd === "loop" ? numberAt(0) : ""
        if (linkIndex > cueIndex) {
            return linkCount === 0 ? qsTr("It skips to cue %1.").arg(from)
                 : then !== "" ? qsTr("It skips to cue %1 %n times, then goes on to cue %2.", "", linkCount).arg(from).arg(then)
                               : qsTr("It skips to cue %1 %n times.", "", linkCount).arg(from)
        }
        if (linkIndex === cueIndex) {
            if (linkCount === 0)
                return qsTr("Cue %1 repeats until you go to another cue.").arg(to)
            return then !== "" ? qsTr("Cue %1 plays %2 times, then cue %3.").arg(to).arg(linkCount + 1).arg(then)
                 : atEnd === "release" ? qsTr("Cue %1 plays %2 times, then the cuelist is released.").arg(to).arg(linkCount + 1)
                 : qsTr("Cue %1 plays %2 times, then the cuelist stays on it.").arg(to).arg(linkCount + 1)
        }
        if (linkCount === 0)
            return qsTr("Cues %1 to %2 loop until you go to another cue.").arg(from).arg(to)
        return then !== "" ? qsTr("Cues %1 to %2 play %3 times, then cue %4.").arg(from).arg(to).arg(linkCount + 1).arg(then)
             : atEnd === "release" ? qsTr("Cues %1 to %2 play %3 times, then the cuelist is released.").arg(from).arg(to).arg(linkCount + 1)
             : qsTr("Cues %1 to %2 play %3 times, then the cuelist stays on cue %2.").arg(from).arg(to).arg(linkCount + 1)
    }

    /* The actions: what they can name */
    readonly property var functionList: {
        revision
        return Core.functions.filter(f => f.id !== cuelistId).sort((a, b) => a.name.localeCompare(b.name))
    }
    readonly property var cuelistList: functionList.filter(f => f.type === "Cuelist")
    readonly property var groupList: {
        revision
        let list = [{ id: -1, name: qsTr("All groups") }]
        for (let i = 0; i < Core.zones.count; i++)
            list.push({ id: Core.zones.zone(i).id, name: Core.zones.zone(i).name })
        return list
    }
    /** The actions without their texts, as setCueActions takes them */
    function plain() {
        return actions.map(a => { let c = Object.assign({}, a); delete c.text; return c })
    }
    function setAction(index, values) {
        let list = plain()
        list[index] = Object.assign(list[index], values)
        Core.cuelists.setCueActions(cuelistId, cueIndex, list)
    }
    function removeAction(index) {
        let list = plain()
        list.splice(index, 1)
        Core.cuelists.setCueActions(cuelistId, cueIndex, list)
    }
    /** The uid of a cue of a cuelist (0: no such cue) */
    function cueUidAt(fid, index) {
        const list = fid >= 0 ? Core.cuelists.cues(fid) : []
        return index >= 0 && index < list.length ? list[index].uid : 0
    }
    function addAction(target, values) {
        let action = Object.assign({ target: target }, values)
        if (target === "playbackGo" || target === "playbackGoTo" || target === "playbackRelease")
            action.functionId = cuelistList.length ? cuelistList[0].id : -1
        if (target === "playbackGoTo") {
            action.cue = 0
            action.cueUid = cueUidAt(action.functionId, 0)
        }
        if (target === "scene")
            action.functionId = functionList.length ? functionList[0].id : -1
        if (target === "groupRelease")
            action.zoneId = -1
        if (target === "bpm")
            action.value = Core.bpm
        Core.cuelists.setCueActions(cuelistId, cueIndex, plain().concat([action]))
    }

    component Section: Text {
        Layout.fillWidth: true
        color: Theme.text
        font.weight: Font.Bold
        elide: Text.ElideRight
    }
    component Hint: Text {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: Theme.textFaint
        font.pixelSize: Theme.fontSmall
    }
    component Caption: Text {
        color: Theme.textDim
        elide: Text.ElideRight
    }
    // the first column of the actions: what each one does (in every language
    // the widest one fits), and Start / Stop of a scene with the arrow of its list
    FontMetrics { id: metrics; font.family: Theme.fontFamily; font.pixelSize: Theme.fontNormal }
    readonly property real kindWidth: Math.ceil(Math.max(...[qsTr("GO"), qsTr("Go to"), qsTr("BPM"), qsTr("Release")]
                                                         .map(s => metrics.advanceWidth(s)))) + 6
    readonly property real modeWidth: Math.ceil(Math.max(metrics.advanceWidth(qsTr("Start")),
                                                         metrics.advanceWidth(qsTr("Stop")))) + 60

    // a function, a cue or a group of an action; "(deleted)" when it is gone
    component Choice: ComboBox {
        Layout.fillWidth: true
        implicitHeight: 28
        focusPolicy: Qt.NoFocus
        textRole: "name"
        displayText: currentIndex < 0 ? qsTr("(deleted)") : currentText
    }

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: content.implicitHeight
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {}

        ColumnLayout {
            id: content
            width: flick.width
            spacing: 6
            enabled: links.cueIndex >= 0

            // the link
            Section { text: qsTr("Link") }
            RowLayout {
                Layout.fillWidth: true
                Layout.rightMargin: 10
                spacing: 8
                enabled: links.linksCount
                Caption { text: qsTr("After this cue") }
                ComboBox {
                    Layout.fillWidth: true
                    implicitHeight: 28
                    focusPolicy: Qt.NoFocus
                    model: links.linkChoices
                    textRole: "name"
                    currentIndex: links.linkIndex + 1
                    // a new link plays its cues once more (1); a link it had keeps its times
                    onActivated: (i) => {
                        const uid = links.linkChoices[i].uid
                        Core.cuelists.setCueLink(links.cuelistId, links.cueIndex, uid,
                                                 links.cue.link ? links.linkCount : 1)
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.rightMargin: 10
                spacing: 8
                visible: links.linkIndex >= 0
                enabled: links.linksCount
                Caption { text: qsTr("Times") }
                SpinBox {
                    from: 0; to: 999
                    editable: true
                    implicitWidth: 110
                    implicitHeight: 28
                    value: links.linkCount
                    onValueModified: Core.cuelists.setCueLink(links.cuelistId, links.cueIndex, links.cue.link || 0, value)
                }
                Hint { text: qsTr("0: always") }
            }
            Text {
                Layout.fillWidth: true
                Layout.rightMargin: 10
                visible: links.linksCount && text.length > 0
                text: links.linkText
                wrapMode: Text.WordWrap
                color: Theme.success
            }
            Hint {
                Layout.rightMargin: 10
                visible: !links.linksCount
                text: qsTr("Links don't count in a Chase or Timecode cuelist")
            }

            Rectangle { Layout.fillWidth: true; Layout.topMargin: 4; Layout.bottomMargin: 2; height: 1; color: Theme.border }

            // the actions
            RowLayout {
                Layout.fillWidth: true
                Layout.rightMargin: 10
                spacing: 8
                Section { text: qsTr("Actions when it starts") }
                AppButton {
                    id: addButton
                    text: qsTr("Add an action") + " ▾"
                    iconGlyph: Theme.iconPlus
                    implicitHeight: 28
                    // (under the button, its right edge on the button's)
                    onClicked: addMenu.popup(addButton, addButton.width - addMenu.width, addButton.height)
                    AppMenu {
                        id: addMenu
                        MenuItem {
                            text: qsTr("GO of a cuelist")
                            enabled: links.cuelistList.length > 0
                            onTriggered: links.addAction("playbackGo", {})
                        }
                        MenuItem {
                            text: qsTr("Go to a cue…")
                            enabled: links.cuelistList.length > 0
                            onTriggered: links.addAction("playbackGoTo", {})
                        }
                        MenuItem {
                            text: qsTr("Release a cuelist")
                            enabled: links.cuelistList.length > 0
                            onTriggered: links.addAction("playbackRelease", {})
                        }
                        MenuSeparator {}
                        MenuItem {
                            text: qsTr("Start a scene")
                            enabled: links.functionList.length > 0
                            onTriggered: links.addAction("scene", { mode: "on" })
                        }
                        MenuItem {
                            text: qsTr("Stop a scene")
                            enabled: links.functionList.length > 0
                            onTriggered: links.addAction("scene", { mode: "off" })
                        }
                        MenuItem {
                            text: qsTr("Release a group")
                            onTriggered: links.addAction("groupRelease", {})
                        }
                        MenuItem {
                            text: qsTr("Release all playbacks")
                            onTriggered: links.addAction("playbackReleaseAll", {})
                        }
                        MenuSeparator {}
                        MenuItem {
                            text: qsTr("Set the BPM")
                            onTriggered: links.addAction("bpm", {})
                        }
                    }
                }
            }
            Hint { text: qsTr("They run with its GO, Follow, Wait and Go to; not with BACK.") }

            // (in the scrolling column: a list of its own would take the wheel)
            Repeater {
                model: actionModel
                RowLayout {
                    id: actionRow
                    required property var action
                    required property int index
                    readonly property string kind: action.target
                    readonly property bool onCuelist: kind === "playbackGo" || kind === "playbackGoTo" || kind === "playbackRelease"
                    Layout.fillWidth: true
                    Layout.rightMargin: 10
                    spacing: 6

                    // what it does
                    Caption {
                        Layout.preferredWidth: actionRow.kind === "playbackReleaseAll" ? -1 : links.kindWidth
                        Layout.fillWidth: actionRow.kind === "playbackReleaseAll"
                        visible: actionRow.kind !== "scene"
                        text: actionRow.kind === "playbackGo" ? qsTr("GO")
                            : actionRow.kind === "playbackGoTo" ? qsTr("Go to")
                            : actionRow.kind === "bpm" ? qsTr("BPM")
                            : actionRow.kind === "playbackReleaseAll" ? qsTr("Release all playbacks")
                            : qsTr("Release")
                    }
                    ComboBox {
                        visible: actionRow.kind === "scene"
                        Layout.preferredWidth: links.modeWidth
                        implicitHeight: 28
                        focusPolicy: Qt.NoFocus
                        model: [qsTr("Start"), qsTr("Stop")]
                        currentIndex: actionRow.action.mode === "off" ? 1 : 0
                        onActivated: (i) => links.setAction(actionRow.index, { mode: i === 1 ? "off" : "on" })
                    }
                    // the cuelist, or the scene
                    Choice {
                        visible: actionRow.onCuelist || actionRow.kind === "scene"
                        model: actionRow.kind === "scene" ? links.functionList : links.cuelistList
                        currentIndex: (actionRow.kind === "scene" ? links.functionList : links.cuelistList)
                                      .findIndex(f => f.id === actionRow.action.functionId)
                        onActivated: (i) => {
                            const fid = model[i].id
                            let values = { functionId: fid }
                            // a new cuelist: its first cue
                            if (actionRow.kind === "playbackGoTo") {
                                values.cue = 0
                                values.cueUid = links.cueUidAt(fid, 0)
                            }
                            links.setAction(actionRow.index, values)
                        }
                    }
                    // the cue of a go to
                    ComboBox {
                        id: cueChoice
                        visible: actionRow.kind === "playbackGoTo"
                        readonly property var cueList: {
                            links.revision
                            return visible && actionRow.action.functionId >= 0 ? Core.cuelists.cues(actionRow.action.functionId) : []
                        }
                        Layout.preferredWidth: 84
                        implicitHeight: 28
                        focusPolicy: Qt.NoFocus
                        model: cueList.map(c => c.number + (c.name ? " · " + c.name : ""))
                        currentIndex: actionRow.action.cueUid ? cueList.findIndex(c => c.uid === actionRow.action.cueUid)
                                                                 : Math.min(actionRow.action.cue || 0, cueList.length - 1)
                        displayText: currentIndex < 0 ? "?" : cueList[currentIndex].number
                        // (the list is wider than the cell, with its right edge on the cell's)
                        popup.width: 200
                        popup.x: width - popup.width
                        onActivated: (i) => links.setAction(actionRow.index, { cue: i, cueUid: cueList[i].uid })
                    }
                    // the group
                    Choice {
                        visible: actionRow.kind === "groupRelease"
                        model: links.groupList
                        currentIndex: links.groupList.findIndex(g => g.id === actionRow.action.zoneId)
                        onActivated: (i) => links.setAction(actionRow.index, { zoneId: links.groupList[i].id })
                    }
                    // the BPM
                    SpinBox {
                        visible: actionRow.kind === "bpm"
                        from: 20; to: 300
                        editable: true
                        implicitWidth: 110
                        implicitHeight: 28
                        value: actionRow.action.value || 120
                        onValueModified: links.setAction(actionRow.index, { value: value })
                    }
                    Item { Layout.fillWidth: true; visible: actionRow.kind === "bpm" }
                    AppButton {
                        implicitWidth: 30
                        implicitHeight: 26
                        flat: true
                        iconGlyph: Theme.iconClose
                        ToolTip.text: qsTr("Remove: %1").arg(actionRow.action.text)
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        onClicked: links.removeAction(actionRow.index)
                    }
                }
            }
            Text {
                Layout.fillWidth: true
                Layout.topMargin: 12
                visible: links.actions.length === 0
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: links.cueIndex < 0 ? qsTr("Select a cue") : qsTr("No actions")
                color: Theme.textFaint
            }
        }
    }
}
