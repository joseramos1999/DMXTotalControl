/*
  DMX Total Control
  SurfaceDial.qml

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
 * A dial of the SHOW screen: it drives what a fader drives.
 * As an incremental wheel ("encoder") it turns forever: each notch adds or
 * takes "step" % from where the target is.
 */
Rectangle {
    id: cell

    property var item: ({})
    property bool editing: false

    readonly property var targets: Core.surface.faderTargets(item)
    readonly property var first: targets.length ? targets[0] : ({ target: "grandMaster" })
    readonly property bool encoder: !!item.encoder
    readonly property bool isBpm: targets.length === 1 && first.target === "bpm"
    readonly property real level: { Core.surface.revision; Core.grandMaster; Core.bpm; Core.mixer.mixers; return Core.surface.level(item) }
    // the wheel turns with the hand, whatever the level
    property real wheelAngle: 0

    function targetName(t) {
        switch (t.target) {
        case "grandMaster": return qsTr("Grand master")
        case "bpm": return qsTr("BPM")
        case "group": {
            let zone = Core.zones.zoneById(t.zoneId)
            return zone ? zone.name : qsTr("Choose a group")
        }
        case "scene": return Core.functionInfo(t.functionId).name || qsTr("Choose a scene")
        case "sceneSpeed": return qsTr("Speed of %1").arg(Core.functionInfo(t.functionId).name || "?")
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
    function label() { return item.label ? item.label : targets.map(t => targetName(t)).join(" + ") }
    function setLevel(v) { Core.surface.setLevel(item, Math.max(0, Math.min(1, v))) }
    /** a turn of $degrees: a dial moves 270° for the whole range, a wheel adds a notch every 15° */
    function turn(degrees) {
        if (encoder) {
            wheelAngle += degrees
            let notches = Math.trunc(wheelAngle / 15)
            if (notches !== 0) {
                wheelAngle -= notches * 15
                setLevel(level + notches * (item.step || 2) / 100)
            }
            knob.requestPaint()
        } else {
            setLevel(level + degrees / 270)
        }
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

        Canvas {
            id: knob
            Layout.fillWidth: true
            Layout.fillHeight: true
            readonly property real level: cell.level
            readonly property color tint: cell.item.color || Theme.accent
            onLevelChanged: requestPaint()
            onTintChanged: requestPaint()
            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()

            onPaint: {
                let ctx = getContext("2d")
                ctx.reset()
                let r = Math.min(width, height) / 2 - 6
                if (r <= 4)
                    return
                let cx = width / 2, cy = height / 2
                let start = Math.PI * 0.75, span = Math.PI * 1.5
                if (cell.encoder) {
                    // an endless wheel with notches; the arc around it is the level
                    ctx.fillStyle = Theme.background
                    ctx.beginPath(); ctx.arc(cx, cy, r, 0, Math.PI * 2); ctx.fill()
                    ctx.strokeStyle = Theme.borderStrong
                    ctx.lineWidth = 2
                    let a0 = cell.wheelAngle * Math.PI / 180
                    for (let i = 0; i < 24; i++) {
                        let a = a0 + i * Math.PI / 12
                        ctx.beginPath()
                        ctx.moveTo(cx + Math.cos(a) * r * 0.62, cy + Math.sin(a) * r * 0.62)
                        ctx.lineTo(cx + Math.cos(a) * r * 0.86, cy + Math.sin(a) * r * 0.86)
                        ctx.stroke()
                    }
                    ctx.strokeStyle = tint
                    ctx.lineWidth = 4
                    ctx.beginPath(); ctx.arc(cx, cy, r + 2, start, start + span * level); ctx.stroke()
                } else {
                    // a knob: the track, the level arc and the pointer
                    ctx.lineWidth = 6
                    ctx.lineCap = "round"
                    ctx.strokeStyle = Theme.background
                    ctx.beginPath(); ctx.arc(cx, cy, r, start, start + span); ctx.stroke()
                    ctx.strokeStyle = tint
                    ctx.beginPath(); ctx.arc(cx, cy, r, start, start + span * level); ctx.stroke()
                    ctx.fillStyle = Theme.surfaceHover
                    ctx.beginPath(); ctx.arc(cx, cy, r * 0.72, 0, Math.PI * 2); ctx.fill()
                    let a = start + span * level
                    ctx.strokeStyle = Theme.text
                    ctx.lineWidth = 3
                    ctx.beginPath()
                    ctx.moveTo(cx + Math.cos(a) * r * 0.25, cy + Math.sin(a) * r * 0.25)
                    ctx.lineTo(cx + Math.cos(a) * r * 0.65, cy + Math.sin(a) * r * 0.65)
                    ctx.stroke()
                }
            }

            // turn it round with the mouse or a finger, or drag up and down
            MouseArea {
                anchors.fill: parent
                enabled: !cell.editing
                property real lastAngle: 0
                property real lastY: 0
                function angleAt(x, y) { return Math.atan2(y - height / 2, x - width / 2) * 180 / Math.PI }
                onPressed: (ev) => { lastAngle = angleAt(ev.x, ev.y); lastY = ev.y }
                onPositionChanged: (ev) => {
                    let a = angleAt(ev.x, ev.y)
                    let d = a - lastAngle
                    if (d > 180) d -= 360
                    if (d < -180) d += 360
                    // near the centre the angle jumps: up and down instead
                    let dist = Math.hypot(ev.x - width / 2, ev.y - height / 2)
                    if (dist < Math.min(width, height) * 0.15)
                        d = (lastY - ev.y) * 1.5
                    lastAngle = a
                    lastY = ev.y
                    cell.turn(d)
                }
                onWheel: (ev) => cell.turn(ev.angleDelta.y > 0 ? 15 : -15)
                onDoubleClicked: if (!cell.encoder) cell.setLevel(cell.isBpm ? 70 / 280 : 0)
            }
        }

        Text {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: cell.isBpm ? Math.round(20 + cell.level * 280) : Math.round(cell.level * 100) + "%"
            color: Theme.textDim
            font.family: Theme.monoFamily
            font.pixelSize: 10
        }
    }
}
