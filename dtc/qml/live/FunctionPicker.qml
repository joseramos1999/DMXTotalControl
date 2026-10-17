/*
  DMX Total Control
  FunctionPicker.qml

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

/** Pick existing functions to add to a zone */
Popup {
    id: picker

    property Zone zoneObject

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 480
    height: Math.min(640, parent ? parent.height - 40 : 640)
    modal: true
    padding: 16

    onOpened: { browser.clearSelection(); browser.clearSearch() }

    // the scenes not in the group yet
    readonly property var available: {
        let inZone = (zoneObject ? zoneObject.functions : []).map(f => f.id)
        return Core.functions.filter(f => inZone.indexOf(f.id) < 0)
    }

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surface
        border.color: Theme.borderStrong
    }

    contentItem: ColumnLayout {
        spacing: 10

        Text {
            text: qsTr("Add scenes to %1").arg(picker.zoneObject ? picker.zoneObject.name : "")
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
        }

        SceneBrowser {
            id: browser
            Layout.fillWidth: true
            Layout.fillHeight: true
            functions: picker.available
            toggleSelect: true
            emptyText: Core.functions.length === 0
                       ? qsTr("There are no scenes yet. Create one on the Program page, or open a QLC+ show.")
                       : qsTr("Nothing left to add")
            onSceneDoubleClicked: (id) => picker.zoneObject.addFunction(id)
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Cancel"); onClicked: picker.close() }
            AppButton {
                text: qsTr("Add %1").arg(browser.selectedIds.length)
                primary: true
                enabled: browser.selectedIds.length > 0
                onClicked: {
                    // in the order they were picked
                    for (let id of browser.selectedIds)
                        picker.zoneObject.addFunction(id)
                    picker.close()
                }
            }
        }
    }
}
