/*
  DMX Total Control
  TextPrompt.qml

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

/** Ask for a line of text: ask(title, initialText, callback) */
Popup {
    id: prompt

    property string title: ""
    property var callback: null

    function ask(title, text, cb) {
        prompt.title = title
        field.text = text || ""
        callback = cb
        open()
        field.forceActiveFocus()
        field.selectAll()
    }

    function accept() {
        if (field.text.trim().length === 0)
            return
        if (callback)
            callback(field.text.trim())
        close()
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 380
    modal: true
    padding: 18

    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    contentItem: ColumnLayout {
        spacing: 12
        Text {
            text: prompt.title
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
        }
        TextField {
            id: field
            Layout.fillWidth: true
            color: Theme.text
            background: Rectangle { radius: Theme.radiusSmall; color: Theme.background; border.color: field.activeFocus ? Theme.accent : Theme.border }
            onAccepted: prompt.accept()
        }
        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Cancel"); onClicked: prompt.close() }
            AppButton { text: qsTr("OK"); primary: true; onClicked: prompt.accept() }
        }
    }
}
