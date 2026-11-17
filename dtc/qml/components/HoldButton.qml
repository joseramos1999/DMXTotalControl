/*
  DMX Total Control
  HoldButton.qml

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

/**
 * A button that acts only once it has been held down for a while (holdMs):
 * with the pointer, or with Space or Enter while it has the keyboard focus
 * (the repeats of a held key don't count). Released earlier, nothing
 * happens and it starts again. It fills from the left while it is held.
 * A click does nothing.
 */
AppButton {
    id: control

    /** How long it must be held */
    property int holdMs: 1500
    /** It was held long enough: once per hold */
    signal held()

    /** 0 to 1 while it is held */
    readonly property real progress: filled
    /** Held now, by the pointer or the keyboard */
    readonly property bool holding: pressed || keyDown

    property bool keyDown: false
    property bool done: false
    property real filled: 0

    /** Back as it was (a dialog opened again) */
    function reset() {
        keyDown = false
        done = false
        holdTimer.stop()
        filled = 0
    }

    focusPolicy: Qt.StrongFocus

    onHoldingChanged: {
        if (holding) {
            done = false
            filled = 0
            holdTimer.restart()
        } else {
            holdTimer.stop()
            if (!done)
                filled = 0
        }
    }
    onActiveFocusChanged: if (!activeFocus) keyDown = false

    function isHoldKey(key) {
        return key === Qt.Key_Space || key === Qt.Key_Return || key === Qt.Key_Enter
    }
    // the keys are its own: no shortcut takes them (Space is the GO of the playbacks)
    Keys.onShortcutOverride: (event) => event.accepted = isHoldKey(event.key)
    Keys.onPressed: (event) => {
        if (!isHoldKey(event.key))
            return
        if (!event.isAutoRepeat)
            control.keyDown = true
        event.accepted = true
    }
    Keys.onReleased: (event) => {
        if (!isHoldKey(event.key))
            return
        if (!event.isAutoRepeat)
            control.keyDown = false
        event.accepted = true
    }

    Timer {
        id: holdTimer
        interval: control.holdMs
        onTriggered: {
            control.done = true
            control.filled = 1
            control.held()
        }
    }
    NumberAnimation on filled {
        running: control.holding && !control.done
        from: 0; to: 1
        duration: control.holdMs
    }

    // how far the hold has come: the button fills from the left, with a
    // strip along its bottom edge
    Rectangle {
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
        width: parent.width * control.filled
        radius: Theme.radiusSmall
        visible: control.filled > 0
        color: Qt.alpha(control.lit ? control.litText : Theme.text, 0.22)
    }
    Rectangle {
        anchors { left: parent.left; bottom: parent.bottom }
        height: 3
        width: parent.width * control.filled
        radius: 1
        visible: control.filled > 0
        color: control.lit ? control.litText : Theme.text
    }
}
