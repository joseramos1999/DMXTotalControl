/*
  DMX Total Control
  MapTarget.qml

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

/**
 * Learn mode (mapping overlay): put it over a control that can be
 * mapped. In learn mode it covers the control in purple; a click picks the
 * control, then the next control that moves on a device drives it.
 * Outside learn mode it is invisible, and Alt+click puts the control on
 * the SHOW; other clicks go through to the control.
 */
Rectangle {
    id: overlay

    /** What the control drives: { target, functionId, zoneId, mixerId... } */
    property var target: ({})

    readonly property bool pending: {
        let p = Core.mapping.pendingTarget
        if (p.target === undefined || p.target !== target.target || p.id !== undefined)
            return false
        for (let k of ["functionId", "zoneId", "index", "mixerId", "page", "section", "cell", "paletteId", "groupId", "pbPage", "slot", "cue"])
            if (target[k] !== undefined && p[k] !== target[k])
                return false
        return true
    }
    readonly property int count: { Core.mapping.mappings; return learning && visible ? Core.mapping.countFor(target) : 0 }

    readonly property bool learning: Core.mapping.learnMode

    anchors.fill: parent
    z: 50
    radius: 4
    color: !learning ? "transparent" : pending ? Qt.rgba(0.68, 0.24, 0.79, 0.55) : Qt.rgba(0.68, 0.24, 0.79, 0.22)
    border.color: "#ae3ec9"
    border.width: !learning ? 0 : pending ? 3 : 1

    // how many mappings drive it already
    Rectangle {
        visible: overlay.learning && overlay.count > 0
        anchors { right: parent.right; top: parent.top; margins: 2 }
        width: Math.max(16, countText.implicitWidth + 8)
        height: 16
        radius: 8
        color: "#ae3ec9"
        Text {
            id: countText
            anchors.centerIn: parent
            text: overlay.count
            color: "white"
            font.pixelSize: 10
            font.weight: Font.Bold
        }
    }

    MouseArea {
        anchors.fill: parent
        // Shift+click still uses the control below
        acceptedButtons: Qt.LeftButton
        onPressed: (ev) => {
            // Alt+click makes a control on the SHOW that does the same
            if (!overlay.learning) {
                if (ev.modifiers & Qt.AltModifier)
                    Core.surface.addFromTarget(overlay.target)
                else
                    ev.accepted = false
                return
            }
            if (ev.modifiers & Qt.ShiftModifier) {
                ev.accepted = false
                return
            }
            if (overlay.pending)
                Core.mapping.cancelLearn()
            else
                Core.mapping.startLearn(overlay.target)
        }
    }
}
