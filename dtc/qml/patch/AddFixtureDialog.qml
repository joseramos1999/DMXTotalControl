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

/** Browse the fixture library (manufacturer > model > mode) and patch; or,
 *  with pickOnly, choose a fixture and a mode (picked()) */
Popup {
    id: dialog

    property string manufacturer: ""
    property string model: ""
    property var modes: []
    property var allManufacturers: []
    /** Why the last patch did nothing */
    property string error: ""
    /** Only choose a fixture and its mode, for $pickFor (the patch import:
     *  a fixture type of a table, with $pickChannels channels) */
    property bool pickOnly: false
    property string pickFor: ""
    property int pickChannels: 0
    signal picked(string manufacturer, string model, string mode)

    readonly property bool isDimmer: model === "Generic Dimmer"
    readonly property int channelsPerFixture: isDimmer ? dimmerChannels.value
                                                       : (modeCombo.currentIndex >= 0 && modes.length > 0 ? modes[modeCombo.currentIndex].channels : 0)
    /** The modes of the other DMX breaks of the mode chosen (a GDTF fixture
     *  with two DMX inputs, or a part on a universe of its own): each one
     *  is patched as a fixture of its own, at its own address */
    readonly property var breakModes: {
        if (isDimmer || modeCombo.currentIndex < 0 || modeCombo.currentIndex >= modes.length)
            return []
        const prefix = modes[modeCombo.currentIndex].name + " (DMX break "
        return modes.filter(m => m.name.startsWith(prefix)).map(m => m.name)
    }

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

    /** Choose $name among the models of the manufacturer shown */
    function pickModel(name) {
        model = name
        modes = Core.patch.modes(manufacturer, name)
        // choosing for a table: its channel count first
        const same = pickOnly && pickChannels > 0 ? modes.findIndex(m => m.channels === pickChannels) : -1
        modeCombo.currentIndex = Math.max(0, same)
        nameField.text = name === "Generic Dimmer" ? qsTr("Dimmer") : name
    }
    /** Choose $name of $maker, ready to patch (also one just imported
     *  from GDTF-Share: the manufacturers are read again) */
    function choose(maker, name) {
        allManufacturers = Core.patch.manufacturers()
        search.text = ""
        manufacturer = maker
        pickModel(name)
        Qt.callLater(() => {
            manufList.positionViewAtIndex(allManufacturers.indexOf(maker), ListView.Contain)
            modelList.positionViewAtIndex(modelList.model.indexOf(name), ListView.Contain)
        })
    }
    /** The models of $maker, none chosen */
    function showManufacturer(maker) {
        allManufacturers = Core.patch.manufacturers()
        search.text = ""
        manufacturer = maker
        model = ""
        modes = []
        Qt.callLater(() => manufList.positionViewAtIndex(allManufacturers.indexOf(maker), ListView.Contain))
    }
    /** The mode $name of the model chosen, when it has it */
    function chooseMode(name) {
        const i = modes.findIndex(m => m.name === name)
        if (i >= 0)
            modeCombo.currentIndex = i
    }
    /** Developer option --patch-view share: the GDTF-Share browser over it */
    function openShare() {
        shareDialog.open()
    }

    // GDTF-Share: a fixture downloaded (or in the library already) is patched here
    GdtfShareDialog {
        id: shareDialog
        fromPatch: true
        onPatchWanted: (maker, name) => dialog.choose(maker, name)
    }

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
        return qsTr("Uses from universe %1, address %2, to universe %3, address %4")
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
                Layout.fillWidth: true
                text: dialog.pickOnly ? qsTr("Choose the fixture for “%1”").arg(dialog.pickFor) : qsTr("Add fixtures")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
                elide: Text.ElideRight
            }
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
                    // (read again after an import: a fixture of GDTF-Share joins the library)
                    model: dialog.manufacturer.length && Core.library.definitionsCount >= 0 ? Core.patch.models(dialog.manufacturer) : []
                    ScrollBar.vertical: ScrollBar {}
                    delegate: ItemDelegate {
                        required property string modelData
                        width: modelList.width
                        height: 32
                        text: modelData
                        highlighted: modelData === dialog.model
                        onClicked: dialog.pickModel(modelData)
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
                title: dialog.pickOnly ? qsTr("Mode") : qsTr("Patch")
                enabled: dialog.model.length > 0
                opacity: enabled ? 1 : 0.5

                GridLayout {
                    anchors { left: parent.left; right: parent.right; top: parent.top; margins: 12 }
                    columns: 2
                    columnSpacing: 10
                    rowSpacing: 10

                    Text { text: qsTr("Name"); color: Theme.textDim; visible: !dialog.pickOnly }
                    TextField {
                        id: nameField
                        Layout.fillWidth: true
                        visible: !dialog.pickOnly
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

                    Text { text: qsTr("Channels"); color: Theme.textDim; visible: dialog.isDimmer && !dialog.pickOnly }
                    SpinBox {
                        id: dimmerChannels
                        Layout.fillWidth: true
                        visible: dialog.isDimmer && !dialog.pickOnly
                        from: 1; to: 512; value: 1
                        editable: true
                    }

                    Text { text: qsTr("Quantity"); color: Theme.textDim; visible: !dialog.pickOnly }
                    SpinBox { id: quantitySpin; Layout.fillWidth: true; visible: !dialog.pickOnly; from: 1; to: 256; value: 1; editable: true }

                    Text { text: qsTr("Universe"); color: Theme.textDim; visible: !dialog.pickOnly }
                    SpinBox {
                        id: universeSpin
                        Layout.fillWidth: true
                        visible: !dialog.pickOnly
                        from: 1; to: Math.max(1, Core.io.universes.length); value: 1
                        editable: true
                        onValueModified: dialog.refreshAddress()
                    }

                    Text { text: qsTr("Address"); color: Theme.textDim; visible: !dialog.pickOnly }
                    SpinBox { id: addressSpin; Layout.fillWidth: true; visible: !dialog.pickOnly; from: 1; to: 512; value: 1; editable: true }

                    Text { text: qsTr("Gap"); color: Theme.textDim; visible: !dialog.pickOnly }
                    SpinBox { id: gapSpin; Layout.fillWidth: true; visible: !dialog.pickOnly; from: 0; to: 511; value: 0; editable: true }

                    // choosing for a table: how many channels its rows have
                    Text {
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        visible: dialog.pickOnly
                        wrapMode: Text.WordWrap
                        text: dialog.isDimmer ? qsTr("A generic dimmer with the channels of the table")
                              : dialog.pickChannels > 0 ? qsTr("The table says %n channel(s)", "", dialog.pickChannels)
                                                        : qsTr("The table doesn't say how many channels")
                        color: dialog.model.length > 0 && !dialog.isDimmer && dialog.pickChannels > 0 &&
                               dialog.channelsPerFixture !== dialog.pickChannels ? Theme.warning : Theme.textDim
                        font.pixelSize: Theme.fontSmall
                    }

                    // ask whether the new fixtures make a group
                    CheckBox {
                        id: groupCheck
                        Layout.columnSpan: 2
                        visible: !dialog.pickOnly && quantitySpin.value > 1
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

                ColumnLayout {
                    anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: 12 }
                    spacing: 6
                    Text {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        visible: !dialog.pickOnly && dialog.channelsPerFixture > 0
                        text: dialog.rangeText()
                        color: Theme.textDim
                        font.pixelSize: Theme.fontSmall
                    }
                    Text {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        visible: dialog.breakModes.length > 0
                        text: qsTr("This mode has %n more DMX break(s): patch the mode “%1” too, at its own address.", "",
                                   dialog.breakModes.length).arg(dialog.breakModes.join(", "))
                        color: Theme.warning
                        font.pixelSize: Theme.fontSmall
                    }
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
            AppButton {
                text: qsTr("Search GDTF-Share…")
                iconGlyph: Theme.iconSearch
                flat: true
                onClicked: shareDialog.open()
            }
            AppButton { text: dialog.pickOnly ? qsTr("Cancel") : qsTr("Close"); onClicked: dialog.close() }
            AppButton {
                visible: dialog.pickOnly
                text: qsTr("Use this fixture")
                iconGlyph: Theme.iconCheck
                primary: true
                enabled: dialog.model.length > 0 && (dialog.isDimmer || dialog.channelsPerFixture > 0)
                onClicked: {
                    dialog.picked(dialog.manufacturer, dialog.model, dialog.isDimmer ? "" : dialog.modes[modeCombo.currentIndex].name)
                    dialog.close()
                }
            }
            AppButton {
                visible: !dialog.pickOnly
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
