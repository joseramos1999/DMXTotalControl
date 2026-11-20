/*
  DMX Total Control
  ActionMenuItem.qml

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

/** An entry of an ActionMenu: a glyph (the one the toolbar has for the same
 *  thing) before its text, so every text starts at the same place, and on
 *  the right what it says of the selection: a mark, a hint (a count, why it
 *  is disabled), a chip, and a switch for what is on or off (toggle:
 *  toggleOn, toggleMixed when only some have it).
 *
 *  Never checkable: a click only emits triggered, and the switch shows only
 *  toggleOn and toggleMixed, so no tick stays from the last time (see
 *  CheckMenuItem). With the mouse still on it, tip shows beside the menu.
 *  shown: false hides it (no room, no stop of the mouse or the arrow keys);
 *  a caller that also sets enabled must include shown */
MenuItem {
    id: item

    /** The glyph before the text (Theme.icon*) and its colour */
    property string glyph: ""
    property color glyphColor: Theme.textDim
    /** A switch on the right: on, off, or halfway (toggleMixed: only some
     *  of the selection have it). (Not "on": a keyword of QML) */
    property bool toggle: false
    property bool toggleOn: false
    property bool toggleMixed: false
    /** A short text on the right (a count, why it is disabled) */
    property string hint: ""
    property color hintColor: Theme.textDim
    /** A small glyph on the right, before the hint */
    property string mark: ""
    property color markColor: Theme.textDim
    /** The hint drawn as a chip of this colour (with chipGlyph before it) */
    property color chipColor: "transparent"
    property string chipGlyph: ""
    /** A destructive action: red */
    property bool danger: false
    property color hoverColor: danger ? Qt.alpha(Theme.danger, 0.14) : Theme.surfaceHover
    /** What it does, beside the menu with the mouse still on it */
    property string tip: ""
    property bool shown: true

    visible: shown
    enabled: shown
    height: shown ? implicitHeight : 0
    focusPolicy: shown ? Qt.StrongFocus : Qt.NoFocus

    checkable: false
    implicitHeight: 30
    leftPadding: 12
    rightPadding: subMenu ? 28 : 12
    topPadding: 0
    bottomPadding: 0
    spacing: 10

    Accessible.checkable: toggle
    Accessible.checked: toggle && toggleOn
    Accessible.description: tip

    indicator: null

    // a submenu (for those who adopt it later): a chevron on the right
    arrow: IconLabel {
        visible: item.subMenu !== null
        x: item.width - width - 12
        y: (item.height - height) / 2
        icon: Theme.iconChevronRight
        size: 10
        color: item.highlighted ? Theme.text : Theme.textFaint
    }

    // (the colours of a disabled entry set here: Main.qml gives every group
    // of the palette the same windowText, and Basic's look enabled)
    contentItem: RowLayout {
        spacing: item.spacing

        IconLabel {
            Layout.preferredWidth: 16
            icon: item.glyph
            size: 13
            color: !item.enabled ? Theme.textFaint : item.danger ? Theme.readable(Theme.danger) : item.glyphColor
        }
        Text {
            Layout.fillWidth: true
            text: item.text
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontNormal
            color: !item.enabled ? Theme.textFaint : item.danger ? Theme.readable(Theme.danger) : Theme.text
            elide: Text.ElideRight
        }
        RowLayout {
            readonly property bool chip: item.chipColor.a > 0
            Layout.leftMargin: 12
            spacing: 6
            visible: item.mark.length > 0 || item.hint.length > 0 || item.toggle

            IconLabel {
                visible: item.mark.length > 0
                icon: item.mark
                size: 11
                color: item.enabled ? item.markColor : Theme.textFaint
            }
            Text {
                visible: item.hint.length > 0 && !parent.chip
                text: item.hint
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSmall
                color: item.enabled ? item.hintColor : Theme.textFaint
            }
            Rectangle {
                visible: item.hint.length > 0 && parent.chip
                implicitWidth: chipRow.implicitWidth + 14
                implicitHeight: 18
                radius: 9
                color: Qt.alpha(item.chipColor, 0.14)
                border.color: Qt.alpha(item.chipColor, 0.6)
                Row {
                    id: chipRow
                    anchors.centerIn: parent
                    spacing: 4
                    IconLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        visible: item.chipGlyph.length > 0
                        icon: item.chipGlyph
                        size: 9
                        color: Theme.readable(item.chipColor)
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: item.hint
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSmall
                        color: item.enabled ? Theme.text : Theme.textFaint
                    }
                }
            }
            // the switch: on, off, or halfway (only some)
            Rectangle {
                visible: item.toggle
                implicitWidth: 28
                implicitHeight: 16
                radius: 8
                opacity: item.enabled ? 1 : 0.4
                color: item.toggleOn ? Theme.accentFill : Theme.background
                border.color: item.toggleOn || item.toggleMixed ? Theme.accentFill : Theme.borderStrong
                Rectangle {
                    x: item.toggleOn ? 14 : item.toggleMixed ? 8 : 2
                    y: 2
                    width: 12
                    height: 12
                    radius: 6
                    color: item.toggleOn ? Theme.accentText : item.toggleMixed ? Theme.accentFill : Theme.textDim
                }
            }
        }
    }

    background: Rectangle {
        implicitHeight: 30
        x: 4
        width: item.width - 8
        radius: Theme.radiusSmall
        color: !item.enabled ? "transparent"
             : item.down ? Theme.mix(item.hoverColor, Theme.borderStrong, 0.35)
             : item.highlighted ? item.hoverColor : "transparent"
    }

    // the note waits for the mouse to stay on it: a timer of its own (the
    // delay of ToolTip never opened it from its visible binding)
    Timer {
        id: tipDelay
        property bool due: false
        interval: 700
        running: item.tip.length > 0 && item.hovered && !due
        onTriggered: due = true
    }
    onHoveredChanged: tipDelay.due = false

    // beside the menu (on its left near the edge of the window), never over
    // the entries above; not after the menu closed with the mouse on it
    ToolTip {
        parent: item
        text: item.tip
        width: Math.min(implicitWidth, 320)
        visible: tipDelay.due && item.tip.length > 0 && item.hovered && item.menu !== null && item.menu.opened
        x: {
            visible
            const win = item.Window.window
            const right = item.mapToItem(null, item.width + 8 + width, 0).x
            return !win || right <= win.width - 8 ? item.width + 8 : -width - 8
        }
        y: (item.height - height) / 2
    }
}
