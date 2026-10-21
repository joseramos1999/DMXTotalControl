/*
  DMX Total Control
  OptionsPage.qml

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

/** Application options, fixture library management and about */
Item {
    id: page

    /** The downloaded update is to be installed now (Main closes the program) */
    signal installUpdate()
    /** The manual and the first steps guide are wanted (Main has them) */
    signal manualWanted()
    signal guideWanted()

    readonly property var themes: [
        { code: "dark", name: qsTr("Dark") },
        { code: "light", name: qsTr("Light") },
        { code: "contrast", name: qsTr("High contrast") },
        { code: "night", name: qsTr("Night (dimmed)") }
    ]

    readonly property var languages: [
        { code: "", name: qsTr("System language") },
        { code: "es_ES", name: "Español" },
        { code: "en_US", name: "English" },
        { code: "pt_BR", name: "Português" },
        { code: "fr_FR", name: "Français" },
        { code: "it_IT", name: "Italiano" },
        { code: "de_DE", name: "Deutsch" }
    ]

    component SectionTitle: Text {
        color: Theme.textDim
        font.pixelSize: Theme.fontSmall
        font.weight: Font.Bold
        font.capitalization: Font.AllUppercase
        font.letterSpacing: 0.8
        topPadding: 8
    }

    component OptionRow: RowLayout {
        property alias label: labelText.text
        property alias hint: hintText.text
        Layout.fillWidth: true
        spacing: 12
        Column {
            Layout.fillWidth: true
            Text { id: labelText; color: Theme.text; font.pixelSize: Theme.fontNormal }
            Text { id: hintText; color: Theme.textFaint; font.pixelSize: Theme.fontSmall; visible: text.length > 0; width: parent.width; wrapMode: Text.WordWrap }
        }
    }

    FileDialog {
        id: gdtfDialog
        title: qsTr("Import GDTF fixtures")
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("GDTF fixtures (*.gdtf *.zip)"), qsTr("All files (*)")]
        onAccepted: Core.library.importFiles(selectedFiles)
    }
    FileDialog {
        id: importDialog
        title: qsTr("Import fixture definitions")
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("Fixture definitions (*.qxf *.gdtf *.zip)"), qsTr("All files (*)")]
        onAccepted: Core.library.importFiles(selectedFiles)
    }

    ColumnLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 14

        Text {
            text: qsTr("Options")
            color: Theme.text
            font.pixelSize: Theme.fontTitle
            font.weight: Font.Bold
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth
            clip: true

            RowLayout {
                width: parent.width
                spacing: 12

                // left column: application
                Panel {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    Layout.preferredHeight: appColumn.implicitHeight + 24

                    ColumnLayout {
                        id: appColumn
                        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 14 }
                        spacing: 12

                        SectionTitle { text: qsTr("General") }
                        OptionRow {
                            label: qsTr("Language")
                            hint: qsTr("Applied the next time the application starts")
                            ComboBox {
                                Layout.preferredWidth: 200
                                textRole: "name"
                                model: page.languages
                                currentIndex: Math.max(0, page.languages.findIndex(l => l.code === Core.settings.language))
                                onActivated: (i) => Core.settings.language = page.languages[i].code
                            }
                        }
                        OptionRow {
                            label: qsTr("Theme")
                            hint: qsTr("Light for bright rooms, high contrast to read from afar, night for a dark booth. The remote control follows it too")
                            ComboBox {
                                Layout.preferredWidth: 200
                                textRole: "name"
                                model: page.themes
                                currentIndex: Math.max(0, page.themes.findIndex(t => t.code === Core.settings.theme))
                                onActivated: (i) => Core.settings.theme = page.themes[i].code
                            }
                        }
                        OptionRow {
                            label: qsTr("Size of the interface")
                            hint: qsTr("Smaller for small laptops, bigger for touch screens. Applied the next time the application starts")
                            SpinBox {
                                from: 80; to: 150; stepSize: 10
                                value: Core.settings.uiScale
                                textFromValue: (v) => v + " %"
                                valueFromText: (t) => parseInt(t)
                                onValueModified: Core.settings.uiScale = value
                            }
                        }
                        OptionRow {
                            label: qsTr("Open the last show at startup")
                            AppSwitch { checked: Core.settings.openLastShow; onToggled: Core.settings.openLastShow = checked }
                        }

                        // the SHOW full screen at startup, and the lock
                        SectionTitle { text: qsTr("SHOW and lock") }
                        OptionRow {
                            label: qsTr("Open the SHOW full screen at startup")
                            hint: qsTr("For installations and kiosks")
                            AppSwitch { checked: Core.settings.startShowFullScreen; onToggled: Core.settings.startShowFullScreen = checked }
                        }
                        OptionRow {
                            label: qsTr("…and locked")
                            hint: qsTr("Only the SHOW, without navigation: hold the lock button to leave (with the password, if there is one)")
                            enabled: Core.settings.startShowFullScreen
                            AppSwitch { checked: Core.settings.startShowLocked; onToggled: Core.settings.startShowLocked = checked }
                        }
                        OptionRow {
                            label: qsTr("Lock screens")
                            hint: Core.settings.lockedScreens.length > 0 ? qsTr("Locked now. Also with the padlock in the top bar")
                                                                       : qsTr("Setup, programming or SHOW editing, with a password or without. Also with the padlock in the top bar")
                            AppButton {
                                text: Core.settings.lockedScreens.length > 0 ? qsTr("Unlock…") : qsTr("Lock…")
                                iconGlyph: Core.settings.lockedScreens.length > 0 ? Theme.iconUnlock : Theme.iconLock
                                onClicked: Core.settings.lockedScreens.length > 0 ? optionsLock.openUnlock(() => Core.settings.lockedScreens = [])
                                                                                  : optionsLock.openLock()
                            }
                        }

                        SectionTitle { text: qsTr("Autosave") }
                        OptionRow {
                            label: qsTr("Save automatically")
                            hint: qsTr("Saved shows are overwritten (the previous version is kept as .bak). Untitled shows are kept apart and offered back after a crash.")
                            AppSwitch { checked: Core.settings.autosaveEnabled; onToggled: Core.settings.autosaveEnabled = checked }
                        }
                        OptionRow {
                            label: qsTr("Every (minutes)")
                            enabled: Core.settings.autosaveEnabled
                            SpinBox {
                                from: 1; to: 60; editable: true
                                value: Core.settings.autosaveMinutes
                                onValueModified: Core.settings.autosaveMinutes = value
                            }
                        }

                        SectionTitle { text: qsTr("Playback") }
                        OptionRow {
                            label: qsTr("Default priority of new scenes")
                            hint: qsTr("0 to 10. When two scenes control the same channel, the higher priority wins.")
                            SpinBox {
                                from: 0; to: 10; editable: true
                                value: Core.settings.defaultPriority
                                onValueModified: Core.settings.defaultPriority = value
                            }
                        }
                        // live edit settings
                        OptionRow {
                            label: qsTr("Priority of the live edits")
                            hint: qsTr("What you set in Program wins over the scenes of this priority and below. 10: over every scene.")
                            SpinBox {
                                from: 0; to: 10; editable: true
                                value: Core.settings.liveEditPriority
                                onValueModified: Core.settings.liveEditPriority = value
                            }
                        }
                        OptionRow {
                            label: qsTr("When a scene starts, the live edits")
                            ComboBox {
                                Layout.preferredWidth: 300
                                readonly property var modes: ["channels", "keep", "all"]
                                model: [qsTr("Give the scene its channels back"), qsTr("Always stay"), qsTr("Are all reset")]
                                currentIndex: Math.max(0, modes.indexOf(Core.settings.liveEditRelease))
                                onActivated: (i) => Core.settings.liveEditRelease = modes[i]
                            }
                        }
                        OptionRow {
                            label: qsTr("Grand master acts on")
                            ComboBox {
                                Layout.preferredWidth: 200
                                model: [qsTr("Intensity channels"), qsTr("All channels")]
                                currentIndex: Core.settings.grandMasterIntensityOnly ? 0 : 1
                                onActivated: (i) => Core.settings.grandMasterIntensityOnly = (i === 0)
                            }
                        }

                        // the input Pulse listens to (one that was chosen and
                        // is not plugged in now stays in the list)
                        SectionTitle { text: qsTr("Audio") }
                        OptionRow {
                            label: qsTr("Audio input")
                            hint: qsTr("The microphone or line input of Pulse and of the audio matrices")
                            ComboBox {
                                Layout.preferredWidth: 300
                                readonly property var available: Core.drive.audioInputs()
                                readonly property string saved: Core.settings.audioInput
                                readonly property var inputs: saved.length > 0 && available.indexOf(saved) < 0 ? available.concat([saved]) : available
                                model: [qsTr("System default")].concat(inputs.map(n => available.indexOf(n) < 0 ? qsTr("%1 (not connected)").arg(n) : n))
                                currentIndex: saved.length > 0 ? inputs.indexOf(saved) + 1 : 0
                                onActivated: (i) => Core.settings.audioInput = (i === 0 ? "" : inputs[i - 1])
                            }
                        }

                        SectionTitle { text: qsTr("Recent shows") }
                        Repeater {
                            model: Core.settings.recentShows
                            delegate: Text {
                                required property string modelData
                                Layout.fillWidth: true
                                text: modelData
                                color: Theme.textDim
                                elide: Text.ElideMiddle
                                font.pixelSize: Theme.fontSmall
                            }
                        }
                        AppButton {
                            text: qsTr("Clear list")
                            flat: true
                            enabled: Core.settings.recentShows.length > 0
                            onClicked: Core.settings.clearRecentShows()
                        }
                    }
                }

                // right column: fixture library + about
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    spacing: 12

                    // the remote control: the SHOW on a phone or a tablet
                    Panel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: remoteColumn.implicitHeight + 24

                        ColumnLayout {
                            id: remoteColumn
                            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 14 }
                            spacing: 12

                            SectionTitle { text: qsTr("Remote control") }
                            Text {
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                                text: qsTr("Play the SHOW pages from a phone or a tablet on the same network (Wi-Fi): open the address below in its browser. Nothing to install.")
                                color: Theme.text
                            }
                            OptionRow {
                                label: qsTr("Remote control on")
                                AppSwitch { checked: Core.remote.enabled; onToggled: Core.remote.enabled = checked }
                            }
                            OptionRow {
                                label: qsTr("Port")
                                hint: qsTr("Change it if another program uses it")
                                SpinBox {
                                    from: 1024; to: 65535
                                    editable: true
                                    value: Core.remote.port
                                    textFromValue: (v) => String(v)
                                    valueFromText: (t) => parseInt(t)
                                    onValueModified: Core.remote.port = value
                                }
                            }
                            OptionRow {
                                label: qsTr("PIN")
                                hint: qsTr("Asked once on each device. Empty: anyone on the network can use it")
                                TextField {
                                    Layout.preferredWidth: 120
                                    color: Theme.text
                                    text: Core.remote.pin
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    placeholderText: qsTr("None")
                                    onEditingFinished: Core.remote.pin = text
                                }
                            }
                            Text {
                                Layout.fillWidth: true
                                visible: Core.remote.lastError.length > 0
                                wrapMode: Text.WordWrap
                                text: Core.remote.lastError
                                color: Theme.danger
                            }
                            // where to open it
                            Repeater {
                                model: Core.remote.urls
                                delegate: RowLayout {
                                    required property string modelData
                                    Layout.fillWidth: true
                                    spacing: 8
                                    Text {
                                        Layout.fillWidth: true
                                        text: parent.modelData
                                        color: Theme.accent
                                        font.family: Theme.monoFamily
                                        font.pixelSize: Theme.fontLarge
                                        elide: Text.ElideRight
                                    }
                                    AppButton {
                                        iconGlyph: Theme.iconCopy
                                        flat: true
                                        implicitHeight: 28
                                        ToolTip.text: qsTr("Copy"); ToolTip.visible: hovered
                                        onClicked: Core.copyToClipboard(parent.modelData)
                                    }
                                }
                            }
                            Text {
                                visible: Core.remote.running
                                text: qsTr("%n device(s) connected", "", Core.remote.clients)
                                color: Core.remote.clients > 0 ? Theme.success : Theme.textDim
                            }
                        }
                    }

                    Panel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: libColumn.implicitHeight + 24

                        ColumnLayout {
                            id: libColumn
                            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 14 }
                            spacing: 12

                            SectionTitle { text: qsTr("Fixture library") }
                            Text {
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                                text: qsTr("%1 fixture definitions available: the built-in library, the generic fixtures and your own.").arg(Core.library.definitionsCount)
                                color: Theme.text
                            }

                            // downloaded: when, and whether it has new fixtures since
                            // (an update keeps your own fixtures, and the ones
                            // it doesn't have any more)
                            OptionRow {
                                label: qsTr("Open Fixture Library")
                                hint: !Core.library.openFixtureLibraryInstalled
                                      ? qsTr("Thousands of extra fixtures from open-fixture-library.org (MIT license). Needs internet.")
                                      : qsTr("%n fixture(s), downloaded on %1.", "", Core.library.oflCount)
                                            .arg(Core.library.oflDownloaded.toLocaleDateString(Qt.locale(), Locale.ShortFormat))
                                        + " " + (Core.library.oflUpdateAvailable ? qsTr("There are new or improved fixtures: update it.")
                                                                                 : qsTr("Updating it keeps your own fixtures."))
                                AppButton {
                                    text: Core.library.downloading ? qsTr("Cancel")
                                          : Core.library.openFixtureLibraryInstalled ? qsTr("Update") : qsTr("Download")
                                    iconGlyph: Core.library.downloading ? Theme.iconClose
                                               : Core.library.openFixtureLibraryInstalled ? Theme.iconRefresh : Theme.iconDownload
                                    primary: !Core.library.downloading && (!Core.library.openFixtureLibraryInstalled || Core.library.oflUpdateAvailable)
                                    onClicked: Core.library.downloading ? Core.library.cancelDownload()
                                                                        : Core.library.downloadOpenFixtureLibrary()
                                }
                            }
                            ProgressBar {
                                Layout.fillWidth: true
                                visible: Core.library.downloading
                                value: Core.library.downloadProgress
                                indeterminate: Core.library.downloadProgress <= 0
                            }

                            OptionRow {
                                label: qsTr("Import fixture files")
                                hint: qsTr(".qxf definitions (QLC+ format), .gdtf files, or a .zip with them")
                                AppButton { text: qsTr("Import…"); iconGlyph: Theme.iconImport; onClicked: importDialog.open() }
                            }
                            OptionRow {
                                label: qsTr("GDTF fixtures")
                                hint: qsTr("The open format the manufacturers publish on GDTF-Share (free account): import .gdtf files, folders of them or the .zip of its bulk download. %n GDTF fixture(s) in the library.", "", Core.library.gdtfCount)
                                AppButton {
                                    text: qsTr("GDTF-Share")
                                    iconGlyph: Theme.iconLink
                                    flat: true
                                    ToolTip.text: "https://gdtf-share.com"
                                    ToolTip.visible: hovered
                                    onClicked: Qt.openUrlExternally("https://gdtf-share.com")
                                }
                                AppButton {
                                    text: Core.library.importing ? qsTr("Stop") : qsTr("Import GDTF…")
                                    iconGlyph: Core.library.importing ? Theme.iconClose : Theme.iconImport
                                    onClicked: Core.library.importing ? Core.library.cancelImport() : gdtfDialog.open()
                                }
                            }
                            ProgressBar {
                                Layout.fillWidth: true
                                visible: Core.library.importing
                                value: Core.library.importProgress
                                indeterminate: Core.library.importProgress <= 0
                            }
                            OptionRow {
                                label: qsTr("Import my QLC+ fixtures")
                                hint: qsTr("Copies the fixtures you created in QLC+")
                                AppButton { text: qsTr("Import"); onClicked: Core.library.importFromQlcPlus() }
                            }
                            OptionRow {
                                label: qsTr("User fixtures folder")
                                AppButton { text: qsTr("Open folder"); iconGlyph: Theme.iconFolder; onClicked: Core.library.openUserFolder() }
                            }

                            Text {
                                Layout.fillWidth: true
                                visible: Core.library.status.length > 0
                                wrapMode: Text.WordWrap
                                text: Core.library.status
                                color: Theme.accent
                            }
                            // the fixtures the last update of the Open Fixture Library brought
                            Text {
                                Layout.fillWidth: true
                                visible: Core.library.oflNewFixtures.length > 0 && !Core.library.downloading
                                wrapMode: Text.WordWrap
                                text: qsTr("New: %1").arg(Core.library.oflNewFixtures.slice(0, 8).join(", "))
                                      + (Core.library.oflNewFixtures.length > 8 ? " " + qsTr("…and %n more", "", Core.library.oflNewFixtures.length - 8) : "")
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSmall
                            }
                            // the GDTF files that couldn't be read, and why
                            Text {
                                Layout.fillWidth: true
                                visible: Core.library.importErrors.length > 0 && !Core.library.importing
                                wrapMode: Text.WordWrap
                                text: Core.library.importErrors.slice(0, 5).join("\n")
                                      + (Core.library.importErrors.length > 5 ? "\n" + qsTr("…and %n more", "", Core.library.importErrors.length - 5) : "")
                                color: Theme.warning
                                font.pixelSize: Theme.fontSmall
                            }
                            Text {
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                                text: qsTr("Libraries of other programs in closed or encrypted formats can't be imported. Most fixtures exist in the Open Fixture Library or on GDTF-Share; missing ones can be made on the Fixtures page.")
                                color: Theme.textFaint
                                font.pixelSize: Theme.fontSmall
                            }
                        }
                    }

                    // the manual and the first steps guide
                    Panel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: helpColumn.implicitHeight + 24

                        ColumnLayout {
                            id: helpColumn
                            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 14 }
                            spacing: 12

                            SectionTitle { text: qsTr("Help") }
                            OptionRow {
                                label: qsTr("Manual")
                                hint: qsTr("Every page of the program, with pictures. F12 or «?» open it on the page you are on")
                                AppButton { text: qsTr("Open the manual"); onClicked: page.manualWanted() }
                            }
                            OptionRow {
                                label: qsTr("First steps guide")
                                hint: qsTr("Five steps to light a stage, with a practice show")
                                AppButton { text: qsTr("Start the guide"); onClicked: page.guideWanted() }
                            }
                            OptionRow {
                                label: qsTr("Example shows")
                                hint: qsTr("Shows ready to open, in the ▾ menu next to Open: a small gig, a concert with moving heads, a venue with playbacks")
                            }
                        }
                    }

                    // the new versions, from the releases on GitHub
                    Panel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: updatesColumn.implicitHeight + 24

                        ColumnLayout {
                            id: updatesColumn
                            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 14 }
                            spacing: 12

                            readonly property int phase: Core.updates.state

                            SectionTitle { text: qsTr("Updates") }
                            OptionRow {
                                label: qsTr("Check for updates at startup")
                                hint: qsTr("Once a day, on GitHub: the program, and the new fixtures of the Open Fixture Library if you downloaded it. Nothing is downloaded or installed without asking you.")
                                AppSwitch { checked: Core.updates.autoCheck; onToggled: Core.updates.autoCheck = checked }
                            }
                            OptionRow {
                                label: qsTr("Include test versions")
                                hint: qsTr("Pre-releases: the new features earlier, less tested")
                                AppSwitch { checked: Core.updates.prereleases; onToggled: Core.updates.prereleases = checked }
                            }
                            Text {
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                                text: {
                                    const v = Core.updates.latestVersion
                                    switch (updatesColumn.phase) {
                                    case UpdateChecker.Checking:
                                        return qsTr("Checking for updates…")
                                    case UpdateChecker.UpToDate:
                                        return qsTr("You have the latest version (%1).").arg(Core.updates.currentVersion)
                                    case UpdateChecker.Available:
                                        return Core.updates.installable ? qsTr("Version %1 is out.").arg(v)
                                                                        : qsTr("Version %1 is out. Its installer has no SHA-256 digest to check it: download it from its page.").arg(v)
                                    case UpdateChecker.Downloading:
                                        return qsTr("Downloading version %1… %2 %").arg(v).arg(Math.round(Core.updates.progress * 100))
                                    case UpdateChecker.Ready:
                                        return Core.updates.installOnExit ? qsTr("Version %1 will be installed when the program closes.").arg(v)
                                                                          : qsTr("Version %1 is downloaded and checked, ready to install.").arg(v)
                                    case UpdateChecker.Failed:
                                        return Core.updates.error
                                    default:
                                        return Core.updates.lastCheck.length > 0 ? qsTr("Version %1. Last check: %2.").arg(Core.updates.currentVersion).arg(Core.updates.lastCheck)
                                                                                 : qsTr("Version %1.").arg(Core.updates.currentVersion)
                                    }
                                }
                                color: updatesColumn.phase === UpdateChecker.Failed ? Theme.warning : Theme.text
                            }
                            Flow {
                                Layout.fillWidth: true
                                spacing: 8
                                AppButton {
                                    visible: updatesColumn.phase !== UpdateChecker.Downloading && updatesColumn.phase !== UpdateChecker.Ready
                                    enabled: updatesColumn.phase !== UpdateChecker.Checking
                                    text: qsTr("Check for updates")
                                    iconGlyph: Theme.iconRefresh
                                    onClicked: {
                                        Core.updates.check(true)
                                        if (Core.library.openFixtureLibraryInstalled)
                                            Core.library.checkOpenFixtureLibrary(true)
                                    }
                                }
                                AppButton {
                                    visible: Core.updates.latestVersion.length > 0
                                    text: qsTr("What's new")
                                    onClicked: optionsNotes.open()
                                }
                                AppButton {
                                    visible: (updatesColumn.phase === UpdateChecker.Available || updatesColumn.phase === UpdateChecker.Failed) && Core.updates.installable
                                    text: qsTr("Download and install")
                                    iconGlyph: Theme.iconDownload
                                    primary: true
                                    onClicked: Core.updates.download()
                                }
                                AppButton {
                                    visible: updatesColumn.phase === UpdateChecker.Ready
                                    text: qsTr("Install now")
                                    primary: true
                                    onClicked: page.installUpdate()
                                }
                                AppButton {
                                    visible: updatesColumn.phase === UpdateChecker.Ready
                                    text: Core.updates.installOnExit ? qsTr("Not when closing") : qsTr("When closing")
                                    onClicked: Core.updates.setInstallOnExit(!Core.updates.installOnExit)
                                }
                                AppButton {
                                    visible: Core.updates.pageUrl.length > 0
                                    text: qsTr("Open on GitHub")
                                    iconGlyph: Theme.iconLink
                                    flat: true
                                    onClicked: Core.updates.openPage()
                                }
                            }
                        }
                    }

                    Panel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: aboutColumn.implicitHeight + 24

                        ColumnLayout {
                            id: aboutColumn
                            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 14 }
                            spacing: 8

                            SectionTitle { text: qsTr("About") }
                            RowLayout {
                                spacing: 12
                                Image {
                                    Layout.preferredWidth: 48
                                    Layout.preferredHeight: 48
                                    source: "qrc:/icons/dtc-128.png"
                                    sourceSize: Qt.size(96, 96)
                                    smooth: true
                                    mipmap: true
                                }
                                Text {
                                    text: Core.appName + " " + Core.appVersion
                                    color: Theme.text
                                    font.pixelSize: Theme.fontLarge
                                    font.weight: Font.Bold
                                }
                            }
                            Text {
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                                text: qsTr("Free and open source software, under the Apache 2.0 license. Based on Q Light Controller Plus by Massimo Callegari, Heikki Junnila and the QLC+ contributors. Extra fixtures from the Open Fixture Library.")
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSmall
                            }
                        }
                    }
                }
            }
        }
    }

    LockDialog { id: optionsLock }
    ReleaseNotes { id: optionsNotes }
}
