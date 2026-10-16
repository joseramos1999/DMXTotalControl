/*
  DMX Total Control
  ScenesPage.qml

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
 * Scenes: the library of every scene and effect of the show, and the
 * editor of the selected one, by type:
 * static, steps, Move FX, Colour FX and effect racks.
 * New effects take the fixtures and values of the programmer.
 */
Item {
    id: page

    property int currentId: -1
    // a super scene and a cuelist use the whole width: no list of scenes
    readonly property bool wide: currentId >= 0 && (info.type === "Show" || info.type === "Cuelist")
    property var info: ({})

    signal editInProgrammer(int id)
    /** Scenes › New › FX: the FX of the programmer */
    signal openFx()
    signal message(string text, bool isError)

    function typeLabel(type) { return Theme.typeLabel(type) }

    function select(id) {
        currentId = id
        if (id >= 0 && browser.selectedIds.indexOf(id) < 0)
            browser.selectedIds = [id]
        refresh()
        // a cuelist opened here is the one Record, Update and the command line
        // work on. Only when it is opened: the page refreshes on every change
        // of the functions, and the cuelist chosen since (PB 2, a new one
        // recorded in Program) must stay.
        if (info.type === "Cuelist")
            Core.cuelists.selectedId = currentId
    }

    readonly property string currentFolder: {
        let f = Core.functions.find(x => x.id === currentId)
        return f ? (f.folder || "") : ""
    }

    function refresh() {
        info = currentId >= 0 ? Core.effects.info(currentId) : ({})
        if (info.id === undefined)
            currentId = -1
    }

    /** Developer option --cue-select: these cues (indexes) of the open cuelist */
    function selectCues(indexes) {
        if (info.type === "Cuelist" && editorLoader.item)
            editorLoader.item.selected = indexes
    }

    function created(id) {
        if (id >= 0)
            select(id)
    }

    Connections {
        target: Core.effects
        function onEdited(id) { if (id === page.currentId) page.refresh() }
        // a problem, or what was done (saved to a file...)
        function onMessage(text, error) { page.message(text, error) }
    }
    // the programmer saved in the scene shown (here or from Program): its
    // channels and fixtures (a moment later: the editor that saved may go,
    // a static scene with FX becomes an FX scene)
    Connections {
        target: Core.programmer
        function onSceneUpdated(id) { if (id === page.currentId) Qt.callLater(page.refresh) }
    }
    Connections {
        target: Core.cuelists
        function onEdited(id) { if (id === page.currentId) page.refresh() }
        // (its messages: the main window shows them, from every page)
    }
    Connections {
        target: Core
        function onFunctionsChanged() { page.refresh() }
    }
    // running state, for the try button
    Timer {
        interval: 400
        repeat: true
        running: page.visible && page.currentId >= 0
        onTriggered: tryButton.running = Core.effects.isRunning(page.currentId)
    }

    RowLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 12

        // library
        Panel {
            Layout.preferredWidth: 320
            Layout.fillHeight: true
            // a super scene needs the whole width
            visible: !page.wide
            title: qsTr("Scenes (%1)").arg(Core.functions.length)
            headerItems: [
                AppButton {
                    text: qsTr("New")
                    iconGlyph: Theme.iconPlus
                    primary: true
                    implicitHeight: 28
                    onClicked: newMenu.popup()
                }
            ]

            SceneBrowser {
                id: browser
                anchors { fill: parent; margins: 8 }
                typeFilter: true
                multiSelect: true
                currentId: page.currentId
                emptyText: qsTr("No scenes. Select fixtures in Program and create one with “New”.")
                onSceneClicked: (id) => page.select(id)
            }
        }

        // editor
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12
            visible: page.currentId >= 0

            // header
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                AppButton {
                    visible: page.wide
                    iconGlyph: Theme.iconList
                    implicitHeight: 34
                    ToolTip.text: qsTr("Back to the list of scenes")
                    ToolTip.visible: hovered
                    onClicked: page.select(-1)
                }
                Rectangle { width: 6; height: 34; radius: 3; color: Theme.functionColor(page.info.type || "") }
                TextField {
                    id: nameField
                    Layout.fillWidth: true
                    Layout.minimumWidth: 120
                    text: page.info.name || ""
                    // a long name shows its start
                    onTextChanged: if (!activeFocus) cursorPosition = 0
                    font.pixelSize: Theme.fontTitle
                    font.weight: Font.Bold
                    color: Theme.text
                    background: Rectangle { color: nameField.activeFocus ? Theme.background : "transparent"; radius: Theme.radiusSmall; border.color: nameField.activeFocus ? Theme.accent : "transparent" }
                    onEditingFinished: { if (text !== page.info.name) Core.renameFunction(page.currentId, text); focus = false }
                }
                Text { text: page.typeLabel(page.info.type || ""); color: Theme.textDim }
                AppButton {
                    visible: !page.wide
                    flat: true
                    implicitHeight: 30
                    iconGlyph: Theme.iconFolder
                    // narrow: the icon only, the folder in the tooltip
                    text: page.width < 1300 ? "" : page.currentFolder.length ? page.currentFolder : qsTr("No folder")
                    ToolTip.text: page.width < 1300 ? qsTr("Folder of the scene") + ": " + (page.currentFolder.length ? page.currentFolder : qsTr("No folder"))
                                                    : qsTr("Folder of the scene")
                    ToolTip.visible: hovered
                    onClicked: browser.openMenuFor([page.currentId])
                }

                AppButton {
                    id: tryButton
                    // a cuelist has its own GO, Back and Release in its editor
                    visible: page.info.type !== "Cuelist"
                    property bool running: false
                    text: running ? qsTr("Stop") : qsTr("Try")
                    iconGlyph: running ? Theme.iconStop : Theme.iconPlay
                    highlighted: running
                    accentColor: Theme.success
                    ToolTip.text: qsTr("Play it now, outside of the groups")
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                    onClicked: { Core.effects.togglePreview(page.currentId); running = !running }
                }
                ComboBox {
                    id: groupCombo
                    Layout.preferredWidth: 160
                    model: {
                        let names = [qsTr("Add to group…")]
                        for (let i = 0; i < Core.zones.count; i++)
                            names.push(Core.zones.zone(i).name)
                        return names
                    }
                    onActivated: (i) => {
                        if (i > 0) {
                            Core.zones.zone(i - 1).addFunction(page.currentId)
                            page.message(qsTr("Added to %1").arg(Core.zones.zone(i - 1).name), false)
                        }
                        currentIndex = 0
                    }
                }
                AppButton {
                    iconGlyph: Theme.iconLayers
                    flat: true
                    visible: ["Collection", "EFX", "RGBMatrix", "CurveFX", "FXScene", "Chaser"].indexOf(page.info.type) >= 0
                    ToolTip.text: qsTr("Convert to steps: records one cycle into a new Steps scene")
                    ToolTip.visible: hovered
                    onClicked: page.created(Core.effects.convertToSteps(page.currentId, 100))
                }
                AppButton {
                    iconGlyph: Theme.iconList
                    flat: true
                    visible: page.info.type === "Sequence"
                    ToolTip.text: qsTr("Convert to a cuelist: each step becomes a cue that holds what changes")
                    ToolTip.visible: hovered
                    onClicked: convertConfirm.open()
                }
                AppButton {
                    iconGlyph: Theme.iconDownload
                    flat: true
                    ToolTip.text: qsTr("Export to a file, to load it in other shows")
                    ToolTip.visible: hovered
                    onClicked: exportDialog.open()
                }
                AppButton {
                    iconGlyph: Theme.iconCopy
                    flat: true
                    ToolTip.text: qsTr("Duplicate")
                    ToolTip.visible: hovered
                    onClicked: page.select(Core.effects.duplicate(page.currentId))
                }
                AppButton {
                    iconGlyph: Theme.iconTrash
                    flat: true
                    ToolTip.text: qsTr("Delete")
                    ToolTip.visible: hovered
                    onClicked: deleteConfirm.open()
                }
            }

            // how it plays: fades, driving, loops
            PlaybackPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: implicitHeight
                visible: page.info.type !== "Show"
                info: page.info
            }

            // effects of the old module: they play as they did, and can become FX
            Rectangle {
                Layout.fillWidth: true
                visible: page.info.type === "EFX" || page.info.type === "CurveFX"
                implicitHeight: bannerRow.implicitHeight + 16
                radius: Theme.radius
                color: Qt.alpha(Theme.warning, 0.12)
                border.color: Qt.alpha(Theme.warning, 0.5)
                RowLayout {
                    id: bannerRow
                    anchors { fill: parent; margins: 8 }
                    spacing: 10
                    IconLabel { icon: Theme.iconWand; color: Theme.warning }
                    Text {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        text: qsTr("This effect is from the old effects module. It plays as before; converted to FX it gets the FX editor (in the programmer too). Groups, SHOW buttons and mappings keep it.")
                        color: Theme.text
                        font.pixelSize: Theme.fontSmall
                    }
                    AppButton {
                        text: qsTr("Convert to FX")
                        iconGlyph: Theme.iconWand
                        primary: true
                        implicitHeight: 30
                        onClicked: {
                            if (Core.effects.convertToFx(page.currentId))
                                page.refresh()
                        }
                    }
                }
            }

            Loader {
                id: editorLoader
                Layout.fillWidth: true
                Layout.fillHeight: true
                sourceComponent: {
                    switch (page.info.type) {
                    case "Scene": return staticEditor
                    case "Sequence": return stepsEditor
                    case "EFX": return moveEditor
                    case "RGBMatrix": return colourEditor
                    case "CurveFX": return curveEditor
                    case "FXScene": return fxEditor
                    case "Collection": return rackEditor
                    case "Show": return timelineEditor
                    case "Cuelist": return cuelistEditor
                    default: return otherEditor
                    }
                }
            }
        }

        // nothing selected
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: page.currentId < 0
            Column {
                anchors.centerIn: parent
                spacing: 10
                IconLabel { anchors.horizontalCenter: parent.horizontalCenter; icon: Theme.iconWand; size: 34; color: Theme.textFaint }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Choose a scene, or create a new one")
                    color: Theme.textDim
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 420
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: qsTr("New effects use the fixtures selected in Program, and new steps the values of the programmer.")
                    color: Theme.textFaint
                    font.pixelSize: Theme.fontSmall
                }
            }
        }
    }

    Component { id: staticEditor; StaticEditor { info: page.info; onEditInProgrammer: (id) => page.editInProgrammer(id)
                                                  onSaved: page.message(qsTr("The programmer was saved in “%1”").arg(page.info.name || ""), false) } }
    Component { id: stepsEditor; StepsEditor { info: page.info } }
    Component { id: moveEditor; MoveFxEditor { info: page.info } }
    Component { id: colourEditor; ColourFxEditor { info: page.info } }
    Component { id: curveEditor; CurveFxEditor { info: page.info } }
    Component { id: fxEditor; FxSceneEditor { info: page.info; onEditInProgrammer: (id) => page.editInProgrammer(id)
                                              onSaved: page.message(qsTr("The programmer was saved in “%1”").arg(page.info.name || ""), false) } }
    Component { id: rackEditor; RackEditor { info: page.info } }
    Component { id: timelineEditor; SuperSceneEditor { info: page.info; onOpenFunction: (id) => page.select(id) } }
    Component { id: cuelistEditor; CuelistView { info: page.info; onMessage: (text) => page.message(text, true) } }
    Component {
        id: otherEditor
        Item {
            Text {
                anchors.centerIn: parent
                text: qsTr("This type is edited in its own page in a later version")
                color: Theme.textDim
            }
        }
    }

    AppMenu {
        id: newMenu
        MenuItem {
            text: qsTr("Static (from the programmer)")
            onTriggered: {
                // (at the default priority of new scenes, like the record dialog)
                let id = Core.programmer.recordScene("", -1, Core.settings.defaultPriority)
                if (id < 0)
                    page.message(qsTr("Set some values in the programmer first"), true)
                page.created(id)
            }
        }
        MenuItem { text: qsTr("Steps (programmer = first step)"); onTriggered: page.created(Core.effects.createSteps("")) }
        MenuItem { text: qsTr("Cuelist (programmer = cue 1)"); onTriggered: page.created(Core.cuelists.create("", -1, Core.settings.defaultPriority)) }
        MenuItem { text: qsTr("FX (in the programmer)"); onTriggered: page.openFx() }
        MenuItem { text: qsTr("Colour FX (selected fixtures)"); onTriggered: page.created(Core.effects.createColourFx("")) }
        MenuItem { text: qsTr("Mapping on the 2D plan (selected fixtures)"); onTriggered: page.created(Core.effects.createMapping("")) }
        MenuItem { text: qsTr("Effect rack"); onTriggered: page.created(Core.effects.createRack("", [])) }
        MenuItem { text: qsTr("Super scene (timeline with audio)"); onTriggered: page.created(Core.effects.createSuperScene("")) }
        MenuSeparator {}
        MenuItem { text: qsTr("Import from a file…"); onTriggered: importDialog.open() }
    }

    FileDialog {
        id: exportDialog
        title: qsTr("Export the scene")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "dtcfx"
        nameFilters: [qsTr("DMX Total Control effects (*.dtcfx)")]
        onAccepted: Core.effects.exportEffect(page.currentId, selectedFile)
    }
    FileDialog {
        id: importDialog
        title: qsTr("Import scenes and effects")
        nameFilters: [qsTr("DMX Total Control effects (*.dtcfx)")]
        onAccepted: page.created(Core.effects.importEffect(selectedFile))
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
            Text { text: qsTr("Delete “%1”?").arg(page.info.name || ""); color: Theme.text; font.pixelSize: Theme.fontLarge; font.weight: Font.Bold }
            Text { text: qsTr("It is also removed from its groups and racks."); color: Theme.textDim }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton { text: qsTr("Cancel"); onClicked: deleteConfirm.close() }
                AppButton {
                    text: qsTr("Delete")
                    primary: true
                    accentColor: Theme.danger
                    onClicked: { Core.deleteFunction(page.currentId); page.currentId = -1; page.info = {}; deleteConfirm.close() }
                }
            }
        }
    }

    Popup {
        id: convertConfirm
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 460
        modal: true
        padding: 18
        background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }
        contentItem: ColumnLayout {
            spacing: 12
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Convert “%1” to a cuelist?").arg(page.info.name || "")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
            }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Each step becomes a cue with what it changes from the step before; timed steps follow each other with Wait. Its groups, SHOW buttons and mappings play the cuelist. The steps are gone.")
                color: Theme.textDim
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton { text: qsTr("Cancel"); onClicked: convertConfirm.close() }
                AppButton {
                    text: qsTr("Convert")
                    primary: true
                    onClicked: {
                        convertConfirm.close()
                        if (Core.cuelists.convertFromSteps(page.currentId) >= 0)
                            page.select(page.currentId)
                    }
                }
            }
        }
    }
}
