/*
  DMX Total Control
  FxPanel.qml

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
 * FX: pick a parameter of the selected fixtures (the
 * last one touched by default), and give it a wave around its value. The FX
 * play at once and are recorded with the scene.
 */
RowLayout {
    id: panel

    /** The parameter whose FX is shown */
    property string param: ""
    property var fx: ({})
    readonly property var params: Core.programmer.fxParams
    readonly property bool hasSelection: Core.programmer.selection.length > 0
    property string status: ""

    function refresh() {
        fx = param.length > 0 && hasSelection ? Core.programmer.fxOf(param) : ({})
    }
    function hasParam(key) { return params.some(p => p.key === key) }
    function pickParam() {
        let last = Core.programmer.lastParam
        if (last.length > 0 && hasParam(last))
            param = last
        else if (!hasParam(param))
            param = params.length > 0 ? params[0].key : ""
        refresh()
    }
    function edit(props) {
        Core.programmer.setFx(param, props)
    }
    function say(text) {
        status = text
        statusTimer.restart()
    }

    onParamChanged: refresh()
    onParamsChanged: pickParam()
    Component.onCompleted: pickParam()
    Connections {
        target: Core.programmer
        function onFxChanged() { panel.refresh() }
        function onLastParamChanged() { if (panel.hasParam(Core.programmer.lastParam)) panel.param = Core.programmer.lastParam }
    }
    Timer { id: statusTimer; interval: 4000; onTriggered: panel.status = "" }

    spacing: 10

    // ------------------------------------------------------------ parameters and running FX
    ColumnLayout {
        Layout.preferredWidth: 190
        Layout.maximumWidth: 190
        Layout.fillHeight: true
        spacing: 6

        Text { text: qsTr("Parameter"); color: Theme.textDim; font.pixelSize: Theme.fontSmall; font.weight: Font.DemiBold }
        ListView {
            id: paramList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 2
            clip: true
            spacing: 2
            model: panel.params
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}
            delegate: Rectangle {
                id: paramRow
                required property var modelData
                readonly property bool current: modelData.key === panel.param
                width: paramList.width
                height: 28
                radius: Theme.radiusSmall
                color: current ? Qt.alpha(Theme.accent, 0.25) : (paramMouse.containsMouse ? Theme.surfaceHover : "transparent")
                border.color: current ? Theme.accent : "transparent"
                RowLayout {
                    anchors { fill: parent; leftMargin: 8; rightMargin: 8 }
                    spacing: 6
                    Rectangle {
                        width: 10; height: 10; radius: 5
                        color: paramRow.modelData.colour || (paramRow.modelData.key === "pantilt" ? Theme.functionColor("EFX") : Theme.textFaint)
                    }
                    Text {
                        Layout.fillWidth: true
                        text: Theme.fxParamLabel(paramRow.modelData.key, paramRow.modelData.name)
                        color: Theme.text
                        font.pixelSize: Theme.fontSmall
                        font.weight: paramRow.modelData.key === "pantilt" ? Font.DemiBold : Font.Normal
                        elide: Text.ElideRight
                    }
                    // it has an FX on the selection
                    Text {
                        visible: !!paramRow.modelData.active
                        text: "FX"
                        color: Theme.accent
                        font.pixelSize: 9
                        font.weight: Font.Bold
                    }
                }
                MouseArea {
                    id: paramMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: { panel.param = paramRow.modelData.key; Core.programmer.lastParam = paramRow.modelData.key }
                }
            }
            Text {
                anchors.centerIn: parent
                width: parent.width - 20
                visible: paramList.count === 0
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: qsTr("Select fixtures")
                color: Theme.textFaint
                font.pixelSize: Theme.fontSmall
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }
        Text {
            text: qsTr("Running FX (%1)").arg(Core.programmer.fxLayers.length)
            color: Theme.textDim
            font.pixelSize: Theme.fontSmall
            font.weight: Font.DemiBold
        }
        ListView {
            id: runningList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 1
            clip: true
            spacing: 2
            model: Core.programmer.fxLayers
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}
            delegate: Rectangle {
                id: runRow
                required property var modelData
                width: runningList.width
                height: (modelData.ownCount || 0) > 0 ? 38 : 30
                radius: Theme.radiusSmall
                color: runMouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised
                RowLayout {
                    anchors { fill: parent; leftMargin: 4; rightMargin: 2 }
                    spacing: 4
                    FxShape {
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 22
                        figure: !!runRow.modelData.panTilt
                        value: runRow.modelData.panTilt ? runRow.modelData.figure : runRow.modelData.mode
                        lineWidth: 1.2
                    }
                    Column {
                        Layout.fillWidth: true
                        Text {
                            width: parent.width
                            text: Theme.fxParamLabel(runRow.modelData.param, "") + " · " + runRow.modelData.count
                            color: Theme.text
                            font.pixelSize: Theme.fontSmall
                            elide: Text.ElideRight
                        }
                        // fixtures that do it their own way
                        Text {
                            width: parent.width
                            visible: (runRow.modelData.ownCount || 0) > 0
                            text: qsTr("%n own", "", runRow.modelData.ownCount || 0)
                            color: Theme.accent
                            font.pixelSize: 9
                            font.weight: Font.Bold
                            elide: Text.ElideRight
                        }
                    }
                    IconLabel { visible: !!runRow.modelData.linked; icon: Theme.iconLink; size: 10; color: Theme.warning }
                    AppButton {
                        iconGlyph: Theme.iconClose
                        implicitHeight: 24
                        flat: true
                        ToolTip.text: qsTr("Stop this FX"); ToolTip.visible: hovered
                        onClicked: { Core.programmer.selectFx(runRow.modelData.index); Core.programmer.stopFx(runRow.modelData.param) }
                    }
                }
                MouseArea {
                    id: runMouse
                    anchors.fill: parent
                    z: -1
                    hoverEnabled: true
                    // its fixtures and its parameter
                    onClicked: Core.programmer.selectFx(runRow.modelData.index)
                }
            }
        }
        AppButton {
            Layout.fillWidth: true
            text: qsTr("Stop all the FX")
            iconGlyph: Theme.iconStop
            implicitHeight: 28
            enabled: Core.programmer.fxLayers.length > 0
            onClicked: Core.programmer.stopAllFx()
        }
    }

    Rectangle { Layout.fillHeight: true; width: 1; color: Theme.border }

    // ------------------------------------------------------------ the FX of the parameter
    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Text {
                Layout.fillWidth: true
                text: panel.param.length > 0 ? qsTr("FX on %1").arg(Theme.fxParamLabel(panel.param, (panel.params.find(p => p.key === panel.param) || {}).name || ""))
                                             : qsTr("FX")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
                elide: Text.ElideRight
            }
            Text {
                visible: panel.hasSelection
                text: qsTr("%n fixture(s)", "", panel.fx.count || 0)
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
            }
            AppButton {
                text: qsTr("Link")
                iconGlyph: Theme.iconLink
                implicitHeight: 28
                checkable: true
                enabled: !!panel.fx.active
                checked: !!panel.fx.linked
                ToolTip.text: qsTr("Linked FX share the speed and the timing (and stop together)"); ToolTip.visible: hovered
                onClicked: panel.edit({ linked: checked })
            }
            AppButton {
                visible: !panel.fx.active
                enabled: panel.hasSelection && panel.param.length > 0
                text: qsTr("Start FX")
                iconGlyph: Theme.iconPlay
                primary: true
                implicitHeight: 28
                onClicked: panel.edit({})
            }
            AppButton {
                visible: !!panel.fx.active
                text: qsTr("Stop")
                iconGlyph: Theme.iconStop
                accentColor: Theme.danger
                implicitHeight: 28
                onClicked: Core.programmer.stopFx(panel.param)
            }
        }

        Text {
            Layout.fillWidth: true
            visible: !panel.hasSelection || !!panel.fx.partial
            wrapMode: Text.WordWrap
            text: !panel.hasSelection ? qsTr("Select fixtures, then choose a parameter or a macro. The FX moves the parameter around its value in the programmer, or around what the scenes below send if it has none.")
                                      : qsTr("This FX is also on other fixtures. The shape, size, direction and phase change only on the selected ones; the speed and the timing on the whole FX, so they all stay in sync.")
            color: Theme.textDim
            font.pixelSize: Theme.fontSmall
        }
        RowLayout {
            Layout.fillWidth: true
            visible: panel.hasSelection && (panel.fx.ownSelected || 0) > 0
            spacing: 8
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("%n of the selected fixture(s) do this FX their own way.", "", panel.fx.ownSelected || 0)
                color: Theme.accent
                font.pixelSize: Theme.fontSmall
            }
            AppButton {
                text: qsTr("Like the rest")
                iconGlyph: Theme.iconRefresh
                implicitHeight: 28
                ToolTip.text: qsTr("The selected fixtures do what the FX does again"); ToolTip.visible: hovered
                onClicked: Core.programmer.resetFxOwn(panel.param)
            }
        }
        // the ones on one side go the other way round: they follow their twins in the wave
        Text {
            Layout.fillWidth: true
            visible: panel.hasSelection && !!panel.fx.mirror
            wrapMode: Text.WordWrap
            text: qsTr("Mirror: the fixtures that go the other way round follow the wave of their twin on the other side.")
            color: Theme.textDim
            font.pixelSize: Theme.fontSmall
        }

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: panel.hasSelection && panel.param.length > 0
            clip: true
            contentHeight: fxControls.implicitHeight
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}
            FxControls {
                id: fxControls
                width: parent.width - 12
                fx: panel.fx
                sets: panel.fx.sets || 1
                partial: !!panel.fx.partial
                opacity: panel.fx.active ? 1 : 0.7
                onEdit: (props) => panel.edit(props)
            }
        }

        FxPreview {
            Layout.fillWidth: true
            Layout.preferredHeight: panel.fx.panTilt ? 110 : 70
            visible: !!panel.fx.active
            fx: panel.fx
            sample: (pos) => Core.programmer.fxPreview(panel.param, pos)
        }
    }

    Rectangle { Layout.fillHeight: true; width: 1; color: Theme.border }

    // ------------------------------------------------------------ macros
    ColumnLayout {
        Layout.preferredWidth: 230
        Layout.maximumWidth: 230
        Layout.fillHeight: true
        spacing: 6

        Text { text: qsTr("FX macros"); color: Theme.textDim; font.pixelSize: Theme.fontSmall; font.weight: Font.DemiBold }
        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentHeight: macroColumn.implicitHeight
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}

            Column {
                id: macroColumn
                width: parent.width - 10
                spacing: 6

                Repeater {
                    model: [
                        { key: "position", label: qsTr("Position") },
                        { key: "intensity", label: qsTr("Intensity") },
                        { key: "colour", label: qsTr("Colour") },
                        { key: "beam", label: qsTr("Beam") },
                        { key: "user", label: qsTr("Mine") }
                    ]
                    delegate: Column {
                        id: category
                        required property var modelData
                        readonly property var items: Core.fxMacros.macros.filter(m => m.category === modelData.key)
                        width: macroColumn.width
                        spacing: 4
                        visible: items.length > 0

                        Text { text: category.modelData.label; color: Theme.textFaint; font.pixelSize: 10 }
                        Flow {
                            width: parent.width
                            spacing: 4
                            Repeater {
                                model: category.items
                                delegate: Rectangle {
                                    id: macro
                                    required property var modelData
                                    width: (macroColumn.width - 4) / 2
                                    height: 46
                                    radius: Theme.radiusSmall
                                    color: macroMouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised
                                    border.color: Theme.border
                                    FxShape {
                                        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 3 }
                                        height: 22
                                        figure: !!macro.modelData.panTilt
                                        value: macro.modelData.panTilt ? macro.modelData.figure : macro.modelData.mode
                                        color: Theme.functionColor(macro.modelData.panTilt ? "EFX" : "FXScene")
                                        lineWidth: 1.2
                                    }
                                    Text {
                                        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: 3 }
                                        horizontalAlignment: Text.AlignHCenter
                                        text: macro.modelData.name
                                        color: Theme.text
                                        font.pixelSize: Theme.fontSmall
                                        elide: Text.ElideRight
                                    }
                                    MouseArea {
                                        id: macroMouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                                        onClicked: (ev) => {
                                            if (ev.button === Qt.RightButton) {
                                                if (!macro.modelData.factory) {
                                                    macroMenu.macroId = macro.modelData.id
                                                    macroMenu.macroName = macro.modelData.name
                                                    macroMenu.popup()
                                                }
                                                return
                                            }
                                            if (!panel.hasSelection)
                                                panel.say(qsTr("Select fixtures first"))
                                            else if (!Core.fxMacros.apply(macro.modelData.id))
                                                panel.say(qsTr("None of the selected fixtures has what %1 moves").arg(macro.modelData.name))
                                        }
                                    }
                                    ToolTip.visible: macroMouse.containsMouse
                                    ToolTip.text: macro.modelData.factory ? macro.modelData.name : qsTr("%1 (right click: rename or delete)").arg(macro.modelData.name)
                                }
                            }
                        }
                    }
                }
            }
        }
        Text {
            Layout.fillWidth: true
            visible: panel.status.length > 0
            text: panel.status
            wrapMode: Text.WordWrap
            color: Theme.warning
            font.pixelSize: Theme.fontSmall
        }
        AppButton {
            Layout.fillWidth: true
            text: qsTr("Save as a macro…")
            iconGlyph: Theme.iconSave
            implicitHeight: 28
            enabled: Core.programmer.fxLayers.length > 0
            onClicked: prompt.ask(qsTr("Name of the macro"), "", name => {
                if (Core.fxMacros.save(name).length > 0)
                    panel.say(qsTr("Saved in your macros"))
            })
        }
    }

    AppMenu {
        id: macroMenu
        property string macroId
        property string macroName
        MenuItem {
            text: qsTr("Rename")
            onTriggered: prompt.ask(qsTr("Name of the macro"), macroMenu.macroName, name => Core.fxMacros.rename(macroMenu.macroId, name))
        }
        MenuItem {
            text: qsTr("Delete")
            onTriggered: Core.fxMacros.remove(macroMenu.macroId)
        }
    }
    TextPrompt { id: prompt }
}
