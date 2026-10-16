/*
  DMX Total Control
  PlaybackAssignDialog.qml

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

/** Put a cuelist or any scene on a playback: openFor(page, slot) */
Popup {
    id: dialog

    /** The playback page (-1: the current one) and slot */
    property int page: -1
    property int slot: 0
    readonly property var info: { Core.playbacks.revision; return Core.playbacks.slotInfo(page, slot) }

    function openFor(page, slot) {
        // the page it was opened on, even if the current page changes
        // meanwhile (a page that is gone: nothing, never the current page)
        const p = Core.playbacks.resolvePage(page)
        if (p < 0)
            return
        dialog.page = p
        dialog.slot = slot
        open()
    }
    function assign(id) {
        if (id === undefined || id < 0)
            return
        Core.playbacks.assign(page, slot, id)
        close()
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 480
    height: Math.min(640, parent ? parent.height - 40 : 640)
    modal: true
    padding: 16

    onOpened: {
        browser.clearSearch()
        browser.selectedIds = info.empty ? [] : [info.functionId]
    }

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surface
        border.color: Theme.borderStrong
    }

    contentItem: ColumnLayout {
        spacing: 10

        Text {
            Layout.fillWidth: true
            text: qsTr("Playback %1").arg(dialog.slot + 1)
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
            elide: Text.ElideRight
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: dialog.info.empty ? qsTr("Choose the cuelist or scene it plays (double click to put it there).")
                                    : qsTr("It plays “%1”: the one you choose takes its place.").arg(dialog.info.name || "")
            color: Theme.textDim
            font.pixelSize: Theme.fontSmall
        }

        SceneBrowser {
            id: browser
            Layout.fillWidth: true
            Layout.fillHeight: true
            typeFilter: true
            currentId: dialog.info.empty ? -1 : dialog.info.functionId
            emptyText: qsTr("There are no scenes yet. Record a cuelist from the programmer, or create a scene in Scenes.")
            onSceneDoubleClicked: (id) => dialog.assign(id)
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Cancel"); onClicked: dialog.close() }
            AppButton {
                text: qsTr("Put it on playback %1").arg(dialog.slot + 1)
                primary: true
                enabled: browser.selectedIds.length === 1
                onClicked: dialog.assign(browser.selectedIds[0])
            }
        }
    }
}
