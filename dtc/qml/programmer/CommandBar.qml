/*
  DMX Total Control
  CommandBar.qml

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
 * The command line of the programmer, like a console:
 * "1 THRU 8 @ FULL", "GROUP 2 COL 3"... from the keyboard or the keypad,
 * and the console tools: one fixture at a time, highlight, locate.
 */
Rectangle {
    id: bar

    /** Where we are in the history with the arrows (-1: the line being typed) */
    property int historyIndex: -1
    property string typed: ""

    function focusLine() {
        commandField.forceActiveFocus()
    }

    function openKeypad() {
        keypad.open()
    }

    /** Text typed somewhere else on the page goes to the line */
    function insertText(text) {
        commandField.forceActiveFocus()
        commandField.insert(commandField.cursorPosition, text)
    }

    function run() {
        const text = commandField.text.trim()
        if (text.length === 0)
            return
        if (Core.commands.execute(text))
            commandField.clear()
        // it stopped part way: what was done is not run again, only the
        // words left to correct stay on the line
        else if (Core.commands.rest.length > 0)
            commandField.text = Core.commands.rest
        historyIndex = -1
    }

    /** A key of the keypad: words and symbols go between spaces */
    function key(k) {
        let t = commandField.text
        if (k === "ENTER") {
            run()
            return
        }
        if (k === "CLR") {
            commandField.clear()
        } else if (k === "BACKSPACE") {
            t = t.replace(/\s+$/, "")
            // a number loses a digit, a word goes whole
            t = /[0-9]$/.test(t) ? t.slice(0, -1) : t.replace(/\S+$/, "")
            commandField.text = t
        } else if (k === ".") {
            // a point cue: 2.5
            commandField.text = t.replace(/\s+$/, "") + "."
        } else if (/^[0-9]$/.test(k)) {
            commandField.text = t + k
        } else {
            commandField.text = (t.length > 0 && !t.endsWith(" ") ? t + " " : t) + k + " "
        }
        commandField.cursorPosition = commandField.text.length
        commandField.forceActiveFocus()
    }

    function browseHistory(step) {
        const history = Core.commands.history
        if (history.length === 0)
            return
        if (historyIndex < 0) {
            if (step > 0)
                return
            typed = commandField.text
            historyIndex = history.length
        }
        historyIndex = Math.max(0, Math.min(history.length, historyIndex + step))
        commandField.text = historyIndex >= history.length ? typed : history[historyIndex]
        if (historyIndex >= history.length)
            historyIndex = -1
        commandField.cursorPosition = commandField.text.length
    }

    readonly property var stepping: Core.programmer.stepping

    implicitHeight: 44
    radius: Theme.radius
    color: Theme.surface
    border.color: commandField.activeFocus ? Theme.accent : Theme.border

    RowLayout {
        anchors { fill: parent; leftMargin: 12; rightMargin: 6 }
        spacing: 8

        IconLabel {
            icon: Theme.iconTerminal
            size: 13
            color: commandField.activeFocus ? Theme.accent : Theme.textDim
        }
        TextField {
            id: commandField
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            placeholderText: qsTr("1 THRU 8 @ FULL")
            placeholderTextColor: Theme.textFaint
            font.family: Theme.monoFamily
            font.pixelSize: Theme.fontNormal + 1
            font.capitalization: Font.AllUppercase
            color: Theme.text
            selectByMouse: true
            verticalAlignment: TextInput.AlignVCenter
            background: Item {}
            // Enter runs the line here, once: TextInput lets the key go on to
            // the page after accepted(), and the page ran the line again (a
            // GO twice)
            Keys.onReturnPressed: (event) => { event.accepted = true; bar.run() }
            Keys.onEnterPressed: (event) => { event.accepted = true; bar.run() }
            onTextEdited: bar.historyIndex = -1
            Keys.onUpPressed: bar.browseHistory(-1)
            Keys.onDownPressed: bar.browseHistory(1)
            // Esc empties the line; with nothing typed it is still "stop all"
            Keys.onShortcutOverride: (event) => event.accepted = event.key === Qt.Key_Escape && text.length > 0
            Keys.onEscapePressed: clear()

            ToolTip.visible: hovered && text.length === 0
            ToolTip.delay: 600
            ToolTip.text: qsTr("Type a command and press Enter:\n1 THRU 8 · 1 + 3 - 2 · GROUP 2 · ALL\n@ 50 · @ 5 (= 50 %) · @ FULL · @ 0 THRU 100 · @ + 10\nPOS 1 · COL 2 · BEAM 3 · INT 4\nLOCATE · HIGHLIGHT · NEXT · PREV · REL · CLEAR\nCuelists: GO · BACK · GO CUE 3 · CUE 3 · REC · REC CUE 2.5 · UPD · DELETE CUE 4 · PB 3\n↑ ↓: the last commands")
        }
        Text {
            Layout.fillWidth: true
            text: Core.commands.message
            color: Core.commands.error ? Theme.danger : Theme.success
            font.pixelSize: Theme.fontSmall
            elide: Text.ElideRight
        }

        // one fixture of the selection at a time (NEXT, PREV)
        AppButton {
            iconGlyph: Theme.iconStepBackward
            flat: true
            implicitHeight: 30
            ToolTip.text: qsTr("Previous fixture in the selection, one at a time (PREV)")
            ToolTip.visible: hovered
            ToolTip.delay: 500
            onClicked: Core.programmer.previousFixture()
        }
        Text {
            visible: bar.stepping.active === true
            text: (bar.stepping.index + 1) + "/" + bar.stepping.count
            color: Theme.accent
            font.family: Theme.monoFamily
            font.pixelSize: Theme.fontSmall
        }
        AppButton {
            iconGlyph: Theme.iconStepForward
            flat: true
            implicitHeight: 30
            ToolTip.text: qsTr("Next fixture in the selection, one at a time (NEXT)")
            ToolTip.visible: hovered
            ToolTip.delay: 500
            onClicked: Core.programmer.nextFixture()
        }
        AppButton {
            visible: bar.stepping.active === true
            text: qsTr("All")
            implicitHeight: 30
            ToolTip.text: qsTr("Back to the whole selection (ALL)")
            ToolTip.visible: hovered
            ToolTip.delay: 500
            onClicked: Core.programmer.allFixtures()
        }
        Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 24; color: Theme.border }
        AppButton {
            text: qsTr("Highlight")
            iconGlyph: Theme.iconLightbulb
            implicitHeight: 30
            highlighted: Core.programmer.highlight
            accentColor: Theme.warning
            ToolTip.text: qsTr("The selection at full in open white, over everything, to see which fixtures they are. It isn't recorded")
            ToolTip.visible: hovered
            ToolTip.delay: 500
            onClicked: Core.programmer.highlight = !Core.programmer.highlight
        }
        AppButton {
            text: qsTr("Locate")
            iconGlyph: Theme.iconLocate
            implicitHeight: 30
            enabled: Core.programmer.selection.length > 0
            ToolTip.text: qsTr("The selection at full, open white, centred and with an open beam, in the programmer")
            ToolTip.visible: hovered
            ToolTip.delay: 500
            onClicked: Core.programmer.locate()
        }
        AppButton {
            id: keypadButton
            iconGlyph: Theme.iconKeyboard
            implicitHeight: 30
            highlighted: keypad.visible
            ToolTip.text: qsTr("Keypad")
            ToolTip.visible: hovered
            ToolTip.delay: 500
            onClicked: keypad.visible ? keypad.close() : keypad.open()
        }
    }

    // the keypad of a console, for touch screens: it stays open while the fixtures are picked
    Popup {
        id: keypad
        x: bar.width - width
        y: bar.height + 6
        padding: 10
        modal: false
        closePolicy: Popup.CloseOnEscape
        background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

        readonly property var keys: [
            "THRU", "@", "FULL", "BACKSPACE",
            "7", "8", "9", "+",
            "4", "5", "6", "-",
            "1", "2", "3", "GROUP",
            "0", "OUT", "CLR", "ENTER",
            "INT", "POS", "COL", "BEAM",
            "NEXT", "PREV", "ALL", "LOCATE",
            // cuelists: act on the selected cuelist
            "CUE", ".", "REC", "UPD",
            "GO", "BACK", "DELETE", "PB"
        ]

        contentItem: Grid {
            columns: 4
            spacing: 6
            Repeater {
                model: keypad.keys
                AppButton {
                    required property string modelData
                    width: 64
                    implicitHeight: 44
                    text: modelData === "BACKSPACE" ? "" : modelData
                    iconGlyph: modelData === "BACKSPACE" ? "" : ""
                    primary: modelData === "ENTER"
                    font.family: /^[0-9]$/.test(modelData) ? Theme.monoFamily : Theme.fontFamily
                    onClicked: bar.key(modelData)
                }
            }
        }
    }
}
