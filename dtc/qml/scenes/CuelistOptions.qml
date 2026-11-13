/*
  DMX Total Control
  CuelistOptions.qml

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
 * How a cuelist plays (cuelist options): its type (a Chase: its speed, the
 * change of a step and its direction; a Timecode list: its clock, its own
 * or a MIDI timecode with an offset), tracking, what happens after the
 * last cue, release, the times of new cues, back and release, priority, the
 * beat driving and Move in black. Saved in the cuelist.
 */
Item {
    id: options

    property var info: ({})
    readonly property int fid: info.id !== undefined ? info.id : -1

    property int revision: 0
    readonly property var values: {
        revision
        return fid >= 0 ? Core.cuelists.options(fid) : ({})
    }

    /** Room for every option in two rows; else the rest folds away */
    readonly property bool roomy: width >= 1250
    property bool expanded: false
    readonly property bool showAll: roomy || expanded

    /** The types of cuelist, and what each one does */
    readonly property var types: [
        { key: "normal", text: qsTr("Normal"), about: qsTr("Normal: GO plays the next cue") },
        { key: "chase", text: qsTr("Chase"), about: qsTr("Chase: the cues follow each other by themselves, at a speed") },
        { key: "timecode", text: qsTr("Timecode"), about: qsTr("Timecode: each cue plays at its time on a clock") },
        { key: "submaster", text: qsTr("Submaster"),
          about: qsTr("Submaster: its fader rules: up from zero starts it, down to zero releases it; GO chooses the cue") },
        { key: "override", text: qsTr("Override"),
          about: qsTr("Override: its fader crossfades everything it holds between what is below and its cue") }
    ]
    readonly property string mode: values.mode || "normal"
    /** A Chase: its speed, its change and its direction instead of the end */
    readonly property bool chase: mode === "chase"
    /** A Timecode list: its clock */
    readonly property bool timecode: mode === "timecode"
    /** Its clock is a MIDI timecode: what comes in, about ten times a second */
    readonly property bool mtc: timecode && values.tcSource === "mtc"
    property string mtcStatus: ""
    property bool mtcReceiving: false
    property int mtcFps: 25
    /** The change of a step of a Chase: 0..100 %, -1 the times of the cues */
    readonly property int chaseXfade: values.chaseXfade !== undefined ? values.chaseXfade : 0
    readonly property var multipliers: [0.25, 0.5, 1, 2, 4]

    function set(key, value) { Core.cuelists.setOption(fid, key, value) }

    Connections {
        target: Core.cuelists
        function onEdited(id) { if (id === options.fid) options.revision++ }
    }

    Timer {
        interval: 100
        repeat: true
        running: options.mtc && options.visible
        triggeredOnStart: true
        onTriggered: {
            options.mtcStatus = Core.timecode.status()
            options.mtcReceiving = Core.timecode.receiving()
            options.mtcFps = Core.timecode.frameRate()
        }
    }

    implicitHeight: flow.implicitHeight

    // a label and its control, kept together when the row wraps
    component Labeled: Row {
        property string label: ""
        property string tip: ""
        spacing: 8
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: parent.label
            color: Theme.textDim
            ToolTip.text: parent.tip
            ToolTip.visible: parent.tip.length > 0 && labelMouse.containsMouse
            ToolTip.delay: 500
            MouseArea { id: labelMouse; anchors.fill: parent; hoverEnabled: true }
        }
    }

    Flow {
        id: flow
        anchors { left: parent.left; right: parent.right }
        spacing: 10

        // the type, on a line of its own: what it does beside it, or under
        // it when there is no room
        Flow {
            id: typeLine
            width: flow.width
            spacing: 10
            Labeled {
                label: qsTr("Type")
                Row {
                    spacing: 0
                    Repeater {
                        model: options.types
                        AppButton {
                            required property var modelData
                            text: modelData.text
                            implicitHeight: 28
                            highlighted: options.mode === modelData.key
                            onClicked: options.set("mode", modelData.key)
                        }
                    }
                }
            }
            Text {
                width: Math.min(implicitWidth, typeLine.width)
                height: Math.max(28, implicitHeight)
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
                text: (options.types.find((t) => t.key === options.mode) || options.types[0]).about
                color: Theme.textDim
                font.pixelSize: Theme.fontSmall
            }
        }

        // a Chase: its speed (its BPM or the one of the show, by a
        // multiplier), the change of a step and its direction
        Labeled {
            visible: options.chase
            label: qsTr("Speed")
            tip: qsTr("A step a beat")
            SpinBox {
                from: 10; to: 600
                editable: true
                implicitWidth: 156
                enabled: !options.values.chaseShowTempo
                value: options.values.chaseBpm || 120
                textFromValue: (v, locale) => v + " BPM"
                valueFromText: (text, locale) => { const n = parseInt(text); return isNaN(n) ? value : n }
                validator: RegularExpressionValidator { regularExpression: /.*/ }
                onValueModified: options.set("chaseBpm", value)
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("BPM of the show")
                color: Theme.textDim
            }
            AppSwitch {
                anchors.verticalCenter: parent.verticalCenter
                checked: !!options.values.chaseShowTempo
                onToggled: options.set("chaseShowTempo", checked)
            }
            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 0
                Repeater {
                    model: options.multipliers
                    AppButton {
                        required property real modelData
                        text: modelData === 0.25 ? "×¼" : modelData === 0.5 ? "×½" : "×" + modelData
                        implicitHeight: 28
                        highlighted: (options.values.chaseMultiplier || 1) === modelData
                        onClicked: options.set("chaseMultiplier", modelData)
                    }
                }
            }
        }
        Labeled {
            visible: options.chase
            label: qsTr("Crossfade")
            tip: qsTr("Part of the step the change takes (0: a cut)")
            SpinBox {
                from: 0; to: 100
                editable: true
                implicitWidth: 136
                enabled: options.chaseXfade >= 0
                value: Math.max(0, options.chaseXfade)
                textFromValue: (v, locale) => v + " %"
                valueFromText: (text, locale) => { const n = parseInt(text); return isNaN(n) ? value : n }
                validator: RegularExpressionValidator { regularExpression: /.*/ }
                onValueModified: options.set("chaseXfade", value)
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Cue times")
                color: Theme.textDim
            }
            AppSwitch {
                anchors.verticalCenter: parent.verticalCenter
                checked: options.chaseXfade < 0
                onToggled: options.set("chaseXfade", checked ? -1 : 0)
            }
        }
        Labeled {
            visible: options.chase
            label: qsTr("Direction")
            tip: qsTr("Random never plays the same step twice in a row")
            Row {
                spacing: 0
                Repeater {
                    model: [{ key: "forward", text: qsTr("Forward") }, { key: "backward", text: qsTr("Backward") },
                            { key: "bounce", text: qsTr("Bounce") }, { key: "random", text: qsTr("Random") }]
                    AppButton {
                        required property var modelData
                        text: modelData.text
                        implicitHeight: 28
                        highlighted: (options.values.chaseDirection || "forward") === modelData.key
                        onClicked: options.set("chaseDirection", modelData.key)
                    }
                }
            }
        }

        // a Timecode list: the clock its cues play on, its own (in a super
        // scene, the timeline) or a MIDI timecode, and where the 00:00 of
        // the list is on that timecode
        Labeled {
            visible: options.timecode
            label: qsTr("Clock")
            tip: options.mtc ? qsTr("The timecode drives the cuelist, also in a super scene: GO and Back do nothing")
                             : qsTr("In a super scene the timeline is the clock: the cuelist follows the cursor")
            Row {
                spacing: 0
                Repeater {
                    model: [{ key: "internal", text: qsTr("Own clock") },
                            { key: "mtc", text: qsTr("MIDI timecode (MTC)") }]
                    AppButton {
                        required property var modelData
                        text: modelData.text
                        implicitHeight: 28
                        highlighted: (options.mtc ? "mtc" : "internal") === modelData.key
                        onClicked: options.set("tcSource", modelData.key)
                    }
                }
            }
        }
        Labeled {
            visible: options.mtc
            label: qsTr("Offset")
            tip: qsTr("The timecode where the cuelist's 00:00 is")
            TextField {
                id: offsetField
                // hh:mm:ss:ff, in frames of the timecode that comes in
                readonly property string shown: {
                    options.mtcFps
                    return Core.timecode.offsetText(options.values.tcOffset || 0)
                }
                /** Something was typed: only then leaving the field sets it */
                property bool typed: false
                function commit() {
                    if (typed) {
                        const ms = Core.timecode.parseOffset(text)
                        if (ms >= 0)
                            options.set("tcOffset", ms)
                    }
                    typed = false
                    text = Qt.binding(() => offsetField.shown)
                }
                anchors.verticalCenter: parent.verticalCenter
                implicitWidth: 120
                text: shown
                font.family: Theme.monoFamily
                color: Theme.text
                selectByMouse: true
                validator: RegularExpressionValidator { regularExpression: /[0-9]{0,2}([:;.][0-9]{0,2}){0,3}/ }
                onTextEdited: typed = true
                onEditingFinished: commit()
                onActiveFocusChanged: if (activeFocus) selectAll()
                Keys.onShortcutOverride: (event) => event.accepted = event.key === Qt.Key_Escape
                Keys.onEscapePressed: { typed = false; text = Qt.binding(() => offsetField.shown); focus = false }
            }
        }
        // what comes in: the time and its frame rate, else where to turn it on
        Text {
            visible: options.mtc
            width: Math.min(implicitWidth, flow.width)
            height: Math.max(28, implicitHeight)
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap
            text: options.mtcReceiving ? options.mtcStatus
                                       : options.mtcStatus + " · " + qsTr("Turn on the input of the MIDI device in Mappings › Devices")
            color: options.mtcReceiving ? Theme.text : Theme.textDim
            font.pixelSize: Theme.fontSmall
        }

        // the ones used most, always there
        Labeled {
            label: qsTr("Tracking")
            tip: qsTr("A cue holds only what it changes; the rest goes on from the cues before it. Off: each cue on its own")
            AppSwitch { checked: !!options.values.tracking; onToggled: options.set("tracking", checked) }
        }
        Labeled {
            // (a Chase always goes round)
            visible: !options.chase
            label: qsTr("After the last cue")
            tip: qsTr("GO on the last cue goes back to the first cue (Loop), releases the cuelist (Release) or does nothing (Stay). With Follow or Wait on the first cue, Loop and Release go on by themselves")
            Row {
                spacing: 0
                Repeater {
                    model: [{ key: "loop", text: qsTr("Loop") }, { key: "release", text: qsTr("Release") },
                            { key: "stay", text: qsTr("Stay") }]
                    AppButton {
                        required property var modelData
                        text: modelData.text
                        implicitHeight: 28
                        highlighted: (options.values.atEnd || "loop") === modelData.key
                        onClicked: options.set("atEnd", modelData.key)
                    }
                }
            }
        }
        Labeled {
            label: qsTr("Release time")
            tip: qsTr("The fade when the cuelist is released")
            TimeSpin { implicitWidth: 120; engineValue: options.values.releaseTime || 0; onEdited: (v) => options.set("releaseTime", v) }
        }
        AppButton {
            visible: !options.roomy
            text: options.expanded ? qsTr("Fewer options") : qsTr("More options")
            iconGlyph: options.expanded ? Theme.iconCompress : Theme.iconExpand
            flat: true
            implicitHeight: 28
            onClicked: options.expanded = !options.expanded
        }

        // the rest: with room, or when asked
        Labeled {
            visible: options.showAll
            label: qsTr("Reset on release")
            tip: qsTr("On: after a release, GO starts again from the first cue. Off: it goes on from the cue it was on")
            AppSwitch { checked: !!options.values.resetOnRelease; onToggled: options.set("resetOnRelease", checked) }
        }
        Labeled {
            visible: options.showAll
            label: qsTr("Released by another GO")
            tip: qsTr("A GO on another cuelist releases this one (a song that ends when the next one starts)")
            AppSwitch { checked: !!options.values.releaseOnNextGo; onToggled: options.set("releaseOnNextGo", checked) }
        }
        Labeled {
            visible: options.showAll
            label: qsTr("New cue fade")
            tip: qsTr("The fade of the first cue recorded; the next ones take the times of the cue before them")
            TimeSpin { implicitWidth: 120; engineValue: options.values.defaultFade || 0; onEdited: (v) => options.set("defaultFade", v) }
        }
        Labeled {
            // (Back on a Chase is a step, with the times of its steps)
            visible: options.showAll && !options.chase
            label: qsTr("Back")
            tip: qsTr("The times of BACK: those of the cue it goes back to, or a time of its own")
            ComboBox {
                implicitWidth: 180
                model: [qsTr("Cue times"), qsTr("Fixed time")]
                currentIndex: (options.values.backTime !== undefined && options.values.backTime >= 0) ? 1 : 0
                onActivated: (i) => options.set("backTime", i === 1 ? 1000 : -1)
            }
            TimeSpin {
                implicitWidth: 120
                visible: options.values.backTime !== undefined && options.values.backTime >= 0
                engineValue: Math.max(0, options.values.backTime || 0)
                onEdited: (v) => options.set("backTime", v)
            }
        }
        Labeled {
            visible: options.showAll
            label: qsTr("Priority")
            SpinBox {
                from: 0; to: 10; editable: true
                implicitWidth: 104
                // (the info follows a priority changed elsewhere, as in a group)
                value: options.info.priority !== undefined ? options.info.priority
                       : options.values.priority !== undefined ? options.values.priority : 5
                onValueModified: options.set("priority", value)
            }
        }
        Labeled {
            visible: options.showAll
            label: qsTr("GO on the beat")
            tip: qsTr("BeatGO and Pulse give a GO on each division, like the GO button on a console")
            Row {
                spacing: 0
                Repeater {
                    model: [{ key: "off", text: qsTr("Off") }, { key: "beatgo", text: qsTr("BeatGO") },
                            { key: "pulse", text: qsTr("Pulse") }]
                    AppButton {
                        required property var modelData
                        text: modelData.text
                        implicitHeight: 28
                        highlighted: ((options.info.drive === "beatgo" || options.info.drive === "pulse")
                                      ? options.info.drive : "off") === modelData.key
                        onClicked: Core.effects.setPlayback(options.fid, "drive", modelData.key)
                    }
                }
            }
        }
        Labeled {
            visible: options.showAll
            label: qsTr("Tapped rhythm")
            tip: qsTr("Press its button several times in a row (Live, SHOW, a mapping): a GO on each beat, at that rhythm")
            TapRhythm {
                period: options.info.tap || 0
                onEdited: (ms) => Core.effects.setPlayback(options.fid, "tap", ms)
            }
        }
        // the dark fixtures get ready for the next cue: the wait after a cue
        // has finished, and the time of their moves (not in a Chase nor in
        // an Override list)
        Labeled {
            visible: options.showAll && !options.chase && options.mode !== "override"
            label: qsTr("Move in black")
            tip: qsTr("When a cue has finished, the fixtures that are dark and that the next cue lights up already take its position, colour and beam: at GO they only fade up")
            AppSwitch { checked: !!options.values.moveInBlack; onToggled: options.set("moveInBlack", checked) }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                visible: !!options.values.moveInBlack
                text: qsTr("After")
                color: Theme.textDim
            }
            TimeSpin {
                implicitWidth: 120
                visible: !!options.values.moveInBlack
                engineValue: options.values.mibDelay || 0
                onEdited: (v) => options.set("mibDelay", v)
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                visible: !!options.values.moveInBlack
                text: qsTr("Time")
                color: Theme.textDim
            }
            TimeSpin {
                implicitWidth: 120
                visible: !!options.values.moveInBlack
                engineValue: options.values.mibFade || 0
                onEdited: (v) => options.set("mibFade", v)
            }
        }
    }
}
