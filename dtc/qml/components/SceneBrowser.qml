/*
  DMX Total Control
  SceneBrowser.qml

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
 * The scenes, in folders that fold (or by group), with a search: the same
 * list wherever scenes are picked (Scenes page, SHOW, super scenes, racks,
 * groups). Drag scenes onto a folder, or right click, to put them there.
 * With a dragTarget, rows are dragged out through the drag signals instead
 * (a folder still takes them when they are dropped on it).
 */
Item {
    id: browser

    /** The scenes to list (Core.functions entries) */
    property var functions: Core.functions
    /** The scene shown as current (outlined) */
    property int currentId: -1
    /** Short rows, without the type, for narrow panels */
    property bool compact: false
    /** Buttons to show one type only */
    property bool typeFilter: false
    /** Ctrl+click and Shift+click select several scenes */
    property bool multiSelect: false
    /** A click adds or removes the scene from the selection (pickers) */
    property bool toggleSelect: false
    /** A glyph at the end of each row (e.g. + to add) */
    property string rowIcon: ""
    /** Dragged out: the item that follows the mouse */
    property Item dragTarget: null
    property string emptyText: qsTr("No scenes")

    property var selectedIds: []
    /** "folders" or "groups" */
    property string mode: "folders"
    property string filter: ""

    signal sceneClicked(int id)
    signal sceneDoubleClicked(int id)
    signal dragPressed(var area, var ev, var fn)
    signal dragMoved(var area, var ev, var fn)
    signal dragReleased(var area, var ev, var fn)

    readonly property var filters: [
        { type: "", label: qsTr("All") },
        { type: "Scene", label: qsTr("Static") },
        { type: "Sequence", label: qsTr("Steps") },
        { type: "Cuelist", label: qsTr("Cuelists") },
        { type: "FX", label: qsTr("FX") },
        { type: "RGBMatrix", label: qsTr("Colour FX") },
        { type: "Collection", label: qsTr("Racks") },
        { type: "Show", label: qsTr("Super scenes") }
    ]

    // the rows of the list: section headers and scenes
    property var rows: []
    property string rowsJson: ""
    // folded sections, by mode and key
    property var closed: ({})

    function isSelected(id) { return selectedIds.indexOf(id) >= 0 }
    function clearSelection() { selectedIds = [] }
    function clearSearch() { search.text = "" }

    function toggleSection(key) {
        let c = Object.assign({}, closed)
        if (c[mode + key]) delete c[mode + key]; else c[mode + key] = true
        closed = c
    }

    // the scenes keep the order they were made in; the folders go by name
    function rebuild() {
        let q = search.text.trim().toLowerCase()
        // "FX": the FX and the effects of the old module
        const fxTypes = ["FXScene", "EFX", "CurveFX"]
        let list = (functions || []).filter(f => filter.length === 0 || f.type === filter ||
                                                  (filter === "FX" && fxTypes.indexOf(f.type) >= 0))
        let sections = []
        if (mode === "groups") {
            let byId = {}
            for (let f of list)
                byId[f.id] = f
            let grouped = {}
            for (let i = 0; i < Core.zones.count; i++) {
                let z = Core.zones.zone(i)
                let items = []
                for (let zf of z.functions) {
                    if (byId[zf.id]) {
                        items.push(byId[zf.id])
                        grouped[zf.id] = true
                    }
                }
                sections.push({ key: "g" + z.id, label: z.name, kind: "group", color: String(z.color), items: items })
            }
            let rest = list.filter(f => !grouped[f.id])
            sections.push(sections.length ? { key: "g-", label: qsTr("Not in a group"), kind: "none", items: rest }
                                          : { key: "", label: "", kind: "flat", items: rest })
        } else {
            let byFolder = {}
            for (let f of list) {
                let k = f.folder || ""
                if (!byFolder[k])
                    byFolder[k] = []
                byFolder[k].push(f)
            }
            let folders = Core.sceneFolders
            for (let name of folders)
                sections.push({ key: "f:" + name, label: name, kind: "folder", folder: name, items: (byFolder[name] || []) })
            let rest = byFolder[""] || []
            sections.push(folders.length ? { key: "f-", label: qsTr("No folder"), kind: "none", folder: "", items: rest }
                                         : { key: "", label: "", kind: "flat", folder: "", items: rest })
        }

        let out = []
        for (let s of sections) {
            // a folder whose name matches shows all its scenes
            let labelMatch = q.length > 0 && s.label.toLowerCase().indexOf(q) >= 0
            let items = q.length === 0 || labelMatch ? s.items : s.items.filter(f => f.name.toLowerCase().indexOf(q) >= 0)
            if (s.kind !== "flat") {
                if (q.length > 0 && items.length === 0)
                    continue
                let open = q.length > 0 || !closed[mode + s.key]
                out.push({ row: "header", key: s.key, label: s.label, kind: s.kind, folder: s.folder === undefined ? null : s.folder,
                           color: s.color || "", count: items.length, open: open })
                if (!open)
                    continue
            }
            for (let f of items)
                out.push({ row: "scene", key: s.key + "/" + f.id, id: f.id, name: f.name, type: f.type,
                           folder: f.folder || "", indent: s.kind !== "flat" })
        }
        // scenes that went away leave the selection
        let known = {}
        for (let f of (functions || []))
            known[f.id] = true
        if (selectedIds.some(id => !known[id]))
            selectedIds = selectedIds.filter(id => known[id])

        let json = JSON.stringify(out)
        if (json !== rowsJson) {
            rowsJson = json
            rows = out
        }
    }
    function scheduleRebuild() { Qt.callLater(rebuild) }
    // the current scene in sight (when it is chosen from another page)
    function showCurrent() {
        let i = rows.findIndex(r => r.row === "scene" && r.id === currentId)
        if (i >= 0)
            list.positionViewAtIndex(i, ListView.Contain)
    }
    onCurrentIdChanged: Qt.callLater(showCurrent)

    onFunctionsChanged: scheduleRebuild()
    onModeChanged: scheduleRebuild()
    onFilterChanged: scheduleRebuild()
    onClosedChanged: scheduleRebuild()
    Component.onCompleted: rebuild()
    Connections {
        target: Core
        function onFunctionsChanged() { browser.scheduleRebuild() }
    }
    // the groups: only what they hold, not whether their scenes play
    readonly property string groupSignature: {
        if (mode !== "groups")
            return ""
        let s = [String(Core.zones.count)]
        for (let i = 0; i < Core.zones.count; i++) {
            let z = Core.zones.zone(i)
            s.push(z.id + ":" + z.name + ":" + z.color + ":" + z.functions.map(f => f.id).join(","))
        }
        return s.join("|")
    }
    onGroupSignatureChanged: scheduleRebuild()

    // the scenes the selection or a row stands for
    function idsFor(id) {
        return multiSelect && isSelected(id) && selectedIds.length > 1 ? selectedIds.slice() : [id]
    }
    function sceneRows() { return rows.filter(r => r.row === "scene") }

    function clickScene(id, modifiers) {
        if (toggleSelect) {
            let s = selectedIds.slice()
            let i = s.indexOf(id)
            if (i >= 0) s.splice(i, 1); else s.push(id)
            selectedIds = s
        } else if (multiSelect && (modifiers & Qt.ControlModifier)) {
            let s = selectedIds.slice()
            let i = s.indexOf(id)
            if (i >= 0) s.splice(i, 1); else s.push(id)
            selectedIds = s
            return
        } else if (multiSelect && (modifiers & Qt.ShiftModifier) && selectedIds.length > 0) {
            let order = sceneRows().map(r => r.id)
            let a = order.indexOf(selectedIds[selectedIds.length - 1])
            let b = order.indexOf(id)
            if (a >= 0 && b >= 0) {
                let s = selectedIds.slice()
                for (let i = Math.min(a, b); i <= Math.max(a, b); i++)
                    if (s.indexOf(order[i]) < 0)
                        s.push(order[i])
                selectedIds = s
                return
            }
        } else {
            selectedIds = [id]
        }
        sceneClicked(id)
    }

    function openSceneMenu(id) {
        sceneMenu.ids = idsFor(id)
        sceneMenu.popup()
    }
    function openSelectionMenu() { openMenuFor(selectedIds.slice()) }
    /** The folder menu for these scenes */
    function openMenuFor(ids) {
        sceneMenu.ids = ids
        sceneMenu.popup()
    }
    function newFolderFor(ids) {
        prompt.ask(qsTr("New folder"), "", name => {
            if (ids.length)
                Core.setFunctionFolder(ids, name)
            else
                Core.addSceneFolder(name)
        })
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        RowLayout {
            Layout.fillWidth: true
            spacing: 4
            TextField {
                id: search
                Layout.fillWidth: true
                placeholderText: qsTr("Search…")
                color: Theme.text
                onTextChanged: browser.scheduleRebuild()
            }
            AppButton {
                iconGlyph: Theme.iconFolderPlus
                implicitHeight: 30
                flat: true
                ToolTip.text: qsTr("New folder")
                ToolTip.visible: hovered
                onClicked: browser.newFolderFor([])
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 4
            AppButton {
                text: qsTr("Folders")
                iconGlyph: Theme.iconFolder
                implicitHeight: 26
                flat: true
                font.pixelSize: Theme.fontSmall
                highlighted: browser.mode === "folders"
                onClicked: browser.mode = "folders"
            }
            AppButton {
                text: qsTr("Groups")
                iconGlyph: Theme.iconLayers
                implicitHeight: 26
                flat: true
                font.pixelSize: Theme.fontSmall
                highlighted: browser.mode === "groups"
                onClicked: browser.mode = "groups"
            }
            Item { Layout.fillWidth: true }
            AppButton {
                visible: browser.rows.some(r => r.row === "header")
                iconGlyph: browser.rows.some(r => r.row === "header" && r.open) ? Theme.iconCompress : Theme.iconExpand
                implicitHeight: 26
                flat: true
                ToolTip.text: browser.rows.some(r => r.row === "header" && r.open) ? qsTr("Fold all") : qsTr("Unfold all")
                ToolTip.visible: hovered
                onClicked: {
                    let anyOpen = browser.rows.some(r => r.row === "header" && r.open)
                    let c = Object.assign({}, browser.closed)
                    for (let r of browser.rows) {
                        if (r.row !== "header")
                            continue
                        if (anyOpen) c[browser.mode + r.key] = true
                        else delete c[browser.mode + r.key]
                    }
                    browser.closed = c
                }
            }
        }
        Flow {
            Layout.fillWidth: true
            visible: browser.typeFilter
            spacing: 4
            Repeater {
                model: browser.filters
                delegate: AppButton {
                    required property var modelData
                    text: modelData.label
                    implicitHeight: 26
                    flat: true
                    highlighted: browser.filter === modelData.type
                    onClicked: browser.filter = modelData.type
                }
            }
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 2
            model: browser.rows
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}

            delegate: Item {
                id: row
                required property var modelData
                readonly property bool header: modelData.row === "header"
                width: ListView.view.width
                height: header ? 30 : (browser.compact ? 28 : 40)

                // ------------------------------------------------ folder / group header
                Rectangle {
                    anchors.fill: parent
                    visible: row.header
                    radius: Theme.radiusSmall
                    color: headerDrop.containsDrag ? Qt.alpha(Theme.accent, 0.3)
                         : headerMouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised
                    border.color: headerDrop.containsDrag ? Theme.accent : "transparent"

                    RowLayout {
                        anchors { fill: parent; leftMargin: 6; rightMargin: 4 }
                        spacing: 6
                        IconLabel {
                            icon: row.modelData.open ? Theme.iconChevronDown : Theme.iconChevronRight
                            size: 9
                            color: Theme.textDim
                        }
                        IconLabel {
                            visible: row.modelData.kind !== "group"
                            icon: row.modelData.open ? Theme.iconFolderOpen : Theme.iconFolder
                            size: 12
                            color: row.modelData.kind === "folder" ? Theme.warning : Theme.textFaint
                        }
                        Rectangle {
                            visible: row.modelData.kind === "group"
                            width: 10
                            height: 10
                            radius: 5
                            color: row.modelData.color || Theme.textFaint
                        }
                        Text {
                            Layout.fillWidth: true
                            text: row.modelData.label || ""
                            color: row.modelData.kind === "none" ? Theme.textDim : Theme.text
                            font.weight: Font.DemiBold
                            font.pixelSize: Theme.fontSmall
                            elide: Text.ElideRight
                        }
                        Text {
                            text: row.modelData.count !== undefined ? row.modelData.count : ""
                            color: Theme.textFaint
                            font.pixelSize: Theme.fontSmall
                        }
                        AppButton {
                            visible: row.modelData.kind === "folder"
                            text: "⋯"
                            implicitHeight: 22
                            leftPadding: 6
                            rightPadding: 6
                            flat: true
                            onClicked: { folderMenu.folder = row.modelData.folder; folderMenu.popup() }
                        }
                    }
                    MouseArea {
                        id: headerMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        z: -1
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onClicked: (ev) => {
                            if (ev.button === Qt.RightButton) {
                                if (row.modelData.kind === "folder") {
                                    folderMenu.folder = row.modelData.folder
                                    folderMenu.popup()
                                }
                            } else {
                                browser.toggleSection(row.modelData.key)
                            }
                        }
                    }
                    // drop scenes on a folder to put them there
                    DropArea {
                        id: headerDrop
                        anchors.fill: parent
                        enabled: row.header && browser.mode === "folders" && row.modelData.folder !== null
                        keys: ["dtc-scene-folder", "dtc-show-scene", "dtc-timeline-scene"]
                        onDropped: (drop) => {
                            let src = drop.source
                            let ids = src && src.ids ? src.ids : (src && src.functionId >= 0 ? [src.functionId] : [])
                            if (ids.length)
                                Core.setFunctionFolder(ids, row.modelData.folder)
                            drop.accept()
                        }
                    }
                }

                // ------------------------------------------------ scene
                Rectangle {
                    id: sceneRow
                    anchors { fill: parent; leftMargin: row.modelData.indent ? 12 : 0 }
                    visible: !row.header
                    readonly property bool selected: !row.header && browser.isSelected(row.modelData.id)
                    readonly property bool current: !row.header && row.modelData.id === browser.currentId
                    radius: Theme.radiusSmall
                    color: selected ? Qt.alpha(Theme.accent, 0.22) : (sceneMouse.containsMouse ? Theme.surfaceHover : "transparent")
                    border.color: current ? Theme.accent : "transparent"

                    RowLayout {
                        anchors { fill: parent; leftMargin: 6; rightMargin: 8 }
                        spacing: 8
                        Rectangle {
                            width: 4
                            height: browser.compact ? 18 : 26
                            radius: 2
                            color: Theme.functionColor(row.modelData.type || "")
                        }
                        Column {
                            Layout.fillWidth: true
                            Text {
                                width: parent.width
                                text: row.modelData.name || ""
                                color: Theme.text
                                elide: Text.ElideRight
                                font.pixelSize: browser.compact ? Theme.fontSmall : Theme.fontNormal
                                font.weight: browser.compact ? Font.Normal : Font.DemiBold
                            }
                            Text {
                                visible: !browser.compact
                                text: Theme.typeLabel(row.modelData.type || "")
                                color: Theme.textFaint
                                font.pixelSize: Theme.fontSmall
                            }
                        }
                        IconLabel {
                            visible: browser.toggleSelect && sceneRow.selected
                            icon: Theme.iconCheck
                            color: Theme.accent
                        }
                        IconLabel {
                            visible: browser.rowIcon.length > 0
                            icon: browser.rowIcon
                            color: Theme.textDim
                        }
                    }

                    MouseArea {
                        id: sceneMouse
                        anchors.fill: parent
                        enabled: !row.header
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        cursorShape: browser.dragTarget ? Qt.OpenHandCursor : Qt.PointingHandCursor
                        drag.target: browser.dragTarget ? browser.dragTarget : folderDrag
                        drag.threshold: 6
                        preventStealing: true
                        readonly property var fn: ({ id: row.modelData.id, name: row.modelData.name, type: row.modelData.type })
                        onPressed: (ev) => {
                            if (ev.button === Qt.RightButton) {
                                browser.openSceneMenu(row.modelData.id)
                                return
                            }
                            if (browser.dragTarget) {
                                browser.dragPressed(sceneMouse, ev, fn)
                            } else {
                                let p = mapToItem(browser, ev.x, ev.y)
                                folderDrag.ids = browser.idsFor(row.modelData.id)
                                folderDrag.label = folderDrag.ids.length > 1 ? qsTr("%n scene(s)", "", folderDrag.ids.length) : row.modelData.name
                                folderDrag.x = p.x - 12
                                folderDrag.y = p.y - 14
                            }
                        }
                        onPositionChanged: (ev) => {
                            if (!drag.active)
                                return
                            if (browser.dragTarget)
                                browser.dragMoved(sceneMouse, ev, fn)
                            else
                                folderDrag.dragging = true
                        }
                        onReleased: (ev) => {
                            if (browser.dragTarget) {
                                browser.dragReleased(sceneMouse, ev, fn)
                            } else if (folderDrag.dragging) {
                                folderDrag.Drag.drop()
                                folderDrag.dragging = false
                            }
                        }
                        onClicked: (ev) => { if (ev.button === Qt.LeftButton) browser.clickScene(row.modelData.id, ev.modifiers) }
                        onDoubleClicked: (ev) => { if (ev.button === Qt.LeftButton) browser.sceneDoubleClicked(row.modelData.id) }
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                width: parent.width - 30
                visible: list.count === 0
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: search.text.length > 0 ? qsTr("No scene matches “%1”").arg(search.text) : browser.emptyText
                color: Theme.textFaint
            }

            // dragging near the edges scrolls
            Timer {
                interval: 40
                repeat: true
                running: folderDrag.dragging || (browser.dragTarget !== null && browser.dragTarget.Drag.active)
                onTriggered: {
                    let item = folderDrag.dragging ? folderDrag : browser.dragTarget
                    let p = item.mapToItem(list, 0, item.height / 2)
                    if (p.x < -40 || p.x > list.width + 40)
                        return
                    if (p.y < 24)
                        list.contentY = Math.max(list.originY, list.contentY - 14)
                    else if (p.y > list.height - 24)
                        list.contentY = Math.min(list.originY + list.contentHeight - list.height, list.contentY + 14)
                }
            }
        }

        // several selected: what to do with them
        RowLayout {
            Layout.fillWidth: true
            visible: browser.multiSelect && browser.selectedIds.length > 1
            spacing: 4
            Text {
                Layout.fillWidth: true
                text: qsTr("%n selected", "", browser.selectedIds.length)
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
                elide: Text.ElideRight
            }
            AppButton {
                text: qsTr("To folder…")
                iconGlyph: Theme.iconFolder
                implicitHeight: 26
                onClicked: browser.openSelectionMenu()
            }
            AppButton {
                iconGlyph: Theme.iconClose
                implicitHeight: 26
                flat: true
                ToolTip.text: qsTr("Clear the selection")
                ToolTip.visible: hovered
                onClicked: browser.clearSelection()
            }
        }
    }

    // what is dragged to a folder
    Rectangle {
        id: folderDrag
        property var ids: []
        property string label: ""
        property bool dragging: false
        z: 100
        width: Math.min(180, folderLabel.implicitWidth + 20)
        height: 26
        radius: Theme.radiusSmall
        visible: dragging
        color: Qt.alpha(Theme.accent, 0.4)
        border.color: Theme.accent
        Drag.active: dragging
        Drag.keys: ["dtc-scene-folder"]
        Drag.hotSpot: Qt.point(12, 13)
        Text {
            id: folderLabel
            anchors { fill: parent; leftMargin: 10; rightMargin: 10 }
            text: folderDrag.label
            color: Theme.text
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: Theme.fontSmall
        }
    }

    AppMenu {
        id: sceneMenu
        property var ids: []
        MenuItem {
            text: qsTr("%n scene(s)", "", sceneMenu.ids.length)
            enabled: false
            visible: sceneMenu.ids.length > 1
            height: visible ? implicitHeight : 0
        }
        AppMenu {
            id: moveMenu
            title: qsTr("Move to folder")
            Instantiator {
                model: Core.sceneFolders
                delegate: MenuItem {
                    required property string modelData
                    text: modelData
                    checkable: true
                    checked: {
                        if (sceneMenu.ids.length !== 1)
                            return false
                        let f = (browser.functions || []).find(x => x.id === sceneMenu.ids[0])
                        return !!f && f.folder === modelData
                    }
                    onTriggered: Core.setFunctionFolder(sceneMenu.ids, modelData)
                }
                onObjectAdded: (index, object) => moveMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => moveMenu.removeItem(object)
            }
            MenuSeparator {}
            MenuItem {
                text: qsTr("New folder…")
                onTriggered: browser.newFolderFor(sceneMenu.ids)
            }
        }
        MenuItem {
            text: qsTr("Out of its folder")
            onTriggered: Core.setFunctionFolder(sceneMenu.ids, "")
        }
    }

    AppMenu {
        id: folderMenu
        property string folder: ""
        function rename(name) {
            prompt.ask(qsTr("Rename folder"), name, newName => Core.renameSceneFolder(name, newName))
        }
        MenuItem {
            text: qsTr("Rename folder")
            onTriggered: folderMenu.rename(folderMenu.folder)
        }
        MenuItem {
            text: qsTr("Delete folder (its scenes stay)")
            onTriggered: Core.removeSceneFolder(folderMenu.folder)
        }
    }

    TextPrompt { id: prompt }
}
