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

/** A flat slider, horizontal (or vertical: orientation Qt.Vertical, filled
 *  from the bottom). Set trackGradient to paint the track (e.g. a hue bar) */
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
    // (horizontal: the size of the style, as always; vertical: a column of 24)
    implicitHeight: horizontal ? 24 : Math.max(implicitBackgroundHeight + topInset + bottomInset,
                                               implicitHandleHeight + topPadding + bottomPadding)
    implicitWidth: horizontal ? Math.max(implicitBackgroundWidth + leftInset + rightInset,
                                         implicitHandleWidth + leftPadding + rightPadding) : 24

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
        x: control.horizontal ? control.leftPadding : control.leftPadding + control.availableWidth / 2 - width / 2
        y: control.horizontal ? control.topPadding + control.availableHeight / 2 - height / 2 : control.topPadding
        width: control.horizontal ? control.availableWidth : 6
        height: control.horizontal ? 6 : control.availableHeight
        radius: 3
        color: Theme.background
        border.color: control.activeFocus ? Theme.accent : Theme.border
        gradient: control.trackGradient

        Rectangle {
            visible: control.trackGradient === null
            // (vertical: up from the bottom)
            y: control.horizontal ? 0 : parent.height - height
            width: control.horizontal ? control.visualPosition * parent.width : parent.width
            height: control.horizontal ? parent.height : control.position * parent.height
            radius: 3
            color: control.fillColor
        }
    }

    handle: Rectangle {
        x: control.horizontal ? control.leftPadding + control.visualPosition * (control.availableWidth - width)
                              : control.leftPadding + control.availableWidth / 2 - width / 2
        y: control.horizontal ? control.topPadding + control.availableHeight / 2 - height / 2
                              : control.topPadding + control.visualPosition * (control.availableHeight - height)
        width: 16
        height: 16
        radius: 8
        color: control.pressed ? Theme.knobBorder : Theme.knob
        border.color: Theme.knobEdge
    }
}
