/*
  DMX Total Control
  SelectedCuelist.qml

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
 * The selected playback: its cues
 * with the live one (green bar) and the next one (outlined). "Next" makes a
 * cue the one the next GO plays, "Go" jumps there; a click on a row only
 * selects it, so nothing plays by mistake. A scene shows its state.
 */
Panel {
    id: panel

    /** Edit in Scenes */
    signal openScene(int functionId)

    readonly property var selectedPlayback: Core.playbacks.selected
    readonly property bool hasSelection: selectedPlayback.functionId !== undefined
    readonly property int pbPage: hasSelection ? selectedPlayback.page : -1
    readonly property int pbSlot: hasSelection ? selectedPlayback.slot : -1
    readonly property int fid: hasSelection ? selectedPlayback.functionId : -1
    readonly property var info: {
        Core.playbacks.revision
        panel.functionsRevision
        return hasSelection ? Core.playbacks.slotInfo(pbPage, pbSlot) : ({})
    }
    readonly property bool isCuelist: !!info.isCuelist
    readonly property real progress: {
        Core.playbacks.progressRevision
        return isCuelist && info.running ? Core.playbacks.functionStatus(fid).progress : 0
    }
    /** The row clicked (it doesn't play) */
    property int pickedUid: -1

    property int functionsRevision: 0
    Connections {
        target: Core
        function onFunctionsChanged() { panel.functionsRevision++ }
    }

    function format(ms) {
        const s = ms / 1000
        return s.toLocaleString(Qt.locale(), 'f', ms % 100 === 0 ? 1 : 2)
    }
    function triggerText(cue) {
        if (cue.trigger === "follow")
            return qsTr("Follow") + (cue.triggerTime > 0 ? " " + format(cue.triggerTime) : "")
        if (cue.trigger === "wait")
            return qsTr("Wait") + " " + format(cue.triggerTime)
        return "GO"
    }

    function refresh() {
        const cues = isCuelist && fid >= 0 ? Core.cuelists.cues(fid) : []
        // the rows stay: a change updates them in place
        for (let i = 0; i < cues.length; i++) {
            const c = cues[i]
            const row = { uid: c.uid, number: c.number, name: c.name, trigger: triggerText(c), delay: c.delay,
                          fade: c.fade, downFade: c.downFade, skip: c.skip }
            if (i < cueModel.count)
                cueModel.set(i, row)
            else
                cueModel.append(row)
        }
        if (cueModel.count > cues.length)
            cueModel.remove(cues.length, cueModel.count - cues.length)
    }
    onFidChanged: { pickedUid = -1; refresh() }
    onIsCuelistChanged: refresh()
    Component.onCompleted: refresh()
    Connections {
        target: Core.cuelists
        function onEdited(id) { if (id === panel.fid) panel.refresh() }
    }
    Connections {
        target: Core
        function onFunctionsChanged() { Qt.callLater(panel.refresh) }
    }

    // the live cue in sight
    readonly property int liveCue: info.running && info.cue !== undefined ? info.cue : -1
    onLiveCueChanged: if (liveCue >= 0) list.positionViewAtIndex(liveCue, ListView.Contain)

    ListModel { id: cueModel }

    component HeaderText: Text {
        color: Theme.textFaint
        font.weight: Font.Bold
        font.pixelSize: Theme.fontSmall
        elide: Text.ElideRight
    }

    title: hasSelection ? qsTr("Playback %1 · %2").arg(pbSlot + 1).arg(info.name || "") : qsTr("Selected playback")

    headerItems: [
        Text {
            anchors.verticalCenter: parent.verticalCenter
            visible: panel.hasSelection
            width: Math.min(implicitWidth, Math.max(0, panel.width - 660))
            elide: Text.ElideRight
            text: {
                if (!panel.hasSelection)
                    return ""
                const i = panel.info
                if (!i.running)
                    return !i.isCuelist ? qsTr("Off")
                         : i.nextNumber ? qsTr("Released · GO plays %1").arg(i.nextNumber) : qsTr("Released")
                const where = i.isCuelist ? "Q " + i.cueNumber + (i.cueName ? " · " + i.cueName : "") : qsTr("On")
                return (i.paused ? qsTr("Paused") + " · " : "") + where
            }
            color: panel.info.paused ? Theme.warning : panel.info.running ? Theme.success : Theme.textDim
            font.pixelSize: Theme.fontSmall
            font.weight: Font.DemiBold
        },
        AppButton {
            iconGlyph: Theme.iconStepBackward
            implicitHeight: 28
            visible: panel.isCuelist
            enabled: !!panel.info.running
            ToolTip.text: qsTr("Back: the cue before")
            ToolTip.visible: hovered
            ToolTip.delay: 500
            onClicked: Core.playbacks.back(panel.pbPage, panel.pbSlot)
        },
        AppButton {
            text: "GO"
            iconGlyph: Theme.iconPlay
            implicitHeight: 28
            visible: panel.hasSelection
            primary: true
            accentColor: Theme.success
            ToolTip.text: panel.isCuelist ? qsTr("The next cue") : qsTr("Start it")
            ToolTip.visible: hovered
            ToolTip.delay: 500
            onClicked: Core.playbacks.go(panel.pbPage, panel.pbSlot)
        },
        AppButton {
            iconGlyph: Theme.iconPause
            implicitHeight: 28
            visible: panel.hasSelection
            enabled: !!panel.info.running
            highlighted: !!panel.info.paused
            accentColor: Theme.warning
            ToolTip.text: panel.info.paused ? qsTr("Resume") : qsTr("Pause")
            ToolTip.visible: hovered
            ToolTip.delay: 500
            onClicked: Core.playbacks.pause(panel.pbPage, panel.pbSlot)
        },
        AppButton {
            iconGlyph: Theme.iconStop
            implicitHeight: 28
            visible: panel.hasSelection
            enabled: !!panel.info.running
            ToolTip.text: qsTr("Release")
            ToolTip.visible: hovered
            ToolTip.delay: 500
            onClicked: Core.playbacks.release(panel.pbPage, panel.pbSlot)
        },
        AppButton {
            text: qsTr("Edit in Scenes")
            iconGlyph: Theme.iconPen
            implicitHeight: 28
            flat: true
            visible: panel.hasSelection
            onClicked: panel.openScene(panel.fid)
        }
    ]

    // nothing selected
    Text {
        anchors.centerIn: parent
        width: parent.width - 40
        visible: !panel.hasSelection
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        text: qsTr("Click the name of a playback to select it: its cues show here, and Main GO plays it.")
        color: Theme.textDim
    }

    // a scene: its type and state
    Column {
        anchors.centerIn: parent
        visible: panel.hasSelection && !panel.isCuelist
        spacing: 6
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: Theme.typeLabel(panel.info.type || "")
            color: Theme.textDim
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: panel.info.paused ? qsTr("Paused") : panel.info.running ? qsTr("On") : qsTr("Off")
            color: panel.info.paused ? Theme.warning : panel.info.running ? Theme.success : Theme.textFaint
            font.pixelSize: Theme.fontTitle
            font.weight: Font.Bold
        }
    }

    // a cuelist: its cues
    ColumnLayout {
        anchors { fill: parent; margins: 8; topMargin: 6 }
        visible: panel.isCuelist
        spacing: 4

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            spacing: 8
            HeaderText { Layout.preferredWidth: 56; text: qsTr("CUE") }
            HeaderText { Layout.fillWidth: true; text: qsTr("NAME") }
            HeaderText { Layout.preferredWidth: 96; text: qsTr("TRIGGER") }
            HeaderText { Layout.preferredWidth: 66; text: qsTr("DELAY"); horizontalAlignment: Text.AlignRight }
            HeaderText { Layout.preferredWidth: 72; text: qsTr("FADE"); horizontalAlignment: Text.AlignRight }
            Item { Layout.preferredWidth: 118 }
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 2
            model: cueModel
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}

            delegate: Rectangle {
                id: row
                required property int index
                required property var model
                readonly property bool isLive: !!panel.info.running && panel.info.cue === index
                readonly property bool isNext: panel.info.next === index && !isLive
                readonly property bool isPicked: panel.pickedUid === model.uid
                width: list.width
                height: 30
                radius: Theme.radiusSmall
                color: isLive ? Qt.alpha(Theme.success, 0.2) : isPicked ? Qt.alpha(Theme.accent, 0.18) : Theme.surfaceRaised
                border.color: isPicked ? Theme.accent : isNext ? Theme.success : Theme.border
                border.width: isNext ? 2 : 1

                Rectangle {
                    visible: row.isLive
                    anchors { left: parent.left; top: parent.top; bottom: parent.bottom; margins: 1 }
                    width: 4
                    radius: 2
                    color: Theme.success
                }
                // how far its fade went
                Rectangle {
                    visible: row.isLive && panel.progress < 1
                    anchors { left: parent.left; bottom: parent.bottom; leftMargin: 6; bottomMargin: 2 }
                    width: (parent.width - 12) * Math.max(0, panel.progress)
                    height: 2
                    color: panel.info.paused ? Theme.warning : Theme.success
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: panel.pickedUid = row.model.uid
                }

                RowLayout {
                    anchors { fill: parent; leftMargin: 10; rightMargin: 6 }
                    spacing: 8
                    Text {
                        Layout.preferredWidth: 56
                        text: row.model.number
                        color: row.model.skip ? Theme.warning : Theme.text
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.fillWidth: true
                        text: row.model.name
                        color: row.isLive ? Theme.text : Theme.textDim
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.preferredWidth: 96
                        text: row.model.trigger
                        color: row.model.trigger === "GO" ? Theme.textFaint : Theme.warning
                        font.pixelSize: Theme.fontSmall
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.preferredWidth: 66
                        horizontalAlignment: Text.AlignRight
                        text: panel.format(row.model.delay)
                        color: row.model.delay > 0 ? Theme.text : Theme.textFaint
                        font.family: Theme.monoFamily
                        font.pixelSize: Theme.fontSmall
                    }
                    Text {
                        Layout.preferredWidth: 72
                        horizontalAlignment: Text.AlignRight
                        // fade, and the down fade when it differs
                        text: panel.format(row.model.fade) + (row.model.downFade >= 0 && row.model.downFade !== row.model.fade
                                                              ? "/" + panel.format(row.model.downFade) : "")
                        color: Theme.text
                        font.family: Theme.monoFamily
                        font.pixelSize: Theme.fontSmall
                        elide: Text.ElideLeft
                    }
                    AppButton {
                        Layout.preferredWidth: 56
                        implicitHeight: 24
                        text: qsTr("Next")
                        flat: true
                        font.pixelSize: Theme.fontSmall
                        enabled: !row.isNext
                        ToolTip.text: qsTr("The next GO plays this cue")
                        ToolTip.visible: hovered
                        ToolTip.delay: 600
                        onClicked: Core.playbacks.setNextCue(panel.pbPage, panel.pbSlot, row.index)
                    }
                    AppButton {
                        Layout.preferredWidth: 54
                        implicitHeight: 24
                        text: qsTr("Go")
                        flat: true
                        ToolTip.text: qsTr("Go to this cue now")
                        ToolTip.visible: hovered
                        ToolTip.delay: 600
                        onClicked: Core.playbacks.goTo(panel.pbPage, panel.pbSlot, row.index)
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                width: parent.width - 40
                visible: list.count === 0
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: qsTr("No cues yet: set a look in the programmer, arm Record and click this playback.")
                color: Theme.textDim
            }
        }
    }
}
