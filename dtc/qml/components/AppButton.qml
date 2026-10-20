/*
  DMX Total Control
  AppButton.qml

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

Button {
    id: control

    /** FontAwesome glyph shown before the text */
    property string iconGlyph: ""
    /** Background when highlighted/checked, or for "primary" buttons */
    property color accentColor: Theme.accent
    property bool primary: false

    implicitHeight: 34
    leftPadding: text.length > 0 ? 12 : 8
    rightPadding: text.length > 0 ? 12 : 8
    hoverEnabled: true
    focusPolicy: Qt.NoFocus

    /** Takes what is being typed in the field (or SpinBox) that still has the
     *  focus: an AppButton never takes it, and a field gives its text (a
     *  SpinBox its number) only on Enter or when it loses the focus. A button
     *  that reads a form calls this first in onClicked. */
    function commitEdits() {
        if (control.parent && Window.activeFocusItem)
            control.parent.forceActiveFocus()
    }

    readonly property bool lit: primary || checked || highlighted
    // lit: its colour, as dark as its text needs to read
    readonly property color litFill: Theme.fill(accentColor)
    readonly property color litText: Theme.textOn(litFill)
    // a lit button looks the same disabled: it fades instead
    opacity: lit && !enabled ? 0.6 : 1.0

    contentItem: Item {
        implicitWidth: contentRow.implicitWidth
        implicitHeight: contentRow.implicitHeight

        Row {
            id: contentRow
            spacing: 7
            anchors.centerIn: parent

            IconLabel {
                anchors.verticalCenter: parent.verticalCenter
                visible: control.iconGlyph.length > 0
                icon: control.iconGlyph
                size: 13
                color: control.lit ? control.litText : (control.enabled ? Theme.text : Theme.textFaint)
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                visible: control.text.length > 0
                text: control.text
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontNormal
                font.weight: Font.DemiBold
                color: control.lit ? control.litText : (control.enabled ? Theme.text : Theme.textFaint)
            }
        }
    }

    background: Rectangle {
        radius: Theme.radiusSmall
        color: {
            if (control.lit)
                return control.down ? Qt.darker(control.litFill, 1.2) : control.litFill
            if (control.flat)
                return control.hovered ? Theme.surfaceHover : "transparent"
            return control.down ? Theme.surface : (control.hovered ? Theme.surfaceHover : Theme.surfaceRaised)
        }
        border.width: control.flat || control.lit ? 0 : 1
        border.color: Theme.border
        Behavior on color { ColorAnimation { duration: 90 } }
    }
}
