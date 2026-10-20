/*
  DMX Total Control
  AimSettingsDialog.qml

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
 * Where the moving heads of the selection hang, for Follow: their height,
 * hung or standing, the way they face, the travel of their pan and tilt and
 * which way they turn. Applied at once, saved with the show.
 */
Popup {
    id: dialog

    property var ids: []
    property var settings: ({})
    readonly property var mixed: settings.mixed || []

    function openFor(list) {
        ids = list
        refresh()
        open()
    }
    function refresh() { settings = ids.length ? Core.patch.aimSettings(ids) : ({}) }
    function set(key, value) { Core.patch.setAimSetting(ids, key, value) }

    Connections {
        target: Core.patch
        function onFixturesChanged() { if (dialog.opened) dialog.refresh() }
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(500, parent.width - 80)
    modal: true
    padding: 18
    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    // a value the selected heads don't share (always there: it keeps its cell)
    component Mixed: Text {
        property string key: ""
        text: dialog.mixed.indexOf(key) >= 0 ? qsTr("(they differ)") : ""
        color: Theme.warning
        font.pixelSize: Theme.fontSmall
    }
    // degrees of travel: 0, the one of its definition
    component TravelSpin: SpinBox {
        from: 0; to: 1080; stepSize: 5
        editable: true
        implicitWidth: 210
        implicitHeight: 30
        textFromValue: (v, locale) => v > 0 ? v + "°" : qsTr("Its definition")
        valueFromText: (text, locale) => { const n = parseInt(text); return isNaN(n) ? 0 : n }
        validator: RegularExpressionValidator { regularExpression: /.*/ }
    }
    // beside it: what the definition says (none: the usual travel is used),
    // or that the heads differ
    component TravelNote: Text {
        property string axis: "pan"
        readonly property int definition: dialog.settings[axis + "Definition"] || 0
        readonly property bool differ: dialog.mixed.indexOf(axis + "Range") >= 0 ||
                                       dialog.mixed.indexOf(axis + "Definition") >= 0
        text: differ ? qsTr("(they differ)")
                     : definition > 0 ? qsTr("Definition: %1°").arg(definition)
                                      : qsTr("Not in the definition: %1°").arg(axis === "pan" ? 540 : 270)
        color: differ || definition <= 0 ? Theme.warning : Theme.textDim
        font.pixelSize: Theme.fontSmall
    }

    contentItem: ColumnLayout {
        spacing: 12

        Text {
            text: qsTr("Height and facing")
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: (dialog.settings.count || 0) > 0
                  ? qsTr("%n moving head(s) of the selection. Follow aims them at a point of the plan with this.", "", dialog.settings.count || 0)
                  : qsTr("There are no moving heads in the selection.")
            color: Theme.textDim
        }

        GridLayout {
            Layout.fillWidth: true
            visible: (dialog.settings.count || 0) > 0
            columns: 3
            columnSpacing: 12
            rowSpacing: 10

            Text { text: qsTr("Height"); color: Theme.textDim }
            // tenths of a metre, shown in metres
            SpinBox {
                from: 0; to: 300; stepSize: 1
                editable: true
                implicitWidth: 210
                implicitHeight: 30
                value: Math.round((dialog.settings.height || 0) / 100)
                textFromValue: (v, locale) => (v / 10).toLocaleString(locale, 'f', 1) + " m"
                valueFromText: (text, locale) => Math.round(Theme.parseNumber(text, value / 10) * 10)
                validator: RegularExpressionValidator { regularExpression: /[0-9]+([.,][0-9])?( ?m)?/ }
                onValueModified: dialog.set("height", value * 100)
            }
            Mixed { key: "height" }

            Text { text: qsTr("Mounted"); color: Theme.textDim }
            Row {
                spacing: 0
                AppButton {
                    text: qsTr("Hung")
                    implicitHeight: 30
                    highlighted: !dialog.settings.standing
                    onClicked: dialog.set("standing", false)
                }
                AppButton {
                    text: qsTr("Standing")
                    implicitHeight: 30
                    highlighted: !!dialog.settings.standing
                    onClicked: dialog.set("standing", true)
                }
            }
            Mixed { key: "standing" }

            Text { text: qsTr("Facing"); color: Theme.textDim }
            SpinBox {
                from: 0; to: 359; stepSize: 15
                editable: true
                wrap: true
                implicitWidth: 210
                implicitHeight: 30
                value: dialog.settings.facing || 0
                textFromValue: (v, locale) => v + "°"
                valueFromText: (text, locale) => { const n = parseInt(text); return isNaN(n) ? 0 : n }
                validator: RegularExpressionValidator { regularExpression: /[0-9]{0,3}°?/ }
                onValueModified: dialog.set("facing", value)
                ToolTip.text: qsTr("0°: towards the audience; it turns clockwise on the plan")
                ToolTip.visible: hovered
                ToolTip.delay: 500
            }
            Mixed { key: "facing" }

            Text { text: qsTr("Pan travel"); color: Theme.textDim }
            TravelSpin {
                value: dialog.settings.panRange || 0
                onValueModified: dialog.set("panRange", value)
            }
            TravelNote { axis: "pan" }

            Text { text: qsTr("Tilt travel"); color: Theme.textDim }
            TravelSpin {
                value: dialog.settings.tiltRange || 0
                onValueModified: dialog.set("tiltRange", value)
            }
            TravelNote { axis: "tilt" }

            Text { text: qsTr("Pan the other way"); color: Theme.textDim }
            AppSwitch {
                checked: !!dialog.settings.panInverted
                onToggled: dialog.set("panInverted", checked)
            }
            Mixed { key: "panInverted" }

            Text { text: qsTr("Tilt the other way"); color: Theme.textDim }
            AppSwitch {
                checked: !!dialog.settings.tiltInverted
                onToggled: dialog.set("tiltInverted", checked)
            }
            Mixed { key: "tiltInverted" }
        }

        Text {
            Layout.fillWidth: true
            visible: (dialog.settings.count || 0) > 0
            wrapMode: Text.WordWrap
            text: qsTr("Facing is where the beam goes with the pan in the middle and the tilt leaning forward; the arrow on the plan shows it. If a head aims somewhere else with Follow, turn it 180° or make its pan or tilt turn the other way here (only for Follow).")
            color: Theme.textFaint
            font.pixelSize: Theme.fontSmall
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton {
                text: qsTr("Close")
                onClicked: dialog.close()
            }
        }
    }
}
