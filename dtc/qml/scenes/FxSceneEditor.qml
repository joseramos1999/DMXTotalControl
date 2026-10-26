/*
  DMX Total Control
  FxSceneEditor.qml

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
 * A scene with FX: its base values and its FX layers. Each layer is
 * edited here, live while the scene plays; new layers and base values are
 * made in the programmer.
 */
RowLayout {
    id: editor

    property var info: ({})
    property int current: 0
    readonly property var layers: info.layers || []
    readonly property var currentLayer: layers.length > 0 ? layers[Math.min(current, layers.length - 1)] : ({})
    /** The rhythm tapped on its button (BPM), 0: none: its FX at the BPM of
     *  the show go at it */
    readonly property real tappedBpm: (info.tap || 0) > 0 ? 60000 / info.tap : 0

    signal editInProgrammer(int id)
    signal saved()

    onInfoChanged: if (current >= layers.length) current = Math.max(0, layers.length - 1)

    spacing: 12

    // ------------------------------------------------------------ the layers
    Panel {
        Layout.preferredWidth: 300
        Layout.fillHeight: true
        title: qsTr("FX (%1)").arg(editor.layers.length)

        ColumnLayout {
            anchors { fill: parent; margins: 8 }
            spacing: 6

            ListView {
                id: layerList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                spacing: 4
                model: editor.layers
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar {}
                delegate: Rectangle {
                    id: row
                    required property var modelData
                    required property int index
                    readonly property bool selected: index === editor.current
                    width: layerList.width
                    height: 52
                    radius: Theme.radiusSmall
                    color: selected ? Qt.alpha(Theme.accent, 0.2) : (rowMouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised)
                    border.color: selected ? Theme.accent : "transparent"

                    RowLayout {
                        anchors { fill: parent; margins: 6 }
                        spacing: 8
                        FxShape {
                            Layout.preferredWidth: 52
                            Layout.preferredHeight: 38
                            figure: !!row.modelData.panTilt
                            value: row.modelData.panTilt ? row.modelData.figure : row.modelData.mode
                            color: Theme.functionColor(row.modelData.panTilt ? "EFX" : "FXScene")
                        }
                        Column {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                width: parent.width
                                text: Theme.fxParamLabel(row.modelData.param, "")
                                color: Theme.text
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }
                            Text {
                                width: parent.width
                                text: qsTr("%n fixture(s)", "", row.modelData.count) + " · " +
                                      (!row.modelData.syncBpm ? Math.round(row.modelData.bpm) + " BPM"
                                       : editor.tappedBpm > 0 ? qsTr("Tapped: %1 BPM").arg(Math.round(editor.tappedBpm))
                                       : qsTr("BPM of the show"))
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSmall
                                elide: Text.ElideRight
                            }
                        }
                        IconLabel { visible: !!row.modelData.linked; icon: Theme.iconLink; size: 11; color: Theme.warning }
                        AppButton {
                            iconGlyph: Theme.iconTrash
                            flat: true
                            implicitHeight: 28
                            ToolTip.text: qsTr("Remove this FX"); ToolTip.visible: hovered
                            onClicked: Core.effects.removeFxLayer(editor.info.id, row.index)
                        }
                    }
                    MouseArea {
                        id: rowMouse
                        anchors.fill: parent
                        z: -1
                        hoverEnabled: true
                        onClicked: editor.current = row.index
                    }
                }

                Text {
                    anchors.centerIn: parent
                    width: parent.width - 30
                    visible: layerList.count === 0
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: qsTr("No FX: only base values. Add FX in the programmer.")
                    color: Theme.textFaint
                }
            }

            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("%1 base values on %2 fixtures. Without a base value, an FX moves what the scenes below send.")
                          .arg(editor.info.channels || 0).arg(editor.info.fixtures || 0)
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
            }
            AppButton {
                Layout.fillWidth: true
                text: qsTr("Edit in the programmer")
                iconGlyph: Theme.iconSliders
                onClicked: editor.editInProgrammer(editor.info.id)
            }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("There, change base values or add FX, then save the programmer in this scene.")
                color: Theme.textFaint
                font.pixelSize: Theme.fontSmall
            }
            AppButton {
                Layout.fillWidth: true
                text: qsTr("Save the programmer in this scene")
                iconGlyph: Theme.iconSave
                enabled: Core.programmer.hasValues || Core.programmer.fxLayers.length > 0
                onClicked: {
                    if (Core.programmer.updateScene(editor.info.id))
                        editor.saved()
                }
            }
        }
    }

    // ------------------------------------------------------------ the selected layer
    Panel {
        Layout.fillWidth: true
        Layout.fillHeight: true
        title: editor.layers.length > 0 ? qsTr("FX on %1").arg(Theme.fxParamLabel(editor.currentLayer.param || "", "")) : qsTr("FX")

        ColumnLayout {
            anchors { fill: parent; margins: 10 }
            spacing: 10
            visible: editor.layers.length > 0

            RowLayout {
                Layout.fillWidth: true
                // what some fixtures do their own way (made in the programmer)
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    visible: (editor.currentLayer.ownCount || 0) > 0
                    text: qsTr("%n fixture(s) do this FX their own way. A change of the shape, size, direction or phase here makes them all do the same.", "", editor.currentLayer.ownCount || 0)
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                }
                Item { Layout.fillWidth: true; visible: (editor.currentLayer.ownCount || 0) === 0 }
                AppButton {
                    text: qsTr("Link")
                    iconGlyph: Theme.iconLink
                    implicitHeight: 28
                    checkable: true
                    checked: !!editor.currentLayer.linked
                    ToolTip.text: qsTr("Linked FX share the speed and the timing"); ToolTip.visible: hovered
                    onClicked: Core.effects.setFxLayer(editor.info.id, editor.current, { linked: checked })
                }
            }

            Flickable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentHeight: controls.implicitHeight
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar {}
                FxControls {
                    id: controls
                    width: parent.width - 12
                    fx: editor.currentLayer
                    sets: editor.currentLayer.sets || 1
                    tappedBpm: editor.tappedBpm
                    onEdit: (props) => Core.effects.setFxLayer(editor.info.id, editor.current, props)
                }
            }

            FxPreview {
                Layout.fillWidth: true
                Layout.preferredHeight: editor.currentLayer.panTilt ? 130 : 80
                fx: editor.currentLayer
                showBpm: editor.tappedBpm > 0 ? editor.tappedBpm : Core.bpm
                sample: (pos) => Core.effects.fxPreview(editor.info.id, editor.current, pos)
            }
        }
    }
}
