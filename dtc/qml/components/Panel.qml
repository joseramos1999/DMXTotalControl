/*
  DMX Total Control
  Panel.qml

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
import DMXTotalControl

/** A titled surface. Children go below the title bar */
Rectangle {
    id: panel

    property string title: ""
    property alias headerItems: headerRow.data
    default property alias content: body.data

    color: Theme.surface
    radius: Theme.radius
    border.color: Theme.border
    border.width: 1

    Item {
        id: header
        visible: panel.title.length > 0 || headerRow.children.length > 0
        height: visible ? 44 : 0
        anchors { left: parent.left; right: parent.right; top: parent.top }

        Text {
            text: panel.title
            elide: Text.ElideRight
            anchors { left: parent.left; leftMargin: Theme.padding; right: headerRow.left; rightMargin: 8; verticalCenter: parent.verticalCenter }
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontNormal
            font.weight: Font.Bold
            font.capitalization: Font.AllUppercase
            font.letterSpacing: 0.8
            color: Theme.textDim
        }
        Row {
            id: headerRow
            spacing: 6
            anchors { right: parent.right; rightMargin: 8; verticalCenter: parent.verticalCenter }
        }
        Rectangle {
            height: 1
            color: Theme.border
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        }
    }

    Item {
        id: body
        anchors { left: parent.left; right: parent.right; top: header.bottom; bottom: parent.bottom }
    }
}
