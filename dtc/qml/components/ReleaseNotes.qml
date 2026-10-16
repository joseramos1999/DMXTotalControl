/*
  DMX Total Control
  ReleaseNotes.qml

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

/** The notes of the new version (the text of its release on GitHub) */
Popup {
    id: popup

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(620, parent ? parent.width - 64 : 620)
    height: Math.min(520, parent ? parent.height - 96 : 520)
    modal: true
    padding: 18

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surface
        border.color: Theme.borderStrong
    }

    contentItem: ColumnLayout {
        spacing: 12

        Text {
            Layout.fillWidth: true
            text: Core.updates.prerelease ? qsTr("What's new in %1 (test version)").arg(Core.updates.latestVersion)
                                          : qsTr("What's new in %1").arg(Core.updates.latestVersion)
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
            elide: Text.ElideRight
        }
        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentHeight: notesText.implicitHeight
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}
            Text {
                id: notesText
                width: parent.width - 12
                text: Core.updates.notes.length > 0 ? Core.updates.notes : qsTr("This version has no notes.")
                textFormat: Text.MarkdownText
                wrapMode: Text.WordWrap
                color: Theme.text
                linkColor: Theme.accent
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            AppButton {
                visible: Core.updates.pageUrl.length > 0
                text: qsTr("Open on GitHub")
                iconGlyph: Theme.iconLink
                flat: true
                onClicked: Core.updates.openPage()
            }
            Item { Layout.fillWidth: true }
            AppButton {
                text: qsTr("Close")
                onClicked: popup.close()
            }
        }
    }
}
