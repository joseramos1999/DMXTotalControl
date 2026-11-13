/*
  DMX Total Control
  CueTimesView.qml

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
 * The times of the attributes and the fixtures of a cue: a delay and a
 * fade of their own, over the ones of the cue (a fixture's over its
 * attribute's), the curve of its fades (and of each attribute), and a fan
 * that spreads a delay or a fade over fixtures. It shows the cue
 * $cueIndex; an edit applies to $targets, the cues selected.
 */
Item {
    id: times

    property int cuelistId: -1
    property int cueIndex: -1
    property var targets: []
    /** Ctrl+Z or Ctrl+Y in a time with nothing typed to undo: for the cuelist */
    signal historyKey(bool redo)

    property int revision: 0
    /** { fade, downFade, delay, attributes, fixtures, length } (Core.cuelists.cueTimes) */
    readonly property var info: {
        revision
        return cuelistId >= 0 && cueIndex >= 0 ? Core.cuelists.cueTimes(cuelistId, cueIndex) : ({})
    }
    readonly property var parts: info.attributes || []
    readonly property var fixtures: info.fixtures || []
    readonly property bool hasSelection: Core.programmer.selection.length > 0

    // the curves of the fades (Cuelist::curveToString), and the ones of an
    // attribute: "" is the cue's
    readonly property var curves: [
        { key: "linear", name: qsTr("Linear") },
        { key: "smooth", name: qsTr("Smooth") },
        { key: "slowstart", name: qsTr("Slow start") },
        { key: "faststart", name: qsTr("Fast start") }
    ]
    readonly property var partCurves: [{ key: "", name: qsTr("Like the cue") }].concat(curves)
    /** How far a fade with $curve is at $p (0..1) of its time, as the engine does it */
    function shaped(curve, p) {
        switch (curve) {
        case "smooth": return p * p * (3 - 2 * p)
        case "slowstart": return p * p
        case "faststart": return 1 - (1 - p) * (1 - p)
        default: return p
        }
    }

    /* The fan: its settings live here only (not saved). The fixtures in
     * the order of the selection in Program, or the ones of the cue by
     * their number (the ones without one at the end), then by ID */
    property string fanWhat: "delay"
    property int fanFrom: 0
    property int fanTo: 2000
    property string fanShape: "line"
    /** "selection" or "cue"; "" until one is chosen: the selection when there is one */
    property string fanSource: ""
    readonly property string fanSourceUsed: fanSource !== "" ? fanSource : (hasSelection ? "selection" : "cue")
    readonly property var cueOrder: {
        revision
        if (cuelistId < 0 || cueIndex < 0)
            return []
        const ids = Core.cuelists.cueFixtures(cuelistId, cueIndex)
        const number = (id) => { const n = Core.patch.fixtureNumber(id); return n > 0 ? n : Number.MAX_SAFE_INTEGER }
        return ids.slice().sort((a, b) => (number(a) - number(b)) || (a - b))
    }
    readonly property var fanOrder: fanSourceUsed === "selection" ? Core.programmer.selection : cueOrder

    // in the order of cueTimes(), with the names and colours of the palettes
    readonly property var attributes: [
        { name: qsTr("Intensity"), icon: Theme.iconSun, color: Theme.text },
        { name: qsTr("Colour"), icon: Theme.iconPalette, color: Theme.category("#ff6a00") },
        { name: qsTr("Position"), icon: Theme.iconMove, color: Theme.functionColor("EFX") },
        { name: qsTr("Beam"), icon: Theme.iconCircle, color: Theme.category("#3d8bfd") }
    ]

    /** The rows of the fixtures, changed in place: a time being edited
     *  (the wheel) keeps its field */
    function syncFixtures() {
        for (let i = 0; i < fixtures.length; i++) {
            const f = fixtures[i]
            const row = { fixture: f.fixture, name: f.name, fade: f.fade, delay: f.delay }
            if (i < fixtureModel.count)
                fixtureModel.set(i, row)
            else
                fixtureModel.append(row)
        }
        if (fixtureModel.count > fixtures.length)
            fixtureModel.remove(fixtures.length, fixtureModel.count - fixtures.length)
    }
    onFixturesChanged: syncFixtures()
    Component.onCompleted: syncFixtures()
    ListModel { id: fixtureModel }

    Connections {
        target: Core.cuelists
        function onEdited(id) { if (id === times.cuelistId) times.revision++ }
    }

    // the widths of the time columns, and of the last one: the curve of an
    // attribute, the button that ends a fixture row
    readonly property int wTime: 62
    readonly property int wEnd: 58

    component Section: Text {
        Layout.fillWidth: true
        color: Theme.text
        font.weight: Font.Bold
        elide: Text.ElideRight
    }
    component Hint: Text {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: Theme.textFaint
        font.pixelSize: Theme.fontSmall
    }
    component HeaderText: Text {
        Layout.preferredWidth: times.wTime
        Layout.minimumWidth: times.wTime
        Layout.maximumWidth: times.wTime
        horizontalAlignment: Text.AlignRight
        rightPadding: 6
        color: Theme.textFaint
        font.weight: Font.Bold
        font.pixelSize: Theme.fontSmall
        elide: Text.ElideRight
    }
    // a time of a part: "=" is the cue's
    component PartField: CueTimeField {
        property int value: -1
        Layout.preferredWidth: times.wTime
        Layout.minimumWidth: times.wTime
        Layout.maximumWidth: times.wTime
        allowSame: true
        same: value < 0
        ms: Math.max(0, value)
        onHistoryKey: (redo) => times.historyKey(redo)
    }
    // one of the choices of a setting, lit when chosen
    component Choice: AppButton {
        implicitHeight: 28
        textSize: Theme.fontSmall + 1
    }
    // a time of the fan, in a box
    component FanTime: CueTimeField {
        id: fanTime
        Layout.preferredWidth: times.wTime
        implicitHeight: 28
        onHistoryKey: (redo) => times.historyKey(redo)
        background: Rectangle {
            radius: Theme.radiusSmall
            color: fanTime.activeFocus ? Theme.background : Theme.surfaceRaised
            border.color: fanTime.activeFocus ? Theme.accent : Theme.border
        }
    }
    // how a fade goes with a curve: from its start (bottom left) to its target (top right)
    component CurveGlyph: Canvas {
        property string curve: "linear"
        property color color: Theme.text
        implicitWidth: 24
        implicitHeight: 14
        onCurveChanged: requestPaint()
        onColorChanged: requestPaint()
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            const pad = 1.5
            const w = width - pad * 2, h = height - pad * 2
            if (w <= 0 || h <= 0)
                return
            ctx.strokeStyle = color
            ctx.lineWidth = 1.6
            ctx.lineJoin = "round"
            ctx.lineCap = "round"
            ctx.beginPath()
            for (let i = 0; i <= 24; i++) {
                const p = i / 24
                const x = pad + p * w, y = pad + (1 - times.shaped(curve, p)) * h
                if (i === 0)
                    ctx.moveTo(x, y)
                else
                    ctx.lineTo(x, y)
            }
            ctx.stroke()
        }
    }

    // one column that scrolls when the panel is short (below the cues)
    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: content.implicitHeight
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {}

        ColumnLayout {
            id: content
            width: flick.width
            spacing: 6
            enabled: times.cueIndex >= 0

            // the curve of the fades of the cue
            Section { text: qsTr("Curve of the fades") }
            Flow {
                Layout.fillWidth: true
                Layout.rightMargin: 10
                spacing: 4
                Repeater {
                    model: times.curves
                    Choice {
                        required property var modelData
                        text: modelData.name
                        highlighted: (times.info.curve || "linear") === modelData.key
                        onClicked: Core.cuelists.setCueCurve(times.cuelistId, times.targets, modelData.key)
                    }
                }
            }

            Rectangle { Layout.fillWidth: true; Layout.topMargin: 4; Layout.bottomMargin: 2; height: 1; color: Theme.border }

            // the attributes
            RowLayout {
                Layout.fillWidth: true
                Layout.rightMargin: 10
                spacing: 8
                Section { text: qsTr("Times per attribute") }
                HeaderText { text: qsTr("DELAY") }
                HeaderText { text: qsTr("FADE") }
                HeaderText {
                    Layout.preferredWidth: times.wEnd
                    Layout.minimumWidth: times.wEnd
                    Layout.maximumWidth: times.wEnd
                    horizontalAlignment: Text.AlignHCenter
                    rightPadding: 0
                    text: qsTr("CURVE")
                }
            }
            Hint { text: qsTr("Empty or =: the times of the cue") }
            Repeater {
                model: times.attributes
                RowLayout {
                    id: attributeRow
                    required property var modelData
                    required property int index
                    readonly property var part: times.parts.length > index ? times.parts[index]
                                                                           : ({ attribute: "", fade: -1, delay: -1, curve: "" })
                    Layout.fillWidth: true
                    Layout.rightMargin: 10
                    spacing: 8
                    IconLabel {
                        Layout.preferredWidth: 18
                        icon: attributeRow.modelData.icon
                        size: 12
                        color: attributeRow.modelData.color
                    }
                    Text {
                        Layout.fillWidth: true
                        text: attributeRow.modelData.name
                        color: attributeRow.part.fade >= 0 || attributeRow.part.delay >= 0 || (attributeRow.part.curve || "") !== ""
                               ? Theme.text : Theme.textDim
                        elide: Text.ElideRight
                    }
                    PartField {
                        value: attributeRow.part.delay
                        sameBase: times.info.delay || 0
                        onCommitted: (v) => Core.cuelists.setAttributeTime(times.cuelistId, times.targets,
                                                                           attributeRow.part.attribute, -1, v)
                    }
                    PartField {
                        value: attributeRow.part.fade
                        sameBase: times.info.fade || 0
                        onCommitted: (v) => Core.cuelists.setAttributeTime(times.cuelistId, times.targets,
                                                                           attributeRow.part.attribute, v, -1)
                    }
                    // its own curve, or "=" the cue's: drawn, its name in the list
                    ComboBox {
                        id: curveBox
                        Layout.preferredWidth: times.wEnd
                        Layout.minimumWidth: times.wEnd
                        Layout.maximumWidth: times.wEnd
                        implicitHeight: 26
                        focusPolicy: Qt.NoFocus
                        model: times.partCurves
                        textRole: "name"
                        currentIndex: Math.max(0, times.partCurves.findIndex(c => c.key === (attributeRow.part.curve || "")))
                        onActivated: (i) => Core.cuelists.setAttributeTime(times.cuelistId, times.targets, attributeRow.part.attribute,
                                                                           -1, -1, i === 0 ? "cue" : times.partCurves[i].key)
                        contentItem: Item {
                            CurveGlyph {
                                anchors.centerIn: parent
                                visible: curveBox.currentIndex > 0
                                curve: curveBox.currentIndex > 0 ? times.partCurves[curveBox.currentIndex].key : "linear"
                            }
                            Text {
                                anchors.centerIn: parent
                                visible: curveBox.currentIndex === 0
                                text: "="
                                color: Theme.textDim
                                font.family: Theme.monoFamily
                                font.pixelSize: Theme.fontSmall + 1
                            }
                        }
                        // the list is wider than the cell, with its right edge on the cell's
                        popup.width: 190
                        popup.x: width - popup.width
                        delegate: ItemDelegate {
                            id: curveItem
                            required property var modelData
                            required property int index
                            width: ListView.view ? ListView.view.width : 190
                            implicitHeight: 30
                            highlighted: curveBox.highlightedIndex === index
                            contentItem: Row {
                                spacing: 10
                                Item {
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 24
                                    height: 14
                                    CurveGlyph {
                                        anchors.fill: parent
                                        visible: curveItem.index > 0
                                        curve: curveItem.modelData.key || "linear"
                                    }
                                    Text {
                                        anchors.centerIn: parent
                                        visible: curveItem.index === 0
                                        text: "="
                                        color: Theme.textDim
                                        font.family: Theme.monoFamily
                                    }
                                }
                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: curveItem.modelData.name
                                    color: Theme.text
                                    font.weight: curveBox.currentIndex === curveItem.index ? Font.Bold : Font.Normal
                                }
                            }
                        }
                        HoverHandler { id: curveHover }
                        ToolTip.text: curveBox.displayText
                        ToolTip.visible: curveHover.hovered && !curveBox.popup.visible
                        ToolTip.delay: 500
                    }
                }
            }
            Hint { text: qsTr("Dimmers going down take the DOWN time of the cue; with =, the intensity fade") }

            Rectangle { Layout.fillWidth: true; Layout.topMargin: 4; Layout.bottomMargin: 2; height: 1; color: Theme.border }

            // the fixtures
            Section { text: qsTr("Times per fixture") }
            RowLayout {
                Layout.fillWidth: true
                spacing: 6
                // (a disabled button gets no hover: its tooltip says why)
                Item {
                    implicitWidth: addButton.implicitWidth
                    implicitHeight: 28
                    AppButton {
                        id: addButton
                        anchors.fill: parent
                        text: qsTr("Selected fixtures")
                        iconGlyph: Theme.iconPlus
                        implicitHeight: 28
                        enabled: times.hasSelection
                        onClicked: Core.cuelists.addFixtureTimes(times.cuelistId, times.targets, Core.programmer.selection)
                    }
                    HoverHandler { id: addHover }
                    ToolTip.text: times.hasSelection
                                  ? qsTr("The fixtures selected in Program get times of their own, starting with the times of the cue")
                                  : qsTr("Select fixtures in Program first")
                    ToolTip.visible: addHover.hovered
                    ToolTip.delay: 500
                }
                AppButton {
                    text: qsTr("Clear")
                    flat: true
                    implicitHeight: 28
                    enabled: fixtureModel.count > 0
                    onClicked: Core.cuelists.clearFixtureTimes(times.cuelistId, times.targets, [])
                }
                Item { Layout.fillWidth: true }
            }

            // the fan: a delay or a fade for each fixture, between two times
            Rectangle {
                Layout.fillWidth: true
                Layout.rightMargin: 10
                implicitHeight: fan.implicitHeight + 16
                radius: Theme.radiusSmall
                color: "transparent"
                border.color: Theme.border

                ColumnLayout {
                    id: fan
                    anchors { left: parent.left; right: parent.right; top: parent.top; margins: 8 }
                    spacing: 6

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        Text {
                            text: qsTr("Fan")
                            color: Theme.text
                            font.weight: Font.Bold
                        }
                        Row {
                            spacing: 0
                            Repeater {
                                model: [{ key: "delay", text: qsTr("Delay") }, { key: "fade", text: qsTr("Fade") }]
                                Choice {
                                    required property var modelData
                                    text: modelData.text
                                    highlighted: times.fanWhat === modelData.key
                                    onClicked: times.fanWhat = modelData.key
                                }
                            }
                        }
                        Item { Layout.fillWidth: true }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Text { text: qsTr("from"); color: Theme.textDim }
                        FanTime {
                            ms: times.fanFrom
                            onCommitted: (v) => times.fanFrom = v
                        }
                        Text { text: qsTr("to"); color: Theme.textDim }
                        FanTime {
                            ms: times.fanTo
                            onCommitted: (v) => times.fanTo = v
                        }
                        Item { Layout.fillWidth: true }
                    }
                    Flow {
                        Layout.fillWidth: true
                        spacing: 4
                        Repeater {
                            model: [{ key: "line", text: qsTr("One after the other") },
                                    { key: "middle", text: qsTr("From the middle") },
                                    { key: "ends", text: qsTr("From the ends") }]
                            Choice {
                                required property var modelData
                                text: modelData.text
                                highlighted: times.fanShape === modelData.key
                                onClicked: times.fanShape = modelData.key
                            }
                        }
                    }
                    Flow {
                        Layout.fillWidth: true
                        spacing: 4
                        Choice {
                            text: qsTr("Selected in Program (%1)").arg(Core.programmer.selection.length)
                            highlighted: times.fanSourceUsed === "selection"
                            onClicked: times.fanSource = "selection"
                        }
                        Choice {
                            text: qsTr("Of this cue, by number (%1)").arg(times.cueOrder.length)
                            highlighted: times.fanSourceUsed === "cue"
                            onClicked: times.fanSource = "cue"
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        AppButton {
                            text: qsTr("Apply")
                            primary: true
                            implicitHeight: 28
                            enabled: times.cueIndex >= 0 && times.fanOrder.length > 0
                            onClicked: Core.cuelists.fanFixtureTimes(times.cuelistId, times.targets, times.fanOrder,
                                                                     times.fanWhat, times.fanFrom, times.fanTo, times.fanShape)
                        }
                        Hint { text: qsTr("Each fixture gets a time between the two, in this order") }
                    }
                }
            }

            // the columns of the fixture rows
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 4
                Layout.rightMargin: 10
                spacing: 8
                visible: fixtureModel.count > 0
                Item { Layout.fillWidth: true }
                HeaderText { text: qsTr("DELAY") }
                HeaderText { text: qsTr("FADE") }
                Item {
                    Layout.preferredWidth: times.wEnd
                    Layout.minimumWidth: times.wEnd
                }
            }
            // (in the scrolling column: a list of its own would take the wheel)
            Repeater {
                model: fixtureModel
                RowLayout {
                    id: fixtureRow
                    required property var model
                    Layout.fillWidth: true
                    Layout.rightMargin: 10
                    spacing: 8
                    Text {
                        Layout.fillWidth: true
                        text: fixtureRow.model.name
                        color: Theme.text
                        elide: Text.ElideRight
                    }
                    PartField {
                        value: fixtureRow.model.delay
                        sameBase: times.info.delay || 0
                        onCommitted: (v) => Core.cuelists.setFixtureTimes(times.cuelistId, times.targets,
                                                                          [fixtureRow.model.fixture], -1, v)
                    }
                    PartField {
                        value: fixtureRow.model.fade
                        sameBase: times.info.fade || 0
                        onCommitted: (v) => Core.cuelists.setFixtureTimes(times.cuelistId, times.targets,
                                                                          [fixtureRow.model.fixture], v, -1)
                    }
                    // (in the column of the curves: the columns line up)
                    Item {
                        Layout.preferredWidth: times.wEnd
                        Layout.minimumWidth: times.wEnd
                        implicitHeight: 26
                        AppButton {
                            anchors.centerIn: parent
                            width: 30
                            iconGlyph: Theme.iconClose
                            flat: true
                            implicitHeight: 26
                            onClicked: Core.cuelists.clearFixtureTimes(times.cuelistId, times.targets, [fixtureRow.model.fixture])
                        }
                    }
                }
            }
            Text {
                Layout.fillWidth: true
                Layout.topMargin: 12
                visible: fixtureModel.count === 0
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: times.cueIndex < 0 ? qsTr("Select a cue") : qsTr("No fixture has its own times")
                color: Theme.textFaint
            }
        }
    }
}
