/*
  DMX Total Control
  RackEditor.qml

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

/** Effect rack: several scenes and effects played as one [Collection] */
RowLayout {
    id: editor

    property var info: ({})
    readonly property var items: info.items || []
    // the racks, steps and super scenes that play this rack: it can't play them
    readonly property var players: info.id !== undefined ? Core.effects.playersOf(info.id) : []

    spacing: 12

    Panel {
        Layout.fillWidth: true
        Layout.fillHeight: true
        title: qsTr("In the rack (%1)").arg(editor.items.length)

        ListView {
            id: inRack
            anchors { fill: parent; margins: 8 }
            clip: true
            spacing: 3
            model: editor.items
            ScrollBar.vertical: ScrollBar {}

            delegate: Rectangle {
                required property var modelData
                width: inRack.width
                height: 42
                radius: Theme.radiusSmall
                color: Theme.surfaceRaised
                border.color: Theme.border
                RowLayout {
                    anchors { fill: parent; leftMargin: 10; rightMargin: 6 }
                    Rectangle { width: 4; height: 24; radius: 2; color: Theme.functionColor(modelData.type) }
                    Text { Layout.fillWidth: true; text: modelData.name; color: Theme.text; elide: Text.ElideRight }
                    AppButton { iconGlyph: Theme.iconClose; flat: true; implicitHeight: 28
                                onClicked: Core.effects.removeFromRack(editor.info.id, modelData.id) }
                }
            }

            Text {
                anchors.centerIn: parent
                width: parent.width - 40
                visible: inRack.count === 0
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: qsTr("Add scenes and effects from the list: they play together, each on its own fixtures.")
                color: Theme.textFaint
            }
        }
    }

    Panel {
        Layout.preferredWidth: 340
        Layout.fillHeight: true
        title: qsTr("Add")

        SceneBrowser {
            anchors { fill: parent; margins: 8 }
            compact: true
            rowIcon: Theme.iconPlus
            functions: Core.functions.filter(f => f.id !== editor.info.id && editor.items.every(i => i.id !== f.id)
                                                 && editor.players.indexOf(f.id) < 0)
            onSceneClicked: (id) => Core.effects.addToRack(editor.info.id, id)
        }
    }
}
