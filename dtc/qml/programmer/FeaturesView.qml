/*
  DMX Total Control
  FeaturesView.qml

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
 * The features of the selection, by tabs: Dimmer, Color,
 * Position, Gobo, Beam, Focus, Others. Each channel shows its presets as
 * buttons (with their colour or gobo) and a fader; the Color tab has a
 * colour picker with favourites, the Position tab an XY grid and the fan of
 * the position (pan and tilt only: their rotations and speeds are in Others).
 */
ColumnLayout {
    id: view

    /** From the programmer page: channel keys, and the channels by key (kept stable while dragging) */
    property var channelKeys: []
    property var channelMap: ({})
    property string tab: "dimmer"

    readonly property var tabs: [
        { id: "dimmer", name: qsTr("Dimmer") },
        { id: "color", name: qsTr("Colour") },
        { id: "position", name: qsTr("Position") },
        { id: "gobo", name: qsTr("Gobo") },
        { id: "beam", name: qsTr("Beam") },
        { id: "focus", name: qsTr("Focus") },
        { id: "other", name: qsTr("Others") }
    ]

    function keysOf(tabId) {
        return channelKeys.filter(k => {
            let c = channelMap[k]
            return c && c.tab === tabId && !c.fine &&
                   // pan and tilt live in the XY grid
                   !(tabId === "position" && (c.group === "Pan" || c.group === "Tilt"))
        })
    }
    function hasTab(tabId) {
        if (tabId === "position" && Core.programmer.position.available)
            return true
        return channelKeys.some(k => channelMap[k] && channelMap[k].tab === tabId && !channelMap[k].fine)
    }

    // rebuilt only when the list changes, or the cards would drop a drag
    property var tabKeys: []
    function refreshKeys() {
        let keys = keysOf(tab)
        if (keys.join("|") !== tabKeys.join("|"))
            tabKeys = keys
    }
    onTabChanged: refreshKeys()
    onChannelKeysChanged: refreshKeys()
    onChannelMapChanged: refreshKeys()

    spacing: 8

    // tabs
    Row {
        spacing: 2
        Repeater {
            model: view.tabs
            AppButton {
                required property var modelData
                text: modelData.name
                implicitHeight: 28
                flat: !highlighted
                highlighted: view.tab === modelData.id
                enabled: view.hasTab(modelData.id)
                onClicked: view.tab = modelData.id
            }
        }
    }

    Flickable {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        contentWidth: content.width
        contentHeight: content.height
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {}
        ScrollBar.horizontal: ScrollBar {}

        Row {
            id: content
            spacing: 14

            // the colour picker and the XY grid come first in their tabs
            ColorField {
                visible: view.tab === "color"
            }
            XYGrid {
                visible: view.tab === "position"
            }

            Flow {
                // (the position is the grid and its fan alone)
                visible: view.tab !== "position" || view.tabKeys.length > 0
                width: Math.max(260, view.width - (view.tab === "color" ? 300 : 0))
                spacing: 8
                Repeater {
                    model: view.tabKeys
                    FeatureCard {
                        required property string modelData
                        channel: view.channelMap[modelData] || ({})
                    }
                }
                Text {
                    visible: view.tabKeys.length === 0 && view.tab !== "color" && view.tab !== "position"
                    text: Core.programmer.selection.length === 0
                          ? qsTr("Select one or more fixtures to see their features")
                          : qsTr("The selected fixtures have nothing in this tab")
                    color: Theme.textDim
                }
            }
        }
    }

    // one channel: its fader (0-100%) and its presets
    component FeatureCard: Rectangle {
        id: card
        property var channel: ({})
        readonly property var caps: channel.capabilities || []
        readonly property int value: channel.value !== undefined ? channel.value : 0

        function presetValue(cap) {
            // a short range is one preset: its middle; a long one starts at its beginning
            return cap.max - cap.min <= 32 ? Math.round((cap.min + cap.max) / 2) : cap.min
        }

        width: 250
        height: cardColumn.implicitHeight + 16
        radius: Theme.radiusSmall
        color: channel.active ? Qt.alpha(Theme.warning, 0.08) : Theme.surfaceRaised
        border.color: channel.active ? Qt.alpha(Theme.warning, 0.6) : Theme.border

        ColumnLayout {
            id: cardColumn
            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 8 }
            spacing: 6

            RowLayout {
                Layout.fillWidth: true
                Text {
                    Layout.fillWidth: true
                    text: card.channel.name || ""
                    color: Theme.text
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }
                Text {
                    text: Math.round(card.value * 100 / 255) + "%"
                    color: card.channel.active ? Theme.warning : Theme.textDim
                    font.family: Theme.monoFamily
                    font.pixelSize: Theme.fontSmall
                }
                AppButton {
                    implicitHeight: 22
                    implicitWidth: 24
                    flat: true
                    iconGlyph: Theme.iconClose
                    enabled: !!card.channel.active
                    ToolTip.text: qsTr("Release")
                    ToolTip.visible: hovered
                    onClicked: Core.programmer.releaseChannel(card.channel.key)
                }
            }
            HSlider {
                Layout.fillWidth: true
                from: 0; to: 255; stepSize: 1
                fillColor: card.channel.colour ? card.channel.colour : (card.channel.active ? Theme.warning : Theme.accent)
                value: card.value
                onMoved: Core.programmer.setChannelValue(card.channel.key, Math.round(value))
            }
            // the presets (colours, gobos, strobe...)
            Flow {
                Layout.fillWidth: true
                visible: card.caps.length > 1
                spacing: 4
                Repeater {
                    model: card.caps.length > 1 ? card.caps : []
                    Rectangle {
                        id: preset
                        required property var modelData
                        readonly property bool current: card.value >= modelData.min && card.value <= modelData.max
                        readonly property bool hasPicture: !!modelData.image || !!modelData.color
                        width: hasPicture ? 44 : Math.min(card.width - 16, presetText.implicitWidth + 14)
                        height: hasPicture ? 44 : 26
                        radius: 4
                        color: presetMouse.containsMouse ? Theme.surfaceHover : Theme.surface
                        border.color: current ? Theme.accent : Theme.border
                        border.width: current ? 2 : 1

                        // colour wheel: the colour (two halves for split colours)
                        Row {
                            anchors { fill: parent; margins: 4 }
                            visible: !!preset.modelData.color && !preset.modelData.image
                            Rectangle { width: parent.width / (preset.modelData.color2 ? 2 : 1); height: parent.height; color: preset.modelData.color || "transparent"; radius: 3 }
                            Rectangle { visible: !!preset.modelData.color2; width: parent.width / 2; height: parent.height; color: preset.modelData.color2 || "transparent"; radius: 3 }
                        }
                        // gobo: its picture
                        Image {
                            anchors { fill: parent; margins: 4 }
                            visible: !!preset.modelData.image
                            source: preset.modelData.image || ""
                            fillMode: Image.PreserveAspectFit
                            sourceSize: Qt.size(64, 64)
                            asynchronous: true
                        }
                        Text {
                            id: presetText
                            anchors.centerIn: parent
                            visible: !preset.hasPicture
                            width: Math.min(implicitWidth, card.width - 30)
                            text: preset.modelData.name
                            color: Theme.text
                            font.pixelSize: Theme.fontSmall
                            elide: Text.ElideRight
                        }
                        MouseArea {
                            id: presetMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: Core.programmer.setChannelValue(card.channel.key, card.presetValue(preset.modelData))
                        }
                        ToolTip.text: modelData.name + "  (" + modelData.min + "–" + modelData.max + ")"
                        ToolTip.visible: presetMouse.containsMouse && preset.hasPicture
                        ToolTip.delay: 300
                    }
                }
            }
        }
    }
}
