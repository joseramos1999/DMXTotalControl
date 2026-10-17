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
    readonly property var selected: { Core.mapping.mappings; return selectedId >= 0 ? Core.mapping.mapping(selectedId) : ({}) }

    function modeName(mode) {
        switch (mode) {
        case "toggle": return qsTr("Toggle")
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
                title: qsTr("Mappings (%1)").arg(Core.mapping.mappings.length)
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
                    model: Core.mapping.mappings
                    ScrollBar.vertical: ScrollBar {}
                    delegate: Rectangle {
                        required property var modelData
                        width: list.width
                        height: 46
                        radius: Theme.radiusSmall
                        color: page.selectedId === modelData.id ? Qt.alpha(Theme.accent, 0.2)
                             : rowMouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised
                        border.color: page.selectedId === modelData.id ? Theme.accent : "transparent"
                        MouseArea { id: rowMouse; anchors.fill: parent; hoverEnabled: true; onClicked: page.selectedId = parent.modelData.id }
                        RowLayout {
                            anchors { fill: parent; leftMargin: 10; rightMargin: 10 }
                            spacing: 10
                            Rectangle {
                                width: 8; height: 8; radius: 4
                                color: parent.parent.modelData.button ? Theme.category("#ae3ec9") : Theme.accent
                            }
                            Text {
                                Layout.preferredWidth: parent.width * 0.42
                                text: parent.parent.modelData.sourceText
                                color: Theme.text
                                font.family: Theme.monoFamily
                                font.pixelSize: Theme.fontSmall
                                elide: Text.ElideRight
                            }
                            Text { text: "→"; color: Theme.textFaint }
                            Text {
                                Layout.fillWidth: true
                                text: parent.parent.modelData.targetText
                                color: Theme.text
                                elide: Text.ElideRight
                            }
                            Text {
                                text: page.modeName(parent.parent.modelData.mode)
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
                model: Core.mapping.devices
                ScrollBar.vertical: ScrollBar {}
                delegate: Rectangle {
                    id: deviceRow
                    required property var modelData
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
                            color: Theme.category(parent.parent.modelData.kind === "midi" ? "#ae3ec9"
                                 : parent.parent.modelData.kind === "osc" ? "#17a2b8" : "#3d8bfd")
                            Text {
                                anchors.centerIn: parent
                                text: parent.parent.parent.modelData.kind.toUpperCase()
                                color: Theme.textOn(parent.color)
                                font.pixelSize: 10
                                font.weight: Font.Bold
                            }
                        }
                        Column {
                            Layout.fillWidth: true
                            Text { text: parent.parent.parent.modelData.name; color: Theme.text; font.weight: Font.DemiBold }
                            Text {
                                text: parent.parent.parent.modelData.plugin +
                                      (parent.parent.parent.modelData.enabled ? "  ·  " + qsTr("universe %1").arg(parent.parent.parent.modelData.universe + 1) : "")
                                color: Theme.textFaint
                                font.pixelSize: Theme.fontSmall
                            }
                        }
                        // a whole controller in one go
                        AppButton {
                            id: mapAllButton
                            visible: parent.parent.modelData.kind === "midi"
                            text: qsTr("Map it all")
                            iconGlyph: Theme.iconWand
                            implicitHeight: 30
                            highlighted: parent.parent.modelData.controller.length > 0
                            ToolTip.text: qsTr("A whole controller in one go: the groups or the playbacks, on an Akai APC mini, APC mini mk2 or APC40 mkII")
                            ToolTip.visible: hovered
                            ToolTip.delay: 500
                            onClicked: templateMenu.popup(mapAllButton, 0, mapAllButton.height)
                            AppMenu {
                                id: templateMenu
                                MenuItem {
                                    text: qsTr("Akai APC mini → Groups")
                                    onTriggered: templateConfirm.ask(deviceRow.modelData, "apcmini", "groups")
                                }
                                MenuItem {
                                    text: qsTr("Akai APC mini → Playbacks")
                                    onTriggered: templateConfirm.ask(deviceRow.modelData, "apcmini", "playbacks")
                                }
                                MenuSeparator {}
                                MenuItem {
                                    text: qsTr("Akai APC mini mk2 → Groups")
                                    onTriggered: templateConfirm.ask(deviceRow.modelData, "apcmini2", "groups")
                                }
                                MenuItem {
                                    text: qsTr("Akai APC mini mk2 → Playbacks")
                                    onTriggered: templateConfirm.ask(deviceRow.modelData, "apcmini2", "playbacks")
                                }
                                MenuSeparator {}
                                MenuItem {
                                    text: qsTr("Akai APC40 mkII → Playbacks")
                                    onTriggered: templateConfirm.ask(deviceRow.modelData, "apc40mk2", "playbacks")
                                }
                            }
                        }
                        Text { text: qsTr("Input"); color: Theme.textDim }
                        AppSwitch {
                            checked: parent.parent.modelData.enabled
                            onToggled: Core.mapping.setDeviceEnabled(parent.parent.modelData.key, checked)
                        }
                        Text { text: qsTr("Feedback (LEDs)"); color: Theme.textDim; visible: parent.parent.modelData.canFeedback }
                        AppSwitch {
                            visible: parent.parent.modelData.canFeedback
                            enabled: parent.parent.modelData.enabled
                            checked: parent.parent.modelData.feedback
                            onToggled: Core.mapping.setDeviceFeedback(parent.parent.modelData.key, checked)
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
                model: page.tab === "monitor" ? Core.mapping.monitor : []
                ScrollBar.vertical: ScrollBar {}
                delegate: RowLayout {
                    required property var modelData
                    width: monitor.width
                    height: 24
                    spacing: 12
                    Text { Layout.preferredWidth: 100; text: parent.modelData.time; color: Theme.textFaint; font.family: Theme.monoFamily; font.pixelSize: Theme.fontSmall }
                    Text {
                        Layout.preferredWidth: 40
                        text: parent.modelData.direction === "in" ? "IN" : "OUT"
                        color: parent.modelData.direction === "in" ? Theme.success : Theme.warning
                        font.family: Theme.monoFamily
                        font.pixelSize: Theme.fontSmall
                        font.weight: Font.Bold
                    }
                    Text { Layout.preferredWidth: 220; text: parent.modelData.device || qsTr("Keyboard"); color: Theme.textDim; elide: Text.ElideRight; font.pixelSize: Theme.fontSmall }
                    Text { Layout.fillWidth: true; text: parent.modelData.text; color: Theme.text; font.family: Theme.monoFamily; font.pixelSize: Theme.fontSmall }
                    Text { Layout.preferredWidth: 50; text: parent.modelData.value; color: Theme.text; horizontalAlignment: Text.AlignRight; font.family: Theme.monoFamily; font.pixelSize: Theme.fontSmall }
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
