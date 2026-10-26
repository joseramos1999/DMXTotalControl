/*
  DMX Total Control
  CheckMenuItem.qml

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

/** A menu entry whose tick shows only "selected". A click toggles a checkable item by
    itself, and in a menu shared by several objects that tick stayed: the binding is only
    evaluated again when the value changes, and opening the menu for another object with
    the same value did not change it. So the click puts the tick back from "selected". */
MenuItem {
    id: item

    property bool selected: false

    checkable: true
    checked: selected
    onToggled: checked = Qt.binding(() => item.selected)
}
