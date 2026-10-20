/*
  DMX Total Control
  TapRhythm.qml

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
 * The rhythm of a scene with steps or of a cuelist, tapped on its own
 * button: shown in BPM, typed, or cleared (its own times again)
 */
RowLayout {
    id: rhythm

    /** ms a step (a cue), 0: its own times */
    property int period: 0
    signal edited(int period)

    spacing: 6

    SpinBox {
        from: 0; to: 600
        editable: true
        implicitWidth: 140
        implicitHeight: 28
        value: rhythm.period > 0 ? Math.round(60000 / rhythm.period) : 0
        textFromValue: (v, locale) => v === 0 ? qsTr("Off") : v + " BPM"
        valueFromText: (text, locale) => { const n = parseInt(text); return isNaN(n) ? 0 : n }
        validator: RegularExpressionValidator { regularExpression: /.*/ }
        // (a step at most every 10 s)
        onValueModified: rhythm.edited(value > 0 ? Math.round(60000 / Math.max(6, value)) : 0)
    }
    AppButton {
        visible: rhythm.period > 0
        text: qsTr("Clear")
        implicitHeight: 28
        flat: true
        ToolTip.text: qsTr("Back to its own times")
        ToolTip.visible: hovered
        ToolTip.delay: 500
        onClicked: rhythm.edited(0)
    }
}
