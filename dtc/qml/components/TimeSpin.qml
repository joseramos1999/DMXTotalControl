/*
  DMX Total Control
  TimeSpin.qml

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

/**
 * A time field in tenths (or hundredths): seconds ("1.5 s") or, when
 * following the BPM, beats ("2.0 b"). The engine value is in ms, or beats
 * x 1000. Emits edited(engineValue) when the user changes it.
 */
SpinBox {
    id: control

    property bool beats: false
    /** Engine value: ms, or beats x 1000 */
    property int engineValue: 0
    /** 1: tenths, 2: hundredths of a second (or beat) */
    property int decimals: 1
    /** the engine units (ms) of one step of the field */
    readonly property int msStep: decimals >= 2 ? 10 : 100
    readonly property int perSecond: decimals >= 2 ? 100 : 10
    /** Narrow − and + (a bar of several fields): the time gets their room */
    property bool compact: false
    /** Wide enough to show its time whole, in the font and the language of
     *  the field: any time up to 999.9 s (or beats), or the one it holds
     *  when that is wider (a block hours into a super scene: the field grows
     *  with it); the − and +, the padding of the text inside the field (6 px
     *  a side in the Basic style: with less, the text scrolls and its first
     *  digit is cut), and 2 px for the cursor (implicitWidth: fitWidth) */
    readonly property real fitWidth: Math.ceil(widest.advanceWidth) + leftPadding + rightPadding
                                     + (contentItem ? (contentItem.leftPadding || 0) + (contentItem.rightPadding || 0) : 0) + 2

    signal edited(int engineValue)

    TextMetrics {
        id: widest
        font: control.font
        text: control.textFromValue(Math.max(control.value, 1000 * control.perSecond - 1), control.locale)
    }
    Binding { target: control.up.indicator; property: "implicitWidth"; value: 30; when: control.compact }
    Binding { target: control.down.indicator; property: "implicitWidth"; value: 30; when: control.compact }

    from: 0
    to: 3600 * perSecond
    stepSize: 1
    editable: true
    implicitWidth: 130
    value: Math.round(engineValue / msStep)

    textFromValue: (v, locale) => (v / perSecond).toLocaleString(locale, 'f', decimals >= 2 ? 2 : 1) + (beats ? " b" : " s")
    valueFromText: (text, locale) => Math.round(Theme.parseNumber(text, value / perSecond) * perSecond)
    validator: RegularExpressionValidator { regularExpression: control.decimals >= 2 ? /[0-9]+([.,][0-9]{1,2})?\s*[sb]?/ : /[0-9]+([.,][0-9])?\s*[sb]?/ }

    onValueModified: edited(value * msStep)
}
