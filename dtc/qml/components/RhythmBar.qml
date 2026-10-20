/*
  DMX Total Control
  RhythmBar.qml

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
import DMXTotalControl
import DMXTotalControl.Backend

/**
 * The rhythm of a scene with steps (or a cuelist) inside its button: a bar
 * in two halves that light one after the other, a step (a cue) each, like a
 * metronome. Only while it plays.
 */
Item {
    id: bar

    /** The scene; -1: none */
    property int functionId: -1
    /** The half that is lit and the other one (on the colour of the button) */
    property color litColor: Theme.accent
    property color offColor: Qt.alpha(litColor, 0.25)

    /** Its step now, -1: it doesn't play (or has no steps) */
    readonly property int step: { Core.stepsRevision; return functionId >= 0 ? Core.stepOf(functionId) : -1 }
    property int lastStep: -1
    property bool leftLit: true
    onStepChanged: {
        // another step of the same run: the other half
        if (step >= 0 && lastStep >= 0)
            leftLit = !leftLit
        lastStep = step
    }

    visible: step >= 0
    implicitWidth: 40
    implicitHeight: 4

    Row {
        id: halves
        anchors.fill: parent
        spacing: 2
        Repeater {
            model: 2
            Rectangle {
                required property int index
                width: (halves.width - halves.spacing) / 2
                height: halves.height
                radius: height / 2
                color: (index === 0) === bar.leftLit ? bar.litColor : bar.offColor
            }
        }
    }
}
