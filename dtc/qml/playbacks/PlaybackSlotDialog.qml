/*
  DMX Total Control
  PlaybackSlotDialog.qml

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
 * The properties of a playback: its label and colour, what its three
 * buttons do, and what its fader does (start it when it goes up, release
 * it at zero). Every change applies at once: openFor(page, slot)
 */
Popup {
    id: dialog

    property int page: -1
    property int slot: 0
    readonly property var info: { Core.playbacks.revision; return Core.playbacks.slotInfo(page, slot) }

    readonly property var actions: ["none", "go", "back", "pause", "pauseBack", "release", "flash", "flashGo", "toggle", "select"]
    readonly property var actionNames: [qsTr("Nothing"), qsTr("GO"), qsTr("Back"), qsTr("Pause"), qsTr("Pause / Back"),
                                        qsTr("Release"), qsTr("Flash"), qsTr("Flash + GO"), qsTr("On / off"), qsTr("Select")]
    readonly property var swatches: ["", "#3d8bfd", "#30a46c", "#e5484d", "#f5a524", "#ae3ec9",
                                     "#17a2b8", "#d6336c", "#f76707", "#e7e9ef", "#868e96"]

    function openFor(page, slot) {
        // this playback, even if the current page changes meanwhile (a page
        // that is gone: nothing, never the current page)
        const p = Core.playbacks.resolvePage(page)
        if (p < 0)
            return
        dialog.page = p
        dialog.slot = slot
        labelField.text = info.label || ""
        open()
    }
    function set(key, value) { Core.playbacks.setSlotOption(page, slot, key, value) }
    function commitLabel() {
        if (labelField.text.trim() !== (info.label || ""))
            set("label", labelField.text)
    }
    onAboutToHide: commitLabel()

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 460
    modal: true
    padding: 18

    // the playback went away (cleared, its scene deleted)
    onInfoChanged: if (opened && info.empty) close()

    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    component Caption: Text {
        color: Theme.textDim
        font.pixelSize: Theme.fontSmall
        font.weight: Font.DemiBold
    }

    contentItem: ColumnLayout {
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Rectangle {
                width: 6
                height: 28
                radius: 3
                color: dialog.info.color ? dialog.info.color : Theme.functionColor(dialog.info.type || "")
            }
            Text {
                Layout.fillWidth: true
                text: qsTr("Playback %1 · %2").arg(dialog.slot + 1).arg(dialog.info.name || "")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
                elide: Text.ElideRight
            }
            Text {
                text: Theme.typeLabel(dialog.info.type || "")
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 12
            rowSpacing: 10

            Caption { text: qsTr("Label") }
            TextField {
                id: labelField
                Layout.fillWidth: true
                color: Theme.text
                placeholderText: qsTr("The name of the scene")
                placeholderTextColor: Theme.textFaint
                selectByMouse: true
                onEditingFinished: dialog.commitLabel()
                Keys.onShortcutOverride: (event) => event.accepted = event.key === Qt.Key_Escape
                Keys.onEscapePressed: { text = dialog.info.label || ""; focus = false }
            }

            Caption { text: qsTr("Colour") }
            Flow {
                Layout.fillWidth: true
                spacing: 5
                Repeater {
                    model: dialog.swatches
                    Rectangle {
                        required property string modelData
                        readonly property bool chosen: modelData.length === 0 ? !dialog.info.color
                                                                              : !!dialog.info.color && Qt.colorEqual(dialog.info.color, modelData)
                        width: 24
                        height: 24
                        radius: 12
                        // the first one: the colour of its type
                        color: modelData.length > 0 ? modelData : Theme.functionColor(dialog.info.type || "")
                        border.color: Theme.text
                        border.width: chosen ? 2 : 0
                        Text {
                            anchors.centerIn: parent
                            visible: parent.modelData.length === 0
                            text: "A"
                            color: Theme.textOn(parent.color)
                            font.pixelSize: 10
                            font.weight: Font.Bold
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: dialog.set("color", parent.modelData)
                        }
                        ToolTip.text: modelData.length === 0 ? qsTr("The colour of its type") : ""
                        ToolTip.visible: modelData.length === 0 && swatchHover.hovered
                        HoverHandler { id: swatchHover }
                    }
                }
            }

            Caption { text: qsTr("Button 1") }
            ComboBox {
                Layout.fillWidth: true
                model: dialog.actionNames
                currentIndex: Math.max(0, dialog.actions.indexOf(dialog.info.button1 || "go"))
                onActivated: (i) => dialog.set("button1", dialog.actions[i])
            }
            Caption { text: qsTr("Button 2") }
            ComboBox {
                Layout.fillWidth: true
                model: dialog.actionNames
                currentIndex: Math.max(0, dialog.actions.indexOf(dialog.info.button2 || "pauseBack"))
                onActivated: (i) => dialog.set("button2", dialog.actions[i])
            }
            Caption { text: qsTr("Button 3") }
            ComboBox {
                Layout.fillWidth: true
                model: dialog.actionNames
                currentIndex: Math.max(0, dialog.actions.indexOf(dialog.info.button3 || "flash"))
                onActivated: (i) => dialog.set("button3", dialog.actions[i])
            }

            Caption { text: qsTr("The fader starts it") }
            AppSwitch {
                checked: dialog.info.faderStarts !== false
                onToggled: dialog.set("faderStarts", checked)
            }

            Caption { text: qsTr("The fader at zero releases it") }
            ComboBox {
                Layout.fillWidth: true
                readonly property bool cuelist: !!dialog.info.isCuelist
                // the default depends on the type: a cuelist keeps its cue, a scene stops
                model: [cuelist ? qsTr("Default (no: it keeps its cue)") : qsTr("Default (yes)"), qsTr("Yes"), qsTr("No")]
                currentIndex: dialog.info.faderReleasesMode === 1 ? 1 : dialog.info.faderReleasesMode === 0 ? 2 : 0
                onModelChanged: currentIndex = Qt.binding(() => dialog.info.faderReleasesMode === 1 ? 1
                                                               : dialog.info.faderReleasesMode === 0 ? 2 : 0)
                onActivated: (i) => dialog.set("faderReleases", i === 0 ? -1 : i === 1 ? 1 : 0)
            }
        }

        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("The fader is the intensity of what it plays. Raising it from zero starts it; GO with the fader down plays it at full.")
            color: Theme.textFaint
            font.pixelSize: Theme.fontSmall
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Close"); primary: true; onClicked: dialog.close() }
        }
    }
}
