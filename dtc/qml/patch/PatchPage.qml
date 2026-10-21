/*
  DMX Total Control
  PatchPage.qml

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
import QtQuick.Dialogs
import DMXTotalControl
import DMXTotalControl.Backend

/**
 * Patch: the fixtures of the rig, as a list or placed on a 2D stage plan,
 * plus the fixture groups.
 */
Item {
    id: page

    property var selectedIds: []
    property bool groupByType: true
    // "list", "grid" (channels of a universe) or "plan"
    property string view: "list"
    readonly property bool planView: view === "plan"
    /** A narrow page (a small screen): the list without the fixture column
        (the type is in the headers of the groups) */
    readonly property bool compact: width < 1200
    /** The toolbar with its labels needs this much (measured while it shows
        them): when it doesn't fit (a small screen, a longer language), icons alone */
    property real toolbarFullWidth: 0
    readonly property bool toolbarCompact: compact || toolbarFullWidth > toolbarRow.width

    readonly property var fixtures: Core.patch.fixtures

    function isSelected(id) { return selectedIds.indexOf(id) >= 0 }

    function select(ids, additive) {
        let s = additive ? selectedIds.slice() : []
        for (let id of ids) {
            let i = s.indexOf(id)
            if (additive && i >= 0 && ids.length === 1)
                s.splice(i, 1)          // ctrl+click toggles
            else if (i < 0)
                s.push(id)
        }
        selectedIds = s
    }

    /** The fixture clicked last in the list: Shift+click selects from it */
    property int anchorId: -1
    /** A click on a fixture of the list: alone, Ctrl toggles it, Shift selects
     *  every fixture shown from the last one clicked (in that direction: the
     *  order of the selection), Ctrl+Shift adds them */
    function clickFixture(id, modifiers) {
        const order = rows.filter(r => !r.header).map(r => r.fixture.id)
        const from = order.indexOf(anchorId)
        const to = order.indexOf(id)
        if ((modifiers & Qt.ShiftModifier) && from >= 0 && to >= 0) {
            let range = order.slice(Math.min(from, to), Math.max(from, to) + 1)
            if (to < from)
                range.reverse()
            select(range, (modifiers & Qt.ControlModifier) !== 0)
            return
        }
        select([id], (modifiers & Qt.ControlModifier) !== 0)
        anchorId = id
    }

    /** Folded in the list: the types (their header) whose fixtures don't show */
    property var foldedTypes: []
    /** Folded in the fixture groups: the groups whose selections don't show */
    property var foldedGroups: []
    function toggleFolded(list, key) {
        return list.indexOf(key) >= 0 ? list.filter(k => k !== key) : list.concat([key])
    }

    signal message(string text, bool isError)

    // the shape tools of the plan, on the selection in its order
    function arrange(shape, options) {
        options.spacing = spacingSpin.value * 10
        options.columns = columnsSpin.value
        Core.patch.arrange(selectedIds, shape, options)
    }
    /** Developer option --patch-view: list, grid, plan, live, limits, matrix */
    function devOpen(what) {
        if (what === "limits" || what === "matrix") {
            selectedIds = fixtures.filter(f => f.hasPan).slice(0, 1).map(f => f.id)
            if (what === "limits") optionsDialog.openFor(selectedIds)
            else matrixDialog.open()
            return
        }
        view = what === "live" ? "plan" : what
        stagePlan.live = what === "live"
        if (what === "plan" || what === "live")
            selectedIds = fixtures.filter(f => f.name.startsWith("Spot")).map(f => f.id)
    }
    function saveSelection(groupId) {
        let ids = selectedIds.slice()
        prompt.ask(qsTr("New selection"), "", (name) => Core.patch.addSelection(name, groupId, ids))
    }

    function selectedFixtures() {
        return fixtures.filter(f => isSelected(f.id))
    }

    // inverted axis of the selection: true when every fixture with that axis has it inverted
    function selectionHas(axis) { return selectedFixtures().some(f => f[axis === "pan" ? "hasPan" : "hasTilt"]) }
    function selectionInverted(axis) {
        let sel = selectedFixtures().filter(f => f[axis === "pan" ? "hasPan" : "hasTilt"])
        return sel.length > 0 && sel.every(f => f[axis === "pan" ? "invertPan" : "invertTilt"])
    }
    function invertSelection(axis) {
        let sel = selectedFixtures().filter(f => f[axis === "pan" ? "hasPan" : "hasTilt"])
        Core.patch.setInverted(sel.map(f => f.id), axis, !selectionInverted(axis))
    }

    readonly property bool identifying: Core.patch.identified.length > 0

    // identifying is a patch tool: it stops when leaving the page
    onVisibleChanged: if (!visible) Core.patch.stopIdentify()

    // rows of the list: fixtures sorted by address, optionally under a header per fixture type
    readonly property var rows: {
        let sorted = fixtures.slice().sort((a, b) => (a.universe - b.universe) || (a.address - b.address))
        if (!groupByType)
            return sorted.map(f => ({ header: false, fixture: f }))

        let groups = {}
        let order = []
        for (let f of sorted) {
            let key = f.manufacturer.length ? f.manufacturer + " — " + f.model : f.type
            if (!groups[key]) { groups[key] = []; order.push(key) }
            groups[key].push(f)
        }
        let out = []
        for (let key of order) {
            const folded = foldedTypes.indexOf(key) >= 0
            out.push({ header: true, title: key, ids: groups[key].map(f => f.id), folded: folded })
            if (!folded) {
                for (let f of groups[key])
                    out.push({ header: false, fixture: f })
            }
        }
        return out
    }

    // forget deleted fixtures
    onFixturesChanged: selectedIds = selectedIds.filter(id => fixtures.some(f => f.id === id))

    ColumnLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 12

        // header
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            // the subtitle shortens: the buttons always fit
            Column {
                Layout.fillWidth: true
                Layout.minimumWidth: 90
                Layout.preferredWidth: 160
                Text {
                    width: parent.width
                    elide: Text.ElideRight
                    text: qsTr("Patch")
                    color: Theme.text
                    font.pixelSize: Theme.fontTitle
                    font.weight: Font.Bold
                }
                Text {
                    width: parent.width
                    elide: Text.ElideRight
                    text: qsTr("%n fixture(s) patched", "", Core.patch.fixturesCount)
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                }
            }

            // list / grid / plan
            Row {
                spacing: 0
                AppButton {
                    text: qsTr("List")
                    iconGlyph: Theme.iconList
                    highlighted: page.view === "list"
                    onClicked: page.view = "list"
                }
                AppButton {
                    text: qsTr("Channels")
                    iconGlyph: Theme.iconTable
                    highlighted: page.view === "grid"
                    ToolTip.text: qsTr("The 512 channels of each universe: drag a fixture to change its address")
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                    onClicked: page.view = "grid"
                }
                AppButton {
                    text: qsTr("2D plan")
                    iconGlyph: Theme.iconGrid
                    highlighted: page.view === "plan"
                    onClicked: { page.view = "plan"; Core.patch.autoLayout(false) }
                }
            }

            AppButton {
                text: qsTr("Import")
                iconGlyph: Theme.iconImport
                onClicked: importMenu.popup()
                AppMenu {
                    id: importMenu
                    MenuItem { text: qsTr("Patch of another show…"); onTriggered: { importDialog.patch = true; importDialog.scenes = false; importDialog.open() } }
                    MenuItem { text: qsTr("Scenes of another show…"); onTriggered: { importDialog.patch = false; importDialog.scenes = true; importDialog.open() } }
                    MenuItem { text: qsTr("Patch and scenes of another show…"); onTriggered: { importDialog.patch = true; importDialog.scenes = true; importDialog.open() } }
                }
            }
            AppButton {
                text: qsTr("LED matrix")
                iconGlyph: Theme.iconMatrix
                ToolTip.text: qsTr("Patch an LED matrix or strip in one go")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: matrixDialog.open()
            }
            AppButton {
                text: qsTr("Add fixtures")
                iconGlyph: Theme.iconPlus
                primary: true
                onClicked: addDialog.open()
            }
        }

        // selection toolbar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 46
            radius: Theme.radius
            color: Theme.surface
            border.color: Theme.border

            RowLayout {
                id: toolbarRow
                anchors { fill: parent; leftMargin: 10; rightMargin: 10 }
                spacing: 6
                onImplicitWidthChanged: if (!page.toolbarCompact) page.toolbarFullWidth = implicitWidth

                // short labels, as in Program: with "Stop identify" the bar
                // didn't fit at 1440 px in Spanish
                AppButton { text: qsTr("All"); flat: true; onClicked: page.selectedIds = page.fixtures.map(f => f.id) }
                AppButton { text: qsTr("None"); flat: true; enabled: page.selectedIds.length > 0; onClicked: page.selectedIds = [] }
                CheckBox {
                    text: qsTr("Group by fixture")
                    checked: page.groupByType
                    visible: page.view === "list"
                    onToggled: page.groupByType = checked
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: qsTr("%n selected", "", page.selectedIds.length)
                    color: page.selectedIds.length ? Theme.text : Theme.textFaint
                    font.pixelSize: Theme.fontSmall
                    rightPadding: 6
                }
                AppButton {
                    text: page.toolbarCompact ? "" : page.identifying ? qsTr("Stop identifying") : qsTr("Identify")
                    iconGlyph: Theme.iconLocate
                    highlighted: page.identifying
                    accentColor: Theme.warning
                    enabled: page.identifying || page.selectedIds.length > 0
                    ToolTip.text: qsTr("Light up the selected fixtures in white, blinking, to find them on stage")
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                    onClicked: Core.patch.identify(page.identifying ? [] : page.selectedIds)
                }
                AppButton {
                    iconGlyph: Theme.iconArrowsH
                    highlighted: page.selectionInverted("pan")
                    enabled: page.selectionHas("pan")
                    ToolTip.text: qsTr("Invert pan")
                    ToolTip.visible: hovered
                    onClicked: page.invertSelection("pan")
                }
                AppButton {
                    iconGlyph: Theme.iconArrowsV
                    highlighted: page.selectionInverted("tilt")
                    enabled: page.selectionHas("tilt")
                    ToolTip.text: qsTr("Invert tilt")
                    ToolTip.visible: hovered
                    onClicked: page.invertSelection("tilt")
                }
                AppButton {
                    text: page.toolbarCompact ? "" : qsTr("Limits")
                    iconGlyph: Theme.iconRuler
                    enabled: page.selectedIds.length > 0
                    ToolTip.text: qsTr("Dimmer, pan and tilt limits, inverted or swapped axes, and the options of each channel")
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                    onClicked: optionsDialog.openFor(page.selectedIds)
                }
                Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 24; color: Theme.border }
                AppButton {
                    text: page.toolbarCompact ? "" : qsTr("Edit")
                    iconGlyph: Theme.iconPen
                    ToolTip.text: qsTr("Edit"); ToolTip.visible: page.toolbarCompact && hovered
                    enabled: page.selectedIds.length === 1
                    onClicked: editDialog.edit(page.selectedFixtures()[0])
                }
                AppButton {
                    text: qsTr("Re-address")
                    enabled: page.selectedIds.length > 0
                    onClicked: {
                        // in the order of the list (by address, under each type when
                        // grouped), from the lowest address of the selection
                        let sel = page.rows.filter(r => !r.header && page.isSelected(r.fixture.id)).map(r => r.fixture)
                        let low = sel.slice().sort((a, b) => a.universe - b.universe || a.address - b.address)[0]
                        readdressDialog.edit(sel.map(f => f.id), low.universe, low.address)
                    }
                }
                AppButton {
                    text: qsTr("Renumber")
                    enabled: page.selectedIds.length > 0
                    ToolTip.text: qsTr("Give the selected fixtures numbers one after the other, in address order (the command line of Program uses them)")
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                    onClicked: {
                        let sel = page.selectedFixtures().sort((a, b) => a.universe - b.universe || a.address - b.address)
                        let ids = sel.map(f => f.id)
                        prompt.ask(qsTr("First number"), String(sel[0].number || 1), (text) => {
                            let error = Core.patch.renumberFixtures(ids, parseInt(text))
                            page.message(error.length > 0 ? error : qsTr("%n fixture(s) renumbered", "", ids.length), error.length > 0)
                        })
                    }
                }
                AppButton {
                    text: page.toolbarCompact ? "" : qsTr("Create group")
                    iconGlyph: Theme.iconSelect
                    ToolTip.text: qsTr("Create group"); ToolTip.visible: page.toolbarCompact && hovered
                    enabled: page.selectedIds.length > 0
                    onClicked: prompt.ask(qsTr("New fixture group"), "", (name) => Core.patch.createGroup(name, page.selectedIds))
                }
                AppButton {
                    text: page.toolbarCompact ? "" : qsTr("Delete")
                    iconGlyph: Theme.iconTrash
                    accentColor: Theme.danger
                    enabled: page.selectedIds.length > 0
                    ToolTip.text: qsTr("Delete"); ToolTip.visible: page.toolbarCompact && hovered
                    onClicked: deleteConfirm.open()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            // fixture groups, with their selections
            Panel {
                Layout.preferredWidth: 250
                Layout.fillHeight: true
                title: qsTr("Fixture groups")

                ListView {
                    id: groupList
                    anchors { fill: parent; margins: 6 }
                    clip: true
                    spacing: 4
                    model: Core.patch.groups
                    ScrollBar.vertical: ScrollBar {}

                    delegate: Column {
                        id: groupItem
                        required property var modelData
                        width: groupList.width
                        spacing: 2
                        readonly property var selections: Core.patch.selections.filter(s => s.groupId === modelData.id)
                        readonly property bool folded: page.foldedGroups.indexOf(modelData.id) >= 0

                        Rectangle {
                            id: groupRow
                            width: parent.width
                            height: 44
                            radius: Theme.radiusSmall
                            readonly property bool allSelected: groupItem.modelData.fixtures.length > 0 &&
                                                                groupItem.modelData.fixtures.every(id => page.isSelected(id))
                            color: allSelected ? Qt.alpha(Theme.accent, 0.2) : (groupMouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised)
                            border.color: allSelected ? Theme.accent : Theme.border

                            MouseArea {
                                id: groupMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                onClicked: (ev) => {
                                    if (ev.button === Qt.RightButton) {
                                        groupMenu.groupId = groupItem.modelData.id
                                        groupMenu.groupName = groupItem.modelData.name
                                        groupMenu.fixtures = groupItem.modelData.fixtures
                                        groupMenu.popup()
                                    } else {
                                        page.select(groupItem.modelData.fixtures, (ev.modifiers & (Qt.ControlModifier | Qt.ShiftModifier)) !== 0)
                                    }
                                }
                            }
                            // its selections, folded or not
                            AppButton {
                                id: foldButton
                                visible: groupItem.selections.length > 0
                                anchors { left: parent.left; leftMargin: 2; verticalCenter: parent.verticalCenter }
                                implicitWidth: 24
                                implicitHeight: 26
                                flat: true
                                iconGlyph: groupItem.folded ? Theme.iconChevronRight : Theme.iconChevronDown
                                ToolTip.text: groupItem.folded ? qsTr("Show its selections") : qsTr("Hide its selections")
                                ToolTip.visible: hovered
                                ToolTip.delay: 500
                                onClicked: page.foldedGroups = page.toggleFolded(page.foldedGroups, groupItem.modelData.id)
                            }
                            Column {
                                anchors { left: parent.left; leftMargin: foldButton.visible ? 28 : 10; right: parent.right; rightMargin: 34
                                          verticalCenter: parent.verticalCenter }
                                Text { width: parent.width; text: groupItem.modelData.name; color: Theme.text; font.weight: Font.DemiBold; elide: Text.ElideRight }
                                Text {
                                    text: qsTr("%n fixture(s)", "", groupItem.modelData.fixtures.length)
                                          + (groupItem.selections.length ? "  ·  " + qsTr("%n selection(s)", "", groupItem.selections.length) : "")
                                    color: groupRow.allSelected ? Theme.textDim : Theme.textFaint
                                    font.pixelSize: Theme.fontSmall
                                }
                            }
                            AppButton {
                                anchors { right: parent.right; rightMargin: 4; verticalCenter: parent.verticalCenter }
                                implicitWidth: 28
                                implicitHeight: 26
                                flat: true
                                iconGlyph: Theme.iconPlus
                                enabled: page.selectedIds.length > 0
                                ToolTip.text: qsTr("Save the selection, in its order, as a selection of this group")
                                ToolTip.visible: hovered
                                ToolTip.delay: 500
                                onClicked: page.saveSelection(groupItem.modelData.id)
                            }
                        }

                        // its selections: ordered parts of the group
                        Repeater {
                            model: groupItem.folded ? [] : groupItem.selections
                            delegate: Rectangle {
                                id: selectionRow
                                required property var modelData
                                readonly property bool active: JSON.stringify(page.selectedIds) === JSON.stringify(modelData.fixtures)
                                x: 14
                                width: groupItem.width - 14
                                height: 30
                                radius: Theme.radiusSmall
                                color: active ? Qt.alpha(Theme.accent, 0.2) : (selectionMouse.containsMouse ? Theme.surfaceHover : "transparent")
                                border.color: active ? Theme.accent : Theme.border
                                MouseArea {
                                    id: selectionMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                                    onClicked: (ev) => {
                                        if (ev.button === Qt.RightButton) {
                                            selectionMenu.selectionId = selectionRow.modelData.id
                                            selectionMenu.selectionName = selectionRow.modelData.name
                                            selectionMenu.popup()
                                        } else {
                                            page.select(selectionRow.modelData.fixtures, (ev.modifiers & (Qt.ControlModifier | Qt.ShiftModifier)) !== 0)
                                        }
                                    }
                                }
                                RowLayout {
                                    anchors { fill: parent; leftMargin: 8; rightMargin: 8 }
                                    IconLabel { icon: Theme.iconSelect; size: 11; color: Theme.textDim }
                                    Text {
                                        Layout.fillWidth: true
                                        text: selectionRow.modelData.name
                                        color: Theme.text
                                        elide: Text.ElideRight
                                        font.pixelSize: Theme.fontSmall
                                    }
                                    Text {
                                        text: selectionRow.modelData.fixtures.length
                                        color: Theme.textFaint
                                        font.pixelSize: Theme.fontSmall
                                    }
                                }
                            }
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        width: parent.width - 30
                        visible: groupList.count === 0
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        text: qsTr("Select fixtures and press “Create group” to make groups like “Front wash” or “Spots”.")
                        color: Theme.textFaint
                        font.pixelSize: Theme.fontSmall
                    }
                }
            }

            // list
            Panel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: page.view === "list"

                ColumnLayout {
                    anchors { fill: parent; margins: 1 }
                    spacing: 0

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        color: "transparent"
                        RowLayout {
                            anchors { fill: parent; leftMargin: 14; rightMargin: 14 }
                            spacing: 12
                            Item { Layout.preferredWidth: 20 }
                            Text { Layout.preferredWidth: 34; text: "#"; color: Theme.textFaint; font.pixelSize: Theme.fontSmall; font.weight: Font.Bold }
                            Text { Layout.preferredWidth: page.compact ? 140 : 220; text: qsTr("NAME"); color: Theme.textFaint; font.pixelSize: Theme.fontSmall; font.weight: Font.Bold }
                            Text { Layout.preferredWidth: 90; text: qsTr("ADDRESS"); color: Theme.textFaint; font.pixelSize: Theme.fontSmall; font.weight: Font.Bold }
                            Text { Layout.preferredWidth: 60; text: qsTr("CH"); color: Theme.textFaint; font.pixelSize: Theme.fontSmall; font.weight: Font.Bold }
                            Text { Layout.fillWidth: true; visible: !page.compact; elide: Text.ElideRight; text: qsTr("FIXTURE"); color: Theme.textFaint; font.pixelSize: Theme.fontSmall; font.weight: Font.Bold }
                            Text { Layout.preferredWidth: page.compact ? 120 : 160; text: qsTr("MODE"); color: Theme.textFaint; font.pixelSize: Theme.fontSmall; font.weight: Font.Bold }
                            Text { Layout.preferredWidth: 76; text: qsTr("INVERT"); color: Theme.textFaint; font.pixelSize: Theme.fontSmall; font.weight: Font.Bold }
                            Item { Layout.preferredWidth: 32 }
                        }
                        Rectangle { height: 1; color: Theme.border; anchors { left: parent.left; right: parent.right; bottom: parent.bottom } }
                    }

                    ListView {
                        id: table
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        // by count: a change of a fixture (an inversion, a
                        // name) updates its row where it is, the list stays
                        // where it was
                        model: page.rows.length
                        boundsBehavior: Flickable.StopAtBounds
                        ScrollBar.vertical: ScrollBar {}

                        delegate: Loader {
                            id: rowLoader
                            required property int index
                            readonly property var modelData: page.rows[index] || ({ header: true, title: "", ids: [], folded: false })
                            width: table.width
                            sourceComponent: modelData.header ? headerRow : fixtureRow

                            Component {
                                id: headerRow
                                Rectangle {
                                    height: 36
                                    color: Theme.background
                                    readonly property bool allSelected: rowLoader.modelData.ids.every(id => page.isSelected(id))
                                    // a click on the header folds or unfolds its fixtures
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: page.foldedTypes = page.toggleFolded(page.foldedTypes, rowLoader.modelData.title)
                                    }
                                    RowLayout {
                                        anchors { fill: parent; leftMargin: 14; rightMargin: 14 }
                                        spacing: 10
                                        CheckBox {
                                            checked: parent.parent.allSelected
                                            onClicked: {
                                                if (parent.parent.allSelected)
                                                    page.selectedIds = page.selectedIds.filter(id => rowLoader.modelData.ids.indexOf(id) < 0)
                                                else
                                                    page.select(rowLoader.modelData.ids, true)
                                            }
                                        }
                                        IconLabel {
                                            icon: rowLoader.modelData.folded ? Theme.iconChevronRight : Theme.iconChevronDown
                                            size: 11
                                            color: Theme.textDim
                                        }
                                        Text {
                                            Layout.fillWidth: true
                                            text: rowLoader.modelData.title
                                            color: Theme.text
                                            font.weight: Font.Bold
                                            elide: Text.ElideRight
                                        }
                                        Text {
                                            text: qsTr("%n fixture(s)", "", rowLoader.modelData.ids.length)
                                            color: Theme.textFaint
                                            font.pixelSize: Theme.fontSmall
                                        }
                                    }
                                }
                            }

                            Component {
                                id: fixtureRow
                                Rectangle {
                                    readonly property var fx: rowLoader.modelData.fixture
                                    readonly property bool selected: page.isSelected(fx.id)
                                    height: 40
                                    color: selected ? Qt.alpha(Theme.accent, 0.18)
                                                    : (rowMouse.containsMouse ? Theme.surfaceHover : "transparent")

                                    MouseArea {
                                        id: rowMouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onClicked: (ev) => page.clickFixture(fx.id, ev.modifiers)
                                        onDoubleClicked: editDialog.edit(fx)
                                    }

                                    RowLayout {
                                        anchors { fill: parent; leftMargin: 14; rightMargin: 14 }
                                        spacing: 12
                                        CheckBox {
                                            Layout.preferredWidth: 20
                                            checked: parent.parent.selected
                                            onClicked: { page.select([parent.parent.fx.id], true); page.anchorId = parent.parent.fx.id }
                                        }
                                        Text {
                                            Layout.preferredWidth: 34
                                            text: parent.parent.fx.number || ""
                                            color: parent.parent.selected ? Theme.text : Theme.accent
                                            font.family: Theme.monoFamily
                                            font.weight: Font.Bold
                                        }
                                        Text { Layout.preferredWidth: page.compact ? 140 : 220; text: parent.parent.fx.name; color: Theme.text; elide: Text.ElideRight }
                                        Text {
                                            Layout.preferredWidth: 90
                                            text: (parent.parent.fx.universe + 1) + " . " + parent.parent.fx.address
                                            color: Theme.text
                                            font.family: Theme.monoFamily
                                        }
                                        Text { Layout.preferredWidth: 60; text: parent.parent.fx.channels; color: Theme.textDim; font.family: Theme.monoFamily }
                                        Text {
                                            Layout.fillWidth: true
                                            visible: !page.compact
                                            text: parent.parent.fx.manufacturer.length ? parent.parent.fx.manufacturer + " — " + parent.parent.fx.model : parent.parent.fx.type
                                            color: Theme.textDim
                                            elide: Text.ElideRight
                                        }
                                        Text { Layout.preferredWidth: page.compact ? 120 : 160; text: parent.parent.fx.mode; color: Theme.textDim; elide: Text.ElideRight }
                                        Row {
                                            Layout.preferredWidth: 76
                                            spacing: 4
                                            AppButton {
                                                implicitHeight: 26; implicitWidth: 36
                                                flat: !highlighted
                                                iconGlyph: Theme.iconArrowsH
                                                visible: parent.parent.parent.fx.hasPan
                                                highlighted: parent.parent.parent.fx.invertPan
                                                ToolTip.text: qsTr("Invert pan"); ToolTip.visible: hovered
                                                onClicked: Core.patch.setInverted([parent.parent.parent.fx.id], "pan", !highlighted)
                                            }
                                            AppButton {
                                                implicitHeight: 26; implicitWidth: 36
                                                flat: !highlighted
                                                iconGlyph: Theme.iconArrowsV
                                                visible: parent.parent.parent.fx.hasTilt
                                                highlighted: parent.parent.parent.fx.invertTilt
                                                ToolTip.text: qsTr("Invert tilt"); ToolTip.visible: hovered
                                                onClicked: Core.patch.setInverted([parent.parent.parent.fx.id], "tilt", !highlighted)
                                            }
                                        }
                                        AppButton {
                                            Layout.preferredWidth: 32
                                            implicitHeight: 26
                                            flat: !highlighted
                                            iconGlyph: Theme.iconLocate
                                            accentColor: Theme.warning
                                            highlighted: Core.patch.identified.indexOf(parent.parent.fx.id) >= 0
                                            ToolTip.text: qsTr("Identify: blink this fixture in white"); ToolTip.visible: hovered
                                            onClicked: Core.patch.toggleIdentify(parent.parent.fx.id)
                                        }
                                    }
                                }
                            }
                        }

                        Column {
                            anchors.centerIn: parent
                            visible: table.count === 0
                            spacing: 12
                            IconLabel { anchors.horizontalCenter: parent.horizontalCenter; icon: Theme.iconLight; size: 34; color: Theme.textFaint }
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: qsTr("Add the fixtures of your rig to start")
                                color: Theme.textDim
                            }
                            AppButton {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: qsTr("Add fixtures")
                                iconGlyph: Theme.iconPlus
                                onClicked: addDialog.open()
                            }
                        }
                    }
                }
            }

            // channels of each universe
            Panel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: page.view === "grid"
                title: qsTr("Channels")

                PatchGrid {
                    anchors { fill: parent; margins: 10 }
                    fixtures: page.fixtures
                    selection: page.selectedIds
                    onSelectRequested: (ids, additive) => page.select(ids, additive)
                    onEditRequested: (fixture) => editDialog.edit(fixture)
                    onMessage: (text, isError) => page.message(text, isError)
                }
            }

            // 2D plan
            Panel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: page.planView
                title: qsTr("Stage plan")
                headerItems: [
                    Text { text: qsTr("Width (m)"); color: Theme.textDim; font.pixelSize: Theme.fontSmall; anchors.verticalCenter: parent.verticalCenter },
                    SpinBox {
                        from: 1; to: 100; editable: true
                        value: Math.round(Core.patch.stageSize.width / 1000)
                        onValueModified: Core.patch.setStageSize(value * 1000, Core.patch.stageSize.height)
                    },
                    Text { text: qsTr("Depth (m)"); color: Theme.textDim; font.pixelSize: Theme.fontSmall; anchors.verticalCenter: parent.verticalCenter },
                    SpinBox {
                        from: 1; to: 100; editable: true
                        value: Math.round(Core.patch.stageSize.height / 1000)
                        onValueModified: Core.patch.setStageSize(Core.patch.stageSize.width, value * 1000)
                    },
                    AppButton { text: qsTr("Auto arrange"); iconGlyph: Theme.iconWand; flat: true; onClicked: Core.patch.autoLayout(true) }
                ]

                ColumnLayout {
                    anchors { fill: parent; margins: 8 }
                    spacing: 6

                    // tools: they work on the selection, in its order
                    Flow {
                        Layout.fillWidth: true
                        spacing: 4
                        AppButton {
                            text: qsTr("Live")
                            iconGlyph: Theme.iconEye
                            implicitHeight: 28
                            highlighted: stagePlan.live
                            ToolTip.text: qsTr("Show the colour, dimmer, strobe and gobo of each fixture")
                            ToolTip.visible: hovered
                            ToolTip.delay: 500
                            onClicked: stagePlan.live = !stagePlan.live
                        }
                        AppButton {
                            text: qsTr("Lasso")
                            iconGlyph: Theme.iconLasso
                            implicitHeight: 28
                            highlighted: stagePlan.lasso
                            ToolTip.text: qsTr("Draw around the fixtures to select them")
                            ToolTip.visible: hovered
                            ToolTip.delay: 500
                            onClicked: stagePlan.lasso = !stagePlan.lasso
                        }
                        Rectangle { width: 1; height: 28; color: Theme.border }
                        // the tools: select, or move the fixtures
                        Row {
                            spacing: 0
                            AppButton {
                                text: qsTr("Select")
                                iconGlyph: Theme.iconSelect
                                implicitHeight: 28
                                highlighted: stagePlan.tool === "select"
                                ToolTip.text: qsTr("Click or draw around the fixtures to select them")
                                ToolTip.visible: hovered
                                ToolTip.delay: 500
                                onClicked: stagePlan.tool = "select"
                            }
                            AppButton {
                                text: qsTr("Move")
                                iconGlyph: Theme.iconMove
                                implicitHeight: 28
                                highlighted: stagePlan.tool === "move"
                                ToolTip.text: qsTr("Drag the fixtures (the whole selection) or the view")
                                ToolTip.visible: hovered
                                ToolTip.delay: 500
                                onClicked: stagePlan.tool = "move"
                            }
                        }
                        Text { text: qsTr("Grid"); color: Theme.textDim; font.pixelSize: Theme.fontSmall; height: 28; verticalAlignment: Text.AlignVCenter }
                        ComboBox {
                            id: snapCombo
                            implicitHeight: 28
                            implicitWidth: 110
                            readonly property var steps: [0, 100, 250, 500, 1000]
                            model: [qsTr("Free"), "10 cm", "25 cm", "50 cm", "1 m"]
                            currentIndex: 3
                        }
                        AppButton {
                            text: qsTr("Look")
                            iconGlyph: Theme.iconCircle
                            implicitHeight: 28
                            enabled: page.selectedIds.length > 0
                            ToolTip.text: qsTr("How the selected fixtures look on the plan")
                            ToolTip.visible: hovered
                            ToolTip.delay: 500
                            onClicked: lookMenu.popup()
                            AppMenu {
                                id: lookMenu
                                MenuItem { text: qsTr("Round"); onTriggered: Core.patch.setFixtureShape(page.selectedIds, "round") }
                                MenuItem { text: qsTr("Square"); onTriggered: Core.patch.setFixtureShape(page.selectedIds, "square") }
                                MenuItem { text: qsTr("Rectangle"); onTriggered: Core.patch.setFixtureShape(page.selectedIds, "rectangle") }
                            }
                        }
                        Rectangle { width: 1; height: 28; color: Theme.border }
                        AppButton {
                            text: qsTr("Arrange")
                            iconGlyph: Theme.iconShapes
                            implicitHeight: 28
                            enabled: page.selectedIds.length > 1
                            onClicked: shapeMenu.popup()
                            AppMenu {
                                id: shapeMenu
                                MenuItem { text: qsTr("Line"); onTriggered: page.arrange("line", {}) }
                                MenuItem { text: qsTr("Line, front to back"); onTriggered: page.arrange("line", { vertical: true }) }
                                MenuItem { text: qsTr("Circle"); onTriggered: page.arrange("circle", {}) }
                                MenuItem { text: qsTr("Grid, by rows"); onTriggered: page.arrange("grid", { order: "rows" }) }
                                MenuItem { text: qsTr("Grid, snake"); onTriggered: page.arrange("grid", { order: "snake" }) }
                                MenuItem { text: qsTr("Grid, by columns"); onTriggered: page.arrange("grid", { order: "columns" }) }
                                MenuItem { text: qsTr("Grid, snake by columns"); onTriggered: page.arrange("grid", { order: "snakeColumns" }) }
                            }
                        }
                        Text { text: qsTr("Spacing (cm)"); color: Theme.textDim; font.pixelSize: Theme.fontSmall; height: 28; verticalAlignment: Text.AlignVCenter }
                        SpinBox {
                            id: spacingSpin
                            implicitHeight: 28
                            implicitWidth: 110
                            from: 10; to: 2000; stepSize: 10; value: 60; editable: true
                        }
                        Text { text: qsTr("Columns"); color: Theme.textDim; font.pixelSize: Theme.fontSmall; height: 28; verticalAlignment: Text.AlignVCenter }
                        SpinBox {
                            id: columnsSpin
                            implicitHeight: 28
                            implicitWidth: 90
                            from: 1; to: 64; value: 4; editable: true
                        }
                        AppButton {
                            text: qsTr("Align")
                            iconGlyph: Theme.iconAlign
                            implicitHeight: 28
                            enabled: page.selectedIds.length > 1
                            onClicked: alignMenu.popup()
                            AppMenu {
                                id: alignMenu
                                MenuItem { text: qsTr("Left"); onTriggered: Core.patch.align(page.selectedIds, "left") }
                                MenuItem { text: qsTr("Centre"); onTriggered: Core.patch.align(page.selectedIds, "centerX") }
                                MenuItem { text: qsTr("Right"); onTriggered: Core.patch.align(page.selectedIds, "right") }
                                MenuSeparator {}
                                MenuItem { text: qsTr("Back"); onTriggered: Core.patch.align(page.selectedIds, "top") }
                                MenuItem { text: qsTr("Middle"); onTriggered: Core.patch.align(page.selectedIds, "centerY") }
                                MenuItem { text: qsTr("Front"); onTriggered: Core.patch.align(page.selectedIds, "bottom") }
                                MenuSeparator {}
                                MenuItem { text: qsTr("Distribute across"); onTriggered: Core.patch.distribute(page.selectedIds, "horizontal") }
                                MenuItem { text: qsTr("Distribute front to back"); onTriggered: Core.patch.distribute(page.selectedIds, "vertical") }
                            }
                        }
                        AppButton {
                            iconGlyph: Theme.iconRotate
                            text: "15°"
                            implicitHeight: 28
                            enabled: page.selectedIds.length > 0
                            ToolTip.text: qsTr("Turn the selection 15° (right click: the other way)")
                            ToolTip.visible: hovered
                            ToolTip.delay: 500
                            onClicked: Core.patch.rotate(page.selectedIds, 15)
                            TapHandler {
                                acceptedButtons: Qt.RightButton
                                onTapped: Core.patch.rotate(page.selectedIds, -15)
                            }
                        }
                        AppButton {
                            iconGlyph: Theme.iconRotate
                            text: "90°"
                            implicitHeight: 28
                            enabled: page.selectedIds.length > 0
                            onClicked: Core.patch.rotate(page.selectedIds, 90)
                        }
                        Rectangle { width: 1; height: 28; color: Theme.border }
                        // Follow: how high the moving heads hang and the way they face
                        AppButton {
                            text: qsTr("Height and facing")
                            iconGlyph: Theme.iconLocate
                            implicitHeight: 28
                            enabled: page.selectedFixtures().some(f => f.aimable)
                            ToolTip.text: qsTr("Where the selected moving heads hang, so Follow can aim them")
                            ToolTip.visible: hovered
                            ToolTip.delay: 500
                            onClicked: aimDialog.openFor(page.selectedIds)
                        }
                    }

                    StagePlan {
                        id: stagePlan
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        fixtures: page.fixtures
                        selection: page.selectedIds
                        editable: true
                        onSelectRequested: (ids, additive) => page.select(ids, additive)
                        snap: snapCombo.steps[snapCombo.currentIndex]
                        onMovedBy: (ids, dx, dy) => Core.patch.moveFixtures(ids, dx, dy, snap)
                    }
                }
            }
        }
    }

    AppMenu {
        id: groupMenu
        property int groupId: -1
        property string groupName: ""
        property var fixtures: []

        MenuItem { text: qsTr("Rename"); onTriggered: prompt.ask(qsTr("Rename group"), groupMenu.groupName, (name) => Core.patch.renameGroup(groupMenu.groupId, name)) }
        MenuItem {
            text: qsTr("Replace with selection")
            enabled: page.selectedIds.length > 0
            onTriggered: Core.patch.setGroupFixtures(groupMenu.groupId, page.selectedIds)
        }
        MenuItem {
            text: qsTr("Save the selection here…")
            enabled: page.selectedIds.length > 0
            onTriggered: page.saveSelection(groupMenu.groupId)
        }
        MenuSeparator {}
        // selection patterns, in the order of the group
        MenuItem { text: qsTr("Select odd"); onTriggered: page.selectedIds = Core.patch.patternSelection(groupMenu.fixtures, "odd") }
        MenuItem { text: qsTr("Select even"); onTriggered: page.selectedIds = Core.patch.patternSelection(groupMenu.fixtures, "even") }
        MenuItem { text: qsTr("Select in reverse order"); onTriggered: page.selectedIds = Core.patch.patternSelection(groupMenu.fixtures, "reverse") }
        MenuItem { text: qsTr("Select the first half"); onTriggered: page.selectedIds = Core.patch.patternSelection(groupMenu.fixtures, "firstHalf") }
        MenuItem { text: qsTr("Select the second half"); onTriggered: page.selectedIds = Core.patch.patternSelection(groupMenu.fixtures, "secondHalf") }
        MenuItem { text: qsTr("Select from the ends to the middle"); onTriggered: page.selectedIds = Core.patch.patternSelection(groupMenu.fixtures, "outsideIn") }
        MenuItem { text: qsTr("Select from the middle to the ends"); onTriggered: page.selectedIds = Core.patch.patternSelection(groupMenu.fixtures, "insideOut") }
        MenuSeparator {}
        MenuItem { text: qsTr("Delete group"); onTriggered: Core.patch.deleteGroup(groupMenu.groupId) }
    }

    AppMenu {
        id: selectionMenu
        property int selectionId: -1
        property string selectionName: ""

        MenuItem { text: qsTr("Rename"); onTriggered: prompt.ask(qsTr("Rename selection"), selectionMenu.selectionName, (name) => Core.patch.renameSelection(selectionMenu.selectionId, name)) }
        MenuItem { text: qsTr("Delete selection"); onTriggered: Core.patch.removeSelection(selectionMenu.selectionId) }
    }

    Popup {
        id: deleteConfirm
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        padding: 18
        background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }
        contentItem: ColumnLayout {
            spacing: 12
            Text {
                text: qsTr("Delete %n fixture(s)?", "", page.selectedIds.length)
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
            }
            Text { text: qsTr("Their values are removed from every scene."); color: Theme.textDim }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton { text: qsTr("Cancel"); onClicked: deleteConfirm.close() }
                AppButton {
                    text: qsTr("Delete")
                    accentColor: Theme.danger
                    primary: true
                    onClicked: { Core.patch.removeFixtures(page.selectedIds); page.selectedIds = []; deleteConfirm.close() }
                }
            }
        }
    }

    AddFixtureDialog { id: addDialog }
    EditFixtureDialog { id: editDialog }
    ReaddressDialog { id: readdressDialog }
    FixtureOptionsDialog { id: optionsDialog }
    AimSettingsDialog { id: aimDialog }
    MatrixDialog {
        id: matrixDialog
        onMessage: (text) => page.message(text, false)
    }
    TextPrompt { id: prompt }

    FileDialog {
        id: importDialog
        property bool patch: true
        property bool scenes: false
        title: patch && scenes ? qsTr("Import the patch and the scenes of a show")
             : patch ? qsTr("Import the patch of a show") : qsTr("Import the scenes of a show")
        nameFilters: [qsTr("Show files (*.qxw)"), qsTr("All files (*)")]
        onAccepted: {
            let done = Core.importFromShow(selectedFile, patch, scenes)
            if (done.length)
                page.message(done, false)
            else
                page.message(Core.lastError, true)
        }
    }
}
