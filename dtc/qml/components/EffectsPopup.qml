/*
  DMX Total Control
  EffectsPopup.qml

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
 * The protected special effects, next to the button that arms them: armed
 * or not (and when they disarm by themselves), what fires now, what waits
 * for its source to let go before it can fire, the last fires of the
 * session (with what ended each one) and the log of the session.
 */
Popup {
    id: popup

    /** The SHOW locked (a kiosk): no way to the Patch */
    property bool locked: false
    /** «Each effect: Patch» */
    signal patchWanted()

    width: 440
    padding: 14
    // (inside the window, whatever the button)
    margins: 8
    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    /** A fire as the list shows it: «CO2 1 · 22:31:05 · 3,0 s · cut at its maximum time» */
    function fireText(fire) {
        return fire.name + "  ·  " + fire.time + "  ·  "
               + (fire.ms / 1000).toLocaleString(Qt.locale(), 'f', 1) + " s  ·  " + fire.reasonText
    }

    component Heading: Text {
        Layout.fillWidth: true
        color: Theme.textDim
        font.pixelSize: Theme.fontSmall
        font.weight: Font.Bold
        elide: Text.ElideRight
    }

    contentItem: ColumnLayout {
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            IconLabel {
                icon: Theme.iconFire
                size: 15
                color: Core.safety.armed ? Theme.readableOn(Theme.danger, Theme.surface) : Theme.textDim
            }
            Text {
                Layout.fillWidth: true
                text: qsTr("Special effects")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
                elide: Text.ElideRight
            }
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: Core.safety.armed
                  ? qsTr("Armed") + (Theme.disarmClock(Core.safety.disarmIn).length > 0 ? " · " + qsTr("they disarm by themselves in %1 without a fire").arg(Theme.disarmClock(Core.safety.disarmIn)) : "")
                  : qsTr("Disarmed: the protected special effects send 0")
            color: Core.safety.armed ? Theme.readableOn(Theme.danger, Theme.surface) : Theme.textDim
            font.weight: Core.safety.armed ? Font.Bold : Font.Normal
        }

        // what fires now
        Heading {
            visible: Core.safety.firing.length > 0
            text: qsTr("Firing now")
        }
        Text {
            Layout.fillWidth: true
            visible: Core.safety.firing.length > 0
            wrapMode: Text.WordWrap
            text: Theme.effectNames(Core.safety.firing)
            color: Theme.readableOn(Theme.warning, Theme.surface)
            font.weight: Font.Bold
        }

        // what a scene, a button or a fader still holds: it fires only once let go
        Heading {
            visible: Core.safety.waiting.length > 0
            text: qsTr("Waiting for a release")
        }
        Text {
            Layout.fillWidth: true
            visible: Core.safety.waiting.length > 0
            wrapMode: Text.WordWrap
            text: Theme.effectNames(Core.safety.waiting)
            color: Theme.text
        }
        Text {
            Layout.fillWidth: true
            visible: Core.safety.waiting.length > 0
            wrapMode: Text.WordWrap
            text: qsTr("Let go of its scene, its button or its fader (to 0): then it can fire")
            color: Theme.textFaint
            font.pixelSize: Theme.fontSmall
        }

        // the last fires of the session, newest first
        Heading { text: qsTr("Last fires") }
        Text {
            Layout.fillWidth: true
            visible: Core.safety.recentFires.length === 0
            text: qsTr("No fire in this session")
            color: Theme.textFaint
        }
        Repeater {
            model: Core.safety.recentFires
            delegate: Text {
                required property var modelData
                Layout.fillWidth: true
                text: popup.fireText(modelData)
                // (cut by its time or by a disarm: it says so in colour)
                color: modelData.reason === "released" ? Theme.text : Theme.readableOn(Theme.warning, Theme.surface)
                font.pixelSize: Theme.fontSmall
                elide: Text.ElideRight
            }
        }

        // (not in a locked SHOW: no file manager over the kiosk, nor setup;
        // the last fires stay)
        RowLayout {
            visible: !popup.locked
            Layout.fillWidth: true
            Layout.topMargin: 4
            spacing: 6
            AppButton {
                text: qsTr("Open the log")
                iconGlyph: Theme.iconFolderOpen
                flat: true
                ToolTip.text: qsTr("The folder of the logs: one file for each session, with every fire")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: Core.safety.openLog()
            }
            Item { Layout.fillWidth: true }
            AppButton {
                text: qsTr("Each effect: Patch")
                iconGlyph: Theme.iconLight
                flat: true
                ToolTip.text: qsTr("Protection, maximum time, minimum wait and safety channel of each fixture: Patch › Special effect")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: { popup.close(); popup.patchWanted() }
            }
        }
    }
}
