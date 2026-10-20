/*
  DMX Total Control
  AppSwitch.qml

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

Switch {
    id: control

    focusPolicy: Qt.NoFocus
    padding: 0
    implicitWidth: 48
    implicitHeight: 28

    indicator: Rectangle {
        implicitWidth: 44
        implicitHeight: 24
        x: control.leftPadding
        y: (control.height - height) / 2
        radius: 12
        color: control.checked ? Theme.accentFill : Theme.background
        border.color: control.checked ? Theme.accentFill : Theme.borderStrong
        opacity: control.enabled ? 1 : 0.4
        Behavior on color { ColorAnimation { duration: 120 } }

        Rectangle {
            x: control.checked ? parent.width - width - 3 : 3
            y: 3
            width: 18
            height: 18
            radius: 9
            color: control.checked ? Theme.accentText : Theme.textDim
            Behavior on x { NumberAnimation { duration: 120 } }
        }
    }

    contentItem: Item {}
}
