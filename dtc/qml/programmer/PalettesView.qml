/*
  DMX Total Control
  PalettesView.qml

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
 * The palettes: intensity, position, colour and beam.
 * "+" records the values of the selection; a click puts a palette on the
 * selection. The scenes recorded with a palette follow it when it changes.
 */
Item {
    id: view

    readonly property var types: [
        { type: "intensity", name: qsTr("Intensity"), word: "INT", icon: Theme.iconSun, color: "#e7e9ef" },
        { type: "position", name: qsTr("Position"), word: "POS", icon: Theme.iconMove, color: Theme.functionColor("EFX") },
        { type: "colour", name: qsTr("Colour"), word: "COL", icon: Theme.iconPalette, color: "#ff6a00" },
        { type: "beam", name: qsTr("Beam"), word: "BEAM", icon: Theme.iconCircle, color: "#3d8bfd" }
    ]
    readonly property bool hasSelection: Core.programmer.selection.length > 0
    property string notice: ""

    function say(text) {
        notice = text
        noticeTimer.restart()
    }

    Timer { id: noticeTimer; interval: 3000; onTriggered: view.notice = "" }

    RowLayout {
        anchors.fill: parent
        spacing: 8

        Repeater {
            model: view.types

            // one pool of palettes
            Rectangle {
                id: pool
                required property var modelData
                readonly property var palettes: Core.palettes.palettes.filter(p => p.type === modelData.type)
                readonly property int recordable: Core.palettes.recordable[modelData.type] || 0

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 1000
                radius: Theme.radius
                color: Theme.background
                border.color: Theme.border

                ColumnLayout {
                    anchors { fill: parent; margins: 8 }
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        IconLabel { icon: pool.modelData.icon; size: 12; color: pool.modelData.color }
                        Text {
                            Layout.fillWidth: true
                            text: pool.modelData.name
                            color: Theme.text
                            font.weight: Font.Bold
                            elide: Text.ElideRight
                        }
                        AppButton {
                            iconGlyph: Theme.iconPlus
                            text: qsTr("Record")
                            implicitHeight: 26
                            enabled: pool.recordable > 0
                            ToolTip.text: pool.recordable > 0
                                ? qsTr("Record the %1 values of the selection as a new palette").arg(pool.modelData.name.toLowerCase())
                                : qsTr("Set some %1 values on the selection first").arg(pool.modelData.name.toLowerCase())
                            ToolTip.visible: hovered
                            ToolTip.delay: 400
                            onClicked: {
                                const id = Core.palettes.record(pool.modelData.type)
                                if (id > 0)
                                    view.say(qsTr("Recorded %1: the scenes you record now follow it").arg(Core.palettes.palette(id).name))
                            }
                        }
                    }

                    Flickable {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        contentHeight: tiles.height
                        boundsBehavior: Flickable.StopAtBounds
                        ScrollBar.vertical: ScrollBar {}

                        Flow {
                            id: tiles
                            width: parent.width
                            spacing: 6

                            Repeater {
                                model: pool.palettes

                                Rectangle {
                                    id: tile
                                    required property var modelData
                                    // on the selection: 2 all of it, 1 some
                                    readonly property int level: Core.palettes.states[String(modelData.id)] || 0
                                    readonly property color swatch: modelData.color.length > 0 ? modelData.color : pool.modelData.color

                                    width: Math.max(86, (tiles.width - 6) / 2)
                                    height: 54
                                    radius: Theme.radiusSmall
                                    color: level === 2 ? Qt.alpha(Theme.accent, 0.35)
                                         : tileMouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised
                                    border.color: level > 0 ? Theme.accent : Theme.border
                                    border.width: level > 0 ? 2 : 1

                                    // the colour of the palette, or of its type
                                    Rectangle {
                                        anchors { left: parent.left; top: parent.top; bottom: parent.bottom; margins: 4 }
                                        width: pool.modelData.type === "colour" ? 14 : 4
                                        radius: 3
                                        color: tile.swatch
                                        border.color: "#00000060"
                                    }
                                    Text {
                                        anchors { right: parent.right; top: parent.top; margins: 5 }
                                        text: pool.modelData.word + " " + tile.modelData.number
                                        color: Theme.textFaint
                                        font.family: Theme.monoFamily
                                        font.pixelSize: 9
                                    }
                                    Text {
                                        anchors { left: parent.left; right: parent.right; bottom: parent.bottom
                                                  leftMargin: pool.modelData.type === "colour" ? 24 : 14; rightMargin: 6; bottomMargin: 6 }
                                        text: tile.modelData.name
                                        color: Theme.text
                                        font.pixelSize: Theme.fontSmall + 1
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }
                                    // used by scenes: they follow it
                                    IconLabel {
                                        visible: tile.modelData.scenes > 0
                                        anchors { left: parent.left; top: parent.top; leftMargin: pool.modelData.type === "colour" ? 24 : 14; topMargin: 6 }
                                        icon: Theme.iconLink
                                        size: 9
                                        color: Theme.textFaint
                                    }

                                    MouseArea {
                                        id: tileMouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                                        cursorShape: Qt.PointingHandCursor
                                        ToolTip.visible: containsMouse
                                        ToolTip.delay: 700
                                        ToolTip.text: qsTr("%1 — recorded on %n fixture(s)", "", tile.modelData.fixtures).arg(tile.modelData.name)
                                                      + (tile.modelData.scenes > 0 ? "\n" + qsTr("Used by %n scene(s): they change with it", "", tile.modelData.scenes) : "")
                                                      + "\n" + qsTr("Click: on the selection · Right click: update, load, rename…")
                                        onClicked: (mouse) => {
                                            if (mouse.button === Qt.RightButton) {
                                                paletteMenu.palette = tile.modelData
                                                paletteMenu.popup()
                                            } else if (!view.hasSelection) {
                                                view.say(qsTr("Select fixtures first"))
                                            } else if (Core.palettes.apply(tile.modelData.id) === 0) {
                                                view.say(qsTr("%1 doesn't go on the selected fixtures").arg(tile.modelData.name))
                                            }
                                        }
                                    }

                                    MapTarget { target: ({ target: "palette", paletteId: tile.modelData.id, groupId: -1 }) }
                                }
                            }
                        }

                        Text {
                            anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 8 }
                            visible: pool.palettes.length === 0
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            text: qsTr("Set the %1 of the selection and press Record").arg(pool.modelData.name.toLowerCase())
                            color: Theme.textFaint
                            font.pixelSize: Theme.fontSmall
                        }
                    }
                }
            }
        }
    }

    // what just happened
    Rectangle {
        anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 6 }
        visible: view.notice.length > 0
        width: noticeText.implicitWidth + 24
        height: 30
        radius: 15
        color: Theme.surfaceRaised
        border.color: Theme.borderStrong
        Text {
            id: noticeText
            anchors.centerIn: parent
            text: view.notice
            color: Theme.text
            font.pixelSize: Theme.fontSmall
        }
    }

    AppMenu {
        id: paletteMenu
        property var palette: ({})
        MenuItem {
            text: qsTr("Put on the selection")
            enabled: view.hasSelection
            onTriggered: Core.palettes.apply(paletteMenu.palette.id)
        }
        MenuItem {
            text: qsTr("Load its fixtures in the programmer")
            onTriggered: Core.palettes.load(paletteMenu.palette.id)
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Update with the programmer")
            enabled: (Core.palettes.recordable[paletteMenu.palette.type] || 0) > 0
            onTriggered: {
                if (Core.palettes.update(paletteMenu.palette.id))
                    view.say(qsTr("%1 updated: the scenes that use it changed too").arg(paletteMenu.palette.name))
            }
        }
        MenuItem {
            text: qsTr("Record over it")
            enabled: (Core.palettes.recordable[paletteMenu.palette.type] || 0) > 0
            onTriggered: {
                if (Core.palettes.replace(paletteMenu.palette.id))
                    view.say(qsTr("%1 recorded again: the scenes that use it changed too").arg(paletteMenu.palette.name))
            }
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Rename…")
            onTriggered: {
                const id = paletteMenu.palette.id
                prompt.ask(qsTr("Rename palette"), paletteMenu.palette.name, (name) => Core.palettes.rename(id, name))
            }
        }
        MenuItem {
            text: qsTr("Change its number…")
            onTriggered: {
                const id = paletteMenu.palette.id
                prompt.ask(qsTr("Number of the palette"), String(paletteMenu.palette.number), (text) => {
                    const error = Core.palettes.setNumber(id, parseInt(text))
                    if (error.length > 0)
                        view.say(error)
                })
            }
        }
        MenuItem {
            text: qsTr("Delete")
            onTriggered: Core.palettes.remove(paletteMenu.palette.id)
        }
    }

    TextPrompt { id: prompt }
}
