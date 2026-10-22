/*
  DMX Total Control
  MissingFilesDialog.qml

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
 * The files the show names that aren't there (its sounds, videos and
 * pictures), after opening it: they can be looked for in a folder, by name
 * (a show brought from another computer)
 */
Popup {
    id: dialog

    /** What happened, for a toast */
    signal message(string text, bool isError)

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(560, parent ? parent.width - 32 : 560)
    modal: true
    padding: 18
    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    FolderDialog {
        id: folderDialog
        title: qsTr("Folder with the missing files")
        onAccepted: {
            const found = Core.locateMissingFiles(selectedFolder)
            if (found === 0) {
                dialog.message(qsTr("None of the missing files is in that folder"), true)
                return
            }
            dialog.message(Core.missingFiles.length > 0
                           ? qsTr("%n file(s) found, %1 still missing", "", found).arg(Core.missingFiles.length)
                           : qsTr("%n file(s) found. Save the show to keep them", "", found), false)
            if (Core.missingFiles.length === 0)
                dialog.close()
        }
    }

    contentItem: ColumnLayout {
        spacing: 12

        RowLayout {
            spacing: 10
            IconLabel { icon: Theme.iconFile; size: 18; color: Theme.warning }
            Text {
                Layout.fillWidth: true
                text: qsTr("%n file(s) of the show are missing", "", Core.missingFiles.length)
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
                wrapMode: Text.WordWrap
            }
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("The show plays without them. If it comes from another computer, choose the folder where they are: they are looked for by name, also in its subfolders.")
            color: Theme.textDim
        }

        // the files, the name first and the folder below
        ListView {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(contentHeight, 220)
            clip: true
            spacing: 6
            model: Core.missingFiles
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            delegate: Column {
                required property string modelData
                width: ListView.view.width
                Text {
                    width: parent.width
                    text: modelData.split(/[\\/]/).pop()
                    color: Theme.text
                    font.weight: Font.DemiBold
                    elide: Text.ElideMiddle
                }
                Text {
                    width: parent.width
                    text: modelData
                    color: Theme.textFaint
                    font.pixelSize: Theme.fontSmall
                    elide: Text.ElideMiddle
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Close"); onClicked: dialog.close() }
            AppButton {
                text: qsTr("Look in a folder…")
                iconGlyph: Theme.iconFolderOpen
                primary: true
                onClicked: {
                    // (from the folder of the show)
                    folderDialog.currentFolder = Core.dialogFolder("show")
                    folderDialog.open()
                }
            }
        }
    }
}
