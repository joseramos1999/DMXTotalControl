/*
  DMX Total Control
  MixerStrip.qml

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
 * The live mixer of a fixture group (or of all the fixtures):
 * dimmer, hue, saturation, colour reset, strobe, Full, Blackout,
 * Flash and their exclusive versions. Flash buttons work while pressed;
 * Ctrl+click keeps them on.
 */
Rectangle {
    id: strip

    property var mixer: ({})
    readonly property int mid: mixer.id !== undefined ? mixer.id : -1

    function set(key, value) { Core.mixer.set(mid, key, value) }
    // learn mode: the mapping target of each control
    readonly property var mapTargets: ({ full: "mixerFull", blackout: "mixerBlackout", flash: "mixerFlash",
                                         flashExclusive: "mixerFlashExclusive", fullExclusive: "mixerFullExclusive",
                                         strobe: "mixerStrobe" })

    width: 150
    radius: Theme.radius
    color: Theme.surface
    border.color: mixer.flash || mixer.flashExclusive || mixer.full || mixer.fullExclusive ? Theme.warning
                : mixer.blackout ? Theme.danger : Theme.border

    // a momentary button that Ctrl+click latches
    component FlashButton: AppButton {
        property string key
        readonly property bool on: !!strip.mixer[key]
        property bool latched: false
        Layout.fillWidth: true
        implicitHeight: 30
        highlighted: on
        accentColor: Theme.warning
        onPressed: {
            if (latched) { latched = false; strip.set(key, false); return }
            strip.set(key, true)
        }
        onReleased: if (!latched) strip.set(key, false)
        MapTarget { target: ({ target: strip.mapTargets[parent.key], mixerId: strip.mid }) }
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            // Ctrl+click: stays on until the next click
            enabled: true
            propagateComposedEvents: true
            onPressed: (ev) => {
                if (ev.modifiers & Qt.ControlModifier) {
                    parent.latched = !parent.latched
                    strip.set(parent.key, parent.latched)
                    ev.accepted = true
                } else {
                    ev.accepted = false
                }
            }
        }
    }
    component ToggleButton: AppButton {
        property string key
        Layout.fillWidth: true
        implicitHeight: 30
        highlighted: !!strip.mixer[key]
        onClicked: strip.set(key, !strip.mixer[key])
        MapTarget { target: ({ target: strip.mapTargets[parent.key], mixerId: strip.mid }) }
    }

    ColumnLayout {
        anchors { fill: parent; margins: 8 }
        spacing: 6

        Text {
            Layout.fillWidth: true
            text: strip.mixer.name || ""
            color: Theme.text
            font.weight: Font.Bold
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
        }
        Text {
            Layout.fillWidth: true
            text: qsTr("%n fixture(s)", "", strip.mixer.fixtures || 0)
            color: Theme.textFaint
            font.pixelSize: Theme.fontSmall
            horizontalAlignment: Text.AlignHCenter
        }

        // dimmer
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 6
            VFader {
                Layout.fillHeight: true
                Layout.minimumHeight: 90
                from: 0; to: 1; stepSize: 0
                fillColor: strip.mixer.colour ? Qt.hsva(strip.mixer.hue, strip.mixer.saturation, 1, 1) : "#e7e9ef"
                value: strip.mixer.dimmer !== undefined ? strip.mixer.dimmer : 1
                resetValue: 0
                onUserMoved: (v) => strip.set("dimmer", v)
                MapTarget { target: ({ target: "mixerDimmer", mixerId: strip.mid }) }
            }
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 4
                Text {
                    text: Math.round((strip.mixer.dimmer !== undefined ? strip.mixer.dimmer : 1) * 100) + "%"
                    color: Theme.text
                    font.family: Theme.monoFamily
                }
                Item { Layout.fillHeight: true }
                ToggleButton { key: "full"; text: qsTr("Full") }
                ToggleButton { key: "blackout"; text: qsTr("Blackout"); accentColor: Theme.danger }
            }
        }

        // colour
        Text { text: qsTr("Hue"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
        HSlider {
            Layout.fillWidth: true
            from: 0; to: 1
            value: strip.mixer.hue || 0
            trackGradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "#ff0000" }
                GradientStop { position: 0.17; color: "#ffff00" }
                GradientStop { position: 0.33; color: "#00ff00" }
                GradientStop { position: 0.5; color: "#00ffff" }
                GradientStop { position: 0.67; color: "#0000ff" }
                GradientStop { position: 0.83; color: "#ff00ff" }
                GradientStop { position: 1.0; color: "#ff0000" }
            }
            onMoved: strip.set("hue", value)
            MapTarget { target: ({ target: "mixerHue", mixerId: strip.mid }) }
        }
        Text { text: qsTr("Saturation"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
        HSlider {
            Layout.fillWidth: true
            from: 0; to: 1
            value: strip.mixer.saturation !== undefined ? strip.mixer.saturation : 1
            fillColor: Qt.hsva(strip.mixer.hue || 0, 1, 1, 1)
            onMoved: strip.set("saturation", value)
            MapTarget { target: ({ target: "mixerSaturation", mixerId: strip.mid }) }
        }
        AppButton {
            Layout.fillWidth: true
            implicitHeight: 28
            text: qsTr("Reset colour")
            enabled: !!strip.mixer.colour
            onClicked: Core.mixer.resetColour(strip.mid)
        }

        // strobe
        RowLayout {
            Layout.fillWidth: true
            ToggleButton { key: "strobe"; text: qsTr("Strobe"); accentColor: Theme.warning }
        }
        HSlider {
            Layout.fillWidth: true
            from: 0.5; to: 20; stepSize: 0.5
            value: strip.mixer.strobeHz || 8
            resetValue: 8
            onMoved: strip.set("strobeHz", value)
            MapTarget { target: ({ target: "mixerStrobeHz", mixerId: strip.mid }) }
        }
        Text {
            text: qsTr("%1 Hz").arg((strip.mixer.strobeHz || 8).toFixed(1))
            color: Theme.textDim
            font.family: Theme.monoFamily
            font.pixelSize: Theme.fontSmall
        }

        // flashes
        FlashButton { key: "flash"; text: qsTr("Flash") }
        FlashButton { key: "flashExclusive"; text: qsTr("Flash excl.") }
        ToggleButton { key: "fullExclusive"; text: qsTr("Full excl."); accentColor: Theme.warning }
    }
}
