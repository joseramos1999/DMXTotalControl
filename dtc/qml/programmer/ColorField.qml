/*
  DMX Total Control
  ColorField.qml

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
 * The colour picker of the features: hue across, saturation down.
 * Drag the cross, or anywhere else for a fine move. Favourites below: "+"
 * saves the current colour, a right click removes one.
 */
ColumnLayout {
    id: picker

    property real hue: 0
    property real saturation: 1
    readonly property color current: Qt.hsva(hue, saturation, 1, 1)

    function apply(h, s) {
        hue = Math.max(0, Math.min(1, h))
        saturation = Math.max(0, Math.min(1, s))
        Core.programmer.setColor(current)
    }

    spacing: 6

    Rectangle {
        id: field
        Layout.preferredWidth: 240
        Layout.preferredHeight: 160
        radius: Theme.radiusSmall
        border.color: Theme.border
        opacity: Theme.lightLevel
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "#ff0000" }
            GradientStop { position: 0.17; color: "#ffff00" }
            GradientStop { position: 0.33; color: "#00ff00" }
            GradientStop { position: 0.5; color: "#00ffff" }
            GradientStop { position: 0.67; color: "#0000ff" }
            GradientStop { position: 0.83; color: "#ff00ff" }
            GradientStop { position: 1.0; color: "#ff0000" }
        }
        // less saturated downwards
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#00ffffff" }
                GradientStop { position: 1.0; color: "#ffffffff" }
            }
        }
        Rectangle { x: picker.hue * field.width; width: 1; height: field.height; color: "#000000" }
        Rectangle { y: (1 - picker.saturation) * field.height; height: 1; width: field.width; color: "#000000" }
        Rectangle {
            x: picker.hue * field.width - 8
            y: (1 - picker.saturation) * field.height - 8
            width: 16; height: 16; radius: 8
            color: picker.current
            border.color: "white"
            border.width: 2
        }
        MouseArea {
            anchors.fill: parent
            preventStealing: true
            property bool fine: false
            property point last
            onPressed: (ev) => {
                let dx = ev.x - picker.hue * field.width, dy = ev.y - (1 - picker.saturation) * field.height
                fine = Math.sqrt(dx * dx + dy * dy) > 14
                last = Qt.point(ev.x, ev.y)
                if (!fine)
                    picker.apply(ev.x / field.width, 1 - ev.y / field.height)
            }
            onPositionChanged: (ev) => {
                if (fine)
                    picker.apply(picker.hue + (ev.x - last.x) / field.width / 4,
                                 picker.saturation - (ev.y - last.y) / field.height / 4)
                else
                    picker.apply(ev.x / field.width, 1 - ev.y / field.height)
                last = Qt.point(ev.x, ev.y)
            }
        }
    }

    // favourites
    Text { text: qsTr("Favourites"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
    Flow {
        Layout.preferredWidth: 240
        spacing: 4
        Repeater {
            model: Core.settings.favoriteColors
            Rectangle {
                required property string modelData
                width: 26; height: 26; radius: 5
                color: modelData
                opacity: Theme.lightLevel
                border.color: Theme.borderStrong
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onClicked: (ev) => {
                        if (ev.button === Qt.RightButton) {
                            Core.settings.removeFavoriteColor(parent.modelData)
                        } else {
                            let c = Qt.color(parent.modelData)
                            picker.hue = Math.max(0, c.hsvHue)
                            picker.saturation = c.hsvSaturation
                            Core.programmer.setColor(c)
                        }
                    }
                }
            }
        }
        Rectangle {
            width: 26; height: 26; radius: 5
            color: "transparent"
            border.color: Theme.border
            Text { anchors.centerIn: parent; text: "+"; color: Theme.textDim; font.pixelSize: 16 }
            MouseArea {
                anchors.fill: parent
                onClicked: Core.settings.addFavoriteColor(picker.current.toString())
            }
            ToolTip.text: qsTr("Save the current colour (right click a favourite to remove it)")
            ToolTip.visible: favHover.hovered
            HoverHandler { id: favHover }
        }
    }
}
