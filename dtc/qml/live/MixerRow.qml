/*
  DMX Total Control
  MixerRow.qml

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

/** The mixers side by side: all the fixtures, then each fixture group */
Flickable {
    id: row

    /** Fit the strips to the width instead of scrolling (SHOW sections) */
    property bool fit: false

    // keyed by mixer: a value change updates the strips in place (a fader keeps its drag)
    property var mixerIds: []
    property var mixerMap: ({})

    function sync() {
        let map = {}, ids = []
        for (let m of Core.mixer.mixers) {
            map[m.id] = m
            ids.push(m.id)
        }
        mixerMap = map
        if (ids.join("|") !== mixerIds.join("|"))
            mixerIds = ids
    }
    Component.onCompleted: sync()
    Connections {
        target: Core.mixer
        // (once for many changes: a show loading changes every mixer)
        function onMixersChanged() { Qt.callLater(row.sync) }
    }

    clip: true
    contentWidth: strips.width
    contentHeight: height
    flickableDirection: Flickable.HorizontalFlick
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.horizontal: ScrollBar {}

    Row {
        id: strips
        height: row.height - (row.fit ? 0 : 12)
        spacing: row.fit ? 6 : 10
        Repeater {
            model: row.mixerIds
            MixerStrip {
                required property int modelData
                mixer: row.mixerMap[modelData] || ({})
                height: strips.height
                width: row.fit ? Math.max(110, (row.width - (row.mixerIds.length - 1) * strips.spacing) / Math.max(1, row.mixerIds.length)) : 150
            }
        }
    }
}
