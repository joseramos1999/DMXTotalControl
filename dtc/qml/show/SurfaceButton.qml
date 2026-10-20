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

    readonly property color tint: item.color ? Theme.category(item.color) : Theme.accent
    // on: the tint, as dark as its text needs to read
    readonly property color litTint: Theme.fill(tint)
    readonly property bool active: {
        Core.surface.revision
        Core.blackout
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
    // off: a dark tint, or a pale one on a light theme
    color: active ? litTint : Theme.isDark ? (mouse.pressed ? Qt.darker(tint, 2.2) : Qt.darker(tint, 3.4))
                                        : Theme.mix(Theme.surface, tint, mouse.pressed ? 0.4 : 0.16)
    border.color: tint
    border.width: active ? 0 : 1
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
    // or an icon above the label
    Text {
        id: iconText
        visible: !!button.item.icon && !button.item.image
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: parent.height * 0.12 }
        text: button.item.icon || ""
        font.family: Theme.iconFamily
        font.pixelSize: Math.max(12, Math.min(button.height * 0.36, button.width * 0.4))
        color: button.active ? Theme.textOn(button.litTint) : Theme.readable(button.tint, button.color)
    }

    Text {
        anchors { fill: parent; margins: 6; topMargin: iconText.visible ? iconText.y + iconText.height : 6 }
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
        visible: button.item.mode === "flash" && ["scene", "blackout", "palette"].indexOf(button.item.action) >= 0
        anchors { left: parent.left; top: parent.top; margins: 4 }
        text: Theme.iconBolt
        font.family: Theme.iconFamily
        font.pixelSize: 10
        color: button.active ? Theme.textOn(button.litTint) : Theme.readable(button.tint, button.color)
    }
    Rectangle {
        visible: !!button.item.key
        anchors { right: parent.right; top: parent.top; margins: 4 }
        width: keyText.implicitWidth + 6
        height: 14
        radius: 3
        color: "transparent"
        Text {
            id: keyText
            anchors.centerIn: parent
            text: (button.item.key || "").toUpperCase()
            color: button.active ? Theme.textOn(button.litTint) : Theme.text
            font.family: Theme.monoFamily
            font.pixelSize: 9
        }
    }

    // a scene with steps (a cuelist) that plays: its rhythm, a half a step
    RhythmBar {
        functionId: button.item.action === "scene" && button.item.functionId !== undefined ? button.item.functionId : -1
        litColor: button.active ? Theme.textOn(button.litTint) : Theme.readable(button.tint, button.color)
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: 6; bottomMargin: 4 }
        height: Math.max(3, Math.min(6, button.height / 16))
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        enabled: !button.editing
        onPressed: button.down()
        onReleased: button.up()
        onCanceled: button.up()
    }

    // learn mode: map the same action to a device
    MapTarget {
        readonly property var targets: ({ scene: "scene", next: "groupNext", previous: "groupPrevious",
                                          release: "groupRelease", blackout: "blackout", stopAll: "stopAll",
                                          tap: "tap", beatgo: "beatgo", palette: "palette" })
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
            return t
        }
    }
}
