/*
  DMX Total Control
  ShowPage.qml

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
 * The SHOW screen: each page is made of
 * sections that tile the screen and split in halves. A section holds a table
 * of buttons or faders that fill its cells, a group, or a utility (clock,
 * timer, BPM, notes, running scenes, DMX levels). Saved with the show.
 */
Item {
    id: page

    /** Set by the main window: while locked, navigation and editing are hidden */
    property bool locked: false
    /** Edit lock of the SHOW: its pages are played, not edited */
    property bool editLocked: false
    onEditLockedChanged: if (editLocked) editing = false
    property bool fullScreen: false

    property bool editing: false
    property int currentPage: 0
    property string selectedSection: ""
    property int selectedCell: -1

    signal lockRequested(bool lock)
    signal fullScreenRequested(bool enable)
    /** Edit in Scenes (the menu of a playback) */
    signal openScene(int id)

    readonly property var sectionTypes: [
        { type: "buttons", name: qsTr("Buttons"), icon: Theme.iconGrid },
        { type: "faders", name: qsTr("Faders"), icon: Theme.iconSliders },
        { type: "group", name: qsTr("A group"), icon: Theme.iconLayers },
        { type: "groups", name: qsTr("All the groups"), icon: Theme.iconLayers },
        { type: "playbacks", name: qsTr("Playbacks"), icon: Theme.iconSliders },
        { type: "mixer", name: qsTr("Live mixer"), icon: Theme.iconSliders },
        { type: "clock", name: qsTr("Clock"), icon: Theme.iconCircle },
        { type: "timer", name: qsTr("Timer"), icon: Theme.iconPlay },
        { type: "bpm", name: qsTr("BPM and tap"), icon: Theme.iconBolt },
        { type: "notes", name: qsTr("Notes"), icon: Theme.iconPen },
        { type: "running", name: qsTr("Playing now"), icon: Theme.iconList },
        { type: "dials", name: qsTr("Dials and wheels"), icon: Theme.iconCircle },
        { type: "colour", name: qsTr("Colour wheel"), icon: Theme.iconPalette },
        { type: "xy", name: qsTr("XY grid (pan and tilt)"), icon: Theme.iconTarget },
        { type: "image", name: qsTr("Image"), icon: Theme.iconStar },
        { type: "monitor", name: qsTr("DMX levels"), icon: Theme.iconPlug },
        { type: "empty", name: qsTr("Empty"), icon: Theme.iconClose }
    ]

    function typeName(type) {
        let t = sectionTypes.find(s => s.type === type)
        return t ? t.name : type
    }

    readonly property var pages: Core.surface.pages
    readonly property var rootNode: pages.length > currentPage ? pages[currentPage].root : ({})

    // the selected section, found in the tree of the page
    function findNode(node, id) {
        if (!node || !id)
            return null
        if (node.id === id)
            return node
        return findNode(node.a, id) || findNode(node.b, id)
    }
    readonly property var selectedNode: findNode(rootNode, selectedSection) || ({})

    function selectSection(id) {
        if (selectedSection !== id)
            selectedCell = -1
        selectedSection = id
        keyCatcher.forceActiveFocus()
    }
    function selectCell(id, cell) {
        selectedSection = id
        selectedCell = cell
        keyCatcher.forceActiveFocus()
    }
    function split(id, orientation) {
        let added = Core.surface.splitSection(currentPage, id, orientation)
        if (added.length)
            selectSection(added)
    }
    function removeSection(id) {
        Core.surface.removeSection(currentPage, id)
        selectedSection = ""
        selectedCell = -1
    }
    function dropScene(id, cell, functionId) {
        let node = findNode(rootNode, id)
        let props = node && node.type === "faders" ? { target: "scene", functionId: functionId, targets: [{ target: "scene", functionId: functionId }] }
                                                   : { action: "scene", functionId: functionId }
        Core.surface.setItem(currentPage, id, cell, props)
        selectCell(id, cell)
    }
    function moveCell(id, from, to) {
        Core.surface.moveItem(currentPage, id, from, to)
        selectCell(id, to)
    }
    function stopScene(fid) {
        for (let i = 0; i < Core.zones.count; i++) {
            let zone = Core.zones.zone(i)
            if (zone.isActive(fid)) {
                zone.stop(fid)
                return
            }
        }
        if (Core.effects.isRunning(fid))
            Core.effects.togglePreview(fid)
    }

    // timers keep running when the page changes: their state lives here
    property var timers: ({})
    property int timerRevision: 0
    function timerState(id) { return timers[id] || { running: false, start: 0, acc: 0 } }
    function timerElapsed(id) {
        let t = timerState(id)
        return t.acc + (t.running ? Date.now() - t.start : 0)
    }
    function timerRunning(id) { return timerState(id).running }
    function timerToggle(id) {
        let t = timerState(id)
        let copy = Object.assign({}, timers)
        copy[id] = t.running ? { running: false, start: 0, acc: t.acc + Date.now() - t.start }
                             : { running: true, start: Date.now(), acc: t.acc }
        timers = copy
        timerRevision++
    }
    function timerReset(id) {
        let copy = Object.assign({}, timers)
        copy[id] = { running: false, start: 0, acc: 0 }
        timers = copy
        timerRevision++
    }

    // the keys of the buttons of the page
    function keyItem(node, text) {
        if (!node)
            return null
        if (node.split === "none" && node.type === "buttons" && node.items) {
            for (let k in node.items)
                if (node.items[k].key && node.items[k].key === text)
                    return node.items[k]
            return null
        }
        return keyItem(node.a, text) || keyItem(node.b, text)
    }
    // the buttons held from the keyboard, by physical key: the release lets
    // go of the button that was pressed, even when another page shows, the
    // edit mode started or Shift changed the text of the key meanwhile
    property var keyHeld: ({})
    function keyCode(event) { return event.nativeScanCode ? "s" + event.nativeScanCode : "k" + event.key }
    function releaseKey(code) {
        const item = keyHeld[code]
        if (item === undefined)
            return false
        delete keyHeld[code]
        keyButton.trigger(item, false)
        return true
    }
    // (a key held while the page goes away or stops playing: its release
    // may never come here)
    function releaseAllKeys() {
        for (let code of Object.keys(keyHeld))
            releaseKey(code)
    }

    onLockedChanged: if (locked) editing = false
    onEditingChanged: { selectedSection = ""; selectedCell = -1; if (editing) releaseAllKeys() }
    // the surface knows it too: Alt+click puts things there
    onCurrentPageChanged: { selectedSection = ""; selectedCell = -1; Core.surface.currentPage = currentPage }
    onPagesChanged: if (currentPage >= pages.length) currentPage = Math.max(0, pages.length - 1)
    onVisibleChanged: if (visible) keyCatcher.forceActiveFocus(); else releaseAllKeys()

    // buttons played from the keyboard
    Keys.onPressed: (event) => {
        // (auto-repeats of a held key)
        if (keyHeld[keyCode(event)] !== undefined) {
            event.accepted = true
            return
        }
        if (editing || !event.text)
            return
        let item = keyItem(rootNode, event.text.toLowerCase())
        if (item) {
            if (!event.isAutoRepeat) {
                keyHeld[keyCode(event)] = item
                keyButton.trigger(item, true)
            }
            event.accepted = true
        }
    }
    Keys.onReleased: (event) => {
        if (event.isAutoRepeat) {
            if (keyHeld[keyCode(event)] !== undefined)
                event.accepted = true
            return
        }
        if (releaseKey(keyCode(event)))
            event.accepted = true
    }
    // runs a keyboard button the way a click does
    SurfaceButton {
        id: keyButton
        visible: false
        function trigger(item, pressed) {
            keyButton.item = item
            if (pressed) keyButton.down(); else keyButton.up()
        }
    }

    Item {
        id: keyCatcher
        focus: true
        // (the keys go elsewhere now, or the window lost the keyboard)
        onActiveFocusChanged: if (!activeFocus) page.releaseAllKeys()
        Keys.onPressed: (event) => {
            if (!page.editing || page.selectedSection === "")
                return
            if (event.key === Qt.Key_Delete || event.key === Qt.Key_Backspace) {
                if (page.selectedCell >= 0)
                    Core.surface.clearItem(page.currentPage, page.selectedSection, page.selectedCell)
                event.accepted = true
            } else if (event.key === Qt.Key_Escape) {
                page.selectedSection = ""
                page.selectedCell = -1
                event.accepted = true
            }
        }
    }

    ColumnLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 10

        // header
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Column {
                Text {
                    text: Core.showTitle
                    color: Theme.text
                    font.pixelSize: Theme.fontTitle + 4
                    font.weight: Font.Bold
                }
                Text {
                    text: page.locked ? qsTr("Locked — hold the lock button to unlock")
                        : page.editing ? qsTr("Editing — split the sections in halves, choose what each one shows, drag scenes onto the buttons")
                        : qsTr("Your show screen")
                    color: page.locked || page.editing ? Theme.warning : Theme.textDim
                    font.pixelSize: Theme.fontSmall
                }
            }

            // pages
            Row {
                Layout.leftMargin: 12
                spacing: 4
                visible: Core.surface.pageCount > 1 || page.editing
                Repeater {
                    model: page.pages
                    delegate: AppButton {
                        required property var modelData
                        required property int index
                        text: modelData.name
                        highlighted: index === page.currentPage
                        implicitHeight: 34
                        onClicked: page.currentPage = index
                        onDoubleClicked: if (page.editing) pageMenu.rename(index)
                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.RightButton
                            enabled: page.editing
                            onClicked: pageMenu.openFor(parent.index)
                        }
                    }
                }
                AppButton {
                    visible: page.editing
                    iconGlyph: Theme.iconPlus
                    text: qsTr("Page")
                    flat: true
                    implicitHeight: 34
                    onClicked: page.currentPage = Core.surface.addPage("")
                }
            }

            Item { Layout.fillWidth: true }

            AppButton {
                text: page.editing ? qsTr("Done") : qsTr("Edit")
                iconGlyph: page.editing ? Theme.iconCheck : Theme.iconPen
                visible: !page.locked && !page.editLocked
                highlighted: page.editing
                accentColor: Theme.warning
                onClicked: { page.editing = !page.editing; keyCatcher.forceActiveFocus() }
            }

            AppButton {
                text: page.fullScreen ? qsTr("Exit full screen") : qsTr("Full screen")
                iconGlyph: page.fullScreen ? Theme.iconCompress : Theme.iconExpand
                visible: !page.locked
                onClicked: page.fullScreenRequested(!page.fullScreen)
                ToolTip.text: "F11"
                ToolTip.visible: hovered
            }

            // locking is a click, unlocking needs a long press
            AppButton {
                id: lockButton
                text: page.locked ? qsTr("Unlock") : qsTr("Lock")
                iconGlyph: page.locked ? Theme.iconUnlock : Theme.iconLock
                accentColor: Theme.warning
                primary: page.locked
                onClicked: if (!page.locked) page.lockRequested(true)
                onPressAndHold: if (page.locked) page.lockRequested(false)

                Rectangle {
                    visible: page.locked && lockButton.pressed
                    anchors { left: parent.left; bottom: parent.bottom }
                    height: 3
                    color: Theme.text
                    width: 0
                    NumberAnimation on width {
                        running: page.locked && lockButton.pressed
                        from: 0; to: lockButton.width; duration: 800
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            // the page: sections that tile the screen
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                SectionNode {
                    anchors.fill: parent
                    visible: Core.surface.pageCount > 0
                    nodeData: page.rootNode
                    surface: page
                }

                // no page yet
                Column {
                    anchors.centerIn: parent
                    visible: Core.surface.pageCount === 0
                    spacing: 12
                    width: Math.min(parent.width - 40, 480)

                    IconLabel { anchors.horizontalCenter: parent.horizontalCenter; icon: Theme.iconGrid; size: 34; color: Theme.textFaint }
                    Text {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        text: qsTr("Build the screen you want for the show: tables of buttons and faders, groups, a clock, a timer… Each section splits in halves.")
                        color: Theme.textDim
                    }
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 8
                        visible: !page.locked && !page.editLocked
                        AppButton {
                            text: qsTr("New page")
                            iconGlyph: Theme.iconPen
                            primary: true
                            onClicked: { page.currentPage = Core.surface.addPage(""); page.editing = true }
                        }
                        AppButton {
                            visible: Core.zones.count > 0
                            text: qsTr("Start from my groups")
                            iconGlyph: Theme.iconLayers
                            onClicked: page.currentPage = Core.surface.buildFromGroups()
                        }
                    }
                }
            }

            // edit mode: the selected section and cell
            Panel {
                Layout.preferredWidth: 300
                Layout.fillHeight: true
                visible: page.editing
                title: qsTr("Properties")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0
                    // (playbacks take a scene dropped on them too)
                    readonly property bool table: page.selectedNode.type === "buttons" || page.selectedNode.type === "faders" ||
                                                  page.selectedNode.type === "playbacks"

                    SectionProperties {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredHeight: 1000
                        surface: page
                        section: page.selectedNode
                        cell: page.selectedCell
                    }

                    // the scenes to drag onto the cells, always in sight
                    Rectangle { visible: parent.table; Layout.fillWidth: true; height: 1; color: Theme.border }
                    Text {
                        visible: parent.table
                        Layout.fillWidth: true
                        Layout.margins: 8
                        Layout.bottomMargin: 0
                        wrapMode: Text.WordWrap
                        text: page.selectedNode.type === "playbacks" ? qsTr("Drag a scene onto a playback:") : qsTr("Drag a scene onto a cell:")
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSmall
                    }
                    SceneBrowser {
                        visible: parent.table
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        // (a playbacks section has more properties: they keep most of the height)
                        Layout.preferredHeight: page.selectedNode.type === "playbacks" ? 300 : 1000
                        Layout.margins: 8
                        compact: true
                        dragTarget: dragProxy
                        onDragPressed: (area, ev, fn) => page.beginDrag(area, ev, "scene", "", -1, fn.id, fn.name)
                        onDragMoved: dragProxy.dragging = true
                        onDragReleased: dragProxy.drop()
                    }
                }
            }
        }
    }

    // what is dragged: a scene from the list, or a control to another cell
    Rectangle {
        id: dragProxy
        property string kind
        property string sectionId
        property int cell: -1
        property int functionId: -1
        property string label
        property bool dragging: false

        function drop() {
            if (dragging)
                Drag.drop()
            dragging = false
        }

        z: 100
        width: 150
        height: 40
        radius: Theme.radius
        visible: dragging
        color: Qt.alpha(Theme.accent, 0.35)
        border.color: Theme.accent
        Drag.active: dragging
        Drag.keys: kind === "scene" ? ["dtc-show-scene"] : ["dtc-show-cell"]
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2
        Text {
            anchors { fill: parent; margins: 6 }
            text: dragProxy.label
            color: Theme.text
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
    function beginDrag(mouseArea, ev, kind, sectionId, cell, functionId, label) {
        let p = mouseArea.mapToItem(page, ev.x, ev.y)
        dragProxy.x = p.x - dragProxy.width / 2
        dragProxy.y = p.y - dragProxy.height / 2
        dragProxy.kind = kind
        dragProxy.sectionId = sectionId
        dragProxy.cell = cell
        dragProxy.functionId = functionId
        dragProxy.label = label
    }
    // for the sections: page.dragItem
    property alias dragItem: dragProxy

    AppMenu {
        id: pageMenu
        property int index: -1
        function openFor(i) { index = i; popup() }
        function rename(i) {
            index = i
            prompt.ask(qsTr("Page name"), Core.surface.pages[i].name, (name) => Core.surface.renamePage(i, name))
        }
        MenuItem { text: qsTr("Rename…"); onTriggered: pageMenu.rename(pageMenu.index) }
        MenuItem {
            text: qsTr("Move left")
            enabled: pageMenu.index > 0
            onTriggered: { Core.surface.movePage(pageMenu.index, pageMenu.index - 1); page.currentPage = pageMenu.index - 1 }
        }
        MenuItem {
            text: qsTr("Move right")
            enabled: pageMenu.index < Core.surface.pageCount - 1
            onTriggered: { Core.surface.movePage(pageMenu.index, pageMenu.index + 1); page.currentPage = pageMenu.index + 1 }
        }
        MenuSeparator {}
        // (a page goes with everything on it, and there is no undo: asked first)
        MenuItem { text: qsTr("Delete page…"); onTriggered: deleteConfirm.openFor(pageMenu.index) }
    }

    TextPrompt { id: prompt }

    Popup {
        id: deleteConfirm
        property int index: -1
        // what goes with it: its sections, and the controls in them
        property int sections: 0
        property int controls: 0
        function countIn(node, total) {
            if (!node)
                return total
            if (node.split && node.split !== "none")
                return deleteConfirm.countIn(node.b, deleteConfirm.countIn(node.a, total))
            return { sections: total.sections + 1, controls: total.controls + Object.keys(node.items || {}).length }
        }
        function openFor(i) {
            const total = deleteConfirm.countIn(Core.surface.pages[i] ? Core.surface.pages[i].root : null,
                                                { sections: 0, controls: 0 })
            deleteConfirm.index = i
            deleteConfirm.sections = total.sections
            deleteConfirm.controls = total.controls
            deleteConfirm.open()
        }
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 440
        modal: true
        padding: 18
        onClosed: keyCatcher.forceActiveFocus()
        background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }
        contentItem: ColumnLayout {
            spacing: 12
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Delete the page “%1”?").arg(Core.surface.pages[deleteConfirm.index] ? Core.surface.pages[deleteConfirm.index].name : "")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
            }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Its sections (%1) and the buttons, faders and dials in them (%2) are deleted with it. This can't be undone.")
                      .arg(deleteConfirm.sections).arg(deleteConfirm.controls)
                color: Theme.textDim
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton { text: qsTr("Cancel"); onClicked: deleteConfirm.close() }
                AppButton {
                    text: qsTr("Delete")
                    primary: true
                    accentColor: Theme.danger
                    onClicked: { Core.surface.removePage(deleteConfirm.index); deleteConfirm.close() }
                }
            }
        }
    }
}
