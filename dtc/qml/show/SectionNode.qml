/*
  DMX Total Control
  SectionNode.qml

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
import DMXTotalControl
import DMXTotalControl.Backend

/**
 * A node of a SHOW page: a section, or a split in two halves (side by side
 * or one above the other) with a divider that moves in edit mode.
 */
Item {
    id: node

    property var nodeData: ({})
    property var surface: null

    readonly property bool isSplit: nodeData.split === "h" || nodeData.split === "v"
    readonly property bool horizontal: nodeData.split === "h"
    readonly property int gap: 6
    // while the divider is dragged, before it is saved
    property real dragRatio: -1
    readonly property real ratio: dragRatio >= 0 ? dragRatio : (nodeData.ratio || 0.5)
    /** Half of a split, and of a split in a column (the halves one above
     *  the other): a group section can fold there, to its strip */
    property bool inSplit: false
    property bool inColumn: false

    // a folded group section takes its strip only: across the row, or its
    // header in a column (with the title of the section, if it shows)
    function folded(d) {
        return !!d && d.split !== "h" && d.split !== "v" && d.type === "group" && d.folded === true
    }
    readonly property bool firstFolded: isSplit && folded(nodeData.a)
    readonly property bool secondFolded: isSplit && folded(nodeData.b) && !firstFolded
    function foldedSize(d) {
        if (horizontal)
            return 46
        const header = (d && (d.title || "").length > 0) || (surface !== null && surface.editing) ? 26 : 0
        return 50 + header
    }
    readonly property real span: (horizontal ? width : height) - gap
    readonly property real firstSpan: firstFolded ? foldedSize(nodeData.a)
                                    : secondFolded ? span - foldedSize(nodeData.b) : span * ratio

    // a section
    Loader {
        anchors.fill: parent
        active: !node.isSplit
        sourceComponent: Component {
            SectionView {
                section: node.nodeData
                surface: node.surface
                inSplit: node.inSplit
                foldsUp: node.inColumn
            }
        }
    }

    // a split: the two halves are nodes too
    Loader {
        id: first
        active: node.isSplit
        source: "SectionNode.qml"
        x: 0
        y: 0
        width: node.horizontal ? node.firstSpan : node.width
        height: node.horizontal ? node.height : node.firstSpan
        onLoaded: {
            item.surface = Qt.binding(() => node.surface)
            item.nodeData = Qt.binding(() => node.nodeData.a || ({}))
            item.inSplit = true
            item.inColumn = Qt.binding(() => !node.horizontal)
        }
    }
    Loader {
        id: second
        active: node.isSplit
        source: "SectionNode.qml"
        x: node.horizontal ? first.width + node.gap : 0
        y: node.horizontal ? 0 : first.height + node.gap
        width: node.horizontal ? node.width - first.width - node.gap : node.width
        height: node.horizontal ? node.height : node.height - first.height - node.gap
        onLoaded: {
            item.surface = Qt.binding(() => node.surface)
            item.nodeData = Qt.binding(() => node.nodeData.b || ({}))
            item.inSplit = true
            item.inColumn = Qt.binding(() => !node.horizontal)
        }
    }

    // the divider: drag it in edit mode (it snaps to quarters, thirds and halves)
    Rectangle {
        visible: node.isSplit
        x: node.horizontal ? first.width : 0
        y: node.horizontal ? 0 : first.height
        width: node.horizontal ? node.gap : node.width
        height: node.horizontal ? node.height : node.gap
        color: dividerMouse.containsMouse || dividerMouse.pressed ? Theme.accent : "transparent"
        opacity: 0.6
        radius: 2

        MouseArea {
            id: dividerMouse
            anchors.fill: parent
            anchors.margins: -2
            enabled: node.surface !== null && node.surface.editing
            hoverEnabled: true
            cursorShape: node.horizontal ? Qt.SplitHCursor : Qt.SplitVCursor
            preventStealing: true
            onPositionChanged: (ev) => {
                if (!pressed)
                    return
                let p = mapToItem(node, ev.x, ev.y)
                let r = node.horizontal ? p.x / node.width : p.y / node.height
                for (let snap of [0.25, 1 / 3, 0.5, 2 / 3, 0.75])
                    if (Math.abs(r - snap) < 0.02)
                        r = snap
                node.dragRatio = Math.max(0.1, Math.min(0.9, r))
            }
            onReleased: {
                if (node.dragRatio >= 0)
                    Core.surface.setSplitRatio(node.surface.currentPage, node.nodeData.id, node.dragRatio)
                node.dragRatio = -1
            }
        }
    }
}
