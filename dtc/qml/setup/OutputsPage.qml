/*
  DMX Total Control
  OutputsPage.qml

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

/** Universes and their outputs: DMX USB interfaces, Art-Net, sACN... */
Item {
    id: page

    property bool levelsView: false

    ColumnLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Column {
                Text {
                    text: qsTr("Outputs")
                    color: Theme.text
                    font.pixelSize: Theme.fontTitle
                    font.weight: Font.Bold
                }
                Text {
                    text: qsTr("Connect each universe to a DMX interface: DMX USB, Art-Net, sACN and more.")
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                }
            }
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Universes"); iconGlyph: Theme.iconPlug; highlighted: !page.levelsView; onClicked: page.levelsView = false }
            AppButton { text: qsTr("DMX levels"); iconGlyph: Theme.iconSliders; highlighted: page.levelsView; onClicked: page.levelsView = true }
            Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 28; color: Theme.border }
            AppButton { text: qsTr("Rescan"); iconGlyph: Theme.iconRefresh; onClicked: Core.io.refresh() }
            AppButton { text: qsTr("Remove universe"); enabled: Core.io.universes.length > 1; onClicked: removeConfirm.ask() }
            AppButton { text: qsTr("Add universe"); iconGlyph: Theme.iconPlus; primary: true; onClicked: Core.io.addUniverse() }
        }

        DmxMonitor {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: page.levelsView
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !page.levelsView
            spacing: 12

            Panel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: qsTr("Universes")

                ListView {
                    id: uniList
                    anchors { fill: parent; margins: 8 }
                    clip: true
                    spacing: 6
                    model: Core.io.universes
                    ScrollBar.vertical: ScrollBar {}

                    delegate: Rectangle {
                        id: uniRow
                        required property var modelData
                        width: uniList.width
                        height: 64
                        radius: Theme.radiusSmall
                        color: Theme.surfaceRaised
                        border.color: modelData.line >= 0 ? Qt.alpha(Theme.success, 0.5) : Theme.border

                        readonly property var outputChoices: [{ key: "", label: qsTr("Not connected") }].concat(
                            Core.io.outputs.map(o => ({ key: o.key, label: o.plugin + " — " + o.name })))

                        RowLayout {
                            anchors { fill: parent; leftMargin: 14; rightMargin: 14 }
                            spacing: 14

                            Rectangle {
                                width: 40; height: 40; radius: 8
                                color: uniRow.modelData.line >= 0 ? Qt.alpha(Theme.success, 0.18) : Theme.background
                                Text {
                                    anchors.centerIn: parent
                                    text: uniRow.modelData.index + 1
                                    color: uniRow.modelData.line >= 0 ? Theme.success : Theme.textDim
                                    font.pixelSize: Theme.fontLarge
                                    font.weight: Font.Bold
                                }
                            }

                            TextField {
                                id: uniName
                                Layout.preferredWidth: 200
                                text: uniRow.modelData.name
                                color: Theme.text
                                selectByMouse: true
                                background: Rectangle { color: uniName.activeFocus ? Theme.background : "transparent"; radius: 4; border.color: uniName.activeFocus ? Theme.accent : "transparent" }
                                onEditingFinished: { Core.io.setUniverseName(uniRow.modelData.index, text); focus = false }
                            }

                            IconLabel { icon: Theme.iconPlug; color: uniRow.modelData.line >= 0 ? Theme.success : Theme.textFaint }

                            ComboBox {
                                id: outCombo
                                Layout.fillWidth: true
                                textRole: "label"
                                model: uniRow.outputChoices
                                currentIndex: {
                                    let key = uniRow.modelData.line >= 0 ? uniRow.modelData.plugin + "|" + uniRow.modelData.line : ""
                                    let idx = uniRow.outputChoices.findIndex(c => c.key === key)
                                    return Math.max(0, idx)
                                }
                                onActivated: (i) => {
                                    let key = uniRow.outputChoices[i].key
                                    if (key.length === 0) {
                                        Core.io.clearOutput(uniRow.modelData.index)
                                    } else {
                                        let sep = key.lastIndexOf("|")
                                        Core.io.setOutput(uniRow.modelData.index, key.substring(0, sep), parseInt(key.substring(sep + 1)))
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Panel {
                Layout.preferredWidth: 340
                Layout.fillHeight: true
                title: qsTr("Detected outputs")

                ListView {
                    id: outList
                    anchors { fill: parent; margins: 8 }
                    clip: true
                    spacing: 4
                    model: Core.io.outputs
                    ScrollBar.vertical: ScrollBar {}
                    delegate: Column {
                        required property var modelData
                        width: outList.width
                        leftPadding: 6
                        bottomPadding: 6
                        Text {
                            width: parent.width - 12
                            text: modelData.name
                            color: Theme.text
                            elide: Text.ElideRight
                        }
                        Text {
                            text: modelData.plugin
                            color: Theme.textFaint
                            font.pixelSize: Theme.fontSmall
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        width: parent.width - 30
                        visible: outList.count === 0
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        text: qsTr("No outputs found. Plug in your DMX interface and press Rescan.")
                        color: Theme.textDim
                    }
                }
            }
        }
    }

    // Removing the last universe: not while fixtures are patched in it (they
    // would be left on a universe that doesn't exist), and asked first when it
    // is the input of a controller
    Popup {
        id: removeConfirm
        property int universe: -1
        property var use: ({})
        readonly property bool blocked: (use.fixtures || 0) > 0
        function ask() {
            universe = Core.io.universes.length - 1
            use = Core.io.universeUse(universe)
            // nothing in it: away at once
            if (!blocked && (use.devices || []).length === 0) {
                Core.io.removeLastUniverse()
                return
            }
            open()
        }
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
                text: removeConfirm.blocked ? qsTr("Universe %1 has fixtures").arg(removeConfirm.universe + 1)
                                            : qsTr("Remove universe %1?").arg(removeConfirm.universe + 1)
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
            }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: removeConfirm.blocked
                      ? qsTr("Fixtures patched in it: %1. Move them to another universe or delete them in Patch first.").arg(removeConfirm.use.fixtures)
                      : qsTr("It is the input of %1: that controller stops working until you enable it again in Mappings.").arg((removeConfirm.use.devices || []).join(", "))
                color: Theme.textDim
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton { text: removeConfirm.blocked ? qsTr("OK") : qsTr("Cancel"); onClicked: removeConfirm.close() }
                AppButton {
                    visible: !removeConfirm.blocked
                    text: qsTr("Remove")
                    primary: true
                    accentColor: Theme.danger
                    onClicked: { Core.io.removeLastUniverse(); removeConfirm.close() }
                }
            }
        }
    }
}
