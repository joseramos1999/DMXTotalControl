/*
  DMX Total Control
  MappingsPage.qml

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
 * External control (mappings): MIDI, OSC, DMX input and keyboard.
 * The mappings with their settings, the devices to listen to (and to light
 * up), and a monitor of the messages in and out.
 */
Item {
    id: page

    property string tab: "mappings"
    signal message(string text, bool isError)
    property int selectedId: -1
    readonly property var selected: { Core.mapping.revision; return selectedId >= 0 ? Core.mapping.mapping(selectedId) : ({}) }

    /* The lists keep their rows: a change updates them in place. (A list given
     * as the model was made again on every change and the view went back to
     * the top: a colour picked for a mapping down the list lost it.) */
    ListModel { id: mappingRows }
    ListModel { id: deviceRows }
    ListModel { id: monitorRows }

    /** $model gets $rows, row by row in place */
    function fillRows(model, rows) {
        for (let i = 0; i < rows.length; i++) {
            if (i < model.count)
                model.set(i, rows[i])
            else
                model.append(rows[i])
        }
        if (model.count > rows.length)
            model.remove(rows.length, model.count - rows.length)
    }
    function readMappings() {
        fillRows(mappingRows, Core.mapping.mappings.map(m => ({
            mid: m.id, button: !!m.button, sourceText: m.sourceText || "", targetText: m.targetText || "",
            // (on a scene "toggle" is a press in the mode of the top bar, GO or Stop)
            mode: m.mode === "toggle" && ["scene", "sceneIndex"].indexOf(m.target || "scene") >= 0 ? "press" : (m.mode || "") })))
    }
    function readDevices() {
        fillRows(deviceRows, Core.mapping.devices.map(d => ({
            devKey: d.key, devName: d.name || "", plugin: d.plugin || "", kind: d.kind || "", inputOn: !!d.enabled,
            universe: d.universe, feedbackOn: !!d.feedback, canFeedback: !!d.canFeedback,
            controller: d.controller || "", group: d.group || "" })))
    }
    // the monitor, newest first: the new messages go on top (the rows being
    // read stay where they are), the oldest go
    readonly property bool monitorShown: page.visible && page.tab === "monitor"
    property int monitorSerial: 0
    function monitorRow(e) {
        return { msgTime: e.time || "", direction: e.direction || "", device: e.device || "",
                 msgText: e.text || "", value: e.value !== undefined ? e.value : 0 }
    }
    function readMonitor(all) {
        const list = Core.mapping.monitor
        let fresh = 0
        while (fresh < list.length && list[fresh].serial > monitorSerial)
            fresh++
        if (all || list.length === 0 || fresh === list.length || monitorRows.count + fresh < list.length) {
            monitorRows.clear()
            for (const e of list)
                monitorRows.append(monitorRow(e))
        } else {
            for (let i = fresh - 1; i >= 0; i--)
                monitorRows.insert(0, monitorRow(list[i]))
            if (monitorRows.count > list.length)
                monitorRows.remove(list.length, monitorRows.count - list.length)
        }
        monitorSerial = list.length > 0 ? list[0].serial : monitorSerial
    }
    onMonitorShownChanged: if (monitorShown) readMonitor(true)

    /** The selected mapping in view (added, duplicated) */
    function showSelected() {
        for (let i = 0; i < mappingRows.count; i++) {
            if (mappingRows.get(i).mid === selectedId) {
                list.positionViewAtIndex(i, ListView.Contain)
                return
            }
        }
    }
    onSelectedIdChanged: showSelected()

    Connections {
        target: Core.mapping
        function onMappingsChanged() { page.readMappings() }
        function onDevicesChanged() { page.readDevices() }
    }
    Connections {
        target: Core.mapping
        enabled: page.monitorShown
        function onMonitorChanged() { page.readMonitor(false) }
    }
    Component.onCompleted: { readMappings(); readDevices() }

    function modeName(mode) {
        switch (mode) {
        case "toggle": return qsTr("Toggle")
        case "press": return qsTr("Press")
        case "flash": return qsTr("Flash")
        case "on": return qsTr("Only on")
        case "off": return qsTr("Only off")
        case "absolute": return qsTr("Absolute")
        case "incremental": return qsTr("Incremental")
        case "relative": return qsTr("Relative")
        case "relative2c": return qsTr("Relative (2's c.)")
        }
        return mode || ""
    }

    ColumnLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 12

        // header
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            // the subtitle shortens: the buttons always fit
            Column {
                Layout.fillWidth: true
                Layout.minimumWidth: 120
                Layout.preferredWidth: 200
                Text {
                    width: parent.width
                    elide: Text.ElideRight
                    text: qsTr("Mappings")
                    color: Theme.text
                    font.pixelSize: Theme.fontTitle
                    font.weight: Font.Bold
                }
                Text {
                    width: parent.width
                    elide: Text.ElideRight
                    text: qsTr("Drive the show from MIDI controllers, OSC, a DMX desk or the keyboard")
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                    HoverHandler { id: subtitleHover }
                    ToolTip.text: text
                    ToolTip.visible: truncated && subtitleHover.hovered
                    ToolTip.delay: 500
                }
            }
            Row {
                spacing: 0
                AppButton { text: qsTr("Mappings"); iconGlyph: Theme.iconList; highlighted: page.tab === "mappings"; onClicked: page.tab = "mappings" }
                AppButton { text: qsTr("Devices"); iconGlyph: Theme.iconPlug; highlighted: page.tab === "devices"; onClicked: page.tab = "devices" }
                AppButton { text: qsTr("Monitor"); iconGlyph: Theme.iconSearch; highlighted: page.tab === "monitor"; onClicked: page.tab = "monitor" }
            }
            AppButton {
                text: Core.mapping.learnMode ? qsTr("Learning…") : qsTr("Learn")
                iconGlyph: Theme.iconTarget
                highlighted: Core.mapping.learnMode
                accentColor: Theme.learn
                ToolTip.text: qsTr("Pick a control of the app, then move the control of your device")
                ToolTip.visible: hovered
                onClicked: Core.mapping.learnMode = !Core.mapping.learnMode
            }
        }

        // ------------------------------------------------------------ mappings
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: page.tab === "mappings"
            spacing: 12

            Panel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: qsTr("Mappings (%1)").arg(list.count)
                headerItems: [
                    AppButton {
                        text: qsTr("Add")
                        iconGlyph: Theme.iconPlus
                        implicitHeight: 26
                        onClicked: page.selectedId = Core.mapping.addMapping({ target: "scene" })
                    }
                ]

                ListView {
                    id: list
                    anchors { fill: parent; margins: 6 }
                    clip: true
                    spacing: 3
                    model: mappingRows
                    ScrollBar.vertical: ScrollBar {}
                    delegate: Rectangle {
                        id: mappingRow
                        required property int mid
                        required property bool button
                        required property string sourceText
                        required property string targetText
                        required property string mode
                        width: list.width
                        height: 46
                        radius: Theme.radiusSmall
                        color: page.selectedId === mid ? Qt.alpha(Theme.accent, 0.2)
                             : rowMouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised
                        border.color: page.selectedId === mid ? Theme.accent : "transparent"
                        MouseArea { id: rowMouse; anchors.fill: parent; hoverEnabled: true; onClicked: page.selectedId = mappingRow.mid }
                        RowLayout {
                            anchors { fill: parent; leftMargin: 10; rightMargin: 10 }
                            spacing: 10
                            Rectangle {
                                width: 8; height: 8; radius: 4
                                color: mappingRow.button ? Theme.category("#ae3ec9") : Theme.accent
                            }
                            Text {
                                Layout.preferredWidth: parent.width * 0.42
                                text: mappingRow.sourceText
                                color: Theme.text
                                font.family: Theme.monoFamily
                                font.pixelSize: Theme.fontSmall
                                elide: Text.ElideRight
                            }
                            Text { text: "→"; color: Theme.textFaint }
                            Text {
                                Layout.fillWidth: true
                                text: mappingRow.targetText
                                color: Theme.text
                                elide: Text.ElideRight
                            }
                            Text {
                                text: page.modeName(mappingRow.mode)
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSmall
                            }
                        }
                    }
                    Text {
                        anchors.centerIn: parent
                        width: parent.width - 60
                        visible: list.count === 0
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        text: qsTr("No mappings yet. Enable your devices, then press “Learn”, click a control of the app (a scene, a master, the mixer...) and move a control of your device.")
                        color: Theme.textDim
                    }
                }
            }

            Panel {
                Layout.preferredWidth: 340
                Layout.fillHeight: true
                title: qsTr("Mapping")
                MappingEditor {
                    anchors.fill: parent
                    mapping: page.selected
                    onRemoved: page.selectedId = -1
                    onDuplicated: (id) => page.selectedId = id
                }
            }
        }

        // ------------------------------------------------------------ devices
        Panel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: page.tab === "devices"
            title: qsTr("Input devices")
            headerItems: [
                AppButton {
                    text: qsTr("Search again")
                    iconGlyph: Theme.iconRefresh
                    implicitHeight: 26
                    onClicked: Core.mapping.refreshDevices()
                }
            ]

            ListView {
                id: devices
                anchors { fill: parent; margins: 6 }
                clip: true
                spacing: 3
                model: deviceRows
                ScrollBar.vertical: ScrollBar {}
                // the controllers and what is in use first; the DMX inputs
                // (a line for each network card) after
                section.property: "group"
                section.delegate: Text {
                    required property string section
                    width: devices.width
                    topPadding: 8
                    bottomPadding: 4
                    leftPadding: 4
                    text: section === "controllers" ? qsTr("Controllers and devices in use")
                                                    : qsTr("DMX inputs: interfaces and network")
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                    font.weight: Font.Bold
                    font.capitalization: Font.AllUppercase
                    elide: Text.ElideRight
                }
                delegate: Rectangle {
                    id: deviceRow
                    required property string devKey
                    required property string devName
                    required property string plugin
                    required property string kind
                    required property bool inputOn
                    required property int universe
                    required property bool feedbackOn
                    required property bool canFeedback
                    required property string controller
                    // (what the template asks about)
                    readonly property var device: ({ key: devKey, name: devName, enabled: inputOn })
                    width: devices.width
                    height: 48
                    radius: Theme.radiusSmall
                    color: Theme.surfaceRaised
                    RowLayout {
                        anchors { fill: parent; leftMargin: 12; rightMargin: 12 }
                        spacing: 12
                        Rectangle {
                            Layout.preferredWidth: 48
                            height: 22
                            radius: 4
                            color: Theme.category(deviceRow.kind === "midi" ? "#ae3ec9"
                                 : deviceRow.kind === "osc" ? "#17a2b8" : "#3d8bfd")
                            Text {
                                anchors.centerIn: parent
                                text: deviceRow.kind.toUpperCase()
                                color: Theme.textOn(parent.color)
                                font.pixelSize: 10
                                font.weight: Font.Bold
                            }
                        }
                        Column {
                            Layout.fillWidth: true
                            Text { text: deviceRow.devName; color: Theme.text; font.weight: Font.DemiBold }
                            Text {
                                text: deviceRow.plugin +
                                      (deviceRow.inputOn ? "  ·  " + qsTr("universe %1").arg(deviceRow.universe + 1) : "")
                                color: Theme.textFaint
                                font.pixelSize: Theme.fontSmall
                            }
                        }
                        // a whole controller in one go
                        AppButton {
                            id: mapAllButton
                            visible: deviceRow.kind === "midi"
                            text: qsTr("Map it all")
                            iconGlyph: Theme.iconWand
                            implicitHeight: 30
                            highlighted: deviceRow.controller.length > 0
                            ToolTip.text: qsTr("A whole controller in one go: the groups or the playbacks, on an Akai APC mini, APC mini mk2 or APC40 mkII")
                            ToolTip.visible: hovered
                            ToolTip.delay: 500
                            onClicked: templateMenu.popup(mapAllButton, 0, mapAllButton.height)
                            AppMenu {
                                id: templateMenu
                                MenuItem {
                                    text: qsTr("Akai APC mini → Groups")
                                    onTriggered: templateConfirm.ask(deviceRow.device, "apcmini", "groups")
                                }
                                MenuItem {
                                    text: qsTr("Akai APC mini → Playbacks")
                                    onTriggered: templateConfirm.ask(deviceRow.device, "apcmini", "playbacks")
                                }
                                MenuSeparator {}
                                MenuItem {
                                    text: qsTr("Akai APC mini mk2 → Groups")
                                    onTriggered: templateConfirm.ask(deviceRow.device, "apcmini2", "groups")
                                }
                                MenuItem {
                                    text: qsTr("Akai APC mini mk2 → Playbacks")
                                    onTriggered: templateConfirm.ask(deviceRow.device, "apcmini2", "playbacks")
                                }
                                MenuSeparator {}
                                MenuItem {
                                    text: qsTr("Akai APC40 mkII → Playbacks")
                                    onTriggered: templateConfirm.ask(deviceRow.device, "apc40mk2", "playbacks")
                                }
                            }
                        }
                        Text { text: qsTr("Input"); color: Theme.textDim }
                        AppSwitch {
                            checked: deviceRow.inputOn
                            // (the row stays: back to what the device is, if it could not change)
                            onToggled: { Core.mapping.setDeviceEnabled(deviceRow.devKey, checked); checked = Qt.binding(() => deviceRow.inputOn) }
                        }
                        Text { text: qsTr("Feedback (LEDs)"); color: Theme.textDim; visible: deviceRow.canFeedback }
                        AppSwitch {
                            visible: deviceRow.canFeedback
                            enabled: deviceRow.inputOn
                            checked: deviceRow.feedbackOn
                            onToggled: { Core.mapping.setDeviceFeedback(deviceRow.devKey, checked); checked = Qt.binding(() => deviceRow.feedbackOn) }
                        }
                    }
                }
                Text {
                    anchors.centerIn: parent
                    width: parent.width - 60
                    visible: devices.count === 0
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: qsTr("No input devices found. Connect a MIDI controller (or a DMX interface with an input) and press “Search again”.")
                    color: Theme.textDim
                }
            }
        }

        // ------------------------------------------------------------ monitor
        Panel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: page.tab === "monitor"
            title: qsTr("Messages in and out")
            headerItems: [
                AppButton {
                    text: qsTr("Clear")
                    iconGlyph: Theme.iconEraser
                    implicitHeight: 26
                    onClicked: Core.mapping.clearMonitor()
                }
            ]

            ListView {
                id: monitor
                anchors { fill: parent; margins: 6 }
                clip: true
                model: monitorRows
                ScrollBar.vertical: ScrollBar {}
                delegate: RowLayout {
                    id: messageRow
                    required property string msgTime
                    required property string direction
                    required property string device
                    required property string msgText
                    required property int value
                    width: monitor.width
                    height: 24
                    spacing: 12
                    Text { Layout.preferredWidth: 100; text: messageRow.msgTime; color: Theme.textFaint; font.family: Theme.monoFamily; font.pixelSize: Theme.fontSmall }
                    Text {
                        Layout.preferredWidth: 40
                        text: messageRow.direction === "in" ? "IN" : "OUT"
                        color: messageRow.direction === "in" ? Theme.success : Theme.warning
                        font.family: Theme.monoFamily
                        font.pixelSize: Theme.fontSmall
                        font.weight: Font.Bold
                    }
                    Text { Layout.preferredWidth: 220; text: messageRow.device || qsTr("Keyboard"); color: Theme.textDim; elide: Text.ElideRight; font.pixelSize: Theme.fontSmall }
                    Text { Layout.fillWidth: true; text: messageRow.msgText; color: Theme.text; font.family: Theme.monoFamily; font.pixelSize: Theme.fontSmall }
                    Text { Layout.preferredWidth: 50; text: messageRow.value; color: Theme.text; horizontalAlignment: Text.AlignRight; font.family: Theme.monoFamily; font.pixelSize: Theme.fontSmall }
                }
                Text {
                    anchors.centerIn: parent
                    visible: monitor.count === 0
                    text: qsTr("Move a control of an enabled device to see its messages")
                    color: Theme.textDim
                }
            }
        }
    }

    // mapping a whole controller replaces the mappings of that device
    Popup {
        id: templateConfirm
        property var device: ({})
        property string controller: ""
        property string layout: "groups"
        function ask(device, controller, layout) {
            templateConfirm.device = device
            templateConfirm.controller = controller
            templateConfirm.layout = layout
            open()
        }
        readonly property string explanation: layout === "groups"
            ? qsTr("Each column of pads plays the scenes of a group (green: playing, yellow: ready), the faders are the masters of the groups and the last one the grand master. The buttons under the pads release the groups; the ones on the right are blackout, stop all, pause, tap, BeatGO, previous, next and flash. The mappings of this device are replaced.")
            : controller === "apc40mk2"
            ? qsTr("Each track is a playback 1-8 of the current playback page: its fader, GO on the track button, Pause/Back on S, Flash on the record arm, select and release, and its first 5 cues on the clips. The scene buttons go to pages 1-5, the bank buttons change the page and the group, Play and Record are GO and Back of the selected playback. The device knobs are the faders of the playbacks 9-16, the track knobs the masters of the first groups. The mappings of this device are replaced.")
            : qsTr("Each column is a playback 1-8 of the current playback page: its fader (it picks up the level), from the bottom GO, Pause/Back, Flash and its first 5 cues (cue 1 at the top); the button under the pads releases it. The last fader is the grand master; the buttons on the right are previous and next page, GO and Back of the selected playback, blackout, tap, BeatGO and stop all. The mappings of this device are replaced.")
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 460
        modal: true
        padding: 18
        background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }
        contentItem: ColumnLayout {
            spacing: 12
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: templateConfirm.layout === "groups" ? qsTr("Map all of %1 to the groups?").arg(templateConfirm.device.name || "")
                                                          : qsTr("Map all of %1 to the playbacks?").arg(templateConfirm.device.name || "")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
            }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: templateConfirm.explanation
                color: Theme.textDim
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton { text: qsTr("Cancel"); onClicked: templateConfirm.close() }
                AppButton {
                    text: qsTr("Map")
                    primary: true
                    onClicked: {
                        if (!templateConfirm.device.enabled)
                            Core.mapping.setDeviceEnabled(templateConfirm.device.key, true)
                        let n = Core.mapping.applyTemplate(templateConfirm.device.key, templateConfirm.controller, templateConfirm.layout)
                        page.message(qsTr("%n control(s) mapped", "", n), false)
                        templateConfirm.close()
                        page.tab = "mappings"
                    }
                }
            }
        }
    }
}
