/*
  DMX Total Control
  ReaddressDialog.qml

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

/** Give consecutive addresses to the selected fixtures */
Popup {
    id: dialog

    property var fixtureIds: []

    function edit(ids, universe, address) {
        fixtureIds = ids
        universeSpin.value = universe + 1
        addressSpin.value = address
        gapSpin.value = 0
        errorText.text = ""
        open()
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 380
    modal: true
    padding: 18

    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    contentItem: ColumnLayout {
        spacing: 12

        Text {
            text: qsTr("Re-address %n fixture(s)", "", dialog.fixtureIds.length)
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("The fixtures get consecutive addresses, in the order of the list.")
            color: Theme.textDim
            font.pixelSize: Theme.fontSmall
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 10
            rowSpacing: 10

            Text { text: qsTr("Universe"); color: Theme.textDim }
            SpinBox { id: universeSpin; Layout.fillWidth: true; from: 1; to: Math.max(1, Core.io.universes.length); editable: true }
            Text { text: qsTr("Start address"); color: Theme.textDim }
            SpinBox { id: addressSpin; Layout.fillWidth: true; from: 1; to: 512; editable: true }
            Text { text: qsTr("Gap"); color: Theme.textDim }
            SpinBox { id: gapSpin; Layout.fillWidth: true; from: 0; to: 511; editable: true }
        }

        Text {
            id: errorText
            Layout.fillWidth: true
            visible: text.length > 0
            wrapMode: Text.WordWrap
            color: Theme.danger
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Cancel"); onClicked: dialog.close() }
            AppButton {
                text: qsTr("Apply")
                primary: true
                onClicked: {
                    commitEdits()
                    let err = Core.patch.readdressFixtures(dialog.fixtureIds, universeSpin.value - 1,
                                                           addressSpin.value, gapSpin.value)
                    if (err.length > 0)
                        errorText.text = err
                    else
                        dialog.close()
                }
            }
        }
    }
}
