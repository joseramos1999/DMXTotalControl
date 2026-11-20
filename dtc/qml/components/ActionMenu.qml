/*
  DMX Total Control
  ActionMenu.qml

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

/** A menu of actions on something (a right click): a card with rounded
 *  corners and a shadow, for a MenuHeader that says what it acts on, the
 *  MenuSection titles and the compact ActionMenuItem rows (a glyph before
 *  each text, a switch for what is on or off). As wide as its widest entry,
 *  as AppMenu. Drawn inside the window (Popup.Item): its shadow and the
 *  notes beside its entries are never cut, and the grabs show it as it is.
 *  A right click opens it with popupAt, never with an entry under the pointer */
AppMenu {
    id: menu

    popupType: Popup.Item
    // (never against the edge of the window)
    margins: Theme.spacing
    topPadding: 4
    bottomPadding: 4
    leftPadding: 0
    rightPadding: 0

    // (the entries made from an Action or a submenu)
    delegate: ActionMenuItem {}

    /** Opens it at $x, $y of $item as a context menu opens: below and to the
     *  right of the point, else above it and/or to its left, so the pointer is
     *  never on an entry when it opens (a click in place must not run one) */
    function popupAt(item, x, y) {
        pointAt = item.mapToItem(null, x, y)
        popup(item, x + 1, y + 1)
        place()
    }
    /** Where popupAt asked for it, in the window (-1, -1: opened with popup(),
     *  placed as usual) */
    property point pointAt: Qt.point(-1, -1)
    // (its size is known only once its list is laid out, and changes with
    // the entries it shows)
    onHeightChanged: if (visible) place()
    onWidthChanged: if (visible) place()
    onClosed: pointAt = Qt.point(-1, -1)
    /** Puts it beside pointAt, inside the window and its margins */
    function place() {
        // (the window, not the overlay: that one may have no size yet the
        // first time a menu opens)
        const win = parent ? parent.Window.window : null
        if (!win || pointAt.x < 0)
            return
        const ww = win.width, wh = win.height
        const m = margins, w = width, h = height, px = pointAt.x, py = pointAt.y
        let sx = px + 1, sy = py + 1
        // above the point, and/or to its left
        if (sy + h > wh - m)
            sy = py - h - 1
        if (sx + w > ww - m)
            sx = px - w - 1
        sx = Math.max(m, Math.min(sx, ww - m - w))
        sy = Math.max(m, Math.min(sy, wh - m - h))
        // (no room on either side: beside the pointer, as high as it fits)
        if (px >= sx && px <= sx + w && py >= sy && py <= sy + h)
            sx = px + 2 + w <= ww - m ? px + 2 : Math.max(m, px - w - 2)
        const p = parent.mapFromItem(null, sx, sy)
        x = p.x
        y = p.y
    }

    // plain rectangles: the software renderer draws them too
    background: Item {
        implicitWidth: 200
        implicitHeight: 40
        // the shadow, a little lower
        Rectangle {
            anchors { fill: parent; topMargin: 3; bottomMargin: -3; leftMargin: -1; rightMargin: -1 }
            radius: Theme.radius + 1
            color: Theme.shadow
        }
        Rectangle {
            anchors.fill: parent
            radius: Theme.radius
            color: Theme.surface
            border.color: Theme.borderStrong
        }
    }
}
