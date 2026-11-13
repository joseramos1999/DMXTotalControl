/*
  DMX Total Control
  CueTimeField.qml

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
 * A time of a cue in a cell, in seconds: typed, or the mouse wheel once
 * clicked (0.1 s, with Shift 1 s). $allowSame: empty or "=" is the time it
 * takes after another one (the fade, the cue's): committed(-2). $clock: a
 * time on the clock of a Timecode cuelist, hh:mm:ss.cc ("1:23.4" and
 * "83.4" too); empty is none: committed(-1)
 */
TextField {
    id: field
    property int ms: 0
    property bool same: false
    property bool allowSame: false
    /** The value the wheel starts from when it is "=" */
    property int sameBase: ms
    property bool clock: false
    /** The latest time a clock time can be (Cuelist::maxCueTime) */
    readonly property int clockMax: 359999999
    signal committed(int ms)
    /** Ctrl+Z or Ctrl+Y with nothing typed to undo (the wheel): for the cuelist */
    signal historyKey(bool redo)

    // to the ms (a time typed with 3 decimals, or from QLC+, shows as it is)
    function format(value) {
        if (clock)
            return Theme.clockText(value, 2)
        const s = value / 1000
        return s.toLocaleString(Qt.locale(), 'f', value % 100 === 0 ? 1 : (value % 10 === 0 ? 2 : 3))
    }
    /** A clock time typed: hours, minutes and seconds with ":" (the ones on
     *  the left can go), in ms; -1 when it isn't one */
    function parseClock(t) {
        const parts = t.split(":")
        let total = 0
        for (let i = 0; i < parts.length; i++) {
            const v = Number(parts[i].length ? parts[i] : "0")
            if (isNaN(v) || v < 0 || (i < parts.length - 1 && parts[i].indexOf(".") >= 0))
                return -1
            total = total * 60 + v
        }
        return Math.round(total * 1000)
    }
    readonly property string shown: allowSame && same ? "=" : format(ms)
    /** Something was typed: only then a focus that leaves sets the time
     *  (of every cue selected), not a click in and out */
    property bool typed: false
    onTextEdited: typed = true
    function commit() {
        if (clock) {
            const c = text.trim().replace(/,/g, ".")
            if (typed) {
                const v = c.length ? parseClock(c) : -1
                if ((c.length === 0 || v >= 0) && Math.min(clockMax, v) !== ms)
                    committed(Math.min(clockMax, v))
            }
            typed = false
            text = Qt.binding(() => field.shown)
            return
        }
        const t = text.trim().replace(",", ".").replace(/s$/, "").trim()
        if (typed) {
            // ("=" is no time: 0 typed on it is one, an instant down fade)
            const current = allowSame && same ? -1 : ms
            if (allowSame && (t === "" || t === "=")) {
                if (!same)
                    committed(-2)
            } else {
                const v = Number(t)
                if (t.length > 0 && !isNaN(v) && v >= 0 && Math.round(v * 1000) !== current)
                    committed(Math.min(3600000, Math.round(v * 1000)))
            }
        }
        typed = false
        text = Qt.binding(() => field.shown)
    }

    text: shown
    horizontalAlignment: TextInput.AlignRight
    font.family: Theme.monoFamily
    font.pixelSize: Theme.fontSmall + 1
    color: (allowSame && same) || ms === 0 ? Theme.textDim : Theme.text
    selectByMouse: true
    leftPadding: 6
    rightPadding: 6
    // (a clock time with no time: none yet)
    placeholderText: clock ? "—" : ""
    placeholderTextColor: Theme.textFaint
    validator: RegularExpressionValidator {
        regularExpression: field.clock ? /([0-9]{0,2}:){0,2}[0-9]{0,6}([.,][0-9]{0,3})?/
                                       : /=?|[0-9]{0,4}([.,][0-9]{0,3})?\s*s?/
    }
    background: Rectangle {
        radius: Theme.radiusSmall
        color: field.activeFocus ? Theme.background : (fieldHover.hovered ? Theme.surfaceHover : "transparent")
        border.color: field.activeFocus ? Theme.accent : "transparent"
    }
    HoverHandler { id: fieldHover }
    onEditingFinished: commit()
    onActiveFocusChanged: { wheelDelta = 0; if (activeFocus) selectAll() }
    Keys.onShortcutOverride: (event) => event.accepted = event.key === Qt.Key_Escape
    Keys.onEscapePressed: { typed = false; text = Qt.binding(() => field.shown); focus = false }
    Keys.onPressed: (event) => {
        const redo = event.matches(StandardKey.Redo)
        if (event.matches(StandardKey.Undo) ? !field.canUndo : (redo && !field.canRedo)) {
            event.accepted = true
            field.historyKey(redo)
        }
    }
    /* The wheel changes the time only while the cell is being edited
     * (clicked): over the other cells it scrolls the list. A notch
     * (120) is a step; a touchpad adds its small deltas up to one. */
    property real wheelDelta: 0
    WheelHandler {
        enabled: field.activeFocus
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        onWheel: (event) => {
            const shift = (event.modifiers & Qt.ShiftModifier) !== 0
            // (Shift+wheel can come horizontal from the system)
            const delta = event.angleDelta.y !== 0 ? event.angleDelta.y : (shift ? event.angleDelta.x : 0)
            if (delta === 0)
                return
            field.wheelDelta += delta
            const notches = Math.trunc(field.wheelDelta / 120)
            if (notches === 0)
                return
            field.wheelDelta -= notches * 120
            const base = field.allowSame && field.same ? field.sameBase : Math.max(0, field.ms)
            field.committed(Math.min(field.clock ? field.clockMax : 3600000,
                                     Math.max(0, base + notches * (shift ? 1000 : 100))))
            // (what was typed and not entered gives way to the new time)
            field.typed = false
            field.text = Qt.binding(() => field.shown)
        }
    }
}
