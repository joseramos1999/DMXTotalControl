/*
  DMX Total Control
  SurfaceFader.qml

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
import DMXTotalControl.Backend

/**
 * A fader of the SHOW screen. It can drive several things
 * at once: grand master, group masters, scenes (up from zero starts them),
 * their dimmer (without playing them), their speed, the dimmer of fixture
 * groups, the movement speed of moving heads, the BPM. Its travel can cover
 * only a part of them ("min", "max", "invert"). The button under it puts it
 * at the top of its travel while pressed, then gives back what they were.
 */
Rectangle {
    id: cell

    property var item: ({})
    property bool editing: false
    // where it is, for the mappings
    property int page: 0
    property string sectionId: ""
    property int cellIndex: -1

    readonly property var targets: Core.surface.faderTargets(item)
    readonly property var first: targets.length ? targets[0] : ({ target: "grandMaster" })
    readonly property bool isBpm: targets.length === 1 && first.target === "bpm"
    /** A speed (of the movement, of a scene, a scene played at a speed): x0.1 at the bottom, x1 in the middle, x10 at the top */
    readonly property bool isSpeed: Core.surface.isSpeed(item)
    function speedText(v) {
        // (a scene played at a speed: stopped at zero)
        if (first.target === "sceneRun" && v <= 0)
            return qsTr("Stopped")
        const factor = Math.pow(10, v * 2 - 1)
        return "×" + factor.toLocaleString(Qt.locale(), 'f', factor < 1 ? 2 : 1)
    }
    // where the fader is on its travel, and what its first target is at (the text)
    readonly property real level: { Core.surface.revision; Core.grandMaster; Core.bpm; Core.mixer.mixers; return Core.surface.level(item) }
    readonly property real value: { Core.surface.revision; Core.grandMaster; Core.bpm; Core.mixer.mixers; return Core.surface.value(item) }

    function targetName(t) {
        switch (t.target) {
        case "grandMaster": return qsTr("Grand master")
        case "bpm": return qsTr("BPM")
        case "group": {
            let zone = Core.zones.zoneById(t.zoneId)
            return zone ? zone.name : qsTr("Choose a group")
        }
        case "scene": {
            let info = Core.functionInfo(t.functionId)
            return info.name || qsTr("Choose a scene")
        }
        case "sceneDimmer": {
            let info = Core.functionInfo(t.functionId)
            return qsTr("Dimmer of %1").arg(info.name || "?")
        }
        case "sceneSpeed": {
            let info = Core.functionInfo(t.functionId)
            return qsTr("Speed of %1").arg(info.name || "?")
        }
        case "sceneRun": {
            let info = Core.functionInfo(t.functionId)
            return info.name || qsTr("Choose a scene")
        }
        case "mixerDimmer": {
            let m = Core.mixer.mixers.find(x => x.id === (t.mixerId !== undefined ? t.mixerId : -1))
            return qsTr("Dimmer of %1").arg(m ? m.name : "?")
        }
        case "moveSpeed": return qsTr("Movement speed")
        case "map":
            // (a playback is named by what its slot holds now, a function by its name)
            Core.surface.revision
            return Core.mapping.describeTarget(t.map || ({}))
        }
        return ""
    }
    function label() {
        if (item.label)
            return item.label
        return targets.map(t => targetName(t)).join(" + ")
    }

    radius: Theme.radiusSmall
    color: Theme.surfaceRaised
    border.color: Theme.border

    ColumnLayout {
        anchors { fill: parent; margins: 4 }
        spacing: 2
        Text {
            Layout.fillWidth: true
            text: cell.label()
            color: Theme.text
            font.pixelSize: Theme.fontSmall
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
        VFader {
            id: fader
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
            implicitWidth: Math.min(44, cell.width - 8)
            enabled: !cell.editing
            fillColor: cell.item.color ? Theme.category(cell.item.color) : Theme.accent
            from: 0
            to: 255
            // (the BPM: where 90 BPM is on its travel; a speed: x1)
            resetValue: Core.surface.resetLevel(cell.item) * 255
            value: cell.level * 255
            onUserMoved: (v) => Core.surface.setLevel(cell.item, v / 255)
        }
        Text {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: cell.isBpm ? Math.round(20 + cell.value * 280) : cell.isSpeed ? cell.speedText(cell.value) : Math.round(cell.value * 100) + "%"
            color: Theme.textDim
            font.family: Theme.monoFamily
            font.pixelSize: 10
            // (a long word, «Stopped», in a narrow cell: smaller, not over the next one)
            fontSizeMode: Text.HorizontalFit
            minimumPixelSize: 7
            elide: Text.ElideRight
        }
        // the top of its travel while pressed, then each target back to what it was
        AppButton {
            id: flashButton
            property var saved: []
            Layout.fillWidth: true
            implicitHeight: 26
            visible: cell.item.flash !== false
            enabled: !cell.editing
            text: qsTr("Flash")
            highlighted: pressed
            accentColor: Theme.warning
            onPressed: { saved = Core.surface.holdLevels(cell.item); Core.surface.setLevel(cell.item, 1) }
            onReleased: Core.surface.restoreLevels(cell.item, saved)
            onCanceled: Core.surface.restoreLevels(cell.item, saved)
        }
    }

    // learn mode: a device moves the whole fader
    MapTarget {
        visible: Core.mapping.learnMode && !cell.editing
        target: ({ target: "surfaceFader", page: cell.page, section: cell.sectionId, cell: cell.cellIndex })
    }
}
