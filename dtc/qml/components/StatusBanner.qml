/*
  DMX Total Control
  StatusBanner.qml

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

/**
 * A bar above the pages that says how things are during the show (on every
 * page, also locked and full screen): "danger" in red, until what it says
 * is over; "warning" and "info" tinted. Its buttons go inside it.
 */
Rectangle {
    id: banner

    /** "danger", "warning" or "info" */
    property string level: "info"
    property string text: ""
    property string icon: level === "info" ? Theme.iconPlug : Theme.iconWarning
    /** Bigger, to be seen from afar */
    property bool big: false
    /** A × hides it: dismissed() */
    property bool dismissible: false
    signal dismissed()
    /** Its buttons, before the × */
    default property alias actions: actionRow.data
    /** A strip along its bottom edge, 0 to 1 (a hold that runs); -1: none */
    property real progress: -1

    readonly property bool solid: level === "danger"
    readonly property color tone: level === "danger" ? Theme.danger : level === "warning" ? Theme.warning : Theme.accent
    readonly property color textColor: solid ? Theme.textOn(Theme.fill(Theme.danger)) : Theme.text

    implicitHeight: big ? 54 : 38
    color: solid ? Theme.fill(Theme.danger) : Qt.alpha(tone, 0.18)
    Rectangle { width: parent.width; height: 1; anchors.bottom: parent.bottom; color: banner.solid ? Qt.darker(Theme.danger, 1.3) : banner.tone }
    Rectangle {
        visible: banner.progress >= 0
        anchors { left: parent.left; bottom: parent.bottom }
        width: parent.width * Math.max(0, Math.min(1, banner.progress))
        height: 4
        color: banner.solid ? banner.textColor : banner.tone
    }

    RowLayout {
        anchors { fill: parent; leftMargin: 14; rightMargin: 8 }
        spacing: 10

        IconLabel {
            icon: banner.icon
            size: banner.big ? 20 : 14
            color: banner.solid ? banner.textColor : Theme.readable(banner.tone, Theme.background)
        }
        Text {
            Layout.fillWidth: true
            text: banner.text
            color: banner.textColor
            font.pixelSize: banner.big ? Theme.fontLarge : Theme.fontNormal
            font.weight: banner.big || banner.solid ? Font.Bold : Font.DemiBold
            elide: Text.ElideRight
        }
        Row {
            id: actionRow
            spacing: 6
        }
        AppButton {
            visible: banner.dismissible
            iconGlyph: Theme.iconClose
            flat: true
            implicitHeight: 28
            ToolTip.text: qsTr("Hide")
            ToolTip.visible: hovered
            onClicked: banner.dismissed()
        }
    }
}
