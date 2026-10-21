/*
  DMX Total Control
  ZoneColumn.qml

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

Rectangle {
    id: column

    property Zone zoneObject
    property int zoneIndex: 0
    /** Show page: big buttons, no editing */
    property bool operatorMode: false
    /** Live page: the item dragged to reorder scenes and groups */
    property Item dragProxy: null
    readonly property bool canDrag: !operatorMode && dragProxy !== null
    /** The colour of the group (dimmed in the night theme) */
    readonly property color tint: zoneObject ? Theme.category(zoneObject.color) : Theme.accent
    /** Folded: only a strip with its name (its scenes play as always). The
     *  page keeps it (Live: the group; SHOW: the section) and folds it on
     *  foldToggled. foldsUp: a strip across, at the top (a column of sections) */
    property bool folded: false
    property bool canFold: false
    property bool foldsUp: false
    readonly property int foldedSize: 46
    signal foldToggled()

    // start dragging a scene or this group
    function beginDrag(mouseArea, ev, functionId, label, isZone) {
        let p = mouseArea.mapToItem(dragProxy.parent, ev.x, ev.y)
        dragProxy.x = p.x - dragProxy.width / 2
        dragProxy.y = p.y - dragProxy.height / 2
        dragProxy.fromZone = zoneIndex
        dragProxy.functionId = functionId
        dragProxy.label = label
        dragProxy.isZone = isZone
    }

    signal editScene(int id)
    signal openScene(int id)

    // The tiles are keyed by scene: only another set of scenes (or order)
    // rebuilds them, a change of state (playing, dimmer) updates them in
    // place. A rebuilt tile would lose its press (the release of a flash
    // scene never comes) and the grid would go back to the top
    property var sceneIds: []
    property var sceneMap: ({})
    function syncScenes() {
        let map = {}, ids = []
        for (let f of (zoneObject ? zoneObject.functions : [])) {
            map[f.id] = f
            ids.push(f.id)
        }
        sceneMap = map
        if (ids.join("|") !== sceneIds.join("|"))
            sceneIds = ids
    }
    onZoneObjectChanged: syncScenes()
    Component.onCompleted: syncScenes()
    Connections {
        target: column.zoneObject
        function onFunctionsChanged() { column.syncScenes() }
    }

    readonly property var fadePresets: [-1, 0, 500, 1000, 2000, 3000, 5000, 10000]

    function fadeLabel(ms) {
        if (ms < 0)
            return qsTr("By scene")
        if (ms === 0)
            return qsTr("Snap")
        return (ms / 1000).toLocaleString(Qt.locale(), 'f', ms % 1000 === 0 ? 0 : 1) + " s"
    }

    width: 300
    radius: Theme.radius
    color: Theme.surface
    border.color: columnDrop.containsDrag || groupDrop.containsDrag ? Theme.accent
                : zoneObject && zoneObject.activeCount > 0 ? Qt.darker(column.tint, 1.4) : Theme.border
    border.width: columnDrop.containsDrag || groupDrop.containsDrag ? 2 : 1

    // a scene dropped on the column, not on a scene: at the end of the group
    DropArea {
        id: columnDrop
        anchors.fill: parent
        z: -1
        keys: ["dtc-scene"]
        enabled: column.canDrag
        onDropped: (drop) => {
            Core.zones.moveScene(column.dragProxy.fromZone, column.dragProxy.functionId, column.zoneIndex, -1)
            drop.accept()
        }
    }
    // a group dropped on this one takes its place
    DropArea {
        id: groupDrop
        anchors.fill: parent
        keys: ["dtc-zone"]
        enabled: column.canDrag
        onDropped: (drop) => {
            Core.zones.moveZone(column.dragProxy.fromZone, column.zoneIndex)
            drop.accept()
        }
    }

    // colour strip
    Rectangle {
        height: 4
        radius: 2
        color: column.tint
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 1 }
    }

    // folded: unfold, the name, what plays, release
    Item {
        anchors { fill: parent; margins: 4; topMargin: 8 }
        visible: column.folded

        // a strip down (a row of groups)
        Column {
            visible: !column.foldsUp
            anchors { horizontalCenter: parent.horizontalCenter; top: parent.top }
            spacing: 6
            AppButton {
                anchors.horizontalCenter: parent.horizontalCenter
                implicitWidth: 34
                flat: true
                iconGlyph: Theme.iconChevronRight
                ToolTip.text: qsTr("Unfold the group")
                ToolTip.visible: hovered
                onClicked: column.foldToggled()
            }
            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: column.zoneObject && column.zoneObject.activeCount > 0
                width: 22; height: 22; radius: 11
                color: Theme.fill(column.tint)
                Text {
                    anchors.centerIn: parent
                    text: column.zoneObject ? column.zoneObject.activeCount : ""
                    color: Theme.textOn(Theme.fill(column.tint))
                    font.pixelSize: Theme.fontSmall
                    font.weight: Font.Bold
                }
                ToolTip.text: qsTr("%n scene(s) playing", "", column.zoneObject ? column.zoneObject.activeCount : 0)
                ToolTip.visible: activeHover.hovered
                HoverHandler { id: activeHover }
            }
        }
        // the name, read from bottom to top
        Text {
            visible: !column.foldsUp
            anchors.centerIn: parent
            width: parent.height - 150
            rotation: -90
            horizontalAlignment: Text.AlignHCenter
            text: column.zoneObject ? column.zoneObject.name : ""
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
            elide: Text.ElideRight
        }

        // a strip across (a column of sections)
        RowLayout {
            visible: column.foldsUp
            anchors { left: parent.left; right: parent.right; top: parent.top; leftMargin: 4; rightMargin: 4 }
            spacing: 6
            AppButton {
                implicitWidth: 34
                flat: true
                iconGlyph: Theme.iconChevronDown
                ToolTip.text: qsTr("Unfold the group")
                ToolTip.visible: hovered
                onClicked: column.foldToggled()
            }
            Text {
                Layout.fillWidth: true
                text: column.zoneObject ? column.zoneObject.name : ""
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
                elide: Text.ElideRight
            }
            Text {
                visible: column.zoneObject && column.zoneObject.activeCount > 0
                text: qsTr("%n playing", "", column.zoneObject ? column.zoneObject.activeCount : 0)
                color: Theme.readable(column.tint)
                font.pixelSize: Theme.fontSmall
                font.weight: Font.Bold
            }
        }
    }

    ColumnLayout {
        anchors { fill: parent; margins: 10; topMargin: 12 }
        spacing: 8
        visible: !column.folded

        // title row
        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            // grip: drag the group to reorder the groups
            IconLabel {
                visible: column.canDrag
                icon: Theme.iconGrip
                size: 13
                color: groupGrip.containsMouse || groupGrip.pressed ? Theme.text : Theme.textFaint
                Layout.preferredWidth: 16
                MouseArea {
                    id: groupGrip
                    anchors.fill: parent
                    anchors.margins: -4
                    hoverEnabled: true
                    cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                    drag.target: column.dragProxy
                    preventStealing: true
                    drag.threshold: 4
                    onPressed: (ev) => column.beginDrag(groupGrip, ev, -1, column.zoneObject ? column.zoneObject.name : "", true)
                    onPositionChanged: if (drag.active) column.dragProxy.dragging = true
                    onReleased: column.dragProxy.drop()
                }
            }

            TextField {
                id: nameField
                Layout.fillWidth: true
                text: column.zoneObject ? column.zoneObject.name : ""
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
                color: Theme.text
                selectByMouse: true
                readOnly: column.operatorMode
                background: Rectangle {
                    color: nameField.activeFocus ? Theme.background : "transparent"
                    radius: Theme.radiusSmall
                    border.color: nameField.activeFocus ? Theme.accent : "transparent"
                }
                onEditingFinished: {
                    if (column.zoneObject && text.trim().length > 0)
                        column.zoneObject.name = text.trim()
                    focus = false
                }
            }

            AppButton {
                visible: column.canFold
                iconGlyph: column.foldsUp ? Theme.iconChevronUp : Theme.iconChevronLeft
                flat: true
                ToolTip.text: qsTr("Fold the group: a strip with its name, to make room for the others. Its scenes go on playing")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: column.foldToggled()
            }
            AppButton {
                iconGlyph: Theme.iconStop
                flat: true
                enabled: column.zoneObject && column.zoneObject.activeCount > 0
                ToolTip.text: qsTr("Release this group")
                ToolTip.visible: hovered
                onClicked: column.zoneObject.stopAll()
                MapTarget { target: ({ target: "groupRelease", zoneId: column.zoneObject ? column.zoneObject.id : -1 }) }
            }
            AppButton {
                iconGlyph: Theme.iconGear
                flat: true
                visible: !column.operatorMode
                onClicked: zoneMenu.popup()

                AppMenu {
                    id: zoneMenu
                    MenuItem {
                        text: qsTr("Change colour")
                        onTriggered: {
                            const colors = ["#3d8bfd", "#f59f00", "#2fb344", "#d63939", "#ae3ec9", "#17a2b8", "#f76707", "#74b816"]
                            let i = colors.indexOf(column.zoneObject.color.toString())
                            column.zoneObject.color = colors[(i + 1) % colors.length]
                        }
                    }
                    MenuItem {
                        text: qsTr("Move left")
                        enabled: column.zoneIndex > 0
                        onTriggered: Core.zones.moveZone(column.zoneIndex, column.zoneIndex - 1)
                    }
                    MenuItem {
                        text: qsTr("Move right")
                        enabled: column.zoneIndex < Core.zones.count - 1
                        onTriggered: Core.zones.moveZone(column.zoneIndex, column.zoneIndex + 1)
                    }
                    MenuSeparator {}
                    MenuItem {
                        text: qsTr("Delete group")
                        onTriggered: Core.zones.removeZone(column.zoneIndex)
                    }
                }
            }
        }

        // mode + fade: the fade box as wide as its longest label ("Por
        // escena" was cut at a fixed width), the mode button takes the rest
        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            AppButton {
                text: "◀"
                implicitHeight: 34
                leftPadding: 9
                rightPadding: 9
                ToolTip.text: qsTr("Previous scene")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: column.zoneObject.previous()
                MapTarget { target: ({ target: "groupPrevious", zoneId: column.zoneObject ? column.zoneObject.id : -1 }) }
            }
            AppButton {
                text: "▶"
                implicitHeight: 34
                leftPadding: 9
                rightPadding: 9
                ToolTip.text: qsTr("Next scene")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: column.zoneObject.next()
                MapTarget { target: ({ target: "groupNext", zoneId: column.zoneObject ? column.zoneObject.id : -1 }) }
            }
            Item { Layout.fillWidth: true; visible: column.operatorMode }
            AppButton {
                Layout.fillWidth: true
                visible: !column.operatorMode
                text: column.zoneObject && column.zoneObject.exclusive ? qsTr("Exclusive") : qsTr("Mix")
                iconGlyph: column.zoneObject && column.zoneObject.exclusive ? Theme.iconCircle : Theme.iconLayers
                ToolTip.text: qsTr("Exclusive: one scene at a time. Mix: scenes add up.")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: column.zoneObject.exclusive = !column.zoneObject.exclusive
            }
            ComboBox {
                id: fadeCombo
                visible: !column.operatorMode
                implicitContentWidthPolicy: ComboBox.WidestText
                Layout.preferredWidth: implicitContentWidth + leftPadding + rightPadding
                implicitHeight: 34
                focusPolicy: Qt.NoFocus
                model: column.fadePresets.map(column.fadeLabel)
                currentIndex: column.zoneObject ? Math.max(0, column.fadePresets.indexOf(column.zoneObject.fadeTime)) : 0
                onActivated: (i) => column.zoneObject.fadeTime = column.fadePresets[i]
                ToolTip.text: qsTr("Crossfade time")
                ToolTip.visible: hovered
                ToolTip.delay: 500
            }
        }

        // scenes
        GridView {
            id: grid
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            cellWidth: width / 2
            cellHeight: column.operatorMode ? 104 : 78
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}

            model: column.sceneIds

            delegate: Item {
                id: tileCell
                // the ID of the scene; what it is, from the map of the column
                required property int modelData
                required property int index
                readonly property var info: column.sceneMap[modelData] || ({ id: modelData, name: "", type: "" })
                width: grid.cellWidth
                height: grid.cellHeight

                HoverHandler { id: cellHover }

                SceneTile {
                    anchors { fill: parent; margins: 3 }
                    name: tileCell.info.name
                    type: tileCell.info.type
                    functionId: tileCell.modelData
                    active: !!tileCell.info.active
                    zoneColor: column.tint
                    keyIndex: tileCell.index + 1
                    priority: tileCell.info.priority !== undefined ? tileCell.info.priority : 5
                    large: column.operatorMode
                    flash: !!tileCell.info.flash
                    tap: tileCell.info.tap || 0
                    onPressedDown: column.zoneObject.press(tileCell.modelData)
                    onReleasedUp: column.zoneObject.unpress(tileCell.modelData)
                    onContextRequested: {
                        if (column.operatorMode)
                            return
                        tileMenu.functionId = tileCell.modelData
                        tileMenu.functionType = tileCell.info.type
                        tileMenu.functionIndex = tileCell.index
                        tileMenu.functionPriority = tileCell.info.priority !== undefined ? tileCell.info.priority : 5
                        tileMenu.popup()
                    }
                }

                // drop a scene here: it goes before this one
                DropArea {
                    id: tileDrop
                    anchors.fill: parent
                    keys: ["dtc-scene"]
                    onDropped: (drop) => {
                        Core.zones.moveScene(column.dragProxy.fromZone, column.dragProxy.functionId,
                                             column.zoneIndex, tileCell.index)
                        drop.accept()
                    }
                }
                Rectangle {
                    visible: tileDrop.containsDrag
                    x: 0
                    y: 6
                    width: 4
                    height: parent.height - 12
                    radius: 2
                    color: Theme.accent
                }

                // learn mode: map this scene
                MapTarget { target: ({ target: "scene", functionId: tileCell.modelData }) }

                // grip: drag the scene to another place or group
                Rectangle {
                    visible: column.canDrag && (cellHover.hovered || gripMouse.pressed)
                    anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 8 }
                    width: 20
                    height: 24
                    radius: 4
                    color: Theme.scrim
                    IconLabel { anchors.centerIn: parent; icon: Theme.iconGrip; size: 11; color: Theme.textOnShade }
                    MouseArea {
                        id: gripMouse
                        anchors.fill: parent
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                        drag.target: column.dragProxy
                        preventStealing: true
                        drag.threshold: 4
                        onPressed: (ev) => column.beginDrag(gripMouse, ev, tileCell.modelData, tileCell.info.name, false)
                        onPositionChanged: if (drag.active) column.dragProxy.dragging = true
                        onReleased: column.dragProxy.drop()
                        ToolTip.text: qsTr("Drag to move the scene, also to another group")
                        ToolTip.visible: containsMouse && !pressed
                        ToolTip.delay: 600
                        hoverEnabled: true
                    }
                }
            }

            footer: Item {
                width: grid.width
                height: column.operatorMode ? 0 : 50
                AppButton {
                    anchors { fill: parent; margins: 3; topMargin: 6 }
                    visible: !column.operatorMode
                    text: qsTr("Add scenes")
                    iconGlyph: Theme.iconPlus
                    flat: true
                    onClicked: picker.open()
                }
            }
        }

        // zone master
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            radius: Theme.radiusSmall
            color: Theme.background
            border.color: Theme.border

            RowLayout {
                anchors { fill: parent; leftMargin: 10; rightMargin: 10 }
                spacing: 8
                Text { text: qsTr("Master"); color: Theme.textDim; font.pixelSize: Theme.fontSmall; font.weight: Font.Bold }
                HSlider {
                    Layout.fillWidth: true
                    fillColor: column.tint
                    from: 0; to: 1
                    value: column.zoneObject ? column.zoneObject.master : 1
                    onMoved: column.zoneObject.master = value
                    MapTarget { target: ({ target: "groupMaster", zoneId: column.zoneObject ? column.zoneObject.id : -1 }) }
                }
                Text {
                    Layout.preferredWidth: 36
                    horizontalAlignment: Text.AlignRight
                    text: Math.round((column.zoneObject ? column.zoneObject.master : 1) * 100) + "%"
                    color: Theme.text
                    font.family: Theme.monoFamily
                    font.pixelSize: Theme.fontSmall
                }
            }
        }
    }

    AppMenu {
        id: tileMenu
        property int functionId: -1
        property string functionType: ""
        property int functionIndex: -1
        property int functionPriority: 5
        readonly property bool isCuelist: functionType === "Cuelist"

        // a cuelist: its transport (the tile toggles it, as in a group)
        MenuItem {
            text: qsTr("GO (next cue)")
            visible: tileMenu.isCuelist
            height: visible ? implicitHeight : 0
            // a stopped list starts in its group, as its tile does (the group
            // releases its other scenes, its master applies, the tile lights
            // and its next press releases it); a list that plays goes on
            onTriggered: {
                const fid = tileMenu.functionId
                if (!column.zoneObject.isActive(fid) && !Core.playbacks.functionStatus(fid).running)
                    column.zoneObject.start(fid)
                else
                    Core.playbacks.goFunction(fid)
            }
        }
        MenuItem {
            text: qsTr("Back (cue before)")
            visible: tileMenu.isCuelist
            height: visible ? implicitHeight : 0
            onTriggered: Core.playbacks.backFunction(tileMenu.functionId)
        }
        MenuItem {
            text: qsTr("Release the cuelist")
            visible: tileMenu.isCuelist
            height: visible ? implicitHeight : 0
            onTriggered: Core.playbacks.releaseFunction(tileMenu.functionId)
        }
        MenuSeparator {
            visible: tileMenu.isCuelist
            height: visible ? implicitHeight : 0
        }
        MenuItem {
            text: qsTr("Properties…")
            onTriggered: propertiesDialog.edit(column.zoneObject, tileMenu.functionId)
        }
        MenuItem {
            text: qsTr("Open in Scenes")
            onTriggered: column.openScene(tileMenu.functionId)
        }
        MenuItem {
            text: qsTr("Edit in the programmer")
            enabled: tileMenu.functionType === "Scene"
            onTriggered: column.editScene(tileMenu.functionId)
        }
        AppMenu {
            id: priorityMenu
            title: qsTr("Priority: %1").arg(tileMenu.functionPriority)
            Instantiator {
                model: 11
                delegate: MenuItem {
                    required property int index
                    text: index === 5 ? qsTr("%1 (normal)").arg(index) : index
                    checkable: true
                    checked: tileMenu.functionPriority === index
                    onTriggered: Core.setFunctionPriority(tileMenu.functionId, index)
                }
                onObjectAdded: (index, object) => priorityMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => priorityMenu.removeItem(object)
            }
        }
        MenuItem {
            text: qsTr("Move up")
            enabled: tileMenu.functionIndex > 0
            onTriggered: column.zoneObject.moveFunction(tileMenu.functionIndex, tileMenu.functionIndex - 1)
        }
        MenuItem {
            text: qsTr("Move down")
            onTriggered: column.zoneObject.moveFunction(tileMenu.functionIndex, tileMenu.functionIndex + 1)
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Remove from group")
            onTriggered: column.zoneObject.removeFunction(tileMenu.functionId)
        }
    }

    SceneProperties { id: propertiesDialog }

    FunctionPicker {
        id: picker
        zoneObject: column.zoneObject
    }
}
