/*
  DMX Total Control
  FxControls.qml

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
 * The controls of one FX: the FX (shape, swing, speed, multiplier)
 * and the FX timing (wave or step per X, grouping, order, wait, phase).
 * Pan/tilt FX have a figure, a swing for each axis, rotation and inversions.
 */
ColumnLayout {
    id: controls

    /** The FX: a layer map (mode, swing, bpm... panTilt) */
    property var fx: ({})
    /** How many sets its fixtures make (for "all" and the limits) */
    property int sets: 1
    /** Only some fixtures of the FX are edited: their shape is theirs, the
     *  speed and the timing are the whole FX's */
    property bool partial: false

    /** The user changed something */
    signal edit(var props)

    readonly property bool panTilt: !!fx.panTilt
    // the speed slider: 0..1 -> 0..360 BPM, finer at the slow end
    function bpmToSlider(bpm) { return Math.sqrt(Math.max(0, Math.min(360, bpm)) / 360) }
    function sliderToBpm(v) { return Math.round(v * v * 360 * 10) / 10 }
    readonly property var multipliers: [0.25, 0.5, 1, 2, 3, 4, 8]

    spacing: 8

    // (the Layout sizes go on the instances: a GridLayout ignores them in a component root)
    component Caption: Text {
        color: Theme.textDim
        font.pixelSize: Theme.fontSmall
        elide: Text.ElideRight
    }
    component Value: Text {
        color: Theme.text
        font.family: Theme.monoFamily
        font.pixelSize: Theme.fontSmall
        horizontalAlignment: Text.AlignRight
    }

    // ------------------------------------------------------------ FX
    RowLayout {
        Layout.fillWidth: true
        spacing: 10

        // the shape: click to pick another
        Rectangle {
            Layout.preferredWidth: 92
            Layout.preferredHeight: 62
            radius: Theme.radiusSmall
            color: shapeMouse.containsMouse ? Theme.surfaceHover : Theme.background
            border.color: Theme.border
            FxShape {
                anchors { left: parent.left; right: parent.right; top: parent.top; bottom: shapeLabel.top; margins: 4 }
                figure: controls.panTilt
                value: controls.panTilt ? (controls.fx.figure || 0) : (controls.fx.mode || 1)
            }
            Text {
                id: shapeLabel
                anchors { left: parent.left; right: parent.right; bottom: parent.bottom; bottomMargin: 3 }
                horizontalAlignment: Text.AlignHCenter
                text: controls.panTilt ? Theme.fxFigureNames[controls.fx.figure || 0] : qsTr("Mode %1").arg(controls.fx.mode || 1)
                color: Theme.textDim
                font.pixelSize: 10
                elide: Text.ElideRight
            }
            MouseArea {
                id: shapeMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: shapePicker.openFor(controls.panTilt, controls.panTilt ? (controls.fx.figure || 0) : (controls.fx.mode || 1))
            }
            ToolTip.visible: shapeMouse.containsMouse
            ToolTip.text: controls.panTilt ? qsTr("Figure: click to choose another") : qsTr("Mode: click to choose another")
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 3
            columnSpacing: 8
            rowSpacing: 2

            Caption { Layout.preferredWidth: 92; Layout.minimumWidth: 92; text: controls.panTilt ? qsTr("Swing pan") : qsTr("Swing") }
            HSlider {
                Layout.minimumWidth: 60
                Layout.preferredWidth: 120
                Layout.fillWidth: true
                from: 0; to: 250; stepSize: 1
                resetValue: 50
                value: controls.fx.swing || 0
                onMoved: controls.edit({ swing: Math.round(value) })
            }
            Value { Layout.preferredWidth: 62; Layout.minimumWidth: 62; text: (controls.fx.swing || 0) + "%" }

            Caption { Layout.preferredWidth: 92; Layout.minimumWidth: 92; visible: controls.panTilt; text: qsTr("Swing tilt") }
            HSlider {
                visible: controls.panTilt
                Layout.minimumWidth: 60
                Layout.preferredWidth: 120
                Layout.fillWidth: true
                from: 0; to: 250; stepSize: 1
                resetValue: 50
                value: controls.fx.swingTilt || 0
                onMoved: controls.edit({ swingTilt: Math.round(value) })
            }
            Value { Layout.preferredWidth: 62; Layout.minimumWidth: 62; visible: controls.panTilt; text: (controls.fx.swingTilt || 0) + "%" }

            Caption { Layout.preferredWidth: 92; Layout.minimumWidth: 92; text: qsTr("Speed") }
            HSlider {
                Layout.minimumWidth: 60
                Layout.preferredWidth: 120
                Layout.fillWidth: true
                enabled: !controls.fx.syncBpm
                from: 0; to: 1
                resetValue: controls.bpmToSlider(30)
                value: controls.bpmToSlider(controls.fx.bpm || 0)
                onMoved: controls.edit({ bpm: controls.sliderToBpm(value) })
            }
            Value {
                Layout.preferredWidth: 62
                Layout.minimumWidth: 62
                text: controls.fx.syncBpm ? qsTr("show") : (controls.fx.bpm || 0) === 0 ? qsTr("stop") : Math.round(controls.fx.bpm || 0) + " BPM"
            }
        }
    }

    Flow {
        Layout.fillWidth: true
        spacing: 6

        AppButton {
            text: "½"
            implicitHeight: 28
            enabled: !controls.fx.syncBpm
            ToolTip.text: qsTr("Half the speed"); ToolTip.visible: hovered
            onClicked: controls.edit({ bpm: Math.max(0.5, (controls.fx.bpm || 0) / 2) })
        }
        AppButton {
            text: "×2"
            implicitHeight: 28
            enabled: !controls.fx.syncBpm
            ToolTip.text: qsTr("Twice the speed"); ToolTip.visible: hovered
            onClicked: controls.edit({ bpm: Math.min(360, Math.max(1, (controls.fx.bpm || 0) * 2)) })
        }
        AppButton {
            text: qsTr("BPM of the show")
            implicitHeight: 28
            checkable: true
            checked: !!controls.fx.syncBpm
            ToolTip.text: qsTr("The speed follows the BPM (tap, BeatGO)"); ToolTip.visible: hovered
            onClicked: controls.edit({ syncBpm: checked })
        }
        Text { text: qsTr("Multiplier"); color: Theme.textDim; font.pixelSize: Theme.fontSmall; height: 28; verticalAlignment: Text.AlignVCenter }
        ComboBox {
            implicitWidth: 80
            implicitHeight: 28
            model: controls.multipliers.map(m => m === 0.25 ? "×¼" : m === 0.5 ? "×½" : "×" + m)
            currentIndex: Math.max(0, controls.multipliers.indexOf(controls.fx.multiplier || 1))
            onActivated: (i) => controls.edit({ multiplier: controls.multipliers[i] })
        }
    }

    // ------------------------------------------------------------ pan/tilt
    Flow {
        Layout.fillWidth: true
        visible: controls.panTilt
        spacing: 6
        AppButton {
            text: qsTr("Invert pan")
            implicitHeight: 28
            checkable: true
            checked: !!controls.fx.panInvert
            onClicked: controls.edit({ panInvert: checked })
        }
        AppButton {
            text: qsTr("Invert tilt")
            implicitHeight: 28
            checkable: true
            checked: !!controls.fx.tiltInvert
            onClicked: controls.edit({ tiltInvert: checked })
        }
        AppButton {
            text: qsTr("Backwards")
            implicitHeight: 28
            checkable: true
            checked: !!controls.fx.reverse
            onClicked: controls.edit({ reverse: checked })
        }
    }
    RowLayout {
        Layout.fillWidth: true
        visible: controls.panTilt
        spacing: 8
        Caption { Layout.preferredWidth: 92; Layout.minimumWidth: 92; text: qsTr("Rotate") }
        HSlider {
            Layout.minimumWidth: 60
            Layout.preferredWidth: 120
            Layout.fillWidth: true
            from: 0; to: 359; stepSize: 1
            value: controls.fx.rotate || 0
            onMoved: controls.edit({ rotate: Math.round(value) })
        }
        Value { Layout.preferredWidth: 62; Layout.minimumWidth: 62; text: (controls.fx.rotate || 0) + "°" }
    }

    // ------------------------------------------------------------ FX timing
    Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }
    Text {
        Layout.fillWidth: true
        text: controls.partial ? qsTr("FX timing: for the whole FX (except the phase)") : qsTr("FX timing")
        color: Theme.textDim
        font.pixelSize: Theme.fontSmall
        font.weight: Font.DemiBold
        elide: Text.ElideRight
    }

    GridLayout {
        Layout.fillWidth: true
        columns: 2
        columnSpacing: 8
        rowSpacing: 6

        component CountSpin: SpinBox {
            id: countSpin
            // (its own property: an inline component does not see the ids of the file)
            property int sets: 1
            from: -1
            to: 64
            editable: false
            implicitWidth: 150
            implicitHeight: 30
            textFromValue: (v) => v === 0 ? qsTr("Off") : v < 0 ? qsTr("All (%1)").arg(sets) : v
            // its own text: the spin box would not redo it when only the sets change
            contentItem: Text {
                text: countSpin.value === 0 ? qsTr("Off") : countSpin.value < 0 ? qsTr("All (%1)").arg(countSpin.sets) : countSpin.value
                color: Theme.text
                font: countSpin.font
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        Caption { Layout.preferredWidth: 92; Layout.minimumWidth: 92; text: qsTr("Wave per X") }
        CountSpin {
            sets: controls.sets
            value: controls.fx.wave || 0
            onValueModified: controls.edit({ wave: value, step: value !== 0 ? 0 : (controls.fx.step || 0) })
            ToolTip.text: qsTr("The shape spreads over X fixtures (or sets): each one a bit later"); ToolTip.visible: hovered
        }
        Caption { Layout.preferredWidth: 92; Layout.minimumWidth: 92; text: qsTr("Step per X") }
        CountSpin {
            sets: controls.sets
            value: controls.fx.step || 0
            onValueModified: controls.edit({ step: value, wave: value !== 0 ? 0 : (controls.fx.wave || 0) })
            ToolTip.text: qsTr("One fixture (or set) after the other, X of them"); ToolTip.visible: hovered
        }

        Caption { Layout.preferredWidth: 92; Layout.minimumWidth: 92; text: qsTr("Grouping") }
        RowLayout {
            spacing: 4
            ComboBox {
                id: groupingBox
                implicitWidth: 150
                implicitHeight: 30
                model: [qsTr("Off"), qsTr("Every X"), qsTr("Blocks of X"), qsTr("Divide by X"), qsTr("Mirror"), qsTr("By group")]
                currentIndex: controls.fx.grouping || 0
                onActivated: (i) => controls.edit({ grouping: i })
            }
            SpinBox {
                visible: (controls.fx.grouping || 0) >= 1 && (controls.fx.grouping || 0) <= 4
                from: 1; to: 32
                implicitWidth: 96
                implicitHeight: 30
                value: controls.fx.groupSize || 2
                onValueModified: controls.edit({ groupSize: value })
            }
        }
        Caption { Layout.preferredWidth: 92; Layout.minimumWidth: 92; text: qsTr("Order") }
        RowLayout {
            spacing: 4
            ComboBox {
                implicitWidth: 130
                implicitHeight: 30
                model: [qsTr("Selection"), qsTr("Reversed"), qsTr("Random"), qsTr("By number")]
                currentIndex: controls.fx.order || 0
                onActivated: (i) => controls.edit({ order: i })
            }
            AppButton {
                visible: (controls.fx.order || 0) === 2
                iconGlyph: Theme.iconRefresh
                implicitHeight: 30
                ToolTip.text: qsTr("Another random order"); ToolTip.visible: hovered
                onClicked: controls.edit({ seed: (controls.fx.seed || 1) + 1 })
            }
        }

        Caption { Layout.preferredWidth: 92; Layout.minimumWidth: 92; text: qsTr("Wait") }
        TimeSpin {
            implicitHeight: 30
            engineValue: controls.fx.wait || 0
            onEdited: (v) => controls.edit({ wait: v })
            ToolTip.text: qsTr("Time at the base after each run"); ToolTip.visible: hovered
        }
        Caption { Layout.preferredWidth: 92; Layout.minimumWidth: 92; text: qsTr("Phase") }
        RowLayout {
            spacing: 4
            HSlider {
                Layout.preferredWidth: 110
                from: 0; to: 359; stepSize: 1
                value: controls.fx.phase || 0
                onMoved: controls.edit({ phase: Math.round(value) })
            }
            Value { Layout.preferredWidth: 36; text: (controls.fx.phase || 0) + "°" }
        }
    }

    FxShapePicker {
        id: shapePicker
        onPicked: (v) => controls.edit(controls.panTilt ? { figure: v } : { mode: v })
    }
}
