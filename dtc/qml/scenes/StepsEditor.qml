/*
  DMX Total Control
  StepsEditor.qml

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
 * Steps scene: every step is a look with its fade and hold
 * times. Steps are recorded from the programmer. With several steps
 * selected (Ctrl/Shift+click), changing a time changes all of them.
 */
Panel {
    id: editor

    property var info: ({})
    property var selected: [0]

    readonly property var steps: info.steps || []
    readonly property int current: selected.length ? selected[selected.length - 1] : -1
    readonly property bool beats: !!info.followBpm

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

    function setTime(index, which, value) {
        let targets = selected.indexOf(index) >= 0 ? selected : [index]
        Core.effects.setStepTimes(info.id, targets,
                                  which === "fadeIn" ? value : -1,
                                  which === "hold" ? value : -1,
                                  which === "fadeOut" ? value : -1)
    }

    function setWait(index, wait) {
        Core.effects.setStepsWait(info.id, selected.indexOf(index) >= 0 ? selected : [index], wait)
    }

    onStepsChanged: selected = selected.filter(i => i < steps.length).length
                               ? selected.filter(i => i < steps.length) : (steps.length ? [0] : [])

    title: qsTr("Steps (%1)").arg(steps.length)
    headerItems: [
        AppButton {
            text: qsTr("Add step")
            iconGlyph: Theme.iconPlus
            primary: true
            implicitHeight: 28
            enabled: Core.programmer.hasValues
            ToolTip.text: qsTr("The programmer becomes a new step after the selected one")
            ToolTip.visible: hovered
            onClicked: {
                if (Core.effects.addStep(editor.info.id, editor.current))
                    editor.selected = [editor.current + 1]
            }
        },
        AppButton {
            text: qsTr("Save in step")
            iconGlyph: Theme.iconSave
            implicitHeight: 28
            enabled: Core.programmer.hasValues && editor.current >= 0
            ToolTip.text: qsTr("Store the programmer in the selected step")
            ToolTip.visible: hovered
            onClicked: Core.effects.updateStep(editor.info.id, editor.current)
        },
        AppButton {
            text: qsTr("Load step")
            iconGlyph: Theme.iconSliders
            implicitHeight: 28
            enabled: editor.current >= 0
            ToolTip.text: qsTr("Put the selected step in the programmer, to see and change it")
            ToolTip.visible: hovered
            onClicked: Core.effects.loadStep(editor.info.id, editor.current)
        },
        AppButton { iconGlyph: Theme.iconCopy; flat: true; implicitHeight: 28; enabled: editor.current >= 0
                    ToolTip.text: qsTr("Duplicate step"); ToolTip.visible: hovered
                    onClicked: Core.effects.duplicateStep(editor.info.id, editor.current) },
        AppButton { text: "▲"; flat: true; implicitHeight: 28; enabled: editor.current > 0
                    onClicked: { Core.effects.moveStep(editor.info.id, editor.current, editor.current - 1); editor.selected = [editor.current - 1] } },
        AppButton { text: "▼"; flat: true; implicitHeight: 28; enabled: editor.current >= 0 && editor.current < editor.steps.length - 1
                    onClicked: { Core.effects.moveStep(editor.info.id, editor.current, editor.current + 1); editor.selected = [editor.current + 1] } },
        AppButton { iconGlyph: Theme.iconTrash; flat: true; implicitHeight: 28; enabled: editor.steps.length > 1
                    ToolTip.text: qsTr("Delete step"); ToolTip.visible: hovered
                    onClicked: Core.effects.removeStep(editor.info.id, editor.current) }
    ]

    ColumnLayout {
        anchors { fill: parent; margins: 8 }
        spacing: 4

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 10
            spacing: 12
            Text { Layout.preferredWidth: 50; text: "#"; color: Theme.textFaint; font.weight: Font.Bold; font.pixelSize: Theme.fontSmall }
            Text { Layout.preferredWidth: 130; text: qsTr("FADE IN"); color: Theme.textFaint; font.weight: Font.Bold; font.pixelSize: Theme.fontSmall }
            Text { Layout.preferredWidth: 162; text: qsTr("HOLD"); color: Theme.textFaint; font.weight: Font.Bold; font.pixelSize: Theme.fontSmall }
            Text { Layout.preferredWidth: 130; text: qsTr("FADE OUT"); color: Theme.textFaint; font.weight: Font.Bold; font.pixelSize: Theme.fontSmall }
            Text { Layout.fillWidth: true; text: qsTr("CHANNELS ON"); color: Theme.textFaint; font.weight: Font.Bold; font.pixelSize: Theme.fontSmall }
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 3
            model: editor.steps
            ScrollBar.vertical: ScrollBar {}

            delegate: Rectangle {
                id: row
                required property var modelData
                required property int index
                readonly property bool isSelected: editor.selected.indexOf(index) >= 0
                width: list.width
                height: 44
                radius: Theme.radiusSmall
                color: isSelected ? Qt.alpha(Theme.accent, 0.18) : Theme.surfaceRaised
                border.color: isSelected ? Theme.accent : Theme.border

                MouseArea {
                    anchors.fill: parent
                    onClicked: (ev) => editor.select(row.index, ev.modifiers)
                    onDoubleClicked: Core.effects.loadStep(editor.info.id, row.index)
                }

                RowLayout {
                    anchors { fill: parent; leftMargin: 10; rightMargin: 10 }
                    spacing: 12
                    Text { Layout.preferredWidth: 50; text: row.index + 1; color: Theme.text; font.family: Theme.monoFamily; font.pixelSize: Theme.fontLarge }
                    TimeSpin { Layout.preferredWidth: 130; beats: editor.beats; engineValue: row.modelData.fadeIn
                               onEdited: (v) => editor.setTime(row.index, "fadeIn", v) }
                    // a hold, or none: the step waits for GO (QLC+ manual chasers)
                    RowLayout {
                        // (a layout fills the width by default)
                        Layout.fillWidth: false
                        Layout.preferredWidth: 162
                        Layout.maximumWidth: 162
                        spacing: 4
                        TimeSpin { Layout.preferredWidth: 130; visible: !row.modelData.waits; beats: editor.beats; engineValue: row.modelData.hold
                                   onEdited: (v) => editor.setTime(row.index, "hold", v) }
                        Text {
                            Layout.preferredWidth: 130
                            visible: !!row.modelData.waits
                            text: qsTr("Until GO")
                            color: Theme.accent
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                        }
                        AppButton {
                            text: "∞"
                            flat: true
                            implicitHeight: 28
                            implicitWidth: 28
                            highlighted: !!row.modelData.waits
                            ToolTip.text: row.modelData.waits ? qsTr("Back to a timed hold")
                                                              : qsTr("Wait for GO: the step holds until the next one is asked for")
                            ToolTip.visible: hovered
                            ToolTip.delay: 500
                            onClicked: editor.setWait(row.index, !row.modelData.waits)
                        }
                    }
                    TimeSpin { Layout.preferredWidth: 130; beats: editor.beats; engineValue: row.modelData.fadeOut
                               onEdited: (v) => editor.setTime(row.index, "fadeOut", v) }
                    Text { Layout.fillWidth: true; text: row.modelData.channels; color: Theme.textDim }
                }
            }
        }

        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Double click a step to load it in the programmer. Ctrl/Shift+click selects several steps to change their times together.")
            color: Theme.textFaint
            font.pixelSize: Theme.fontSmall
        }
    }
}
