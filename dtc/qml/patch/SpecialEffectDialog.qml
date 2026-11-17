/*
  DMX Total Control
  SpecialEffectDialog.qml

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
 * A protected special effect (CO2, flames, sparks, confetti): its channels
 * only go out while the effects are armed, from scenes, cuelists and
 * chasers, each fire cut at its maximum time, with a minimum wait before the
 * next one, and optionally a safety channel the machine needs within a
 * window of values to fire. For the selected fixtures, applied at once and
 * saved with the show. Taking the protection away asks first.
 */
Popup {
    id: dialog

    property var ids: []
    property var settings: ({})
    readonly property var mixed: settings.mixed || []
    readonly property int count: settings.count || 0
    readonly property bool isProtected: !!settings.protected
    readonly property bool hasSafety: settings.safetyChannel !== undefined && settings.safetyChannel >= 0
    /** A safety value that couldn't be (said under the fields) */
    property string error: ""

    function openFor(list) {
        ids = list
        error = ""
        refresh()
        open()
    }
    function refresh() { settings = ids.length ? Core.patch.effectSettings(ids) : ({}) }
    function set(key, value) {
        error = Core.patch.setEffectSetting(ids, key, value) ? "" : Core.patch.lastError()
        // (refused: the fields show what it is)
        refresh()
    }
    /** The names of the fixtures of the dialog (a few, then how many) */
    function namesOf(list) {
        const names = Core.patch.fixtures.filter(f => list.indexOf(f.id) >= 0).map(f => f.name)
        return names.length <= 3 ? names.join(", ") : qsTr("%n fixture(s)", "", names.length)
    }

    Connections {
        target: Core.patch
        function onFixturesChanged() { if (dialog.opened) dialog.refresh() }
        function onProtectionChanged() { if (dialog.opened) dialog.refresh() }
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(560, parent.width - 80)
    modal: true
    padding: 18
    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    // a value the selected fixtures don't share (always there: it keeps its cell)
    component Mixed: Text {
        property string key: ""
        text: dialog.mixed.indexOf(key) >= 0 ? qsTr("(they differ)") : ""
        color: Theme.warning
        font.pixelSize: Theme.fontSmall
    }

    contentItem: ColumnLayout {
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            IconLabel {
                icon: Theme.iconShield
                size: 18
                color: dialog.isProtected ? Theme.readableOn(Theme.danger, Theme.surface) : Theme.textDim
            }
            Text {
                Layout.fillWidth: true
                text: qsTr("Special effect")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
                elide: Text.ElideRight
            }
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: dialog.namesOf(dialog.ids)
            color: Theme.textDim
        }

        // across two universes (a show from QLC+): it never fires
        Rectangle {
            Layout.fillWidth: true
            visible: !!dialog.settings.crossUniverse
            implicitHeight: crossText.implicitHeight + 16
            radius: Theme.radiusSmall
            color: Qt.alpha(Theme.danger, 0.16)
            border.color: Theme.danger
            Text {
                id: crossText
                anchors { fill: parent; margins: 8 }
                wrapMode: Text.WordWrap
                text: qsTr("It is across two universes: it never fires. Move it into a single universe")
                color: Theme.text
                font.weight: Font.DemiBold
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 3
            columnSpacing: 12
            rowSpacing: 10

            Text { text: qsTr("Protected special effect"); color: Theme.text; font.weight: Font.DemiBold }
            AppSwitch {
                id: protectSwitch
                checked: dialog.isProtected
                onToggled: {
                    const on = checked
                    checked = Qt.binding(() => dialog.isProtected)
                    // (taking it away asks first: unprotected, it goes out as any light)
                    if (on)
                        dialog.set("protected", true)
                    else
                        unprotectConfirm.openFor(null)
                }
            }
            Mixed { key: "protected" }

            Text { text: qsTr("Maximum fire time"); color: dialog.isProtected ? Theme.textDim : Theme.textFaint }
            TimeSpin {
                enabled: dialog.isProtected
                implicitWidth: 150
                from: 1
                to: 300
                engineValue: dialog.settings.maxFireMs || 3000
                onEdited: (ms) => dialog.set("maxFireMs", ms)
            }
            Mixed { key: "maxFireMs" }

            Text { text: qsTr("Minimum wait between fires"); color: dialog.isProtected ? Theme.textDim : Theme.textFaint }
            TimeSpin {
                enabled: dialog.isProtected
                implicitWidth: 150
                from: 0
                to: 600
                engineValue: dialog.settings.minWaitMs !== undefined ? dialog.settings.minWaitMs : 1000
                onEdited: (ms) => dialog.set("minWaitMs", ms)
            }
            Mixed { key: "minWaitMs" }

            Text { text: qsTr("Safety channel"); color: dialog.isProtected ? Theme.textDim : Theme.textFaint }
            ComboBox {
                id: safetyCombo
                Layout.preferredWidth: 220
                implicitHeight: 30
                // (the fixtures must have the same channels)
                enabled: dialog.isProtected && !!dialog.settings.sameMode
                readonly property var channels: dialog.settings.channels || []
                // (one of one channel: none, it would have nothing left to fire)
                model: [qsTr("None")].concat(channels.length < 2 ? [] : channels.map(c => (c.index + 1) + " · " + (c.name || qsTr("Channel %1").arg(c.index + 1))))
                currentIndex: dialog.mixed.indexOf("safetyChannel") >= 0 ? -1 : (dialog.settings.safetyChannel !== undefined ? dialog.settings.safetyChannel + 1 : 0)
                displayText: currentIndex < 0 ? qsTr("(they differ)") : currentText
                onActivated: (i) => dialog.set("safetyChannel", i - 1)
            }
            Text {
                Layout.fillWidth: true
                visible: !dialog.settings.sameMode && dialog.count > 1
                text: qsTr("Their channels differ")
                color: Theme.warning
                font.pixelSize: Theme.fontSmall
                wrapMode: Text.WordWrap
            }
            Mixed { visible: !!dialog.settings.sameMode || dialog.count <= 1; key: "safetyChannel" }

            Text {
                text: qsTr("Values from / to")
                color: dialog.isProtected && dialog.hasSafety ? Theme.textDim : Theme.textFaint
            }
            Row {
                spacing: 6
                enabled: dialog.isProtected && dialog.hasSafety
                SpinBox {
                    from: 0; to: 255
                    editable: true
                    implicitWidth: 110
                    implicitHeight: 30
                    value: dialog.settings.safetyFrom !== undefined ? dialog.settings.safetyFrom : 1
                    // (refused: back to what it is)
                    onValueModified: {
                        dialog.set("safetyFrom", value)
                        value = Qt.binding(() => dialog.settings.safetyFrom !== undefined ? dialog.settings.safetyFrom : 1)
                    }
                }
                Text { anchors.verticalCenter: parent.verticalCenter; text: "–"; color: Theme.textDim }
                SpinBox {
                    from: 0; to: 255
                    editable: true
                    implicitWidth: 110
                    implicitHeight: 30
                    value: dialog.settings.safetyTo !== undefined ? dialog.settings.safetyTo : 255
                    onValueModified: {
                        dialog.set("safetyTo", value)
                        value = Qt.binding(() => dialog.settings.safetyTo !== undefined ? dialog.settings.safetyTo : 255)
                    }
                }
            }
            Mixed { key: dialog.mixed.indexOf("safetyFrom") >= 0 ? "safetyFrom" : "safetyTo" }
        }

        Text {
            Layout.fillWidth: true
            visible: dialog.error.length > 0
            wrapMode: Text.WordWrap
            text: dialog.error
            color: Theme.readableOn(Theme.danger, Theme.surface)
        }

        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("It only fires from scenes, cuelists and chasers with «Arm effects» on. Identify, Highlight, the programmer, the mixer and the FX never fire it. Limits only count while it fires. The safety channel goes out within its values only while it fires; else 0.")
            color: Theme.textFaint
            font.pixelSize: Theme.fontSmall
        }

        RowLayout {
            Layout.fillWidth: true
            AppButton {
                text: Theme.kindName(dialog.settings.kind || "").length > 0 && dialog.mixed.indexOf("kind") < 0
                      ? qsTr("As its kind (%1)").arg(Theme.kindName(dialog.settings.kind))
                      : qsTr("As their kind")
                enabled: dialog.count > 0 && (!dialog.settings.byKind || dialog.mixed.length > 0)
                ToolTip.text: qsTr("Protected or not as its kind says (CO2, flames, sparks and confetti are), with the times of its kind and no safety channel")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: {
                    const reset = () => { Core.patch.resetEffectSettings(dialog.ids); dialog.error = ""; dialog.refresh() }
                    // (one that its kind doesn't protect loses its protection: asked too)
                    if (Core.patch.fixtures.some(f => dialog.ids.indexOf(f.id) >= 0 && f.protected && !f.kindProtected))
                        unprotectConfirm.openFor(reset)
                    else
                        reset()
                }
            }
            Item { Layout.fillWidth: true }
            AppButton {
                text: qsTr("Close")
                onClicked: dialog.close()
            }
        }
    }

    // taking the protection away: it goes out as any other light from then on
    Popup {
        id: unprotectConfirm
        property var then: null
        function openFor(action) { then = action; open() }
        onClosed: then = null
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(480, parent.width - 80)
        modal: true
        focus: true
        padding: 18
        background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }
        contentItem: ColumnLayout {
            spacing: 12
            Text {
                Layout.fillWidth: true
                text: qsTr("Take the protection away?")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
                wrapMode: Text.WordWrap
            }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Without it, %1 goes out as any other channel: without arming the effects, without a maximum time, and also from the programmer, Identify, the mixer and the FX.").arg(dialog.namesOf(dialog.ids))
                color: Theme.textDim
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton { text: qsTr("Cancel"); onClicked: unprotectConfirm.close() }
                AppButton {
                    text: qsTr("Take it away")
                    accentColor: Theme.danger
                    primary: true
                    onClicked: {
                        const action = unprotectConfirm.then
                        unprotectConfirm.close()
                        if (action)
                            action()
                        else
                            dialog.set("protected", false)
                    }
                }
            }
        }
    }
}
