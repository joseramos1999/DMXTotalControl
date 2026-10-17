/*
  DMX Total Control
  PatchGrid.qml

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
 * The patch as a grid of the 512 channels of a universe: each
 * fixture covers its channels. Drag a fixture to give it a new address.
 */
Item {
    id: grid

    property var fixtures: []
    property var selection: []
    property int universe: 0

    signal selectRequested(var ids, bool additive)
    signal editRequested(var fixture)
    signal message(string text, bool isError)

    readonly property int columns: 32
    readonly property real cellWidth: area.width / columns
    readonly property real cellHeight: area.height / (512 / columns)
    readonly property var here: fixtures.filter(f => f.universe === universe)

    // dragging a fixture: where it would start
    property var dragged: null
    property int dragStart: -1
    property int grabOffset: 0
    readonly property bool dragValid: dragged !== null && rangeFree(dragStart, dragged.channels, dragged.id)

    function fixtureAt(channel) {
        for (let f of here)
            if (channel >= f.address - 1 && channel < f.address - 1 + f.channels)
                return f
        return null
    }
    function rangeFree(start, count, ignoreId) {
        if (start < 0 || start + count > 512)
            return false
        for (let f of here)
            if (f.id !== ignoreId && start < f.address - 1 + f.channels && f.address - 1 < start + count)
                return false
        return true
    }
    // the rows of the grid a range of channels covers
    function segments(start, count) {
        let out = []
        let ch = start
        while (ch < start + count) {
            let row = Math.floor(ch / columns)
            let end = Math.min(start + count, (row + 1) * columns)
            out.push({ row: row, from: ch % columns, to: (end - 1) % columns, first: ch === start })
            ch = end
        }
        return out
    }
    function tint(f) {
        let key = f.manufacturer + f.model + f.type
        let h = 0
        for (let i = 0; i < key.length; i++)
            h = (h * 31 + key.charCodeAt(i)) % 360
        // dark blocks on a dark theme, pale ones on a light theme
        return Theme.isDark ? Theme.category(Qt.hsla(h / 360, 0.45, 0.32, 1)) : Qt.hsla(h / 360, 0.5, 0.82, 1)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        // universes
        RowLayout {
            Layout.fillWidth: true
            spacing: 4
            Repeater {
                model: Core.io.universes
                delegate: AppButton {
                    required property var modelData
                    required property int index
                    text: modelData.name
                    implicitHeight: 28
                    highlighted: grid.universe === index
                    onClicked: grid.universe = index
                }
            }
            Item { Layout.fillWidth: true }
            Text {
                text: qsTr("%1 of 512 channels used").arg(grid.here.reduce((sum, f) => sum + f.channels, 0))
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
            }
        }

        Item {
            id: area
            Layout.fillWidth: true
            Layout.fillHeight: true

            // the 512 channels
            Repeater {
                model: 512
                Rectangle {
                    required property int index
                    x: (index % grid.columns) * grid.cellWidth
                    y: Math.floor(index / grid.columns) * grid.cellHeight
                    width: grid.cellWidth - 1
                    height: grid.cellHeight - 1
                    color: Theme.surfaceRaised
                    Text {
                        anchors { left: parent.left; top: parent.top; margins: 2 }
                        text: parent.index + 1
                        color: Theme.textFaint
                        font.pixelSize: 8
                        visible: grid.cellWidth > 18
                    }
                }
            }

            // the fixtures over their channels
            Repeater {
                model: grid.here
                delegate: Item {
                    id: fixtureItem
                    required property var modelData
                    readonly property bool selected: grid.selection.indexOf(modelData.id) >= 0
                    readonly property bool moving: grid.dragged !== null && grid.dragged.id === modelData.id
                    opacity: moving ? 0.35 : 1
                    Repeater {
                        model: grid.segments(fixtureItem.modelData.address - 1, fixtureItem.modelData.channels)
                        Rectangle {
                            required property var modelData
                            x: modelData.from * grid.cellWidth
                            y: modelData.row * grid.cellHeight
                            width: (modelData.to - modelData.from + 1) * grid.cellWidth - 1
                            height: grid.cellHeight - 1
                            radius: 3
                            color: fixtureItem.selected ? Theme.accent : grid.tint(fixtureItem.modelData)
                            border.color: fixtureItem.selected ? Qt.lighter(Theme.accent, 1.4) : Theme.isDark ? Qt.lighter(color, 1.5) : Qt.darker(color, 1.3)
                            clip: true
                            Text {
                                anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 4 }
                                visible: parent.modelData.first
                                width: parent.width - 6
                                elide: Text.ElideRight
                                text: fixtureItem.modelData.address + " " + fixtureItem.modelData.name
                                color: Theme.textOn(parent.color)
                                font.pixelSize: Math.min(11, grid.cellHeight * 0.45)
                                font.weight: Font.DemiBold
                            }
                        }
                    }
                }
            }

            // where the dragged fixture would go
            Repeater {
                model: grid.dragged !== null ? grid.segments(Math.max(0, grid.dragStart), grid.dragged.channels) : []
                Rectangle {
                    required property var modelData
                    x: modelData.from * grid.cellWidth
                    y: modelData.row * grid.cellHeight
                    width: (modelData.to - modelData.from + 1) * grid.cellWidth - 1
                    height: grid.cellHeight - 1
                    radius: 3
                    color: Qt.alpha(grid.dragValid ? Theme.success : Theme.danger, 0.45)
                    border.color: grid.dragValid ? Theme.success : Theme.danger
                    border.width: 2
                    Text {
                        anchors.centerIn: parent
                        visible: parent.modelData.first
                        text: grid.dragStart + 1
                        color: Theme.text
                        font.weight: Font.Bold
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                property int pressChannel: -1
                function channelAt(x, y) {
                    let col = Math.max(0, Math.min(grid.columns - 1, Math.floor(x / grid.cellWidth)))
                    let row = Math.max(0, Math.min(512 / grid.columns - 1, Math.floor(y / grid.cellHeight)))
                    return row * grid.columns + col
                }
                onPressed: (ev) => {
                    pressChannel = channelAt(ev.x, ev.y)
                    let f = grid.fixtureAt(pressChannel)
                    if (f !== null && !(ev.modifiers & (Qt.ControlModifier | Qt.ShiftModifier))) {
                        grid.grabOffset = pressChannel - (f.address - 1)
                        grid.dragged = f
                        grid.dragStart = f.address - 1
                    }
                }
                onPositionChanged: (ev) => {
                    if (grid.dragged !== null)
                        grid.dragStart = channelAt(ev.x, ev.y) - grid.grabOffset
                }
                onReleased: (ev) => {
                    let f = grid.dragged
                    if (f !== null && grid.dragStart !== f.address - 1) {
                        if (grid.dragValid)
                            Core.patch.setFixtureAddress(f.id, grid.universe, grid.dragStart + 1)
                        else
                            grid.message(qsTr("Channels %1 to %2 are not free").arg(grid.dragStart + 1).arg(grid.dragStart + f.channels), true)
                    } else {
                        let hit = grid.fixtureAt(pressChannel)
                        grid.selectRequested(hit !== null ? [hit.id] : [], (ev.modifiers & (Qt.ControlModifier | Qt.ShiftModifier)) !== 0)
                    }
                    grid.dragged = null
                    grid.dragStart = -1
                }
                onDoubleClicked: (ev) => {
                    let f = grid.fixtureAt(channelAt(ev.x, ev.y))
                    if (f !== null)
                        grid.editRequested(f)
                }
            }
        }
    }
}
