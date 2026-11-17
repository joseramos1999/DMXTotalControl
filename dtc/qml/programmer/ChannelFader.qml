/*
  DMX Total Control
  ChannelFader.qml

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
import DMXTotalControl.Backend

/** One merged programmer channel: { key, name, group, colour, value, active,
 *  capabilities, held } (held: of a protected special effect, recorded but
 *  never sent) */
Rectangle {
    id: root

    property var channel: ({})

    width: 74
    radius: Theme.radiusSmall
    color: channel.active ? Qt.alpha(Theme.warning, 0.10) : Theme.surfaceRaised
    border.color: channel.active ? Qt.alpha(Theme.warning, 0.6) : Theme.border

    Column {
        anchors { fill: parent; margins: 6 }
        spacing: 4

        Text {
            width: parent.width
            text: root.channel.name || ""
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            font.pixelSize: Theme.fontSmall
            font.weight: Font.DemiBold
            color: Theme.text
            ToolTip.text: text
            ToolTip.visible: nameMouse.containsMouse && truncated
            MouseArea { id: nameMouse; anchors.fill: parent; hoverEnabled: true }
        }
        Text {
            width: parent.width
            visible: !root.channel.held
            text: root.channel.group || ""
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            font.pixelSize: 9
            font.capitalization: Font.AllUppercase
            color: root.channel.colour ? root.channel.colour : Theme.textFaint
        }
        // DMX Total Control: a protected special effect: what it is given is
        // recorded in the scenes, it never goes out from here
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: !!root.channel.held
            spacing: 3
            IconLabel {
                anchors.verticalCenter: parent.verticalCenter
                icon: Theme.iconShield
                size: 9
                color: Theme.danger
            }
            Text {
                // (shortened in a narrow fader: the tooltip says it all)
                width: Math.min(implicitWidth, root.width - 12 - 12)
                text: qsTr("not sent")
                elide: Text.ElideRight
                font.pixelSize: 9
                font.capitalization: Font.AllUppercase
                font.weight: Font.DemiBold
                color: Theme.danger
            }
            HoverHandler { id: heldHover }
            ToolTip.text: qsTr("A protected special effect: recorded in the scenes, never sent from the programmer. It fires from its scenes, armed")
            ToolTip.visible: heldHover.hovered
            ToolTip.delay: 300
        }

        VFader {
            anchors.horizontalCenter: parent.horizontalCenter
            height: parent.height - 108
            value: root.channel.value !== undefined ? root.channel.value : 0
            fillColor: root.channel.colour ? root.channel.colour : (root.channel.active ? Theme.warning : Theme.accent)
            onUserMoved: (v) => Core.programmer.setChannelValue(root.channel.key, Math.round(v))
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: root.channel.value !== undefined ? root.channel.value : ""
            font.family: Theme.monoFamily
            font.pixelSize: Theme.fontSmall
            color: root.channel.active ? Theme.warning : Theme.textDim
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 2
            AppButton {
                implicitHeight: 24
                iconGlyph: Theme.iconList
                flat: true
                visible: root.channel.capabilities && root.channel.capabilities.length > 1
                ToolTip.text: qsTr("Presets")
                ToolTip.visible: hovered
                onClicked: capMenu.popup()
            }
            AppButton {
                implicitHeight: 24
                iconGlyph: Theme.iconClose
                flat: true
                enabled: !!root.channel.active
                ToolTip.text: qsTr("Release")
                ToolTip.visible: hovered
                onClicked: Core.programmer.releaseChannel(root.channel.key)
            }
        }
    }

    AppMenu {
        id: capMenu
        Instantiator {
            model: root.channel.capabilities || []
            delegate: MenuItem {
                required property var modelData
                text: modelData.name + "  (" + modelData.min + "–" + modelData.max + ")"
                onTriggered: Core.programmer.setChannelValue(root.channel.key, modelData.min)
            }
            onObjectAdded: (index, object) => capMenu.insertItem(index, object)
            onObjectRemoved: (index, object) => capMenu.removeItem(object)
        }
    }
}
