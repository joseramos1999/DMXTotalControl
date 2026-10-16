/*
  DMX Total Control
  NavButton.qml

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

AbstractButton {
    id: control

    property string iconGlyph: ""
    property string shortcutHint: ""

    width: 76
    height: 66
    checkable: true
    hoverEnabled: true
    focusPolicy: Qt.NoFocus

    ToolTip.visible: hovered && shortcutHint.length > 0
    ToolTip.delay: 600
    ToolTip.text: shortcutHint

    background: Rectangle {
        color: control.checked ? Theme.surfaceRaised : (control.hovered ? Theme.surface : "transparent")
        radius: Theme.radius

        Rectangle {
            visible: control.checked
            width: 3
            height: parent.height - 20
            radius: 2
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            color: Theme.accent
        }
    }

    contentItem: Column {
        spacing: 5
        topPadding: 11

        IconLabel {
            anchors.horizontalCenter: parent.horizontalCenter
            icon: control.iconGlyph
            size: 18
            color: control.checked ? Theme.accent : Theme.textDim
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: control.text
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSmall
            font.weight: Font.DemiBold
            color: control.checked ? Theme.text : Theme.textDim
        }
    }
}
