/*
  DMX Total Control
  PlaybackPanel.qml

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
 * How a scene plays: fades, cycle,
 * staggered fade, driving mode with divisions, loops and what comes after
 * them, start mode. Saved in the scene.
 */
Panel {
    id: panel

    property var info: ({})

    readonly property int fid: info.id !== undefined ? info.id : -1
    readonly property string type: info.type || ""
    readonly property bool dynamic: ["Sequence", "EFX", "RGBMatrix", "Chaser", "CurveFX", "FXScene", "Collection", "Cuelist"].indexOf(type) >= 0
    // a cuelist: its own options (the times are in its cues)
    readonly property bool cuelist: type === "Cuelist"
    readonly property bool timed: ["EFX", "RGBMatrix", "CurveFX"].indexOf(type) >= 0
    readonly property bool hasLoops: ["Sequence", "EFX", "RGBMatrix", "Chaser", "CurveFX"].indexOf(type) >= 0
    readonly property bool staggers: type === "Scene" || type === "Sequence"

    readonly property var drives: ["off", "bpm", "beatgo", "pulse"]
    readonly property var driveNames: [qsTr("Duration"), qsTr("BPM"), qsTr("BeatGO"), qsTr("Pulse")]

    // the other scenes, to jump to
    readonly property var jumpTargets: {
        let list = [{ id: -1, name: qsTr("Nothing") }, { id: -2, name: qsTr("Next scene of the group") },
                    { id: -3, name: qsTr("Previous scene of the group") }]
        for (let f of Core.functions)
            if (f.id !== panel.fid)
                list.push({ id: f.id, name: f.name })
        return list
    }

    function set(key, value) { Core.effects.setPlayback(fid, key, value) }

    implicitHeight: (cuelist ? cuelistOptions.implicitHeight : grid.implicitHeight) + 20

    CuelistOptions {
        id: cuelistOptions
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 10 }
        visible: panel.cuelist
        info: panel.info
    }

    GridLayout {
        id: grid
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 10 }
        visible: !panel.cuelist
        columns: 2
        columnSpacing: 24
        rowSpacing: 8

        // ---------------------------------------------------------------- times
        RowLayout {
            Layout.columnSpan: 2
            spacing: 10

            Text { text: qsTr("Fade in"); color: Theme.textDim; visible: panel.type !== "Sequence" && panel.type !== "Collection" }
            TimeSpin {
                visible: panel.type !== "Sequence" && panel.type !== "Collection"
                beats: panel.info.drive === "bpm"
                engineValue: panel.info.fadeIn || 0
                onEdited: (v) => Core.effects.setTiming(panel.fid, v, -1, -1)
            }
            Text { text: qsTr("Fade out"); color: Theme.textDim; visible: panel.type !== "Sequence" && panel.type !== "Collection" }
            TimeSpin {
                visible: panel.type !== "Sequence" && panel.type !== "Collection"
                beats: panel.info.drive === "bpm"
                engineValue: panel.info.fadeOut || 0
                onEdited: (v) => Core.effects.setTiming(panel.fid, -1, v, -1)
            }
            Text {
                text: panel.type === "RGBMatrix" ? qsTr("Step time") : qsTr("Cycle time")
                color: Theme.textDim
                visible: panel.timed
            }
            TimeSpin {
                visible: panel.timed
                beats: panel.info.drive === "bpm"
                engineValue: panel.info.duration || 0
                onEdited: (v) => Core.effects.setTiming(panel.fid, -1, -1, v)
            }
            Text {
                text: qsTr("Staggered fade")
                color: Theme.textDim
                visible: panel.staggers
                ToolTip.text: qsTr("Each fixture starts its fade this long after the previous one, in selection order")
                ToolTip.visible: staggerMouse.containsMouse
                MouseArea { id: staggerMouse; anchors.fill: parent; hoverEnabled: true }
            }
            TimeSpin {
                visible: panel.staggers
                engineValue: panel.info.stagger || 0
                onEdited: (v) => panel.set("stagger", v)
            }
        }

        // speed: slow on the left (a cycle a minute), fast on the right (ten a second)
        RowLayout {
            Layout.columnSpan: 2
            visible: panel.timed
            spacing: 10
            Text { text: qsTr("Speed"); color: Theme.textDim; visible: panel.timed && panel.info.drive !== "bpm" }
            HSlider {
                visible: panel.timed && panel.info.drive !== "bpm"
                implicitWidth: 170
                from: 0; to: 1
                resetValue: Math.log(60) / Math.log(600)
                value: Math.log(60000 / Math.max(100, Math.min(60000, panel.info.duration || 1000))) / Math.log(600)
                onMoved: Core.effects.setTiming(panel.fid, -1, -1, Math.round(60000 / Math.pow(600, value)))
                ToolTip.text: qsTr("Speed of the effect (double click: one cycle a second)")
                ToolTip.visible: hovered
                ToolTip.delay: 600
            }
            AppButton {
                visible: panel.timed
                text: "×½"
                implicitHeight: 28
                flat: true
                ToolTip.text: qsTr("Half as fast")
                ToolTip.visible: hovered
                onClicked: Core.effects.setTiming(panel.fid, -1, -1, Math.min(600000, (panel.info.duration || 1000) * 2))
            }
            AppButton {
                visible: panel.timed
                text: "×2"
                implicitHeight: 28
                flat: true
                ToolTip.text: qsTr("Twice as fast")
                ToolTip.visible: hovered
                onClicked: Core.effects.setTiming(panel.fid, -1, -1, Math.max(20, Math.round((panel.info.duration || 1000) / 2)))
            }
        }

        // ---------------------------------------------------------------- driving
        RowLayout {
            visible: panel.dynamic && panel.type !== "Collection" && panel.type !== "FXScene"
            spacing: 8
            Text { text: qsTr("Driven by"); color: Theme.textDim }
            Row {
                spacing: 0
                Repeater {
                    model: panel.drives
                    AppButton {
                        required property int index
                        required property string modelData
                        text: panel.driveNames[index]
                        implicitHeight: 28
                        highlighted: (panel.info.drive || "off") === modelData
                        onClicked: panel.set("drive", modelData)
                    }
                }
            }
            Text {
                visible: panel.timed && (panel.info.drive === "beatgo" || panel.info.drive === "pulse")
                text: qsTr("Divisions")
                color: Theme.textDim
                leftPadding: 6
            }
            AppButton {
                visible: panel.timed && (panel.info.drive === "beatgo" || panel.info.drive === "pulse")
                text: "/2"; implicitHeight: 28; flat: true
                onClicked: panel.set("divisions", Math.max(1, Math.floor((panel.info.divisions || 4) / 2)))
            }
            Text {
                visible: panel.timed && (panel.info.drive === "beatgo" || panel.info.drive === "pulse")
                text: panel.info.divisions || 4
                color: Theme.text
                font.family: Theme.monoFamily
                horizontalAlignment: Text.AlignHCenter
                Layout.preferredWidth: 24
            }
            AppButton {
                visible: panel.timed && (panel.info.drive === "beatgo" || panel.info.drive === "pulse")
                text: "×2"; implicitHeight: 28; flat: true
                onClicked: panel.set("divisions", Math.min(64, (panel.info.divisions || 4) * 2))
            }
        }

        RowLayout {
            visible: panel.dynamic
            spacing: 8
            Text { text: qsTr("Start"); color: Theme.textDim }
            ComboBox {
                Layout.preferredWidth: 220
                model: [qsTr("Playing"), qsTr("Paused, press again to play")]
                currentIndex: panel.info.startMode === "paused" ? 1 : 0
                onActivated: (i) => panel.set("startMode", i === 1 ? "paused" : "play")
            }
        }

        // ---------------------------------------------------------------- loops
        RowLayout {
            Layout.columnSpan: 2
            visible: panel.hasLoops
            spacing: 8

            Text { text: qsTr("Plays"); color: Theme.textDim }
            ComboBox {
                id: loopMode
                Layout.preferredWidth: 150
                model: [qsTr("Always"), qsTr("N times")]
                currentIndex: (panel.info.loops || 0) > 0 ? 1 : 0
                onActivated: (i) => panel.set("loops", i === 1 ? 1 : 0)
            }
            SpinBox {
                visible: (panel.info.loops || 0) > 0
                from: 1; to: 999; editable: true
                value: panel.info.loops || 1
                onValueModified: panel.set("loops", value)
            }
            Text { visible: (panel.info.loops || 0) > 0; text: qsTr("then"); color: Theme.textDim; leftPadding: 6 }
            ComboBox {
                visible: (panel.info.loops || 0) > 0
                Layout.preferredWidth: 190
                model: [qsTr("Release it"), qsTr("Hold the last frame")]
                currentIndex: panel.info.endAction === "pause" ? 1 : 0
                onActivated: (i) => panel.set("endAction", i === 1 ? "pause" : "release")
            }
            Text { visible: (panel.info.loops || 0) > 0; text: qsTr("and play"); color: Theme.textDim; leftPadding: 6 }
            ComboBox {
                visible: (panel.info.loops || 0) > 0
                Layout.preferredWidth: 230
                textRole: "name"
                model: panel.jumpTargets
                currentIndex: Math.max(0, panel.jumpTargets.findIndex(t => t.id === panel.info.jumpTo))
                onActivated: (i) => panel.set("jumpTo", panel.jumpTargets[i].id)
            }
            Item { Layout.fillWidth: true }
        }
    }
}
