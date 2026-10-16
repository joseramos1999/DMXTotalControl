/*
  DMX Total Control
  LibraryPicker.qml

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

/** Pick a fixture of the library: emits picked(manufacturer, model) */
Popup {
    id: picker

    property string manufacturer: ""
    property var allManufacturers: []

    signal picked(string manufacturer, string model)

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(760, parent.width - 80)
    height: Math.min(560, parent.height - 80)
    modal: true
    padding: 18

    onOpened: { allManufacturers = Core.patch.manufacturers(); search.text = ""; search.forceActiveFocus() }

    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    contentItem: ColumnLayout {
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Text {
                text: qsTr("Start from a library fixture")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
            }
            Item { Layout.fillWidth: true }
            TextField {
                id: search
                Layout.preferredWidth: 240
                placeholderText: qsTr("Search manufacturer…")
                color: Theme.text
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            Panel {
                Layout.preferredWidth: 260
                Layout.fillHeight: true
                title: qsTr("Manufacturer")
                ListView {
                    id: manufList
                    anchors { fill: parent; margins: 4 }
                    clip: true
                    model: picker.allManufacturers.filter(m => m.toLowerCase().indexOf(search.text.toLowerCase()) >= 0)
                    ScrollBar.vertical: ScrollBar {}
                    delegate: ItemDelegate {
                        required property string modelData
                        width: manufList.width
                        height: 32
                        text: modelData
                        highlighted: modelData === picker.manufacturer
                        onClicked: picker.manufacturer = modelData
                    }
                }
            }
            Panel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: qsTr("Model")
                ListView {
                    id: modelList
                    anchors { fill: parent; margins: 4 }
                    clip: true
                    model: picker.manufacturer.length
                           ? Core.patch.models(picker.manufacturer).filter(m => m !== "Generic Dimmer") : []
                    ScrollBar.vertical: ScrollBar {}
                    delegate: ItemDelegate {
                        required property string modelData
                        width: modelList.width
                        height: 32
                        text: modelData
                        onClicked: { picker.picked(picker.manufacturer, modelData); picker.close() }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Cancel"); onClicked: picker.close() }
        }
    }
}
