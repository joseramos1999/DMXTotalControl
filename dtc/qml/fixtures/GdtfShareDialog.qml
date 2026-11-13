/*
  DMX Total Control
  GdtfShareDialog.qml

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
 * GDTF-Share, with the user's own free account: its fixtures searched,
 * downloaded and converted into the library (GdtfShare). Opened from
 * Options, and from Add fixtures, which patches them (patchWanted).
 */
Popup {
    id: dialog

    /** Opened from Add fixtures: a fixture of the library can be patched */
    property bool fromPatch: false
    /** The manufacturer chosen ("" for all), and the filter: "" all,
     *  "manufacturer", "library" or "updates" (see GdtfShare::search) */
    property string manufacturer: ""
    property string filter: ""
    /** The search: {rows, total}, and the manufacturers it finds */
    property var result: ({ rows: [], total: 0 })
    property var makers: []
    /** The row chosen (a fixture type), and its revision to download */
    property var selected: null
    property int chosenRid: 0
    /** The upload whose conversion this dialog waits for (0: none), then
     *  the fixture it put in the library: {uuid, manufacturer, model} */
    property int waitingRid: 0
    property string waitingUuid: ""
    property var imported: null

    readonly property var share: Core.library.share
    readonly property bool busy: share.busy || Core.library.importing
    /** Every revision of the row chosen, newest first, and the one chosen */
    readonly property var revisions: selected && share.listRevision >= 0 ? share.revisions(selected.uuid) : []
    readonly property var chosen: {
        for (const revision of revisions)
            if (revision.rid === chosenRid)
                return revision
        return revisions.length > 0 ? revisions[0] : selected
    }
    /** What downloading the revision chosen does: "import" (not in the
     *  library), "update" (another revision is, or one imported from a
     *  file), "again" (this one is) */
    readonly property string action: {
        if (!selected || !chosen)
            return ""
        if (selected.libraryRid > 0 && chosen.rid === selected.libraryRid)
            return "again"
        return selected.status.length > 0 ? "update" : "import"
    }
    /** The fixture of the library the row chosen is, to patch it */
    readonly property var patchTarget: {
        if (!selected)
            return null
        if (imported && imported.uuid === selected.uuid)
            return imported
        if (selected.status.length > 0 && selected.libraryModel.length > 0)
            return { manufacturer: selected.libraryManufacturer || selected.manufacturer, model: selected.libraryModel }
        return null
    }

    /** "Patch it": the fixture of the library, for Add fixtures */
    signal patchWanted(string manufacturer, string model)

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(1100, parent.width - 80)
    height: Math.min(680, parent.height - 80)
    modal: true
    padding: 18

    /** Open on the fixtures with a newer revision (Options › Update) */
    function openUpdates() {
        filter = "updates"
        open()
    }
    /** $keepPlace: the rows changed under the user (a download, a new
     *  list), and the list stays where it was; else it goes to the top */
    function runSearch(keepPlace) {
        const y = fixtureList.contentY
        result = share.search(search.text, manufacturer, filter, 400)
        makers = share.manufacturers(search.text, filter)
        if (keepPlace) {
            fixtureList.contentY = y
            fixtureList.returnToBounds()
        }
        // the row chosen as the list has it now (its status after an import)
        if (selected) {
            const same = result.rows.find(row => row.uuid === selected.uuid)
            if (same)
                selected = same
        }
    }
    function setFilter(name) {
        filter = name
        runSearch()
    }
    function choose(row) {
        selected = row
        chosenRid = row.rid
    }
    function download(rid) {
        imported = null
        if (share.download([rid]) > 0) {
            waitingRid = rid
            waitingUuid = selected.uuid
        }
    }
    /** The download waited for is in the library: it can be patched */
    function checkImported() {
        if (waitingRid === 0 || Core.library.importing)
            return
        const found = Core.library.lastImported.find(f => f.rid === waitingRid)
        if (!found)
            return
        imported = { uuid: waitingUuid, manufacturer: found.manufacturer, model: found.model }
        waitingRid = 0
    }

    function dateText(date) {
        return date && !isNaN(date.getTime()) ? date.toLocaleDateString(Qt.locale(), Locale.ShortFormat) : ""
    }
    function sizeText(bytes) {
        if (!(bytes > 0))
            return ""
        if (bytes < 1048576)
            return qsTr("%1 KB").arg(Math.max(1, Math.round(bytes / 1024)))
        return qsTr("%1 MB").arg(Number(bytes / 1048576).toLocaleString(Qt.locale(), 'f', 1))
    }
    function ratingText(rating) {
        return Number(rating).toLocaleString(Qt.locale(), 'f', 1)
    }
    function uploaderText(entry) {
        return entry.uploader === "manufacturer" ? qsTr("Manufacturer") : qsTr("User")
    }
    /** "Rev 3 · 03/11/2026 · Manufacturer", for the list of revisions */
    function revisionText(entry) {
        return [entry.revision, dateText(entry.modified), uploaderText(entry)].filter(t => t.length > 0).join(" · ")
    }

    onOpened: {
        share.load()
        search.text = ""
        manufacturer = ""
        selected = null
        imported = null
        runSearch()
        search.forceActiveFocus()
    }
    onClosed: {
        filter = ""
        login.close()
    }

    Connections {
        target: Core.library.share
        enabled: dialog.visible
        function onListChanged() { dialog.runSearch(true) }
        function onIndexChanged() { dialog.runSearch(true) }
        function onUpdatesChanged() {
            // (all of them updated: the filter goes with its button, and
            // the list starts at the top)
            const emptied = dialog.filter === "updates" && dialog.share.updatesCount === 0
            if (emptied)
                dialog.filter = ""
            dialog.runSearch(!emptied)
        }
        function onLoginNeeded() { login.ask(true) }
        // signed in from the popup: it closes, and the list comes if it is old
        function onChanged() {
            if (dialog.share.signedIn && login.visible) {
                login.close()
                dialog.share.load()
            }
        }
    }
    Connections {
        target: Core.library
        enabled: dialog.visible
        function onImportChanged() { dialog.checkImported() }
    }
    // the search runs a moment after the typing stops
    Timer {
        id: searchTimer
        interval: 200
        onTriggered: dialog.runSearch()
    }

    /** A small label on a fixture: who uploaded it, how the library has it */
    component Badge: Rectangle {
        property alias text: badgeText.text
        /** Its colour: filled with it, else a plain outline */
        property color tint: "transparent"
        readonly property bool filled: tint.a > 0
        implicitWidth: badgeText.implicitWidth + 10
        implicitHeight: 18
        radius: 4
        color: filled ? Theme.fill(tint) : Theme.surfaceRaised
        border.color: filled ? "transparent" : Theme.border
        Text {
            id: badgeText
            anchors.centerIn: parent
            font.pixelSize: 10
            font.weight: Font.Bold
            color: parent.filled ? Theme.textOn(parent.color) : Theme.textDim
        }
    }

    background: Rectangle {
        radius: Theme.radius
        color: Theme.surface
        border.color: Theme.borderStrong
    }

    contentItem: ColumnLayout {
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Text {
                text: qsTr("GDTF-Share")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
            }
            Item { Layout.fillWidth: true }
            TextField {
                id: search
                Layout.preferredWidth: 300
                placeholderText: qsTr("Search manufacturer or fixture…")
                color: Theme.text
                background: Rectangle { radius: Theme.radiusSmall; color: Theme.background; border.color: search.activeFocus ? Theme.accent : Theme.border }
                onTextEdited: searchTimer.restart()
                onAccepted: { searchTimer.stop(); dialog.runSearch() }
            }
            // the account: Sign in, or its menu (also signed out with a saved
            // password: the menu can still forget it)
            AppButton {
                id: accountButton
                iconGlyph: Theme.iconLock
                flat: true
                text: dialog.share.sessionPossible && dialog.share.user.length > 0 ? dialog.share.user : qsTr("Sign in")
                onClicked: dialog.share.sessionPossible || dialog.share.remembered
                           ? accountMenu.popup(accountButton, 0, accountButton.height) : login.ask(false)
            }
            AppButton {
                iconGlyph: Theme.iconRefresh
                flat: true
                enabled: !dialog.share.busy && !dialog.share.listLoading
                ToolTip.text: qsTr("Refresh")
                ToolTip.visible: hovered
                onClicked: dialog.share.refresh(true)
            }
            Text {
                visible: text.length > 0
                text: dialog.dateText(dialog.share.listDate).length > 0
                      ? qsTr("List of %1").arg(dialog.dateText(dialog.share.listDate)) : ""
                color: Theme.textFaint
                font.pixelSize: Theme.fontSmall
            }
        }

        // the filters (a Flow: long translations wrap)
        Flow {
            Layout.fillWidth: true
            spacing: 6
            AppButton { text: qsTr("All"); implicitHeight: 28; flat: true; highlighted: dialog.filter === ""; onClicked: dialog.setFilter("") }
            AppButton {
                text: qsTr("From manufacturers")
                implicitHeight: 28
                flat: true
                highlighted: dialog.filter === "manufacturer"
                onClicked: dialog.setFilter("manufacturer")
            }
            AppButton {
                text: qsTr("In my library")
                implicitHeight: 28
                flat: true
                highlighted: dialog.filter === "library"
                onClicked: dialog.setFilter("library")
            }
            AppButton {
                visible: dialog.share.updatesCount > 0
                text: qsTr("Updates (%1)").arg(dialog.share.updatesCount)
                implicitHeight: 28
                flat: true
                highlighted: dialog.filter === "updates"
                onClicked: dialog.setFilter("updates")
            }
            AppButton {
                visible: dialog.filter === "updates" && dialog.share.updatesCount > 0
                text: qsTr("Update all")
                iconGlyph: Theme.iconDownload
                implicitHeight: 28
                primary: true
                enabled: !dialog.busy
                onClicked: dialog.share.updateAll()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            // the manufacturers of what the search finds
            Panel {
                Layout.preferredWidth: 240
                Layout.fillHeight: true
                title: qsTr("Manufacturer")

                ListView {
                    id: makerList
                    anchors { fill: parent; margins: 4 }
                    clip: true
                    // ("All manufacturers" first, once there is a list)
                    model: dialog.share.listCount > 0
                           ? [{ name: "", count: dialog.makers.reduce((sum, m) => sum + m.count, 0) }].concat(dialog.makers) : []
                    ScrollBar.vertical: ScrollBar {}
                    delegate: ItemDelegate {
                        id: makerRow
                        required property var modelData
                        width: makerList.width
                        height: 32
                        topPadding: 0
                        bottomPadding: 0
                        highlighted: modelData.name === dialog.manufacturer
                        background: Rectangle {
                            radius: Theme.radiusSmall
                            color: makerRow.highlighted ? Theme.accentFill : makerRow.hovered ? Theme.surfaceHover : "transparent"
                        }
                        contentItem: RowLayout {
                            spacing: 6
                            Text {
                                Layout.fillWidth: true
                                text: makerRow.modelData.name.length > 0 ? makerRow.modelData.name : qsTr("All manufacturers")
                                color: makerRow.highlighted ? Theme.accentText : Theme.text
                                font.pixelSize: Theme.fontNormal
                                elide: Text.ElideRight
                            }
                            Text {
                                text: makerRow.modelData.count
                                color: makerRow.highlighted ? Theme.accentText : Theme.textFaint
                                font.pixelSize: Theme.fontSmall
                            }
                        }
                        onClicked: { dialog.manufacturer = modelData.name; dialog.runSearch() }
                    }
                }
            }

            // the fixture types: the newest revision of each one
            Panel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: qsTr("Fixtures")

                ListView {
                    id: fixtureList
                    anchors { fill: parent; margins: 4 }
                    clip: true
                    spacing: 2
                    model: dialog.result.rows
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ScrollBar {}
                    delegate: Rectangle {
                        id: fixtureRow
                        required property var modelData
                        readonly property bool current: dialog.selected !== null && dialog.selected.uuid === modelData.uuid
                        width: fixtureList.width - 8
                        height: 50
                        radius: Theme.radiusSmall
                        color: current ? Theme.mix(Theme.surface, Theme.accent, 0.22)
                                       : rowMouse.containsMouse ? Theme.surfaceHover : "transparent"
                        border.color: current ? Theme.accent : "transparent"

                        ColumnLayout {
                            anchors { fill: parent; leftMargin: 10; rightMargin: 8; topMargin: 6; bottomMargin: 6 }
                            spacing: 3
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6
                                Text {
                                    Layout.fillWidth: true
                                    text: fixtureRow.modelData.fixture
                                    color: Theme.text
                                    font.pixelSize: Theme.fontNormal
                                    font.weight: Font.DemiBold
                                    elide: Text.ElideRight
                                }
                                Badge {
                                    text: fixtureRow.modelData.uploader === "manufacturer" ? qsTr("Manufacturer") : qsTr("User")
                                    tint: fixtureRow.modelData.uploader === "manufacturer" ? Theme.accent : "transparent"
                                }
                                Row {
                                    visible: fixtureRow.modelData.rating > 0
                                    spacing: 3
                                    IconLabel {
                                        anchors.verticalCenter: parent.verticalCenter
                                        icon: Theme.iconStar
                                        size: 10
                                        color: Theme.readable(Theme.warning)
                                    }
                                    Text {
                                        text: dialog.ratingText(fixtureRow.modelData.rating)
                                        color: Theme.textDim
                                        font.pixelSize: Theme.fontSmall
                                    }
                                }
                                Badge {
                                    visible: fixtureRow.modelData.status.length > 0
                                    text: fixtureRow.modelData.status === "older" ? qsTr("Newer revision")
                                          : fixtureRow.modelData.status === "file" ? qsTr("From a file") : qsTr("In library")
                                    tint: fixtureRow.modelData.status === "older" ? Theme.warning
                                          : fixtureRow.modelData.status === "current" ? Theme.success : "transparent"
                                }
                            }
                            Text {
                                Layout.fillWidth: true
                                text: [fixtureRow.modelData.manufacturer, fixtureRow.modelData.revision,
                                       dialog.dateText(fixtureRow.modelData.modified),
                                       fixtureRow.modelData.modes.length > 0 ? qsTr("%n mode(s)", "", fixtureRow.modelData.modes.length) : ""]
                                      .filter(t => t.length > 0).join(" · ")
                                // (on the tint of the row chosen, a text that reads)
                                color: fixtureRow.current ? Theme.textDim : Theme.textFaint
                                font.pixelSize: Theme.fontSmall
                                elide: Text.ElideRight
                            }
                        }
                        MouseArea {
                            id: rowMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: dialog.choose(fixtureRow.modelData)
                        }
                    }
                    // (only the first ones: the list stays quick)
                    footer: Text {
                        width: fixtureList.width - 8
                        visible: dialog.result.total > dialog.result.rows.length
                        height: visible ? implicitHeight + 16 : 0
                        topPadding: 8
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        text: qsTr("Showing %1 of %2: type more to narrow the list")
                              .arg(dialog.result.rows.length).arg(dialog.result.total.toLocaleString(Qt.locale(), 'f', 0))
                        color: Theme.textFaint
                        font.pixelSize: Theme.fontSmall
                    }
                }

                // nothing to show, and why
                ColumnLayout {
                    anchors.centerIn: parent
                    width: parent.width - 40
                    visible: fixtureList.count === 0
                    spacing: 10
                    Text {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        text: {
                            if (dialog.share.listCount === 0) {
                                if (dialog.share.listLoading)
                                    return qsTr("Downloading the list of GDTF-Share…")
                                if (!dialog.share.sessionPossible)
                                    return qsTr("Sign in with your GDTF-Share account to see its fixtures")
                                return dialog.share.error.length > 0 ? dialog.share.error : dialog.share.notice
                            }
                            return search.text.trim().length > 0 ? qsTr("No fixture matches “%1”").arg(search.text.trim())
                                                                  : qsTr("No fixture with this filter")
                        }
                        color: Theme.textFaint
                    }
                    AppButton {
                        Layout.alignment: Qt.AlignHCenter
                        visible: dialog.share.listCount === 0 && !dialog.share.listLoading && !dialog.share.sessionPossible
                        text: qsTr("Sign in")
                        iconGlyph: Theme.iconLock
                        primary: true
                        onClicked: login.ask(false)
                    }
                }
            }

            // the fixture type chosen
            Panel {
                Layout.preferredWidth: 320
                Layout.fillHeight: true
                title: qsTr("Details")

                Text {
                    anchors.centerIn: parent
                    visible: !dialog.selected
                    text: qsTr("Choose a fixture")
                    color: Theme.textFaint
                }

                ScrollView {
                    id: detailsView
                    anchors { fill: parent; margins: 12 }
                    visible: dialog.selected !== null
                    contentWidth: availableWidth
                    clip: true

                    ColumnLayout {
                        width: detailsView.availableWidth
                        spacing: 10

                        Column {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                width: parent.width
                                text: dialog.chosen ? dialog.chosen.fixture : ""
                                color: Theme.text
                                font.pixelSize: Theme.fontLarge
                                font.weight: Font.Bold
                                wrapMode: Text.Wrap
                            }
                            Text {
                                width: parent.width
                                text: dialog.chosen ? dialog.chosen.manufacturer : ""
                                color: Theme.textDim
                                wrapMode: Text.Wrap
                            }
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            columns: 2
                            columnSpacing: 10
                            rowSpacing: 6

                            Text { text: qsTr("Uploaded by"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                            Text {
                                Layout.fillWidth: true
                                text: dialog.chosen ? [dialog.uploaderText(dialog.chosen), dialog.chosen.creator].filter(t => t.length > 0).join(" · ") : ""
                                color: Theme.text
                                font.pixelSize: Theme.fontSmall
                                wrapMode: Text.Wrap
                            }
                            Text {
                                visible: dialog.chosen && dialog.chosen.rating > 0
                                text: qsTr("Rating")
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSmall
                            }
                            Row {
                                visible: dialog.chosen && dialog.chosen.rating > 0
                                spacing: 4
                                IconLabel {
                                    anchors.verticalCenter: parent.verticalCenter
                                    icon: Theme.iconStar
                                    size: 11
                                    color: Theme.readable(Theme.warning)
                                }
                                Text {
                                    text: dialog.chosen ? dialog.ratingText(dialog.chosen.rating) : ""
                                    color: Theme.text
                                    font.pixelSize: Theme.fontSmall
                                }
                            }
                            Text { text: qsTr("Revision"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                            Text {
                                Layout.fillWidth: true
                                text: dialog.chosen ? [dialog.chosen.revision, dialog.dateText(dialog.chosen.modified)].filter(t => t.length > 0).join(" · ") : ""
                                color: Theme.text
                                font.pixelSize: Theme.fontSmall
                                wrapMode: Text.Wrap
                            }
                            Text {
                                visible: dialog.chosen && dialog.chosen.version.length > 0
                                text: qsTr("GDTF version")
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSmall
                            }
                            Text {
                                visible: dialog.chosen && dialog.chosen.version.length > 0
                                text: dialog.chosen ? dialog.chosen.version : ""
                                color: Theme.text
                                font.pixelSize: Theme.fontSmall
                            }
                            Text {
                                visible: dialog.chosen && dialog.chosen.filesize > 0
                                text: qsTr("Size")
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSmall
                            }
                            Text {
                                visible: dialog.chosen && dialog.chosen.filesize > 0
                                text: dialog.chosen ? dialog.sizeText(dialog.chosen.filesize) : ""
                                color: Theme.text
                                font.pixelSize: Theme.fontSmall
                            }
                            Text {
                                Layout.alignment: Qt.AlignTop
                                visible: dialog.chosen && dialog.chosen.modes.length > 0
                                text: qsTr("Modes")
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSmall
                            }
                            Column {
                                Layout.fillWidth: true
                                visible: dialog.chosen && dialog.chosen.modes.length > 0
                                spacing: 2
                                Repeater {
                                    model: dialog.chosen ? dialog.chosen.modes : []
                                    delegate: Text {
                                        required property var modelData
                                        width: parent.width
                                        text: qsTr("%1 — %2 ch").arg(modelData.name).arg(modelData.channels)
                                        color: Theme.text
                                        font.pixelSize: Theme.fontSmall
                                        wrapMode: Text.Wrap
                                    }
                                }
                            }
                        }

                        // an older revision can be chosen
                        Text {
                            visible: dialog.revisions.length > 1
                            text: qsTr("Revisions")
                            color: Theme.textDim
                            font.pixelSize: Theme.fontSmall
                        }
                        ComboBox {
                            Layout.fillWidth: true
                            visible: dialog.revisions.length > 1
                            model: dialog.revisions.map(r => dialog.revisionText(r))
                            currentIndex: Math.max(0, dialog.revisions.findIndex(r => r.rid === dialog.chosenRid))
                            onActivated: (index) => dialog.chosenRid = dialog.revisions[index].rid
                        }

                        // how the library has it
                        Text {
                            Layout.fillWidth: true
                            visible: text.length > 0
                            wrapMode: Text.WordWrap
                            text: !dialog.selected ? ""
                                  : dialog.selected.status === "file" ? qsTr("In your library (imported from a file)")
                                  : dialog.selected.status.length > 0 ? qsTr("In your library as “%1”").arg(dialog.selected.libraryModel) : ""
                            color: Theme.text
                            font.pixelSize: Theme.fontSmall
                        }
                        Text {
                            Layout.fillWidth: true
                            visible: dialog.selected !== null && dialog.selected.status === "older"
                            wrapMode: Text.WordWrap
                            text: dialog.selected ? qsTr("A newer revision: %1 (%2)").arg(dialog.selected.revision)
                                                                                      .arg(dialog.dateText(dialog.selected.modified)) : ""
                            color: Theme.readable(Theme.warning)
                            font.pixelSize: Theme.fontSmall
                        }
                        Text {
                            Layout.fillWidth: true
                            visible: dialog.selected !== null && dialog.selected.replaces === true
                            wrapMode: Text.WordWrap
                            text: qsTr("Another upload of this fixture is in your library: it will be replaced")
                            color: Theme.readable(Theme.warning)
                            font.pixelSize: Theme.fontSmall
                        }

                        AppButton {
                            Layout.fillWidth: true
                            visible: dialog.action.length > 0
                            text: dialog.action === "again" ? qsTr("Download again")
                                  : dialog.action === "update" ? qsTr("Update to this revision") : qsTr("Download and import")
                            iconGlyph: Theme.iconDownload
                            primary: !(dialog.fromPatch && dialog.patchTarget)
                            enabled: !dialog.busy
                            onClicked: dialog.download(dialog.chosen.rid)
                        }
                        AppButton {
                            Layout.fillWidth: true
                            visible: dialog.fromPatch && dialog.patchTarget !== null
                            text: qsTr("Patch it")
                            iconGlyph: Theme.iconPlus
                            primary: true
                            enabled: !dialog.busy
                            onClicked: {
                                dialog.patchWanted(dialog.patchTarget.manufacturer, dialog.patchTarget.model)
                                dialog.close()
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            // what happened: an error, else what is going on, else the last import
            Text {
                Layout.fillWidth: true
                readonly property bool failed: dialog.share.error.length > 0
                text: failed ? dialog.share.error
                             : dialog.share.notice.length > 0 ? dialog.share.notice : Core.library.status
                color: failed ? Theme.readable(Theme.warning) : Theme.textDim
                font.pixelSize: Theme.fontSmall
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }
            // the download, then its conversion
            ProgressBar {
                readonly property bool downloading: dialog.share.state === GdtfShare.Downloading
                Layout.preferredWidth: 160
                visible: downloading || Core.library.importing
                value: downloading ? dialog.share.progress : Core.library.importProgress
                indeterminate: value <= 0
            }
            AppButton {
                visible: dialog.busy
                text: qsTr("Cancel")
                iconGlyph: Theme.iconClose
                // (the downloads that arrived whole are still converted)
                onClicked: {
                    Core.library.cancelShare()
                    dialog.waitingRid = 0
                }
            }
            AppButton {
                text: "gdtf-share.com"
                iconGlyph: Theme.iconLink
                flat: true
                onClicked: Qt.openUrlExternally("https://gdtf-share.com")
            }
            AppButton { text: qsTr("Close"); onClicked: dialog.close() }
        }
    }

    AppMenu {
        id: accountMenu
        MenuItem {
            text: dialog.share.sessionPossible ? qsTr("Sign out") : qsTr("Sign in")
            onTriggered: dialog.share.sessionPossible ? dialog.share.signOut(false) : login.ask(false)
        }
        MenuItem {
            text: qsTr("Forget the saved password")
            visible: dialog.share.remembered
            height: visible ? implicitHeight : 0
            onTriggered: dialog.share.signOut(true)
        }
    }

    // the sign-in, with the user's own account
    Popup {
        id: login

        /** Opened because something waits for the sign-in: closed without
         *  it, what waited is dropped */
        property bool asked: false

        function ask(byRequest) {
            // (open already: a wrong password, what was typed stays)
            if (visible) {
                asked = asked || byRequest
                passwordField.forceActiveFocus()
                return
            }
            asked = byRequest
            userField.text = dialog.share.user
            passwordField.text = ""
            remember.checked = dialog.share.remembered
            open()
            if (userField.text.length > 0)
                passwordField.forceActiveFocus()
            else
                userField.forceActiveFocus()
        }
        function accept() {
            if (userField.text.trim().length === 0 || passwordField.text.length === 0)
                return
            dialog.share.signIn(userField.text, passwordField.text, remember.checked)
            // (the password is never kept here)
            passwordField.text = ""
        }

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 420
        modal: true
        padding: 18
        onClosed: {
            if (asked && !dialog.share.signedIn)
                dialog.share.cancel()
            asked = false
            passwordField.text = ""
        }

        background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.borderStrong }

        contentItem: ColumnLayout {
            spacing: 10

            RowLayout {
                spacing: 10
                IconLabel { icon: Theme.iconLock; size: 18; color: Theme.accent }
                Text {
                    text: qsTr("Sign in to GDTF-Share")
                    color: Theme.text
                    font.pixelSize: Theme.fontLarge
                    font.weight: Font.Bold
                }
            }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Use your own free gdtf-share.com account. Your user name and password are only sent to gdtf-share.com.")
                color: Theme.textDim
            }

            Text { text: qsTr("User name"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
            TextField {
                id: userField
                Layout.fillWidth: true
                color: Theme.text
                onAccepted: passwordField.forceActiveFocus()
            }
            Text { text: qsTr("Password"); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
            TextField {
                id: passwordField
                Layout.fillWidth: true
                echoMode: TextInput.Password
                color: Theme.text
                onAccepted: login.accept()
            }
            CheckBox {
                id: remember
                visible: dialog.share.canRemember
                text: qsTr("Remember on this computer")
            }
            Text {
                Layout.fillWidth: true
                visible: remember.visible
                wrapMode: Text.WordWrap
                text: qsTr("Saved encrypted for your Windows user: anyone using this Windows account can sign in with it.")
                color: Theme.textFaint
                font.pixelSize: Theme.fontSmall
            }
            Text {
                Layout.fillWidth: true
                visible: dialog.share.error.length > 0 && !dialog.share.busy
                wrapMode: Text.WordWrap
                text: dialog.share.error
                color: Theme.danger
            }

            RowLayout {
                Layout.fillWidth: true
                // (GDTF-Share asks the programs to make the accounts on its website)
                AppButton {
                    text: qsTr("Create a free account")
                    flat: true
                    onClicked: Qt.openUrlExternally("https://gdtf-share.com")
                }
                Item { Layout.fillWidth: true }
                AppButton { text: qsTr("Cancel"); onClicked: login.close() }
                AppButton {
                    text: qsTr("Sign in")
                    iconGlyph: Theme.iconLock
                    primary: true
                    enabled: !dialog.share.busy && userField.text.trim().length > 0 && passwordField.text.length > 0
                    onClicked: login.accept()
                }
            }
        }
    }
}
