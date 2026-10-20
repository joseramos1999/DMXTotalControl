/*
  DMX Total Control
  SceneTile.qml

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
import DMXTotalControl.Backend

/** A big, touch friendly scene button */
Rectangle {
    id: tile

    property string name: ""
    property string type: ""
    property bool active: false
    property color zoneColor: Theme.accent
    // playing: the colour of its group, as dark as its text needs to read
    readonly property color fillColor: Theme.fill(zoneColor)
    property int keyIndex: 0
    property int priority: 5
    property bool large: false
    /** A cuelist shows the cue it plays: "Q 3 · Verso" */
    property int functionId: -1
    readonly property var cueStatus: {
        Core.playbacks.revision
        return type === "Cuelist" && functionId >= 0 ? Core.playbacks.functionStatus(functionId) : ({})
    }
    readonly property string cueText: cueStatus.running && cueStatus.cueNumber
                                      ? "Q " + cueStatus.cueNumber + (cueStatus.cueName ? " · " + cueStatus.cueName : "") : ""

    property bool flash: false
    /** Its own rhythm, tapped on its button (ms a step, 0: none) */
    property int tap: 0
    readonly property string rhythmText: tap > 0 ? " · " + Math.round(60000 / tap) + " BPM" : ""

    /** Left button down and up: scenes react on press, flash scenes also on release */
    signal pressedDown()
    signal releasedUp()
    signal contextRequested()

    radius: Theme.radius
    color: active ? fillColor : (mouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised)
    border.color: active ? Qt.lighter(zoneColor, 1.3) : Theme.border
    border.width: 1
    scale: mouse.pressed ? 0.97 : 1.0

    Behavior on color { ColorAnimation { duration: 120 } }
    Behavior on scale { NumberAnimation { duration: 80 } }

    // function type stripe
    Rectangle {
        width: 3
        radius: 2
        color: Theme.functionColor(tile.type)
        visible: !tile.active
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom; margins: 6 }
    }

    Text {
        anchors { left: parent.left; right: parent.right; top: parent.top; leftMargin: 14; rightMargin: tile.priority !== 5 ? 34 : 8; topMargin: 9 }
        text: tile.name
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        elide: Text.ElideRight
        font.pixelSize: tile.large ? Theme.fontLarge + 2 : Theme.fontNormal
        font.weight: Font.DemiBold
        color: tile.active ? Theme.textOn(tile.fillColor) : Theme.text
    }

    Text {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; leftMargin: 14; rightMargin: 24; bottomMargin: 7 }
        text: (tile.cueText.length > 0 ? tile.cueText : Theme.typeLabel(tile.type)) + tile.rhythmText
        elide: Text.ElideRight
        font.pixelSize: tile.cueText.length > 0 ? 11 : 10
        font.weight: tile.cueText.length > 0 ? Font.Bold : Font.Normal
        font.capitalization: tile.cueText.length > 0 ? Font.MixedCase : Font.AllUppercase
        font.letterSpacing: 0.6
        color: tile.active ? Theme.textOn(tile.fillColor) : tile.cueText.length > 0 ? Theme.success : Theme.textFaint
    }

    // a scene with steps (a cuelist) that plays: its rhythm, a half a step
    RhythmBar {
        functionId: tile.functionId
        litColor: tile.active ? Theme.textOn(tile.fillColor) : Theme.accent
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; leftMargin: 14; rightMargin: 14; bottomMargin: 3 }
        height: 3
    }

    // priority badge, only when it differs from the default
    Rectangle {
        visible: tile.priority !== 5
        width: prioText.implicitWidth + 8
        height: 16
        radius: 4
        color: tile.priority > 5 ? Theme.warning : Theme.surfaceHover
        anchors { right: parent.right; top: parent.top; margins: 6 }
        Text {
            id: prioText
            anchors.centerIn: parent
            text: "P" + tile.priority
            font.pixelSize: 10
            font.weight: Font.Bold
            color: tile.priority > 5 ? Theme.textOn(Theme.warning) : Theme.textDim
        }
    }

    // flash scenes play while pressed
    IconLabel {
        visible: tile.flash
        icon: Theme.iconBolt
        size: 11
        color: tile.active ? Theme.textOn(tile.fillColor) : Theme.warning
        anchors { right: parent.right; bottom: parent.bottom; rightMargin: tile.active ? 22 : 9; bottomMargin: 6 }
    }

    // running indicator
    Rectangle {
        visible: tile.active
        width: 8; height: 8; radius: 4
        color: Theme.textOn(tile.fillColor)
        anchors { right: parent.right; bottom: parent.bottom; margins: 9 }

        SequentialAnimation on opacity {
            running: tile.active
            loops: Animation.Infinite
            NumberAnimation { from: 1; to: 0.25; duration: 650 }
            NumberAnimation { from: 0.25; to: 1; duration: 650 }
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        cursorShape: Qt.PointingHandCursor
        // a press taken away (the grid scrolls with the finger) is released
        // too: a flash scene doesn't stay on
        property bool leftDown: false
        onPressed: (ev) => {
            if (ev.button === Qt.RightButton) {
                tile.contextRequested()
            } else {
                leftDown = true
                tile.pressedDown()
            }
        }
        onReleased: (ev) => {
            if (ev.button === Qt.LeftButton && leftDown) {
                leftDown = false
                tile.releasedUp()
            }
        }
        onCanceled: {
            if (leftDown) {
                leftDown = false
                tile.releasedUp()
            }
        }
    }
}
