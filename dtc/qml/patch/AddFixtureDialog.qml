/*
  DMX Total Control
  AddFixtureDialog.qml

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

/** Browse the fixture library (manufacturer > model > mode) and patch */
Popup {
    id: dialog

    property string manufacturer: ""
    property string model: ""
    property var modes: []
    property var allManufacturers: []
    /** Why the last patch did nothing */
    property string error: ""

    readonly property bool isDimmer: model === "Generic Dimmer"
    readonly property int channelsPerFixture: isDimmer ? dimmerChannels.value
                                                       : (modeCombo.currentIndex >= 0 && modes.length > 0 ? modes[modeCombo.currentIndex].channels : 0)

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(980, parent.width - 80)
    height: Math.min(640, parent.height - 80)
    modal: true
    padding: 18

    function refreshAddress() {
        if (channelsPerFixture > 0) {
            let free = Core.patch.nextFreeAddress(universeSpin.value - 1, channelsPerFixture)
            if (free > 0)
                addressSpin.value = free
        }
    }

    onOpened: {
        error = ""
        allManufacturers = Core.patch.manufacturers()
        if (manufacturer.length === 0)
            manufacturer = "Generic"
        search.text = ""
        search.forceActiveFocus()
    }
    onChannelsPerFixtureChanged: refreshAddress()

    /** Where the fixtures go, the way addFixtures places them: a fixture
     *  that doesn't fit in what is left of a universe starts the next one */
    function rangeText() {
        const ch = channelsPerFixture
        if (ch <= 0 || ch > 512)
            return ""
        let uni = universeSpin.value, addr = addressSpin.value - 1
        let firstUni = -1, first = 0, lastUni = uni, last = 0
        for (let i = 0; i < quantitySpin.value; i++) {
            if (addr + ch > 512) {
                uni++
                addr = 0
            }
            if (firstUni < 0) {
                firstUni = uni
                first = addr
            }
            lastUni = uni
            last = addr + ch - 1
            addr += ch + gapSpin.value
        }
        if (firstUni === lastUni && firstUni === universeSpin.value)
            return qsTr("Uses addresses %1 to %2").arg(first + 1).arg(last + 1)
        return qsTr("Uses universe %1, address %2, to universe %3, address %4")
                .arg(firstUni).arg(first + 1).arg(lastUni).arg(last + 1)
    }

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surface
        border.color: Theme.borderStrong
    }

    contentItem: ColumnLayout {
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Text {
                text: qsTr("Add fixtures")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
            }
            Item { Layout.fillWidth: true }
            TextField {
                id: search
                Layout.preferredWidth: 280
                placeholderText: qsTr("Search manufacturer…")
                color: Theme.text
                background: Rectangle { radius: Theme.radiusSmall; color: Theme.background; border.color: search.activeFocus ? Theme.accent : Theme.border }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            // manufacturers
            Panel {
                Layout.preferredWidth: 260
                Layout.fillHeight: true
                title: qsTr("Manufacturer")

                ListView {
                    id: manufList
                    anchors { fill: parent; margins: 4 }
                    clip: true
                    model: dialog.allManufacturers.filter(m => m.toLowerCase().indexOf(search.text.toLowerCase()) >= 0)
                    ScrollBar.vertical: ScrollBar {}
                    delegate: ItemDelegate {
                        required property string modelData
                        width: manufList.width
                        height: 32
                        text: modelData
                        highlighted: modelData === dialog.manufacturer
                        onClicked: { dialog.manufacturer = modelData; dialog.model = ""; dialog.modes = [] }
                    }
                }
            }

            // models
            Panel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: qsTr("Model")

                ListView {
                    id: modelList
                    anchors { fill: parent; margins: 4 }
                    clip: true
                    model: dialog.manufacturer.length ? Core.patch.models(dialog.manufacturer) : []
                    ScrollBar.vertical: ScrollBar {}
                    delegate: ItemDelegate {
                        required property string modelData
                        width: modelList.width
                        height: 32
                        text: modelData
                        highlighted: modelData === dialog.model
                        onClicked: {
                            dialog.model = modelData
                            dialog.modes = Core.patch.modes(dialog.manufacturer, modelData)
                            modeCombo.currentIndex = 0
                            nameField.text = modelData === "Generic Dimmer" ? qsTr("Dimmer") : modelData
                        }
                    }
                    Text {
                        anchors.centerIn: parent
                        visible: modelList.count === 0
                        text: qsTr("Choose a manufacturer")
                        color: Theme.textFaint
                    }
                }
            }

            // settings
            Panel {
                Layout.preferredWidth: 280
                Layout.fillHeight: true
                title: qsTr("Patch")
                enabled: dialog.model.length > 0
                opacity: enabled ? 1 : 0.5

                GridLayout {
                    anchors { left: parent.left; right: parent.right; top: parent.top; margins: 12 }
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

                    Text { text: qsTr("Mode"); color: Theme.textDim; visible: !dialog.isDimmer }
                    ComboBox {
                        id: modeCombo
                        Layout.fillWidth: true
                        visible: !dialog.isDimmer
                        model: dialog.modes.map(m => m.name + " (" + m.channels + " ch)")
                    }

                    Text { text: qsTr("Channels"); color: Theme.textDim; visible: dialog.isDimmer }
                    SpinBox {
                        id: dimmerChannels
                        Layout.fillWidth: true
                        visible: dialog.isDimmer
                        from: 1; to: 512; value: 1
                        editable: true
                    }

                    Text { text: qsTr("Quantity"); color: Theme.textDim }
                    SpinBox { id: quantitySpin; Layout.fillWidth: true; from: 1; to: 256; value: 1; editable: true }

                    Text { text: qsTr("Universe"); color: Theme.textDim }
                    SpinBox {
                        id: universeSpin
                        Layout.fillWidth: true
                        from: 1; to: Math.max(1, Core.io.universes.length); value: 1
                        editable: true
                        onValueModified: dialog.refreshAddress()
                    }

                    Text { text: qsTr("Address"); color: Theme.textDim }
                    SpinBox { id: addressSpin; Layout.fillWidth: true; from: 1; to: 512; value: 1; editable: true }

                    Text { text: qsTr("Gap"); color: Theme.textDim }
                    SpinBox { id: gapSpin; Layout.fillWidth: true; from: 0; to: 511; value: 0; editable: true }

                    // ask whether the new fixtures make a group
                    CheckBox {
                        id: groupCheck
                        Layout.columnSpan: 2
                        visible: quantitySpin.value > 1
                        checked: true
                        text: qsTr("Make a group with them")
                    }
                    Text { text: qsTr("Group"); color: Theme.textDim; visible: groupCheck.visible && groupCheck.checked }
                    TextField {
                        id: groupField
                        Layout.fillWidth: true
                        visible: groupCheck.visible && groupCheck.checked
                        placeholderText: nameField.text
                        color: Theme.text
                        background: Rectangle { radius: Theme.radiusSmall; color: Theme.background; border.color: groupField.activeFocus ? Theme.accent : Theme.border }
                    }
                }

                Text {
                    anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: 12 }
                    wrapMode: Text.WordWrap
                    visible: dialog.channelsPerFixture > 0
                    text: dialog.rangeText()
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            // why nothing was patched (the addresses are taken...), else the credits
            Text {
                Layout.fillWidth: true
                text: dialog.error.length > 0 ? dialog.error : qsTr("Built-in fixture library")
                color: dialog.error.length > 0 ? Theme.danger : Theme.textFaint
                font.pixelSize: Theme.fontSmall
                wrapMode: Text.WordWrap
            }
            AppButton { text: qsTr("Close"); onClicked: dialog.close() }
            AppButton {
                text: quantitySpin.value > 1 ? qsTr("Patch %1 fixtures").arg(quantitySpin.value) : qsTr("Patch fixture")
                iconGlyph: Theme.iconPlus
                primary: true
                enabled: dialog.model.length > 0 && dialog.channelsPerFixture > 0
                onClicked: {
                    commitEdits()
                    let modeName = dialog.isDimmer ? "" : dialog.modes[modeCombo.currentIndex].name
                    let n = Core.patch.addFixtures(dialog.manufacturer, dialog.model, modeName, nameField.text,
                                                   universeSpin.value - 1, addressSpin.value, quantitySpin.value,
                                                   gapSpin.value, dimmerChannels.value,
                                                   groupCheck.visible && groupCheck.checked
                                                   ? (groupField.text.trim().length ? groupField.text.trim() : nameField.text) : "")
                    dialog.error = n > 0 ? "" : Core.patch.lastError()
                    if (n > 0)
                        dialog.refreshAddress()
                }
            }
        }
    }
}
