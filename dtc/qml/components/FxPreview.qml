/*
  DMX Total Control
  FxPreview.qml

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
 * An FX in motion, fixture by fixture: a bar each (the wave around the base)
 * or, for pan/tilt, a dot each on the figure. $sample(cyclePos) gives the
 * fixtures at a point of the effect.
 */
Canvas {
    id: preview

    property var fx: ({})
    /** function(cyclePos) -> [{ value } or { x, y }] */
    property var sample: null
    property bool running: visible

    property real cyclePos: 0
    property var items: []

    readonly property var colours: ["#ae3ec9", "#3d8bfd", "#30a46c", "#f5a524", "#e5484d", "#17a2b8", "#d6336c", "#e7e9ef"]

    Timer {
        interval: 40
        repeat: true
        running: preview.running && preview.sample !== null
        onTriggered: {
            let bpm = preview.fx.syncBpm ? Core.bpm : (preview.fx.bpm || 0)
            preview.cyclePos += 0.04 / 60 * bpm * (preview.fx.multiplier || 1)
            preview.items = preview.sample(preview.cyclePos) || []
            preview.requestPaint()
        }
    }

    onPaint: {
        let ctx = getContext("2d")
        ctx.reset()
        ctx.fillStyle = Theme.background
        ctx.fillRect(0, 0, width, height)
        let n = items.length
        if (n === 0)
            return

        if (preview.fx.panTilt) {
            // each fixture is a dot around its base position
            let side = Math.min(width, height) - 16
            let cx = width / 2, cy = height / 2
            ctx.strokeStyle = Theme.border
            ctx.lineWidth = 1
            ctx.strokeRect(cx - side / 2, cy - side / 2, side, side)
            ctx.beginPath(); ctx.moveTo(cx - side / 2, cy); ctx.lineTo(cx + side / 2, cy); ctx.stroke()
            ctx.beginPath(); ctx.moveTo(cx, cy - side / 2); ctx.lineTo(cx, cy + side / 2); ctx.stroke()
            for (let i = 0; i < n; i++) {
                let p = items[i]
                ctx.fillStyle = colours[i % colours.length]
                ctx.beginPath()
                ctx.arc(cx + (p.x || 0) * side / 2 * 0.9, cy - (p.y || 0) * side / 2 * 0.9, 5, 0, Math.PI * 2)
                ctx.fill()
            }
            return
        }

        // a bar for each fixture, up or down from the base line
        let gap = 3
        let w = Math.max(3, (width - gap * (n + 1)) / n)
        let mid = height / 2
        ctx.strokeStyle = Theme.borderStrong
        ctx.setLineDash([3, 3])
        ctx.beginPath(); ctx.moveTo(0, mid); ctx.lineTo(width, mid); ctx.stroke()
        ctx.setLineDash([])
        for (let i = 0; i < n; i++) {
            let v = items[i].value || 0
            let x = gap + i * (w + gap)
            let h = v * (height / 2 - 4)
            ctx.fillStyle = colours[i % colours.length]
            if (h >= 0)
                ctx.fillRect(x, mid - h, w, Math.max(1, h))
            else
                ctx.fillRect(x, mid, w, Math.max(1, -h))
        }
    }
}
