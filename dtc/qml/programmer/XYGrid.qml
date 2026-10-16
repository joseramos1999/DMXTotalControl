/*
  DMX Total Control
  XYGrid.qml

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
 * Pan and tilt of the selection (XY grid): drag the cross to place
 * it, or drag anywhere else for a slow, fine move. Sliders below and beside.
 */
ColumnLayout {
    id: xy

    readonly property var position: Core.programmer.position
    readonly property real px: position.x !== undefined ? position.x : 0.5
    readonly property real py: position.y !== undefined ? position.y : 0.5

    spacing: 6
    enabled: !!position.available

    RowLayout {
        spacing: 6
        // tilt: up is the top of the grid
        VFader {
            id: tiltSlider
            Layout.preferredHeight: 220
            implicitWidth: 30
            from: 0; to: 1; stepSize: 0
            value: 1 - xy.py
            onUserMoved: (v) => Core.programmer.setPosition(xy.px, 1 - v)
        }
        Rectangle {
            id: grid
            Layout.preferredWidth: 220
            Layout.preferredHeight: 220
            radius: Theme.radiusSmall
            color: Theme.background
            border.color: Theme.border

            // quarters
            Repeater {
                model: 3
                Rectangle { required property int index; x: grid.width * (index + 1) / 4; width: 1; height: grid.height; color: Theme.border }
            }
            Repeater {
                model: 3
                Rectangle { required property int index; y: grid.height * (index + 1) / 4; height: 1; width: grid.width; color: Theme.border }
            }
            // the cross
            Rectangle { x: xy.px * grid.width; width: 1; height: grid.height; color: Theme.accent }
            Rectangle { y: xy.py * grid.height; height: 1; width: grid.width; color: Theme.accent }
            Rectangle {
                x: xy.px * grid.width - width / 2
                y: xy.py * grid.height - height / 2
                width: 16; height: 16; radius: 8
                color: Theme.accent
                border.color: "white"
                border.width: 2
            }

            MouseArea {
                anchors.fill: parent
                preventStealing: true
                property bool fine: false
                property point last
                onPressed: (ev) => {
                    // on the cross: it follows the mouse; elsewhere: a quarter of the move
                    let dx = ev.x - xy.px * grid.width, dy = ev.y - xy.py * grid.height
                    fine = Math.sqrt(dx * dx + dy * dy) > 14
                    last = Qt.point(ev.x, ev.y)
                    if (!fine)
                        Core.programmer.setPosition(ev.x / grid.width, ev.y / grid.height)
                }
                onPositionChanged: (ev) => {
                    if (fine)
                        Core.programmer.setPosition(xy.px + (ev.x - last.x) / grid.width / 4,
                                                    xy.py + (ev.y - last.y) / grid.height / 4)
                    else
                        Core.programmer.setPosition(ev.x / grid.width, ev.y / grid.height)
                    last = Qt.point(ev.x, ev.y)
                }
            }
        }
    }
    // pan
    RowLayout {
        Layout.leftMargin: tiltSlider.width + 6
        Layout.preferredWidth: 220
        HSlider {
            Layout.fillWidth: true
            from: 0; to: 1
            value: xy.px
            onMoved: Core.programmer.setPosition(value, xy.py)
        }
    }
    Text {
        Layout.leftMargin: tiltSlider.width + 6
        text: qsTr("Pan %1%   Tilt %2%").arg(Math.round(xy.px * 100)).arg(Math.round(xy.py * 100))
        color: Theme.textDim
        font.family: Theme.monoFamily
        font.pixelSize: Theme.fontSmall
    }
    AppButton {
        Layout.leftMargin: tiltSlider.width + 6
        text: qsTr("Centre")
        iconGlyph: Theme.iconTarget
        implicitHeight: 28
        onClicked: Core.programmer.centreBeam()
    }
}
