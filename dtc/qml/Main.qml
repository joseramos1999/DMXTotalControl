/*
  DMX Total Control
  Main.qml

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

ApplicationWindow {
    id: window

    width: 1440
    height: 900
    minimumWidth: 1100
    minimumHeight: 680
    visible: true
    color: Theme.background
    title: (Core.modified ? "● " : "") + Core.showTitle + " — " + Core.appName

    font.family: Theme.fontFamily
    font.pixelSize: Theme.fontNormal
    palette.window: Theme.background
    palette.windowText: Theme.text
    palette.base: Theme.surfaceRaised
    palette.text: Theme.text
    palette.button: Theme.surfaceRaised
    palette.buttonText: Theme.text
    palette.highlight: Theme.accentFill
    palette.highlightedText: Theme.accentText
    palette.placeholderText: Theme.textFaint
    palette.mid: Theme.border
    palette.dark: Theme.borderStrong
    palette.light: Theme.surfaceHover
    palette.toolTipBase: Theme.surfaceRaised
    palette.toolTipText: Theme.text
    palette.shadow: Theme.dialogShade

    /* Pages, in navigation order */
    readonly property int pageLive: 0
    readonly property int pageShow: 1
    readonly property int pageProgram: 2
    readonly property int pageScenes: 3
    readonly property int pagePatch: 4
    readonly property int pageFixtures: 5
    readonly property int pageOutputs: 6
    readonly property int pageMappings: 7
    readonly property int pageOptions: 8
    // (appended: the --page numbers and F1-F9 stay; the menu shows it after Show)
    readonly property int pagePlaybacks: 9

    property int currentPage: pageLive
    /* The top bar on smaller screens: icons for Save as and the running
       count, a shorter grand master; narrow: Blackout and Stop all too */
    readonly property bool compactBar: width < 1600
    readonly property bool narrowBar: width < 1300
    /** Developer option --scene: open the Scenes page on this scene */
    property int startScene: -1
    onStartSceneChanged: if (startScene >= 0) { scenesPage.select(startScene); currentPage = pageScenes }
    /** Developer option --cue-select: the cues selected in the cuelist open in Scenes */
    property var startCueSelection: []
    onStartCueSelectionChanged: scenesPage.selectCues(startCueSelection)
    /** Developer option --edit-show: the Show page in edit mode, with a control selected */
    /** Developer option --play: the Outputs page shows the DMX levels */
    property bool showLevels: false
    /** Developer option --mixer: the Live page shows the mixer */
    property bool showMixer: false
    /** The Program page on the FX (developer option --fx, and Scenes › New › FX) */
    property bool showFx: false
    property bool showPalettes: false
    property bool showKeypad: false
    /** Developer option --record: the record dialog of the Program page */
    property bool showRecord: false
    /** Developer options --pb-assign and --pb-record: the Playbacks page with the scenes to assign, Record armed */
    property bool showPlaybackAssign: false
    onShowPlaybackAssignChanged: playbacksPage.assignOpen = showPlaybackAssign
    property bool armPlaybackRecord: false
    onArmPlaybackRecordChanged: playbacksPage.recordArmed = armPlaybackRecord
    /** Developer option --pb-dialog: a menu or dialog of the Playbacks page (kind:slot) */
    /** Developer options --guide and --help-page: the guide on a step, the manual on a page */
    property int startGuide: -1
    onStartGuideChanged: if (startGuide >= 0) { firstSteps.start(); firstSteps.go(startGuide) }
    property string startHelp: ""
    onStartHelpChanged: if (startHelp.length > 0) helpPanel.show(startHelp)
    property string startPlaybackDialog: ""
    onStartPlaybackDialogChanged: { currentPage = pagePlaybacks; playbacksPage.devOpen(startPlaybackDialog) }
    /** Developer option --mapping: the Mappings page with a mapping selected */
    property int startMapping: -1
    onStartMappingChanged: { mappingsPage.selectedId = startMapping; currentPage = pageMappings }
    /** Developer option --patch-view: a view or a dialog of the Patch page */
    property string startPatchView: ""
    onStartPatchViewChanged: { patchPage.devOpen(startPatchView); currentPage = pagePatch }
    property string startEditShow: ""
    onStartEditShowChanged: {
        let parts = startEditShow.split(":")
        showPage.currentPage = parts.length > 1 ? parseInt(parts[0]) : 0
        showPage.editing = true
        // page:section, or page:section:cell
        if (parts.length > 2)
            showPage.selectCell(parts[1], parseInt(parts[2]))
        else
            showPage.selectSection(parts[parts.length - 1])
        currentPage = pageShow
    }
    // screen lock: the screens that need the password to be opened
    function screenOf(page) {
        if (page === pageProgram || page === pageScenes)
            return "program"
        // the playbacks always play; what changes them checks the "program" lock
        if (page === pagePlaybacks)
            return ""
        if (page >= pagePatch)
            return "setup"
        return ""
    }
    // the setup locked: no other show can be opened
    readonly property bool setupLocked: Core.settings.lockedScreens.indexOf("setup") >= 0
    function pageLocked(page) {
        let s = screenOf(page)
        return s.length > 0 && Core.settings.lockedScreens.indexOf(s) >= 0
    }
    /** Go to a page; a locked one asks the password and unlocks everything */
    function goTo(page) {
        if (pageLocked(page)) {
            lockDialog.openUnlock(() => { Core.settings.lockedScreens = []; window.currentPage = page })
            return
        }
        currentPage = page
    }
    Connections {
        target: Core.settings
        function onLockChanged() { if (window.pageLocked(window.currentPage)) window.currentPage = window.pageLive }
    }

    /** Show page lock: no navigation, no editing */
    property bool locked: false
    onLockedChanged: if (locked) Core.mapping.learnMode = false
    property bool closeConfirmed: false

    function showMessage(text, isError) {
        toast.text = text
        toast.isError = !!isError
        toast.open()
    }

    function saveOrAsk(then) {
        if (Core.fileName.length === 0) {
            saveDialog.then = then || null
            saveDialog.start()
        } else if (Core.saveShow()) {
            showMessage(qsTr("Show saved"))
            if (then) then()
        } else {
            showMessage(Core.lastError, true)
        }
    }

    function openShow(url) {
        if (Core.openShow(url))
            showMessage(qsTr("Opened %1").arg(Core.showTitle))
        else
            showMessage(Core.lastError, true)
    }

    /** Do $action (New, Open...: it replaces the show) once the unsaved
     *  changes are saved or left, as the user says */
    function whenSaved(action) {
        if (Core.modified) {
            closeDialog.then = action
            closeDialog.open()
        } else {
            action()
        }
    }

    /** A second launch of the program (a show opened from the Explorer)
     *  came here: the show opens as File > Open opens it. Not while the
     *  SHOW or the setup is locked: it only says so */
    function openFromOutside(url) {
        if (window.locked || window.setupLocked)
            showMessage(qsTr("The screens are locked: another show can't be opened now"), true)
        else
            whenSaved(() => window.openShow(url))
    }

    /** The example shows installed with the program */
    readonly property var exampleShows: Core.examples()

    // the page of the manual about the page on screen
    function helpPage() {
        switch (window.locked ? window.pageShow : window.currentPage) {
        case window.pageLive: return "directo"
        case window.pageShow: return "show"
        case window.pageProgram: return "programar"
        case window.pageScenes: return "escenas"
        case window.pagePatch: return "patch"
        case window.pageFixtures: return "fixtures"
        case window.pageOutputs: return "salidas"
        case window.pageMappings: return "mapeos"
        case window.pageOptions: return "opciones"
        case window.pagePlaybacks: return "playbacks"
        default: return "index"
        }
    }
    function showHelp() {
        if (helpPanel.opened)
            helpPanel.close()
        else
            helpPanel.show(helpPage())
    }

    // the downloaded update: the show saved (or not) first, then the installer
    // runs and opens the program again
    function installUpdate() {
        whenSaved(() => {
            if (Core.updates.installNow()) {
                window.closeConfirmed = true
                window.close()
            } else {
                showMessage(Core.updates.error, true)
            }
        })
    }

    function setFullScreen(enable) {
        window.visibility = enable ? Window.FullScreen : Window.Windowed
    }

    // the show given to open (or the last one) couldn't be opened, or it
    // opened without some of its files: said once, false when all went well
    function tellOpenProblems() {
        if (Core.lastError.length > 0)
            showMessage(Core.lastError, true)
        else if (Core.missingFiles.length > 0)
            missingFiles.notify()
        else
            return false
        return true
    }

    onClosing: (close) => {
        if (Core.modified && !closeConfirmed) {
            close.accepted = false
            closeDialog.then = null
            closeDialog.open()
        }
    }

    Component.onCompleted: {
        // the SHOW full screen at startup, locked as a kiosk if asked
        if (Core.settings.startShowFullScreen) {
            currentPage = pageShow
            setFullScreen(true)
            if (Core.settings.startShowLocked)
                locked = true
        }
        // (the problems of the show opened wait for the answer: the copy may
        // replace it)
        if (Core.hasRecovery)
            recoveryDialog.open()
        // the very first start: the first steps guide, once
        else if (!tellOpenProblems() && !Core.settings.guideShown && !Core.settings.startShowFullScreen
                 && Core.settings.recentShows.length === 0)
            firstSteps.start()
        Core.settings.guideShown = true
    }

    /* ****************************************************************
     * Keyboard shortcuts
     * ****************************************************************/
    Shortcut { sequences: [StandardKey.Save]; enabled: !window.locked; onActivated: window.saveOrAsk() }
    Shortcut { sequences: [StandardKey.SaveAs]; enabled: !window.locked && !window.setupLocked; onActivated: saveDialog.start() }
    Shortcut { sequences: [StandardKey.Open]; enabled: !window.locked && !window.setupLocked; onActivated: window.whenSaved(() => openDialog.start()) }
    Shortcut { sequences: [StandardKey.New]; enabled: !window.locked && !window.setupLocked; onActivated: window.whenSaved(() => Core.newShow()) }
    Shortcut { sequence: "F1"; enabled: !window.locked; onActivated: window.currentPage = window.pageLive }
    Shortcut { sequence: "F2"; enabled: !window.locked; onActivated: window.currentPage = window.pageShow }
    Shortcut { sequence: "F3"; enabled: !window.locked; onActivated: window.goTo(window.pageProgram) }
    Shortcut { sequence: "F4"; enabled: !window.locked; onActivated: window.goTo(window.pageScenes) }
    Shortcut { sequence: "F5"; enabled: !window.locked; onActivated: window.goTo(window.pagePatch) }
    Shortcut { sequence: "F6"; enabled: !window.locked; onActivated: window.goTo(window.pageFixtures) }
    Shortcut { sequence: "F7"; enabled: !window.locked; onActivated: window.goTo(window.pageOutputs) }
    Shortcut { sequence: "F8"; enabled: !window.locked; onActivated: window.goTo(window.pageMappings) }
    Shortcut { sequence: "F9"; enabled: !window.locked; onActivated: window.goTo(window.pageOptions) }
    Shortcut { sequence: "F10"; enabled: !window.locked; onActivated: window.goTo(window.pagePlaybacks) }
    // the Playbacks page: Space is Main GO, Shift+Space Main Back, PgUp/PgDown the pages
    // (not while typing in a text field). GO and Back fire once per press, as
    // a console key: a held Space doesn't run through the cues
    readonly property bool textFocus: activeFocusItem !== null && activeFocusItem.cursorPosition !== undefined
    // A hidden page keeps the focus of its fields: after Program (its command
    // line takes the focus), what was typed on the next page went into the
    // hidden command line and the keyboard mappings stopped. The window takes
    // it back, unless the page shown took it (Show, Playbacks, Program)
    function dropHiddenFocus() {
        if (activeFocusItem !== null && !activeFocusItem.visible)
            contentItem.forceActiveFocus()
    }
    readonly property bool playbackKeys: !window.locked && window.currentPage === window.pagePlaybacks && !window.textFocus
    Shortcut { sequence: "Space"; enabled: window.playbackKeys; autoRepeat: false; onActivated: Core.playbacks.mainGo() }
    Shortcut { sequence: "Shift+Space"; enabled: window.playbackKeys; autoRepeat: false; onActivated: Core.playbacks.mainBack() }
    Shortcut { sequence: "PgUp"; enabled: window.playbackKeys; onActivated: Core.playbacks.previousPage() }
    Shortcut { sequence: "PgDown"; enabled: window.playbackKeys; onActivated: Core.playbacks.nextPage() }
    Shortcut { sequence: "F11"; onActivated: window.setFullScreen(window.visibility !== Window.FullScreen) }
    // the manual of the page on screen (F1-F10 open the pages)
    Shortcut { sequence: "F12"; onActivated: window.showHelp() }
    Shortcut { sequence: "Ctrl+B"; onActivated: Core.blackout = !Core.blackout }
    // on the Show page the letters belong to the buttons of the technician
    Shortcut { sequence: "G"; enabled: window.currentPage !== window.pageShow; onActivated: Core.drive.beatGo() }
    Shortcut { sequence: "Ctrl+Right"; onActivated: Core.zones.nextAll() }
    Shortcut { sequence: "Ctrl+Left"; onActivated: Core.zones.previousAll() }
    // while learning a mapping, Esc only cancels it; Record armed, it disarms it
    Shortcut {
        sequence: "Esc"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (Core.mapping.pendingTarget.target !== undefined) Core.mapping.cancelLearn()
            else if (Core.mapping.learnMode) Core.mapping.learnMode = false
            else if (playbacksPage.recordArmed) playbacksPage.recordArmed = false
            else Core.stopAll()
        }
    }
    Connections {
        target: Core.mapping
        function onLearnt(description) { window.showMessage(qsTr("Mapped: %1").arg(description)) }
    }
    // nothing protects the show when the autosave fails (a full disk, a
    // read-only file): the user must know
    Connections {
        target: Core
        function onAutosaveFailed(error) { window.showMessage(qsTr("Autosave failed: %1").arg(error), true) }
    }
    // what the playbacks and the cuelist editor have to say, from any page
    Connections {
        target: Core.playbacks
        function onMessage(text, error) { window.showMessage(text, error) }
    }
    Connections {
        target: Core.cuelists
        function onMessage(text) { window.showMessage(text, true) }
    }
    // (and the super scene editor: an audio it can't add, static values)
    Connections {
        target: Core.timeline
        function onMessage(text) { window.showMessage(text, true) }
    }
    // Alt+click on a control puts it on the SHOW
    Connections {
        target: Core.surface
        function onControlAdded(where) { window.showMessage(qsTr("Added to the SHOW, page “%1”").arg(where.pageName)) }
    }
    // the check of the start found new fixtures in the Open Fixture Library
    Connections {
        target: Core.library
        function onOflUpdateFound() { window.showMessage(qsTr("New fixtures in the Open Fixture Library: update it in Options")) }
    }

    AppFileDialog {
        id: openDialog
        title: qsTr("Open show")
        nameFilters: [qsTr("Show files (*.qxw)"), qsTr("All files (*)")]
        onAccepted: window.openShow(selectedFile)
    }

    AppFileDialog {
        id: saveDialog
        property var then: null
        title: qsTr("Save show as")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "qxw"
        nameFilters: [qsTr("Show files (*.qxw)")]
        onAccepted: {
            if (Core.saveShow(selectedFile)) {
                window.showMessage(qsTr("Show saved"))
                if (then) then()
            } else {
                window.showMessage(Core.lastError, true)
            }
            then = null
        }
    }

    /* ****************************************************************
     * Top bar
     * ****************************************************************/
    header: Rectangle {
        height: 56
        color: Theme.surface

        Rectangle {
            height: 1
            color: Theme.border
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        }

        RowLayout {
            anchors { fill: parent; leftMargin: 14; rightMargin: 14 }
            spacing: 10

            // brand: the icon of the application
            Image {
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                source: "qrc:/icons/dtc-64.png"
                sourceSize: Qt.size(64, 64)
                smooth: true
                mipmap: true
                // (dimmed at night, as the colours of the lights)
                opacity: Theme.lightLevel
            }
            // the show title gives way first when the bar is short
            ColumnLayout {
                Layout.rightMargin: 12
                Layout.fillWidth: true
                Layout.minimumWidth: 90
                Layout.maximumWidth: implicitWidth
                spacing: 0
                Text {
                    Layout.fillWidth: true
                    text: Core.appName
                    font.pixelSize: Theme.fontNormal
                    font.weight: Font.Bold
                    color: Theme.text
                    elide: Text.ElideRight
                }
                Text {
                    Layout.fillWidth: true
                    text: (Core.modified ? "● " : "") + Core.showTitle
                          + (Core.lastAutosave.length ? "  ·  " + qsTr("recovery copy %1").arg(Core.lastAutosave) : "")
                    font.pixelSize: Theme.fontSmall
                    color: Core.modified ? Theme.warning : Theme.textDim
                    elide: Text.ElideRight
                }
            }

            Row {
                visible: !window.locked
                spacing: 2
                enabled: !window.setupLocked
                AppButton { iconGlyph: Theme.iconFile; flat: true; ToolTip.text: qsTr("New show (Ctrl+N)"); ToolTip.visible: hovered; onClicked: window.whenSaved(() => Core.newShow()) }
                AppButton { iconGlyph: Theme.iconOpen; flat: true; ToolTip.text: qsTr("Open (Ctrl+O)"); ToolTip.visible: hovered; onClicked: window.whenSaved(() => openDialog.start()) }
                AppButton {
                    text: "▾"
                    flat: true
                    enabled: Core.settings.recentShows.length > 0 || window.exampleShows.length > 0
                    ToolTip.text: qsTr("Recent shows and examples")
                    ToolTip.visible: hovered
                    onClicked: recentMenu.popup()

                    AppMenu {
                        id: recentMenu
                        Instantiator {
                            model: Core.settings.recentShows
                            delegate: MenuItem {
                                required property string modelData
                                text: modelData.substring(modelData.lastIndexOf("/") + 1)
                                // (a path with '#' or '%' stays a path)
                                onTriggered: {
                                    const url = Core.localFileUrl(modelData)
                                    window.whenSaved(() => window.openShow(url))
                                }
                            }
                            onObjectAdded: (index, object) => recentMenu.insertItem(index, object)
                            onObjectRemoved: (index, object) => recentMenu.removeItem(object)
                        }
                        MenuSeparator { visible: Core.settings.recentShows.length > 0 && window.exampleShows.length > 0 }
                        // the example shows installed with the program
                        AppMenu {
                            id: examplesMenu
                            title: qsTr("Examples")
                            enabled: window.exampleShows.length > 0
                            Instantiator {
                                model: window.exampleShows
                                delegate: MenuItem {
                                    required property var modelData
                                    text: modelData.name
                                    onTriggered: window.whenSaved(() => window.openShow(modelData.url))
                                }
                                onObjectAdded: (index, object) => examplesMenu.insertItem(index, object)
                                onObjectRemoved: (index, object) => examplesMenu.removeItem(object)
                            }
                        }
                    }
                }
                AppButton { iconGlyph: Theme.iconSave; flat: true; ToolTip.text: qsTr("Save (Ctrl+S)"); ToolTip.visible: hovered; onClicked: window.saveOrAsk() }
                AppButton {
                    text: window.compactBar ? "" : qsTr("Save as…")
                    iconGlyph: window.compactBar ? Theme.iconSaveAs : ""
                    flat: true
                    ToolTip.text: qsTr("Save as… (Ctrl+Shift+S)")
                    ToolTip.visible: hovered
                    ToolTip.delay: window.compactBar ? 0 : 500
                    onClicked: saveDialog.start()
                }
            }

            Item { Layout.fillWidth: true }

            // how the buttons of the scenes act, always one of the two (GO
            // when the program starts): GO starts them and marks their
            // rhythm (each button shows the rhythm of its scene with steps),
            // Stop stops them
            Rectangle {
                Layout.preferredHeight: 46
                Layout.preferredWidth: pressRow.implicitWidth + 10
                radius: Theme.radiusSmall
                color: Theme.surfaceRaised
                border.color: Theme.border

                Row {
                    id: pressRow
                    anchors.centerIn: parent
                    spacing: 6

                    AppButton {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "GO"
                        iconGlyph: Theme.iconPlay
                        iconSize: 15
                        textSize: Theme.fontLarge
                        implicitHeight: 38
                        implicitWidth: Math.max(78, implicitContentWidth + leftPadding + rightPadding)
                        highlighted: Core.pressMode === "go"
                        accentColor: Theme.success
                        onClicked: Core.pressMode = "go"
                        MapTarget { target: ({ target: "pressGo" }) }
                        ToolTip.text: qsTr("GO: a press of a scene starts it, and the next ones mark its rhythm (it never stops it)")
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                    }
                    AppButton {
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Stop")
                        iconGlyph: Theme.iconStop
                        iconSize: 15
                        textSize: Theme.fontLarge
                        implicitHeight: 38
                        implicitWidth: Math.max(78, implicitContentWidth + leftPadding + rightPadding)
                        highlighted: Core.pressMode === "stop"
                        accentColor: Theme.danger
                        onClicked: Core.pressMode = "stop"
                        MapTarget { target: ({ target: "pressStop" }) }
                        ToolTip.text: qsTr("Stop: a press of a scene stops it")
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                    }
                }
            }

            // BeatGO and the audio pulses
            Rectangle {
                Layout.preferredHeight: 36
                Layout.preferredWidth: beatRow.implicitWidth + 16
                radius: Theme.radiusSmall
                color: Theme.surfaceRaised
                border.color: Theme.border

                Row {
                    id: beatRow
                    anchors.centerIn: parent
                    spacing: 8

                    // BeatGO: scenes driven by BeatGO go to their next division
                    AppButton {
                        id: beatGoButton
                        anchors.verticalCenter: parent.verticalCenter
                        text: "BeatGO"
                        implicitHeight: 28
                        onPressed: Core.drive.beatGo()
                        MapTarget { target: ({ target: "beatgo" }) }
                        ToolTip.text: qsTr("Next division of the scenes driven by BeatGO (G)")
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        Connections {
                            target: Core.drive
                            function onBeatGoTriggered() { beatGoFlash.restart() }
                        }
                        Rectangle {
                            id: beatGoGlow
                            anchors.fill: parent
                            radius: Theme.radiusSmall
                            color: Theme.warning
                            opacity: 0
                            NumberAnimation on opacity { id: beatGoFlash; running: false; from: 0.6; to: 0; duration: 200 }
                        }
                    }
                    // pulses from the audio input: a level meter with its threshold
                    AppButton {
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Pulse")
                        implicitHeight: 28
                        highlighted: Core.drive.pulseEnabled
                        accentColor: Theme.warning
                        onClicked: Core.drive.pulseEnabled = !Core.drive.pulseEnabled
                        ToolTip.text: qsTr("Listen to the audio input: each peak over the line moves the scenes driven by Pulse")
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                    }
                    Rectangle {
                        id: pulseMeter
                        visible: Core.drive.pulseEnabled
                        anchors.verticalCenter: parent.verticalCenter
                        width: 90
                        height: 18
                        radius: 3
                        color: Theme.background
                        border.color: Theme.border

                        Rectangle {
                            x: 1; y: 1
                            height: parent.height - 2
                            width: (parent.width - 2) * Core.drive.level
                            radius: 2
                            color: Core.drive.level >= Core.drive.threshold ? Theme.warning : Theme.success
                        }
                        // the threshold: drag it
                        Rectangle {
                            x: (parent.width - 2) * Core.drive.threshold
                            width: 2
                            height: parent.height
                            color: Theme.text
                        }
                        MouseArea {
                            anchors.fill: parent
                            onPressed: (ev) => Core.drive.threshold = Math.max(0.01, Math.min(1, ev.x / width))
                            onPositionChanged: (ev) => { if (pressed) Core.drive.threshold = Math.max(0.01, Math.min(1, ev.x / width)) }
                        }
                        Connections {
                            target: Core.drive
                            function onPulseTriggered() { pulseFlash.restart() }
                        }
                        Rectangle {
                            anchors.fill: parent
                            radius: 3
                            color: "transparent"
                            border.color: Theme.warning
                            border.width: 2
                            opacity: 0
                            NumberAnimation on opacity { id: pulseFlash; running: false; from: 1; to: 0; duration: 200 }
                        }
                    }
                }
            }

            // screen lock: screens that need a password
            AppButton {
                visible: !window.locked
                iconGlyph: Core.settings.lockedScreens.length > 0 ? Theme.iconLock : Theme.iconUnlock
                highlighted: Core.settings.lockedScreens.length > 0
                accentColor: Theme.warning
                onClicked: Core.settings.lockedScreens.length > 0
                           ? lockDialog.openUnlock(() => Core.settings.lockedScreens = [])
                           : lockDialog.openLock()
                ToolTip.text: Core.settings.lockedScreens.length > 0 ? qsTr("Locked screens: click to unlock")
                                                                     : qsTr("Lock screens so nothing changes by mistake")
                ToolTip.visible: hovered
                ToolTip.delay: 500
            }

            // external control: learn mode, and a LED that blinks on each message in
            AppButton {
                id: learnButton
                visible: !window.locked
                iconGlyph: Theme.iconTarget
                highlighted: Core.mapping.learnMode
                accentColor: Theme.learn
                onClicked: Core.mapping.learnMode = !Core.mapping.learnMode
                ToolTip.text: qsTr("Learn mode: click a control of the app, then move the control of your MIDI/OSC device or press a key")
                ToolTip.visible: hovered
                ToolTip.delay: 500

                Rectangle {
                    id: inputLed
                    anchors { right: parent.right; top: parent.top; margins: 3 }
                    width: 7; height: 7; radius: 3.5
                    color: Theme.success
                    opacity: 0
                    Connections {
                        target: Core.mapping
                        function onActivityChanged() { inputLedAnim.restart() }
                    }
                    NumberAnimation { id: inputLedAnim; target: inputLed; property: "opacity"; from: 1; to: 0; duration: 250 }
                }
            }

            // running functions (on a short bar: the number alone)
            Row {
                spacing: 6
                HoverHandler { id: runningHover }
                ToolTip.text: qsTr("%n running", "", Core.runningFunctions)
                ToolTip.visible: window.compactBar && runningHover.hovered
                Rectangle {
                    width: 8; height: 8; radius: 4
                    anchors.verticalCenter: parent.verticalCenter
                    color: Core.runningFunctions > 0 ? Theme.success : Theme.textFaint
                }
                Text {
                    text: window.compactBar ? Core.runningFunctions : qsTr("%n running", "", Core.runningFunctions)
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                }
            }

            // grand master
            Rectangle {
                Layout.preferredWidth: window.compactBar ? 150 : 190
                Layout.preferredHeight: 36
                radius: Theme.radiusSmall
                color: Theme.surfaceRaised
                border.color: Theme.border

                RowLayout {
                    anchors { fill: parent; leftMargin: 10; rightMargin: 10 }
                    spacing: 8
                    Text { text: "GM"; color: Theme.textDim; font.weight: Font.Bold; font.pixelSize: Theme.fontSmall }
                    HSlider {
                        id: gmSlider
                        fillColor: Core.grandMaster < 255 ? Theme.warning : Theme.accent
                        Layout.fillWidth: true
                        from: 0; to: 255; stepSize: 1
                        value: Core.grandMaster
                        onMoved: Core.grandMaster = value
                        MapTarget { target: ({ target: "grandMaster" }) }
                    }
                    Text {
                        Layout.preferredWidth: 34
                        horizontalAlignment: Text.AlignRight
                        text: Math.round(Core.grandMaster * 100 / 255) + "%"
                        color: Core.grandMaster < 255 ? Theme.warning : Theme.text
                        font.family: Theme.monoFamily
                        font.pixelSize: Theme.fontSmall
                    }
                }
            }

            // the manual, on the page on screen
            AppButton {
                text: "?"
                flat: true
                font.weight: Font.Bold
                ToolTip.text: qsTr("Manual (F12)")
                ToolTip.visible: hovered
                onClicked: window.showHelp()
            }
            AppButton {
                text: window.narrowBar ? "" : qsTr("Blackout")
                iconGlyph: Theme.iconPower
                highlighted: Core.blackout
                accentColor: Theme.danger
                onClicked: Core.blackout = !Core.blackout
                ToolTip.text: window.narrowBar ? qsTr("Blackout") + " (Ctrl+B)" : "Ctrl+B"
                ToolTip.visible: hovered
                MapTarget { target: ({ target: "blackout" }) }
            }
            AppButton {
                text: window.narrowBar ? "" : qsTr("Stop all")
                iconGlyph: Theme.iconStop
                accentColor: Theme.danger
                primary: true
                onClicked: Core.stopAll()
                ToolTip.text: window.narrowBar ? qsTr("Stop all") + " (Esc)" : "Esc"
                ToolTip.visible: hovered
                MapTarget { target: ({ target: "stopAll" }) }
            }
        }
    }

    /* ****************************************************************
     * Navigation rail + pages
     * ****************************************************************/
    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: navRail
            Layout.fillHeight: true
            Layout.preferredWidth: window.locked ? 0 : 88
            visible: !window.locked
            color: Theme.background

            Column {
                id: navColumn
                anchors { top: parent.top; topMargin: 10; horizontalCenter: parent.horizontalCenter }
                spacing: 2

                readonly property var entries: [
                    { text: qsTr("Live"), icon: Theme.iconGrid, key: "F1", page: window.pageLive },
                    { text: qsTr("Show"), icon: Theme.iconPlay, key: "F2", page: window.pageShow },
                    { text: qsTr("Playbacks"), icon: Theme.iconStepForward, key: "F10", page: window.pagePlaybacks },
                    { text: qsTr("Program"), icon: Theme.iconSliders, key: "F3", page: window.pageProgram },
                    { text: qsTr("Scenes"), icon: Theme.iconWand, key: "F4", page: window.pageScenes },
                    { text: qsTr("Patch"), icon: Theme.iconLight, key: "F5", page: window.pagePatch },
                    { text: qsTr("Fixtures"), icon: Theme.iconPen, key: "F6", page: window.pageFixtures },
                    { text: qsTr("Outputs"), icon: Theme.iconPlug, key: "F7", page: window.pageOutputs },
                    { text: qsTr("Mappings"), icon: Theme.iconTarget, key: "F8", page: window.pageMappings },
                    { text: qsTr("Options"), icon: Theme.iconGear, key: "F9", page: window.pageOptions }
                ]
                // a short window: shorter buttons, so the version stays in sight
                readonly property real buttonHeight: Math.max(48, Math.min(66, (navRail.height - 56) / entries.length - spacing))

                Repeater {
                    model: navColumn.entries
                    delegate: NavButton {
                        required property var modelData
                        text: modelData.text
                        iconGlyph: modelData.icon
                        shortcutHint: modelData.key
                        height: navColumn.buttonHeight
                        checkable: false
                        checked: window.currentPage === modelData.page
                        onClicked: window.goTo(modelData.page)
                        // locked: it asks the password
                        IconLabel {
                            visible: window.pageLocked(parent.modelData.page)
                            anchors { right: parent.right; top: parent.top; margins: 6 }
                            icon: Theme.iconLock
                            size: 10
                            color: Theme.warning
                        }
                    }
                }
            }

            // the processor the application takes: a warning colour when the
            // output may not keep its pace
            Text {
                anchors { bottom: versionText.top; bottomMargin: 3; horizontalCenter: parent.horizontalCenter }
                visible: Core.cpuLoad >= 0
                text: qsTr("CPU %1 %").arg(Core.cpuLoad)
                color: Core.cpuLoad >= 90 ? Theme.readableOn(Theme.danger, Theme.background)
                     : Core.cpuLoad >= 70 ? Theme.readableOn(Theme.warning, Theme.background) : Theme.textFaint
                font.pixelSize: 10
                font.weight: Core.cpuLoad >= 70 ? Font.Bold : Font.Normal
                HoverHandler { id: cpuHover }
                ToolTip.visible: cpuHover.hovered
                ToolTip.text: qsTr("Processor used by DMX Total Control (all the cores of the computer). Above 70 % the output may lose its pace: close other programs or stop some effects.")
            }
            Text {
                id: versionText
                anchors { bottom: parent.bottom; bottomMargin: 10; horizontalCenter: parent.horizontalCenter }
                text: "v" + Core.appVersion
                color: Theme.textFaint
                font.pixelSize: 10
            }

            Rectangle {
                width: 1
                color: Theme.border
                anchors { top: parent.top; bottom: parent.bottom; right: parent.right }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // learn mode: what to do next
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 38
                visible: Core.mapping.learnMode && !window.locked
                color: Qt.alpha(Theme.learn, 0.18)
                Rectangle { width: parent.width; height: 1; anchors.bottom: parent.bottom; color: Theme.learn }

                RowLayout {
                    anchors { fill: parent; leftMargin: 14; rightMargin: 8 }
                    spacing: 10
                    IconLabel { icon: Theme.iconTarget; color: Theme.learnText }
                    Text {
                        Layout.fillWidth: true
                        readonly property var pending: Core.mapping.pendingTarget
                        text: pending.target === undefined
                              ? qsTr("Learn mode: click a purple control (Shift+click still uses it)")
                              : qsTr("Move a control of your device or press a key for: %1  ·  Esc cancels").arg(Core.mapping.describeTarget(pending))
                        color: Theme.text
                        font.weight: pending.target === undefined ? Font.Normal : Font.DemiBold
                        elide: Text.ElideRight
                    }
                    AppButton {
                        text: qsTr("Mappings")
                        implicitHeight: 28
                        flat: true
                        onClicked: window.currentPage = window.pageMappings
                    }
                    AppButton {
                        text: qsTr("Done")
                        implicitHeight: 28
                        accentColor: Theme.learn
                        primary: true
                        onClicked: Core.mapping.learnMode = false
                    }
                }
            }

            // a new version on GitHub
            UpdateBanner {
                Layout.fillWidth: true
                Layout.preferredHeight: implicitHeight
                visible: wanted && !window.locked
                onInstallNow: window.installUpdate()
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: window.locked ? window.pageShow : window.currentPage
                // after the pages' own onVisibleChanged
                onCurrentIndexChanged: Qt.callLater(window.dropHiddenFocus)

                LivePage {
                    mixerView: window.showMixer
                    onEditScene: (id) => { Core.programmer.loadScene(id); window.currentPage = window.pageProgram }
                    onOpenScene: (id) => { scenesPage.select(id); window.currentPage = window.pageScenes }
                }
                ShowPage {
                    id: showPage
                    locked: window.locked
                    editLocked: Core.settings.lockedScreens.indexOf("show") >= 0
                    fullScreen: window.visibility === Window.FullScreen
                    // the kiosk unlocks with the password, if there is one
                    onLockRequested: (lock) => { if (lock) window.locked = true; else lockDialog.openUnlock(() => window.locked = false) }
                    onFullScreenRequested: (enable) => window.setFullScreen(enable)
                    onOpenScene: (id) => { scenesPage.select(id); window.goTo(window.pageScenes) }
                }
                ProgrammerPage {
                    id: programmerPage
                    startFx: window.showFx
                    startPalettes: window.showPalettes
                    startKeypad: window.showKeypad
                    startRecord: window.showRecord
                    onMessage: (text, isError) => window.showMessage(text, isError)
                }
                ScenesPage {
                    id: scenesPage
                    onEditInProgrammer: (id) => { Core.programmer.loadScene(id); window.currentPage = window.pageProgram }
                    onOpenFx: { programmerPage.openFx(); window.currentPage = window.pageProgram }
                    onMessage: (text, isError) => window.showMessage(text, isError)
                }
                PatchPage { id: patchPage; onMessage: (text, isError) => window.showMessage(text, isError) }
                FixtureEditorPage { onMessage: (text, isError) => window.showMessage(text, isError) }
                OutputsPage { levelsView: window.showLevels }
                MappingsPage { id: mappingsPage; onMessage: (text, isError) => window.showMessage(text, isError) }
                OptionsPage {
                    onInstallUpdate: window.installUpdate()
                    onManualWanted: helpPanel.show("index")
                    onGuideWanted: firstSteps.start()
                }
                // (the last one: its index is pagePlaybacks)
                PlaybacksPage {
                    id: playbacksPage
                    onOpenScene: (id) => { scenesPage.select(id); window.goTo(window.pageScenes) }
                }
            }
        }
    }

    /* ****************************************************************
     * Dialogs
     * ****************************************************************/
    component ConfirmPopup: Popup {
        id: popupRoot
        property string heading: ""
        property string body: ""
        default property alias buttons: buttonRow.data

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 460
        modal: true
        padding: 18
        closePolicy: Popup.NoAutoClose
        background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }
        contentItem: ColumnLayout {
            spacing: 12
            Text { text: popupRoot.heading; color: Theme.text; font.pixelSize: Theme.fontLarge; font.weight: Font.Bold }
            Text { Layout.fillWidth: true; text: popupRoot.body; color: Theme.textDim; wrapMode: Text.WordWrap }
            RowLayout {
                id: buttonRow
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
            }
        }
    }

    LockDialog { id: lockDialog }

    // the sounds, videos and pictures of the show that opened and aren't there
    // (a locked SHOW, a kiosk, only says it)
    MissingFilesDialog {
        id: missingFiles
        function notify() {
            if (window.locked)
                window.showMessage(qsTr("%n file(s) of the show are missing", "", Core.missingFiles.length), true)
            else if (!opened)
                open()
        }
        onMessage: (text, isError) => window.showMessage(text, isError)
    }
    Connections {
        target: Core
        function onMissingFilesChanged() {
            if (Core.missingFiles.length > 0)
                missingFiles.notify()
        }
    }

    ConfirmPopup {
        id: closeDialog
        /** What replaces the show afterwards (New, Open...); null: closing */
        property var then: null
        function proceed() {
            const action = then
            then = null
            if (action) {
                action()
            } else {
                window.closeConfirmed = true
                window.close()
            }
        }
        heading: qsTr("Save changes?")
        body: qsTr("“%1” has unsaved changes.").arg(Core.showTitle)
        AppButton { text: qsTr("Cancel"); onClicked: { closeDialog.then = null; closeDialog.close() } }
        AppButton {
            text: qsTr("Don't save")
            onClicked: { closeDialog.close(); closeDialog.proceed() }
        }
        AppButton {
            text: qsTr("Save")
            primary: true
            onClicked: {
                closeDialog.close()
                window.saveOrAsk(() => closeDialog.proceed())
            }
        }
    }

    ConfirmPopup {
        id: recoveryDialog
        heading: qsTr("Recover the unsaved show?")
        body: qsTr("The application didn't close properly last time. There is a recovery copy of “%1” from %2, with the changes that weren't saved.")
              .arg(Core.recoveryShow).arg(Core.recoveryTime)
        AppButton { text: qsTr("Discard"); onClicked: { Core.discardRecovery(); recoveryDialog.close(); window.tellOpenProblems() } }
        AppButton {
            text: qsTr("Recover")
            primary: true
            onClicked: {
                recoveryDialog.close()
                if (Core.recoverShow())
                    window.showMessage(qsTr("Show recovered. Remember to save it."))
                else
                    window.showMessage(Core.lastError, true)
            }
        }
    }

    /* ****************************************************************
     * Toast notifications
     * ****************************************************************/

    // the manual (F12, «?») and the first steps guide (first start, Options)
    // below the top bar: blackout and stop all stay at hand
    HelpPanel {
        id: helpPanel
        y: window.header ? window.header.height : 0
        height: window.height - y
    }
    FirstStepsGuide {
        id: firstSteps
        pagePatch: window.pagePatch
        pageProgram: window.pageProgram
        pageLive: window.pageLive
        pagePlaybacks: window.pagePlaybacks
        onPageWanted: (page) => window.goTo(page)
        onPracticeWanted: (url) => window.whenSaved(() => window.openShow(url))
        onManualWanted: helpPanel.show("primeros-pasos")
    }

    Popup {
        id: toast

        property string text: ""
        property bool isError: false

        x: (window.width - width) / 2
        y: window.height - height - 90
        // a long message (a path) wraps inside the window
        width: Math.min(implicitWidth, window.width - 64)
        padding: 12
        closePolicy: Popup.NoAutoClose
        background: Rectangle {
            radius: Theme.radius
            color: toast.isError ? Theme.fill(Theme.danger) : Theme.surfaceRaised
            border.color: toast.isError ? Qt.darker(Theme.danger, 1.3) : Theme.borderStrong
            // a click closes it (an error stays longer: it covers what is below)
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: toast.close() }
        }
        contentItem: Text {
            text: toast.text
            color: toast.isError ? Theme.textOn(Theme.fill(Theme.danger)) : Theme.text
            font.pixelSize: Theme.fontNormal
            wrapMode: Text.Wrap
        }
        onOpened: toastTimer.restart()

        Timer { id: toastTimer; interval: toast.isError ? 8000 : 3200; onTriggered: toast.close() }
    }
}
