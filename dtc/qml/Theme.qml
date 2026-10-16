/*
  DMX Total Control
  Theme.qml

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

pragma Singleton
import QtQuick

QtObject {
    // Surfaces: a dark, low glare palette for dark venues
    readonly property color background: "#0e1014"
    readonly property color surface: "#161920"
    readonly property color surfaceRaised: "#1d212a"
    readonly property color surfaceHover: "#252a35"
    readonly property color border: "#2b303c"
    readonly property color borderStrong: "#3a4150"

    readonly property color text: "#e7e9ef"
    readonly property color textDim: "#9098ab"
    readonly property color textFaint: "#5d6475"

    readonly property color accent: "#3d8bfd"
    readonly property color accentText: "#ffffff"
    readonly property color danger: "#e5484d"
    readonly property color success: "#30a46c"
    readonly property color warning: "#f5a524"

    readonly property int radius: 8
    readonly property int radiusSmall: 5
    readonly property int spacing: 8
    readonly property int padding: 12

    readonly property int fontSmall: 11
    readonly property int fontNormal: 13
    readonly property int fontLarge: 16
    readonly property int fontTitle: 20

    readonly property string fontFamily: "Segoe UI"
    readonly property string monoFamily: monoLoader.name
    readonly property string iconFamily: iconLoader.name

    readonly property FontLoader iconLoader: FontLoader { source: "qrc:/FontAwesome7-Free-Solid-900.otf" }
    readonly property FontLoader monoLoader: FontLoader { source: "qrc:/RobotoMono-Regular.ttf" }

    // FontAwesome 7 solid glyphs
    readonly property string iconBolt: ""
    readonly property string iconCheck: ""
    readonly property string iconCircle: ""
    readonly property string iconEraser: ""
    readonly property string iconFile: ""
    readonly property string iconSave: ""
    readonly property string iconOpen: ""
    readonly property string iconGear: ""
    readonly property string iconGrip: ""
    readonly property string iconLayers: ""
    readonly property string iconLight: ""
    readonly property string iconList: ""
    readonly property string iconSearch: ""
    readonly property string iconPalette: ""
    readonly property string iconPen: ""
    readonly property string iconPlay: ""
    readonly property string iconPlug: ""
    readonly property string iconPlus: "+"
    readonly property string iconPower: ""
    readonly property string iconRefresh: ""
    readonly property string iconSliders: ""
    readonly property string iconStop: ""
    readonly property string iconGrid: ""
    readonly property string iconTrash: ""
    readonly property string iconMove: ""
    readonly property string iconClose: ""
    readonly property string iconSelect: ""
    readonly property string iconNetwork: ""
    readonly property string iconRecord: ""
    readonly property string iconPause: ""
    readonly property string iconLock: "\uf023"
    readonly property string iconUnlock: "\uf09c"
    readonly property string iconExpand: "\uf065"
    readonly property string iconCompress: "\uf066"
    readonly property string iconDownload: "\uf019"
    readonly property string iconImport: "\uf56f"
    readonly property string iconFolder: "\uf07b"
    readonly property string iconFolderOpen: "\uf07c"
    readonly property string iconFolderPlus: "\uf65e"
    readonly property string iconLink: "\uf0c1"
    readonly property string iconCopy: "\uf0c5"
    readonly property string iconStar: "\uf005"
    readonly property string iconWand: "\uf72b"
    readonly property string iconTarget: "\uf05b"
    readonly property string iconTable: "\uf00a"
    readonly property string iconMatrix: "\uf43c"
    readonly property string iconLasso: "\uf5ee"
    readonly property string iconRotate: "\uf2f9"
    readonly property string iconAlign: "\uf036"
    readonly property string iconRuler: "\uf546"
    readonly property string iconZoomIn: "\uf00e"
    readonly property string iconZoomOut: "\uf010"
    readonly property string iconEye: "\uf06e"
    readonly property string iconShapes: "\uf61f"
    readonly property string iconChevronRight: "\uf054"
    readonly property string iconChevronDown: "\uf078"
    readonly property string iconLocate: "\uf601"
    readonly property string iconSun: "\uf185"
    readonly property string iconCloud: "\uf0c2"
    readonly property string iconKeyboard: "\uf11c"
    readonly property string iconTerminal: "\uf120"
    readonly property string iconLightbulb: "\uf0eb"
    readonly property string iconStepForward: "\uf051"
    readonly property string iconStepBackward: "\uf048"
    readonly property string iconArrowsH: ""
    readonly property string iconArrowsV: ""

    /** Pick black or white text for a given background */
    function textOn(bg) {
        let c = Qt.color(bg)
        let luma = 0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b
        return luma > 0.6 ? "#111111" : "#ffffff"
    }

    /** The figures of the pan/tilt FX, in the order of the engine */
    readonly property var fxFigureNames: [qsTr("Circle"), qsTr("Eight"), qsTr("Square"), qsTr("Triangle"),
                                          qsTr("Diamond"), qsTr("Pan line"), qsTr("Tilt line"), qsTr("Diagonal"),
                                          qsTr("Leaf"), qsTr("Lissajous"), qsTr("Ballyhoo"), qsTr("Cross")]

    /** The name of an FX parameter: its own for the common ones, else the channel name */
    function fxParamLabel(key, name) {
        switch (key) {
        case "pantilt": return qsTr("Pan / tilt")
        case "pan": return qsTr("Pan")
        case "tilt": return qsTr("Tilt")
        case "dimmer": return qsTr("Dimmer")
        case "red": return qsTr("Red")
        case "green": return qsTr("Green")
        case "blue": return qsTr("Blue")
        case "white": return qsTr("White")
        case "amber": return qsTr("Amber")
        case "uv": return qsTr("UV")
        case "cyan": return qsTr("Cyan")
        case "magenta": return qsTr("Magenta")
        case "yellow": return qsTr("Yellow")
        case "lime": return qsTr("Lime")
        case "indigo": return qsTr("Indigo")
        default:
            if (name && name.length > 0)
                return name
            return key.length > 0 ? key.charAt(0).toUpperCase() + key.slice(1) : key
        }
    }

    /** Colour for a function type badge */
    /** User facing name of a function type (its scene type) */
    function typeLabel(type) {
        switch (type) {
        case "Scene": return qsTr("Static")
        case "Sequence": return qsTr("Steps")
        case "EFX": return qsTr("Move FX")
        case "RGBMatrix": return qsTr("Colour FX")
        case "CurveFX": return qsTr("Curve FX")
        case "FXScene": return qsTr("FX")
        case "Collection": return qsTr("Effect rack")
        case "Chaser": return qsTr("Chaser")
        case "Show": return qsTr("Super scene")
        case "Cuelist": return qsTr("Cuelist")
        case "Audio": return qsTr("Audio")
        default: return type
        }
    }

    function functionColor(type) {
        switch (type) {
        case "Scene": return "#3d8bfd"
        case "Chaser": return "#f59f00"
        case "Sequence": return "#f76707"
        case "EFX": return "#ae3ec9"
        case "RGBMatrix": return "#2fb344"
        case "CurveFX": return "#e8590c"
        case "FXScene": return "#be4bdb"
        case "Collection": return "#17a2b8"
        case "Show": return "#d6336c"
        case "Cuelist": return "#f03e3e"
        case "Audio": case "Video": return "#74b816"
        default: return "#868e96"
        }
    }
}
