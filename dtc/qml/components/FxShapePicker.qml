/*
  DMX Total Control
  FxShapePicker.qml

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

/** The 22 modes of an FX, or the figures of pan/tilt, to pick one */
Popup {
    id: picker

    property bool figure: false
    property int current: 1

    signal picked(int value)

    function openFor(isFigure, value) {
        figure = isFigure
        current = value
        open()
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: figure ? 560 : 640
    modal: true
    padding: 16
    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    contentItem: ColumnLayout {
        spacing: 10

        Text {
            text: picker.figure ? qsTr("Figure") : qsTr("Mode")
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
        }
        Text {
            Layout.fillWidth: true
            visible: !picker.figure
            wrapMode: Text.WordWrap
            text: qsTr("The dotted line is the base value: the wave goes above it, below it or both. Modes 9 to 12 are for chases, with a step.")
            color: Theme.textDim
            font.pixelSize: Theme.fontSmall
        }

        GridLayout {
            Layout.fillWidth: true
            columns: picker.figure ? 4 : 6
            columnSpacing: 6
            rowSpacing: 6

            Repeater {
                model: picker.figure ? Theme.fxFigureNames.length : 22
                delegate: Rectangle {
                    id: cell
                    required property int index
                    readonly property int value: picker.figure ? index : index + 1
                    readonly property bool selected: value === picker.current
                    Layout.fillWidth: true
                    implicitHeight: picker.figure ? 96 : 64
                    radius: Theme.radiusSmall
                    color: selected ? Qt.alpha(Theme.accent, 0.25) : (cellMouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised)
                    border.color: selected ? Theme.accent : Theme.border

                    FxShape {
                        anchors { left: parent.left; right: parent.right; top: parent.top; bottom: label.top; margins: 6 }
                        figure: picker.figure
                        value: cell.value
                        color: cell.selected ? Theme.accent : Theme.text
                    }
                    Text {
                        id: label
                        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; bottomMargin: 4 }
                        horizontalAlignment: Text.AlignHCenter
                        text: picker.figure ? Theme.fxFigureNames[cell.index] : cell.value
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSmall
                        elide: Text.ElideRight
                    }
                    MouseArea {
                        id: cellMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: { picker.picked(cell.value); picker.close() }
                    }
                }
            }
        }
    }
}
