/*
  DMX Total Control
  TrackingSheet.qml

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
 * The tracking sheet of a cue: what the stage shows on it, fixture by
 * fixture. What the cue changes stands out, what it tracks says from which
 * cue, block values are amber and releases are struck through. Chips can be
 * picked to release them in the cue or take them out of it.
 */
Item {
    id: sheet

    property int cuelistId: -1
    property int cueIndex: -1

    property int revision: 0
    /** The picked chips: "fixture:channel" or "fixture:axis" */
    property var picked: []

    onCueIndexChanged: picked = []
    onCuelistIdChanged: picked = []

    readonly property var rows: {
        revision
        Core.palettes.palettes
        return cuelistId >= 0 && cueIndex >= 0 ? Core.cuelists.trackingSheet(cuelistId, cueIndex) : []
    }
    readonly property var cuePalettes: {
        revision
        Core.palettes.palettes
        return cuelistId >= 0 && cueIndex >= 0 ? Core.cuelists.cuePalettes(cuelistId, cueIndex) : []
    }
    readonly property int changedCount: {
        let n = 0
        for (let r of rows) {
            for (let c of r.channels) if (c.state === "changed" || c.state === "blocked") n++
            for (let f of r.fx) if (f.state === "changed" || f.state === "blocked") n++
        }
        return n
    }

    function togglePick(key) {
        let p = picked.slice()
        let i = p.indexOf(key)
        if (i >= 0) p.splice(i, 1); else p.push(key)
        picked = p
    }

    Connections {
        target: Core.cuelists
        function onEdited(id) { if (id === sheet.cuelistId) sheet.revision++ }
    }

    // a channel or an FX of a fixture
    component Chip: Rectangle {
        id: chip
        property string label: ""
        property string value: ""
        property string chipState: "tracked"
        property string from: ""
        property bool isPicked: false
        signal clicked()

        readonly property color tint: chipState === "changed" ? Theme.accent
                                      : chipState === "blocked" ? Theme.warning : Theme.border
        width: chipRow.implicitWidth + 16
        height: 24
        radius: 12
        color: chipState === "changed" ? Qt.alpha(Theme.accent, 0.22)
               : chipState === "blocked" ? Qt.alpha(Theme.warning, 0.18)
               : chipMouse.containsMouse ? Theme.surfaceHover : "transparent"
        border.color: isPicked ? Theme.text : tint
        border.width: isPicked ? 2 : 1

        Row {
            id: chipRow
            anchors.centerIn: parent
            spacing: 5
            Text {
                text: chip.label
                color: chip.chipState === "released" ? Theme.textFaint
                       : chip.chipState === "tracked" ? Theme.textDim : Theme.text
                font.pixelSize: Theme.fontSmall
                font.strikeout: chip.chipState === "released"
            }
            Text {
                visible: chip.value.length > 0
                text: chip.value
                color: chip.chipState === "tracked" ? Theme.textDim : Theme.text
                font.family: Theme.monoFamily
                font.pixelSize: Theme.fontSmall
                font.weight: Font.DemiBold
            }
            Text {
                visible: chip.chipState === "released"
                text: "[R]"
                color: Theme.danger
                font.pixelSize: Theme.fontSmall
                font.weight: Font.Bold
            }
            // the cue it comes from
            Text {
                visible: chip.chipState === "tracked" && chip.from.length > 0
                text: "Q" + chip.from
                color: Theme.textFaint
                font.family: Theme.monoFamily
                font.pixelSize: 10
            }
        }
        MouseArea {
            id: chipMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: chip.clicked()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        // what the colours say
        Flow {
            Layout.fillWidth: true
            spacing: 6
            Repeater {
                model: [{ s: "changed", t: qsTr("Changes") }, { s: "tracked", t: qsTr("Tracked") },
                        { s: "blocked", t: qsTr("Block") }, { s: "released", t: qsTr("Released") }]
                Chip {
                    required property var modelData
                    label: modelData.t
                    chipState: modelData.s
                    enabled: false
                }
            }
        }

        // the palettes the cue takes values from: they change it when they change
        Flow {
            Layout.fillWidth: true
            visible: sheet.cuePalettes.length > 0
            spacing: 6
            Repeater {
                model: sheet.cuePalettes
                Rectangle {
                    required property var modelData
                    width: paletteText.implicitWidth + 30
                    height: 24
                    radius: 12
                    color: Theme.surfaceRaised
                    border.color: Theme.border
                    Rectangle {
                        anchors { left: parent.left; leftMargin: 8; verticalCenter: parent.verticalCenter }
                        width: 10; height: 10; radius: 5
                        color: modelData.color && modelData.color.length > 0 ? modelData.color : Theme.textFaint
                    }
                    Text {
                        id: paletteText
                        anchors { left: parent.left; leftMargin: 22; verticalCenter: parent.verticalCenter }
                        text: modelData.name
                        color: Theme.text
                        font.pixelSize: Theme.fontSmall
                    }
                }
            }
        }

        ListView {
            id: fixtureList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            boundsBehavior: Flickable.StopAtBounds
            model: sheet.rows
            ScrollBar.vertical: ScrollBar {}

            delegate: Column {
                id: fixtureRow
                required property var modelData
                width: fixtureList.width - 10
                spacing: 4
                Text {
                    text: fixtureRow.modelData.fixtureName
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                    font.weight: Font.Bold
                }
                Flow {
                    width: parent.width
                    spacing: 4
                    Repeater {
                        model: fixtureRow.modelData.channels
                        Chip {
                            required property var modelData
                            label: modelData.name
                            value: modelData.paletteName && modelData.paletteName.length > 0
                                   ? modelData.paletteName : (modelData.value >= 0 ? String(modelData.value) : "")
                            chipState: modelData.state
                            from: modelData.from
                            isPicked: sheet.picked.indexOf(modelData.key) >= 0
                            onClicked: sheet.togglePick(modelData.key)
                        }
                    }
                    Repeater {
                        model: fixtureRow.modelData.fx
                        Chip {
                            required property var modelData
                            label: qsTr("FX %1").arg(Theme.fxParamLabel(modelData.axis, ""))
                            chipState: modelData.state
                            from: modelData.from
                            isPicked: sheet.picked.indexOf(modelData.key) >= 0
                            onClicked: sheet.togglePick(modelData.key)
                        }
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                width: parent.width - 20
                visible: fixtureList.count === 0
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: sheet.cueIndex < 0 ? qsTr("Select a cue") : qsTr("The stage is empty on this cue")
                color: Theme.textFaint
            }
        }

        // what to do with the picked chips
        RowLayout {
            Layout.fillWidth: true
            spacing: 6
            Text {
                Layout.fillWidth: true
                text: sheet.picked.length > 0 ? qsTr("%n picked", "", sheet.picked.length)
                                              : qsTr("Click channels to pick them")
                color: sheet.picked.length > 0 ? Theme.text : Theme.textFaint
                font.pixelSize: Theme.fontSmall
                elide: Text.ElideRight
            }
            AppButton {
                text: qsTr("Release")
                implicitHeight: 28
                enabled: sheet.picked.length > 0
                ToolTip.text: qsTr("[RELEASE] in this cue: from here on they go back to what is below (other scenes or their rest value)")
                ToolTip.visible: hovered
                ToolTip.delay: 400
                onClicked: { Core.cuelists.releaseKeysInCue(sheet.cuelistId, sheet.cueIndex, sheet.picked); sheet.picked = [] }
            }
            AppButton {
                text: qsTr("Take out")
                implicitHeight: 28
                enabled: sheet.picked.length > 0
                ToolTip.text: qsTr("Take them out of this cue: they track from the cues before it again")
                ToolTip.visible: hovered
                ToolTip.delay: 400
                onClicked: { Core.cuelists.removeKeysFromCue(sheet.cuelistId, sheet.cueIndex, sheet.picked); sheet.picked = [] }
            }
            AppButton {
                iconGlyph: Theme.iconClose
                flat: true
                implicitHeight: 28
                visible: sheet.picked.length > 0
                onClicked: sheet.picked = []
            }
        }
    }
}
