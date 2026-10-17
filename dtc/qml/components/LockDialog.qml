/*
  DMX Total Control
  LockDialog.qml

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
 * Lock the setup, the programming or the editing of the SHOW
 * so nothing changes by mistake, with a password or without. Also asks the
 * password to unlock (screens, or the SHOW kiosk).
 */
Popup {
    id: dialog

    /** "lock" or "unlock" */
    property string mode: "lock"
    property var onUnlocked: null
    property bool wrong: false
    property var chosen: ({ setup: true, program: false, show: true })

    readonly property var screens: [
        { key: "setup", name: qsTr("Setup"), hint: qsTr("Patch, Fixtures, Outputs, Mappings and Options") },
        { key: "program", name: qsTr("Programming"), hint: qsTr("Program and Scenes") },
        { key: "show", name: qsTr("SHOW editing"), hint: qsTr("The SHOW pages are played, not changed") }
    ]

    function openLock() {
        mode = "lock"
        wrong = false
        password.text = ""
        let locked = Core.settings.lockedScreens
        chosen = locked.length > 0 ? { setup: locked.indexOf("setup") >= 0, program: locked.indexOf("program") >= 0,
                                       show: locked.indexOf("show") >= 0 }
                                   : { setup: true, program: false, show: true }
        open()
    }
    /** Ask the password (if there is one), then run $callback */
    function openUnlock(callback) {
        if (!Core.settings.hasLockPassword) {
            if (callback) callback()
            return
        }
        mode = "unlock"
        wrong = false
        onUnlocked = callback
        password.text = ""
        open()
        password.forceActiveFocus()
    }
    function accept() {
        if (mode === "unlock") {
            if (!Core.settings.checkLockPassword(password.text)) {
                wrong = true
                password.selectAll()
                return
            }
            let cb = onUnlocked
            close()
            if (cb) cb()
            return
        }
        // lock: a new password if one was typed
        if (password.text.length > 0)
            Core.settings.setLockPassword(password.text)
        let list = []
        for (let s of screens)
            if (chosen[s.key])
                list.push(s.key)
        Core.settings.lockedScreens = list
        close()
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 420
    modal: true
    padding: 18
    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    contentItem: ColumnLayout {
        spacing: 12

        RowLayout {
            spacing: 10
            IconLabel { icon: dialog.mode === "lock" ? Theme.iconLock : Theme.iconUnlock; size: 18; color: Theme.warning }
            Text {
                text: dialog.mode === "lock" ? qsTr("Lock") : qsTr("Unlock")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
            }
        }

        Text {
            Layout.fillWidth: true
            visible: dialog.mode === "lock"
            wrapMode: Text.WordWrap
            text: qsTr("So nothing changes by mistake during the show. Live and the SHOW keep working.")
            color: Theme.textDim
        }
        Repeater {
            model: dialog.mode === "lock" ? dialog.screens : []
            delegate: RowLayout {
                required property var modelData
                Layout.fillWidth: true
                spacing: 10
                AppSwitch {
                    checked: !!dialog.chosen[parent.modelData.key]
                    onToggled: {
                        let c = Object.assign({}, dialog.chosen)
                        c[parent.modelData.key] = checked
                        dialog.chosen = c
                    }
                }
                Column {
                    Layout.fillWidth: true
                    Text { text: parent.parent.modelData.name; color: Theme.text; font.weight: Font.DemiBold }
                    Text { width: parent.width; text: parent.parent.modelData.hint; color: Theme.textFaint; font.pixelSize: Theme.fontSmall; wrapMode: Text.WordWrap }
                }
            }
        }

        Text {
            text: dialog.mode === "unlock" ? qsTr("Password")
                  : Core.settings.hasLockPassword ? qsTr("New password (empty: keep the current one)")
                  : qsTr("Password (optional)")
            color: Theme.textDim
            font.pixelSize: Theme.fontSmall
        }
        TextField {
            id: password
            Layout.fillWidth: true
            echoMode: TextInput.Password
            color: Theme.text
            onAccepted: dialog.accept()
        }
        Text {
            visible: dialog.wrong
            text: qsTr("Wrong password")
            color: Theme.danger
        }

        RowLayout {
            Layout.fillWidth: true
            AppButton {
                visible: dialog.mode === "lock" && Core.settings.hasLockPassword
                text: qsTr("Remove the password")
                flat: true
                onClicked: dialog.openUnlock(() => Core.settings.setLockPassword(""))
            }
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Cancel"); onClicked: dialog.close() }
            AppButton {
                text: dialog.mode === "lock" ? qsTr("Lock") : qsTr("Unlock")
                iconGlyph: dialog.mode === "lock" ? Theme.iconLock : Theme.iconUnlock
                primary: true
                onClicked: dialog.accept()
            }
        }
    }
}
