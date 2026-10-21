/*
  DMX Total Control
  LivePage.qml

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
 * The live page: zones side by side. Every zone is a column
 * of scenes; in an exclusive zone a click crossfades to the clicked scene.
 */
Item {
    id: page

    property bool mixerView: false

    signal editScene(int id)
    signal openScene(int id)

    ColumnLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 14

        // page header
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            // the subtitle shortens: the buttons always fit
            Column {
                Layout.fillWidth: true
                Layout.minimumWidth: 120
                Layout.preferredWidth: 200
                Text {
                    width: parent.width
                    elide: Text.ElideRight
                    text: qsTr("Live")
                    color: Theme.text
                    font.pixelSize: Theme.fontTitle
                    font.weight: Font.Bold
                }
                Text {
                    width: parent.width
                    elide: Text.ElideRight
                    text: qsTr("Groups run in parallel. In an exclusive group only one scene plays at a time.")
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                    HoverHandler { id: subtitleHover }
                    ToolTip.text: text
                    ToolTip.visible: truncated && subtitleHover.hovered
                    ToolTip.delay: 500
                }
            }
            AppButton {
                text: "◀"
                enabled: Core.zones.count > 0
                ToolTip.text: qsTr("Previous scene in every playing group")
                ToolTip.visible: hovered
                onClicked: Core.zones.previousAll()
                MapTarget { target: ({ target: "groupPrevious", zoneId: -1 }) }
            }
            AppButton {
                text: "▶"
                enabled: Core.zones.count > 0
                ToolTip.text: qsTr("Next scene in every playing group")
                ToolTip.visible: hovered
                onClicked: Core.zones.nextAll()
                MapTarget { target: ({ target: "groupNext", zoneId: -1 }) }
            }
            AppButton {
                text: Core.zones.paused ? qsTr("Resume") : qsTr("Pause")
                iconGlyph: Core.zones.paused ? Theme.iconPlay : Theme.iconPause
                highlighted: Core.zones.paused
                accentColor: Theme.warning
                onClicked: Core.zones.paused = !Core.zones.paused
                MapTarget { target: ({ target: "pauseAll" }) }
            }
            AppButton {
                text: qsTr("Release groups")
                iconGlyph: Theme.iconStop
                enabled: Core.zones.count > 0
                onClicked: Core.zones.stopAll()
                MapTarget { target: ({ target: "groupRelease", zoneId: -1 }) }
            }
            // live mixer: by fixture group, on top of the scenes
            AppButton {
                text: qsTr("Mixer")
                iconGlyph: Theme.iconSliders
                highlighted: page.mixerView
                accentColor: Core.mixer.active ? Theme.warning : Theme.accent
                ToolTip.text: Core.mixer.active ? qsTr("The mixer is changing the output") : qsTr("Live mixer by fixture group")
                ToolTip.visible: hovered
                onClicked: page.mixerView = !page.mixerView
                Rectangle {
                    visible: Core.mixer.active && !page.mixerView
                    anchors { right: parent.right; top: parent.top; margins: 4 }
                    width: 8; height: 8; radius: 4
                    color: Theme.warning
                }
            }
            AppButton {
                text: qsTr("New group")
                iconGlyph: Theme.iconPlus
                primary: true
                onClicked: Core.zones.addZone("")
            }
        }

        // zones
        Flickable {
            id: flick
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !page.mixerView
            clip: true
            contentWidth: zonesRow.width
            contentHeight: height
            flickableDirection: Flickable.HorizontalFlick
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }

            Row {
                id: zonesRow
                height: flick.height - 12
                spacing: 12

                Repeater {
                    model: Core.zones
                    delegate: ZoneColumn {
                        required property var zone
                        required property int index
                        zoneObject: zone
                        zoneIndex: index
                        height: zonesRow.height
                        // folded: a strip with its name, saved with the show
                        folded: zone.folded
                        canFold: true
                        width: folded ? foldedSize : 300
                        onFoldToggled: zone.folded = !zone.folded
                        dragProxy: dragItem
                        onEditScene: (id) => page.editScene(id)
                        onOpenScene: (id) => page.openScene(id)
                    }
                }

                // add zone placeholder
                Rectangle {
                    width: 200
                    height: zonesRow.height
                    radius: Theme.radius
                    color: addZoneArea.containsMouse ? Theme.surface : "transparent"
                    border.color: Theme.border
                    border.width: 1

                    Column {
                        anchors.centerIn: parent
                        spacing: 10
                        IconLabel { anchors.horizontalCenter: parent.horizontalCenter; icon: Theme.iconLayers; size: 28; color: Theme.textFaint }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: Core.zones.count === 0 ? qsTr("Create your first group") : qsTr("Add group")
                            color: Theme.textDim
                            font.weight: Font.DemiBold
                        }
                        Text {
                            width: 160
                            visible: Core.zones.count === 0
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            text: qsTr("e.g. Wash, Spots, Strobes, Effects")
                            color: Theme.textFaint
                            font.pixelSize: Theme.fontSmall
                        }
                    }
                    MouseArea {
                        id: addZoneArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Core.zones.addZone("")
                    }
                }
            }
        }

        // the live mixer: all the fixtures, then each fixture group
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: page.mixerView
            spacing: 12

            MixerRow {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: false
                Layout.preferredWidth: 200
                Layout.maximumWidth: 200
                spacing: 8
                AppButton {
                    Layout.preferredWidth: 200
                    text: qsTr("Reset the mixer")
                    iconGlyph: Theme.iconRefresh
                    enabled: Core.mixer.active
                    onClicked: Core.mixer.reset()
                }
                Text {
                    Layout.preferredWidth: 200
                    wrapMode: Text.WordWrap
                    text: qsTr("The mixer works on what goes out, on top of every scene and live edit. Make fixture groups in Patch to get a mixer for each. Ctrl+click keeps a flash on.")
                    color: Theme.textFaint
                    font.pixelSize: Theme.fontSmall
                }
            }
        }
    }

    // what is dragged: a scene or a group (Drag and DropArea)
    Rectangle {
        id: dragItem
        property int fromZone: -1
        property int functionId: -1
        property bool isZone: false
        property string label
        property bool dragging: false

        function drop() {
            if (dragging)
                Drag.drop()
            dragging = false
        }

        z: 100
        width: 160
        height: 44
        radius: Theme.radius
        visible: dragging
        color: Qt.alpha(Theme.accent, 0.35)
        border.color: Theme.accent
        Drag.active: dragging
        Drag.keys: isZone ? ["dtc-zone"] : ["dtc-scene"]
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2

        Text {
            anchors { fill: parent; margins: 8 }
            text: dragItem.label
            color: Theme.text
            font.weight: Font.DemiBold
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
