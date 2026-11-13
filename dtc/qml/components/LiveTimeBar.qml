/*
  DMX Total Control
  LiveTimeBar.qml

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

/**
 * Live Time of the cue a cuelist fades: its time (its delay and fade, as it
 * runs), to type, halve, double or end now; the fade goes on from where it
 * is. For this run only: Store keeps the time in the cue. It shows while a
 * cue fades, and after a change until the next cue (and in learn mode, with
 * a mapping target). The cuelist of a playback (pbPage, pbSlot), or of
 * functionId wherever it is.
 */
Row {
    id: bar

    /** slotInfo() or functionStatus() of the cuelist */
    property var info: ({})
    property int pbPage: -1
    property int pbSlot: -1
    /** >= 0: the cuelist itself (the cuelist editor) */
    property int functionId: -1
    /** What a mapping of the time drives (learn mode), or null */
    property var mapTarget: null
    /** Store with its icon only (its name in the tooltip) */
    property bool compact: false

    /** (not one an A/B fader crossfades by hand: the hand has its time) */
    readonly property bool fading: !!info.fading && !info.manual
    /** Live Time changed the cue that plays */
    readonly property bool changed: !!info.running && info.liveScale !== undefined && Math.abs(info.liveScale - 1) > 0.0005
    /** The cue runs with its own times (not a BACK time or a fade-in): Store can keep the new one */
    readonly property bool ownTimes: info.ownTimes !== false

    function setTime(ms) {
        if (functionId >= 0)
            Core.playbacks.setLiveTimeFunction(functionId, ms)
        else
            Core.playbacks.setLiveTime(pbPage, pbSlot, ms)
    }
    function scaleTime(factor) {
        if (functionId >= 0)
            Core.playbacks.scaleLiveTimeFunction(functionId, factor)
        else
            Core.playbacks.scaleLiveTime(pbPage, pbSlot, factor)
    }
    function store() {
        if (functionId >= 0)
            Core.playbacks.storeLiveTimeFunction(functionId)
        else
            Core.playbacks.storeLiveTime(pbPage, pbSlot)
    }

    visible: !!info.isCuelist && (fading || changed || (mapTarget !== null && Core.mapping.learnMode))
    spacing: 4

    Text {
        anchors.verticalCenter: parent.verticalCenter
        text: qsTr("Live time")
        // amber: not the time of the cue
        color: bar.changed ? Theme.warning : Theme.textDim
        font.pixelSize: Theme.fontSmall
        font.weight: Font.DemiBold
        MouseArea { id: labelMouse; anchors.fill: parent; hoverEnabled: true }
        ToolTip.text: qsTr("The time of the cue that is fading: change it and the fade goes on from where it is")
        ToolTip.visible: labelMouse.containsMouse
        ToolTip.delay: 500
    }
    // (the learn overlay beside the field, not in it: a disabled field
    // would disable it too, and the time is learnt with no cue fading)
    Item {
        anchors.verticalCenter: parent.verticalCenter
        implicitWidth: timeField.implicitWidth
        implicitHeight: timeField.implicitHeight
        TimeSpin {
            id: timeField
            anchors.fill: parent
            implicitWidth: 136
            implicitHeight: 30
            enabled: bar.fading
            engineValue: bar.info.fadeTotal || 0
            onEdited: (ms) => bar.setTime(ms)
        }
        MapTarget {
            visible: bar.mapTarget !== null
            target: bar.mapTarget || ({})
        }
    }
    AppButton {
        anchors.verticalCenter: parent.verticalCenter
        implicitHeight: 28
        text: "×½"
        enabled: bar.fading
        onClicked: bar.scaleTime(0.5)
    }
    AppButton {
        anchors.verticalCenter: parent.verticalCenter
        implicitHeight: 28
        text: "×2"
        enabled: bar.fading
        onClicked: bar.scaleTime(2)
    }
    AppButton {
        anchors.verticalCenter: parent.verticalCenter
        implicitHeight: 28
        text: qsTr("Now")
        enabled: bar.fading
        ToolTip.text: qsTr("Finish the fade now")
        ToolTip.visible: hovered
        ToolTip.delay: 500
        onClicked: bar.setTime(0)
    }
    // (a disabled button gets no hover: its tooltip says why)
    Item {
        anchors.verticalCenter: parent.verticalCenter
        implicitWidth: storeButton.implicitWidth
        implicitHeight: storeButton.implicitHeight
        AppButton {
            id: storeButton
            anchors.fill: parent
            implicitHeight: 28
            iconGlyph: Theme.iconSave
            text: bar.compact ? "" : qsTr("Store in the cue")
            enabled: bar.changed && bar.ownTimes
            onClicked: bar.store()
        }
        HoverHandler { id: storeHover }
        ToolTip.text: (bar.compact ? qsTr("Store in the cue") + "\n" : "")
                      + (bar.changed && !bar.ownTimes ? qsTr("This cue runs with a BACK time or a fade-in, not its own times")
                                                      : qsTr("Keep this time in the cue"))
        ToolTip.visible: storeHover.hovered
        ToolTip.delay: 500
    }
}
