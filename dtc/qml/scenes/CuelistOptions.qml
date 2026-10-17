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
 * How a cuelist plays (cuelist options): tracking, what happens after
 * the last cue, release, the times of new cues, back and release, priority
 * and the beat driving. Saved in the cuelist.
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

    function set(key, value) { Core.cuelists.setOption(fid, key, value) }

    Connections {
        target: Core.cuelists
        function onEdited(id) { if (id === options.fid) options.revision++ }
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


        // the ones used most, always there
        Labeled {
            label: qsTr("Tracking")
            tip: qsTr("A cue holds only what it changes; the rest goes on from the cues before it. Off: each cue on its own")
            AppSwitch { checked: !!options.values.tracking; onToggled: options.set("tracking", checked) }
        }
        Labeled {
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
            visible: options.showAll
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
    }
}
