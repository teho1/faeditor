import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

/**
 * Modulation family (Phaser / Tremolo / Auto Pan / Chorus / Flanger / …).
 * LFO viz reads via paramValues (NOTIFY mfxChanged); faders write via paramList.
 */
ColumnLayout {
    id: root
    spacing: 8

    required property MfxModel mfx
    property bool editorEnabled: true

    function p(name) { return root.mfx ? root.mfx.paramIndexByName(name) : -1 }

    /** NOTIFY tick — any binding that reads this re-evaluates on mfxChanged. */
    readonly property var paramValues: root.mfx ? root.mfx.paramValues : []

    readonly property int rateIdx: {
        const names = ["Rate", "Chorus Rate", "Tremolo Rate", "Speed"]
        for (let i = 0; i < names.length; ++i) {
            const idx = root.p(names[i])
            if (idx >= 0) return idx
        }
        return -1
    }
    readonly property int depthIdx: {
        const names = ["Depth", "Chorus Depth", "Tremolo Depth"]
        for (let i = 0; i < names.length; ++i) {
            const idx = root.p(names[i])
            if (idx >= 0) return idx
        }
        return -1
    }
    readonly property int waveIdx: {
        const names = ["Mod Wave", "Waveform", "Shape", "LFO Waveform", "LFO Shape"]
        for (let i = 0; i < names.length; ++i) {
            const idx = root.p(names[i])
            if (idx >= 0) return idx
        }
        return -1
    }

    /**
     * Map an MFX param into 0–127 for LfoWaveformGraph.
     * Catalog often omits Rate min/max → DefaultMin/Max (−20000…20000); for those,
     * treat values as Roland Rate (0–127 Hz, notes above → clamp to 127).
     */
    function graphNorm(index, defVal) {
        const values = root.paramValues
        if (!root.mfx || index < 0)
            return defVal
        const v = Number(values[index])
        const lo = root.mfx.paramMin(index)
        const hi = root.mfx.paramMax(index)
        if (!isFinite(v))
            return defVal
        if (hi - lo > 1000) {
            if (v < 0)
                return 0
            return Math.round(Math.max(0, Math.min(127, v)))
        }
        if (v < lo || v > hi)
            return defVal
        if (hi <= lo)
            return defVal
        return Math.round(((v - lo) / (hi - lo)) * 127)
    }

    readonly property int graphRate: root.graphNorm(root.rateIdx, 64)
    readonly property int graphDepth: root.graphNorm(root.depthIdx, 64)

    readonly property int graphShape: {
        const values = root.paramValues
        if (!root.mfx || root.waveIdx < 0)
            return 1
        const lo = root.mfx.paramMin(root.waveIdx)
        const hi = root.mfx.paramMax(root.waveIdx)
        const v = Number(values[root.waveIdx])
        if (!isFinite(v) || v < lo || v > hi)
            return 1
        return Math.round(v - lo)
    }

    readonly property string graphShapeName: {
        const _ = root.paramValues
        if (!root.mfx || root.waveIdx < 0)
            return "SIN"
        if (root.mfx.paramHasEnum(root.waveIdx)) {
            const names = root.mfx.paramEnumNames(root.waveIdx)
            const idx = root.graphShape
            return names[Math.max(0, Math.min(names.length - 1, idx))] || "SIN"
        }
        // FA Tremolo / Auto Pan Mod Wave fallback: TRI SQR SIN SAW1 SAW2
        const waves = ["TRI", "SQR", "SIN", "SAW", "SAW2"]
        return waves[Math.max(0, Math.min(waves.length - 1, root.graphShape))] || "SIN"
    }

    Label {
        text: (root.mfx ? root.mfx.typeName : "Modulation") + "  ·  LFO"
        color: LogicTheme.textPrimary
        font.bold: true
        font.pixelSize: LogicTheme.fontSize
    }

    LfoWaveformGraph {
        Layout.fillWidth: true
        Layout.preferredHeight: 110
        Layout.maximumHeight: 120
        enabled: root.editorEnabled
        shape: root.graphShape
        shapeName: root.graphShapeName
        rate: root.graphRate
        depth: root.graphDepth
        running: root.editorEnabled && root.visible
    }

    MfxParamGrid {
        Layout.fillWidth: true
        mfxModel: root.mfx
        enabled: root.editorEnabled
    }
}
