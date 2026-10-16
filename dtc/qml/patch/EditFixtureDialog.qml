/*
  DMX Total Control
  EditFixtureDialog.qml

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

/** Edit name, mode, universe and address of a patched fixture */
Popup {
    id: dialog

    property var fixture: ({})
    property var modes: []

    function edit(fx) {
        fixture = fx
        modes = fx.editableMode ? Core.patch.modes(fx.manufacturer, fx.model) : []
        nameField.text = fx.name
        modeCombo.currentIndex = Math.max(0, modes.findIndex(m => m.name === fx.mode))
        universeSpin.value = fx.universe + 1
        addressSpin.value = fx.address
        numberSpin.value = fx.number || 1
        errorText.text = ""
        open()
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 420
    modal: true
    padding: 18

    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    contentItem: ColumnLayout {
        spacing: 12

        Text {
            text: qsTr("Edit fixture")
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
        }
        Text {
            visible: dialog.fixture.manufacturer !== undefined && dialog.fixture.manufacturer.length > 0
            text: (dialog.fixture.manufacturer || "") + " — " + (dialog.fixture.model || "")
            color: Theme.textDim
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 10
            rowSpacing: 10

            Text { text: qsTr("Name"); color: Theme.textDim }
            TextField {
                id: nameField
                Layout.fillWidth: true
                color: Theme.text
                background: Rectangle { radius: Theme.radiusSmall; color: Theme.background; border.color: nameField.activeFocus ? Theme.accent : Theme.border }
            }

            Text { text: qsTr("Mode"); color: Theme.textDim; visible: dialog.modes.length > 0 }
            ComboBox {
                id: modeCombo
                Layout.fillWidth: true
                visible: dialog.modes.length > 0
                model: dialog.modes.map(m => m.name + " (" + m.channels + " ch)")
            }

            Text { text: qsTr("Universe"); color: Theme.textDim }
            SpinBox { id: universeSpin; Layout.fillWidth: true; from: 1; to: Math.max(1, Core.io.universes.length); editable: true }

            Text { text: qsTr("Address"); color: Theme.textDim }
            SpinBox { id: addressSpin; Layout.fillWidth: true; from: 1; to: 512; editable: true }

            Text { text: qsTr("Number"); color: Theme.textDim }
            SpinBox {
                id: numberSpin
                Layout.fillWidth: true
                from: 1; to: 99999; editable: true
                ToolTip.text: qsTr("For the command line of Program: 1 THRU 8 @ FULL")
                ToolTip.visible: hovered
                ToolTip.delay: 500
            }
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
                    let mode = dialog.modes.length > 0 ? dialog.modes[modeCombo.currentIndex].name : ""
                    let err = Core.patch.updateFixture(dialog.fixture.id, nameField.text, mode,
                                                       universeSpin.value - 1, addressSpin.value)
                    if (err.length === 0 && numberSpin.value !== dialog.fixture.number)
                        err = Core.patch.setFixtureNumber(dialog.fixture.id, numberSpin.value)
                    if (err.length > 0)
                        errorText.text = err
                    else
                        dialog.close()
                }
            }
        }
    }
}
