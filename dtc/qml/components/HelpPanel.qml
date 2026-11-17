/*
  DMX Total Control
  HelpPanel.qml

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
 * The manual (docs/manual, built in), beside the page: F12 or the «?» of the
 * top bar open the page of the manual about the page on screen. Its links
 * go to other pages of the manual, or to the browser.
 */
Drawer {
    id: help

    /** The page shown: "index", "programar"... */
    property string page: "index"
    property var history: []
    /** The page as HTML; the links take the colour of the theme (RichText
        ignores linkColor: a style sheet) */
    property string html: ""

    /** A place of the page to scroll to once it is laid out ("" none) */
    property string pendingAnchor: ""

    /** $name: a page, or a place of it: "opciones#usb-selective-suspend" */
    function show(name) {
        history = []
        const hash = name.indexOf("#")
        if (!load(hash < 0 ? name : name.substring(0, hash), false, hash < 0 ? "" : name.substring(hash + 1)))
            load("index")
        open()
    }
    function load(name, keepScroll, anchor) {
        // (the width of the drawer: the text has none before it opens)
        const html = Core.manualHtml(name, Math.max(200, help.width - 56))
        if (html.length === 0)
            return false
        help.html = html
        page = name
        // (the same page at another width, while it opens: its place stays)
        if (anchor)
            pendingAnchor = anchor
        else if (!keepScroll)
            pendingAnchor = ""
        if (pendingAnchor.length > 0) {
            // (laid out at the width of the drawer once it is open: onOpened)
            if (help.opened)
                Qt.callLater(help.scrollToAnchor)
        } else if (!keepScroll) {
            flick.contentY = 0
        }
        return true
    }
    /** Down to the place pendingAnchor of the page: by the height of what
     *  comes before it (the top when the page doesn't have it) */
    function scrollToAnchor() {
        if (pendingAnchor.length === 0)
            return
        const before = Core.manualHtmlBefore(page, pendingAnchor, Math.max(200, help.width - 56))
        pendingAnchor = ""
        if (before.length === 0) {
            flick.contentY = 0
            return
        }
        measure.text = before
        const y = content.y + measure.implicitHeight
        measure.text = ""
        flick.contentY = Math.max(0, Math.min(y, flick.contentHeight - flick.height))
    }
    function follow(link) {
        // another page of the manual: "programar.md", "programar.md#paletas"
        const match = /^([a-z0-9-]+)\.md(#.*)?$/.exec(link)
        if (match) {
            const from = page
            if (load(match[1], false, match[2] ? match[2].substring(1) : ""))
                history = history.concat([from])
        } else {
            Qt.openUrlExternally(link)
        }
    }

    onOpened: if (pendingAnchor.length > 0) scrollToAnchor()

    edge: Qt.RightEdge
    width: Math.min(640, Math.max(360, parent ? parent.width * 0.45 : 640))
    height: parent ? parent.height : 600
    modal: false
    padding: 0

    // a new width: the pictures follow it (also while it opens)
    onWidthChanged: if (html.length > 0) Qt.callLater(() => load(page, true))

    background: Rectangle {
        color: Theme.surface
        Rectangle { width: 1; height: parent.height; color: Theme.border }
    }

    contentItem: ColumnLayout {
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            color: Theme.surfaceRaised
            Rectangle { width: parent.width; height: 1; anchors.bottom: parent.bottom; color: Theme.border }
            RowLayout {
                anchors { fill: parent; leftMargin: 12; rightMargin: 8 }
                spacing: 6
                AppButton {
                    iconGlyph: Theme.iconStepBackward
                    flat: true
                    enabled: help.history.length > 0
                    ToolTip.text: qsTr("Back"); ToolTip.visible: hovered
                    onClicked: {
                        const back = help.history[help.history.length - 1]
                        help.history = help.history.slice(0, -1)
                        help.load(back)
                    }
                }
                AppButton {
                    text: qsTr("Contents")
                    iconGlyph: Theme.iconList
                    flat: true
                    enabled: help.page !== "index"
                    onClicked: help.follow("index.md")
                }
                Text {
                    Layout.fillWidth: true
                    // the manual in the language of the app; else the original, in Spanish: said
                    text: Qt.uiLanguage.startsWith("es") || Core.manualLanguage().length > 0 ? qsTr("Manual") : qsTr("Manual (in Spanish)")
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                    font.weight: Font.Bold
                    font.capitalization: Font.AllUppercase
                    horizontalAlignment: Text.AlignRight
                    elide: Text.ElideRight
                }
                AppButton {
                    iconGlyph: Theme.iconClose
                    flat: true
                    ToolTip.text: qsTr("Close (Esc)"); ToolTip.visible: hovered
                    onClicked: help.close()
                }
            }
        }

        Flickable {
            id: flick
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentHeight: content.implicitHeight + 40
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}

            Text {
                id: content
                x: 20
                y: 16
                width: flick.width - 40
                textFormat: Text.RichText
                text: help.html.replace("</head>", "<style>a { color: " + Theme.accent + "; }</style></head>")
                wrapMode: Text.WordWrap
                baseUrl: "qrc:/manual/"
                color: Theme.text
                font.pixelSize: Theme.fontNormal
                onLinkActivated: (link) => help.follow(link)
                HoverHandler { cursorShape: content.hoveredLink.length > 0 ? Qt.PointingHandCursor : Qt.ArrowCursor }
            }
            // (not shown: what comes before a place of the page, laid out as
            // the page is, for its height)
            Text {
                id: measure
                x: content.x
                width: content.width
                opacity: 0
                enabled: false
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
                baseUrl: content.baseUrl
                font: content.font
            }
        }
    }
}
