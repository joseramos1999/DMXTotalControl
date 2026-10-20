/*
  DMX Total Control
  DmxMonitor.qml

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

/** Live DMX levels of a universe, as they leave the application */
Item {
    id: monitor

    property int universe: 0
    property var values: []
    property bool percent: false

    // fixture name by channel, to show who uses each address
    readonly property var owners: {
        let map = {}
        for (let f of Core.patch.fixtures) {
            if (f.universe !== universe)
                continue
            for (let c = 0; c < f.channels; c++)
                map[f.address - 1 + c] = f.name
        }
        return map
    }

    Timer {
        interval: 100
        repeat: true
        running: monitor.visible
        triggeredOnStart: true
        onTriggered: monitor.values = Core.io.channelValues(monitor.universe)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            Text { text: qsTr("Universe"); color: Theme.textDim }
            ComboBox {
                Layout.preferredWidth: 220
                model: Core.io.universes.map(u => (u.index + 1) + " — " + u.name)
                currentIndex: monitor.universe
                onActivated: (i) => monitor.universe = i
            }
            Item { Layout.fillWidth: true }
            CheckBox {
                text: qsTr("Show as %")
                checked: monitor.percent
                onToggled: monitor.percent = checked
            }
            Text {
                text: qsTr("Hover a channel to see its fixture")
                color: Theme.textFaint
                font.pixelSize: Theme.fontSmall
            }
        }

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentHeight: grid.height
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}

            Grid {
                id: grid
                width: parent.width
                columns: 32
                spacing: 2
                readonly property real cell: (width - 31 * spacing) / 32

                Repeater {
                    model: 512
                    delegate: Rectangle {
                        id: cellRect
                        required property int index
                        readonly property int value: monitor.values.length > index ? monitor.values[index] : 0
                        readonly property string owner: monitor.owners[index] || ""
                        // the colour it shows, for its text
                        readonly property color shown: Theme.fill(Theme.mix(Theme.background, Theme.accent, 0.08 + 0.8 * value / 255))

                        width: grid.cell
                        height: 36
                        radius: 3
                        color: shown
                        border.color: owner.length ? Theme.borderStrong : "transparent"

                        Text {
                            anchors { top: parent.top; left: parent.left; margins: 2 }
                            text: cellRect.index + 1
                            font.pixelSize: 8
                            color: Theme.readableOn(Theme.textFaint, cellRect.shown)
                        }
                        Text {
                            anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 2 }
                            text: monitor.percent ? Math.round(cellRect.value * 100 / 255) : cellRect.value
                            font.family: Theme.monoFamily
                            font.pixelSize: 10
                            color: cellRect.value > 0 ? Theme.textOver(cellRect.shown) : Theme.textFaint
                        }

                        MouseArea { id: cellMouse; anchors.fill: parent; hoverEnabled: true }
                        ToolTip.visible: cellMouse.containsMouse && owner.length > 0
                        ToolTip.text: qsTr("%1 — channel %2").arg(owner).arg(index + 1)
                    }
                }
            }
        }
    }
}
