/*
  DMX Total Control
  HSlider.qml

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

/** A flat horizontal slider. Set trackGradient to paint the track (e.g. a hue bar) */
Slider {
    id: control

    property color fillColor: Theme.accent
    property Gradient trackGradient: null

    /** Arrow keys step: one value, or 1% on continuous ranges */
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

    focusPolicy: Qt.StrongFocus
    implicitHeight: 24

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
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.availableWidth
        height: 6
        radius: 3
        color: Theme.background
        border.color: control.activeFocus ? Theme.accent : Theme.border
        gradient: control.trackGradient

        Rectangle {
            visible: control.trackGradient === null
            width: control.visualPosition * parent.width
            height: parent.height
            radius: 3
            color: control.fillColor
        }
    }

    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: 16
        height: 16
        radius: 8
        color: control.pressed ? Theme.knobBorder : Theme.knob
        border.color: Theme.knobEdge
    }
}
