/*
  DMX Total Control
  PlaybackStrip.qml

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
 * A playback, the way the playback bar shows it: the name (a click
 * selects it, the × releases it), the cue it plays and the next one, three
 * buttons (GO, Pause/Back and Flash unless the properties say otherwise)
 * and the fader, which is its intensity master. An empty playback takes a
 * new cuelist from the programmer or a scene ("+", or a scene dropped on
 * it). Shared by the Playbacks page and the SHOW.
 */
Rectangle {
    id: strip

    /** The playback page (-1: the current one, it follows the page changes) */
    property int page: -1
    property int slot: 0
    /** Scenes dropped on it replace what it holds (else only an empty one takes them) */
    property bool editing: false
    /** Record + a playback: a click records the programmer on it */
    property bool recordMode: false
    /** Low: one line of cue, smaller buttons */
    property bool compact: false
    /** Too low for all of it (a short section of the SHOW): button 2 goes,
     *  so the fader and the flash stay */
    readonly property bool tight: compact && height > 0 && height < 240
    /** Buttons 2 and 3, and the cue lines */
    property bool showBack: true
    property bool showFlash: true
    property bool showCue: true
    /** The item dragged from a list of scenes ({ functionId }) */
    property Item dragSource: null
    /** A locked SHOW: it plays, and that's all (no menu, nothing to add) */
    property bool locked: false

    /** Edit in Scenes */
    signal openScene(int functionId)
    /** A click in record mode (it recorded, or said why not) */
    signal recordClicked()

    // a renamed scene shows its new name
    property int functionsRevision: 0
    Connections {
        target: Core
        function onFunctionsChanged() { strip.functionsRevision++ }
    }

    readonly property var info: {
        Core.playbacks.revision
        strip.functionsRevision
        return Core.playbacks.slotInfo(strip.page, strip.slot)
    }
    readonly property bool empty: strip.info.empty !== false
    /** A fixed page that is gone (a SHOW row on a deleted page): nothing to
     *  put on it, never the same slot of the current page instead */
    readonly property bool pageGone: strip.page >= Core.playbacks.pageCount
    readonly property int playState: strip.info.state !== undefined ? strip.info.state : -1
    readonly property bool running: !!strip.info.running
    readonly property real progress: {
        Core.playbacks.progressRevision
        return strip.info.isCuelist && strip.running ? Core.playbacks.functionStatus(strip.info.functionId).progress
                                                     : (strip.running ? 1 : 0)
    }
    readonly property color tint: strip.info.color ? Theme.category(strip.info.color) : Theme.functionColor(strip.info.type || "")
    /** Lock of the programming screens: no recording or changing the playbacks */
    readonly property bool programLocked: Core.settings.lockedScreens.indexOf("program") >= 0
    readonly property bool hasProgrammer: Core.programmer.hasValues || Core.programmer.fxLayers.length > 0

    /* the actions of the three buttons: how they look, what they map to */
    function actionText(action) {
        switch (action) {
        case "go": return "GO"
        case "back": return strip.width >= 110 ? qsTr("Back") : ""
        case "pause": return strip.width >= 110 ? qsTr("Pause") : ""
        case "release": return qsTr("Release")
        case "flash": return qsTr("Flash")
        case "flashGo": return qsTr("Flash+GO")
        case "toggle": return qsTr("On/Off")
        case "select": return qsTr("Select")
        default: return ""
        }
    }
    function actionIcons(action) {
        switch (action) {
        case "back": return [Theme.iconStepBackward]
        case "pauseBack": return [Theme.iconPause, Theme.iconStepBackward]
        case "pause": return [Theme.iconPause]
        default: return []
        }
    }
    function actionTarget(action) {
        switch (action) {
        case "go": return "playbackGo"
        case "back": return "playbackBack"
        case "pauseBack": return "playbackPauseBack"
        case "flash": return "playbackFlash"
        case "release": return "playbackRelease"
        case "select": return "playbackSelect"
        default: return ""
        }
    }
    function actionTip(action) {
        switch (action) {
        case "go": return qsTr("GO: the next cue (starts it when it is off)")
        case "back": return qsTr("Back: the cue before")
        case "pauseBack": return qsTr("Pause / Back: pauses a fade, else the cue before")
        case "pause": return qsTr("Pause")
        case "release": return qsTr("Release")
        case "flash": return qsTr("Flash: at full while pressed")
        case "flashGo": return qsTr("Flash + GO: at full while pressed, and the next cue")
        case "toggle": return qsTr("On / off")
        case "select": return qsTr("Select it (Main GO, Record and Update work on it)")
        default: return ""
        }
    }
    /** Lit: the colour, or "" when it is dark */
    function actionLit(action) {
        switch (action) {
        case "go": return strip.playState >= 1 ? Theme.success : ""
        case "toggle": return strip.running ? Theme.success : ""
        case "pauseBack":
        case "pause": return strip.playState === 3 ? Theme.warning : ""
        case "flash":
        case "flashGo": return strip.info.flashing ? Theme.warning : ""
        case "select": return strip.info.selected ? Theme.warning : ""
        default: return ""
        }
    }
    /** 0 steady, 1 fast (a cue fading or waiting), 2 slow (paused) */
    function actionBlink(action) {
        if (action === "go")
            return strip.playState === 2 ? 1 : strip.playState === 3 ? 2 : 0
        if (action === "pauseBack" || action === "pause")
            return strip.playState === 3 ? 2 : 0
        return 0
    }

    function cueLine() {
        if (strip.empty)
            return ""
        if (!strip.info.isCuelist)
            return Theme.typeLabel(strip.info.type || "")
        if (strip.running && strip.info.cueNumber)
            return strip.info.cueNumber + (strip.info.cueName ? " · " + strip.info.cueName : "")
        return strip.info.cueCount > 0 ? qsTr("Released") : qsTr("No cues")
    }
    function nextLine() {
        if (strip.empty)
            return ""
        if (!strip.info.isCuelist)
            return strip.playState === 3 ? qsTr("Paused") : strip.running ? qsTr("On") : qsTr("Off")
        if (strip.info.nextNumber)
            return "▸ " + strip.info.nextNumber + (strip.info.nextName ? " · " + strip.info.nextName : "")
        return ""
    }

    /** The context menu (at the mouse; $below: under the name, for the keyboard and screenshots) */
    function openMenu(below) {
        menuLoader.active = true
        if (below)
            menuLoader.item.popup(strip, 6, header.y + header.height)
        else
            menuLoader.item.popup()
    }
    function openAddMenu() { addMenu.popup(addButton, 0, addButton.height) }
    function openAssign() {
        if (strip.pageGone)
            return
        assignLoader.active = true
        assignLoader.item.openFor(strip.page, strip.slot)
    }
    function openProperties() {
        propertiesLoader.active = true
        propertiesLoader.item.openFor(strip.page, strip.slot)
    }
    function updateCue(cueOnly) {
        Core.playbacks.select(strip.page, strip.slot)
        Core.playbacks.updateSelected(cueOnly)
    }

    implicitWidth: 120
    implicitHeight: 420
    radius: Theme.radius
    color: strip.empty ? Theme.background : Theme.surface
    border.color: strip.info.selected ? Theme.warning : strip.running ? Qt.alpha(strip.tint, 0.8) : Theme.border
    border.width: strip.info.selected ? 2 : 1

    // a button of the strip: icons and/or a text, lit and blinking by state
    component StripButton: AbstractButton {
        id: sb
        property string act: "none"
        property string label: ""
        property var icons: []
        property string litColor: ""
        property int blink: 0
        property bool big: false
        readonly property bool lit: litColor.length > 0
        property real phase: 1

        focusPolicy: Qt.NoFocus
        hoverEnabled: true
        implicitHeight: big ? 46 : 32
        opacity: enabled ? 1 : 0.5

        SequentialAnimation on phase {
            running: sb.lit && sb.blink > 0
            loops: Animation.Infinite
            NumberAnimation { to: 0.2; duration: sb.blink === 1 ? 260 : 650 }
            NumberAnimation { to: 1; duration: sb.blink === 1 ? 260 : 650 }
        }

        background: Rectangle {
            radius: Theme.radiusSmall
            color: sb.down ? Theme.surface : sb.hovered ? Theme.surfaceHover : Theme.surfaceRaised
            border.color: sb.lit ? sb.litColor : Theme.border
            Rectangle {
                anchors.fill: parent
                radius: parent.radius
                visible: sb.lit
                color: sb.lit ? Theme.fill(sb.litColor) : "transparent"
                opacity: sb.blink > 0 ? sb.phase : 1
            }
        }
        contentItem: Item {
            implicitWidth: row.implicitWidth
            implicitHeight: row.implicitHeight
            Row {
                id: row
                anchors.centerIn: parent
                spacing: 4
                Repeater {
                    model: sb.icons
                    IconLabel {
                        required property string modelData
                        anchors.verticalCenter: parent.verticalCenter
                        icon: modelData
                        size: sb.big ? 15 : 12
                        color: sb.lit ? Theme.textOn(Theme.fill(sb.litColor)) : Theme.text
                    }
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: sb.label.length > 0
                    text: sb.label
                    font.family: Theme.fontFamily
                    font.pixelSize: sb.big ? Theme.fontLarge : Theme.fontSmall + 1
                    font.weight: Font.Bold
                    color: sb.lit ? Theme.textOn(Theme.fill(sb.litColor)) : Theme.text
                }
            }
        }
        ToolTip.visible: hovered && ToolTip.text.length > 0
        ToolTip.delay: 700
    }

    // right click anywhere: what can be done with the playback
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        enabled: !strip.locked
        onClicked: strip.openMenu()
    }

    ColumnLayout {
        anchors { fill: parent; margins: 6 }
        spacing: strip.compact ? 4 : 6

        // number and name, on its colour: a click selects it, the × releases it
        Item {
            id: header
            Layout.fillWidth: true
            Layout.preferredHeight: tab.height + (strip.compact ? 18 : 36)

            MouseArea {
                anchors.fill: parent
                enabled: !strip.empty
                cursorShape: strip.empty ? Qt.ArrowCursor : Qt.PointingHandCursor
                onClicked: Core.playbacks.select(strip.page, strip.slot)
            }
            Rectangle {
                id: tab
                anchors { left: parent.left; right: parent.right; top: parent.top }
                height: 20
                radius: 4
                // lit while it plays
                color: strip.empty ? Theme.surfaceRaised : strip.running ? Theme.fill(strip.tint) : Qt.alpha(strip.tint, 0.3)
                Text {
                    anchors { left: parent.left; leftMargin: 6; verticalCenter: parent.verticalCenter }
                    text: strip.slot + 1
                    color: strip.running ? Theme.textOn(Theme.fill(strip.tint)) : strip.empty ? Theme.textFaint : Theme.text
                    font.family: Theme.monoFamily
                    font.pixelSize: Theme.fontSmall
                    font.weight: Font.Bold
                }
                // a scene with steps (a cuelist, an FX at the BPM of the show) that plays: its rhythm, a half a step
                RhythmBar {
                    functionId: !strip.empty && strip.info.functionId !== undefined ? strip.info.functionId : -1
                    litColor: Theme.textOn(Theme.fill(strip.tint))
                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; leftMargin: 26; rightMargin: 24 }
                    height: 4
                }
                AbstractButton {
                    id: releaseButton
                    anchors { right: parent.right; rightMargin: 1; verticalCenter: parent.verticalCenter }
                    width: 18
                    height: 18
                    visible: (!strip.empty && strip.running) || Core.mapping.learnMode
                    focusPolicy: Qt.NoFocus
                    hoverEnabled: true
                    background: Rectangle {
                        radius: 9
                        color: releaseButton.down ? Qt.darker(Theme.danger, 1.2) : releaseButton.hovered ? Theme.danger : Qt.rgba(0, 0, 0, 0.3)
                    }
                    contentItem: IconLabel { icon: Theme.iconClose; size: 9; color: releaseButton.hovered ? Theme.textOn(Theme.danger) : Theme.textOnShade }
                    onClicked: Core.playbacks.release(strip.page, strip.slot)
                    ToolTip.text: qsTr("Release")
                    ToolTip.visible: hovered
                    ToolTip.delay: 600
                    MapTarget { target: ({ target: "playbackRelease", pbPage: strip.page, slot: strip.slot }) }
                }
            }
            Text {
                id: nameText
                anchors { left: parent.left; right: parent.right; top: tab.bottom; topMargin: 3; bottom: parent.bottom }
                text: strip.pageGone ? qsTr("Page deleted") : strip.empty ? qsTr("Empty") : (strip.info.name || "")
                color: strip.empty ? Theme.textFaint : Theme.text
                // a word too long for the strip: a smaller font
                font.pixelSize: longestWord.advanceWidth + 4 > width ? Theme.fontSmall : Theme.fontNormal
                font.weight: strip.empty ? Font.Normal : Font.DemiBold
                font.italic: strip.empty
                // two lines, at word breaks when it can
                wrapMode: Text.Wrap
                maximumLineCount: strip.compact ? 1 : 2
                elide: Text.ElideRight
                lineHeight: 0.9
            }
            TextMetrics {
                id: longestWord
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontNormal
                font.weight: Font.DemiBold
                text: nameText.text.split(" ").reduce((a, b) => b.length > a.length ? b : a, "")
            }
            MapTarget {
                anchors.fill: undefined
                anchors { left: parent.left; top: tab.bottom; bottom: parent.bottom; right: parent.right }
                visible: !strip.empty
                target: ({ target: "playbackSelect", pbPage: strip.page, slot: strip.slot })
            }
        }

        // the cue it plays, the next one, how far its fade went
        ColumnLayout {
            Layout.fillWidth: true
            visible: strip.showCue && !strip.empty
            spacing: 2

            Text {
                Layout.fillWidth: true
                visible: !strip.empty
                text: strip.cueLine()
                color: strip.playState === 3 ? Theme.warning
                     : strip.running && strip.info.isCuelist ? Theme.success
                     : strip.running ? Theme.text : Theme.textDim
                font.pixelSize: Theme.fontSmall + 1
                font.weight: strip.running ? Font.Bold : Font.Normal
                elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: true
                visible: !strip.empty && !strip.compact
                text: strip.nextLine()
                color: Theme.textFaint
                font.pixelSize: Theme.fontSmall
                elide: Text.ElideRight
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 3
                visible: !strip.empty
                radius: 1.5
                color: Theme.background
                Rectangle {
                    width: parent.width * Math.max(0, Math.min(1, strip.progress))
                    height: parent.height
                    radius: 1.5
                    color: strip.playState === 3 ? Theme.warning : Theme.success
                    visible: strip.running
                }
            }
        }

        // an empty playback: a cuelist from the programmer, or a scene
        AppButton {
            id: addButton
            Layout.fillWidth: true
            Layout.preferredHeight: strip.compact ? 26 : 36
            visible: strip.empty
            enabled: !strip.programLocked && !strip.locked && !strip.pageGone
            text: strip.width >= 110 ? qsTr("Add") : ""
            iconGlyph: Theme.iconPlus
            flat: true
            onClicked: strip.openAddMenu()
            ToolTip.text: qsTr("A new cuelist from the programmer, or a scene")
            ToolTip.visible: hovered
            ToolTip.delay: 600

            AppMenu {
                id: addMenu
                MenuItem {
                    text: qsTr("New cuelist from the programmer")
                    enabled: strip.hasProgrammer
                    onTriggered: Core.playbacks.recordToSlot(strip.page, strip.slot)
                }
                MenuItem {
                    text: qsTr("Put a scene here…")
                    onTriggered: strip.openAssign()
                }
            }
        }

        // button 1 (GO) and button 2 (Pause / Back)
        StripButton {
            id: button1
            Layout.fillWidth: true
            big: !strip.compact
            act: strip.info.button1 || "go"
            // (learn mode: the controls of an empty playback can be mapped too)
            enabled: (!strip.empty && act !== "none") || Core.mapping.learnMode
            label: strip.actionText(act)
            icons: strip.actionIcons(act)
            litColor: strip.actionLit(act)
            blink: strip.actionBlink(act)
            ToolTip.text: strip.actionTip(act)
            onPressed: Core.playbacks.pressButton(strip.page, strip.slot, 1, true)
            onReleased: Core.playbacks.pressButton(strip.page, strip.slot, 1, false)
            onCanceled: Core.playbacks.pressButton(strip.page, strip.slot, 1, false)
            MapTarget {
                visible: strip.actionTarget(button1.act).length > 0
                target: ({ target: strip.actionTarget(button1.act), pbPage: strip.page, slot: strip.slot })
            }
        }
        StripButton {
            id: button2
            Layout.fillWidth: true
            visible: strip.showBack && !strip.tight
            act: strip.info.button2 || "pauseBack"
            // (learn mode: the controls of an empty playback can be mapped too)
            enabled: (!strip.empty && act !== "none") || Core.mapping.learnMode
            label: strip.actionText(act)
            icons: strip.actionIcons(act)
            litColor: strip.actionLit(act)
            blink: strip.actionBlink(act)
            ToolTip.text: strip.actionTip(act)
            onPressed: Core.playbacks.pressButton(strip.page, strip.slot, 2, true)
            onReleased: Core.playbacks.pressButton(strip.page, strip.slot, 2, false)
            onCanceled: Core.playbacks.pressButton(strip.page, strip.slot, 2, false)
            MapTarget {
                visible: strip.actionTarget(button2.act).length > 0
                target: ({ target: strip.actionTarget(button2.act), pbPage: strip.page, slot: strip.slot })
            }
        }

        // the fader: the intensity of what it plays
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 60

            VFader {
                id: fader
                anchors { top: parent.top; bottom: levelText.top; bottomMargin: 2; horizontalCenter: parent.horizontalCenter }
                implicitWidth: 44
                from: 0
                to: 1
                stepSize: 0
                // the keys belong to the page (Space = GO, PgUp/PgDown = pages)
                focusPolicy: Qt.NoFocus
                enabled: !strip.empty || Core.mapping.learnMode
                fillColor: strip.empty ? Theme.border : strip.tint
                value: strip.info.level !== undefined ? strip.info.level : 0
                resetValue: value > 0.5 ? 0 : 1
                onUserMoved: (v) => Core.playbacks.setLevel(strip.page, strip.slot, v)
                MapTarget {
                    target: ({ target: "playbackFader", pbPage: strip.page, slot: strip.slot })
                }
            }
            Text {
                id: levelText
                anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter }
                text: strip.empty ? "" : Math.round((strip.info.level || 0) * 100) + "%"
                color: (strip.info.level || 0) < 0.05 && !strip.empty ? Theme.textFaint : Theme.textDim
                font.family: Theme.monoFamily
                font.pixelSize: Theme.fontSmall
            }
        }

        // button 3 (Flash)
        StripButton {
            id: button3
            Layout.fillWidth: true
            visible: strip.showFlash
            act: strip.info.button3 || "flash"
            // (learn mode: the controls of an empty playback can be mapped too)
            enabled: (!strip.empty && act !== "none") || Core.mapping.learnMode
            label: strip.actionText(act)
            icons: strip.actionIcons(act)
            litColor: strip.actionLit(act)
            blink: strip.actionBlink(act)
            ToolTip.text: strip.actionTip(act)
            onPressed: Core.playbacks.pressButton(strip.page, strip.slot, 3, true)
            onReleased: Core.playbacks.pressButton(strip.page, strip.slot, 3, false)
            onCanceled: Core.playbacks.pressButton(strip.page, strip.slot, 3, false)
            MapTarget {
                visible: strip.actionTarget(button3.act).length > 0
                target: ({ target: strip.actionTarget(button3.act), pbPage: strip.page, slot: strip.slot })
            }
        }
    }

    // a scene dragged from a list: an empty playback takes it (any, while assigning)
    DropArea {
        id: drop
        anchors.fill: parent
        enabled: (strip.editing || strip.empty) && !strip.programLocked && !strip.locked && !strip.pageGone
        keys: ["dtc-show-scene"]
        onDropped: (ev) => {
            // (its page as it is now: -1 would be the current page)
            const p = Core.playbacks.resolvePage(strip.page)
            if (p >= 0 && strip.dragSource && strip.dragSource.functionId >= 0)
                Core.playbacks.assign(p, strip.slot, strip.dragSource.functionId)
            ev.accept()
        }
    }
    Rectangle {
        anchors.fill: parent
        z: 40
        visible: drop.containsDrag
        radius: strip.radius
        color: Qt.alpha(Theme.accent, 0.25)
        border.color: Theme.accent
        border.width: 2
    }

    // Record armed: a click records the programmer here
    Rectangle {
        anchors.fill: parent
        z: 60
        visible: strip.recordMode
        radius: strip.radius
        // a scene can't take cues: it stays grey
        readonly property bool takes: !strip.pageGone && (strip.empty || !!strip.info.isCuelist)
        color: !takes ? Qt.alpha(Theme.background, 0.4) : recordMouse.containsMouse ? Qt.alpha(Theme.danger, 0.3) : Qt.alpha(Theme.danger, 0.1)
        border.color: takes ? Theme.danger : Theme.borderStrong
        border.width: 2

        Rectangle {
            anchors.centerIn: parent
            width: Math.min(parent.width - 8, recordText.implicitWidth + 12)
            height: recordText.implicitHeight + 10
            radius: Theme.radiusSmall
            color: parent.takes ? Theme.danger : Theme.surfaceHover
            Text {
                id: recordText
                anchors.centerIn: parent
                width: Math.min(implicitWidth, parent.parent.width - 20)
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                text: strip.empty ? qsTr("New cuelist")
                    : strip.info.isCuelist ? qsTr("Record cue %1").arg(Core.cuelists.nextNumber(strip.info.functionId, -1))
                    : qsTr("Holds a scene")
                color: parent.parent.takes ? Theme.textOn(Theme.danger) : Theme.textDim
                font.pixelSize: Theme.fontSmall
                font.weight: Font.Bold
            }
        }
        MouseArea {
            id: recordMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (!strip.pageGone)
                    Core.playbacks.recordToSlot(strip.page, strip.slot)
                strip.recordClicked()
            }
        }
    }

    // created when they are needed: a page has up to 20 playbacks
    Loader {
        id: menuLoader
        active: false
        sourceComponent: Component {
            AppMenu {
                MenuItem {
                    text: qsTr("Assign a scene…")
                    enabled: !strip.programLocked && !strip.pageGone
                    onTriggered: strip.openAssign()
                }
                MenuItem {
                    text: qsTr("Edit in Scenes")
                    enabled: !strip.empty
                    onTriggered: strip.openScene(strip.info.functionId)
                }
                MenuSeparator {}
                MenuItem {
                    text: strip.empty ? qsTr("Record a new cuelist")
                        : qsTr("Record cue %1").arg(strip.info.isCuelist ? Core.cuelists.nextNumber(strip.info.functionId, -1) : "")
                    enabled: !strip.programLocked && !strip.pageGone && strip.hasProgrammer && (strip.empty || !!strip.info.isCuelist)
                    onTriggered: Core.playbacks.recordToSlot(strip.page, strip.slot)
                }
                MenuItem {
                    text: qsTr("Update the current cue (the next cues follow)")
                    enabled: !strip.programLocked && strip.hasProgrammer && !!strip.info.isCuelist
                    onTriggered: strip.updateCue(false)
                }
                MenuItem {
                    text: qsTr("Update only the current cue")
                    enabled: !strip.programLocked && strip.hasProgrammer && !!strip.info.isCuelist
                    onTriggered: strip.updateCue(true)
                }
                MenuSeparator {}
                MenuItem {
                    text: qsTr("Properties…")
                    enabled: !strip.empty && !strip.programLocked
                    onTriggered: strip.openProperties()
                }
                MenuItem {
                    text: qsTr("Release")
                    enabled: strip.running
                    onTriggered: Core.playbacks.release(strip.page, strip.slot)
                }
                MenuItem {
                    text: qsTr("Clear the playback")
                    enabled: !strip.empty && !strip.programLocked
                    onTriggered: Core.playbacks.clearSlot(strip.page, strip.slot)
                }
            }
        }
    }
    Loader {
        id: assignLoader
        active: false
        sourceComponent: Component { PlaybackAssignDialog {} }
    }
    Loader {
        id: propertiesLoader
        active: false
        sourceComponent: Component { PlaybackSlotDialog {} }
    }
}
