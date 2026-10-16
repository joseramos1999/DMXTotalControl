/*
  DMX Total Control
  FixtureEditorPage.qml

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

/**
 * Fixture creator: channels (from presets or custom), and modes that use
 * them in a given order. Definitions are saved as QLC+ .qxf files in the
 * user fixtures folder and are immediately available in the Patch.
 */
Item {
    id: page

    property var draft: Core.library.newDefinition()
    property int currentMode: 0
    property bool dirty: false

    readonly property var presets: Core.library.channelPresets()
    readonly property var presetNames: [qsTr("Custom")].concat(presets.map(p => p.name))
    readonly property var groups: Core.library.channelGroups()
    readonly property var colours: ["", "Red", "Green", "Blue", "White", "Amber", "UV", "Cyan", "Magenta", "Yellow", "Lime", "Indigo"]

    signal message(string text, bool isError)

    /** Apply a change to a deep copy of the draft, so bindings update */
    function update(fn) {
        let d = JSON.parse(JSON.stringify(draft))
        fn(d)
        draft = d
        dirty = true
    }

    function load(def) {
        draft = def
        currentMode = 0
        dirty = false
    }

    /** What is being typed in a field goes into the draft first: the
     *  buttons and the list take no focus, and a field gives its text only
     *  on Enter or when it loses the focus */
    function commitEdits() { focusSink.forceActiveFocus() }

    /** Another definition ($make returns it): the changes not saved are
     *  asked about first. $changed: it is not saved as it is (a copy) */
    function open(make, changed) {
        commitEdits()
        if (dirty) {
            discardDialog.make = make
            discardDialog.changed = !!changed
            discardDialog.open()
            return
        }
        load(make())
        dirty = !!changed
    }

    function presetIndex(preset) {
        let i = presets.findIndex(p => p.preset === preset)
        return i < 0 ? 0 : i + 1
    }

    function setPreset(chIndex, comboIndex) {
        update(d => {
            let ch = d.channels[chIndex]
            let previous = presets.find(p => p.preset === ch.preset)
            if (comboIndex === 0) {
                ch.preset = "Custom"
                if (!ch.capabilities || ch.capabilities.length === 0)
                    ch.capabilities = [{ min: 0, max: 255, name: ch.name }]
                return
            }
            let p = presets[comboIndex - 1]
            // keep a custom name, replace the default one
            if (!ch.name || (previous && ch.name === previous.name) || ch.name === "Channel")
                ch.name = p.name
            ch.preset = p.preset
            ch.group = p.group
            ch.fine = p.fine
        })
    }

    function addChannel(presetIdx) {
        update(d => {
            let base = presetIdx >= 0 ? presets[presetIdx]
                                      : { preset: "Custom", name: qsTr("Channel %1").arg(d.channels.length + 1), group: "Effect", fine: false }
            let name = base.name
            let n = 2
            while (d.channels.some(c => c.name.toLowerCase() === name.toLowerCase()))
                name = base.name + " " + n++
            d.channels.push({ name: name, preset: base.preset, group: base.group, colour: "",
                              fine: base.fine, defaultValue: 0,
                              capabilities: base.preset === "Custom" ? [{ min: 0, max: 255, name: name }] : [] })
            // new channels go at the end of the current mode
            if (d.modes.length > 0)
                d.modes[Math.min(currentMode, d.modes.length - 1)].channels.push(d.channels.length - 1)
        })
    }

    function removeChannel(index) {
        commitEdits()
        update(d => {
            d.channels.splice(index, 1)
            for (let m of d.modes)
                m.channels = m.channels.filter(c => c !== index).map(c => c > index ? c - 1 : c)
        })
    }

    function save() {
        commitEdits()
        let err = Core.library.saveDefinition(draft)
        if (err.length > 0) {
            page.message(err, true)
        } else {
            dirty = false
            update(d => d.isUser = true)
            dirty = false
            page.message(qsTr("Fixture saved. It is available in the Patch, under %1.").arg(draft.manufacturer), false)
        }
    }

    RowLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 12

        // user fixtures
        Panel {
            Layout.preferredWidth: 270
            Layout.fillHeight: true
            title: qsTr("My fixtures")

            ColumnLayout {
                anchors { fill: parent; margins: 8 }
                spacing: 8

                AppButton {
                    Layout.fillWidth: true
                    text: qsTr("New fixture")
                    iconGlyph: Theme.iconPlus
                    primary: true
                    onClicked: page.open(() => Core.library.newDefinition())
                }
                AppButton {
                    Layout.fillWidth: true
                    text: qsTr("Start from the library…")
                    iconGlyph: Theme.iconCopy
                    onClicked: libraryPicker.open()
                }

                ListView {
                    id: userList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 3
                    model: Core.library.userFixtures
                    ScrollBar.vertical: ScrollBar {}

                    delegate: Rectangle {
                        required property var modelData
                        width: userList.width
                        height: 42
                        radius: Theme.radiusSmall
                        readonly property bool current: page.draft.manufacturer === modelData.manufacturer &&
                                                        page.draft.model === modelData.model
                        color: current ? Qt.alpha(Theme.accent, 0.2) : (um.containsMouse ? Theme.surfaceHover : "transparent")
                        Column {
                            anchors { left: parent.left; leftMargin: 8; right: parent.right; rightMargin: 8; verticalCenter: parent.verticalCenter }
                            Text { width: parent.width; text: modelData.model; color: Theme.text; elide: Text.ElideRight; font.weight: Font.DemiBold }
                            Text { width: parent.width; text: modelData.manufacturer; color: Theme.textFaint; elide: Text.ElideRight; font.pixelSize: Theme.fontSmall }
                        }
                        MouseArea {
                            id: um
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: page.open(() => Core.library.loadDefinition(modelData.manufacturer, modelData.model))
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        width: parent.width - 20
                        visible: userList.count === 0
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        text: qsTr("The fixtures you create appear here")
                        color: Theme.textFaint
                        font.pixelSize: Theme.fontSmall
                    }
                }
            }
        }

        // editor
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Column {
                    Text {
                        text: page.draft.model && page.draft.model.length ? page.draft.model : qsTr("New fixture")
                        color: Theme.text
                        font.pixelSize: Theme.fontTitle
                        font.weight: Font.Bold
                    }
                    Text {
                        readonly property bool inUserList: Core.library.userFixtures.some(f => f.manufacturer === page.draft.manufacturer && f.model === page.draft.model)
                        text: page.dirty ? qsTr("Unsaved changes")
                                         : (inUserList ? qsTr("Saved in your fixtures") : qsTr("Not saved yet"))
                        color: page.dirty ? Theme.warning : Theme.textDim
                        font.pixelSize: Theme.fontSmall
                    }
                }
                Item { Layout.fillWidth: true }
                AppButton {
                    text: qsTr("Delete")
                    iconGlyph: Theme.iconTrash
                    visible: page.draft.isUser && Core.library.userFixtures.some(f => f.manufacturer === page.draft.manufacturer && f.model === page.draft.model)
                    onClicked: {
                        if (Core.library.deleteUserDefinition(page.draft.manufacturer, page.draft.model))
                            page.load(Core.library.newDefinition())
                    }
                }
                AppButton {
                    text: qsTr("Save fixture")
                    iconGlyph: Theme.iconSave
                    primary: true
                    onClicked: page.save()
                }
            }

            // identity
            Panel {
                Layout.fillWidth: true
                Layout.preferredHeight: 96
                title: qsTr("Fixture")

                RowLayout {
                    anchors { fill: parent; margins: 10 }
                    spacing: 10
                    Text { text: qsTr("Manufacturer"); color: Theme.textDim }
                    TextField {
                        Layout.fillWidth: true
                        text: page.draft.manufacturer
                        placeholderText: qsTr("e.g. Generic, or the brand")
                        color: Theme.text
                        onEditingFinished: if (text !== page.draft.manufacturer) page.update(d => d.manufacturer = text)
                    }
                    Text { text: qsTr("Model"); color: Theme.textDim }
                    TextField {
                        Layout.fillWidth: true
                        text: page.draft.model
                        placeholderText: qsTr("e.g. PAR 18x10W RGBWA")
                        color: Theme.text
                        onEditingFinished: if (text !== page.draft.model) page.update(d => d.model = text)
                    }
                    Text { text: qsTr("Type"); color: Theme.textDim }
                    ComboBox {
                        Layout.preferredWidth: 170
                        model: Core.library.fixtureTypes()
                        currentIndex: Math.max(0, model.indexOf(page.draft.type))
                        onActivated: (i) => page.update(d => d.type = model[i])
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 12

                // channels
                Panel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    title: qsTr("Channels (%1)").arg(page.draft.channels.length)
                    headerItems: [
                        AppButton { text: qsTr("Add"); iconGlyph: Theme.iconPlus; flat: true; implicitHeight: 28; onClicked: addMenu.popup() }
                    ]

                    ListView {
                        id: channelList
                        anchors { fill: parent; margins: 6 }
                        clip: true
                        spacing: 4
                        model: page.draft.channels
                        ScrollBar.vertical: ScrollBar {}

                        delegate: Rectangle {
                            id: chRow
                            required property var modelData
                            required property int index
                            readonly property bool custom: modelData.preset === "Custom"
                            width: channelList.width
                            height: custom ? 82 : 46
                            radius: Theme.radiusSmall
                            color: Theme.surfaceRaised
                            border.color: Theme.border

                            ColumnLayout {
                                anchors { fill: parent; margins: 6 }
                                spacing: 4

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6
                                    Text { text: chRow.index + 1; color: Theme.textFaint; font.family: Theme.monoFamily; Layout.preferredWidth: 22 }
                                    TextField {
                                        Layout.fillWidth: true
                                        text: chRow.modelData.name
                                        color: Theme.text
                                        onEditingFinished: if (text !== chRow.modelData.name) page.update(d => d.channels[chRow.index].name = text)
                                    }
                                    ComboBox {
                                        Layout.preferredWidth: 200
                                        model: page.presetNames
                                        currentIndex: page.presetIndex(chRow.modelData.preset)
                                        onActivated: (i) => page.setPreset(chRow.index, i)
                                    }
                                    AppButton {
                                        iconGlyph: Theme.iconTrash
                                        flat: true
                                        onClicked: page.removeChannel(chRow.index)
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    visible: chRow.custom
                                    spacing: 6
                                    Item { Layout.preferredWidth: 22 }
                                    Text { text: qsTr("Group"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                                    ComboBox {
                                        Layout.preferredWidth: 130
                                        model: page.groups
                                        currentIndex: Math.max(0, page.groups.indexOf(chRow.modelData.group))
                                        onActivated: (i) => page.update(d => d.channels[chRow.index].group = page.groups[i])
                                    }
                                    Text { text: qsTr("Colour"); color: Theme.textDim; font.pixelSize: Theme.fontSmall; visible: chRow.modelData.group === "Intensity" }
                                    ComboBox {
                                        Layout.preferredWidth: 110
                                        visible: chRow.modelData.group === "Intensity"
                                        model: page.colours.map(c => c.length ? c : qsTr("None"))
                                        currentIndex: Math.max(0, page.colours.indexOf(chRow.modelData.colour))
                                        onActivated: (i) => page.update(d => d.channels[chRow.index].colour = page.colours[i])
                                    }
                                    CheckBox {
                                        text: qsTr("Fine")
                                        checked: !!chRow.modelData.fine
                                        onToggled: page.update(d => d.channels[chRow.index].fine = checked)
                                    }
                                    Item { Layout.fillWidth: true }
                                    AppButton {
                                        text: qsTr("Ranges (%1)").arg(chRow.modelData.capabilities ? chRow.modelData.capabilities.length : 0)
                                        flat: true
                                        onClicked: {
                                            // (the row is made again when the draft changes: its index is kept)
                                            const i = chRow.index
                                            page.commitEdits()
                                            const ch = page.draft.channels[i]
                                            capDialog.edit(ch.name, ch.capabilities,
                                                           (caps) => page.update(d => d.channels[i].capabilities = caps))
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // modes
                Panel {
                    Layout.preferredWidth: 360
                    Layout.fillHeight: true
                    title: qsTr("Modes")
                    headerItems: [
                        AppButton {
                            text: qsTr("Add"); iconGlyph: Theme.iconPlus; flat: true; implicitHeight: 28
                            onClicked: {
                                page.commitEdits()
                                page.update(d => d.modes.push({ name: qsTr("Mode %1").arg(d.modes.length + 1),
                                                                channels: d.channels.map((c, i) => i) }))
                                page.currentMode = page.draft.modes.length - 1
                            }
                        }
                    ]

                    ColumnLayout {
                        anchors { fill: parent; margins: 8 }
                        spacing: 8

                        // mode tabs
                        Flow {
                            Layout.fillWidth: true
                            spacing: 6
                            Repeater {
                                model: page.draft.modes
                                delegate: AppButton {
                                    required property var modelData
                                    required property int index
                                    text: modelData.name + " (" + modelData.channels.length + ")"
                                    implicitHeight: 28
                                    highlighted: index === page.currentMode
                                    onClicked: { const i = index; page.commitEdits(); page.currentMode = i }
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            visible: page.draft.modes.length > 0
                            Text { text: qsTr("Name"); color: Theme.textDim }
                            TextField {
                                Layout.fillWidth: true
                                text: page.draft.modes.length ? page.draft.modes[Math.min(page.currentMode, page.draft.modes.length - 1)].name : ""
                                color: Theme.text
                                onEditingFinished: if (page.draft.modes[page.currentMode] && text !== page.draft.modes[page.currentMode].name)
                                                       page.update(d => d.modes[page.currentMode].name = text)
                            }
                            AppButton {
                                iconGlyph: Theme.iconTrash
                                flat: true
                                enabled: page.draft.modes.length > 1
                                onClicked: { page.commitEdits(); let m = page.currentMode; page.currentMode = 0; page.update(d => d.modes.splice(m, 1)) }
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: qsTr("DMX order of this mode. Tick the channels it uses and move them to the right position.")
                            color: Theme.textFaint
                            font.pixelSize: Theme.fontSmall
                        }

                        ListView {
                            id: modeChannels
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            spacing: 2
                            ScrollBar.vertical: ScrollBar {}

                            readonly property var mode: page.draft.modes.length
                                ? page.draft.modes[Math.min(page.currentMode, page.draft.modes.length - 1)] : ({ channels: [] })
                            // used channels in DMX order, then the unused ones
                            model: mode.channels.concat(page.draft.channels.map((c, i) => i).filter(i => mode.channels.indexOf(i) < 0))

                            delegate: Rectangle {
                                id: mcRow
                                required property int modelData
                                required property int index
                                readonly property int position: modeChannels.mode.channels.indexOf(modelData)
                                readonly property bool used: position >= 0
                                width: modeChannels.width
                                height: 34
                                radius: Theme.radiusSmall
                                color: used ? Theme.surfaceRaised : "transparent"

                                RowLayout {
                                    anchors { fill: parent; leftMargin: 4; rightMargin: 4 }
                                    spacing: 4
                                    CheckBox {
                                        checked: mcRow.used
                                        onToggled: page.update(d => {
                                            let chs = d.modes[page.currentMode].channels
                                            if (mcRow.used) chs.splice(chs.indexOf(mcRow.modelData), 1)
                                            else chs.push(mcRow.modelData)
                                        })
                                    }
                                    Text {
                                        Layout.preferredWidth: 26
                                        text: mcRow.used ? (mcRow.position + 1) : ""
                                        color: Theme.accent
                                        font.family: Theme.monoFamily
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: page.draft.channels[mcRow.modelData] ? page.draft.channels[mcRow.modelData].name : ""
                                        color: mcRow.used ? Theme.text : Theme.textFaint
                                        elide: Text.ElideRight
                                    }
                                    AppButton {
                                        text: "▲"; flat: true; implicitHeight: 26
                                        visible: mcRow.used
                                        enabled: mcRow.position > 0
                                        onClicked: {
                                            const from = mcRow.position
                                            page.commitEdits()
                                            page.update(d => {
                                                let chs = d.modes[page.currentMode].channels
                                                chs.splice(from - 1, 0, chs.splice(from, 1)[0])
                                            })
                                        }
                                    }
                                    AppButton {
                                        text: "▼"; flat: true; implicitHeight: 26
                                        visible: mcRow.used
                                        enabled: mcRow.position < modeChannels.mode.channels.length - 1
                                        onClicked: {
                                            const from = mcRow.position
                                            page.commitEdits()
                                            page.update(d => {
                                                let chs = d.modes[page.currentMode].channels
                                                chs.splice(from + 1, 0, chs.splice(from, 1)[0])
                                            })
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // quick add: the most common presets first, then everything
    AppMenu {
        id: addMenu
        Instantiator {
            model: ["IntensityMasterDimmer", "IntensityRed", "IntensityGreen", "IntensityBlue", "IntensityWhite",
                    "IntensityAmber", "IntensityUV", "PositionPan", "PositionPanFine", "PositionTilt",
                    "PositionTiltFine", "ShutterStrobeSlowFast", "ColorWheel", "GoboWheel", "SpeedPanTiltFastSlow"]
            delegate: MenuItem {
                required property string modelData
                readonly property int presetIdx: page.presets.findIndex(p => p.preset === modelData)
                text: presetIdx >= 0 ? page.presets[presetIdx].name : modelData
                onTriggered: page.addChannel(presetIdx)
            }
            onObjectAdded: (index, object) => addMenu.insertItem(index, object)
            onObjectRemoved: (index, object) => addMenu.removeItem(object)
        }
        MenuSeparator {}
        MenuItem { text: qsTr("Custom channel"); onTriggered: page.addChannel(-1) }
    }

    CapabilitiesDialog { id: capDialog }
    LibraryPicker {
        id: libraryPicker
        onPicked: (manufacturer, model) => page.open(() => {
            let def = Core.library.loadDefinition(manufacturer, model)
            def.isUser = false
            def.model = def.model + " " + qsTr("(copy)")
            return def
        }, true)
    }

    // takes the focus from a field, so that it gives its text (commitEdits)
    Item { id: focusSink }

    // another fixture opened over changes not saved
    Popup {
        id: discardDialog
        property var make: null
        property bool changed: false
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(460, parent.width - 80)
        modal: true
        padding: 18
        background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

        contentItem: ColumnLayout {
            spacing: 14
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("“%1” has changes that are not saved. Discard them?")
                        .arg(page.draft.model && page.draft.model.length ? page.draft.model : qsTr("New fixture"))
                color: Theme.text
                font.pixelSize: Theme.fontLarge
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton { text: qsTr("Keep editing"); onClicked: discardDialog.close() }
                AppButton {
                    text: qsTr("Discard changes")
                    accentColor: Theme.danger
                    primary: true
                    onClicked: {
                        discardDialog.close()
                        page.load(discardDialog.make())
                        page.dirty = discardDialog.changed
                    }
                }
            }
        }
    }
}
