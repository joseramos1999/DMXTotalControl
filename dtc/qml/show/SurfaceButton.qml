/*
  DMX Total Control
  SurfaceButton.qml

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
import DMXTotalControl
import DMXTotalControl.Backend

/** A button of the SHOW screen, the size of its cell */
Rectangle {
    id: button

    property var item: ({})
    property bool editing: false

    // its colour, bright (arming the special effects: red, the show stop
    // yellow, as on the phone, unless it has a colour of its own)
    readonly property color tint: Theme.category(Theme.showColour(item.color ? item.color
                                  : item.action === "armEffects" ? "#e5484d"
                                  : item.action === "showStop" ? "#f5a524" : Theme.accent))
    // on: the tint, as dark as its text needs to read
    readonly property color litTint: Theme.fill(tint)
    /** Off: the body of the theme with a touch of its colour (pressed, more) */
    readonly property color body: Theme.mix(Theme.surfaceRaised, tint, mouse.pressed ? 0.3 : 0.07)
    /** The marks (flash, key) on its colour strip when it is tall enough and
     *  they read on it (not at night on the light colours), else under it */
    readonly property bool marksOnStrip: strip.height >= 14 && Theme.contrast(Theme.textOn(tint), tint) >= 4.4
    /** Its icon: the one chosen, or the flame of the special effects (the
     *  one that only disarms: the power sign, as Disarm on the banner) */
    readonly property string glyph: item.icon ? item.icon
                                    : item.action === "armEffects" ? (item.mode === "off" ? Theme.iconPower : Theme.iconFire)
                                    : item.action === "showStop" ? Theme.iconHand
                                    : item.action === "recoverStopped" ? Theme.iconUndo
                                    : item.action === "fadeToBlack" ? Theme.iconMoon
                                    : ""
    readonly property bool active: {
        Core.surface.revision
        Core.blackout
        Core.safety.armed
        Core.safety.showStop
        Core.safety.fadingToBlack
        Core.recall.available
        // palettes are live edits of the programmer: they change with it
        Core.palettes.states
        return Core.surface.isActive(item)
    }

    function label() {
        if (item.label)
            return item.label
        switch (item.action) {
        case "scene": {
            let info = Core.functionInfo(item.functionId)
            return info.name || qsTr("Choose a scene")
        }
        case "next": return qsTr("Next")
        case "previous": return qsTr("Previous")
        case "release": return qsTr("Release")
        case "blackout": return qsTr("Blackout")
        case "stopAll": return qsTr("Stop all")
        case "tap": return qsTr("Tap")
        case "beatgo": return "BeatGO"
        // (the one that only disarms says so, armed or not)
        case "armEffects": return item.mode === "off" ? qsTr("Disarm effects")
                                  : Core.safety.armed ? qsTr("Effects armed") : qsTr("Arm effects")
        case "showStop": return qsTr("Show stop")
        case "recoverStopped": return qsTr("Bring back")
        case "fadeToBlack": return qsTr("Fade to black")
        case "map":
            // (a playback is named by what its slot holds now, a function by its name)
            Core.surface.revision
            return Core.surface.mapLabel(item.map || ({}))
        case "palette": {
            const palette = Core.palettes.palettes.find(p => p.id === item.paletteId)
            return palette ? palette.name : qsTr("Choose a palette")
        }
        }
        return ""
    }

    // everything runs in the surface (the remote control does the same)
    function down() { Core.surface.press(item) }
    function up() { Core.surface.release(item) }

    radius: Theme.radiusSmall
    // off: the body, its colour on top; on: all of it (its edge: over the strip, below)
    color: active ? litTint : body
    clip: true

    // a picture behind (background image), dimmed while off
    Image {
        anchors { fill: parent; margins: button.active ? 0 : 1 }
        visible: !!button.item.image
        source: button.item.image ? (button.item.image.indexOf("file:") === 0 ? button.item.image : "file:///" + button.item.image) : ""
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        opacity: button.active ? 1 : 0.45
    }
    // its colour: the top fifth, on or off
    Rectangle {
        id: strip
        anchors { left: parent.left; right: parent.right; top: parent.top }
        height: Math.max(4, Math.round(button.height / 5))
        topLeftRadius: button.radius
        topRightRadius: button.radius
        color: button.tint
        // (a light theme: an edge under it, for the white and the yellow)
        Rectangle {
            visible: !Theme.isDark && !button.active
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
            height: 1
            color: Theme.mix(button.tint, Theme.text, 0.35)
        }
    }
    // its edge, round the strip and the picture too (on, on a light theme: a
    // dark one, else a white one would look off)
    Rectangle {
        anchors.fill: parent
        radius: button.radius
        color: "transparent"
        border.color: button.active ? (Theme.isDark ? button.litTint : Theme.mix(button.litTint, Theme.text, 0.6))
                                    : Theme.mix(Theme.borderStrong, button.tint, 0.35)
        border.width: button.active && !Theme.isDark ? 2 : 1
    }
    // or an icon above the label, under the strip (a short button: smaller,
    // so that the label fits under it)
    Text {
        id: iconText
        visible: button.glyph.length > 0 && !button.item.image
        anchors { horizontalCenter: parent.horizontalCenter; top: strip.bottom; topMargin: (button.height - strip.height) * 0.08 }
        text: button.glyph
        font.family: Theme.iconFamily
        font.pixelSize: Math.max(8, Math.min(button.height * 0.32, button.width * 0.4,
                                             button.height - strip.height - labelText.font.pixelSize * 1.3 - 10))
        color: button.active ? Theme.textOn(button.litTint) : Theme.readable(button.tint, button.color)
    }

    Text {
        id: labelText
        // (the marks under the strip: it starts under them, or, a short
        // button, it keeps clear of them at its sides)
        readonly property real marksBottom: button.marksOnStrip || !(flashMark.visible || keyMark.visible) ? 0 : strip.height + 17
        readonly property bool marksBeside: marksBottom > 0 && button.height - 6 - marksBottom < font.pixelSize * 2.6
        anchors { fill: parent; margins: 6
                  leftMargin: marksBeside && flashMark.visible ? flashMark.x + flashMark.width + 4 : 6
                  rightMargin: marksBeside && keyMark.visible ? keyMark.width + 8 : 6
                  topMargin: Math.max(iconText.visible ? iconText.y + iconText.height : strip.height + 4,
                                      marksBeside ? 0 : marksBottom) }
        text: button.label()
        style: button.item.image ? Text.Outline : Text.Normal
        styleColor: "black"
        color: button.active ? Theme.textOn(button.litTint) : Theme.text
        font.pixelSize: Math.max(10, Math.min(20, button.height / 4, button.width / 7))
        font.weight: Font.DemiBold
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    // flash buttons are marked like the flash scenes of the groups
    Text {
        id: flashMark
        visible: button.item.mode === "flash" && ["scene", "blackout", "palette"].indexOf(button.item.action) >= 0
        x: 5
        y: button.marksOnStrip ? (strip.height - implicitHeight) / 2 : strip.height + 3
        text: Theme.iconBolt
        font.family: Theme.iconFamily
        font.pixelSize: 10
        color: button.marksOnStrip ? Theme.textOn(button.tint)
             : button.active ? Theme.textOn(button.litTint) : Theme.readable(button.tint, button.color)
    }
    Rectangle {
        id: keyMark
        visible: !!button.item.key
        x: button.width - width - 4
        y: button.marksOnStrip ? (strip.height - height) / 2 : strip.height + 3
        width: keyText.implicitWidth + 6
        height: 14
        radius: 3
        color: "transparent"
        Text {
            id: keyText
            anchors.centerIn: parent
            text: (button.item.key || "").toUpperCase()
            color: button.marksOnStrip ? Theme.textOn(button.tint)
                 : button.active ? Theme.textOn(button.litTint) : Theme.text
            font.family: Theme.monoFamily
            font.pixelSize: 9
        }
    }

    // a scene with steps (a cuelist, an FX at the BPM of the show) that plays: its rhythm, a half a step
    RhythmBar {
        functionId: button.item.action === "scene" && button.item.functionId !== undefined ? button.item.functionId : -1
        litColor: button.active ? Theme.textOn(button.litTint) : Theme.readable(button.tint, button.color)
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: 6; bottomMargin: 4 }
        height: Math.max(3, Math.min(6, button.height / 16))
    }

    // the show stop: how far the hold is while it is pressed (held a second,
    // the backend puts it or takes it away)
    Rectangle {
        id: holdStrip
        property real progress: 0
        visible: button.item.action === "showStop" && mouse.pressed && !mouse.stopping && Core.safety.holding
        anchors { left: parent.left; bottom: parent.bottom }
        width: parent.width * progress
        height: Math.max(4, Math.min(8, button.height / 12))
        color: button.active ? Theme.textOn(button.litTint) : Theme.readable(button.tint, button.color)
        NumberAnimation on progress {
            running: holdStrip.visible
            from: 0; to: 1
            duration: Core.safety.holdMs
        }
    }
    // the fade to black: how far down it is (Blackout at its end)
    Rectangle {
        id: fadeStrip
        property real progress: 0
        visible: button.item.action === "fadeToBlack" && Core.safety.fadingToBlack
        anchors { left: parent.left; bottom: parent.bottom }
        width: parent.width * progress
        height: Math.max(4, Math.min(8, button.height / 12))
        color: button.active ? Theme.textOn(button.litTint) : Theme.readable(button.tint, button.color)
        // (started here, with the time of the fade that starts)
        NumberAnimation { id: fadeRun; target: fadeStrip; property: "progress"; from: 0; to: 1 }
        onVisibleChanged: {
            fadeRun.stop()
            if (visible) {
                fadeRun.duration = Math.max(1, Core.safety.fadeToBlackMs)
                fadeRun.start()
            }
        }
    }

    MouseArea {
        id: mouse
        // a right click stops what the button plays, in GO as in Stop
        property bool stopping: false
        anchors.fill: parent
        enabled: !button.editing
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: (event) => {
            stopping = event.button === Qt.RightButton
            if (stopping)
                Core.surface.stop(button.item)
            else
                button.down()
        }
        onReleased: if (!stopping) button.up()
        onCanceled: if (!stopping) button.up()
    }

    // learn mode: map the same action to a device
    MapTarget {
        readonly property var targets: ({ scene: "scene", next: "groupNext", previous: "groupPrevious",
                                          release: "groupRelease", blackout: "blackout", stopAll: "stopAll",
                                          tap: "tap", beatgo: "beatgo", palette: "palette", armEffects: "armEffects",
                                          showStop: "showStop", recoverStopped: "recoverStopped", fadeToBlack: "fadeToBlack" })
        visible: Core.mapping.learnMode && !button.editing && (targets[button.item.action] !== undefined || button.item.action === "map")
        target: {
            let action = button.item.action
            if (action === "map")
                return button.item.map || ({})
            let t = { target: targets[action] }
            if (action === "scene")
                t.functionId = button.item.functionId
            else if (["next", "previous", "release"].indexOf(action) >= 0)
                t.zoneId = button.item.zoneId !== undefined ? button.item.zoneId : -1
            else if (action === "palette") {
                t.paletteId = button.item.paletteId
                t.groupId = button.item.groupId !== undefined ? button.item.groupId : -1
            }
            else if (action === "fadeToBlack")
                t.time = button.item.time !== undefined ? button.item.time : Core.safety.defaultFadeToBlackMs
            return t
        }
    }
}
