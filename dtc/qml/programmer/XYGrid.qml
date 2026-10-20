/*
  DMX Total Control
  XYGrid.qml

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
 * Pan and tilt of the selection (XY grid): drag the cross to place it, or
 * drag anywhere else for a slow, fine move; the same bar for each beside
 * and below. A dot for each fixture. Beside it the fan: each fixture moves
 * from where it is, in the order of the selection; with «Fan» on, a drag on
 * the grid opens it (across the pan, up and down the tilt). With «Follow»
 * on, a point picked on the stage plan instead: every moving head aims there.
 */
RowLayout {
    id: xy

    readonly property var position: Core.programmer.position
    readonly property real px: position.x !== undefined ? position.x : 0.5
    readonly property real py: position.y !== undefined ? position.y : 0.5
    readonly property var points: position.points || []
    readonly property var fan: Core.programmer.positionFan
    readonly property string fanMode: fan.mode || "spread"
    /** A drag on the grid opens the fan instead of moving */
    property bool fanDrag: false
    /** Follow: the heads aim at a point picked on the stage plan */
    property bool follow: false
    readonly property var aim: Core.programmer.aim
    /** The height of that point (m over the stage floor) */
    property real aimHeight: 0

    function aimAgain() {
        if (aim.active)
            Core.programmer.aimAt(aim.x, aim.y, aimHeight * 1000)
    }

    function setFan(pan, tilt, mode) {
        Core.programmer.setPositionFan(Math.max(-1, Math.min(1, pan)), Math.max(-1, Math.min(1, tilt)), mode)
    }

    spacing: 16
    enabled: !!position.available

    ColumnLayout {
        Layout.alignment: Qt.AlignTop
        spacing: 6

        RowLayout {
            spacing: 6
            // tilt: up is the top of the grid
            HSlider {
                id: tiltSlider
                orientation: Qt.Vertical
                Layout.preferredHeight: 220
                from: 0; to: 1
                value: 1 - xy.py
                onMoved: Core.programmer.setPosition(xy.px, 1 - value)
            }
            Rectangle {
                id: grid
                Layout.preferredWidth: 220
                Layout.preferredHeight: 220
                radius: Theme.radiusSmall
                color: Theme.background
                border.color: xy.fanDrag ? Theme.warning : Theme.border

                // quarters
                Repeater {
                    model: 3
                    Rectangle { required property int index; x: grid.width * (index + 1) / 4; width: 1; height: grid.height; color: Theme.border }
                }
                Repeater {
                    model: 3
                    Rectangle { required property int index; y: grid.height * (index + 1) / 4; height: 1; width: grid.width; color: Theme.border }
                }
                // each fixture (when there are more: the fan shows)
                Repeater {
                    model: xy.points.length > 1 ? xy.points : []
                    Rectangle {
                        required property var modelData
                        x: modelData.x * grid.width - width / 2
                        y: modelData.y * grid.height - height / 2
                        width: 8; height: 8; radius: 4
                        color: Theme.textDim
                        opacity: 0.8
                    }
                }
                // the cross
                Rectangle { x: xy.px * grid.width; width: 1; height: grid.height; color: Theme.accent }
                Rectangle { y: xy.py * grid.height; height: 1; width: grid.width; color: Theme.accent }
                Rectangle {
                    x: xy.px * grid.width - width / 2
                    y: xy.py * grid.height - height / 2
                    width: 16; height: 16; radius: 8
                    color: Theme.accent
                    border.color: Theme.accentText
                    border.width: 2
                }

                MouseArea {
                    anchors.fill: parent
                    preventStealing: true
                    property bool fine: false
                    property point last
                    property point start
                    property real startPan: 0
                    property real startTilt: 0
                    onPressed: (ev) => {
                        last = Qt.point(ev.x, ev.y)
                        start = last
                        // the fan: from where it is
                        if (xy.fanDrag) {
                            startPan = xy.fan.pan || 0
                            startTilt = xy.fan.tilt || 0
                            return
                        }
                        // on the cross: it follows the mouse; elsewhere: a quarter of the move
                        let dx = ev.x - xy.px * grid.width, dy = ev.y - xy.py * grid.height
                        fine = Math.sqrt(dx * dx + dy * dy) > 14
                        if (!fine)
                            Core.programmer.setPosition(ev.x / grid.width, ev.y / grid.height)
                    }
                    onPositionChanged: (ev) => {
                        if (xy.fanDrag)
                            xy.setFan(startPan + (ev.x - start.x) / grid.width * 2,
                                      startTilt + (ev.y - start.y) / grid.height * 2, xy.fanMode)
                        else if (fine)
                            Core.programmer.setPosition(xy.px + (ev.x - last.x) / grid.width / 4,
                                                        xy.py + (ev.y - last.y) / grid.height / 4)
                        else
                            Core.programmer.setPosition(ev.x / grid.width, ev.y / grid.height)
                        last = Qt.point(ev.x, ev.y)
                    }
                }
            }
        }
        // pan
        HSlider {
            Layout.leftMargin: tiltSlider.width + 6
            Layout.preferredWidth: 220
            from: 0; to: 1
            value: xy.px
            onMoved: Core.programmer.setPosition(value, xy.py)
        }
        Text {
            Layout.leftMargin: tiltSlider.width + 6
            text: qsTr("Pan %1%   Tilt %2%").arg(Math.round(xy.px * 100)).arg(Math.round(xy.py * 100))
            color: Theme.textDim
            font.family: Theme.monoFamily
            font.pixelSize: Theme.fontSmall
        }
        Row {
            Layout.leftMargin: tiltSlider.width + 6
            spacing: 6
            AppButton {
                text: qsTr("Centre")
                iconGlyph: Theme.iconTarget
                implicitHeight: 28
                onClicked: Core.programmer.centreBeam()
            }
            AppButton {
                text: qsTr("Follow")
                iconGlyph: Theme.iconLocate
                implicitHeight: 28
                highlighted: xy.follow
                onClicked: {
                    xy.follow = !xy.follow
                    if (xy.follow)
                        xy.fanDrag = false
                }
                ToolTip.text: qsTr("The moving heads aim at a point of the stage plan")
                ToolTip.visible: hovered
                ToolTip.delay: 500
            }
            AppButton {
                text: qsTr("Fan")
                iconGlyph: Theme.iconArrowsH
                implicitHeight: 28
                highlighted: xy.fanDrag
                accentColor: Theme.warning
                onClicked: {
                    xy.fanDrag = !xy.fanDrag
                    if (xy.fanDrag)
                        xy.follow = false
                }
                ToolTip.text: qsTr("Drag on the grid to open the fan: across, the pan; up and down, the tilt")
                ToolTip.visible: hovered
                ToolTip.delay: 500
            }
        }
    }

    // Follow: pick the point on the plan, and its height
    ColumnLayout {
        visible: xy.follow
        Layout.alignment: Qt.AlignTop
        Layout.preferredWidth: 340
        Layout.fillWidth: true
        Layout.maximumWidth: 520
        spacing: 8

        Text {
            text: qsTr("Follow")
            color: Theme.textDim
            font.pixelSize: Theme.fontSmall
            font.weight: Font.Bold
        }
        StagePlan {
            Layout.fillWidth: true
            Layout.preferredHeight: 230
            fixtures: Core.patch.fixtures
            selection: Core.programmer.selection
            aiming: true
            showNames: false
            target: xy.aim.active ? ({ x: xy.aim.x, y: xy.aim.y }) : null
            aimedIds: xy.aim.active ? xy.aim.aimed : []
            onPointPicked: (x, y) => Core.programmer.aimAt(x, y, xy.aimHeight * 1000)
        }
        RowLayout {
            spacing: 8
            Text {
                text: qsTr("Height of the point")
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
            }
            // tenths of a metre, shown in metres
            SpinBox {
                from: 0; to: 100; stepSize: 1
                editable: true
                implicitWidth: 120
                implicitHeight: 28
                value: Math.round(xy.aimHeight * 10)
                textFromValue: (v, locale) => (v / 10).toLocaleString(locale, 'f', 1) + " m"
                valueFromText: (text, locale) => Math.round(Theme.parseNumber(text, value / 10) * 10)
                validator: RegularExpressionValidator { regularExpression: /[0-9]+([.,][0-9])?( ?m)?/ }
                onValueModified: {
                    xy.aimHeight = value / 10
                    xy.aimAgain()
                }
            }
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            font.pixelSize: Theme.fontSmall
            color: xy.aim.active && (xy.aim.unreachable.length > 0 || xy.aim.guessed.length > 0) ? Theme.warning : Theme.textDim
            text: {
                if (!xy.aim.active)
                    return qsTr("Click or drag on the stage: the selected moving heads aim there. Their height and the way they face are set in Patch › 2D plan.")
                let parts = [qsTr("%n moving head(s) aim at the point.", "", xy.aim.aimed.length)]
                if (xy.aim.unreachable.length > 0)
                    parts.push(qsTr("%n can't reach it: as near as they can.", "", xy.aim.unreachable.length))
                if (xy.aim.guessed.length > 0)
                    parts.push(qsTr("%n without pan and tilt travel in their definition: 540° and 270° are used.", "", xy.aim.guessed.length))
                if (xy.aim.skipped.length > 0)
                    parts.push(qsTr("%n scanner(s) left as they are.", "", xy.aim.skipped.length))
                return parts.join(" ")
            }
        }
    }

    // the fan of the position
    ColumnLayout {
        visible: !xy.follow
        Layout.alignment: Qt.AlignTop
        Layout.preferredWidth: 200
        spacing: 8

        Text {
            text: qsTr("Fan")
            color: Theme.textDim
            font.pixelSize: Theme.fontSmall
            font.weight: Font.Bold
        }
        // how the fixtures share it out, in the order of the selection
        Flow {
            Layout.fillWidth: true
            spacing: 4
            Repeater {
                model: [
                    { mode: "spread", name: qsTr("Open"), tip: qsTr("From one end to the other") },
                    { mode: "v", name: qsTr("V"), tip: qsTr("The ends out, the middle still") },
                    { mode: "first", name: qsTr("From the first"), tip: qsTr("From the first one, more and more") }
                ]
                AppButton {
                    required property var modelData
                    text: modelData.name
                    implicitHeight: 28
                    flat: !highlighted
                    highlighted: xy.fanMode === modelData.mode
                    onClicked: xy.setFan(xy.fan.pan || 0, xy.fan.tilt || 0, modelData.mode)
                    ToolTip.text: modelData.tip
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                }
            }
        }
        Repeater {
            model: [ { key: "pan", name: qsTr("Pan") }, { key: "tilt", name: qsTr("Tilt") } ]
            RowLayout {
                required property var modelData
                Layout.fillWidth: true
                spacing: 6
                Text {
                    Layout.preferredWidth: 30
                    text: modelData.name
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                }
                HSlider {
                    Layout.fillWidth: true
                    from: -1; to: 1
                    resetValue: 0
                    value: xy.fan[modelData.key] || 0
                    fillColor: value !== 0 ? Theme.warning : Theme.accent
                    onMoved: modelData.key === "pan" ? xy.setFan(value, xy.fan.tilt || 0, xy.fanMode)
                                                     : xy.setFan(xy.fan.pan || 0, value, xy.fanMode)
                }
                Text {
                    Layout.preferredWidth: 38
                    horizontalAlignment: Text.AlignRight
                    text: Math.round((xy.fan[modelData.key] || 0) * 100) + "%"
                    color: (xy.fan[modelData.key] || 0) !== 0 ? Theme.warning : Theme.textDim
                    font.family: Theme.monoFamily
                    font.pixelSize: Theme.fontSmall
                }
            }
        }
        AppButton {
            text: qsTr("No fan")
            implicitHeight: 28
            enabled: (xy.fan.pan || 0) !== 0 || (xy.fan.tilt || 0) !== 0
            onClicked: xy.setFan(0, 0, xy.fanMode)
            ToolTip.text: qsTr("The fixtures back where they were")
            ToolTip.visible: hovered
            ToolTip.delay: 500
        }
    }
}
