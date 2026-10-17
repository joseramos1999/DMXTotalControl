/*
  DMX Total Control
  MatrixDialog.qml

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
 * LED matrices and strips in one go: the size, the colour order
 * of the pixels and how they are wired. It patches one fixture per line of
 * pixels and a fixture group that knows where each pixel is.
 */
Popup {
    id: dialog

    readonly property var componentList: ["RGB", "GRB", "BGR", "BRG", "GBR", "RBG", "RGBW"]
    readonly property var wiringList: ["rows", "snake", "columns", "snakeColumns"]
    readonly property var startList: ["topLeft", "topRight", "bottomLeft", "bottomRight"]

    readonly property int columns: columnsSpin.value
    readonly property int rows: rowsSpin.value
    readonly property string wiring: wiringList[wiringCombo.currentIndex]
    readonly property string start: startList[startCombo.currentIndex]
    readonly property bool vertical: wiring === "columns" || wiring === "snakeColumns"
    readonly property int perPixel: componentCombo.currentText === "RGBW" ? 4 : 3
    readonly property int lineChannels: (vertical ? rows : columns) * perPixel
    property string error: ""

    signal message(string text)

    function refreshAddress() {
        let free = Core.patch.nextFreeAddress(universeSpin.value - 1, Math.min(512, lineChannels))
        if (free > 0)
            addressSpin.value = free
    }

    // the pixels in wiring order, like the patch does it
    function pixelOrder() {
        let snake = wiring.startsWith("snake")
        let fromRight = start.endsWith("Right"), fromBottom = start.startsWith("bottom")
        let lines = vertical ? columns : rows, length = vertical ? rows : columns
        let points = []
        for (let line = 0; line < lines; line++) {
            let across = vertical ? (fromRight ? columns - 1 - line : line) : (fromBottom ? rows - 1 - line : line)
            let backwards = vertical ? fromBottom : fromRight
            if (snake && line % 2 === 1)
                backwards = !backwards
            for (let h = 0; h < length; h++) {
                let along = backwards ? length - 1 - h : h
                points.push(vertical ? Qt.point(across, along) : Qt.point(along, across))
            }
        }
        return points
    }

    onOpened: { error = ""; refreshAddress() }
    onLineChannelsChanged: refreshAddress()

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(820, parent.width - 80)
    height: Math.min(560, parent.height - 80)
    modal: true
    padding: 18
    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    contentItem: ColumnLayout {
        spacing: 12

        Text {
            text: qsTr("LED matrix or strip")
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            GridLayout {
                Layout.fillWidth: false
                Layout.preferredWidth: 320
                Layout.maximumWidth: 320
                Layout.alignment: Qt.AlignTop
                columns: 2
                columnSpacing: 10
                rowSpacing: 8

                Text { text: qsTr("Name"); color: Theme.textDim }
                TextField {
                    id: nameField
                    Layout.fillWidth: true
                    placeholderText: dialog.rows === 1 ? qsTr("LED strip") : qsTr("LED matrix")
                    color: Theme.text
                    background: Rectangle { radius: Theme.radiusSmall; color: Theme.background; border.color: nameField.activeFocus ? Theme.accent : Theme.border }
                }

                Text { text: qsTr("Width (pixels)"); color: Theme.textDim }
                SpinBox { id: columnsSpin; Layout.fillWidth: true; from: 1; to: 512; value: 16; editable: true }

                Text { text: qsTr("Height (pixels)"); color: Theme.textDim }
                SpinBox { id: rowsSpin; Layout.fillWidth: true; from: 1; to: 512; value: 8; editable: true }

                Text { text: qsTr("Colours"); color: Theme.textDim }
                ComboBox { id: componentCombo; Layout.fillWidth: true; model: dialog.componentList }

                Text { text: qsTr("Wiring"); color: Theme.textDim }
                ComboBox {
                    id: wiringCombo
                    Layout.fillWidth: true
                    model: [qsTr("Rows"), qsTr("Rows, snake"), qsTr("Columns"), qsTr("Columns, snake")]
                    currentIndex: 1
                }

                Text { text: qsTr("Pixel 1"); color: Theme.textDim }
                ComboBox {
                    id: startCombo
                    Layout.fillWidth: true
                    model: [qsTr("Top left"), qsTr("Top right"), qsTr("Bottom left"), qsTr("Bottom right")]
                }

                Text { text: qsTr("Universe"); color: Theme.textDim }
                SpinBox {
                    id: universeSpin
                    Layout.fillWidth: true
                    from: 1; to: Math.max(1, Core.io.universes.length); value: 1
                    editable: true
                    onValueModified: dialog.refreshAddress()
                }

                Text { text: qsTr("Address"); color: Theme.textDim }
                SpinBox { id: addressSpin; Layout.fillWidth: true; from: 1; to: 512; value: 1; editable: true }

                Text {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: qsTr("%1 pixels, %2 channels: %3 fixtures of %4 channels (one per line of pixels), and a group for the colour effects.")
                          .arg(dialog.columns * dialog.rows)
                          .arg(dialog.columns * dialog.rows * dialog.perPixel)
                          .arg(dialog.vertical ? dialog.columns : dialog.rows)
                          .arg(dialog.lineChannels)
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                }
                Text {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    visible: dialog.error.length > 0 || dialog.lineChannels > 512
                    wrapMode: Text.WordWrap
                    text: dialog.lineChannels > 512 ? qsTr("A line of pixels needs more than one universe: make it shorter or wire it the other way.")
                                                    : dialog.error
                    color: Theme.danger
                    font.pixelSize: Theme.fontSmall
                }
            }

            // the wiring, as seen from the front
            Panel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: qsTr("Pixel order")

                Item {
                    id: preview
                    anchors { fill: parent; margins: 14 }
                    readonly property int shownColumns: Math.min(dialog.columns, 32)
                    readonly property int shownRows: Math.min(dialog.rows, 32)
                    readonly property real cell: Math.max(4, Math.min(width / shownColumns, height / shownRows))
                    readonly property var order: dialog.pixelOrder()

                    Canvas {
                        id: wires
                        // the colours of the theme
                        readonly property string themeName: Theme.name
                        onThemeNameChanged: requestPaint()
                        width: preview.shownColumns * preview.cell
                        height: preview.shownRows * preview.cell
                        anchors.centerIn: parent
                        onPaint: {
                            let ctx = getContext("2d")
                            ctx.reset()
                            let c = preview.cell
                            // the cells
                            ctx.fillStyle = Theme.surfaceRaised
                            for (let x = 0; x < preview.shownColumns; x++)
                                for (let y = 0; y < preview.shownRows; y++)
                                    ctx.fillRect(x * c + 1, y * c + 1, c - 2, c - 2)
                            // the wire, from pixel 1
                            ctx.strokeStyle = Theme.accent
                            ctx.lineWidth = Math.max(1, c / 8)
                            ctx.beginPath()
                            let first = true
                            for (let p of preview.order) {
                                if (p.x >= preview.shownColumns || p.y >= preview.shownRows)
                                    continue
                                let px = p.x * c + c / 2, py = p.y * c + c / 2
                                if (first) ctx.moveTo(px, py)
                                else ctx.lineTo(px, py)
                                first = false
                            }
                            ctx.stroke()
                            // pixel 1
                            if (preview.order.length) {
                                let p = preview.order[0]
                                ctx.fillStyle = Theme.success
                                ctx.beginPath()
                                ctx.arc(p.x * c + c / 2, p.y * c + c / 2, Math.max(3, c / 3), 0, 2 * Math.PI)
                                ctx.fill()
                            }
                        }
                        Connections {
                            target: preview
                            function onOrderChanged() { wires.requestPaint() }
                            function onCellChanged() { wires.requestPaint() }
                        }
                    }
                    Text {
                        anchors { right: parent.right; bottom: parent.bottom }
                        visible: dialog.columns > 32 || dialog.rows > 32
                        text: qsTr("Showing the first 32 × 32")
                        color: Theme.textFaint
                        font.pixelSize: Theme.fontSmall
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            AppButton {
                text: qsTr("Strip")
                flat: true
                ToolTip.text: qsTr("One line of pixels")
                ToolTip.visible: hovered
                onClicked: { commitEdits(); rowsSpin.value = 1; columnsSpin.value = Math.max(columnsSpin.value, 30); wiringCombo.currentIndex = 0 }
            }
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Close"); onClicked: dialog.close() }
            AppButton {
                text: qsTr("Patch")
                iconGlyph: Theme.iconPlus
                primary: true
                enabled: dialog.lineChannels <= 512
                onClicked: {
                    commitEdits()
                    dialog.error = Core.patch.addMatrix(nameField.text, dialog.columns, dialog.rows, componentCombo.currentText,
                                                        universeSpin.value - 1, addressSpin.value, dialog.wiring, dialog.start)
                    if (dialog.error.length === 0) {
                        dialog.message(qsTr("Patched %1 × %2 pixels").arg(dialog.columns).arg(dialog.rows))
                        dialog.close()
                    }
                }
            }
        }
    }
}
