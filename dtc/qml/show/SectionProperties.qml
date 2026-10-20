/*
  DMX Total Control
  SectionProperties.qml

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

/** Edit mode of the SHOW screen: the selected section, its selected cell, and the scenes to drag */
Flickable {
    id: props

    property var surface: null
    property var section: ({})
    property int cell: -1

    readonly property string sid: section.id || ""
    readonly property string type: section.type || ""
    readonly property bool isTable: type === "buttons" || type === "faders" || type === "dials"
    // faders and dials drive the same things
    readonly property bool levelType: type === "faders" || type === "dials"
    // the icons a button can have (FontAwesome); the first one is none
    readonly property var icons: ["", "", "", "", "", "", "", "", "",
                                  "", "", "", "", "", "", "", "",
                                  "", "", "", "", "", "", "", ""]
    function fileName(path) { return decodeURIComponent(String(path).split(/[\\/]/).pop()) }
    readonly property var item: isTable && cell >= 0 ? (section.items || {})[String(cell)] : undefined

    readonly property var swatches: ["#3d8bfd", "#30a46c", "#e5484d", "#f5a524", "#ae3ec9",
                                     "#17a2b8", "#d6336c", "#f76707", "#e7e9ef", "#5d6475"]
    readonly property var actions: ["scene", "next", "previous", "release", "blackout", "stopAll", "tap", "beatgo", "palette",
                                    "cuelistGo", "cuelistBack", "cuelistGoTo", "cuelistRelease"]
    readonly property var actionNames: [qsTr("Play a scene"), qsTr("Next scene of a group"),
                                        qsTr("Previous scene of a group"), qsTr("Release a group"),
                                        qsTr("Blackout"), qsTr("Stop all"), qsTr("Tap tempo"), qsTr("BeatGO (next division)"),
                                        qsTr("A palette"), qsTr("Cuelist GO"), qsTr("Cuelist back"), qsTr("Go to a cue…"),
                                        qsTr("Release a cuelist")]
    // the cuelist actions are playback targets on a function (stored as "map"
    // buttons, with no slot: without a cuelist yet they do nothing)
    readonly property var cuelistTargets: ({ cuelistGo: "playbackGo", cuelistBack: "playbackBack",
                                             cuelistGoTo: "playbackGoTo", cuelistRelease: "playbackRelease" })
    /** The action of a button as the list shows it ("cuelistGo"... for those map buttons) */
    function actionOf(item) {
        if (!item)
            return ""
        if (item.action === "map" && item.map &&
            ((item.map.functionId !== undefined && item.map.functionId >= 0) || item.map.slot === -1))
            for (let k in cuelistTargets)
                if (cuelistTargets[k] === item.map.target)
                    return k
        return item.action || ""
    }
    readonly property string itemAction: actionOf(item)
    readonly property bool cuelistAction: cuelistTargets[itemAction] !== undefined
    function setAction(action) {
        if (cuelistTargets[action] === undefined) {
            setItemKey("action", action)
            return
        }
        // the cuelist it had, else the first one
        let current = item && item.map ? item.map : ({})
        let fid = current.functionId !== undefined && current.functionId >= 0 ? current.functionId
                                                                             : (cuelistList.length ? cuelistList[0].id : -1)
        // (slot -1: never a playback of the current page, even before a cuelist is chosen;
        // a cue is bound to that cue by its uid: cues recorded or deleted before it don't move it)
        const cue = current.functionId === fid ? (current.cue || 0) : 0
        const cueUid = current.functionId === fid && current.cueUid ? current.cueUid : cueUidAt(fid, cue)
        setItem({ action: "map", map: { target: cuelistTargets[action], functionId: fid, cue: cue, cueUid: cueUid, slot: -1 } })
    }
    function setMapKeys(values) {
        let m = Object.assign({}, item && item.map ? item.map : ({}))
        for (let k in values)
            m[k] = values[k]
        setItem({ map: m })
    }
    function setMapKey(key, value) {
        let values = {}
        values[key] = value
        setMapKeys(values)
    }
    /** The uid of a cue of a cuelist (0: no such cue) */
    function cueUidAt(fid, index) {
        if (fid === undefined || fid < 0)
            return 0
        const cue = Core.cuelists.cues(fid)[index]
        return cue ? cue.uid : 0
    }
    readonly property var cuelistList: functionList.filter(f => f.type === "Cuelist")
    // the cues of the cuelist of a "go to a cue" button (its edits show)
    property int cuesRevision: 0
    Connections {
        target: Core.cuelists
        function onEdited() { props.cuesRevision++ }
    }
    readonly property var cueList: {
        props.cuesRevision
        if (!cuelistAction || !item.map || item.map.functionId < 0)
            return []
        return Core.cuelists.cues(item.map.functionId)
    }
    readonly property var cueChoices: cueList.map(c => c.number + (c.name ? " · " + c.name : ""))
    /** The cue a "go to a cue" button plays now (-1: none, or deleted) */
    readonly property int cueChosen: {
        props.cuesRevision
        if (itemAction !== "cuelistGoTo" || !item.map)
            return -1
        const index = Core.mapping.cueIndexOf(item.map)
        return index < cueList.length ? index : -1
    }
    readonly property var targets: ["scene", "sceneDimmer", "group", "grandMaster", "mixerDimmer", "moveSpeed", "sceneSpeed", "bpm"]
    readonly property var targetNames: [qsTr("Play a scene (its intensity)"), qsTr("Dimmer of a scene (does not play it)"),
                                        qsTr("Master of a group"), qsTr("Grand master"), qsTr("Dimmer of fixtures"),
                                        qsTr("Movement speed of moving heads"), qsTr("Speed of a scene"), qsTr("BPM")]
    readonly property var fixtureGroupChoices: [{ id: -1, name: qsTr("All the moving heads") }].concat(Core.patch.groups)

    // the elements of a fader
    function faderTargets() { return item ? Core.surface.faderTargets(item) : [] }
    function setTargets(list) { setItem({ targets: list, target: list.length ? list[0].target : "grandMaster" }) }
    function setTarget(index, values) {
        let list = faderTargets()
        let t = Object.assign({}, list[index])
        if (values.target !== undefined && values.target !== t.target) {
            // a new kind of element: start from its first choice
            t = { target: values.target }
            if (values.target === "scene" || values.target === "sceneDimmer" || values.target === "sceneSpeed")
                t.functionId = functionList.length ? functionList[0].id : -1
            else if (values.target === "group")
                t.zoneId = zoneList.length ? zoneList[0].id : -1
            else if (values.target === "mixerDimmer")
                t.mixerId = -1
            else if (values.target === "moveSpeed")
                t.groupId = -1
        } else {
            for (let k in values)
                t[k] = values[k]
        }
        list[index] = t
        setTargets(list)
    }
    function addTarget() {
        let list = faderTargets()
        list.push(zoneList.length ? { target: "group", zoneId: zoneList[0].id } : { target: "grandMaster" })
        setTargets(list)
    }
    function removeTarget(index) {
        let list = faderTargets()
        list.splice(index, 1)
        setTargets(list)
    }

    readonly property var functionList: Core.functions.slice().sort((a, b) => a.name.localeCompare(b.name))
    readonly property var zoneList: {
        let list = []
        for (let i = 0; i < Core.zones.count; i++) {
            let z = Core.zones.zone(i)
            list.push({ id: z.id, name: z.name })
        }
        return list
    }

    function setSection(values) { Core.surface.updateSection(surface.currentPage, sid, values) }
    function setItem(values) { Core.surface.setItem(surface.currentPage, sid, cell, values) }
    function setItemKey(key, value) { let v = {}; v[key] = value; setItem(v) }

    // the −/+ buttons of a SpinBox take 80 px: below this width its biggest value is cut ("100 %" shows "ı00 %")
    FontMetrics { id: spinFont; font.family: Theme.fontFamily; font.pixelSize: Theme.fontNormal }
    function spinMinWidth(spin) { return spin.leftPadding + spin.rightPadding + spinFont.advanceWidth(spin.textFromValue(spin.to, spin.locale)) + 4 }

    contentHeight: column.implicitHeight + 24
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical: ScrollBar {}

    component Caption: Text {
        Layout.topMargin: 6
        color: Theme.textDim
        font.pixelSize: Theme.fontSmall
        font.weight: Font.DemiBold
    }

    ColumnLayout {
        id: column
        x: 12
        y: 12
        width: props.width - 24
        spacing: 6

        Text {
            visible: props.sid === ""
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Select a section to set it up. Split it in half with the buttons in its title bar.")
            color: Theme.textFaint
        }

        // ------------------------------------------------------------ section
        ColumnLayout {
            Layout.fillWidth: true
            visible: props.sid !== ""
            spacing: 6

            Text {
                text: qsTr("Section")
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
            }
            Caption { text: qsTr("Shows") }
            ComboBox {
                Layout.fillWidth: true
                model: props.surface ? props.surface.sectionTypes.map(t => t.name) : []
                currentIndex: props.surface ? Math.max(0, props.surface.sectionTypes.findIndex(t => t.type === props.type)) : 0
                onActivated: (i) => props.setSection({ type: props.surface.sectionTypes[i].type })
            }
            Caption { text: qsTr("Title") }
            TextField {
                Layout.fillWidth: true
                color: Theme.text
                text: props.section.title || ""
                placeholderText: qsTr("No title")
                onEditingFinished: if (text !== (props.section.title || "")) props.setSection({ title: text })
            }

            RowLayout {
                visible: props.isTable || props.type === "playbacks"
                Layout.fillWidth: true
                Caption { text: props.type === "playbacks" ? qsTr("Playbacks") : qsTr("Columns") }
                SpinBox {
                    from: 1; to: props.type === "playbacks" ? 20 : 32; editable: true
                    value: props.section.cols || 1
                    onValueModified: props.setSection({ cols: value })
                }
            }

            // playbacks: which ones, and what each strip shows
            Caption { visible: props.type === "playbacks"; text: qsTr("Playback page") }
            ComboBox {
                Layout.fillWidth: true
                visible: props.type === "playbacks"
                readonly property int pageShown: props.section.playbackPage !== undefined ? props.section.playbackPage : -1
                model: [qsTr("Follow the current page")].concat(Core.playbacks.pageNames)
                // (a page that was deleted: its strips are empty)
                displayText: pageShown >= Core.playbacks.pageCount ? qsTr("Page %1 (deleted)").arg(pageShown + 1) : currentText
                currentIndex: Math.min(pageShown + 1, Core.playbacks.pageCount)
                onModelChanged: currentIndex = Qt.binding(() => Math.min(pageShown + 1, Core.playbacks.pageCount))
                onActivated: (i) => props.setSection({ playbackPage: i - 1 })
            }
            RowLayout {
                visible: props.type === "playbacks"
                Layout.fillWidth: true
                Caption { text: qsTr("First playback") }
                SpinBox {
                    from: 1; to: 20; editable: true
                    value: (props.section.first || 0) + 1
                    onValueModified: props.setSection({ first: value - 1 })
                }
            }
            GridLayout {
                visible: props.type === "playbacks"
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 8
                rowSpacing: 4
                AppSwitch { checked: props.section.showBack !== false; onToggled: props.setSection({ showBack: checked }) }
                Text { Layout.fillWidth: true; text: qsTr("Second button (Pause / Back)"); color: Theme.text; wrapMode: Text.WordWrap }
                AppSwitch { checked: props.section.showFlash !== false; onToggled: props.setSection({ showFlash: checked }) }
                Text { Layout.fillWidth: true; text: qsTr("Third button (Flash)"); color: Theme.text; wrapMode: Text.WordWrap }
                AppSwitch { checked: props.section.showCue !== false; onToggled: props.setSection({ showCue: checked }) }
                Text { Layout.fillWidth: true; text: qsTr("The cue it plays and the next one"); color: Theme.text; wrapMode: Text.WordWrap }
            }
            Text {
                Layout.fillWidth: true
                visible: props.type === "playbacks"
                wrapMode: Text.WordWrap
                text: qsTr("The playbacks of the Playbacks page, with their GO, buttons and fader. Drop a scene on one to put it there.")
                color: Theme.textFaint
                font.pixelSize: Theme.fontSmall
            }
            RowLayout {
                visible: props.type === "buttons" || props.type === "dials"
                Layout.fillWidth: true
                Caption { text: qsTr("Rows") }
                SpinBox {
                    from: 1; to: 32; editable: true
                    value: props.section.rows || 1
                    onValueModified: props.setSection({ rows: value })
                }
            }
            // a whole group in the buttons, in one go
            Caption { visible: props.type === "buttons" && props.zoneList.length > 0; text: qsTr("Fill with the scenes of a group") }
            ComboBox {
                Layout.fillWidth: true
                visible: props.type === "buttons" && props.zoneList.length > 0
                textRole: "name"
                model: props.zoneList
                displayText: qsTr("Choose a group…")
                currentIndex: -1
                onActivated: (i) => { Core.surface.fillWithGroup(props.surface.currentPage, props.sid, props.zoneList[i].id); currentIndex = -1 }
            }

            Caption { visible: props.type === "group"; text: qsTr("Group") }
            ComboBox {
                Layout.fillWidth: true
                visible: props.type === "group"
                textRole: "name"
                model: props.zoneList
                displayText: currentIndex < 0 ? qsTr("Choose…") : currentText
                currentIndex: props.zoneList.findIndex(z => z.id === props.section.zoneId)
                onActivated: (i) => props.setSection({ zoneId: props.zoneList[i].id })
            }

            // colour wheel and XY grid: on which fixtures
            Caption { visible: props.type === "colour" || props.type === "xy"; text: qsTr("Fixtures") }
            ComboBox {
                Layout.fillWidth: true
                visible: props.type === "colour" || props.type === "xy"
                readonly property var choices: [{ id: -1, name: props.type === "xy" ? qsTr("All the moving heads") : qsTr("All the fixtures with colour") }]
                                                .concat(Core.patch.groups)
                textRole: "name"
                model: choices
                currentIndex: Math.max(0, choices.findIndex(g => g.id === (props.section.groupId !== undefined ? props.section.groupId : -1)))
                onActivated: (i) => props.setSection({ groupId: choices[i].id })
            }
            Text {
                Layout.fillWidth: true
                visible: props.type === "colour" || props.type === "xy"
                wrapMode: Text.WordWrap
                text: qsTr("Like a live edit of the programmer: it goes over the scenes until you release it.")
                color: Theme.textFaint
                font.pixelSize: Theme.fontSmall
            }

            // image: a file, fitted or filling the section
            Caption { visible: props.type === "image"; text: qsTr("Image") }
            AppButton {
                Layout.fillWidth: true
                visible: props.type === "image"
                text: props.section.image ? props.fileName(props.section.image) : qsTr("Choose an image…")
                iconGlyph: Theme.iconFolder
                onClicked: { imageDialog.forSection = true; imageDialog.open() }
            }
            ComboBox {
                Layout.fillWidth: true
                visible: props.type === "image"
                model: [qsTr("Whole image"), qsTr("Fill the section")]
                currentIndex: props.section.fit === "fill" ? 1 : 0
                onActivated: (i) => props.setSection({ fit: i === 1 ? "fill" : "fit" })
            }

            RowLayout {
                visible: props.type === "clock"
                Caption { text: qsTr("Seconds") }
                AppSwitch { checked: props.section.seconds !== false; onToggled: props.setSection({ seconds: checked }) }
                Caption { text: qsTr("Date") }
                AppSwitch { checked: props.section.date !== false; onToggled: props.setSection({ date: checked }) }
            }

            Caption { visible: props.type === "timer"; text: qsTr("Counts") }
            ComboBox {
                Layout.fillWidth: true
                visible: props.type === "timer"
                model: [qsTr("Up (stopwatch)"), qsTr("Down (countdown)")]
                currentIndex: props.section.mode === "countdown" ? 1 : 0
                onActivated: (i) => props.setSection({ mode: i === 1 ? "countdown" : "stopwatch" })
            }
            RowLayout {
                visible: props.type === "timer" && props.section.mode === "countdown"
                Caption { text: qsTr("Minutes") }
                SpinBox {
                    from: 1; to: 600; editable: true
                    value: props.section.minutes || 5
                    onValueModified: props.setSection({ minutes: value })
                }
            }

            Caption { visible: props.type === "notes"; text: qsTr("Text") }
            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                visible: props.type === "notes"
                TextArea {
                    color: Theme.text
                    wrapMode: TextArea.Wrap
                    text: props.section.text || ""
                    onEditingFinished: if (text !== (props.section.text || "")) props.setSection({ text: text })
                    background: Rectangle { color: Theme.background; radius: Theme.radiusSmall; border.color: Theme.border }
                }
            }
            RowLayout {
                visible: props.type === "notes"
                Caption { text: qsTr("Size") }
                SpinBox {
                    from: 10; to: 96; editable: true
                    value: props.section.fontSize || 20
                    onValueModified: props.setSection({ fontSize: value })
                }
            }

            Caption { visible: props.type === "monitor"; text: qsTr("Universe") }
            ComboBox {
                Layout.fillWidth: true
                visible: props.type === "monitor"
                model: Core.io.universes.map(u => (u.index + 1) + " — " + u.name)
                currentIndex: props.section.universe || 0
                onActivated: (i) => props.setSection({ universe: i })
            }

            RowLayout {
                Layout.topMargin: 8
                Layout.fillWidth: true
                spacing: 4
                AppButton {
                    Layout.fillWidth: true
                    iconGlyph: Theme.iconArrowsH
                    text: qsTr("Side by side")
                    onClicked: props.surface.split(props.sid, "h")
                }
                AppButton {
                    Layout.fillWidth: true
                    iconGlyph: Theme.iconArrowsV
                    text: qsTr("Top and bottom")
                    onClicked: props.surface.split(props.sid, "v")
                }
            }
            AppButton {
                Layout.fillWidth: true
                text: qsTr("Remove section")
                iconGlyph: Theme.iconTrash
                accentColor: Theme.danger
                onClicked: props.surface.removeSection(props.sid)
            }
        }

        // ------------------------------------------------------------ cell
        Rectangle { visible: props.isTable && props.cell >= 0; Layout.fillWidth: true; Layout.topMargin: 10; height: 1; color: Theme.border }

        ColumnLayout {
            Layout.fillWidth: true
            visible: props.isTable && props.cell >= 0
            spacing: 6

            Text {
                text: props.type === "faders" ? qsTr("Fader %1").arg(props.cell + 1) : props.type === "dials" ? qsTr("Dial %1").arg(props.cell + 1) : qsTr("Button %1").arg(props.cell + 1)
                color: Theme.text
                font.pixelSize: Theme.fontLarge
                font.weight: Font.Bold
            }
            AppButton {
                visible: props.item === undefined
                Layout.fillWidth: true
                text: props.type === "faders" ? qsTr("Add a fader here") : props.type === "dials" ? qsTr("Add a dial here") : qsTr("Add a button here")
                iconGlyph: Theme.iconPlus
                primary: true
                onClicked: props.setItem({})
            }

            ColumnLayout {
                Layout.fillWidth: true
                visible: props.item !== undefined
                spacing: 6

                Caption { text: qsTr("Label") }
                TextField {
                    Layout.fillWidth: true
                    color: Theme.text
                    text: props.item ? (props.item.label || "") : ""
                    placeholderText: qsTr("Automatic")
                    onEditingFinished: if (props.item && text !== (props.item.label || "")) props.setItemKey("label", text)
                }

                Caption { visible: props.type === "buttons"; text: qsTr("Action") }
                ComboBox {
                    Layout.fillWidth: true
                    visible: props.type === "buttons"
                    model: props.actionNames
                    displayText: props.itemAction === "map" ? qsTr("A control on another page") : currentText
                    currentIndex: Math.max(0, props.actions.indexOf(props.itemAction))
                    onActivated: (i) => props.setAction(props.actions[i])
                }
                // GO, back, go to a cue or release of a cuelist: which one, and the cue
                Caption { visible: props.type === "buttons" && props.cuelistAction; text: qsTr("Cuelist") }
                ComboBox {
                    Layout.fillWidth: true
                    visible: props.type === "buttons" && props.cuelistAction
                    enabled: props.cuelistList.length > 0
                    textRole: "name"
                    model: props.cuelistList
                    displayText: props.cuelistList.length === 0 ? qsTr("No cuelists yet") : currentIndex < 0 ? qsTr("Choose…") : currentText
                    currentIndex: props.cuelistAction ? props.cuelistList.findIndex(f => f.id === props.item.map.functionId) : -1
                    // (another cuelist: its first cue)
                    onActivated: (i) => {
                        const fid = props.cuelistList[i].id
                        if (fid !== props.item.map.functionId)
                            props.setMapKeys({ functionId: fid, cue: 0, cueUid: props.cueUidAt(fid, 0) })
                    }
                }
                Caption { visible: props.type === "buttons" && props.itemAction === "cuelistGoTo"; text: qsTr("Cue") }
                ComboBox {
                    Layout.fillWidth: true
                    visible: props.type === "buttons" && props.itemAction === "cuelistGoTo"
                    enabled: props.cueChoices.length > 0
                    model: props.cueChoices
                    displayText: props.cueChoices.length === 0 ? qsTr("No cues") : currentIndex < 0 ? qsTr("Choose…") : currentText
                    currentIndex: props.cueChosen
                    onModelChanged: currentIndex = Qt.binding(() => props.cueChosen)
                    // (that cue, wherever later edits put it)
                    onActivated: (i) => props.setMapKeys({ cue: i, cueUid: props.cueList[i].uid })
                }
                // a control brought from another page (Alt+click)
                Text {
                    Layout.fillWidth: true
                    visible: props.type === "buttons" && props.itemAction === "map"
                    text: qsTr("Does: %1").arg(Core.mapping.describeTarget(props.item && props.item.map ? props.item.map : ({})))
                    color: Theme.text
                    wrapMode: Text.WordWrap
                }
                // a fader drives one or more things at once
                Caption { visible: props.levelType; text: qsTr("Controls") }
                Repeater {
                    model: props.levelType && props.item ? props.faderTargets() : []
                    delegate: Rectangle {
                        id: targetBox
                        required property var modelData
                        required property int index
                        Layout.fillWidth: true
                        implicitHeight: targetColumn.implicitHeight + 12
                        radius: Theme.radiusSmall
                        color: Theme.surfaceRaised
                        border.color: Theme.border

                        ColumnLayout {
                            id: targetColumn
                            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 6 }
                            spacing: 4
                            RowLayout {
                                Layout.fillWidth: true
                                ComboBox {
                                    Layout.fillWidth: true
                                    visible: targetBox.modelData.target !== "map"
                                    model: props.targetNames
                                    currentIndex: Math.max(0, props.targets.indexOf(targetBox.modelData.target))
                                    onActivated: (i) => props.setTarget(targetBox.index, { target: props.targets[i] })
                                }
                                // a control of another page (Alt+click)
                                Text {
                                    Layout.fillWidth: true
                                    visible: targetBox.modelData.target === "map"
                                    text: Core.mapping.describeTarget(targetBox.modelData.map || ({}))
                                    color: Theme.text
                                    wrapMode: Text.WordWrap
                                }
                                AppButton {
                                    implicitWidth: 30
                                    implicitHeight: 30
                                    flat: true
                                    iconGlyph: Theme.iconTrash
                                    visible: props.faderTargets().length > 1
                                    ToolTip.text: qsTr("Remove this element")
                                    ToolTip.visible: hovered
                                    onClicked: props.removeTarget(targetBox.index)
                                }
                            }
                            ComboBox {
                                Layout.fillWidth: true
                                visible: ["scene", "sceneDimmer", "sceneSpeed"].indexOf(targetBox.modelData.target) >= 0
                                textRole: "name"
                                model: props.functionList
                                displayText: currentIndex < 0 ? qsTr("Choose a scene…") : currentText
                                currentIndex: props.functionList.findIndex(f => f.id === targetBox.modelData.functionId)
                                onActivated: (i) => props.setTarget(targetBox.index, { functionId: props.functionList[i].id })
                            }
                            ComboBox {
                                Layout.fillWidth: true
                                visible: targetBox.modelData.target === "group"
                                textRole: "name"
                                model: props.zoneList
                                displayText: currentIndex < 0 ? qsTr("Choose a group…") : currentText
                                currentIndex: props.zoneList.findIndex(z => z.id === targetBox.modelData.zoneId)
                                onActivated: (i) => props.setTarget(targetBox.index, { zoneId: props.zoneList[i].id })
                            }
                            ComboBox {
                                Layout.fillWidth: true
                                visible: targetBox.modelData.target === "mixerDimmer"
                                textRole: "name"
                                model: Core.mixer.mixers
                                currentIndex: Math.max(0, Core.mixer.mixers.findIndex(m => m.id === (targetBox.modelData.mixerId !== undefined ? targetBox.modelData.mixerId : -1)))
                                onActivated: (i) => props.setTarget(targetBox.index, { mixerId: Core.mixer.mixers[i].id })
                            }
                            ComboBox {
                                Layout.fillWidth: true
                                visible: targetBox.modelData.target === "moveSpeed"
                                textRole: "name"
                                model: props.fixtureGroupChoices
                                currentIndex: Math.max(0, props.fixtureGroupChoices.findIndex(g => g.id === (targetBox.modelData.groupId !== undefined ? targetBox.modelData.groupId : -1)))
                                onActivated: (i) => props.setTarget(targetBox.index, { groupId: props.fixtureGroupChoices[i].id })
                            }
                        }
                    }
                }
                AppButton {
                    Layout.fillWidth: true
                    visible: props.levelType
                    text: qsTr("Add another element")
                    iconGlyph: Theme.iconPlus
                    flat: true
                    onClicked: props.addTarget()
                }
                // the travel covers only a part of the targets (as the mappings: "min" above "max" inverts it)
                Caption { visible: props.levelType; text: qsTr("Range") }
                GridLayout {
                    visible: props.levelType
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: 8
                    rowSpacing: 4
                    Text { text: qsTr("From"); color: Theme.textDim }
                    SpinBox {
                        id: minSpin
                        Layout.fillWidth: true
                        Layout.minimumWidth: props.spinMinWidth(minSpin)
                        from: 0; to: 100; editable: true
                        value: props.item && props.item.min !== undefined ? props.item.min : 0
                        textFromValue: (v) => v + " %"
                        valueFromText: (t) => { const v = parseInt(t); return isNaN(v) ? minSpin.value : v }
                        validator: RegularExpressionValidator { regularExpression: /[0-9]{1,3}\s*%?/ }
                        onValueModified: props.setItemKey("min", value)
                    }
                    Text { text: qsTr("to"); color: Theme.textDim }
                    SpinBox {
                        id: maxSpin
                        Layout.fillWidth: true
                        Layout.minimumWidth: props.spinMinWidth(maxSpin)
                        from: 0; to: 100; editable: true
                        value: props.item && props.item.max !== undefined ? props.item.max : 100
                        textFromValue: (v) => v + " %"
                        valueFromText: (t) => { const v = parseInt(t); return isNaN(v) ? maxSpin.value : v }
                        validator: RegularExpressionValidator { regularExpression: /[0-9]{1,3}\s*%?/ }
                        onValueModified: props.setItemKey("max", value)
                    }
                }
                RowLayout {
                    visible: props.levelType
                    AppSwitch {
                        checked: !!(props.item && props.item.invert)
                        onToggled: props.setItemKey("invert", checked)
                    }
                    Text {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        text: qsTr("Invert")
                        color: Theme.text
                    }
                }
                Text {
                    Layout.fillWidth: true
                    visible: props.levelType
                    text: qsTr("The travel covers only this part of what it drives; with From above to, it works upside down")
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                    wrapMode: Text.WordWrap
                }
                RowLayout {
                    visible: props.type === "faders"
                    AppSwitch {
                        checked: props.item !== undefined && props.item.flash !== false
                        onToggled: props.setItemKey("flash", checked)
                    }
                    Text {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        text: qsTr("Flash button: full while pressed")
                        color: Theme.text
                    }
                }
                // a dial, or an incremental wheel that turns forever
                RowLayout {
                    visible: props.type === "dials"
                    AppSwitch {
                        checked: !!(props.item && props.item.encoder)
                        onToggled: props.setItemKey("encoder", checked)
                    }
                    Text {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        text: qsTr("Incremental wheel: turns forever, each notch adds or removes a step")
                        color: Theme.text
                    }
                }
                RowLayout {
                    visible: props.type === "dials" && !!(props.item && props.item.encoder)
                    Caption { text: qsTr("Step") }
                    SpinBox {
                        from: 1; to: 25
                        value: props.item ? (props.item.step || 2) : 2
                        textFromValue: (v) => v + " %"
                        onValueModified: props.setItemKey("step", value)
                    }
                }

                readonly property bool needsScene: props.item !== undefined &&
                    props.type === "buttons" && props.item.action === "scene"
                Caption { visible: parent.needsScene; text: qsTr("Scene") }
                ComboBox {
                    Layout.fillWidth: true
                    visible: parent.needsScene
                    textRole: "name"
                    model: props.functionList
                    displayText: currentIndex < 0 ? qsTr("Choose…") : currentText
                    currentIndex: props.item ? props.functionList.findIndex(f => f.id === props.item.functionId) : -1
                    onActivated: (i) => props.setItemKey("functionId", props.functionList[i].id)
                }

                // a palette on its fixtures or a fixture group
                readonly property bool needsPalette: props.item !== undefined &&
                    props.type === "buttons" && props.item.action === "palette"
                readonly property var paletteList: Core.palettes.palettes.map(p => ({
                    id: p.id, color: p.color,
                    name: ({ intensity: "INT", position: "POS", colour: "COL", beam: "BEAM" })[p.type] + " " + p.number + " · " + p.name }))
                readonly property var paletteGroups: [{ id: -1, name: qsTr("Its own fixtures") }].concat(Core.patch.groups)
                Caption { visible: parent.needsPalette; text: qsTr("Palette") }
                ComboBox {
                    Layout.fillWidth: true
                    visible: parent.needsPalette
                    textRole: "name"
                    model: parent.paletteList
                    displayText: currentIndex < 0 ? qsTr("Choose…") : currentText
                    currentIndex: props.item ? parent.paletteList.findIndex(p => p.id === props.item.paletteId) : -1
                    onActivated: (i) => {
                        let palette = parent.paletteList[i]
                        let values = { paletteId: palette.id }
                        if (palette.color.length > 0)
                            values.color = palette.color
                        props.setItem(values)
                    }
                }
                Caption { visible: parent.needsPalette; text: qsTr("On the fixtures") }
                ComboBox {
                    Layout.fillWidth: true
                    visible: parent.needsPalette
                    textRole: "name"
                    model: parent.paletteGroups
                    currentIndex: props.item ? Math.max(0, parent.paletteGroups.findIndex(g => g.id === (props.item.groupId !== undefined ? props.item.groupId : -1))) : 0
                    onActivated: (i) => props.setItemKey("groupId", parent.paletteGroups[i].id)
                }
                Text {
                    Layout.fillWidth: true
                    visible: parent.needsPalette
                    text: qsTr("Like a live edit of the programmer: it goes over the scenes until you press it again.")
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                    wrapMode: Text.WordWrap
                }

                readonly property bool hasMode: props.item !== undefined && props.type === "buttons" &&
                                                ["scene", "blackout", "palette"].indexOf(props.item.action) >= 0
                Caption { visible: parent.hasMode; text: qsTr("Behaviour") }
                ComboBox {
                    Layout.fillWidth: true
                    visible: parent.hasMode
                    model: [qsTr("Toggle: on / off"), qsTr("Flash: while pressed")]
                    currentIndex: props.item && props.item.mode === "flash" ? 1 : 0
                    onActivated: (i) => props.setItemKey("mode", i === 1 ? "flash" : "toggle")
                }

                readonly property bool needsGroup: props.item !== undefined &&
                    props.type === "buttons" && ["next", "previous", "release"].indexOf(props.item.action) >= 0
                readonly property var groupChoices: props.type === "buttons"
                    ? [{ id: -1, name: qsTr("All groups") }].concat(props.zoneList) : props.zoneList
                Caption { visible: parent.needsGroup; text: qsTr("Group") }
                ComboBox {
                    Layout.fillWidth: true
                    visible: parent.needsGroup
                    textRole: "name"
                    model: parent.groupChoices
                    displayText: currentIndex < 0 ? qsTr("Choose…") : currentText
                    currentIndex: props.item ? parent.groupChoices.findIndex(z => z.id === props.item.zoneId) : -1
                    onActivated: (i) => props.setItemKey("zoneId", parent.groupChoices[i].id)
                }

                RowLayout {
                    visible: props.type === "buttons"
                    Caption { text: qsTr("Keyboard key") }
                    TextField {
                        Layout.preferredWidth: 50
                        color: Theme.text
                        maximumLength: 1
                        horizontalAlignment: Text.AlignHCenter
                        text: props.item ? (props.item.key || "").toUpperCase() : ""
                        onTextEdited: props.setItemKey("key", text.toLowerCase())
                    }
                }

                Caption { text: qsTr("Colour") }
                Flow {
                    Layout.fillWidth: true
                    spacing: 5
                    Repeater {
                        model: props.swatches
                        Rectangle {
                            required property string modelData
                            width: 24
                            height: 24
                            radius: 12
                            color: modelData
                            opacity: Theme.lightLevel
                            border.color: Theme.text
                            border.width: props.item && props.item.color && Qt.colorEqual(props.item.color, modelData) ? 2 : 0
                            MouseArea { anchors.fill: parent; onClicked: props.setItemKey("color", parent.modelData) }
                        }
                    }
                }

                // an icon, or a picture behind the button
                Caption { visible: props.type === "buttons"; text: qsTr("Icon") }
                Flow {
                    Layout.fillWidth: true
                    visible: props.type === "buttons"
                    spacing: 3
                    Repeater {
                        model: props.icons
                        Rectangle {
                            required property string modelData
                            readonly property bool chosen: !!props.item && (props.item.icon || "") === modelData
                            width: 28
                            height: 28
                            radius: Theme.radiusSmall
                            color: chosen ? Qt.alpha(Theme.accent, 0.3) : Theme.surfaceRaised
                            border.color: chosen ? Theme.accent : Theme.border
                            Text {
                                anchors.centerIn: parent
                                text: parent.modelData.length > 0 ? parent.modelData : "∅"
                                font.family: parent.modelData.length > 0 ? Theme.iconFamily : Theme.fontFamily
                                font.pixelSize: 13
                                color: Theme.text
                            }
                            MouseArea { anchors.fill: parent; onClicked: props.setItemKey("icon", parent.modelData) }
                        }
                    }
                }
                Caption { visible: props.type === "buttons"; text: qsTr("Image") }
                RowLayout {
                    Layout.fillWidth: true
                    visible: props.type === "buttons"
                    AppButton {
                        Layout.fillWidth: true
                        text: props.item && props.item.image ? props.fileName(props.item.image) : qsTr("Choose an image…")
                        iconGlyph: Theme.iconFolder
                        implicitHeight: 30
                        onClicked: { imageDialog.forSection = false; imageDialog.open() }
                    }
                    AppButton {
                        visible: !!(props.item && props.item.image)
                        iconGlyph: Theme.iconClose
                        implicitHeight: 30
                        flat: true
                        onClicked: props.setItemKey("image", "")
                    }
                }

                AppButton {
                    Layout.topMargin: 6
                    Layout.fillWidth: true
                    text: props.type === "faders" ? qsTr("Remove the fader") : qsTr("Remove the button")
                    iconGlyph: Theme.iconTrash
                    accentColor: Theme.danger
                    onClicked: Core.surface.clearItem(props.surface.currentPage, props.sid, props.cell)
                }
            }
        }
    }

    // pictures for buttons and image sections (kept as file URLs)
    FileDialog {
        id: imageDialog
        property bool forSection: false
        title: qsTr("Choose an image")
        nameFilters: [qsTr("Images (*.png *.jpg *.jpeg *.svg *.gif *.bmp *.webp)")]
        onAccepted: {
            let url = String(selectedFile)
            if (forSection)
                props.setSection({ image: url })
            else
                props.setItemKey("image", url)
        }
    }
}
