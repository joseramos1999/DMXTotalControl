/*
  DMX Total Control
  UpdateBanner.qml

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
 * A new version on GitHub: a bar above the pages to see what's new, download
 * it and install it (now or when the program closes). "Later" hides it for
 * that version; Options still shows it.
 */
Rectangle {
    id: banner

    /** The user wants the downloaded version installed now (the program closes) */
    signal installNow()

    readonly property int phase: Core.updates.state
    readonly property bool known: Core.updates.latestVersion.length > 0
    readonly property bool wanted: (phase === UpdateChecker.Available && !Core.updates.dismissed) ||
                                   phase === UpdateChecker.Downloading || phase === UpdateChecker.Ready ||
                                   (phase === UpdateChecker.Failed && known)

    implicitHeight: 40
    color: Qt.alpha(Theme.accent, 0.16)
    Rectangle { width: parent.width; height: 1; anchors.bottom: parent.bottom; color: Theme.accent }

    // the download, along the bottom edge
    Rectangle {
        anchors { left: parent.left; bottom: parent.bottom }
        height: 3
        width: parent.width * Core.updates.progress
        visible: banner.phase === UpdateChecker.Downloading
        color: Theme.accent
    }

    RowLayout {
        anchors { fill: parent; leftMargin: 14; rightMargin: 8 }
        spacing: 10

        Image {
            Layout.preferredWidth: 22
            Layout.preferredHeight: 22
            source: "qrc:/icons/dtc-48.png"
            sourceSize: Qt.size(44, 44)
            smooth: true
        }
        Text {
            Layout.fillWidth: true
            text: {
                const v = Core.updates.latestVersion
                switch (banner.phase) {
                case UpdateChecker.Downloading:
                    return qsTr("Downloading version %1… %2 %").arg(v).arg(Math.round(Core.updates.progress * 100))
                case UpdateChecker.Ready:
                    return Core.updates.installOnExit ? qsTr("Version %1 will be installed when the program closes").arg(v)
                                                      : qsTr("Version %1 is downloaded and checked, ready to install").arg(v)
                case UpdateChecker.Failed:
                    return qsTr("Version %1 could not be downloaded: %2").arg(v).arg(Core.updates.error)
                default:
                    return Core.updates.prerelease ? qsTr("A test version of DMX Total Control is out: %1").arg(v)
                                                   : qsTr("A new version of DMX Total Control is out: %1").arg(v)
                }
            }
            color: banner.phase === UpdateChecker.Failed ? Theme.warning : Theme.text
            elide: Text.ElideRight
        }
        AppButton {
            text: qsTr("What's new")
            implicitHeight: 28
            flat: true
            onClicked: notes.open()
        }
        // released without a digest: only from its page
        AppButton {
            visible: (banner.phase === UpdateChecker.Available || banner.phase === UpdateChecker.Failed) && !Core.updates.installable
            text: qsTr("Open on GitHub")
            implicitHeight: 28
            onClicked: Core.updates.openPage()
        }
        AppButton {
            visible: (banner.phase === UpdateChecker.Available || banner.phase === UpdateChecker.Failed) && Core.updates.installable
            text: banner.phase === UpdateChecker.Failed ? qsTr("Try again") : qsTr("Download and install")
            iconGlyph: Theme.iconDownload
            implicitHeight: 28
            primary: true
            onClicked: Core.updates.download()
        }
        AppButton {
            visible: banner.phase === UpdateChecker.Ready
            text: qsTr("Install now")
            implicitHeight: 28
            primary: true
            ToolTip.text: qsTr("The program closes (it asks to save the show first) and opens again when the update ends")
            ToolTip.visible: hovered
            onClicked: banner.installNow()
        }
        AppButton {
            visible: banner.phase === UpdateChecker.Ready
            text: Core.updates.installOnExit ? qsTr("Not when closing") : qsTr("When closing")
            implicitHeight: 28
            onClicked: Core.updates.setInstallOnExit(!Core.updates.installOnExit)
        }
        AppButton {
            visible: banner.phase === UpdateChecker.Available || banner.phase === UpdateChecker.Failed
            text: qsTr("Later")
            implicitHeight: 28
            flat: true
            ToolTip.text: qsTr("No more notices for this version (Options › Updates still has it)")
            ToolTip.visible: hovered
            onClicked: Core.updates.dismiss()
        }
    }

    ReleaseNotes { id: notes }
}
