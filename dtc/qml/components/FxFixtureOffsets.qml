/*
  DMX Total Control
  FxFixtureOffsets.qml

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
 * The fixtures of one FX, in its order, each with its shift and its delay
 * typed by hand: how far behind the others it goes in the wave, and how much
 * later it does the FX. Folded at first.
 */
ColumnLayout {
    id: offsets

    /** [{ id, name, selected, place (degrees, -1: none), step (1..., 0: none), shift, delay (ms),
     *     virtual (no dimmer: a dimmer FX dims its colour) }] */
    property var rows: []
    /** The programmer: the fan and the clearing go to the selected fixtures only */
    property bool selectionBased: false
    property bool open: false
    // the fields, and below this width each row in two lines
    readonly property int shiftWidth: 120
    readonly property int delayWidth: 130
    readonly property bool narrow: width < 480

    signal setOffset(int fixtureId, string key, int value)
    /** Spread $key from 0 on the first fixture to $last on the last one */
    signal fan(string key, int last)
    /** No shift or delay on the fixtures the buttons act on */
    signal clearAll()

    function hasOffset(row) { return (row.shift || 0) > 0 || (row.delay || 0) > 0 }
    // the buttons act on the selected fixtures in the programmer, on all of them in a scene
    function acted(row) { return !selectionBased || !!row.selected }
    readonly property int withOffset: rows.filter(r => hasOffset(r)).length
    readonly property var actedRows: rows.filter(r => acted(r))

    function placeText(row) {
        return row.step > 0 ? qsTr("Step %1").arg(row.step) : row.place >= 0 ? row.place + "°" : "—"
    }
    // the value of the last fixture the fan goes to, to start from
    function lastValue(key) {
        let last = actedRows.length > 0 ? actedRows[actedRows.length - 1] : ({})
        return key === "shift" ? String(last.shift || 0) : String((last.delay || 0) / 1000)
    }

    spacing: 4

    Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

    // ------------------------------------------------------------ header: click to fold
    Item {
        Layout.fillWidth: true
        implicitHeight: 24
        RowLayout {
            anchors.fill: parent
            spacing: 6
            IconLabel { icon: offsets.open ? Theme.iconChevronDown : Theme.iconChevronRight; size: 9; color: Theme.textDim }
            Text {
                text: qsTr("Per fixture")
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                visible: offsets.withOffset > 0
                text: qsTr("%n with shift or delay", "", offsets.withOffset)
                color: Theme.accent
                font.pixelSize: Theme.fontSmall
                elide: Text.ElideRight
            }
            Item { Layout.fillWidth: true; visible: offsets.withOffset === 0 }
        }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: offsets.open = !offsets.open
        }
    }

    Flow {
        Layout.fillWidth: true
        visible: offsets.open
        spacing: 6
        AppButton {
            text: qsTr("Fan the shift…")
            implicitHeight: 28
            enabled: offsets.actedRows.length > 0
            ToolTip.text: qsTr("From 0 on the first to this on the last, in the order of the FX"); ToolTip.visible: hovered
            onClicked: prompt.ask(qsTr("Shift of the last one (°)"), offsets.lastValue("shift"),
                                  text => offsets.fan("shift", Math.round(Theme.parseNumber(text, 0))))
        }
        AppButton {
            text: qsTr("Fan the delay…")
            implicitHeight: 28
            enabled: offsets.actedRows.length > 0
            ToolTip.text: qsTr("From 0 on the first to this on the last, in the order of the FX"); ToolTip.visible: hovered
            onClicked: prompt.ask(qsTr("Delay of the last one (s)"), offsets.lastValue("delay"),
                                  text => offsets.fan("delay", Math.round(Theme.parseNumber(text, 0) * 1000)))
        }
        AppButton {
            text: qsTr("No shift or delay")
            iconGlyph: Theme.iconRefresh
            implicitHeight: 28
            enabled: offsets.actedRows.some(r => offsets.hasOffset(r))
            onClicked: offsets.clearAll()
        }
    }

    // ------------------------------------------------------------ the fixtures
    // each row: the number, the name and the place, then the two fields (on a
    // second line when the list is narrow, so the names still show)
    GridLayout {
        Layout.fillWidth: true
        Layout.leftMargin: 6
        Layout.rightMargin: 4
        visible: offsets.open && offsets.rows.length > 0
        columns: offsets.narrow ? 1 : 2
        columnSpacing: 8
        rowSpacing: 0
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Item { Layout.fillWidth: true }
            Text { Layout.preferredWidth: 60; text: qsTr("In the wave"); color: Theme.textFaint; font.pixelSize: 10; elide: Text.ElideRight }
        }
        RowLayout {
            Layout.fillWidth: offsets.narrow
            spacing: 8
            Text {
                Layout.fillWidth: offsets.narrow
                Layout.preferredWidth: offsets.narrow ? 1 : offsets.shiftWidth
                text: qsTr("Shift"); color: Theme.textFaint; font.pixelSize: 10; elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: offsets.narrow
                Layout.preferredWidth: offsets.narrow ? 1 : offsets.delayWidth
                text: qsTr("Delay"); color: Theme.textFaint; font.pixelSize: 10; elide: Text.ElideRight
            }
        }
    }
    // (a count as the model: a change of the values keeps the rows, and the field being typed in)
    Repeater {
        model: offsets.open ? offsets.rows.length : 0
        delegate: Rectangle {
            id: row
            required property int index
            readonly property var entry: offsets.rows[index] || ({})
            readonly property int number: Core.patch.fixtureNumber(entry.id || 0)
            Layout.fillWidth: true
            implicitHeight: rowGrid.implicitHeight + 4
            radius: Theme.radiusSmall
            color: row.entry.selected ? Qt.alpha(Theme.accent, 0.15) : "transparent"

            GridLayout {
                id: rowGrid
                anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; leftMargin: 6; rightMargin: 4 }
                columns: offsets.narrow ? 1 : 2
                columnSpacing: 8
                rowSpacing: 2
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Text {
                        Layout.preferredWidth: 22
                        visible: row.number > 0
                        text: row.number
                        color: Theme.textDim
                        font.family: Theme.monoFamily
                        font.pixelSize: Theme.fontSmall
                        horizontalAlignment: Text.AlignRight
                    }
                    Text {
                        Layout.fillWidth: true
                        text: row.entry.name || ""
                        color: Theme.text
                        font.pixelSize: Theme.fontSmall
                        elide: Text.ElideRight
                    }
                    // without a dimmer: the dimmer FX dims its colour
                    Text {
                        visible: !!row.entry.virtual
                        text: qsTr("virtual")
                        color: Theme.textDim
                        font.pixelSize: 9
                        font.italic: true
                    }
                    Text {
                        Layout.preferredWidth: 60
                        text: offsets.placeText(row.entry)
                        color: Theme.textDim
                        font.family: Theme.monoFamily
                        font.pixelSize: Theme.fontSmall
                    }
                }
                RowLayout {
                    Layout.fillWidth: offsets.narrow
                    spacing: 8
                    SpinBox {
                        Layout.fillWidth: offsets.narrow
                        Layout.preferredWidth: offsets.narrow ? 1 : offsets.shiftWidth
                        implicitHeight: 28
                        from: 0; to: 1440; stepSize: 15
                        editable: true
                        value: row.entry.shift || 0
                        textFromValue: (v) => v + "°"
                        valueFromText: (text) => Math.round(Theme.parseNumber(text, value))
                        validator: RegularExpressionValidator { regularExpression: /[0-9]{1,4}\s*°?/ }
                        onValueModified: offsets.setOffset(row.entry.id, "shift", value)
                        ToolTip.text: qsTr("How far behind the others it goes in the wave: 360° is a whole cycle (in a step, one place)")
                        ToolTip.visible: hovered
                    }
                    TimeSpin {
                        Layout.fillWidth: offsets.narrow
                        Layout.preferredWidth: offsets.narrow ? 1 : offsets.delayWidth
                        implicitHeight: 28
                        decimals: 2
                        to: 6000
                        engineValue: row.entry.delay || 0
                        onEdited: (v) => offsets.setOffset(row.entry.id, "delay", v)
                        ToolTip.text: qsTr("How much later it does the FX. When the FX starts, it waits that long at its base")
                        ToolTip.visible: hovered
                    }
                }
            }
        }
    }

    TextPrompt { id: prompt }
}
