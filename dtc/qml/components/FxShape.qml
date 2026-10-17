/*
  DMX Total Control
  FxShape.qml

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

/** The drawing of an FX mode (the wave around the base line) or of a pan/tilt figure */
Canvas {
    id: shape
    // the colours of the theme
    readonly property string themeName: Theme.name
    onThemeNameChanged: requestPaint()

    /** a figure (pan/tilt) instead of a mode */
    property bool figure: false
    property int value: 1
    property color color: Theme.accent
    property real lineWidth: 1.6

    implicitWidth: 56
    implicitHeight: 32

    readonly property var points: figure ? Core.effects.fxFigurePath(value, 64) : Core.effects.fxModeCurve(value, 64)
    onPointsChanged: requestPaint()
    onColorChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()

    onPaint: {
        let ctx = getContext("2d")
        ctx.reset()
        let pad = 3
        let w = width - pad * 2, h = height - pad * 2
        if (w <= 0 || h <= 0)
            return
        ctx.lineWidth = lineWidth
        ctx.lineJoin = "round"
        if (figure) {
            // the figure in a square, the centre is the base position
            let side = Math.min(w, h)
            let cx = pad + w / 2, cy = pad + h / 2
            ctx.strokeStyle = Theme.border
            ctx.beginPath(); ctx.moveTo(cx - side / 2, cy); ctx.lineTo(cx + side / 2, cy); ctx.stroke()
            ctx.beginPath(); ctx.moveTo(cx, cy - side / 2); ctx.lineTo(cx, cy + side / 2); ctx.stroke()
            ctx.strokeStyle = color
            ctx.beginPath()
            for (let i = 0; i < points.length; i++) {
                let x = cx + points[i].x * side / 2 * 0.9
                let y = cy - points[i].y * side / 2 * 0.9
                if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
            }
            ctx.stroke()
            // where it starts
            if (points.length > 0) {
                ctx.fillStyle = color
                ctx.beginPath()
                ctx.arc(cx + points[0].x * side / 2 * 0.9, cy - points[0].y * side / 2 * 0.9, 2.2, 0, Math.PI * 2)
                ctx.fill()
            }
        } else {
            // the base line in the middle, the wave around it
            let mid = pad + h / 2
            ctx.strokeStyle = Theme.borderStrong
            ctx.setLineDash([2, 2])
            ctx.beginPath(); ctx.moveTo(pad, mid); ctx.lineTo(pad + w, mid); ctx.stroke()
            ctx.setLineDash([])
            ctx.strokeStyle = color
            ctx.beginPath()
            for (let i = 0; i <= points.length; i++) {
                let v = points[i % points.length]
                let x = pad + w * i / points.length
                let y = mid - v * h / 2 * 0.92
                if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
            }
            ctx.stroke()
        }
    }
}
