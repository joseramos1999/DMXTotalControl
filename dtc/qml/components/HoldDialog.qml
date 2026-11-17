/*
  DMX Total Control
  HoldDialog.qml

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

/**
 * Show mode: what would end the show (closing the program, another show)
 * waits until a button is held down for a moment. Released earlier, Cancel
 * or Esc: nothing happens.
 */
Popup {
    id: dialog

    property string heading: ""
    property string body: ""
    property string actionText: ""
    /** What runs once the button was held */
    property var callback: null
    /** The button was held (the callback ran): it closed without a cancel */
    property bool passed: false
    /** It closed without the hold: Cancel, Esc */
    signal cancelled()

    /** Ask for the hold, then run $callback. The body and the text of the
     *  button may be left empty */
    function ask(heading, body, actionText, callback) {
        dialog.heading = heading
        dialog.body = body ? body : qsTr("Show mode is on: hold the button to confirm. Release it or press Esc to cancel.")
        dialog.actionText = actionText ? actionText : qsTr("Hold to continue")
        dialog.callback = callback ? callback : null
        dialog.passed = false
        holdButton.reset()
        open()
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 460
    modal: true
    padding: 18
    closePolicy: Popup.CloseOnEscape
    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.warning; border.width: 2 }

    // the keyboard holds the button too (Space or Enter)
    onOpened: holdButton.forceActiveFocus()
    onClosed: {
        holdButton.reset()
        if (!passed) {
            callback = null
            cancelled()
        }
    }

    contentItem: ColumnLayout {
        spacing: 12

        RowLayout {
            spacing: 10
            IconLabel { icon: Theme.iconShield; size: 18; color: Theme.readableOn(Theme.warning, Theme.surface) }
            Text {
                Layout.fillWidth: true
                text: dialog.heading
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
                wrapMode: Text.WordWrap
            }
        }
        Text {
            Layout.fillWidth: true
            text: dialog.body
            color: Theme.textDim
            wrapMode: Text.WordWrap
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 4
            spacing: 8
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Cancel"); onClicked: dialog.close() }
            HoldButton {
                id: holdButton
                text: dialog.actionText
                iconGlyph: Theme.iconShield
                primary: true
                accentColor: Theme.warning
                implicitHeight: 40
                holdMs: 1500
                onHeld: {
                    const action = dialog.callback
                    dialog.passed = true
                    dialog.callback = null
                    dialog.close()
                    if (action)
                        action()
                }
            }
        }
    }
}
