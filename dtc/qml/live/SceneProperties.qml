/*
  DMX Total Control
  SceneProperties.qml

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
 * Properties of a scene inside a group:
 * fades, priority, release mode, release protection and flash.
 */
Popup {
    id: dialog

    property Zone zoneObject
    property int functionId: -1
    property string functionType: ""
    /** Scenes that move: the live speed applies */
    readonly property bool dynamic: ["Sequence", "Chaser", "EFX", "RGBMatrix", "CurveFX", "FXScene", "Cuelist"].indexOf(functionType) >= 0
    /** Scenes with a shape: the live phase and size apply */
    readonly property bool shaped: functionType === "EFX" || functionType === "CurveFX" || functionType === "FXScene"
    /** Steps, cuelists and FX at the BPM of the show: they take the rhythm
     *  tapped on their button */
    property bool tappable: false
    property int tapPeriod: 0

    // combo index -> Zone::ReleaseMode / Zone::Protection
    readonly property var releaseValues: [-1, 0, 1, 2, 3]
    readonly property var releaseNames: [qsTr("Group default"), qsTr("None"), qsTr("Its group"),
                                         qsTr("All the scenes"), qsTr("The other groups")]
    readonly property var protectionNames: [qsTr("None"), qsTr("Everything (manual stop only)"),
                                            qsTr("Its own group"), qsTr("The other groups")]

    function edit(zone, fid) {
        zoneObject = zone
        functionId = fid
        let info = Core.functionInfo(fid)
        let opts = zone.options(fid)
        nameField.text = info.name
        fadeInSpin.value = Math.round(info.fadeIn / 100)
        fadeOutSpin.value = Math.round(info.fadeOut / 100)
        prioritySpin.value = info.priority
        releaseCombo.currentIndex = Math.max(0, releaseValues.indexOf(opts.release))
        protectionCombo.currentIndex = opts.protection
        flashSwitch.checked = opts.flash
        dimmerSlider.value = opts.dimmer !== undefined ? opts.dimmer : 1
        functionType = info.type || ""
        tappable = !!info.tappable
        tapPeriod = info.tap || 0
        speedSlider.value = Math.log10(opts.speed || 1)
        phaseSlider.value = opts.phase || 0
        sizeSlider.value = opts.size !== undefined ? opts.size : 1
        fadeSpeedSlider.value = Math.log10(opts.fadeSpeed || 1)
        open()
    }

    function apply() {
        Core.renameFunction(functionId, nameField.text)
        Core.setFunctionFades(functionId, fadeInSpin.value * 100, fadeOutSpin.value * 100)
        Core.setFunctionPriority(functionId, prioritySpin.value)
        zoneObject.setOption(functionId, "release", releaseValues[releaseCombo.currentIndex])
        zoneObject.setOption(functionId, "protection", protectionCombo.currentIndex)
        zoneObject.setOption(functionId, "flash", flashSwitch.checked)
        close()
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 480
    modal: true
    padding: 18

    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    // tenths of a second, shown as seconds
    component SecondsSpin: SpinBox {
        from: 0; to: 6000; stepSize: 1
        editable: true
        textFromValue: (value, locale) => (value / 10).toLocaleString(locale, 'f', 1) + " s"
        valueFromText: (text, locale) => Math.round(Theme.parseNumber(text, value / 10) * 10)
        validator: RegularExpressionValidator { regularExpression: /[0-9]+([.,][0-9])?( s)?/ }
    }

    contentItem: ColumnLayout {
        spacing: 12

        Text {
            text: qsTr("Scene properties")
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 12
            rowSpacing: 10

            Text { text: qsTr("Name"); color: Theme.textDim }
            TextField {
                id: nameField
                Layout.fillWidth: true
                color: Theme.text
                background: Rectangle { radius: Theme.radiusSmall; color: Theme.background; border.color: nameField.activeFocus ? Theme.accent : Theme.border }
            }

            Text { text: qsTr("Fade in"); color: Theme.textDim }
            SecondsSpin { id: fadeInSpin; Layout.fillWidth: true }

            Text { text: qsTr("Fade out"); color: Theme.textDim }
            SecondsSpin { id: fadeOutSpin; Layout.fillWidth: true }

            Text { text: qsTr("Priority (0-10)"); color: Theme.textDim }
            SpinBox { id: prioritySpin; Layout.fillWidth: true; from: 0; to: 10; editable: true }

            Text { text: qsTr("When it starts, release"); color: Theme.textDim }
            ComboBox { id: releaseCombo; Layout.fillWidth: true; model: dialog.releaseNames }

            Text { text: qsTr("Protected from releases by"); color: Theme.textDim }
            ComboBox { id: protectionCombo; Layout.fillWidth: true; model: dialog.protectionNames }

            Text { text: qsTr("Flash (plays while pressed)"); color: Theme.textDim }
            AppSwitch { id: flashSwitch }

            Text { text: qsTr("Live dimmer"); color: Theme.textDim }
            RowLayout {
                Layout.fillWidth: true
                HSlider {
                    id: dimmerSlider
                    Layout.fillWidth: true
                    from: 0; to: 1
                    // a live control: applied while moving
                    onMoved: dialog.zoneObject.setOption(dialog.functionId, "dimmer", value)
                }
                Text {
                    Layout.preferredWidth: 52
                    horizontalAlignment: Text.AlignRight
                    text: Math.round(dimmerSlider.value * 100) + "%"
                    color: Theme.text
                    font.family: Theme.monoFamily
                }
            }

            // live dials: they change how it plays, not the scene
            Text { text: qsTr("Live speed"); color: Theme.textDim; visible: dialog.dynamic }
            RowLayout {
                Layout.fillWidth: true
                visible: dialog.dynamic
                HSlider {
                    id: speedSlider
                    Layout.fillWidth: true
                    // logarithmic: ×0.1 .. ×10, the middle is ×1
                    from: -1; to: 1
                    onMoved: dialog.zoneObject.setOption(dialog.functionId, "speed", Math.pow(10, value))
                }
                Text {
                    Layout.preferredWidth: 52
                    horizontalAlignment: Text.AlignRight
                    text: "×" + Math.pow(10, speedSlider.value).toLocaleString(Qt.locale(), "f", 2)
                    color: Theme.text
                    font.family: Theme.monoFamily
                }
            }

            // its own rhythm: presses in a row of its button
            Text {
                text: qsTr("Tapped rhythm")
                color: Theme.textDim
                visible: dialog.tappable
                ToolTip.text: dialog.functionType === "FXScene"
                              ? qsTr("Press its button several times in a row: the first press starts the cycle of its FX at the BPM of the show again, each next one takes them to the next beat, and they do a cycle on each beat, at that rhythm")
                              : qsTr("Press its button several times in a row: a step (a GO) on each beat, at that rhythm")
                ToolTip.visible: tapMouse.containsMouse
                ToolTip.delay: 500
                MouseArea { id: tapMouse; anchors.fill: parent; hoverEnabled: true }
            }
            TapRhythm {
                visible: dialog.tappable
                fx: dialog.functionType === "FXScene"
                period: dialog.tapPeriod
                // a live setting: applied at once
                onEdited: (ms) => {
                    Core.effects.setPlayback(dialog.functionId, "tap", ms)
                    dialog.tapPeriod = Core.functionInfo(dialog.functionId).tap || 0
                }
            }

            Text { text: qsTr("Live phase"); color: Theme.textDim; visible: dialog.shaped }
            RowLayout {
                Layout.fillWidth: true
                visible: dialog.shaped
                HSlider {
                    id: phaseSlider
                    Layout.fillWidth: true
                    from: 0; to: 359; stepSize: 1
                    onMoved: dialog.zoneObject.setOption(dialog.functionId, "phase", Math.round(value))
                }
                Text {
                    Layout.preferredWidth: 52
                    horizontalAlignment: Text.AlignRight
                    text: Math.round(phaseSlider.value) + "°"
                    color: Theme.text
                    font.family: Theme.monoFamily
                }
            }

            Text { text: qsTr("Live size"); color: Theme.textDim; visible: dialog.shaped }
            RowLayout {
                Layout.fillWidth: true
                visible: dialog.shaped
                HSlider {
                    id: sizeSlider
                    Layout.fillWidth: true
                    from: 0; to: 2
                    onMoved: dialog.zoneObject.setOption(dialog.functionId, "size", value)
                }
                Text {
                    Layout.preferredWidth: 52
                    horizontalAlignment: Text.AlignRight
                    text: Math.round(sizeSlider.value * 100) + "%"
                    color: Theme.text
                    font.family: Theme.monoFamily
                }
            }

            Text { text: qsTr("Fade speed"); color: Theme.textDim }
            RowLayout {
                Layout.fillWidth: true
                HSlider {
                    id: fadeSpeedSlider
                    Layout.fillWidth: true
                    from: -1; to: 1
                    onMoved: dialog.zoneObject.setOption(dialog.functionId, "fadeSpeed", Math.pow(10, value))
                }
                Text {
                    Layout.preferredWidth: 52
                    horizontalAlignment: Text.AlignRight
                    text: "×" + Math.pow(10, fadeSpeedSlider.value).toLocaleString(Qt.locale(), "f", 2)
                    color: Theme.text
                    font.family: Theme.monoFamily
                }
            }
        }

        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("The group fade time, when set, replaces the scene fades. Higher priority wins on shared channels.")
            color: Theme.textFaint
            font.pixelSize: Theme.fontSmall
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Cancel"); onClicked: dialog.close() }
            AppButton { text: qsTr("Apply"); primary: true; onClicked: dialog.apply() }
        }
    }
}
