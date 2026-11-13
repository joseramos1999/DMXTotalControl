/*
  DMX Total Control
  QrCode.qml

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

/** The QR code of a text (an address), to scan with the camera of a phone:
 *  black on white with the quiet zone of 4 modules around it. The modules
 *  are square and on whole pixels of the screen, without antialiasing, so
 *  that the camera reads them at any scale of the interface */
Canvas {
    id: code

    property string text: ""
    /** '1' dark, '0' light, row by row */
    readonly property var rows: text.length ? Core.remote.qrCode(text) : []

    /** Pixels of the screen for one of the item (the scale of the interface) */
    readonly property real pixelRatio: Screen.devicePixelRatio > 0 ? Screen.devicePixelRatio : 1

    implicitWidth: 168
    implicitHeight: 168
    antialiasing: false
    // (nor smoothed on the screen, where the item lands between pixels)
    smooth: false
    // (dimmed in the night theme, as the colours of the lights, not to dazzle)
    opacity: Math.max(0.75, Theme.lightLevel)

    onRowsChanged: requestPaint()
    onPixelRatioChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()

    onPaint: {
        const ctx = getContext("2d")
        ctx.reset()
        ctx.fillStyle = "#ffffff"
        ctx.fillRect(0, 0, width, height)
        const n = rows.length
        if (n === 0)
            return
        // the module in pixels of the screen, then back to the ones of the item
        const ratio = pixelRatio
        const module = Math.max(1, Math.floor(Math.min(width, height) * ratio / (n + 8))) / ratio
        const left = Math.round((width - n * module) / 2 * ratio) / ratio
        const top = Math.round((height - n * module) / 2 * ratio) / ratio
        ctx.fillStyle = "#000000"
        // (each run of dark modules of a row at once)
        for (let y = 0; y < n; ++y) {
            const row = rows[y]
            let x = 0
            while (x < n) {
                if (row[x] !== "1") {
                    ++x
                    continue
                }
                let end = x + 1
                while (end < n && row[end] === "1")
                    ++end
                ctx.fillRect(left + x * module, top + y * module, (end - x) * module, module)
                x = end
            }
        }
    }
}
