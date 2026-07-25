pragma Singleton
import QtQuick

// Icon glyphs — Material Icons first; Font Awesome only where Material has no match
// (guitar f7a6, drum f569). Emoji for sax.
QtObject {
    id: root

    readonly property string family: materialLoader.status === FontLoader.Ready
                                     ? materialLoader.name
                                     : "Material Icons"
    readonly property string faFamily: faLoader.status === FontLoader.Ready
                                       ? faLoader.name
                                       : "Font Awesome 6 Free"
    readonly property bool loaded: materialLoader.status === FontLoader.Ready

    property FontLoader materialLoader: FontLoader {
        source: "qrc:/qt/qml/FAEditor/resources/fonts/MaterialIcons-Regular.ttf"
    }
    property FontLoader faLoader: FontLoader {
        source: "qrc:/qt/qml/FAEditor/resources/fonts/fa-solid-900.ttf"
    }

    // —— UI (Material) ——
    readonly property string download: "\uf090"
    readonly property string upload: "\uf09b"
    readonly property string save: "\ue161"
    readonly property string add: "\ue145"            // add — new library file
    readonly property string copy: "\ue14d"
    readonly property string trash: "\ue872"
    readonly property string refresh: "\ue5d5"
    readonly property string edit: "\ue3c9"
    readonly property string search: "\ue8b6"
    readonly property string folder: "\ue2c7"
    readonly property string star: "\ue838"
    readonly property string starEmpty: "\ue83a"
    readonly property string exchange: "\ue8d4"
    readonly property string check: "\ue86c"
    readonly property string circle: "\ue061"          // fiber_manual_record — MIDI LED
    readonly property string cable: "\uefe6"           // MIDI / connection
    readonly property string chevronRight: "\ue5cc"    // disclosure / navigate next
    readonly property string chevronUp: "\ue5ce"       // expand_less — collapse panel
    readonly property string close: "\ue5cd"           // close

    // —— Instruments (Material) ——
    readonly property string piano: "\ue521"
    readonly property string music: "\ue405"           // music_note
    readonly property string microphone: "\ue029"
    readonly property string headphones: "\uf01f"
    readonly property string equalizer: "\ue01d"
    readonly property string tune: "\ue429"
    readonly property string graphicEq: "\ue1b8"
    readonly property string bolt: "\uea0b"
    readonly property string sparkle: "\ue65f"         // auto_awesome — FX
    readonly property string bell: "\ue7f4"            // notifications
    readonly property string album: "\ue019"
    readonly property string waves: "\ue176"
    readonly property string nightlife: "\uea62"       // wind/brass / sax (monochrome)
    readonly property string libraryMusic: "\ue030"
    readonly property string saxophone: nightlife      // same tintable Material glyph (emoji can't be recolored)

    // —— FA-only instruments (not in Material) ——
    readonly property string guitar: "\uf7a6"
    readonly property string drum: "\uf569"

    function fontForGlyph(glyph) {
        if (glyph === guitar || glyph === drum)
            return faFamily
        return family
    }

    function forCategory(category) {
        const c = (category || "").toLowerCase()
        if (c === "favorites")
            return star
        if (c.indexOf("guitar") >= 0 || c.indexOf("plucked") >= 0)
            return guitar
        if (c.indexOf("bass") >= 0)
            return waves
        if (c.indexOf("drum") >= 0 || c.indexOf("perc") >= 0 || c.indexOf("beat") >= 0 || c === "hit")
            return drum
        if (c.indexOf("piano") >= 0 || c.indexOf("clav") >= 0 || c.indexOf("harpsi") >= 0
                || c.indexOf("celesta") >= 0 || c.indexOf("keyboard") >= 0 || c.indexOf("organ") >= 0)
            return piano
        if (c.indexOf("sax") >= 0)
            return saxophone
        if (c.indexOf("vox") >= 0 || c.indexOf("choir") >= 0 || c === "scat" || c.indexOf("mic") >= 0)
            return microphone
        if (c.indexOf("fx") >= 0 || c.indexOf("pulsat") >= 0)
            return sparkle
        if (c.indexOf("synth") >= 0 || c.indexOf("pad") >= 0 || c.indexOf("seq") >= 0)
            return equalizer
        if (c.indexOf("bell") >= 0 || c.indexOf("mallet") >= 0)
            return bell
        if (c.indexOf("brass") >= 0 || c.indexOf("wind") >= 0
                || c.indexOf("flute") >= 0 || c.indexOf("recorder") >= 0 || c.indexOf("harmonica") >= 0)
            return nightlife
        if (c.indexOf("string") >= 0 || c.indexOf("orchestr") >= 0 || c.indexOf("ensemble") >= 0)
            return libraryMusic
        return music
    }
}
