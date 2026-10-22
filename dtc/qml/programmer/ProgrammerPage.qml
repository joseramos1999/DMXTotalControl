/*
  DMX Total Control
  ProgrammerPage.qml

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
 * The programmer: select fixtures, set them live, record scenes.
 * Programmer values override playbacks until they are cleared.
 */
Item {
    id: page

    readonly property var swatches: [
        "#ffffff", "#ffd8a8", "#ff0000", "#ff6a00", "#ffbf00", "#ffff00",
        "#80ff00", "#00ff00", "#00ffaa", "#00ffff", "#0080ff", "#0000ff",
        "#6a00ff", "#bf00ff", "#ff00ff", "#ff0080"
    ]

    // The faders are keyed by channel: only a different set of channels rebuilds them,
    // value changes update them in place (a rebuilt delegate would drop the drag)
    // values or FX: what can be recorded or released
    readonly property bool hasContent: Core.programmer.hasValues || Core.programmer.fxLayers.length > 0

    /** Open on the FX view */
    property bool startFx: false
    property bool startPalettes: false
    property bool startKeypad: false
    onStartKeypadChanged: if (startKeypad) commandBar.openKeypad()
    /** Developer option --record: the record dialog open */
    /** A narrow window: the buttons with an icon show only the icon (the tooltip names them) */
    readonly property bool narrow: width < 1200
    property bool startRecord: false
    onStartRecordChanged: if (startRecord) recordDialog.openFor(false)

    signal message(string text, bool isError)

    /** The cue the programmer holds (Load in a cuelist): { id, uid, index, number, name, tracked } */
    readonly property var editing: Core.cuelists.editing
    readonly property string editingList: {
        if (editing.id === undefined)
            return ""
        const f = Core.functions.find(x => x.id === editing.id)
        return f ? f.name : ""
    }
    function updateEditing(cueOnly) {
        const e = editing
        if (e.id === undefined)
            return
        if (Core.cuelists.updateCue(e.id, e.index, { cueOnly: cueOnly })) {
            page.message(qsTr("Cue %1 of %2 updated").arg(e.number).arg(editingList), false)
            if (Core.settings.clearAfterRecord)
                Core.programmer.clear()
        }
    }
    function openFx() { channelsPanel.view = "fx" }
    function openPalettes() { channelsPanel.view = "palettes" }

    property var channelKeys: []
    property var channelMap: ({})

    function syncChannels() {
        let map = {}, keys = []
        for (let c of Core.programmer.channels) {
            map[c.key] = c
            keys.push(c.key)
        }
        channelMap = map
        if (keys.join("|") !== channelKeys.join("|"))
            channelKeys = keys
    }

    Component.onCompleted: syncChannels()

    // typing on the page goes to the command line (console style)
    onVisibleChanged: if (visible) commandBar.focusLine()
    Keys.onPressed: (event) => {
        if ((event.modifiers & (Qt.ControlModifier | Qt.AltModifier)) !== 0)
            return
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            commandBar.run()
            event.accepted = true
        } else if (event.text.length === 1 && /[0-9@+\-<>\/,a-zA-Z ]/.test(event.text)) {
            commandBar.insertText(event.text)
            event.accepted = true
        }
    }

    Connections {
        target: Core.programmer
        function onChannelsChanged() { page.syncChannels() }
    }

    ColumnLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 14

        // header
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            // the subtitle shortens: the buttons always fit
            Column {
                Layout.fillWidth: true
                Layout.minimumWidth: 120
                Layout.preferredWidth: 200
                Text {
                    text: qsTr("Program")
                    color: Theme.text
                    font.pixelSize: Theme.fontTitle
                    font.weight: Font.Bold
                }
                Text {
                    text: Core.programmer.blind
                          ? qsTr("Blind: the lights don't show what you program")
                          : Core.programmer.hasValues
                          ? qsTr("%n channel(s) in the programmer — they override the playbacks until you clear them", "", Core.programmer.valuesCount)
                            + (Core.programmer.fxLayers.length > 0 ? " · " + qsTr("%n FX", "", Core.programmer.fxLayers.length) : "")
                          : Core.programmer.fxLayers.length > 0
                          ? qsTr("%n FX in the programmer — they play over the scenes until you clear them", "", Core.programmer.fxLayers.length)
                          : qsTr("Select fixtures, set their attributes, then record a scene")
                    color: Core.programmer.blind || page.hasContent ? Theme.warning : Theme.textDim
                    font.pixelSize: Theme.fontSmall
                    width: parent.width
                    elide: Text.ElideRight
                    // shortened: all of it on hover
                    HoverHandler { id: subtitleHover }
                    ToolTip.text: text
                    ToolTip.visible: truncated && subtitleHover.hovered
                    ToolTip.delay: 500
                }
            }
            AppButton {
                text: page.narrow ? "" : qsTr("Beam on")
                iconGlyph: Theme.iconSun
                enabled: Core.programmer.selection.length > 0
                ToolTip.text: (page.narrow ? qsTr("Beam on") + ": " : "") + qsTr("Dimmer full and shutter open")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: Core.programmer.beamOn()
            }
            AppButton {
                text: qsTr("Beam off")
                enabled: Core.programmer.selection.length > 0
                onClicked: Core.programmer.beamOff()
            }
            AppButton {
                text: page.narrow ? "" : qsTr("Centre")
                iconGlyph: Theme.iconTarget
                enabled: Core.programmer.selection.length > 0
                ToolTip.text: (page.narrow ? qsTr("Centre") + ": " : "") + qsTr("Pan and tilt to the centre")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: Core.programmer.centreBeam()
            }
            AppButton {
                text: page.narrow ? "" : qsTr("Blind")
                iconGlyph: ""
                highlighted: Core.programmer.blind
                accentColor: Theme.warning
                ToolTip.text: (page.narrow ? qsTr("Blind") + ": " : "") + qsTr("Edit without sending the programmer to the lights")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: Core.programmer.blind = !Core.programmer.blind
            }
            Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 28; color: Theme.border }
            // reset all the beams, or the selected ones
            AppButton {
                text: page.narrow ? "" : qsTr("Reset all")
                iconGlyph: Theme.iconEraser
                enabled: page.hasContent
                ToolTip.text: (page.narrow ? qsTr("Reset all") + ": " : "") + qsTr("Release every live edit: the scenes show again")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: Core.programmer.clear()
            }
            AppButton {
                text: qsTr("Reset selected")
                enabled: page.hasContent && Core.programmer.selection.length > 0
                ToolTip.text: qsTr("Release the live edits of the selected fixtures")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: Core.programmer.resetSelected()
            }
            AppButton {
                text: page.narrow ? "" : qsTr("Snapshot")
                iconGlyph: Theme.iconCopy
                ToolTip.text: qsTr("Snapshot")
                ToolTip.visible: page.narrow && hovered
                ToolTip.delay: 500
                onClicked: snapshotMenu.popup()

                AppMenu {
                    id: snapshotMenu
                    readonly property var scenes: Core.functions.filter(f => f.type === "Scene")
                    AppMenu {
                        id: intoScene
                        title: qsTr("Save the edits into a scene")
                        enabled: Core.programmer.hasValues && snapshotMenu.scenes.length > 0
                        Instantiator {
                            model: snapshotMenu.scenes
                            delegate: MenuItem {
                                required property var modelData
                                text: modelData.name
                                onTriggered: Core.programmer.mergeIntoScene(modelData.id)
                            }
                            onObjectAdded: (index, object) => intoScene.insertItem(index, object)
                            onObjectRemoved: (index, object) => intoScene.removeItem(object)
                        }
                    }
                    MenuItem {
                        text: qsTr("Everything on stage into a new scene…")
                        onTriggered: recordDialog.openFor(true)
                    }
                    MenuItem {
                        text: qsTr("Only the edits into a new scene…")
                        enabled: Core.programmer.hasValues
                        onTriggered: recordDialog.openFor(false, true)
                    }
                }
            }
            AppButton {
                text: qsTr("Record…")
                iconGlyph: Theme.iconRecord
                ToolTip.text: qsTr("Record the programmer into a new scene, a new cuelist or a cue of a cuelist")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                accentColor: Theme.danger
                primary: true
                enabled: page.hasContent
                onClicked: recordDialog.openFor(false)
            }
        }

        // the cue loaded from a cuelist: update it when the changes are done
        Rectangle {
            Layout.fillWidth: true
            visible: page.editing.id !== undefined
            implicitHeight: 40
            radius: Theme.radius
            color: Qt.alpha(Theme.functionColor("Cuelist"), 0.12)
            border.color: Qt.alpha(Theme.functionColor("Cuelist"), 0.6)

            RowLayout {
                anchors { fill: parent; leftMargin: 12; rightMargin: 6 }
                spacing: 8
                IconLabel { icon: Theme.iconPen; size: 13; color: Theme.functionColor("Cuelist") }
                Text {
                    Layout.fillWidth: true
                    text: qsTr("Editing cue %1 of “%2”").arg(page.editing.number + (page.editing.name ? " · " + page.editing.name : "")).arg(page.editingList)
                          + "  ·  " + (page.editing.tracked ? qsTr("loaded with what it tracks") : qsTr("loaded with its changes only"))
                    color: Theme.text
                    elide: Text.ElideRight
                }
                AppButton {
                    text: qsTr("Update") + " ▾"
                    iconGlyph: Theme.iconSave
                    implicitHeight: 30
                    primary: true
                    accentColor: Theme.functionColor("Cuelist")
                    enabled: page.hasContent
                    onClicked: editUpdateMenu.popup()
                    AppMenu {
                        id: editUpdateMenu
                        MenuItem {
                            text: qsTr("Update cue %1 (the next cues follow)").arg(page.editing.number || "")
                            onTriggered: page.updateEditing(false)
                        }
                        MenuItem {
                            text: qsTr("Only this cue (the next cues keep their look)")
                            onTriggered: page.updateEditing(true)
                        }
                    }
                }
                AppButton {
                    iconGlyph: Theme.iconClose
                    flat: true
                    implicitHeight: 30
                    ToolTip.text: qsTr("Stop editing the cue (the programmer keeps its values)")
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                    onClicked: Core.cuelists.stopEditing()
                }
            }
        }

        // the command line of a console
        CommandBar {
            id: commandBar
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            // fixtures
            Panel {
                id: fixturesPanel
                property bool planView: false

                Layout.preferredWidth: planView ? 520 : 380
                Layout.fillHeight: true
                title: qsTr("Fixtures (%1 selected)").arg(Core.programmer.selection.length)
                headerItems: [
                    AppButton { iconGlyph: Theme.iconGrid; flat: true; highlighted: fixturesPanel.planView; implicitHeight: 28
                                ToolTip.text: qsTr("2D plan"); ToolTip.visible: hovered
                                onClicked: { fixturesPanel.planView = !fixturesPanel.planView; Core.patch.autoLayout(false) } },
                    AppButton { text: qsTr("All"); flat: true; implicitHeight: 28; onClicked: Core.programmer.selectAll() },
                    AppButton { text: qsTr("None"); flat: true; implicitHeight: 28; onClicked: Core.programmer.clearSelection() }
                ]

                ColumnLayout {
                    anchors { fill: parent; margins: 8 }
                    spacing: 8

                    // selection tools, following the 2D plan left to right
                    Flow {
                        Layout.fillWidth: true
                        spacing: 4
                        AppButton { text: qsTr("Invert"); implicitHeight: 26; flat: true; onClicked: Core.programmer.invertSelection() }
                        AppButton { text: "½"; implicitHeight: 26; flat: true; ToolTip.text: qsTr("One of every two"); ToolTip.visible: hovered; onClicked: Core.programmer.selectEvery(2) }
                        AppButton { text: "⅓"; implicitHeight: 26; flat: true; ToolTip.text: qsTr("One of every three"); ToolTip.visible: hovered; onClicked: Core.programmer.selectEvery(3) }
                        AppButton { text: "¼"; implicitHeight: 26; flat: true; ToolTip.text: qsTr("One of every four"); ToolTip.visible: hovered; onClicked: Core.programmer.selectEvery(4) }
                        AppButton { text: "◀"; implicitHeight: 26; flat: true; ToolTip.text: qsTr("Shift the selection left"); ToolTip.visible: hovered; onClicked: Core.programmer.shiftSelection(-1) }
                        AppButton { text: "▶"; implicitHeight: 26; flat: true; ToolTip.text: qsTr("Shift the selection right"); ToolTip.visible: hovered; onClicked: Core.programmer.shiftSelection(1) }
                    }

                    // fixture groups: click selects, ctrl+click adds. Many
                    // groups keep to three rows that scroll ("All the groups"
                    // opens them, up to half the panel): the fixtures stay in sight
                    Item {
                        id: groupsBox
                        readonly property real collapsedHeight: 3 * 28 + 2 * groupsFlow.spacing
                        readonly property bool overflows: groupsFlow.implicitHeight > collapsedHeight + 1
                        property bool expanded: false
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.min(groupsFlow.implicitHeight,
                                                         expanded ? Math.max(collapsedHeight, fixturesPanel.height / 2) : collapsedHeight)
                        visible: Core.patch.groups.length > 0

                        Flickable {
                            id: groupsFlick
                            anchors.fill: parent
                            clip: true
                            contentHeight: groupsFlow.implicitHeight
                            interactive: contentHeight > height + 1
                            boundsBehavior: Flickable.StopAtBounds
                            ScrollBar.vertical: ScrollBar { policy: groupsFlick.interactive ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff }

                            Flow {
                                id: groupsFlow
                                // (room for the scroll bar)
                                width: groupsFlick.width - 8
                                spacing: 6
                                Repeater {
                                    model: Core.patch.groups
                                    delegate: Rectangle {
                                        required property var modelData
                                        required property int index
                                        readonly property bool allSelected: modelData.fixtures.length > 0 &&
                                            modelData.fixtures.every(id => Core.programmer.selection.indexOf(id) >= 0)
                                        width: chipText.implicitWidth + 20
                                        height: 28
                                        radius: 14
                                        color: allSelected ? Theme.accentFill : (chipMouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised)
                                        border.color: allSelected ? Theme.accentFill : Theme.border
                                        Text {
                                            id: chipText
                                            anchors.centerIn: parent
                                            text: modelData.name
                                            color: parent.allSelected ? Theme.accentText : Theme.text
                                            font.pixelSize: Theme.fontSmall
                                            font.weight: Font.DemiBold
                                        }
                                        MouseArea {
                                            id: chipMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            ToolTip.visible: containsMouse
                                            ToolTip.delay: 700
                                            ToolTip.text: qsTr("GROUP %1 in the command line").arg(parent.index + 1)
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: (ev) => Core.programmer.selectFixtures(modelData.fixtures,
                                                           (ev.modifiers & (Qt.ControlModifier | Qt.ShiftModifier)) !== 0)
                                        }
                                    }
                                }
                                // the selections of the groups: their own order
                                Repeater {
                                    model: Core.patch.selections
                                    delegate: Rectangle {
                                        required property var modelData
                                        readonly property bool active: JSON.stringify(Core.programmer.selection) === JSON.stringify(modelData.fixtures)
                                        width: selectionText.implicitWidth + 34
                                        height: 28
                                        radius: 14
                                        color: active ? Theme.accentFill : (selectionMouse.containsMouse ? Theme.surfaceHover : "transparent")
                                        border.color: active ? Theme.accentFill : Theme.border
                                        IconLabel {
                                            anchors { left: parent.left; leftMargin: 10; verticalCenter: parent.verticalCenter }
                                            icon: Theme.iconSelect
                                            size: 10
                                            color: parent.active ? Theme.accentText : Theme.textDim
                                        }
                                        Text {
                                            id: selectionText
                                            anchors { left: parent.left; leftMargin: 24; verticalCenter: parent.verticalCenter }
                                            text: modelData.name
                                            color: parent.active ? Theme.accentText : Theme.text
                                            font.pixelSize: Theme.fontSmall
                                        }
                                        MouseArea {
                                            id: selectionMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: (ev) => Core.programmer.selectFixtures(modelData.fixtures,
                                                           (ev.modifiers & (Qt.ControlModifier | Qt.ShiftModifier)) !== 0)
                                        }
                                    }
                                }
                            }
                        }
                    }
                    AppButton {
                        Layout.alignment: Qt.AlignRight
                        Layout.topMargin: -4
                        visible: groupsBox.visible && groupsBox.overflows
                        flat: true
                        implicitHeight: 22
                        text: groupsBox.expanded ? qsTr("Fewer") : qsTr("All the groups (%1)").arg(Core.patch.groups.length)
                        iconGlyph: groupsBox.expanded ? Theme.iconChevronUp : Theme.iconChevronDown
                        onClicked: {
                            groupsBox.expanded = !groupsBox.expanded
                            groupsFlick.contentY = 0
                        }
                    }

                    StagePlan {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: fixturesPanel.planView
                        live: true
                        fixtures: Core.patch.fixtures
                        selection: Core.programmer.selection
                        onSelectRequested: (ids, additive) => {
                            if (ids.length === 0 && !additive)
                                Core.programmer.clearSelection()
                            else if (additive && ids.length === 1)
                                Core.programmer.toggle(ids[0])
                            else
                                Core.programmer.selectFixtures(ids, additive)
                        }
                    }

                    GridView {
                        id: fixtureGrid
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: !fixturesPanel.planView
                        clip: true
                        cellWidth: width / 3
                        cellHeight: 70
                        model: Core.patch.fixtures
                        boundsBehavior: Flickable.StopAtBounds
                        ScrollBar.vertical: ScrollBar {}

                        /** The fixture clicked last: Shift+click selects from it */
                        property int anchorId: -1
                        /** Alone: only it; Ctrl: toggles it; Shift: every fixture
                         *  from the last one clicked (in that direction: the
                         *  order of the selection); Ctrl+Shift: adds them */
                        function clickFixture(id, modifiers) {
                            const order = Core.patch.fixtures.map(f => f.id)
                            const from = order.indexOf(anchorId)
                            const to = order.indexOf(id)
                            if ((modifiers & Qt.ShiftModifier) && from >= 0 && to >= 0) {
                                let range = order.slice(Math.min(from, to), Math.max(from, to) + 1)
                                if (to < from)
                                    range.reverse()
                                Core.programmer.selectFixtures(range, (modifiers & Qt.ControlModifier) !== 0)
                                return
                            }
                            if (modifiers & Qt.ControlModifier)
                                Core.programmer.toggle(id)
                            else
                                Core.programmer.select(id, false)
                            anchorId = id
                        }

                        delegate: Item {
                            required property var modelData
                            width: fixtureGrid.cellWidth
                            height: fixtureGrid.cellHeight

                            FixtureTile {
                                anchors { fill: parent; margins: 3 }
                                name: modelData.name
                                type: modelData.type
                                number: modelData.number || 0
                                address: (modelData.universe + 1) + "." + modelData.address
                                selected: Core.programmer.selection.indexOf(modelData.id) >= 0
                                onClicked: (modifiers) => fixtureGrid.clickFixture(modelData.id, modifiers)
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            width: parent.width - 40
                            visible: fixtureGrid.count === 0
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            text: qsTr("No fixtures patched yet.\nAdd some on the Patch page.")
                            color: Theme.textDim
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 12

                // quick controls
                Panel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 252
                    // the FX need the room
                    visible: channelsPanel.view !== "fx"
                    title: qsTr("Quick")
                    enabled: Core.programmer.selection.length > 0
                    opacity: enabled ? 1 : 0.5

                    RowLayout {
                        anchors { fill: parent; margins: 12 }
                        spacing: 18

                        // dimmer
                        ColumnLayout {
                            Layout.fillHeight: true
                            spacing: 6
                            Text { text: qsTr("Dimmer"); color: Theme.textDim; font.pixelSize: Theme.fontSmall; font.weight: Font.Bold }
                            RowLayout {
                                Layout.fillHeight: true
                                spacing: 8
                                VFader {
                                    id: dimmerFader
                                    Layout.fillHeight: true
                                    fillColor: Theme.text
                                    onUserMoved: (v) => Core.programmer.setDimmer(v)
                                    // the dimmer the selection has in the programmer (also
                                    // from the command line or a palette), but not while dragged
                                    function follow() {
                                        const ch = Core.programmer.channels.find(c => c.key === "dimmer")
                                        if (ch !== undefined && !pressed)
                                            value = ch.value
                                    }
                                    Component.onCompleted: follow()
                                    Connections {
                                        target: Core.programmer
                                        function onChannelsChanged() { dimmerFader.follow() }
                                        function onValuesChanged() { dimmerFader.follow() }
                                    }
                                }
                                Column {
                                    Layout.alignment: Qt.AlignTop
                                    spacing: 4
                                    Repeater {
                                        model: [100, 75, 50, 25, 0]
                                        AppButton {
                                            required property int modelData
                                            width: 58
                                            implicitHeight: 26
                                            text: modelData + "%"
                                            onClicked: {
                                                dimmerFader.value = Math.round(modelData * 2.55)
                                                Core.programmer.setDimmer(Math.round(modelData * 2.55))
                                            }
                                        }
                                    }
                                }
                            }
                            Text {
                                text: Math.round(dimmerFader.value * 100 / 255) + "%"
                                color: Theme.text
                                font.family: Theme.monoFamily
                                font.pixelSize: Theme.fontSmall
                            }
                        }

                        Rectangle { Layout.fillHeight: true; Layout.preferredWidth: 1; color: Theme.border }

                        // colour
                        ColumnLayout {
                            id: colourColumn
                            Layout.fillWidth: true
                            // its width doesn't follow the swatches: they follow it (narrower in a small window)
                            Layout.preferredWidth: 8 * 42 - 6
                            Layout.minimumWidth: 8 * 26 - 6
                            Layout.fillHeight: true
                            spacing: 8
                            Text { text: qsTr("Colour"); color: Theme.textDim; font.pixelSize: Theme.fontSmall; font.weight: Font.Bold }

                            Grid {
                                columns: 8
                                spacing: 6
                                Repeater {
                                    model: page.swatches
                                    Rectangle {
                                        required property string modelData
                                        width: Math.max(20, Math.min(36, Math.floor((colourColumn.width - 7 * 6) / 8))); height: 30; radius: 6
                                        color: modelData
                                        opacity: Theme.lightLevel
                                        border.color: swatchMouse.containsMouse ? Theme.text : Theme.edgeFor(modelData)
                                        border.width: swatchMouse.containsMouse ? 2 : 1
                                        MouseArea {
                                            id: swatchMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: Core.programmer.setColor(modelData)
                                        }
                                    }
                                }
                            }

                            Item { Layout.fillHeight: true }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Text { id: hueLabel; Layout.preferredWidth: Math.max(34, hueLabel.implicitWidth, satLabel.implicitWidth); text: qsTr("Hue"); color: Theme.textFaint; font.pixelSize: Theme.fontSmall }
                                HSlider {
                                    id: hueSlider
                                    Layout.fillWidth: true
                                    from: 0; to: 1
                                    opacity: Theme.lightLevel
                                    trackGradient: Gradient {
                                        orientation: Gradient.Horizontal
                                        GradientStop { position: 0.0; color: "#ff0000" }
                                        GradientStop { position: 0.17; color: "#ffff00" }
                                        GradientStop { position: 0.33; color: "#00ff00" }
                                        GradientStop { position: 0.5; color: "#00ffff" }
                                        GradientStop { position: 0.67; color: "#0000ff" }
                                        GradientStop { position: 0.83; color: "#ff00ff" }
                                        GradientStop { position: 1.0; color: "#ff0000" }
                                    }
                                    onMoved: Core.programmer.setColor(Qt.hsva(value, satSlider.value, 1, 1))
                                }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                Text { id: satLabel; Layout.preferredWidth: Math.max(34, hueLabel.implicitWidth, satLabel.implicitWidth); text: qsTr("Sat"); color: Theme.textFaint; font.pixelSize: Theme.fontSmall }
                                HSlider {
                                    id: satSlider
                                    Layout.fillWidth: true
                                    from: 0; to: 1; value: 1
                                    fillColor: Theme.category(Qt.hsva(hueSlider.value, 1, 1, 1))
                                    onMoved: Core.programmer.setColor(Qt.hsva(hueSlider.value, value, 1, 1))
                                }
                            }
                        }
                    }
                }

                // features by tabs, or every channel as a fader
                Panel {
                    id: channelsPanel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    /** "features", "faders", "palettes" or "fx" (FX on the parameters) */
                    property string view: page.startFx ? "fx" : page.startPalettes ? "palettes" : "features"
                    readonly property bool faders: view === "faders"
                    title: view === "fx" ? qsTr("FX") : view === "palettes" ? qsTr("Palettes") : faders ? qsTr("Faders") : qsTr("Features")
                    headerItems: [
                        AppButton {
                            text: qsTr("Features")
                            implicitHeight: 26
                            highlighted: channelsPanel.view === "features"
                            onClicked: channelsPanel.view = "features"
                        },
                        AppButton {
                            text: qsTr("Faders")
                            implicitHeight: 26
                            highlighted: channelsPanel.view === "faders"
                            onClicked: channelsPanel.view = "faders"
                        },
                        AppButton {
                            text: qsTr("Palettes")
                            iconGlyph: Theme.iconPalette
                            implicitHeight: 26
                            highlighted: channelsPanel.view === "palettes"
                            ToolTip.text: qsTr("Position, colour, beam and intensity palettes: the scenes follow them when they change"); ToolTip.visible: hovered
                            onClicked: channelsPanel.view = "palettes"
                        },
                        AppButton {
                            text: qsTr("FX")
                            iconGlyph: Theme.iconWand
                            implicitHeight: 26
                            highlighted: channelsPanel.view === "fx"
                            ToolTip.text: qsTr("Effects on the parameters of the selection"); ToolTip.visible: hovered
                            onClicked: channelsPanel.view = "fx"
                        }
                    ]

                    FeaturesView {
                        anchors { fill: parent; margins: 8 }
                        visible: channelsPanel.view === "features"
                        channelKeys: page.channelKeys
                        channelMap: page.channelMap
                    }

                    FxPanel {
                        anchors { fill: parent; margins: 8 }
                        visible: channelsPanel.view === "fx"
                    }

                    PalettesView {
                        anchors { fill: parent; margins: 8 }
                        visible: channelsPanel.view === "palettes"
                    }

                    ListView {
                        id: channelList
                        visible: channelsPanel.faders
                        anchors { fill: parent; margins: 8 }
                        orientation: ListView.Horizontal
                        spacing: 6
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        model: page.channelKeys
                        ScrollBar.horizontal: ScrollBar {}

                        delegate: ChannelFader {
                            required property string modelData
                            height: channelList.height - 14
                            channel: page.channelMap[modelData] || ({})
                        }

                        Text {
                            anchors.centerIn: parent
                            visible: channelList.count === 0
                            text: qsTr("Select one or more fixtures to see their channels")
                            color: Theme.textDim
                        }
                    }
                }
            }
        }
    }

    RecordDialog {
        id: recordDialog
        onRecorded: (text) => page.message(text, false)
    }
}
