/*
  DMX Total Control
  MenuSection.qml

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

/** The title of a part of an ActionMenu, in small capitals under a line (the
 *  small form of the titles of the panels), with a state on its right (a
 *  glyph and a short text: how many are parked). Not an entry (a plain
 *  Item): the mouse and the arrow keys pass it by. shown: false hides it */
Item {
    id: section

    property string title: ""
    property string stateText: ""
    property string stateGlyph: ""
    property color stateColor: Theme.textDim
    property bool shown: true

    visible: shown
    height: shown ? implicitHeight : 0
    implicitHeight: 28
    implicitWidth: 24 + label.implicitWidth + (stateText.length > 0 ? 16 + stateRow.implicitWidth : 0)

    Accessible.role: Accessible.StaticText
    Accessible.name: stateText.length > 0 ? title + ", " + stateText : title

    // (the separator of the part above)
    Rectangle {
        x: 8
        y: 4
        width: section.width - 16
        height: 1
        color: Theme.border
    }

    Text {
        id: label
        x: 12
        anchors.bottom: parent.bottom
        bottomPadding: 4
        text: section.title
        font.family: Theme.fontFamily
        font.pixelSize: Theme.fontSmall
        font.weight: Font.Bold
        font.capitalization: Font.AllUppercase
        font.letterSpacing: 0.8
        color: Theme.textDim
    }

    // (on the line of the title)
    Row {
        id: stateRow
        visible: section.stateText.length > 0
        anchors { right: parent.right; rightMargin: 12; bottom: parent.bottom; bottomMargin: 4 }
        spacing: 4
        IconLabel {
            anchors.verticalCenter: parent.verticalCenter
            visible: section.stateGlyph.length > 0
            icon: section.stateGlyph
            size: 10
            color: section.stateColor
        }
        Text {
            text: section.stateText
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSmall
            font.weight: Font.DemiBold
            color: section.stateColor
        }
    }
}
