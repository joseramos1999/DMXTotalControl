/*
  DMX Total Control
  FixtureTile.qml

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

Rectangle {
    id: tile

    property string name: ""
    property string type: ""
    property string address: ""
    /** Its number, for the command line (0: none) */
    property int number: 0
    property bool selected: false

    /** additive is true with Ctrl/Shift, to build multi selections */
    signal clicked(bool additive)

    radius: Theme.radiusSmall
    color: selected ? Qt.alpha(Theme.accent, 0.22) : (mouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised)
    border.color: selected ? Theme.accent : Theme.border
    border.width: selected ? 2 : 1

    Text {
        id: numberText
        anchors { right: parent.right; top: parent.top; margins: 7 }
        visible: tile.number > 0
        text: tile.number
        font.family: Theme.monoFamily
        font.pixelSize: 10
        font.weight: Font.Bold
        color: tile.selected ? Theme.text : Theme.textFaint
    }
    Text {
        anchors { left: parent.left; right: numberText.visible ? numberText.left : parent.right; top: parent.top; margins: 8 }
        text: tile.name
        elide: Text.ElideRight
        font.pixelSize: Theme.fontSmall + 1
        font.weight: Font.DemiBold
        color: Theme.text
    }
    Text {
        anchors { left: parent.left; bottom: parent.bottom; margins: 8 }
        text: tile.address
        font.family: Theme.monoFamily
        font.pixelSize: 10
        color: Theme.textDim
    }
    // its type, where it fits whole (else: on hover)
    Text {
        id: typeText
        anchors { right: parent.right; bottom: parent.bottom; margins: 8 }
        text: tile.type
        width: parent.width / 2
        visible: implicitWidth <= width
        horizontalAlignment: Text.AlignRight
        font.pixelSize: 9
        color: tile.selected ? Theme.textDim : Theme.textFaint
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: (ev) => tile.clicked((ev.modifiers & (Qt.ControlModifier | Qt.ShiftModifier)) !== 0)
        ToolTip.text: tile.type
        ToolTip.visible: containsMouse && !typeText.visible && tile.type.length > 0
        ToolTip.delay: 600
    }
}
