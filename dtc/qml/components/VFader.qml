/*
  DMX Total Control
  VFader.qml

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

/** A console style vertical fader. Emits moved(value) on user interaction only */
Slider {
    id: control

    property color fillColor: Theme.accent
    property bool showPercent: false

    signal userMoved(real value)

    orientation: Qt.Vertical
    from: 0
    to: 255
    stepSize: 1
    focusPolicy: Qt.StrongFocus
    implicitWidth: 44
    implicitHeight: 180
    padding: 0

    /** Arrow keys step: one DMX value */
    property real keyStep: stepSize > 0 ? stepSize : Math.abs(to - from) / 100
    /** Value set by a double click */
    property real resetValue: 0

    /** Set a value as if the user dragged there: keeps the value binding and emits moved() */
    function userSet(v) {
        v = Math.max(Math.min(from, to), Math.min(Math.max(from, to), v))
        if (v === value)
            return
        Core.setControlValue(control, v)
        moved()
    }

    onMoved: userMoved(value)

    Keys.onPressed: (event) => {
        switch (event.key) {
        case Qt.Key_Up: case Qt.Key_Right: userSet(value + keyStep); break
        case Qt.Key_Down: case Qt.Key_Left: userSet(value - keyStep); break
        case Qt.Key_PageUp: userSet(value + keyStep * 10); break
        case Qt.Key_PageDown: userSet(value - keyStep * 10); break
        default: return
        }
        event.accepted = true
    }

    TapHandler {
        onDoubleTapped: control.userSet(control.resetValue)
    }

    background: Rectangle {
        x: (control.width - width) / 2
        y: 0
        width: 30
        height: control.height
        radius: Theme.radiusSmall
        color: Theme.background
        border.color: control.activeFocus ? Theme.accent : Theme.border

        Rectangle {
            width: parent.width - 4
            x: 2
            height: Math.max(0, control.visualPosition >= 0 ? (1 - control.visualPosition) * (parent.height - 4) : 0)
            y: parent.height - 2 - height
            radius: Theme.radiusSmall - 1
            color: control.fillColor
            opacity: control.enabled ? 0.85 : 0.3
        }
    }

    handle: Rectangle {
        x: (control.width - width) / 2
        y: control.visualPosition * (control.height - height)
        width: 38
        height: 14
        radius: 3
        color: control.pressed ? "#ffffff" : "#d9dce3"
        border.color: "#00000060"

        Rectangle {
            width: parent.width - 10
            height: 2
            anchors.centerIn: parent
            color: "#00000080"
        }
    }
}
