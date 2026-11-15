/*
  DMX Total Control
  PatchImportDialog.qml

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
 * Patch import: the fixture table (CSV) a visualizer exports. What the
 * import would do (each fixture type of the table and what it patches, the
 * options, the problems); nothing changes until Import. Open it with
 * openFile().
 */
Popup {
    id: dialog

    property url file: ""
    /** Core.patch.tablePreview() */
    property var preview: ({})
    /** The fixtures chosen by hand, by type: { key: { manufacturer, model,
     *  mode } | { generic } | { skip } | { auto } } */
    property var choices: ({})
    /** place, view, groupsByType, groupsByLayer, groupsFromTable, removeMissing */
    property var options: ({})
    readonly property bool ok: preview.ok === true
    readonly property var types: ok ? preview.types : []
    readonly property var problems: ok ? preview.problems : []
    readonly property int importCount: ok ? preview.newCount + preview.updateCount : 0
    readonly property bool removing: ok && options.removeMissing === true && preview.removeCount > 0
    /** The type the library picker chooses for */
    property var picking: null

    signal message(string text, bool isError)

    function openFile(url) {
        file = url
        choices = ({})
        options = ({})
        refresh()
        open()
    }
    function refresh() {
        let opts = Object.assign({}, options)
        opts.choices = choices
        preview = Core.patch.tablePreview(file, opts)
        // (the options in effect: the ones not given take their default)
        if (ok)
            options = preview.options
    }
    /** refresh(), the list of types where it was (a new preview is a new
     *  model: the list would go back to the top) */
    function refreshInPlace() {
        const y = typeList.contentY - typeList.originY
        refresh()
        Qt.callLater(() => {
            typeList.contentY = typeList.originY + Math.max(0, Math.min(y, typeList.contentHeight - typeList.height))
        })
    }
    function setOption(key, value) {
        let o = Object.assign({}, options)
        o[key] = value
        options = o
        refreshInPlace()
    }
    function choose(key, choice) {
        let c = Object.assign({}, choices)
        c[key] = choice
        choices = c
        refreshInPlace()
    }
    function pick(type) {
        picking = type
        picker.pickFor = type.fixture.length ? type.fixture : qsTr("No name")
        picker.pickChannels = type.channels
        picker.open()
        // what it patches now; not found: the model found with no mode like
        // it, or the manufacturer the table names; else nothing chosen (not
        // the fixture chosen for the type before)
        if (type.manufacturer.length) {
            picker.choose(type.manufacturer, type.model)
            picker.chooseMode(type.modeName)
        } else if (type.foundModel.length) {
            picker.choose(type.foundManufacturer, type.foundModel)
        } else if (type.foundManufacturer.length) {
            picker.showManufacturer(type.foundManufacturer)
        } else {
            picker.showManufacturer("Generic")
        }
    }
    /** Developer option --patch-import-pick: the picker of the first type */
    function devPick() {
        if (types.length > 0)
            pick(types.find(t => t.generic) || types[0])
    }
    function doImport() {
        let opts = Object.assign({}, options)
        opts.choices = choices
        const done = Core.patch.importTable(file, opts)
        if (done.length) {
            close()
            message(done, false)
        } else {
            message(Core.patch.lastError(), true)
        }
    }

    function statusText(type) {
        switch (type.status) {
        case "exact": return qsTr("Exact")
        case "channels": return qsTr("By channel count")
        case "similar": return qsTr("Similar name")
        case "remembered": return qsTr("Remembered")
        case "manual": return qsTr("Chosen by hand")
        case "generic": return qsTr("Not in the library")
        case "skip": return qsTr("Not imported")
        }
        return ""
    }
    /** What a type patches: the fixture, and its mode with how it was found */
    function targetText(type) {
        if (type.status === "skip")
            return qsTr("Don't import")
        if (type.generic)
            return qsTr("Generic dimmer")
        return type.manufacturer + " " + type.model
    }
    function targetDetail(type) {
        let parts = []
        if (type.status !== "skip")
            parts.push(type.generic ? qsTr("%n channel(s)", "", type.modeChannels) : type.modeName)
        parts.push(statusText(type))
        if (type.note.length)
            parts.push(type.note)
        return parts.join(" · ")
    }
    function universesText() {
        if (!ok || preview.universeFrom <= 0)
            return ""
        return preview.universeFrom === preview.universeTo ? qsTr("in universe %1").arg(preview.universeFrom)
                                                           : qsTr("in universes %1 to %2").arg(preview.universeFrom).arg(preview.universeTo)
    }
    function metres(mm) {
        return (mm / 1000).toLocaleString(Qt.locale(), 'f', 0)
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(1040, parent ? parent.width - 60 : 1040)
    height: Math.min(700, parent ? parent.height - 60 : 700)
    modal: true
    padding: 18

    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    /** A title of a group of options */
    component OptionTitle: Text {
        Layout.fillWidth: true
        Layout.topMargin: 6
        color: Theme.textFaint
        font.pixelSize: Theme.fontSmall
        font.weight: Font.Bold
        font.capitalization: Font.AllUppercase
        wrapMode: Text.WordWrap
    }
    /** A check box whose text wraps */
    component OptionCheck: CheckBox {
        id: check
        Layout.fillWidth: true
        contentItem: Text {
            leftPadding: check.indicator.width + check.spacing
            text: check.text
            color: check.enabled ? Theme.text : Theme.textFaint
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
        }
    }
    component Hint: Text {
        Layout.fillWidth: true
        color: Theme.textDim
        font.pixelSize: Theme.fontSmall
        wrapMode: Text.WordWrap
    }

    contentItem: ColumnLayout {
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            IconLabel { icon: Theme.iconImport; size: 16; color: Theme.accent }
            Text {
                text: qsTr("Import a patch")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
            }
            Text {
                Layout.fillWidth: true
                text: dialog.preview.fileName || ""
                color: Theme.textDim
                elide: Text.ElideMiddle
                horizontalAlignment: Text.AlignRight
            }
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("The fixture table (CSV) your visualizer exports. Check what each type of fixture becomes: nothing changes until you press Import.")
            color: Theme.textDim
        }

        // the file can't be imported
        Text {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !dialog.ok
            wrapMode: Text.WordWrap
            text: dialog.preview.error || ""
            color: Theme.danger
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: dialog.ok
            spacing: 12

            // the types of fixture, and the problems
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: dialog.ok ? qsTr("%n fixture(s) in the table", "", dialog.preview.rows) + ", "
                                      + qsTr("of %n type(s)", "", dialog.types.length) + ", " + dialog.universesText()
                                    : ""
                    color: Theme.text
                    font.weight: Font.DemiBold
                }

                Panel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    title: qsTr("Fixture types")

                    ListView {
                        id: typeList
                        anchors { fill: parent; margins: 4 }
                        clip: true
                        spacing: 2
                        boundsBehavior: Flickable.StopAtBounds
                        model: dialog.types
                        ScrollBar.vertical: ScrollBar {}

                        delegate: Rectangle {
                            id: typeRow
                            required property var modelData
                            // to check: not found, found by similar words (a guess), or a note
                            readonly property bool warn: modelData.generic || modelData.status === "similar" || modelData.note.length > 0
                            readonly property bool skipped: modelData.status === "skip"
                            width: ListView.view.width
                            implicitHeight: typeLayout.implicitHeight + 14
                            radius: Theme.radiusSmall
                            color: typeMouse.containsMouse ? Theme.surfaceHover : "transparent"
                            MouseArea { id: typeMouse; anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.NoButton }

                            RowLayout {
                                id: typeLayout
                                anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; leftMargin: 8; rightMargin: 6 }
                                spacing: 10

                                Text {
                                    Layout.preferredWidth: 42
                                    text: "×" + typeRow.modelData.count
                                    color: Theme.accent
                                    font.family: Theme.monoFamily
                                    font.weight: Font.Bold
                                    horizontalAlignment: Text.AlignRight
                                }
                                // in the table
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 1
                                    spacing: 1
                                    Text {
                                        Layout.fillWidth: true
                                        text: typeRow.modelData.fixture.length ? typeRow.modelData.fixture : qsTr("No name")
                                        color: Theme.text
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: (typeRow.modelData.mode.length ? typeRow.modelData.mode + " · " : "")
                                              + (typeRow.modelData.channels > 0 ? qsTr("%n channel(s)", "", typeRow.modelData.channels)
                                                                                : qsTr("channels not given"))
                                        color: Theme.textDim
                                        font.pixelSize: Theme.fontSmall
                                        elide: Text.ElideRight
                                    }
                                }
                                IconLabel { icon: Theme.iconChevronRight; size: 11; color: Theme.textFaint }
                                // the glyph its fixtures get on the plan (a generic
                                // dimmer: the kind the name in the table says)
                                Item {
                                    Layout.preferredWidth: 22
                                    Layout.preferredHeight: 22
                                    opacity: typeRow.skipped ? 0 : 1
                                    IconLabel {
                                        anchors.centerIn: parent
                                        icon: Theme.kindIcon(typeRow.modelData.kind)
                                        size: 15
                                        color: Theme.text
                                    }
                                    HoverHandler { id: kindHover }
                                    ToolTip.visible: kindHover.hovered && !typeRow.skipped
                                    ToolTip.delay: 400
                                    ToolTip.text: qsTr("On the plan: %1").arg(Theme.kindName(typeRow.modelData.kind))
                                                  + (typeRow.modelData.icon.length ? " " + qsTr("(from its name in the table)") : "")
                                }
                                // what it patches
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 1
                                    spacing: 1
                                    Text {
                                        Layout.fillWidth: true
                                        text: dialog.targetText(typeRow.modelData)
                                        color: typeRow.skipped ? Theme.textFaint : typeRow.warn ? Theme.warning : Theme.text
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: dialog.targetDetail(typeRow.modelData)
                                        color: typeRow.warn && !typeRow.skipped ? Theme.warning : Theme.textDim
                                        font.pixelSize: Theme.fontSmall
                                        wrapMode: Text.WordWrap
                                        maximumLineCount: 2
                                        elide: Text.ElideRight
                                    }
                                }
                                AppButton {
                                    text: qsTr("Change")
                                    iconGlyph: Theme.iconPen
                                    flat: true
                                    implicitHeight: 28
                                    onClicked: typeMenu.popup()
                                    AppMenu {
                                        id: typeMenu
                                        MenuItem { text: qsTr("Choose from the library…"); onTriggered: dialog.pick(typeRow.modelData) }
                                        MenuItem {
                                            text: qsTr("Generic dimmer")
                                            onTriggered: dialog.choose(typeRow.modelData.key, { generic: true })
                                        }
                                        MenuItem {
                                            text: qsTr("Don't import")
                                            onTriggered: dialog.choose(typeRow.modelData.key, { skip: true })
                                        }
                                        MenuSeparator {}
                                        MenuItem {
                                            text: qsTr("Find it in the library again")
                                            enabled: ["manual", "remembered", "skip"].indexOf(typeRow.modelData.status) >= 0
                                            onTriggered: dialog.choose(typeRow.modelData.key, { auto: true })
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // what can't be imported as it is
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.min(problemList.contentHeight, 118) + 16
                    visible: dialog.problems.length > 0
                    radius: Theme.radiusSmall
                    color: Qt.alpha(Theme.warning, 0.08)
                    border.color: Qt.alpha(Theme.warning, 0.5)
                    ListView {
                        id: problemList
                        anchors { fill: parent; margins: 8 }
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        model: dialog.problems
                        ScrollBar.vertical: ScrollBar { policy: problemList.contentHeight > problemList.height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded }
                        delegate: RowLayout {
                            required property string modelData
                            width: ListView.view.width - 10
                            spacing: 6
                            IconLabel { Layout.alignment: Qt.AlignTop; icon: Theme.iconWarning; size: 10; color: Theme.warning }
                            Text {
                                Layout.fillWidth: true
                                text: modelData
                                color: Theme.text
                                font.pixelSize: Theme.fontSmall
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }

            // options
            Panel {
                Layout.preferredWidth: 320
                Layout.fillHeight: true
                title: qsTr("Options")

                Flickable {
                    anchors { fill: parent; margins: 12 }
                    clip: true
                    contentHeight: optionsColumn.implicitHeight
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ScrollBar {}

                    ColumnLayout {
                        id: optionsColumn
                        width: parent.width
                        spacing: 6

                        OptionTitle { Layout.topMargin: 0; text: qsTr("Stage plan") }
                        OptionCheck {
                            visible: dialog.preview.hasPositions === true
                            text: qsTr("Place them on the stage plan")
                            checked: dialog.options.place === true
                            onToggled: dialog.setOption("place", checked)
                        }
                        Hint {
                            visible: dialog.preview.hasPositions !== true
                            text: qsTr("The table doesn't say where the fixtures are: the new ones go to free places of the plan.")
                        }
                        // the view: from above or from the front
                        Text {
                            visible: dialog.preview.hasPositions === true
                            text: qsTr("View")
                            color: Theme.textDim
                        }
                        Row {
                            visible: dialog.preview.hasPositions === true
                            enabled: dialog.options.place === true
                            opacity: enabled ? 1 : 0.5
                            spacing: 0
                            AppButton {
                                text: qsTr("From above")
                                iconGlyph: Theme.iconGrid
                                highlighted: dialog.options.view === "top"
                                onClicked: dialog.setOption("view", "top")
                            }
                            AppButton {
                                text: qsTr("From the front")
                                iconGlyph: Theme.iconEye
                                highlighted: dialog.options.view === "front"
                                onClicked: dialog.setOption("view", "front")
                            }
                        }
                        Hint {
                            visible: dialog.preview.hasPositions === true && dialog.options.place === true
                            text: (dialog.options.view === "front"
                                   ? qsTr("As the audience sees the rig: the highest fixtures at the top of the plan. Follow aims with the plan seen from above.")
                                   : qsTr("Seen from above, the audience at the bottom of the plan."))
                                  + (dialog.preview.suggestedView !== dialog.options.view
                                     ? " " + (dialog.preview.suggestedView === "front"
                                              ? qsTr("This rig hangs at different heights: it reads better from the front.")
                                              : qsTr("This rig is spread from front to back: it reads better from above."))
                                     : "")
                        }
                        Hint {
                            visible: dialog.preview.hasPositions === true && dialog.options.place === true
                            readonly property var stage: dialog.preview.stage || ({ width: 0, height: 0 })
                            readonly property var now: dialog.preview.stageNow || ({ width: 0, height: 0 })
                            readonly property bool grows: stage.width > now.width || stage.height > now.height
                            text: grows ? qsTr("The plan grows to %1 × %2 m to hold the rig.").arg(dialog.metres(stage.width)).arg(dialog.metres(stage.height))
                                        : qsTr("The rig fits in the plan (%1 × %2 m).").arg(dialog.metres(stage.width)).arg(dialog.metres(stage.height))
                            color: Theme.text
                        }

                        OptionTitle { text: qsTr("Groups") }
                        OptionCheck {
                            text: qsTr("A group for each fixture type")
                            checked: dialog.options.groupsByType === true
                            onToggled: dialog.setOption("groupsByType", checked)
                        }
                        OptionCheck {
                            visible: dialog.ok && dialog.preview.layers.length > 1
                            text: qsTr("A group for each layer (%1)").arg(dialog.ok ? dialog.preview.layers.join(", ") : "")
                            checked: dialog.options.groupsByLayer === true
                            onToggled: dialog.setOption("groupsByLayer", checked)
                        }
                        OptionCheck {
                            visible: dialog.preview.hasGroups === true
                            text: qsTr("The groups of the table")
                            checked: dialog.options.groupsFromTable === true
                            onToggled: dialog.setOption("groupsFromTable", checked)
                        }

                        OptionTitle { text: qsTr("Fixtures already in the show") }
                        Hint {
                            text: dialog.ok && dialog.preview.updateCount > 0
                                  ? qsTr("%n fixture(s) imported before, or with the same name and fixture, will be updated: mode, address, name, number and place.", "", dialog.preview.updateCount)
                                  : qsTr("None of them comes from this table: the fixtures of the table are added.")
                        }
                        OptionCheck {
                            visible: dialog.ok && dialog.preview.removeCount > 0
                            text: qsTr("Remove the %n fixture(s) imported before that are no longer in the table", "", dialog.ok ? dialog.preview.removeCount : 0)
                            checked: dialog.options.removeMissing === true
                            onToggled: dialog.setOption("removeMissing", checked)
                        }
                        // which ones they are (from another table, they may be all of the show)
                        Hint {
                            visible: dialog.ok && dialog.preview.removeCount > 0
                            text: dialog.ok ? (dialog.preview.removeNames || []).join(", ") : ""
                            color: dialog.removing ? Theme.danger : Theme.textDim
                            maximumLineCount: 4
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Text {
                Layout.fillWidth: true
                visible: dialog.ok
                wrapMode: Text.WordWrap
                text: dialog.ok ? qsTr("%n new", "", dialog.preview.newCount) + ", " + qsTr("%n updated", "", dialog.preview.updateCount)
                                  + (dialog.removing ? ", " + qsTr("%n removed", "", dialog.preview.removeCount) : "")
                                  + (dialog.preview.skippedRows > 0 ? " · " + qsTr("%n row(s) not imported", "", dialog.preview.skippedRows) : "")
                                : ""
                color: Theme.textDim
            }
            Item { Layout.fillWidth: true; visible: !dialog.ok }
            AppButton { text: qsTr("Cancel"); onClicked: dialog.close() }
            AppButton {
                text: qsTr("Import %n fixture(s)", "", dialog.importCount)
                iconGlyph: Theme.iconImport
                primary: true
                enabled: dialog.ok && (dialog.importCount > 0 || dialog.removing)
                onClicked: dialog.doImport()
            }
        }
    }

    // another fixture of the library for a type
    AddFixtureDialog {
        id: picker
        pickOnly: true
        onPicked: (maker, model, mode) => {
            if (dialog.picking === null)
                return
            dialog.choose(dialog.picking.key, model === "Generic Dimmer" ? { generic: true }
                                                                        : { manufacturer: maker, model: model, mode: mode })
            dialog.picking = null
        }
    }
}
