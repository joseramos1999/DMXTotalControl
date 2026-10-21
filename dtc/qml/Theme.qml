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
import DMXTotalControl.Backend

QtObject {
    id: theme

    /** The theme of Options: "dark", "light", "contrast" or "night" */
    readonly property string name: Core.settings.theme
    readonly property var palette: palettes[name] || palettes.dark

    // The colours of each theme. The remote control has the same ones
    // (dtc/remote/remote.css)
    readonly property var palettes: ({
        // a low glare palette for dark venues
        dark: {
            dark: true,
            background: "#0e1014", surface: "#161920", surfaceRaised: "#1d212a", surfaceHover: "#252a35",
            border: "#2b303c", borderStrong: "#3a4150",
            text: "#e7e9ef", textDim: "#a3aaba", textFaint: "#848997",
            accent: "#3d8bfd", accentText: "#ffffff", danger: "#e5484d", success: "#30a46c", warning: "#f5a524",
            learn: "#ae3ec9", learnText: "#d68ee8",
            knob: "#d9dce3", knobBorder: "#ffffff", shadow: "#60000000", scrim: "#70000000",
            plan: "#0b0d12", planGrid: "#12151b", planLine: "#262c38", dialogShade: "#28282a",
            categoryLevel: 1.0
        },
        // offices, rehearsals by day, outdoors
        light: {
            dark: false,
            background: "#eceef2", surface: "#ffffff", surfaceRaised: "#f3f4f7", surfaceHover: "#e4e7ed",
            border: "#d2d7e0", borderStrong: "#b2b9c7",
            text: "#171b23", textDim: "#4b5364", textFaint: "#5b6171",
            accent: "#1a62cc", accentText: "#ffffff", danger: "#c32929", success: "#167546", warning: "#965700",
            learn: "#9c36b5", learnText: "#7b2a91",
            knob: "#ffffff", knobBorder: "#7d8598", shadow: "#30000000", scrim: "#66000000",
            plan: "#f6f7f9", planGrid: "#fafbfc", planLine: "#cfd5df", dialogShade: "#28282a",
            categoryLevel: 1.0
        },
        // pure black and white, marked edges: low vision, a lot of light around
        contrast: {
            dark: true,
            background: "#000000", surface: "#000000", surfaceRaised: "#101010", surfaceHover: "#2a2a2a",
            border: "#ffffff", borderStrong: "#ffffff",
            text: "#ffffff", textDim: "#f0f0f0", textFaint: "#c8c8c8",
            accent: "#ffd400", accentText: "#000000", danger: "#ff5c5c", success: "#4ade80", warning: "#ff9f1a",
            learn: "#ff6bff", learnText: "#ff9cff",
            knob: "#ffffff", knobBorder: "#ffd400", shadow: "#a0000000", scrim: "#b0000000",
            plan: "#000000", planGrid: "#0c0c0c", planLine: "#808080", dialogShade: "#000000",
            categoryLevel: 1.0,
            // the text on a colour: the one of the two that reads best
            textOnLight: "#000000", textOnDark: "#ffffff"
        },
        // very dark and red: a dark booth, without dazzling or losing the night sight
        night: {
            dark: true,
            background: "#050202", surface: "#0b0505", surfaceRaised: "#120808", surfaceHover: "#1b0b0b",
            border: "#271111", borderStrong: "#3b1818",
            text: "#e78075", textDim: "#c4726a", textFaint: "#ab6b64",
            accent: "#d84a3d", accentText: "#1a0505", danger: "#ff3b2f", success: "#b4652a", warning: "#d9822b",
            learn: "#a3315f", learnText: "#c4507e",
            knob: "#8a2c22", knobBorder: "#b8392c", shadow: "#a0000000", scrim: "#a0000000",
            plan: "#030101", planGrid: "#0a0404", planLine: "#2a1010", dialogShade: "#000000",
            categoryLevel: 0.45,
            textOnLight: "#1a0505", textOnDark: "#e0a59c"
        }
    })

    /** A dark theme (dark, contrast, night): for the few things drawn on it */
    readonly property bool isDark: palette.dark

    // Surfaces
    readonly property color background: palette.background
    readonly property color surface: palette.surface
    readonly property color surfaceRaised: palette.surfaceRaised
    readonly property color surfaceHover: palette.surfaceHover
    readonly property color border: palette.border
    readonly property color borderStrong: palette.borderStrong

    readonly property color text: palette.text
    readonly property color textDim: palette.textDim
    readonly property color textFaint: palette.textFaint

    readonly property color accent: palette.accent
    readonly property color accentText: palette.accentText
    /** The accent behind text (a chip, a selected fixture, a switch on): as
     *  dark as accentText needs to read on it */
    readonly property color accentFill: fill(accent)
    readonly property color danger: palette.danger
    readonly property color success: palette.success
    readonly property color warning: palette.warning

    /** Learn mode of the mappings, and what can be mapped */
    readonly property color learn: palette.learn
    readonly property color learnText: palette.learnText
    /** The handles of the faders and sliders (knobBorder: one being dragged, its bright edge) */
    readonly property color knob: palette.knob
    readonly property color knobBorder: palette.knobBorder
    readonly property color shadow: palette.shadow
    /** What covers the page behind a dialog, or a disabled area */
    readonly property color scrim: palette.scrim
    /** The 2D plan of the patch and the programmer */
    readonly property color plan: palette.plan
    readonly property color planGrid: palette.planGrid
    readonly property color planLine: palette.planLine
    /** What dims the page behind a dialog (the controls use it at 50 %): no grey veil on black or at night */
    readonly property color dialogShade: palette.dialogShade

    /** A colour that means a kind of thing (a scene type, a group): dimmed in the night theme */
    function category(c) {
        const level = palette.categoryLevel
        if (level >= 1.0)
            return c
        const k = Qt.color(c)
        return Qt.rgba(k.r * level, k.g * level, k.b * level, k.a)
    }

    /** A colour between a and b (t from 0, a, to 1, b): the shades between two tokens */
    function mix(a, b, t) {
        const x = Qt.color(a), y = Qt.color(b)
        return Qt.rgba(x.r + (y.r - x.r) * t, x.g + (y.g - x.g) * t, x.b + (y.b - x.b) * t, x.a + (y.a - x.a) * t)
    }

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
    readonly property string iconSaveAs: "\uf56e"
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
    readonly property string iconChevronLeft: "\uf053"
    readonly property string iconChevronUp: "\uf077"
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
        // high contrast and night: of their two text colours, the one with
        // the most contrast (WCAG): black on the yellow, no white glare at night
        if (palette.textOnLight !== undefined) {
            const l = luminance(c)
            const dark = luminance(Qt.color(palette.textOnLight))
            const light = luminance(Qt.color(palette.textOnDark))
            return (l + 0.05) / (dark + 0.05) >= (light + 0.05) / (l + 0.05) ? palette.textOnLight : palette.textOnDark
        }
        // white while it reads (4.5, WCAG), else the one that reads best
        const l = luminance(c)
        const onWhite = 1.05 / (l + 0.05), onBlack = (l + 0.05) / (luminance(Qt.color("#111111")) + 0.05)
        return onWhite >= 4.5 || onWhite >= onBlack ? "#ffffff" : "#111111"
    }

    /** A colour to fill behind text (a lit button, a scene playing): itself,
     *  or just as much darker as white text needs to read on it (4.5); light
     *  colours (yellow, white) keep their dark text and stay as they are */
    function fill(c) {
        let col = Qt.color(c)
        // the dark text (high contrast and night: theirs) where it reads, else
        // the light one, on the colour made darker until it does
        const dark = Qt.color(palette.textOnLight !== undefined ? palette.textOnLight : "#111111")
        const light = Qt.color(palette.textOnDark !== undefined ? palette.textOnDark : "#ffffff")
        if (contrast(dark, col) >= (palette.textOnLight !== undefined ? 4.5 : 7))
            return col
        for (let i = 0; i < 60 && contrast(light, col) < 4.5; i++)
            col = Qt.darker(col, 1.03)
        return col
    }

    /** The text colour over a background that varies (a level): the normal text while it reads */
    function textOver(bg) {
        return readableOn(text, bg)
    }

    /** Text of colour $c over $bg: itself while it reads (4.5, WCAG), else the text colour that does */
    function readableOn(c, bg) {
        return contrast(c, bg) >= 4.5 ? c : textOn(bg)
    }

    /** A colour for text or thin lines on a background: itself, or towards the text colour where it doesn't read (yellow on a light theme) */
    function readable(c, bg) {
        return contrast(c, bg === undefined ? surface : bg) >= 3.2 ? c : mix(c, text, 0.55)
    }

    /** Text over a dark spot (a shadow, a scrim, a dark tint): white, without glare at night */
    readonly property color textOnShade: textOn("#000000")

    /** How bright the colours of the lights show (swatches, wheels, colour
     *  bars): dimmed in the night theme, as the categories */
    readonly property real lightLevel: palette.categoryLevel

    /** A number typed in a field, with a point or a comma whatever the
     *  language, without its unit ("2,5 s", "1.5 m"); $fallback if none */
    function parseNumber(text, fallback) {
        const n = parseFloat(String(text).replace(",", ".").replace(/[^0-9.-]/g, ""))
        return isNaN(n) ? fallback : n
    }

    /** The contrast ratio of two colours (WCAG): 1 the same, 21 black on white */
    function contrast(a, b) {
        const x = luminance(Qt.color(a)), y = luminance(Qt.color(b))
        return (Math.max(x, y) + 0.05) / (Math.min(x, y) + 0.05)
    }

    /** An edge for a colour that doesn't stand out from the background (a white swatch on a light theme) */
    function edgeFor(c, bg) {
        return contrast(c, bg === undefined ? surface : bg) < 1.6 ? borderStrong : "transparent"
    }

    /** The edge of a handle, where it doesn't stand out (light theme) */
    readonly property color knobEdge: contrast(knob, surface) < 1.6 ? knobBorder : "transparent"

    /** The relative luminance of a colour (WCAG) */
    function luminance(c) {
        const lin = (v) => v <= 0.03928 ? v / 12.92 : Math.pow((v + 0.055) / 1.055, 2.4)
        return 0.2126 * lin(c.r) + 0.7152 * lin(c.g) + 0.0722 * lin(c.b)
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
        return category(functionBaseColor(type))
    }

    function functionBaseColor(type) {
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
