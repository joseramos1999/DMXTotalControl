/*
  DMX Total Control
  MenuHeader.qml

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

/** The top of an ActionMenu: what it acts on. A disc with a glyph (its ring
 *  in the colour of the type), a number, a title and a detail on the right
 *  (an address), and a subtitle below. Not an entry (a plain Item): the
 *  mouse and the arrow keys pass it by. A long title never widens the menu:
 *  it is shortened */
Item {
    id: header

    property string glyph: ""
    property color glyphColor: Theme.textDim
    property color ringColor: Theme.border
    property string number: ""
    property string title: ""
    property string detail: ""
    property string subtitle: ""

    implicitHeight: 48
    implicitWidth: 50 + Math.min(Math.max(firstRow.implicitWidth, subtitleText.implicitWidth), 260) + 12

    Accessible.role: Accessible.StaticText
    Accessible.name: [number, title, detail, subtitle].filter(s => s.length > 0).join(", ")

    // the line under it
    Rectangle {
        x: 8
        y: header.height - 1
        width: header.width - 16
        height: 1
        color: Theme.border
    }

    Rectangle {
        x: 12
        y: (header.height - 1 - height) / 2
        width: 28
        height: 28
        radius: 14
        color: Theme.surfaceRaised
        border.width: 2
        border.color: header.ringColor
        IconLabel {
            anchors.centerIn: parent
            icon: header.glyph
            size: 13
            color: header.glyphColor
        }
    }

    Column {
        x: 50
        y: (header.height - 1 - height) / 2
        width: header.width - 62
        spacing: 1

        RowLayout {
            id: firstRow
            width: parent.width
            spacing: 6
            Text {
                visible: text.length > 0
                text: header.number
                font.family: Theme.monoFamily
                font.weight: Font.Bold
                font.pixelSize: Theme.fontNormal
                color: Theme.readable(Theme.accent)
            }
            Text {
                Layout.fillWidth: true
                text: header.title
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontNormal
                font.weight: Font.DemiBold
                color: Theme.text
                elide: Text.ElideRight
            }
            Text {
                Layout.leftMargin: 10
                visible: text.length > 0
                text: header.detail
                font.family: Theme.monoFamily
                font.pixelSize: Theme.fontSmall
                color: Theme.textDim
            }
        }
        Text {
            id: subtitleText
            width: parent.width
            text: header.subtitle
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSmall
            color: Theme.textDim
            elide: Text.ElideRight
        }
    }
}
