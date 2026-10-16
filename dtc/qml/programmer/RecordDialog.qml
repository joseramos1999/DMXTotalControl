/*
  DMX Total Control
  RecordDialog.qml

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
 * Record the programmer: into a new scene (optionally straight into a
 * group), into a new cuelist (the programmer is its cue 1), or as a cue of a
 * cuelist (only what changes from what the cue tracks)
 */
Popup {
    id: dialog

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 460
    modal: true
    padding: 18

    /** Snapshot of everything that leaves the software, instead of the programmer */
    property bool everything: false
    /** The cuelists to record into: [{ id, name }] */
    property var cuelistList: []
    /** -2 a new scene, -1 a new cuelist, else the ID of a cuelist */
    readonly property int target: everything || targetCombo.currentIndex <= 0 ? -2
                                  : targetCombo.currentIndex === 1 ? -1
                                  : (cuelistList[targetCombo.currentIndex - 2] || { id: -2 }).id
    readonly property bool toCue: target >= 0
    readonly property string targetName: toCue ? (cuelistList[targetCombo.currentIndex - 2] || { name: "" }).name : ""
    /** The number a cue gets when none is typed */
    readonly property string nextNumber: {
        Core.functions
        return toCue ? Core.cuelists.nextNumber(target) : ""
    }

    /** A result to show (the cue, the playback) */
    signal recorded(string text)

    /** : into a new scene (the Snapshot menu), not the selected cuelist */
    property bool sceneOnly: false

    function openFor(all, toScene) {
        everything = all
        sceneOnly = !!toScene
        open()
    }

    function zoneNames() {
        let names = [qsTr("No group")]
        for (let i = 0; i < Core.zones.count; i++)
            names.push(Core.zones.zone(i).name)
        return names
    }

    function finished(text) {
        if (Core.settings.clearAfterRecord)
            Core.programmer.clear()
        if (text.length > 0)
            dialog.recorded(text)
    }

    function cueDone(cuelistId, cuelistName, index) {
        const cues = Core.cuelists.cues(cuelistId)
        const number = index >= 0 && index < cues.length ? cues[index].number : ""
        finished(qsTr("Cue %1 recorded in %2").arg(number).arg(cuelistName))
    }

    function record() {
        const name = nameField.text.trim()
        if (dialog.everything) {
            // (the highlight isn't part of the look: off first)
            if (Core.programmer.highlight)
                return
            if (Core.programmer.snapshotAll(name, zoneCombo.currentIndex - 1, prioritySpin.value) >= 0)
                finished("")
            dialog.close()
            return
        }
        if (dialog.target === -2) {
            if (Core.programmer.recordScene(name, zoneCombo.currentIndex - 1, prioritySpin.value) >= 0)
                finished("")
            dialog.close()
            return
        }
        if (dialog.target === -1) {
            const id = Core.cuelists.create(name, zoneCombo.currentIndex - 1, prioritySpin.value)
            if (id < 0)
                return
            let text = qsTr("New cuelist with cue 1")
            if (onPlaybackCheck.checked) {
                const slot = Core.playbacks.firstFreeSlot(-1)
                if (slot >= 0) {
                    Core.playbacks.assign(-1, slot, id)
                    text = qsTr("New cuelist on playback %1 with cue 1").arg(slot + 1)
                } else {
                    text = qsTr("New cuelist with cue 1: the playback page is full")
                }
            }
            finished(text)
            dialog.close()
            return
        }

        // a cue of a cuelist
        const cuelistId = dialog.target
        const cuelistName = dialog.targetName
        const number = cueField.text.trim()
        const opts = { name: name, cueOnly: cueOnlyCheck.checked, block: blockCheck.checked }
        const index = Core.cuelists.recordCue(cuelistId, number, Object.assign({ existing: "ask" }, opts))
        if (index === -2) {
            dialog.close()
            existsDialog.ask(cuelistId, number, opts, (i) => dialog.cueDone(cuelistId, cuelistName, i))
        } else if (index >= 0) {
            dialog.close()
            cueDone(cuelistId, cuelistName, index)
        }
        // -1: the message says why, the dialog stays open
    }

    onOpened: {
        nameField.text = ""
        cueField.text = ""
        cueOnlyCheck.checked = false
        blockCheck.checked = false
        onPlaybackCheck.checked = true
        zoneCombo.model = zoneNames()
        zoneCombo.currentIndex = Core.zones.count > 0 ? 1 : 0
        prioritySpin.value = Core.settings.defaultPriority
        // the cuelists, the selected one first in mind: it is the default
        cuelistList = Core.functions.filter(f => f.type === "Cuelist").map(f => ({ id: f.id, name: f.name }))
        targetCombo.model = [qsTr("New scene"), qsTr("New cuelist")].concat(cuelistList.map(c => c.name))
        const selected = cuelistList.findIndex(c => c.id === Core.cuelists.selectedId)
        targetCombo.currentIndex = selected >= 0 && !dialog.sceneOnly ? selected + 2 : 0
        if (dialog.toCue)
            cueField.forceActiveFocus()
        else
            nameField.forceActiveFocus()
    }

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surface
        border.color: Theme.borderStrong
    }

    contentItem: ColumnLayout {
        spacing: 10

        Text {
            text: dialog.everything ? qsTr("Snapshot of the stage") : qsTr("Record")
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
        }
        Text {
            Layout.fillWidth: true
            visible: dialog.everything
            wrapMode: Text.WordWrap
            text: qsTr("Everything the lights show now (scenes, live edits and the mixer, without the grand master) becomes a new static scene, on every fixture with light.")
            color: Theme.textDim
            font.pixelSize: Theme.fontSmall
        }
        // the highlight is over everything: what is under it can't be read
        RowLayout {
            Layout.fillWidth: true
            visible: dialog.everything && Core.programmer.highlight
            spacing: 10
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Highlight is on: it isn't part of the look. Turn it off for the snapshot.")
                color: Theme.warning
                font.pixelSize: Theme.fontSmall
            }
            AppButton {
                text: qsTr("Highlight off")
                onClicked: Core.programmer.highlight = false
            }
        }

        Text { visible: !dialog.everything; text: qsTr("Record into"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
        ComboBox {
            id: targetCombo
            Layout.fillWidth: true
            visible: !dialog.everything
            onActivated: (i) => { if (i >= 2) cueField.forceActiveFocus(); else nameField.forceActiveFocus() }
        }
        Text {
            Layout.fillWidth: true
            visible: dialog.toCue
            wrapMode: Text.WordWrap
            text: qsTr("The cue holds what the programmer changes from what the cues before it leave on stage.")
            color: Theme.textFaint
            font.pixelSize: Theme.fontSmall
        }
        Text {
            Layout.fillWidth: true
            visible: dialog.target === -1
            wrapMode: Text.WordWrap
            text: qsTr("The programmer becomes cue 1 of a new cuelist; record the next cues into it.")
            color: Theme.textFaint
            font.pixelSize: Theme.fontSmall
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            ColumnLayout {
                visible: dialog.toCue
                spacing: 4
                Text { text: qsTr("Cue"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                TextField {
                    id: cueField
                    Layout.preferredWidth: 90
                    placeholderText: dialog.nextNumber
                    placeholderTextColor: Theme.textFaint
                    color: Theme.text
                    font.family: Theme.monoFamily
                    validator: RegularExpressionValidator { regularExpression: /[0-9]{0,5}([.,][0-9]{0,3})?/ }
                    background: Rectangle { radius: Theme.radiusSmall; color: Theme.background; border.color: cueField.activeFocus ? Theme.accent : Theme.border }
                    ToolTip.text: qsTr("Empty: the next number. 2.5 goes between 2 and 3")
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                    onAccepted: dialog.record()
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text { text: qsTr("Name"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                TextField {
                    id: nameField
                    Layout.fillWidth: true
                    placeholderText: dialog.toCue ? qsTr("e.g. Chorus") : dialog.target === -1 ? qsTr("e.g. Song 1")
                                                                                           : qsTr("e.g. Warm wash")
                    placeholderTextColor: Theme.textFaint
                    color: Theme.text
                    background: Rectangle { radius: Theme.radiusSmall; color: Theme.background; border.color: nameField.activeFocus ? Theme.accent : Theme.border }
                    onAccepted: dialog.record()
                }
            }
        }

        // a cue
        CheckBox {
            id: cueOnlyCheck
            visible: dialog.toCue
            text: qsTr("Only this cue (what it changes doesn't go on to the next cues)")
        }
        CheckBox {
            id: blockCheck
            visible: dialog.toCue
            text: qsTr("All the values (block cue)")
            ToolTip.text: qsTr("Also the values equal to what the cue tracks: it looks the same whatever the cues before it do")
            ToolTip.visible: hovered
            ToolTip.delay: 500
        }

        // a scene or a new cuelist
        Text { visible: !dialog.toCue; text: qsTr("Add to group"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
        ComboBox {
            id: zoneCombo
            Layout.fillWidth: true
            visible: !dialog.toCue
        }
        RowLayout {
            Layout.fillWidth: true
            visible: !dialog.toCue
            Text { Layout.fillWidth: true; text: qsTr("Priority (0-10)"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
            SpinBox { id: prioritySpin; from: 0; to: 10; value: 5; editable: true }
        }
        Text {
            Layout.fillWidth: true
            visible: !dialog.toCue
            wrapMode: Text.WordWrap
            text: qsTr("When two scenes control the same channel, the higher priority wins.")
            color: Theme.textFaint
            font.pixelSize: Theme.fontSmall
        }
        CheckBox {
            id: onPlaybackCheck
            visible: dialog.target === -1
            text: qsTr("Put it on the first free playback")
            checked: true
        }

        CheckBox {
            id: clearCheck
            text: qsTr("Clear the programmer after recording")
            checked: Core.settings.clearAfterRecord
            onToggled: Core.settings.clearAfterRecord = checked
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Cancel"); onClicked: dialog.close() }
            AppButton {
                text: dialog.toCue ? qsTr("Record cue %1").arg(cueField.text.trim().length ? cueField.text.trim() : dialog.nextNumber)
                                   : qsTr("Record")
                iconGlyph: Theme.iconRecord
                accentColor: Theme.danger
                primary: true
                enabled: !(dialog.everything && Core.programmer.highlight)
                onClicked: dialog.record()
            }
        }
    }

    CueExistsDialog { id: existsDialog }
}
