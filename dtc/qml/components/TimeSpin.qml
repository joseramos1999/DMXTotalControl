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
 * A time field in tenths: seconds ("1.5 s") or, when following the BPM,
 * beats ("2.0 b"). The engine value is in ms, or beats x 1000.
 * Emits edited(engineValue) when the user changes it.
 */
SpinBox {
    id: control

    property bool beats: false
    /** Engine value: ms, or beats x 1000 */
    property int engineValue: 0

    signal edited(int engineValue)

    from: 0
    to: 36000
    stepSize: 1
    editable: true
    implicitWidth: 130
    value: Math.round(engineValue / 100)

    textFromValue: (v, locale) => (v / 10).toLocaleString(locale, 'f', 1) + (beats ? " b" : " s")
    valueFromText: (text, locale) => Math.round(Theme.parseNumber(text, value / 10) * 10)
    validator: RegularExpressionValidator { regularExpression: /[0-9]+([.,][0-9])?\s*[sb]?/ }

    onValueModified: edited(value * 100)
}
