/*
  DMX Total Control
  CueExistsDialog.qml

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
 * Recording on a cue number that exists: merge the
 * programmer into it, replace what it holds, or record a point cue after it.
 * ask(cuelistId, number, opts, done): done(index) when recorded
 */
Popup {
    id: dialog

    property int cuelistId: -1
    property string number: ""
    property var opts: ({})
    property var done: null

    function ask(id, cueNumber, options, callback) {
        cuelistId = id
        number = cueNumber
        opts = options || {}
        done = callback || null
        open()
    }

    function record(mode) {
        let o = Object.assign({}, dialog.opts)
        o.existing = mode
        const index = Core.cuelists.recordCue(dialog.cuelistId, dialog.number, o)
        close()
        if (index >= 0 && dialog.done)
            dialog.done(index)
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 460
    modal: true
    padding: 18

    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    // a choice: title and what it does
    component Choice: Rectangle {
        id: choice
        property string title: ""
        property string detail: ""
        property string glyph: ""
        signal chosen()
        implicitHeight: choiceColumn.implicitHeight + 16
        radius: Theme.radiusSmall
        color: choiceMouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised
        border.color: choiceMouse.containsMouse ? Theme.accent : Theme.border
        RowLayout {
            anchors { fill: parent; leftMargin: 12; rightMargin: 12 }
            spacing: 12
            IconLabel { icon: choice.glyph; size: 15; color: Theme.accent; Layout.alignment: Qt.AlignVCenter }
            ColumnLayout {
                id: choiceColumn
                Layout.fillWidth: true
                spacing: 2
                Text { text: choice.title; color: Theme.text; font.weight: Font.DemiBold }
                Text {
                    Layout.fillWidth: true
                    text: choice.detail
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                    wrapMode: Text.WordWrap
                }
            }
        }
        MouseArea {
            id: choiceMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: choice.chosen()
        }
    }

    contentItem: ColumnLayout {
        spacing: 10

        Text {
            text: qsTr("Cue %1 already exists").arg(dialog.number)
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("What should the programmer do with it?")
            color: Theme.textDim
        }
        Choice {
            Layout.fillWidth: true
            title: qsTr("Merge")
            detail: qsTr("The programmer goes into the cue; what it doesn't hold stays as it is")
            glyph: Theme.iconLayers
            onChosen: dialog.record("merge")
        }
        Choice {
            Layout.fillWidth: true
            title: qsTr("Replace")
            detail: qsTr("The cue holds only what the programmer changes; its name and times stay")
            glyph: Theme.iconRefresh
            onChosen: dialog.record("replace")
        }
        Choice {
            Layout.fillWidth: true
            title: qsTr("Insert after")
            detail: qsTr("A new cue between this one and the next one")
            glyph: Theme.iconPlus
            onChosen: dialog.record("insertAfter")
        }
        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Cancel"); onClicked: dialog.close() }
        }
    }
}
