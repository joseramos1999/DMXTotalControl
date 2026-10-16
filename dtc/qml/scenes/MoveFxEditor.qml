/*
  DMX Total Control
  MoveFxEditor.qml

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

/** Move FX: a shape followed by pan/tilt [EFX] */
RowLayout {
    id: editor

    property var info: ({})
    property var points: []
    // where the heads are: live while it plays, else the start of their path
    property var live: ({})

    readonly property var shapes: Core.effects.moveShapes()
    readonly property var shapeNames: ({
        "Circle": qsTr("Circle"), "Eight": qsTr("Eight"), "Line": qsTr("Line"), "Line2": qsTr("Line 2"),
        "Diamond": qsTr("Diamond"), "Square": qsTr("Square"), "SquareChoppy": qsTr("Square (choppy)"),
        "SquareTrue": qsTr("Square (exact)"), "Leaf": qsTr("Leaf"), "Lissajous": qsTr("Lissajous")
    })

    function set(key, value) { Core.effects.setMoveFx(info.id, key, value) }

    onInfoChanged: { points = Core.effects.movePreview(info.id); live = Core.effects.liveView(info.id); preview.requestPaint() }

    // fast while it plays; stopped, only to see it start (or the heads move
    // from elsewhere)
    Timer {
        interval: editor.live.running ? 40 : 400
        repeat: true
        running: editor.visible && editor.info.id !== undefined
        onTriggered: { editor.live = Core.effects.liveView(editor.info.id); preview.requestPaint() }
    }

    spacing: 12

    // shape preview, on the pan/tilt range
    Panel {
        Layout.preferredWidth: 340
        Layout.fillHeight: true
        title: editor.live.running ? qsTr("Shape — live") : qsTr("Shape")

        Canvas {
            id: preview
            anchors { fill: parent; margins: 14 }
            onPaint: {
                let ctx = getContext("2d")
                let size = Math.min(width, height)
                let ox = (width - size) / 2, oy = (height - size) / 2
                ctx.reset()
                ctx.fillStyle = Theme.background
                ctx.fillRect(ox, oy, size, size)
                ctx.strokeStyle = Theme.border
                ctx.lineWidth = 1
                for (let i = 1; i < 4; i++) {
                    ctx.beginPath(); ctx.moveTo(ox + size * i / 4, oy); ctx.lineTo(ox + size * i / 4, oy + size); ctx.stroke()
                    ctx.beginPath(); ctx.moveTo(ox, oy + size * i / 4); ctx.lineTo(ox + size, oy + size * i / 4); ctx.stroke()
                }
                let pts = editor.points
                if (pts.length < 2)
                    return
                ctx.strokeStyle = Theme.functionColor("EFX")
                ctx.lineWidth = 2.5
                ctx.beginPath()
                ctx.moveTo(ox + pts[0].x * size, oy + pts[0].y * size)
                for (let p of pts)
                    ctx.lineTo(ox + p.x * size, oy + p.y * size)
                ctx.stroke()
                // start point
                ctx.fillStyle = Theme.text
                ctx.beginPath()
                ctx.arc(ox + pts[0].x * size, oy + pts[0].y * size, 5, 0, 2 * Math.PI)
                ctx.fill()

                // the heads, numbered in the effect order
                let items = (editor.live.items || [])
                let running = !!editor.live.running
                ctx.font = "bold 10px sans-serif"
                ctx.textAlign = "center"
                ctx.textBaseline = "middle"
                for (let i = items.length - 1; i >= 0; i--) {
                    let it = items[i]
                    let px = running && it.pan >= 0 ? it.pan : it.startX
                    let py = running && it.tilt >= 0 ? it.tilt : it.startY
                    let cx = ox + px * size, cy = oy + py * size
                    ctx.fillStyle = running ? (i === 0 ? "#ffffff" : Theme.functionColor("EFX")) : Theme.surfaceRaised
                    ctx.strokeStyle = i === 0 ? Theme.functionColor("EFX") : "#ffffff"
                    ctx.lineWidth = 2
                    ctx.beginPath()
                    ctx.arc(cx, cy, 10, 0, 2 * Math.PI)
                    ctx.fill()
                    ctx.stroke()
                    ctx.fillStyle = running && i === 0 ? "#111111" : "#ffffff"
                    ctx.fillText(String(it.index), cx, cy)
                }
            }
        }
        Text {
            anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 2 }
            text: qsTr("Pan →   Tilt ↓")
            color: Theme.textFaint
            font.pixelSize: 10
        }
    }

    Panel {
        Layout.fillWidth: true
        Layout.fillHeight: true
        title: qsTr("Movement")

        // scrolls above the footer: at 900 px high the last rows ("Control
        // dimmer") were drawn over the footer
        Flickable {
            id: movementScroll
            anchors { fill: parent; margins: 14; bottomMargin: 54 }
            contentHeight: grid.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { policy: movementScroll.contentHeight > movementScroll.height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded }

            GridLayout {
                id: grid
                width: movementScroll.width - 12
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
                    Layout.preferredWidth: 40
                    horizontalAlignment: Text.AlignRight
                    color: Theme.text
                    font.family: Theme.monoFamily
                }

                Text { text: qsTr("Shape"); color: Theme.textDim }
                ComboBox {
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    model: editor.shapes.map(s => editor.shapeNames[s] || s)
                    currentIndex: Math.max(0, editor.shapes.indexOf(editor.info.shape))
                    onActivated: (i) => editor.set("shape", editor.shapes[i])
                }

                Text { text: qsTr("Width"); color: Theme.textDim }
                ValueSlider { id: w; Layout.fillWidth: true; key: "width"; from: 0; to: 127; value: editor.info.width || 0 }
                ValueText { text: Math.round(w.value) }

                Text { text: qsTr("Height"); color: Theme.textDim }
                ValueSlider { id: h; Layout.fillWidth: true; key: "height"; from: 0; to: 127; value: editor.info.height || 0 }
                ValueText { text: Math.round(h.value) }

                Text { text: qsTr("Rotation"); color: Theme.textDim }
                ValueSlider { id: r; Layout.fillWidth: true; key: "rotation"; from: 0; to: 359; value: editor.info.rotation || 0 }
                ValueText { text: Math.round(r.value) + "°" }

                Text { text: qsTr("Pan position"); color: Theme.textDim }
                ValueSlider { id: x; Layout.fillWidth: true; key: "xOffset"; from: 0; to: 255; value: editor.info.xOffset || 0 }
                ValueText { text: Math.round(x.value) }

                Text { text: qsTr("Tilt position"); color: Theme.textDim }
                ValueSlider { id: y; Layout.fillWidth: true; key: "yOffset"; from: 0; to: 255; value: editor.info.yOffset || 0 }
                ValueText { text: Math.round(y.value) }

                Text { text: qsTr("Phase"); color: Theme.textDim }
                ValueSlider { id: ph; Layout.fillWidth: true; key: "phase"; from: 0; to: 359; value: editor.info.phase || 0 }
                ValueText { text: Math.round(ph.value) + "°" }

                Text { text: qsTr("Pan frequency"); color: Theme.textDim; visible: editor.info.shape === "Lissajous" }
                ValueSlider { id: fx; Layout.fillWidth: true; key: "xFrequency"; from: 1; to: 32; value: editor.info.xFrequency || 1; visible: editor.info.shape === "Lissajous" }
                ValueText { text: Math.round(fx.value); visible: editor.info.shape === "Lissajous" }

                Text { text: qsTr("Tilt frequency"); color: Theme.textDim; visible: editor.info.shape === "Lissajous" }
                ValueSlider { id: fy; Layout.fillWidth: true; key: "yFrequency"; from: 1; to: 32; value: editor.info.yFrequency || 1; visible: editor.info.shape === "Lissajous" }
                ValueText { text: Math.round(fy.value); visible: editor.info.shape === "Lissajous" }

                Text { text: qsTr("Fixtures"); color: Theme.textDim }
                ComboBox {
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    model: [qsTr("All together"), qsTr("One after the other"), qsTr("Offset along the selection")]
                    currentIndex: editor.info.spread || 0
                    onActivated: (i) => editor.set("spread", i)
                }

                // with an offset: how far apart the first and the last one move
                Text { text: qsTr("Phase between fixtures"); color: Theme.textDim; visible: editor.info.spread === 2 }
                ValueSlider {
                    id: spreadSlider
                    Layout.fillWidth: true
                    visible: editor.info.spread === 2
                    key: "phaseSpread"
                    from: 0; to: 1440
                    value: editor.info.phaseSpread >= 0 ? editor.info.phaseSpread : 360
                }
                ValueText { text: Math.round(spreadSlider.value) + "°"; visible: editor.info.spread === 2 }

                Text { text: qsTr("Symmetric"); color: Theme.textDim }
                AppSwitch { checked: !!editor.info.symmetric; onToggled: editor.set("symmetric", checked) }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: qsTr("The second half of the fixtures mirrors the first one, from the outside in")
                    color: Theme.textFaint
                    font.pixelSize: Theme.fontSmall
                }

                Text { text: qsTr("Relative"); color: Theme.textDim }
                AppSwitch { checked: !!editor.info.relative; onToggled: editor.set("relative", checked) }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: qsTr("Moves around the position set by other scenes")
                    color: Theme.textFaint
                    font.pixelSize: Theme.fontSmall
                }

                Text { text: qsTr("Control dimmer"); color: Theme.textDim }
                AppSwitch { checked: !!editor.info.dimmer; onToggled: editor.set("dimmer", checked) }
                Item { Layout.fillWidth: true }
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
