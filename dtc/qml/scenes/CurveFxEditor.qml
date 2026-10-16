/*
  DMX Total Control
  CurveFxEditor.qml

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

/** Curve FX: a waveform on any channel of the fixtures [CurveFX] */
RowLayout {
    id: editor

    property var info: ({})
    property var lines: []
    property var features: []

    readonly property var waves: Core.effects.curveWaveforms()
    readonly property var waveNames: ({
        "Sine": qsTr("Sine"), "Triangle": qsTr("Triangle"), "RampUp": qsTr("Ramp up"),
        "RampDown": qsTr("Ramp down"), "Square": qsTr("Square"), "Random": qsTr("Random"),
        "Strobe": qsTr("Strobe"), "RandomStrobe": qsTr("Random strobe")
    })
    readonly property var lineColors: ["#ae3ec9", "#3d8bfd", "#30a46c", "#f5a524", "#e5484d", "#17a2b8", "#d6336c", "#e7e9ef"]

    function set(key, value) { Core.effects.setCurveFx(info.id, key, value) }

    // the level of each fixture as it leaves the software (live while it plays)
    property var live: ({})
    Timer {
        interval: 50
        repeat: true
        running: editor.visible && editor.info.id !== undefined
        onTriggered: editor.live = Core.effects.liveView(editor.info.id)
    }

    function toggleFeature(key) {
        let selected = features.filter(f => f.selected).map(f => f.key)
        let i = selected.indexOf(key)
        if (i >= 0)
            selected.splice(i, 1)
        else
            selected.push(key)
        set("features", selected)
    }

    onInfoChanged: {
        lines = Core.effects.curvePreview(info.id)
        features = Core.effects.curveFeatures(info.id)
        preview.requestPaint()
    }

    spacing: 12

    // the wave of each fixture along one cycle
    Panel {
        Layout.preferredWidth: 380
        Layout.fillHeight: true
        title: editor.live.running ? qsTr("Wave — live") : qsTr("Wave")

        Canvas {
            id: preview
            anchors { fill: parent; margins: 14; bottomMargin: 110 }
            onPaint: {
                let ctx = getContext("2d")
                ctx.reset()
                ctx.fillStyle = Theme.background
                ctx.fillRect(0, 0, width, height)
                ctx.strokeStyle = Theme.border
                ctx.lineWidth = 1
                for (let i = 1; i < 4; i++) {
                    ctx.beginPath(); ctx.moveTo(0, height * i / 4); ctx.lineTo(width, height * i / 4); ctx.stroke()
                    ctx.beginPath(); ctx.moveTo(width * i / 4, 0); ctx.lineTo(width * i / 4, height); ctx.stroke()
                }
                for (let l = editor.lines.length - 1; l >= 0; l--) {
                    let pts = editor.lines[l]
                    ctx.strokeStyle = editor.lineColors[l % editor.lineColors.length]
                    ctx.lineWidth = l === 0 ? 2.5 : 1.5
                    ctx.beginPath()
                    for (let i = 0; i < pts.length; i++) {
                        let x = width * i / (pts.length - 1)
                        let y = height - height * pts[i] / 255
                        if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
                    }
                    ctx.stroke()
                }
            }
        }
        // live levels, one bar per fixture in the effect order
        Row {
            id: liveBars
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: 14; bottomMargin: 30 }
            height: 64
            spacing: 3
            readonly property var items: editor.live.items || []
            Repeater {
                model: liveBars.items
                Item {
                    required property var modelData
                    required property int index
                    width: Math.max(8, (liveBars.width - (liveBars.items.length - 1) * 3) / Math.max(1, liveBars.items.length))
                    height: liveBars.height
                    Rectangle {
                        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; bottomMargin: 14 }
                        height: (parent.height - 14) * parent.modelData.level
                        radius: 2
                        color: editor.lineColors[parent.index % editor.lineColors.length]
                        opacity: editor.live.running ? 1 : 0.3
                    }
                    Text {
                        anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter }
                        text: parent.modelData.index
                        color: Theme.textDim
                        font.pixelSize: 9
                    }
                }
            }
        }
        Text {
            anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 8 }
            text: qsTr("One cycle →   one line per fixture (first 8)")
            color: Theme.textFaint
            font.pixelSize: 10
        }
    }

    Panel {
        Layout.fillWidth: true
        Layout.fillHeight: true
        title: qsTr("Curve")

        Flickable {
            anchors { fill: parent; margins: 14; bottomMargin: 54 }
            contentHeight: grid.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            GridLayout {
                id: grid
                width: parent.width
                columns: 3
                columnSpacing: 12
                rowSpacing: 8

                component ValueSlider: HSlider {
                    property string key
                    Layout.preferredWidth: 400
                    Layout.fillWidth: true
                    stepSize: 1
                    onMoved: editor.set(key, Math.round(value))
                }
                component ValueText: Text {
                    Layout.preferredWidth: 44
                    horizontalAlignment: Text.AlignRight
                    color: Theme.text
                    font.family: Theme.monoFamily
                }

                Text { text: qsTr("Wave"); color: Theme.textDim }
                ComboBox {
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    model: editor.waves.map(w => editor.waveNames[w] || w)
                    currentIndex: Math.max(0, editor.waves.indexOf(editor.info.wave))
                    onActivated: (i) => editor.set("wave", editor.waves[i])
                }

                Text { text: qsTr("Low"); color: Theme.textDim }
                ValueSlider { id: lowSlider; key: "low"; from: 0; to: 255; value: editor.info.low || 0 }
                ValueText { text: Math.round(lowSlider.value) }

                Text { text: qsTr("High"); color: Theme.textDim }
                ValueSlider { id: highSlider; key: "high"; from: 0; to: 255; value: editor.info.high !== undefined ? editor.info.high : 255 }
                ValueText { text: Math.round(highSlider.value) }

                // how long a square or strobe wave is up
                readonly property bool hasDuty: ["Square", "Strobe", "RandomStrobe"].indexOf(editor.info.wave) >= 0
                Text { text: qsTr("Flash length"); color: Theme.textDim; visible: grid.hasDuty }
                ValueSlider { id: dutySlider; key: "duty"; from: 1; to: 99; value: editor.info.duty || 50; visible: grid.hasDuty }
                ValueText { text: Math.round(dutySlider.value) + "%"; visible: grid.hasDuty }

                Text { text: qsTr("Phase"); color: Theme.textDim }
                ValueSlider { id: phaseSlider; key: "phase"; from: 0; to: 359; value: editor.info.phase || 0 }
                ValueText { text: Math.round(phaseSlider.value) + "°" }

                Text { text: qsTr("Offset between fixtures"); color: Theme.textDim }
                ValueSlider { id: spreadSlider; key: "spread"; from: 0; to: 1440; value: editor.info.spread || 0 }
                ValueText { text: Math.round(spreadSlider.value) + "°" }

                Text { text: qsTr("Relative"); color: Theme.textDim }
                AppSwitch { checked: !!editor.info.relative; onToggled: editor.set("relative", checked) }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: qsTr("Moves the channels around the value other scenes give them")
                    color: Theme.textFaint
                    font.pixelSize: Theme.fontSmall
                }

                Text { text: qsTr("Channels"); color: Theme.textDim; Layout.alignment: Qt.AlignTop; topPadding: 4 }
                Flow {
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    spacing: 4
                    Repeater {
                        model: editor.features
                        AppButton {
                            required property var modelData
                            text: modelData.name
                            implicitHeight: 28
                            highlighted: modelData.selected
                            onClicked: editor.toggleFeature(modelData.key)
                        }
                    }
                }
            }
        }

        RowLayout {
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: 14 }
            Text {
                Layout.fillWidth: true
                text: qsTr("%n fixture(s) in the effect", "", editor.info.fixtures || 0)
                color: Theme.textDim
            }
            AppButton {
                text: qsTr("Use the programmer selection")
                iconGlyph: Theme.iconSelect
                onClicked: Core.effects.useSelection(editor.info.id)
            }
        }
    }
}
