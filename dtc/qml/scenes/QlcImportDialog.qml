/*
  DMX Total Control
  QlcImportDialog.qml

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
 * QLC+ cue lists: the chasers, sequences and Virtual Console cue lists of a
 * QLC+ show (this one or another one) become cuelists. Open it with start().
 */
Popup {
    id: dialog

    /** The show they come from: empty for this one */
    property url source: ""
    readonly property bool thisShow: source.toString().length === 0
    /** [{ key, kind, name, functionId, steps, warnings, exists, enabled,
     *  checked }] (Core.qlcCueLists) */
    property var lists: []
    /** The keys of the ones checked */
    property var chosen: []
    /** Another show that couldn't be read */
    property string error: ""

    signal message(string text, bool isError)

    /** Some are cue lists of the Virtual Console: they have buttons */
    readonly property bool hasCueLists: lists.some(l => l.kind === "vcCueList")

    function start() {
        load("")
        replaceCheck.checked = false
        playbacksCheck.checked = true
        controlsCheck.checked = true
        open()
    }
    function load(url) {
        source = url
        lists = Core.qlcCueLists(url)
        error = lists.length === 0 && !thisShow ? Core.lastError : ""
        chosen = lists.filter(l => l.checked).map(l => l.key)
    }
    function toggle(key) {
        chosen = chosen.indexOf(key) >= 0 ? chosen.filter(x => x !== key) : chosen.concat([key])
    }
    function make() {
        const done = Core.importQlcCueLists(source, chosen, {
            replace: thisShow && replaceCheck.checked,
            playbacks: playbacksCheck.checked,
            controls: controlsCheck.checked
        })
        close()
        if (done.length)
            message(done, false)
        else
            message(Core.lastError, true)
    }
    function kindLabel(kind) {
        return kind === "vcCueList" ? qsTr("Virtual Console cue list") : Theme.typeLabel(kind)
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(600, parent ? parent.width - 40 : 600)
    modal: true
    padding: 18

    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

    // where they come from: its name and what it gives
    component SourceChoice: Rectangle {
        id: choice
        property bool selected: false
        property string title: ""
        property string detail: ""
        signal clicked()
        implicitHeight: choiceColumn.implicitHeight + 16
        radius: Theme.radiusSmall
        color: selected ? Qt.alpha(Theme.accent, 0.14) : choiceMouse.containsMouse ? Theme.surfaceHover : Theme.surfaceRaised
        border.color: selected ? Theme.accent : Theme.border
        ColumnLayout {
            id: choiceColumn
            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; leftMargin: 12; rightMargin: 12 }
            spacing: 2
            Text {
                Layout.fillWidth: true
                text: choice.title
                color: Theme.text
                font.weight: Font.DemiBold
                wrapMode: Text.WordWrap
            }
            Text {
                Layout.fillWidth: true
                text: choice.detail
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
                wrapMode: Text.Wrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }
        }
        MouseArea {
            id: choiceMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: choice.clicked()
        }
    }

    contentItem: ColumnLayout {
        spacing: 10

        Text {
            Layout.fillWidth: true
            text: qsTr("QLC+ cue lists")
            color: Theme.text
            font.pixelSize: Theme.fontLarge
            font.weight: Font.Bold
            wrapMode: Text.WordWrap
        }
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Each one checked becomes a cuelist: each step a cue with what it changes, timed steps chained with Wait. Under each one, what doesn't play the same.")
            color: Theme.textDim
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            SourceChoice {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                selected: dialog.thisShow
                title: qsTr("In this show")
                detail: Core.showTitle
                onClicked: if (!dialog.thisShow) dialog.load("")
            }
            SourceChoice {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                selected: !dialog.thisShow
                title: qsTr("From another show…")
                detail: dialog.thisShow ? qsTr("Only the cuelists come in, on the fixtures of this show")
                                        : decodeURIComponent(dialog.source.toString().replace(/^.*\//, ""))
                onClicked: showDialog.start()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6
            Text {
                Layout.fillWidth: true
                text: qsTr("To convert")
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
            }
            AppButton {
                text: qsTr("All")
                flat: true
                implicitHeight: 26
                onClicked: dialog.chosen = dialog.lists.filter(l => l.enabled).map(l => l.key)
            }
            AppButton { text: qsTr("None"); flat: true; implicitHeight: 26; onClicked: dialog.chosen = [] }
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(Math.min(listView.contentHeight, 260), 60) + 2
            radius: Theme.radiusSmall
            color: Theme.background
            border.color: Theme.border
            ListView {
                id: listView
                anchors { fill: parent; margins: 1 }
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                model: dialog.lists
                ScrollBar.vertical: ScrollBar { policy: listView.contentHeight > listView.height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded }
                delegate: Rectangle {
                    id: row
                    required property var modelData
                    readonly property bool checked: dialog.chosen.indexOf(modelData.key) >= 0
                    // what doesn't play the same, and a cuelist of the same name
                    readonly property var notes: modelData.exists.length
                                                 ? modelData.warnings.concat([qsTr("already a cuelist: “%1”").arg(modelData.exists)])
                                                 : modelData.warnings
                    width: ListView.view.width
                    implicitHeight: rowLayout.implicitHeight + 12
                    color: rowMouse.containsMouse && modelData.enabled ? Theme.surfaceHover : "transparent"
                    opacity: modelData.enabled ? 1 : 0.55
                    RowLayout {
                        id: rowLayout
                        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; leftMargin: 8; rightMargin: 12 }
                        spacing: 8
                        // (the row takes the clicks: the box only shows)
                        CheckBox { checked: row.checked; enabled: row.modelData.enabled; focusPolicy: Qt.NoFocus }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 1
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8
                                Text {
                                    Layout.fillWidth: true
                                    text: row.modelData.name
                                    color: Theme.text
                                    font.weight: Font.DemiBold
                                    elide: Text.ElideRight
                                }
                                Text {
                                    text: dialog.kindLabel(row.modelData.kind) + " · " + qsTr("%n step(s)", "", row.modelData.steps)
                                    color: Theme.textFaint
                                    font.pixelSize: Theme.fontSmall
                                }
                            }
                            Text {
                                Layout.fillWidth: true
                                visible: text.length > 0
                                text: row.notes.join(" · ")
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSmall
                                wrapMode: Text.WordWrap
                                maximumLineCount: 3
                                elide: Text.ElideRight
                            }
                        }
                    }
                    MouseArea {
                        id: rowMouse
                        anchors.fill: parent
                        enabled: row.modelData.enabled
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: dialog.toggle(row.modelData.key)
                    }
                }
            }
            Text {
                anchors { fill: parent; margins: 12 }
                visible: dialog.lists.length === 0
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                text: dialog.error.length ? dialog.error : qsTr("No chasers, sequences or cue lists of QLC+ to convert")
                color: dialog.error.length ? Theme.danger : Theme.textFaint
            }
        }

        CheckBox {
            id: replaceCheck
            Layout.fillWidth: true
            visible: dialog.thisShow
            text: qsTr("Replace the originals (their groups, SHOW buttons and mappings play the cuelist; QLC+ won't see them)")
            contentItem: Text {
                leftPadding: replaceCheck.indicator.width + replaceCheck.spacing
                text: replaceCheck.text
                color: Theme.text
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
            }
        }
        CheckBox {
            id: playbacksCheck
            Layout.fillWidth: true
            checked: true
            text: qsTr("Put them on the free playbacks of this page")
        }
        CheckBox {
            id: controlsCheck
            Layout.fillWidth: true
            visible: dialog.hasCueLists
            checked: true
            text: qsTr("Bring their keys and controller buttons as mappings")
            ToolTip.text: qsTr("Next, Previous, Playback and Stop of each cue list become GO, Back, Pause/Back and Release of its cuelist")
            ToolTip.visible: hovered
            ToolTip.delay: 500
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            AppButton { text: qsTr("Cancel"); onClicked: dialog.close() }
            AppButton {
                text: qsTr("Make %n cuelist(s)", "", dialog.chosen.length)
                iconGlyph: Theme.iconList
                primary: true
                enabled: dialog.chosen.length > 0
                onClicked: dialog.make()
            }
        }

        AppFileDialog {
            id: showDialog
            kind: "show"
            title: qsTr("QLC+ cue lists of another show")
            nameFilters: [qsTr("Show files (*.qxw)"), qsTr("All files (*)")]
            onAccepted: dialog.load(selectedFile)
        }
    }
}
