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

/** One merged programmer channel: { key, name, group, colour, value, active, capabilities } */
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
            text: root.channel.group || ""
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            font.pixelSize: 9
            font.capitalization: Font.AllUppercase
            color: root.channel.colour ? root.channel.colour : Theme.textFaint
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
