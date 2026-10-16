/*
  DMX Total Control
  ColourFxEditor.qml

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
import QtQuick.Dialogs
import DMXTotalControl
import DMXTotalControl.Backend

/**
 * Colour FX / Value FX / text mappings: an effect that runs
 * along the fixtures in selection order [RGBMatrix and its scripts]
 */
Panel {
    id: editor

    property var info: ({})
    property int pickingColor: -1

    readonly property var effects: Core.effects.colourEffects()
    readonly property var swatches: [
        "#ffffff", "#ffd8a8", "#ff0000", "#ff6a00", "#ffbf00", "#ffff00", "#80ff00", "#00ff00",
        "#00ffaa", "#00ffff", "#0080ff", "#0000ff", "#6a00ff", "#bf00ff", "#ff00ff", "#ff0080"
    ]

    function set(key, value) { Core.effects.setColourFx(info.id, key, value) }

    // what the effect sends, head by head (live while it plays)
    property var live: ({})
    onInfoChanged: live = Core.effects.liveView(info.id)
    // fast while it plays; stopped, only to see it start (or the heads
    // change from elsewhere)
    Timer {
        interval: editor.live.running ? 60 : 400
        repeat: true
        running: editor.visible && editor.info.id !== undefined
        onTriggered: editor.live = Core.effects.liveView(editor.info.id)
    }

    title: editor.live.running ? qsTr("Colour effect — live") : qsTr("Colour effect")

    ColumnLayout {
        anchors { fill: parent; margins: 14 }
        spacing: 12

        // what doesn't fit scrolls above the footer (scripts with many
        // parameters, a tall 2D plan)
        Flickable {
            id: colourScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentHeight: colourContent.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { policy: colourScroll.contentHeight > colourScroll.height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded }

            ColumnLayout {
                id: colourContent
                width: colourScroll.width - 12
                spacing: 12

                // the heads where the effect places them: in a row, or like the 2D plan
                // (a line below them for the hint: on the plan, heads at the right
                // edge were under it)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: livePreview.height + 8 + 20
                    radius: Theme.radiusSmall
                    color: Theme.background
                    border.color: Theme.border

                    Item {
                        id: livePreview
                        readonly property int cols: Math.max(1, editor.live.cols || 1)
                        readonly property int rows: Math.max(1, editor.live.rows || 1)
                        readonly property real cell: Math.max(6, Math.min(44, (parent.width - 16) / cols, 150 / rows))
                        anchors { top: parent.top; topMargin: 8; horizontalCenter: parent.horizontalCenter }
                        width: cols * cell
                        height: rows * cell

                        // one cell per head, kept from one reading to the next (a new
                        // list as model would make every cell again, 16 times a second)
                        Repeater {
                            model: (editor.live.items || []).length
                            Rectangle {
                                id: headCell
                                required property int index
                                readonly property var head: (editor.live.items || [])[index] || ({ x: 0, y: 0, color: "black", index: 0 })
                                x: head.x * livePreview.cell + 1
                                y: head.y * livePreview.cell + 1
                                width: livePreview.cell - 2
                                height: livePreview.cell - 2
                                radius: Math.min(4, width / 4)
                                color: head.color
                                border.color: Theme.borderStrong
                                Text {
                                    anchors.centerIn: parent
                                    visible: livePreview.cell >= 18
                                    text: headCell.head.index
                                    font.pixelSize: 9
                                    color: Theme.textOn(headCell.head.color)
                                }
                            }
                        }
                    }
                    Text {
                        anchors { right: parent.right; bottom: parent.bottom; rightMargin: 6; bottomMargin: 3 }
                        visible: !editor.live.running
                        text: qsTr("Press Try to see it")
                        color: Theme.textFaint
                        font.pixelSize: 10
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: 12
                    rowSpacing: 10

                    Text { text: qsTr("Effect"); color: Theme.textDim }
                    ComboBox {
                        Layout.fillWidth: true
                        model: editor.effects
                        currentIndex: Math.max(0, editor.effects.indexOf(editor.info.effect))
                        onActivated: (i) => editor.set("effect", editor.effects[i])
                    }

                    Text { text: qsTr("Works on"); color: Theme.textDim }
                    ComboBox {
                        Layout.fillWidth: true
                        model: [qsTr("Colour (RGB)"), qsTr("White"), qsTr("Amber"), qsTr("UV"), qsTr("Dimmer (value)"), qsTr("Shutter")]
                        currentIndex: editor.info.mode || 0
                        onActivated: (i) => editor.set("mode", i)
                    }

                    Text { text: qsTr("Colours"); color: Theme.textDim; visible: (editor.info.acceptColors || 0) > 0 }
                    Row {
                        visible: (editor.info.acceptColors || 0) > 0
                        spacing: 8
                        Repeater {
                            model: editor.info.acceptColors || 0
                            delegate: Rectangle {
                                required property int index
                                readonly property string hex: (editor.info.colors || [])[index] || ""
                                width: 42; height: 42; radius: 8
                                color: hex.length ? hex : "transparent"
                                border.color: editor.pickingColor === index ? "white" : Theme.borderStrong
                                border.width: editor.pickingColor === index ? 3 : 1
                                Text {
                                    anchors.centerIn: parent
                                    visible: parent.hex.length === 0
                                    text: "∅"
                                    color: Theme.textFaint
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: { editor.pickingColor = index; colorPopup.open() }
                                }
                            }
                        }
                    }

                    Text { text: qsTr("Text"); color: Theme.textDim; visible: editor.info.text !== undefined }
                    TextField {
                        Layout.fillWidth: true
                        visible: editor.info.text !== undefined
                        text: editor.info.text || ""
                        color: Theme.text
                        onEditingFinished: editor.set("text", text)
                    }

                    // images (media mappings)
                    Text { text: qsTr("Image"); color: Theme.textDim; visible: editor.info.image !== undefined }
                    RowLayout {
                        visible: editor.info.image !== undefined
                        Layout.fillWidth: true
                        Text {
                            Layout.fillWidth: true
                            text: editor.info.image ? editor.info.image.split(/[\\/]/).pop() : qsTr("No image")
                            color: editor.info.image ? Theme.text : Theme.textFaint
                            elide: Text.ElideMiddle
                        }
                        AppButton { text: qsTr("Choose…"); iconGlyph: Theme.iconOpen; onClicked: imageDialog.open() }
                    }
                    Text { text: qsTr("Animation"); color: Theme.textDim; visible: editor.info.image !== undefined }
                    ComboBox {
                        Layout.fillWidth: true
                        visible: editor.info.image !== undefined
                        readonly property var styles: ["Static", "Horizontal", "Vertical", "Animation"]
                        model: [qsTr("Still"), qsTr("Scroll sideways"), qsTr("Scroll up and down"), qsTr("Animated (GIF)")]
                        currentIndex: Math.max(0, styles.indexOf(editor.info.animation))
                        onActivated: (i) => editor.set("animation", styles[i])
                    }

                    // one row in selection order, or where the fixtures stand on the plan
                    Text { text: qsTr("Layout"); color: Theme.textDim }
                    Row {
                        spacing: 0
                        AppButton {
                            text: qsTr("In a row")
                            iconGlyph: Theme.iconList
                            implicitHeight: 30
                            highlighted: editor.info.layout !== "plan"
                            onClicked: editor.set("layout", "line")
                        }
                        AppButton {
                            text: qsTr("On the 2D plan")
                            iconGlyph: Theme.iconGrid
                            implicitHeight: 30
                            highlighted: editor.info.layout === "plan"
                            onClicked: editor.set("layout", "plan")
                        }
                    }
                }

                // script parameters
                Text {
                    visible: (editor.info.properties || []).length > 0
                    text: qsTr("Parameters")
                    color: Theme.textDim
                    font.weight: Font.Bold
                }
                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: 12
                    rowSpacing: 8
                    Repeater {
                        model: editor.info.properties || []
                        delegate: Item {
                            required property var modelData
                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            implicitHeight: 36
                            RowLayout {
                                anchors.fill: parent
                                spacing: 12
                                Text { Layout.preferredWidth: 160; text: modelData.label; color: Theme.textDim; elide: Text.ElideRight }
                                ComboBox {
                                    Layout.fillWidth: true
                                    visible: modelData.type === "list"
                                    model: modelData.values || []
                                    currentIndex: Math.max(0, (modelData.values || []).indexOf(modelData.value))
                                    onActivated: (i) => Core.effects.setColourFxProperty(editor.info.id, modelData.name, modelData.values[i])
                                }
                                SpinBox {
                                    Layout.fillWidth: true
                                    visible: modelData.type === "range"
                                    from: modelData.min || 0
                                    to: modelData.max || 255
                                    editable: true
                                    value: parseInt(modelData.value) || 0
                                    onValueModified: Core.effects.setColourFxProperty(editor.info.id, modelData.name, String(value))
                                }
                                TextField {
                                    Layout.fillWidth: true
                                    visible: modelData.type === "float" || modelData.type === "string"
                                    text: modelData.value
                                    color: Theme.text
                                    onEditingFinished: Core.effects.setColourFxProperty(editor.info.id, modelData.name, text)
                                }
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Text {
                Layout.fillWidth: true
                text: qsTr("%n fixture(s), in selection order", "", editor.info.fixtures || 0)
                color: Theme.textDim
            }
            AppButton {
                text: qsTr("Use the programmer selection")
                iconGlyph: Theme.iconSelect
                onClicked: Core.effects.useSelection(editor.info.id)
            }
        }
    }

    Popup {
        id: colorPopup
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        padding: 16
        background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }
        contentItem: ColumnLayout {
            spacing: 10
            Text { text: qsTr("Colour %1").arg(editor.pickingColor + 1); color: Theme.text; font.weight: Font.Bold }
            Grid {
                columns: 8
                spacing: 6
                Repeater {
                    model: editor.swatches
                    delegate: Rectangle {
                        required property string modelData
                        width: 34; height: 34; radius: 6
                        color: modelData
                        border.color: "#00000040"
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: { editor.set("color" + editor.pickingColor, modelData); colorPopup.close() }
                        }
                    }
                }
            }
            RowLayout {
                Text { text: qsTr("Hue"); color: Theme.textDim }
                HSlider {
                    id: huePick
                    Layout.preferredWidth: 220
                    from: 0; to: 1
                    trackGradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "#ff0000" }
                        GradientStop { position: 0.17; color: "#ffff00" }
                        GradientStop { position: 0.33; color: "#00ff00" }
                        GradientStop { position: 0.5; color: "#00ffff" }
                        GradientStop { position: 0.67; color: "#0000ff" }
                        GradientStop { position: 0.83; color: "#ff00ff" }
                        GradientStop { position: 1.0; color: "#ff0000" }
                    }
                }
                AppButton {
                    text: qsTr("Use")
                    onClicked: { editor.set("color" + editor.pickingColor, Qt.hsva(huePick.value, 1, 1, 1).toString()); colorPopup.close() }
                }
            }
            AppButton {
                text: qsTr("No colour")
                visible: editor.pickingColor > 0
                onClicked: { editor.set("color" + editor.pickingColor, ""); colorPopup.close() }
            }
        }
    }

    FileDialog {
        id: imageDialog
        title: qsTr("Image for the effect")
        nameFilters: [qsTr("Images (*.png *.jpg *.jpeg *.gif *.bmp)")]
        onAccepted: editor.set("image", selectedFile.toString())
    }
}
