/*
  DMX Total Control
  CapabilitiesDialog.qml

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

/** Edit the value ranges of a custom channel: edit(channelName, caps, callback) */
Popup {
    id: dialog

    property string channelName: ""
    property var caps: []
    property var callback: null

    function edit(name, capabilities, cb) {
        channelName = name
        caps = JSON.parse(JSON.stringify(capabilities || []))
        callback = cb
        open()
    }

    function setCap(i, key, value) {
        let c = JSON.parse(JSON.stringify(caps))
        c[i][key] = value
        caps = c
    }

    function addRange() {
        let c = JSON.parse(JSON.stringify(caps))
        let start = c.length ? Math.min(255, c[c.length - 1].max + 1) : 0
        c.push({ min: start, max: 255, name: "" })
        caps = c
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 520
    height: Math.min(560, parent.height - 80)
    modal: true
    padding: 18

    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    contentItem: ColumnLayout {
        spacing: 10

        Text {
            text: qsTr("Value ranges of %1").arg(dialog.channelName)
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Describe what each DMX range does, e.g. 0-7 Closed, 8-255 Strobe. Ranges can't overlap.")
            color: Theme.textDim
            font.pixelSize: Theme.fontSmall
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: dialog.caps
            ScrollBar.vertical: ScrollBar {}

            delegate: RowLayout {
                required property var modelData
                required property int index
                width: list.width
                spacing: 6

                SpinBox { from: 0; to: 255; editable: true; value: modelData.min; Layout.preferredWidth: 110
                          onValueModified: dialog.setCap(index, "min", value) }
                Text { text: "–"; color: Theme.textDim }
                SpinBox { from: 0; to: 255; editable: true; value: modelData.max; Layout.preferredWidth: 110
                          onValueModified: dialog.setCap(index, "max", value) }
                TextField {
                    Layout.fillWidth: true
                    text: modelData.name
                    placeholderText: qsTr("Description")
                    color: Theme.text
                    onEditingFinished: dialog.setCap(index, "name", text)
                }
                AppButton {
                    iconGlyph: Theme.iconTrash
                    flat: true
                    onClicked: { const i = index; commitEdits(); let c = dialog.caps.slice(); c.splice(i, 1); dialog.caps = c }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            AppButton { text: qsTr("Add range"); iconGlyph: Theme.iconPlus; onClicked: { commitEdits(); dialog.addRange() } }
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Cancel"); onClicked: dialog.close() }
            AppButton {
                text: qsTr("OK")
                primary: true
                onClicked: {
                    // (the range being typed first)
                    commitEdits()
                    if (dialog.callback) dialog.callback(dialog.caps)
                    dialog.close()
                }
            }
        }
    }
}
