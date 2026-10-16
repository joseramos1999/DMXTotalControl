/*
  DMX Total Control
  MappingEditor.qml

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

/** One mapping: where it comes from, what it drives, how, and what it sends back */
Flickable {
    id: editor

    property var mapping: ({})
    readonly property int mid: mapping.id !== undefined ? mapping.id : -1
    readonly property bool button: Core.mapping.isButton(mapping.target || "scene")
    readonly property string source: mapping.source || "midi"
    readonly property var targetList: Core.mapping.targets()
    readonly property var targetInfo: targetList.find(t => t.target === mapping.target) || ({})
    readonly property bool learning: Core.mapping.pendingTarget.id !== undefined && Core.mapping.pendingTarget.id === mid

    // MIDI: the QLC+ channel holds the MIDI channel (top bits), CC 0-127 and notes 128-255
    readonly property int midiChannel: ((mapping.channel || 0) >> 12) + 1
    readonly property int midiCode: (mapping.channel || 0) & 0x0FFF
    readonly property bool midiNote: midiCode >= 128 && midiCode <= 255
    readonly property int midiNumber: midiNote ? midiCode - 128 : midiCode

    // the LEDs of the buttons
    readonly property bool ledsVisible: button && mapping.feedback !== false && (source === "midi" || source === "osc")
    readonly property var apcMiniColours: [
        { value: 0, color: "#20242c", name: qsTr("Off") },
        { value: 1, color: "#2fb344", name: qsTr("Green") },
        { value: 2, color: "#2fb344", name: qsTr("Green, blinking"), blink: true },
        { value: 3, color: "#e5484d", name: qsTr("Red") },
        { value: 4, color: "#e5484d", name: qsTr("Red, blinking"), blink: true },
        { value: 5, color: "#f5c524", name: qsTr("Yellow") },
        { value: 6, color: "#f5c524", name: qsTr("Yellow, blinking"), blink: true }
    ]
    readonly property var apcMini2Colours: [
        { value: 0, color: "#20242c", name: qsTr("Off") },
        { value: 3, color: "#ffffff", name: qsTr("White") },
        { value: 5, color: "#ff2020", name: qsTr("Red") },
        { value: 9, color: "#ff6a00", name: qsTr("Orange") },
        { value: 13, color: "#ffe600", name: qsTr("Yellow") },
        { value: 17, color: "#9dff00", name: qsTr("Lime") },
        { value: 21, color: "#00e040", name: qsTr("Green") },
        { value: 33, color: "#00ffaa", name: qsTr("Turquoise") },
        { value: 37, color: "#00d5ff", name: qsTr("Sky") },
        { value: 41, color: "#0088ff", name: qsTr("Ocean") },
        { value: 45, color: "#1f3dff", name: qsTr("Blue") },
        { value: 49, color: "#7a2bff", name: qsTr("Purple") },
        { value: 53, color: "#ff3df5", name: qsTr("Magenta") },
        { value: 57, color: "#ff2a8a", name: qsTr("Pink") }
    ]
    // the APC40 mkII has the same colours, and a dim blue for the cues that are there
    readonly property var apc40Colours: apcMini2Colours.slice(0, 11)
        .concat([{ value: 46, color: "#1a2a7a", name: qsTr("Dim blue") }]).concat(apcMini2Colours.slice(11))
    // the RGB pads (else buttons with one LED: on, blinking and off are numbers)
    readonly property bool rgbPad: midiNote && midiChannel === 1 &&
        (mapping.controller === "apcmini2" ? midiNumber <= 63
         : mapping.controller === "apc40mk2" ? midiNumber <= 39 || (midiNumber >= 82 && midiNumber <= 86) : false)
    readonly property var ledPalette: mapping.controller === "apcmini" ? apcMiniColours
                                 : mapping.controller === "apcmini2" && rgbPad ? apcMini2Colours
                                 : mapping.controller === "apc40mk2" && rgbPad ? apc40Colours : []
    // how the RGB pads light (the DTC channel of the LED values): the APC40 is solid only for now
    readonly property var ledChannels: mapping.controller === "apcmini2" ? [1, 7, 10, 14] : [1]
    readonly property var ledChannelNames: mapping.controller === "apcmini2"
        ? [qsTr("Dim"), qsTr("Full"), qsTr("Pulsing"), qsTr("Blinking")] : [qsTr("Solid")]

    // the playbacks: their pages (one more past the last, for a mapping made before the page)
    function pageChoices(follows, shown) {
        let names = Core.playbacks.pageNames.slice()
        for (let i = names.length; i <= shown; i++)
            names.push(qsTr("Page %1").arg(i + 1))
        return follows ? [qsTr("Current page (follows)")].concat(names) : names
    }
    readonly property bool functionPlayback: (targetInfo.needs === "playback" || targetInfo.needs === "playbackCue")
                                             && mapping.target !== "playbackSelect" && mapping.target !== "playbackFader"
    readonly property bool onFunction: functionPlayback && mapping.functionId !== undefined && mapping.functionId >= 0
    readonly property bool momentaryPlayback: button && (mapping.target || "").startsWith("playback") && mapping.target !== "playbackFlash"
    /** The cue a "go to a cue" mapping plays now (the cuelist of its playback, or its own) */
    readonly property string cueNow: {
        if (targetInfo.needs !== "playbackCue")
            return ""
        Core.playbacks.revision
        const info = onFunction ? Core.playbacks.functionStatus(mapping.functionId)
                                : Core.playbacks.slotInfo(mapping.pbPage !== undefined ? mapping.pbPage : -1, mapping.slot || 0)
        if (!info.isCuelist)
            return info.empty === false ? qsTr("It is not a cuelist") : ""
        const cue = Core.cuelists.cues(info.functionId)[mapping.cue || 0]
        return cue ? qsTr("Now: cue %1 of %2").arg(cue.number + (cue.name ? " · " + cue.name : "")).arg(info.name)
                   : qsTr("%1 has no cue %2").arg(info.name).arg((mapping.cue || 0) + 1)
    }

    signal removed()
    signal duplicated(int id)

    // the −/+ buttons of a SpinBox take 80 px: below this width its biggest value is cut ("100" shows "ı00")
    FontMetrics { id: spinFont; font.family: Theme.fontFamily; font.pixelSize: Theme.fontNormal }
    function spinMinWidth(spin) { return spin.leftPadding + spin.rightPadding + spinFont.advanceWidth(String(spin.to)) + 4 }

    function set(key, value) { let v = {}; v[key] = value; Core.mapping.updateMapping(mid, v) }
    function setMidi(note, number, channel) {
        set("channel", ((channel - 1) << 12) | (note ? 128 + number : number))
    }

    readonly property var functionList: Core.functions.slice().sort((a, b) => a.name.localeCompare(b.name))
    readonly property var zoneList: {
        let list = []
        for (let i = 0; i < Core.zones.count; i++)
            list.push({ id: Core.zones.zone(i).id, name: Core.zones.zone(i).name })
        return list
    }
    readonly property var deviceList: {
        let list = [{ key: "", name: qsTr("Any device") }].concat(
            Core.mapping.devices.filter(d => d.kind === editor.source && d.enabled).map(d => ({ key: d.key, name: d.name })))
        // a device that isn't connected now keeps its mappings
        let key = mapping.device || ""
        if (key.length > 0 && list.findIndex(d => d.key === key) < 0)
            list.push({ key: key, name: qsTr("%1 (not connected)").arg(key.substring(key.indexOf("|") + 1)) })
        return list
    }

    contentHeight: column.implicitHeight + 24
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical: ScrollBar {}

    component Caption: Text {
        Layout.topMargin: 6
        color: Theme.textDim
        font.pixelSize: Theme.fontSmall
        font.weight: Font.DemiBold
    }
    component Section: Text {
        Layout.topMargin: 10
        color: Theme.text
        font.weight: Font.Bold
    }

    ColumnLayout {
        id: column
        x: 12
        y: 12
        width: editor.width - 24
        spacing: 4

        Text {
            visible: editor.mid < 0
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Select a mapping, or add one")
            color: Theme.textFaint
        }

        ColumnLayout {
            Layout.fillWidth: true
            visible: editor.mid >= 0
            spacing: 4

            // ---------------------------------------------------------------- source
            Section { text: qsTr("Control") }
            ComboBox {
                Layout.fillWidth: true
                readonly property var kinds: ["midi", "osc", "dmx", "key"]
                model: ["MIDI", "OSC", qsTr("DMX input"), qsTr("Keyboard")]
                currentIndex: Math.max(0, kinds.indexOf(editor.source))
                onActivated: (i) => editor.set("source", kinds[i])
            }
            Caption { visible: editor.source !== "key"; text: qsTr("Device") }
            ComboBox {
                Layout.fillWidth: true
                visible: editor.source !== "key"
                textRole: "name"
                model: editor.deviceList
                currentIndex: Math.max(0, editor.deviceList.findIndex(d => d.key === (editor.mapping.device || "")))
                onModelChanged: currentIndex = Qt.binding(() => Math.max(0, editor.deviceList.findIndex(d => d.key === (editor.mapping.device || ""))))
                onActivated: (i) => editor.set("device", editor.deviceList[i].key)
            }
            // CC / note and its number, the MIDI channel below (on one row "127" and "16" didn't fit)
            GridLayout {
                visible: editor.source === "midi"
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 6
                ComboBox {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 90
                    model: ["CC", qsTr("Note")]
                    currentIndex: editor.midiNote ? 1 : 0
                    onActivated: (i) => editor.setMidi(i === 1, editor.midiNumber, editor.midiChannel)
                }
                SpinBox {
                    id: midiNumberSpin
                    Layout.fillWidth: true
                    Layout.preferredWidth: 140
                    Layout.minimumWidth: editor.spinMinWidth(midiNumberSpin)
                    from: 0; to: 127; editable: true
                    value: editor.midiNumber
                    onValueModified: editor.setMidi(editor.midiNote, value, editor.midiChannel)
                }
                Text { text: qsTr("Channel"); color: Theme.textDim }
                SpinBox {
                    id: midiChannelSpin
                    Layout.fillWidth: true
                    Layout.preferredWidth: 140
                    Layout.minimumWidth: editor.spinMinWidth(midiChannelSpin)
                    from: 1; to: 16; editable: true
                    value: editor.midiChannel
                    onValueModified: editor.setMidi(editor.midiNote, editor.midiNumber, value)
                }
            }
            TextField {
                Layout.fillWidth: true
                visible: editor.source === "osc" || editor.source === "key"
                color: Theme.text
                placeholderText: editor.source === "osc" ? "/scene/1" : qsTr("A key, e.g. Q or F9")
                text: editor.mapping.key || ""
                onEditingFinished: if (text !== (editor.mapping.key || "")) editor.set("key", editor.source === "key" ? text.toLowerCase() : text)
            }
            RowLayout {
                visible: editor.source === "dmx"
                Text { text: qsTr("Channel"); color: Theme.textDim }
                SpinBox {
                    from: 1; to: 512; editable: true
                    value: (editor.mapping.channel || 0) + 1
                    onValueModified: editor.set("channel", value - 1)
                }
            }
            AppButton {
                Layout.fillWidth: true
                text: editor.learning ? qsTr("Move a control of your device…") : qsTr("Learn the control")
                iconGlyph: Theme.iconTarget
                highlighted: editor.learning
                accentColor: "#ae3ec9"
                onClicked: editor.learning ? Core.mapping.cancelLearn() : Core.mapping.learnSource(editor.mid)
            }

            // ---------------------------------------------------------------- target
            Section { text: qsTr("Drives") }
            ComboBox {
                Layout.fillWidth: true
                textRole: "name"
                model: editor.targetList
                currentIndex: Math.max(0, editor.targetList.findIndex(t => t.target === editor.mapping.target))
                onActivated: (i) => editor.set("target", editor.targetList[i].target)
            }
            ComboBox {
                Layout.fillWidth: true
                visible: editor.targetInfo.needs === "function"
                textRole: "name"
                model: editor.functionList
                displayText: currentIndex < 0 ? qsTr("Choose a scene…") : currentText
                currentIndex: editor.functionList.findIndex(f => f.id === editor.mapping.functionId)
                onModelChanged: currentIndex = Qt.binding(() => editor.functionList.findIndex(f => f.id === editor.mapping.functionId))
                onActivated: (i) => editor.set("functionId", editor.functionList[i].id)
            }
            readonly property var groupChoices: editor.targetInfo.needs === "zoneAll"
                ? [{ id: -1, name: qsTr("All groups") }].concat(editor.zoneList) : editor.zoneList
            ComboBox {
                Layout.fillWidth: true
                visible: ["zone", "zoneAll", "zoneIndex"].indexOf(editor.targetInfo.needs) >= 0
                textRole: "name"
                model: parent.groupChoices
                displayText: currentIndex < 0 ? qsTr("Choose a group…") : currentText
                currentIndex: parent.groupChoices.findIndex(z => z.id === editor.mapping.zoneId)
                onModelChanged: currentIndex = Qt.binding(() => parent.groupChoices.findIndex(z => z.id === editor.mapping.zoneId))
                onActivated: (i) => editor.set("zoneId", parent.groupChoices[i].id)
            }
            RowLayout {
                visible: editor.targetInfo.needs === "zoneIndex"
                Text { text: qsTr("Scene number"); color: Theme.textDim }
                SpinBox {
                    from: 1; to: 99; editable: true
                    value: (editor.mapping.index || 0) + 1
                    onValueModified: editor.set("index", value - 1)
                }
            }
            ComboBox {
                Layout.fillWidth: true
                visible: editor.targetInfo.needs === "mixer"
                textRole: "name"
                model: Core.mixer.mixers
                currentIndex: Math.max(0, Core.mixer.mixers.findIndex(m => m.id === editor.mapping.mixerId))
                onModelChanged: currentIndex = Qt.binding(() => Math.max(0, Core.mixer.mixers.findIndex(m => m.id === editor.mapping.mixerId)))
                onActivated: (i) => editor.set("mixerId", Core.mixer.mixers[i].id)
            }
            readonly property var paletteList: Core.palettes.palettes.map(p => ({
                id: p.id,
                name: ({ intensity: "INT", position: "POS", colour: "COL", beam: "BEAM" })[p.type] + " " + p.number + " · " + p.name }))
            readonly property var paletteGroups: [{ id: -1, name: qsTr("Its own fixtures") }].concat(Core.patch.groups)
            ComboBox {
                Layout.fillWidth: true
                visible: editor.targetInfo.needs === "palette"
                textRole: "name"
                model: parent.paletteList
                displayText: currentIndex < 0 ? qsTr("Choose a palette…") : currentText
                currentIndex: parent.paletteList.findIndex(p => p.id === editor.mapping.paletteId)
                onModelChanged: currentIndex = Qt.binding(() => parent.paletteList.findIndex(p => p.id === editor.mapping.paletteId))
                onActivated: (i) => editor.set("paletteId", parent.paletteList[i].id)
            }
            ComboBox {
                Layout.fillWidth: true
                visible: editor.targetInfo.needs === "palette"
                textRole: "name"
                model: parent.paletteGroups
                currentIndex: Math.max(0, parent.paletteGroups.findIndex(g => g.id === editor.mapping.groupId))
                onModelChanged: currentIndex = Qt.binding(() => Math.max(0, parent.paletteGroups.findIndex(g => g.id === editor.mapping.groupId)))
                onActivated: (i) => editor.set("groupId", parent.paletteGroups[i].id)
            }

            // a playback: its page (or the current one) and its number, or a cuelist wherever it is
            readonly property bool slotNeeded: editor.targetInfo.needs === "playback" || editor.targetInfo.needs === "playbackCue"
            readonly property int pbPage: editor.mapping.pbPage !== undefined ? editor.mapping.pbPage : -1
            ComboBox {
                Layout.fillWidth: true
                visible: parent.slotNeeded
                enabled: !editor.onFunction
                model: editor.pageChoices(true, parent.pbPage)
                currentIndex: parent.pbPage + 1
                onModelChanged: currentIndex = Qt.binding(() => parent.pbPage + 1)
                onActivated: (i) => editor.set("pbPage", i - 1)
            }
            RowLayout {
                visible: parent.slotNeeded
                enabled: !editor.onFunction
                Text { text: qsTr("Playback"); color: Theme.textDim }
                SpinBox {
                    from: 1; to: 20; editable: true
                    value: (editor.mapping.slot || 0) + 1
                    onValueModified: editor.set("slot", value - 1)
                }
            }
            Caption { visible: editor.functionPlayback; text: qsTr("Or a cuelist or a scene, wherever it is") }
            ComboBox {
                Layout.fillWidth: true
                visible: editor.functionPlayback
                readonly property var choices: [{ id: -1, name: qsTr("No: the playback above") }].concat(editor.functionList)
                textRole: "name"
                model: choices
                currentIndex: Math.max(0, choices.findIndex(f => f.id === (editor.mapping.functionId !== undefined ? editor.mapping.functionId : -1)))
                onModelChanged: currentIndex = Qt.binding(() => Math.max(0, choices.findIndex(f => f.id === (editor.mapping.functionId !== undefined ? editor.mapping.functionId : -1))))
                onActivated: (i) => editor.set("functionId", choices[i].id)
            }
            RowLayout {
                visible: editor.targetInfo.needs === "playbackCue"
                Text { text: qsTr("Cue (1 is the first one of the list)"); color: Theme.textDim }
                SpinBox {
                    from: 1; to: 999; editable: true
                    value: (editor.mapping.cue || 0) + 1
                    onValueModified: editor.set("cue", value - 1)
                }
            }
            Text {
                Layout.fillWidth: true
                visible: editor.cueNow.length > 0
                text: editor.cueNow
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
                wrapMode: Text.WordWrap
            }
            // playback page N
            ComboBox {
                Layout.fillWidth: true
                visible: editor.targetInfo.needs === "playbackPage"
                model: editor.pageChoices(false, Math.max(0, parent.pbPage))
                currentIndex: Math.max(0, parent.pbPage)
                onModelChanged: currentIndex = Qt.binding(() => Math.max(0, parent.pbPage))
                onActivated: (i) => editor.set("pbPage", i)
            }

            // ---------------------------------------------------------------- mode
            // (the buttons of the playbacks act on the press, but Flash)
            Section { visible: !editor.momentaryPlayback; text: qsTr("Behaviour") }
            ComboBox {
                Layout.fillWidth: true
                visible: !editor.momentaryPlayback
                readonly property var modes: editor.button ? ["toggle", "flash", "on", "off"] : ["absolute", "incremental", "relative", "relative2c"]
                model: editor.button
                       ? [qsTr("Toggle: each press on / off"), qsTr("Flash: while pressed"), qsTr("Only on"), qsTr("Only off")]
                       : [qsTr("Absolute: follows the fader"), qsTr("Incremental: each press one step"), qsTr("Relative: endless encoder"),
                          qsTr("Relative: encoder, 1 up and 127 down")]
                currentIndex: Math.max(0, modes.indexOf(editor.mapping.mode))
                // the list changes between buttons and faders: pick the mode again
                onModelChanged: currentIndex = Qt.binding(() => Math.max(0, modes.indexOf(editor.mapping.mode)))
                onActivated: (i) => editor.set("mode", modes[i])
            }
            // the range on its own row: the labels of the switches (Loop, Step %) don't eat its width
            RowLayout {
                visible: !editor.button
                Layout.fillWidth: true
                spacing: 6
                Text { text: qsTr("From"); color: Theme.textDim }
                SpinBox {
                    id: minSpin
                    Layout.fillWidth: true
                    Layout.minimumWidth: editor.spinMinWidth(minSpin)
                    from: 0; to: 100; editable: true
                    value: editor.mapping.min !== undefined ? editor.mapping.min : 0
                    onValueModified: editor.set("min", value)
                }
                Text { text: qsTr("to"); color: Theme.textDim }
                SpinBox {
                    id: maxSpin
                    Layout.fillWidth: true
                    Layout.minimumWidth: editor.spinMinWidth(maxSpin)
                    from: 0; to: 100; editable: true
                    value: editor.mapping.max !== undefined ? editor.mapping.max : 100
                    onValueModified: editor.set("max", value)
                }
            }
            GridLayout {
                visible: !editor.button
                Layout.fillWidth: true
                columns: 4
                columnSpacing: 6
                Text { text: qsTr("Invert"); color: Theme.textDim }
                AppSwitch { checked: !!editor.mapping.invert; onToggled: editor.set("invert", checked) }
                Text { text: qsTr("Step %"); color: Theme.textDim; visible: editor.mapping.mode !== "absolute" }
                SpinBox {
                    id: stepSpin
                    Layout.fillWidth: true
                    Layout.minimumWidth: editor.spinMinWidth(stepSpin)
                    visible: editor.mapping.mode !== "absolute"
                    from: 1; to: 50; editable: true
                    value: editor.mapping.step || 5
                    onValueModified: editor.set("step", value)
                }
                Text { text: qsTr("Loop"); color: Theme.textDim; visible: editor.mapping.mode !== "absolute" }
                AppSwitch { visible: editor.mapping.mode !== "absolute"; checked: !!editor.mapping.loop; onToggled: editor.set("loop", checked) }
            }
            // a fader without a motor: it takes over where the level is (no jumps)
            RowLayout {
                visible: !editor.button && (editor.mapping.mode || "absolute") === "absolute"
                Layout.fillWidth: true
                AppSwitch { checked: !!editor.mapping.pickup; onToggled: editor.set("pickup", checked) }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: qsTr("Pick up the fader (no motor): it drives the level only after meeting it")
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                }
            }

            // ---------------------------------------------------------------- feedback
            Section { text: qsTr("Feedback"); visible: editor.source === "midi" || editor.source === "osc" }
            RowLayout {
                visible: editor.source === "midi" || editor.source === "osc"
                AppSwitch { checked: editor.mapping.feedback !== false; onToggled: editor.set("feedback", checked) }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: editor.button ? qsTr("Light the button of the device") : qsTr("Move the motor fader of the device")
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                }
            }
            // the LEDs: colours to pick on known controllers, numbers on the others
            RowLayout {
                Layout.fillWidth: true
                visible: editor.ledsVisible && editor.source === "midi"
                Text { text: qsTr("LEDs of"); color: Theme.textDim }
                ComboBox {
                    Layout.fillWidth: true
                    readonly property var kinds: ["", "apcmini", "apcmini2", "apc40mk2"]
                    model: [qsTr("Any controller (numbers)"), "Akai APC mini", "Akai APC mini mk2", "Akai APC40 mkII"]
                    currentIndex: Math.max(0, kinds.indexOf(editor.mapping.controller || ""))
                    onActivated: (i) => editor.set("controller", kinds[i])
                }
            }
            Repeater {
                model: editor.ledsVisible ? [
                    { key: "on", name: qsTr("On") },
                    { key: "off", name: qsTr("Off") },
                    { key: "paused", name: qsTr("Paused") }
                ] : []
                delegate: ColumnLayout {
                    id: ledRow
                    required property var modelData
                    readonly property int current: editor.mapping[modelData.key + "Value"] !== undefined ? editor.mapping[modelData.key + "Value"] : 0
                    readonly property int channel: editor.mapping[modelData.key + "Channel"] || 0
                    Layout.fillWidth: true
                    spacing: 3
                    RowLayout {
                        Layout.fillWidth: true
                        Text { Layout.preferredWidth: 70; text: ledRow.modelData.name; color: Theme.textDim }
                        // numbers: any controller
                        SpinBox {
                            visible: editor.ledPalette.length === 0
                            from: 0; to: 127; editable: true
                            value: ledRow.current
                            onValueModified: editor.set(ledRow.modelData.key + "Value", value)
                        }
                        // the RGB pads (APC mini mk2, APC40 mkII): how they light
                        ComboBox {
                            Layout.fillWidth: true
                            visible: editor.rgbPad
                            model: editor.ledChannelNames
                            currentIndex: Math.max(0, editor.ledChannels.indexOf(ledRow.channel === 0 ? 1 : ledRow.channel))
                            onModelChanged: currentIndex = Qt.binding(() => Math.max(0, editor.ledChannels.indexOf(ledRow.channel === 0 ? 1 : ledRow.channel)))
                            onActivated: (i) => editor.set(ledRow.modelData.key + "Channel", editor.ledChannels[i])
                        }
                        Item { Layout.fillWidth: true; visible: !editor.rgbPad }
                    }
                    Flow {
                        Layout.fillWidth: true
                        visible: editor.ledPalette.length > 0
                        spacing: 4
                        Repeater {
                            model: editor.ledPalette
                            delegate: Rectangle {
                                required property var modelData
                                width: 26
                                height: 26
                                radius: 4
                                color: modelData.color
                                border.color: ledRow.current === modelData.value ? Theme.text : Theme.border
                                border.width: ledRow.current === modelData.value ? 3 : 1
                                // a blinking colour
                                Text {
                                    anchors.centerIn: parent
                                    visible: !!modelData.blink
                                    text: "~"
                                    color: "black"
                                    font.weight: Font.Bold
                                }
                                ToolTip.text: modelData.name
                                ToolTip.visible: swatchMouse.containsMouse
                                ToolTip.delay: 300
                                MouseArea {
                                    id: swatchMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        let v = {}
                                        v[ledRow.modelData.key + "Value"] = parent.modelData.value
                                        // the RGB pads need a channel for their brightness (APC40: solid)
                                        if (editor.rgbPad && ledRow.channel === 0)
                                            v[ledRow.modelData.key + "Channel"] = editor.mapping.controller === "apcmini2" ? 7 : 1
                                        Core.mapping.updateMapping(editor.mid, v)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.topMargin: 12
                Layout.fillWidth: true
                AppButton {
                    Layout.fillWidth: true
                    text: qsTr("Duplicate")
                    iconGlyph: Theme.iconCopy
                    onClicked: editor.duplicated(Core.mapping.duplicateMapping(editor.mid))
                }
                AppButton {
                    Layout.fillWidth: true
                    text: qsTr("Delete")
                    iconGlyph: Theme.iconTrash
                    accentColor: Theme.danger
                    onClicked: { Core.mapping.removeMapping(editor.mid); editor.removed() }
                }
            }
        }
    }
}
