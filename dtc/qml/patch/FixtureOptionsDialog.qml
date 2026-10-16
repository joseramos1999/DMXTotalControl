/*
  DMX Total Control
  FixtureOptionsDialog.qml

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
 * Limitations: the range of the dimmer, pan and tilt of fixtures,
 * inverted or swapped axes, and per channel whether it fades and follows
 * the dimmers. Everything that goes out obeys them (scenes, effects, live).
 */
Popup {
    id: dialog

    property var ids: []
    property var options: ({})
    readonly property bool single: ids.length === 1

    function openFor(list) {
        ids = list
        refresh()
        open()
    }
    // what any of the fixtures has (a dimmer first, heads after it), with
    // the values of the first one that has each
    function refresh() { options = ids.length ? Core.patch.selectionOptions(ids) : ({}) }
    function set(values) { Core.patch.setLimits(ids, values) }

    Connections {
        target: Core.patch
        function onFixturesChanged() { if (dialog.opened) dialog.refresh() }
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(560, parent.width - 80)
    height: Math.min(dialog.single ? 720 : 460, parent.height - 80)
    modal: true
    padding: 18
    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    // a range of 0-100 %, applied when a handle is released
    component LimitSlider: RangeSlider {
        id: range
        property string key
        from: 0; to: 100; stepSize: 1
        snapMode: RangeSlider.SnapAlways
        first.value: dialog.options[key + "Min"] !== undefined ? dialog.options[key + "Min"] : 0
        second.value: dialog.options[key + "Max"] !== undefined ? dialog.options[key + "Max"] : 100
        first.onPressedChanged: if (!first.pressed) apply()
        second.onPressedChanged: if (!second.pressed) apply()
        function apply() {
            let v = {}
            v[key + "Min"] = Math.round(first.value)
            v[key + "Max"] = Math.round(second.value)
            dialog.set(v)
        }
        background: Rectangle {
            x: range.leftPadding
            y: range.topPadding + range.availableHeight / 2 - height / 2
            width: range.availableWidth
            height: 4
            radius: 2
            color: Theme.surfaceHover
            Rectangle {
                x: range.first.visualPosition * parent.width
                width: (range.second.visualPosition - range.first.visualPosition) * parent.width
                height: parent.height
                radius: 2
                color: Theme.accent
            }
        }
        first.handle: Rectangle {
            x: range.leftPadding + range.first.visualPosition * (range.availableWidth - width)
            y: range.topPadding + range.availableHeight / 2 - height / 2
            implicitWidth: 16; implicitHeight: 16; radius: 8
            color: range.first.pressed ? Theme.accent : "#e7e9ef"
        }
        second.handle: Rectangle {
            x: range.leftPadding + range.second.visualPosition * (range.availableWidth - width)
            y: range.topPadding + range.availableHeight / 2 - height / 2
            implicitWidth: 16; implicitHeight: 16; radius: 8
            color: range.second.pressed ? Theme.accent : "#e7e9ef"
        }
    }
    component Caption: Text {
        color: Theme.textDim
        font.pixelSize: Theme.fontSmall
        font.weight: Font.DemiBold
    }
    component Value: Text {
        Layout.preferredWidth: 80
        horizontalAlignment: Text.AlignRight
        color: Theme.text
        font.family: Theme.monoFamily
        font.pixelSize: Theme.fontSmall
    }

    contentItem: ColumnLayout {
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Column {
                Layout.fillWidth: true
                Text {
                    text: qsTr("Limits")
                    color: Theme.text
                    font.pixelSize: Theme.fontLarge
                    font.weight: Font.Bold
                }
                Text {
                    width: parent.width
                    elide: Text.ElideRight
                    text: dialog.single ? (dialog.options.name || "") : qsTr("%n fixture(s)", "", dialog.ids.length)
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                }
            }
            AppButton {
                text: qsTr("Reset")
                iconGlyph: Theme.iconRefresh
                flat: true
                ToolTip.text: qsTr("No limits, no inverted or swapped axes")
                ToolTip.visible: hovered
                onClicked: dialog.set({ dimmerMin: 0, dimmerMax: 100, panMin: 0, panMax: 100, tiltMin: 0, tiltMax: 100,
                                        invertPan: false, invertTilt: false, swapPanTilt: false })
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 3
            columnSpacing: 12
            rowSpacing: 6

            Caption { text: qsTr("Dimmer"); visible: !!dialog.options.hasDimmer }
            LimitSlider { Layout.fillWidth: true; key: "dimmer"; visible: !!dialog.options.hasDimmer }
            Value { visible: !!dialog.options.hasDimmer; text: (dialog.options.dimmerMin || 0) + "–" + (dialog.options.dimmerMax !== undefined ? dialog.options.dimmerMax : 100) + " %" }

            Caption { text: qsTr("Pan"); visible: !!dialog.options.hasPan }
            LimitSlider { Layout.fillWidth: true; key: "pan"; visible: !!dialog.options.hasPan }
            Value { visible: !!dialog.options.hasPan; text: (dialog.options.panMin || 0) + "–" + (dialog.options.panMax !== undefined ? dialog.options.panMax : 100) + " %" }

            Caption { text: qsTr("Tilt"); visible: !!dialog.options.hasTilt }
            LimitSlider { Layout.fillWidth: true; key: "tilt"; visible: !!dialog.options.hasTilt }
            Value { visible: !!dialog.options.hasTilt; text: (dialog.options.tiltMin || 0) + "–" + (dialog.options.tiltMax !== undefined ? dialog.options.tiltMax : 100) + " %" }
        }

        Flow {
            Layout.fillWidth: true
            spacing: 14
            visible: !!dialog.options.hasPan || !!dialog.options.hasTilt
            Row {
                spacing: 6
                visible: !!dialog.options.hasPan
                AppSwitch { checked: !!dialog.options.invertPan; onToggled: dialog.set({ invertPan: checked }) }
                Text { text: qsTr("Invert pan"); color: Theme.text; anchors.verticalCenter: parent.verticalCenter }
            }
            Row {
                spacing: 6
                visible: !!dialog.options.hasTilt
                AppSwitch { checked: !!dialog.options.invertTilt; onToggled: dialog.set({ invertTilt: checked }) }
                Text { text: qsTr("Invert tilt"); color: Theme.text; anchors.verticalCenter: parent.verticalCenter }
            }
            Row {
                spacing: 6
                visible: !!dialog.options.hasPan && !!dialog.options.hasTilt
                AppSwitch { checked: !!dialog.options.swapPanTilt; onToggled: dialog.set({ swapPanTilt: checked }) }
                Text { text: qsTr("Swap pan and tilt"); color: Theme.text; anchors.verticalCenter: parent.verticalCenter }
            }
        }

        Text {
            Layout.fillWidth: true
            visible: !dialog.options.hasDimmer && !dialog.options.hasPan && !dialog.options.hasTilt
            wrapMode: Text.WordWrap
            text: dialog.single ? qsTr("This fixture has no dimmer, pan or tilt to limit.")
                                 : qsTr("These fixtures have no dimmer, pan or tilt to limit.")
            color: Theme.textFaint
        }
        Text {
            Layout.fillWidth: true
            visible: !dialog.single
            wrapMode: Text.WordWrap
            text: qsTr("The values shown are the ones of the first fixture that has each control. What you change applies to all the fixtures that have it.")
            color: Theme.textFaint
            font.pixelSize: Theme.fontSmall
        }

        // channels: fade and dimmers, one fixture at a time
        Panel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: dialog.single
            title: qsTr("Channels")
            headerItems: [
                Text { text: qsTr("Fades"); color: Theme.textDim; font.pixelSize: Theme.fontSmall; width: 64; horizontalAlignment: Text.AlignHCenter; anchors.verticalCenter: parent.verticalCenter },
                Text { text: qsTr("Dimmer"); color: Theme.textDim; font.pixelSize: Theme.fontSmall; width: 64; horizontalAlignment: Text.AlignHCenter; anchors.verticalCenter: parent.verticalCenter }
            ]

            ListView {
                id: channelList
                anchors { fill: parent; margins: 4 }
                clip: true
                model: dialog.single ? (dialog.options.channels || []) : []
                ScrollBar.vertical: ScrollBar {}
                delegate: Rectangle {
                    required property var modelData
                    width: channelList.width
                    height: 34
                    color: modelData.index % 2 ? "transparent" : Qt.alpha(Theme.surfaceHover, 0.5)
                    RowLayout {
                        anchors { fill: parent; leftMargin: 10; rightMargin: 10 }
                        spacing: 8
                        Text {
                            Layout.preferredWidth: 28
                            text: parent.parent.modelData.index + 1
                            color: Theme.textFaint
                            font.family: Theme.monoFamily
                            font.pixelSize: Theme.fontSmall
                        }
                        Text {
                            Layout.fillWidth: true
                            text: parent.parent.modelData.name
                            color: Theme.text
                            elide: Text.ElideRight
                        }
                        Text {
                            text: parent.parent.modelData.group
                            color: Theme.textFaint
                            font.pixelSize: Theme.fontSmall
                        }
                        CheckBox {
                            Layout.preferredWidth: 64
                            checked: parent.parent.modelData.canFade
                            ToolTip.text: qsTr("Follows the fade times. Off: jumps straight to its value (colour and gobo wheels)")
                            ToolTip.visible: hovered
                            ToolTip.delay: 600
                            onToggled: Core.patch.setChannelOptions(dialog.ids[0], parent.parent.modelData.index,
                                                                    checked, parent.parent.modelData.dimmer)
                        }
                        CheckBox {
                            Layout.preferredWidth: 64
                            checked: parent.parent.modelData.dimmer
                            ToolTip.text: qsTr("Scaled by the grand master, the group masters and the scene dimmers")
                            ToolTip.visible: hovered
                            ToolTip.delay: 600
                            onToggled: Core.patch.setChannelOptions(dialog.ids[0], parent.parent.modelData.index,
                                                                    parent.parent.modelData.canFade, checked)
                        }
                    }
                }
            }
        }
        Item { Layout.fillHeight: true; visible: !dialog.single }

        RowLayout {
            Layout.fillWidth: true
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Limits act on the output: scenes, effects and live edits keep their values.")
                color: Theme.textFaint
                font.pixelSize: Theme.fontSmall
            }
            AppButton { text: qsTr("Close"); primary: true; onClicked: dialog.close() }
        }
    }
}
