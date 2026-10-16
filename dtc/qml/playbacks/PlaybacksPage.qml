/*
  DMX Total Control
  PlaybacksPage.qml

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
 * The playbacks: pages of faders, each one with a cuelist or
 * any scene, its GO, Pause/Back and Flash. Record + a click on a playback
 * records the programmer there (an empty one gets a new cuelist). Main GO
 * plays the next cue of the selected playback (Space; Shift+Space goes
 * back, PgUp/PgDown change the page). The selected cuelist shows its cues
 * at the bottom.
 */
Item {
    id: view

    /** Edit in Scenes */
    signal openScene(int id)

    /** Record: the next click on a playback records the programmer there */
    property bool recordArmed: false
    /** The list of scenes, to drag them onto the playbacks */
    property bool assignOpen: false

    /** Lock of the programming screens: the playbacks play, nothing changes */
    readonly property bool programLocked: Core.settings.lockedScreens.indexOf("program") >= 0
    readonly property bool hasProgrammer: Core.programmer.hasValues || Core.programmer.fxLayers.length > 0

    readonly property var selectedPlayback: Core.playbacks.selected
    readonly property bool hasSelection: selectedPlayback.functionId !== undefined
    readonly property var selectedInfo: {
        Core.playbacks.revision
        view.functionsRevision
        return hasSelection ? Core.playbacks.slotInfo(selectedPlayback.page, selectedPlayback.slot) : ({})
    }
    /** The cue Update changes: the one loaded in the programmer, else the one it plays */
    readonly property string updateNumber: {
        if (!selectedInfo.isCuelist)
            return ""
        const e = Core.cuelists.editing
        if (e.id !== undefined && e.id === selectedInfo.functionId)
            return e.number || ""
        return selectedInfo.running ? (selectedInfo.cueNumber || "") : ""
    }

    property int functionsRevision: 0
    Connections {
        target: Core
        function onFunctionsChanged() { view.functionsRevision++ }
    }

    /** Developer option --pb-dialog kind:slot (menu, add, assign, properties, pages): open it, for screenshots */
    function devOpen(what) {
        devTimer.what = what
        devTimer.restart()
    }
    Timer {
        id: devTimer
        property string what: ""
        interval: 600
        onTriggered: {
            const parts = what.split(":")
            const strip = stripsRepeater.itemAt(parts.length > 1 ? parseInt(parts[1]) : 0)
            switch (parts[0]) {
            case "menu": if (strip) strip.openMenu(true); break
            case "add": if (strip) strip.openAddMenu(); break
            case "assign": if (strip) strip.openAssign(); break
            case "properties": if (strip) strip.openProperties(); break
            case "pages": pageMenu.index = Core.playbacks.currentPage; pageMenu.popup(tabsView, 0, tabsView.height); break
            case "update": updateMenu.popup(updateButton, 0, updateButton.height); break
            }
        }
    }

    onProgramLockedChanged: if (programLocked) { recordArmed = false; assignOpen = false }
    onHasProgrammerChanged: if (!hasProgrammer) recordArmed = false
    // Shown, the page takes the keyboard (Space is Main GO, PgUp/PgDown the
    // pages): a hidden page keeps its focus otherwise, and a text field there
    // would take Space. Record is for this page: leaving it disarms it
    onVisibleChanged: {
        if (visible)
            view.forceActiveFocus()
        else
            recordArmed = false
    }
    // the search field of the list of scenes closes with it
    onAssignOpenChanged: if (!assignOpen && visible) view.forceActiveFocus()

    // Record armed: it blinks
    property bool blinkOn: true
    Timer {
        interval: 400
        repeat: true
        running: view.recordArmed
        onTriggered: view.blinkOn = !view.blinkOn
        onRunningChanged: view.blinkOn = true
    }

    ColumnLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 10

        /* ------------------------------------------------------------ pages */
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            AppButton {
                iconGlyph: Theme.iconStepBackward
                implicitHeight: 34
                enabled: Core.playbacks.currentPage > 0
                onClicked: Core.playbacks.previousPage()
                ToolTip.text: qsTr("Previous page (PgUp)")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                MapTarget { target: ({ target: "playbackPagePrevious" }) }
            }

            // the pages: a click shows it; double click or right click to rename, move or delete
            Flickable {
                id: tabsView
                Layout.fillWidth: true
                Layout.maximumWidth: tabsRow.implicitWidth
                Layout.preferredHeight: 34
                contentWidth: tabsRow.implicitWidth
                clip: true
                flickableDirection: Flickable.HorizontalFlick
                boundsBehavior: Flickable.StopAtBounds
                interactive: contentWidth > width

                Row {
                    id: tabsRow
                    spacing: 4
                    Repeater {
                        model: Core.playbacks.pageNames
                        delegate: AppButton {
                            required property string modelData
                            required property int index
                            text: modelData
                            implicitHeight: 34
                            highlighted: index === Core.playbacks.currentPage
                            onClicked: Core.playbacks.currentPage = index
                            onDoubleClicked: if (!view.programLocked) pageMenu.rename(index)
                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.RightButton
                                onClicked: pageMenu.openFor(parent.index)
                            }
                            MapTarget { target: ({ target: "playbackPage", pbPage: parent.index }) }
                        }
                    }
                }
            }
            AppButton {
                iconGlyph: Theme.iconStepForward
                implicitHeight: 34
                enabled: Core.playbacks.currentPage < Core.playbacks.pageCount - 1
                onClicked: Core.playbacks.nextPage()
                ToolTip.text: qsTr("Next page (PgDown)")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                MapTarget { target: ({ target: "playbackPageNext" }) }
            }
            AppButton {
                iconGlyph: Theme.iconPlus
                text: qsTr("Page")
                flat: true
                implicitHeight: 34
                enabled: !view.programLocked
                onClicked: Core.playbacks.currentPage = Core.playbacks.addPage("")
            }

            Item { Layout.fillWidth: true }

            Text {
                text: qsTr("Faders per page")
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
            }
            ComboBox {
                id: slotsCombo
                // Space stays Main GO
                focusPolicy: Qt.NoFocus
                Layout.preferredWidth: 76
                implicitHeight: 34
                // (part of the show: an edit, as the rest of the page)
                enabled: !view.programLocked
                readonly property var counts: [8, 10, 12, 16, 20]
                model: counts
                currentIndex: counts.indexOf(Core.playbacks.slotsPerPage)
                displayText: String(Core.playbacks.slotsPerPage)
                onActivated: (i) => Core.playbacks.slotsPerPage = counts[i]
            }
            AppButton {
                text: qsTr("Assign")
                iconGlyph: Theme.iconList
                implicitHeight: 34
                highlighted: view.assignOpen
                enabled: !view.programLocked
                onClicked: view.assignOpen = !view.assignOpen
                ToolTip.text: qsTr("The scenes, to drag them onto the playbacks")
                ToolTip.visible: hovered
                ToolTip.delay: 500
            }
        }

        /* ------------------------------------------- record, update, main GO */
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            AppButton {
                id: recordButton
                text: qsTr("Record")
                iconGlyph: Theme.iconRecord
                implicitHeight: 52
                leftPadding: 16
                rightPadding: 16
                accentColor: Theme.danger
                highlighted: view.recordArmed && view.blinkOn
                enabled: view.recordArmed || (view.hasProgrammer && !view.programLocked)
                onClicked: view.recordArmed = !view.recordArmed
                ToolTip.text: view.programLocked ? qsTr("The programming screens are locked")
                            : !view.hasProgrammer ? qsTr("Set some values in the programmer first")
                            : qsTr("Then click a playback: an empty one gets a new cuelist, a cuelist its next cue (Esc cancels)")
                ToolTip.visible: hovered
                ToolTip.delay: 400
            }
            AppButton {
                id: updateButton
                text: qsTr("Update") + " ▾"
                iconGlyph: Theme.iconSave
                implicitHeight: 52
                enabled: !view.programLocked && view.hasProgrammer && !!view.selectedInfo.isCuelist
                onClicked: updateMenu.popup(updateButton, 0, updateButton.height)
                ToolTip.text: !view.selectedInfo.isCuelist ? qsTr("Select a cuelist playback first")
                            : qsTr("Merge the programmer into the cue of the selected cuelist")
                ToolTip.visible: hovered
                ToolTip.delay: 400
                AppMenu {
                    id: updateMenu
                    MenuItem {
                        text: view.updateNumber.length ? qsTr("Update cue %1 (the next cues follow)").arg(view.updateNumber)
                                                       : qsTr("Update the cue (the next cues follow)")
                        onTriggered: Core.playbacks.updateSelected(false)
                    }
                    MenuItem {
                        text: qsTr("Only this cue (the next cues keep their look)")
                        onTriggered: Core.playbacks.updateSelected(true)
                    }
                }
            }
            Text {
                Layout.fillWidth: true
                Layout.leftMargin: 4
                text: view.recordArmed ? qsTr("Click a playback to record the programmer on it · Esc cancels")
                    : view.assignOpen ? qsTr("Drag a scene onto a playback (double click: the first free one)")
                    : ""
                color: view.recordArmed ? Theme.danger : Theme.textDim
                font.pixelSize: Theme.fontSmall
                font.weight: view.recordArmed ? Font.DemiBold : Font.Normal
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }

            AppButton {
                text: qsTr("Release all")
                iconGlyph: Theme.iconStop
                implicitHeight: 52
                onClicked: Core.playbacks.releaseAll()
                ToolTip.text: qsTr("Release every playback and every cuelist that plays")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                MapTarget { target: ({ target: "playbackReleaseAll" }) }
            }
            AppButton {
                text: qsTr("Back")
                iconGlyph: Theme.iconStepBackward
                implicitHeight: 52
                leftPadding: 16
                rightPadding: 16
                onClicked: Core.playbacks.mainBack()
                ToolTip.text: qsTr("The cue before, on the selected playback (Shift+Space)")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                MapTarget { target: ({ target: "playbackMainBack" }) }
            }
            // Main GO: the selected playback, and what it plays next
            AbstractButton {
                id: mainGo
                Layout.preferredWidth: 250
                Layout.minimumWidth: 160
                implicitHeight: 52
                focusPolicy: Qt.NoFocus
                hoverEnabled: true
                onClicked: Core.playbacks.mainGo()
                readonly property string target: {
                    const i = view.selectedInfo
                    if (!view.hasSelection)
                        return qsTr("Select a playback")
                    if (i.isCuelist && i.nextNumber)
                        return (i.name || "") + "  ▸ " + i.nextNumber + (i.nextName ? " · " + i.nextName : "")
                    return i.name || ""
                }
                background: Rectangle {
                    radius: Theme.radius
                    color: mainGo.down ? Qt.darker(Theme.success, 1.25) : mainGo.hovered ? Qt.lighter(Theme.success, 1.1) : Theme.success
                }
                contentItem: ColumnLayout {
                    spacing: 0
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "GO"
                        color: "white"
                        font.pixelSize: Theme.fontTitle
                        font.weight: Font.Black
                    }
                    Text {
                        Layout.fillWidth: true
                        Layout.leftMargin: 8
                        Layout.rightMargin: 8
                        horizontalAlignment: Text.AlignHCenter
                        text: mainGo.target
                        color: "white"
                        opacity: view.hasSelection ? 0.95 : 0.7
                        font.pixelSize: Theme.fontSmall
                        elide: Text.ElideRight
                    }
                }
                ToolTip.text: qsTr("The next cue of the selected playback (Space)")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                MapTarget { target: ({ target: "playbackMainGo" }) }
            }
        }

        /* ------------------------------------------------- the playbacks */
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            Flickable {
                id: stripsView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: stripsRow.width
                contentHeight: height
                flickableDirection: Flickable.HorizontalFlick
                boundsBehavior: Flickable.StopAtBounds
                interactive: contentWidth > width
                ScrollBar.horizontal: ScrollBar { id: stripsBar; policy: stripsView.contentWidth > stripsView.width ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded }

                readonly property int count: Core.playbacks.slotsPerPage
                readonly property int gap: 6
                // equal widths, at least 84 px: more scroll
                readonly property real stripWidth: Math.max(84, Math.floor((width - (count - 1) * gap) / count))

                Row {
                    id: stripsRow
                    spacing: stripsView.gap
                    height: stripsView.height - (stripsView.contentWidth > stripsView.width ? 12 : 0)
                    Repeater {
                        id: stripsRepeater
                        model: stripsView.count
                        delegate: PlaybackStrip {
                            required property int index
                            width: stripsView.stripWidth
                            height: stripsRow.height
                            page: -1
                            slot: index
                            // a short window: one line of name and cue, smaller buttons
                            compact: height < 360
                            recordMode: view.recordArmed
                            editing: view.assignOpen
                            dragSource: dragProxy
                            onOpenScene: (id) => view.openScene(id)
                            onRecordClicked: view.recordArmed = false
                        }
                    }
                }
            }

            // Assign: the scenes, to drag onto the playbacks
            Panel {
                id: assignPanel
                Layout.preferredWidth: 290
                Layout.fillHeight: true
                visible: view.assignOpen
                title: qsTr("Scenes")
                headerItems: [
                    AppButton {
                        iconGlyph: Theme.iconClose
                        implicitHeight: 28
                        flat: true
                        onClicked: view.assignOpen = false
                    }
                ]
                SceneBrowser {
                    anchors { fill: parent; margins: 8 }
                    compact: true
                    typeFilter: true
                    dragTarget: dragProxy
                    emptyText: qsTr("No scenes yet. Record a cuelist from the programmer with Record.")
                    onDragPressed: (area, ev, fn) => view.beginDrag(area, ev, fn.id, fn.name)
                    onDragMoved: dragProxy.dragging = true
                    onDragReleased: dragProxy.drop()
                    onSceneDoubleClicked: (id) => {
                        const slot = Core.playbacks.firstFreeSlot(-1)
                        if (slot >= 0)
                            Core.playbacks.assign(-1, slot, id)
                    }
                }
            }
        }

        /* --------------------------------------- the selected playback */
        SelectedCuelist {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(150, Math.min(240, view.height * 0.27))
            onOpenScene: (id) => view.openScene(id)
        }
    }

    // a scene dragged from the list
    Rectangle {
        id: dragProxy
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
        Drag.keys: ["dtc-show-scene"]
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
    // A press anywhere on the page but the list of scenes (its search field)
    // gives the keyboard back to the page, after typing there: the buttons
    // and faders take no focus. On top of everything, so it sees the press
    // first; the PointHandler only watches (a passive grab): the press, the
    // drag and the hover still reach what is below
    Item {
        anchors.fill: parent
        z: 1000
        PointHandler {
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onActiveChanged: {
                if (!active || view.activeFocus)
                    return
                const p = assignPanel.mapFromItem(view, point.position)
                if (!assignPanel.visible || !assignPanel.contains(p))
                    view.forceActiveFocus()
            }
        }
    }

    function beginDrag(mouseArea, ev, functionId, label) {
        let p = mouseArea.mapToItem(view, ev.x, ev.y)
        dragProxy.x = p.x - dragProxy.width / 2
        dragProxy.y = p.y - dragProxy.height / 2
        dragProxy.functionId = functionId
        dragProxy.label = label
    }

    AppMenu {
        id: pageMenu
        property int index: -1
        function openFor(i) { index = i; popup() }
        function rename(i) {
            index = i
            prompt.ask(qsTr("Page name"), Core.playbacks.pageNames[i], (name) => Core.playbacks.renamePage(i, name))
        }
        MenuItem { text: qsTr("Rename…"); enabled: !view.programLocked; onTriggered: pageMenu.rename(pageMenu.index) }
        MenuItem {
            text: qsTr("Move left")
            enabled: !view.programLocked && pageMenu.index > 0
            onTriggered: Core.playbacks.movePage(pageMenu.index, pageMenu.index - 1)
        }
        MenuItem {
            text: qsTr("Move right")
            enabled: !view.programLocked && pageMenu.index < Core.playbacks.pageCount - 1
            onTriggered: Core.playbacks.movePage(pageMenu.index, pageMenu.index + 1)
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Delete page…")
            enabled: !view.programLocked && Core.playbacks.pageCount > 1
            onTriggered: deleteConfirm.openFor(pageMenu.index)
        }
    }

    TextPrompt { id: prompt }

    Popup {
        id: deleteConfirm
        property int index: -1
        function openFor(i) { index = i; open() }
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 440
        modal: true
        padding: 18
        background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }
        contentItem: ColumnLayout {
            spacing: 12
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Delete the page “%1”?").arg(Core.playbacks.pageNames[deleteConfirm.index] || "")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
            }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Its playbacks are cleared; what they play keeps playing, and the cuelists and scenes stay in Scenes.")
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
                    onClicked: { Core.playbacks.removePage(deleteConfirm.index); deleteConfirm.close() }
                }
            }
        }
    }
}
