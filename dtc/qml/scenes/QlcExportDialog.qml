/*
  DMX Total Control
  QlcExportDialog.qml

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
 * Save a copy for QLC+: another show in which the cuelists checked are QLC+
 * steps (a Sequence or a Chaser), each with a Cue List in its Virtual
 * Console. The show open doesn't change. Open it with start().
 */
Popup {
    id: dialog

    /** "sequence" or "chaser" */
    property string format: "sequence"
    /** Every cuelist: [{ id, name, cues, lost }] (Core.qlcExportInfo) */
    property var lists: []
    /** The IDs of the cuelists checked */
    property var chosen: []

    signal message(string text, bool isError)

    /** The keys and controller buttons go on the buttons of the Cue Lists */
    readonly property bool controls: consoleCheck.checked && controlsCheck.checked

    function start() {
        format = "sequence"
        consoleCheck.checked = true
        controlsCheck.checked = true
        lists = Core.qlcExportInfo(format, controls)
        chosen = lists.map(l => l.id)
        open()
    }
    function toggle(id) {
        chosen = chosen.indexOf(id) >= 0 ? chosen.filter(x => x !== id) : chosen.concat([id])
    }
    function saveTo(url) {
        if (Core.saveCopyForQlc(url, chosen, { format: format, virtualConsole: consoleCheck.checked, controls: controls })) {
            const name = decodeURIComponent(url.toString().replace(/^.*\//, ""))
            message(qsTr("Saved for QLC+: %1").arg(name), false)
        } else {
            message(Core.lastError, true)
        }
    }

    // what QLC+ won't get depends on the format and on the controls
    onFormatChanged: if (visible) lists = Core.qlcExportInfo(format, controls)
    onControlsChanged: if (visible) lists = Core.qlcExportInfo(format, controls)

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(600, parent ? parent.width - 40 : 600)
    modal: true
    padding: 18

    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    // a format: its name and what it gives in QLC+
    component FormatChoice: Rectangle {
        id: choice
        property string value: ""
        property string title: ""
        property string detail: ""
        readonly property bool selected: dialog.format === value
        implicitHeight: choiceColumn.implicitHeight + 16
        radius: Theme.radiusSmall
        color: selected ? Qt.alpha(Theme.accent, 0.14) : choiceMouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised
        border.color: selected ? Theme.accent : Theme.border
        ColumnLayout {
            id: choiceColumn
            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; leftMargin: 12; rightMargin: 12 }
            spacing: 2
            Text {
                Layout.fillWidth: true
                text: choice.title
                color: Theme.text
                font.weight: Font.DemiBold
                wrapMode: Text.WordWrap
            }
            Text {
                Layout.fillWidth: true
                text: choice.detail
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
                wrapMode: Text.WordWrap
            }
        }
        MouseArea {
            id: choiceMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: dialog.format = choice.value
        }
    }

    contentItem: ColumnLayout {
        spacing: 10

        Text {
            Layout.fillWidth: true
            text: qsTr("Save a copy for QLC+")
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
            wrapMode: Text.WordWrap
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("QLC+ doesn't read cuelists. The copy is another show in which each cuelist checked is a list of QLC+ steps; each step holds the whole look of its cue. Under each cuelist, what QLC+ won't get. This show doesn't change.")
            color: Theme.textDim
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6
            Text {
                Layout.fillWidth: true
                text: qsTr("Cuelists")
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
            }
            AppButton { text: qsTr("All"); flat: true; implicitHeight: 26; onClicked: dialog.chosen = dialog.lists.map(l => l.id) }
            AppButton { text: qsTr("None"); flat: true; implicitHeight: 26; onClicked: dialog.chosen = [] }
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(listView.contentHeight, 230) + 2
            radius: Theme.radiusSmall
            color: Theme.background
            border.color: Theme.border
            ListView {
                id: listView
                anchors { fill: parent; margins: 1 }
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                model: dialog.lists
                ScrollBar.vertical: ScrollBar { policy: listView.contentHeight > listView.height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded }
                delegate: Rectangle {
                    id: row
                    required property var modelData
                    readonly property bool checked: dialog.chosen.indexOf(modelData.id) >= 0
                    width: ListView.view.width
                    implicitHeight: rowLayout.implicitHeight + 12
                    color: rowMouse.containsMouse ? Theme.surfaceHover : "transparent"
                    RowLayout {
                        id: rowLayout
                        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; leftMargin: 8; rightMargin: 12 }
                        spacing: 8
                        // (the row takes the clicks: the box only shows)
                        CheckBox { checked: row.checked; focusPolicy: Qt.NoFocus }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 1
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8
                                Text {
                                    Layout.fillWidth: true
                                    text: row.modelData.name
                                    color: Theme.text
                                    font.weight: Font.DemiBold
                                    elide: Text.ElideRight
                                }
                                Text {
                                    text: qsTr("%n cue(s)", "", row.modelData.cues)
                                    color: Theme.textFaint
                                    font.pixelSize: Theme.fontSmall
                                }
                            }
                            Text {
                                Layout.fillWidth: true
                                visible: text.length > 0
                                text: row.modelData.lost.join(" · ")
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSmall
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                    MouseArea {
                        id: rowMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: dialog.toggle(row.modelData.id)
                    }
                }
            }
            Text {
                anchors.centerIn: parent
                visible: dialog.lists.length === 0
                text: qsTr("This show has no cuelists")
                color: Theme.textFaint
            }
        }

        Text { text: qsTr("As"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            FormatChoice {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                value: "sequence"
                title: qsTr("Sequence (one step list)")
                detail: qsTr("One item in QLC+. Released values go to their rest value.")
            }
            FormatChoice {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                value: "chaser"
                title: qsTr("Chaser (a scene for each cue)")
                detail: qsTr("The scenes go in a folder named like the cuelist. Released values are left to QLC+.")
            }
        }

        CheckBox {
            id: consoleCheck
            Layout.fillWidth: true
            checked: true
            text: qsTr("A cue list for each one in the Virtual Console")
            ToolTip.text: qsTr("Its Next button is the GO, Previous is the BACK and Stop releases it")
            ToolTip.visible: hovered
            ToolTip.delay: 500
        }
        CheckBox {
            id: controlsCheck
            Layout.fillWidth: true
            Layout.leftMargin: 24
            // (it goes with the box above)
            Layout.topMargin: -10
            checked: true
            enabled: consoleCheck.checked
            // (the app palette doesn't dim a disabled box)
            opacity: enabled ? 1 : 0.45
            text: qsTr("With their keys and controller buttons")
            ToolTip.text: qsTr("GO, Back, Pause/Back and Release become Next, Previous, Playback and Stop of its cue list")
            ToolTip.visible: hovered
            ToolTip.delay: 500
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Cancel"); onClicked: dialog.close() }
            AppButton {
                text: qsTr("Save the copy…")
                iconGlyph: Theme.iconSaveAs
                primary: true
                enabled: dialog.chosen.length > 0
                onClicked: {
                    dialog.close()
                    // "<show> (QLC+).qxw" beside the show
                    fileDialog.selectedFile = Core.qlcCopyUrl()
                    fileDialog.start()
                }
            }
        }

        AppFileDialog {
            id: fileDialog
            kind: "show"
            title: qsTr("Save a copy for QLC+")
            fileMode: FileDialog.SaveFile
            defaultSuffix: "qxw"
            nameFilters: [qsTr("Show files (*.qxw)")]
            onAccepted: dialog.saveTo(selectedFile)
        }
    }
}
