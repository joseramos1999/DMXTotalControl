/*
  DMX Total Control
  StaticEditor.qml

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

/** A static scene: its values are edited in the programmer */
Panel {
    id: editor

    property var info: ({})
    /** The last save into it didn't go: steps play their channels through
     *  this scene (QLC+ shows), it can't take the FX of the programmer */
    property bool saveRefused: false
    onInfoChanged: saveRefused = false

    signal editInProgrammer(int id)
    /** The programmer was saved in it */
    signal saved()

    title: qsTr("Static scene")

    ColumnLayout {
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 16 }
        spacing: 14

        Text {
            text: qsTr("%1 channels on %2 fixtures").arg(editor.info.channels || 0).arg(editor.info.fixtures || 0)
            color: Theme.text
            font.pixelSize: Theme.fontLarge
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("A static scene stores fixed values. Load it in the programmer to change them, then save the programmer back into the scene.")
            color: Theme.textDim
        }
        // the palettes it was recorded with: it changes with them
        ColumnLayout {
            Layout.fillWidth: true
            readonly property var palettes: {
                Core.palettes.palettes
                return editor.info.id !== undefined ? Core.palettes.scenePalettes(editor.info.id) : []
            }
            visible: palettes.length > 0
            spacing: 6
            Text {
                text: qsTr("Uses these palettes: when they change, the scene changes too")
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
            }
            Flow {
                Layout.fillWidth: true
                spacing: 6
                Repeater {
                    model: parent.parent.palettes
                    Rectangle {
                        required property var modelData
                        width: chip.implicitWidth + 30
                        height: 26
                        radius: 13
                        color: Theme.surfaceRaised
                        border.color: Theme.border
                        Rectangle {
                            anchors { left: parent.left; leftMargin: 8; verticalCenter: parent.verticalCenter }
                            width: 10; height: 10; radius: 5
                            color: modelData.color.length > 0 ? modelData.color : Theme.textFaint
                        }
                        Text {
                            id: chip
                            anchors { left: parent.left; leftMargin: 22; verticalCenter: parent.verticalCenter }
                            text: modelData.name
                            color: Theme.text
                            font.pixelSize: Theme.fontSmall
                        }
                    }
                }
            }
        }
        RowLayout {
            spacing: 8
            AppButton {
                text: qsTr("Edit in the programmer")
                iconGlyph: Theme.iconSliders
                primary: true
                onClicked: editor.editInProgrammer(editor.info.id)
            }
            AppButton {
                text: qsTr("Save the programmer in this scene")
                iconGlyph: Theme.iconSave
                enabled: Core.programmer.hasValues
                onClicked: {
                    editor.saveRefused = !Core.programmer.updateScene(editor.info.id)
                    if (!editor.saveRefused)
                        editor.saved()
                }
            }
        }
        Text {
            Layout.fillWidth: true
            visible: editor.saveRefused
            wrapMode: Text.WordWrap
            text: qsTr("Steps play their channels through this scene: it can't take FX. Record the programmer as a new scene.")
            color: Theme.warning
            font.pixelSize: Theme.fontSmall
        }
    }
}
