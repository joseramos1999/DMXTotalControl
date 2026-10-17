/*
  DMX Total Control
  FirstStepsGuide.qml

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
 * The first steps: a card in a corner that goes through the pages of the
 * basic way of working (patch, program, record, live, playbacks), with the
 * practice show of the examples. The pages stay usable while it is open.
 */
Popup {
    id: guide

    /** The pages of the window, to show them (Main sets them) */
    property int pagePatch: 0
    property int pageProgram: 0
    property int pageLive: 0
    property int pagePlaybacks: 0

    /** A page to show */
    signal pageWanted(int page)
    /** The practice show is wanted (Main asks to save the show first) */
    signal practiceWanted(url url)
    /** The manual, at the first steps */
    signal manualWanted()

    property int step: 0
    readonly property var steps: [
        { page: -1, title: qsTr("First steps"),
          text: qsTr("Five steps to light a stage: the fixtures, a look, a scene, the live page and the playbacks. Open the practice show to follow them, or go on with your own show.") },
        { page: guide.pagePatch, title: qsTr("1. Your fixtures"),
          text: qsTr("Patch holds the fixtures of the show. The practice show has 4 PARs and 2 moving heads. Add more with «Add fixtures»: the model, its mode, the universe, the address and how many.") },
        { page: guide.pageProgram, title: qsTr("2. A look"),
          text: qsTr("In Program, select fixtures (click them, or a group like «PARs») and give them dimmer, colour or position. It goes out at once, over everything else, while you program.") },
        { page: guide.pageProgram, title: qsTr("3. A scene"),
          text: qsTr("«Record…» keeps what the programmer holds as a scene, in a group: choose «Mis escenas». Then «Reset all» clears the programmer so the scenes can play.") },
        { page: guide.pageLive, title: qsTr("4. Live"),
          text: qsTr("Live shows a column for each group. Click a scene to play it; in an exclusive group the next one crossfades. The master of the group dims all of it.") },
        { page: guide.pagePlaybacks, title: qsTr("5. Playbacks"),
          text: qsTr("The playbacks are faders with GO for cuelists and scenes. Arm «Record» on this page and click a playback: the programmer becomes a cue. GO plays the next cue.") },
        { page: -1, title: qsTr("Ready"),
          text: qsTr("That's the way of working. F12, or the «?» of the top bar, opens the manual on the page you are on. Save your show with Ctrl+S.") }
    ]
    readonly property var current: steps[Math.min(step, steps.length - 1)]

    function start() {
        step = 0
        open()
    }
    function go(to) {
        step = Math.max(0, Math.min(steps.length - 1, to))
        if (current.page >= 0)
            pageWanted(current.page)
    }

    parent: Overlay.overlay
    x: parent ? parent.width - width - 24 : 0
    y: parent ? parent.height - height - 24 : 0
    width: 380
    padding: 16
    modal: false
    closePolicy: Popup.NoAutoClose

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surfaceRaised
        border.color: Theme.accent
        border.width: 2
    }

    contentItem: ColumnLayout {
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Image {
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                source: "qrc:/icons/dtc-48.png"
                sourceSize: Qt.size(48, 48)
                smooth: true
            }
            Text {
                Layout.fillWidth: true
                text: guide.current.title
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
                elide: Text.ElideRight
            }
            Text {
                text: qsTr("%1 of %2").arg(guide.step + 1).arg(guide.steps.length)
                color: Theme.textFaint
                font.pixelSize: Theme.fontSmall
            }
            AppButton {
                iconGlyph: Theme.iconClose
                flat: true
                implicitHeight: 26
                ToolTip.text: qsTr("Close the guide (Options › Help opens it again)"); ToolTip.visible: hovered
                onClicked: guide.close()
            }
        }
        Text {
            Layout.fillWidth: true
            text: guide.current.text
            wrapMode: Text.WordWrap
            color: Theme.text
            font.pixelSize: Theme.fontNormal
            lineHeight: 1.15
        }
        // the practice show, at the start
        AppButton {
            readonly property var practice: Core.examples().find(e => e.name === "Primeros pasos")
            visible: guide.step === 0 && practice !== undefined
            Layout.fillWidth: true
            text: qsTr("Open the practice show")
            iconGlyph: Theme.iconOpen
            onClicked: { guide.practiceWanted(practice.url); guide.go(1) }
        }
        AppButton {
            visible: guide.step === guide.steps.length - 1
            Layout.fillWidth: true
            text: qsTr("Open the manual")
            iconGlyph: Theme.iconList
            onClicked: { guide.manualWanted(); guide.close() }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            AppButton {
                text: qsTr("Back")
                enabled: guide.step > 0
                flat: true
                onClicked: guide.go(guide.step - 1)
            }
            Item { Layout.fillWidth: true }
            AppButton {
                text: guide.step === guide.steps.length - 1 ? qsTr("Done") : qsTr("Next")
                primary: true
                onClicked: guide.step === guide.steps.length - 1 ? guide.close() : guide.go(guide.step + 1)
            }
        }
    }
}
