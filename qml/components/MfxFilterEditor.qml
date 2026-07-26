import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

/**
 * Filter family (4 Step Filter, 5 Enhancer, 6 Auto Wah).
 * Graph + faders read via paramValues (NOTIFY mfxChanged); write via setParamValue / paramList.
 */
ColumnLayout {
    id: root
    spacing: 8

    // One-way from parent only — never bind mfx: mfx.
    required property MfxModel mfx
    property bool editorEnabled: true

    function p(name) { return root.mfx ? root.mfx.paramIndexByName(name) : -1 }

    readonly property int cutoffIdx: {
        const candidates = ["Manual", "Cutoff Freq", "Cutoff", "Post Filter Cutoff", "Frequency"]
        for (let i = 0; i < candidates.length; ++i) {
            const idx = root.p(candidates[i])
            if (idx >= 0) return idx
        }
        return -1
    }
    readonly property int resIdx: {
        const candidates = ["Peak", "Filter Resonance", "Resonance", "Sens"]
        for (let i = 0; i < candidates.length; ++i) {
            const idx = root.p(candidates[i])
            if (idx >= 0) return idx
        }
        return -1
    }
    readonly property int typeIdx: root.p("Filter Type")

    /** NOTIFY tick — any binding that reads this re-evaluates on mfxChanged. */
    readonly property var paramValues: root.mfx ? root.mfx.paramValues : []

    function saneVal(index, defVal) {
        if (!root.mfx || index < 0)
            return defVal
        const v = Number(root.paramValues[index])
        const lo = root.mfx.paramMin(index)
        const hi = root.mfx.paramMax(index)
        if (v < lo || v > hi)
            return defVal
        return v
    }

    function toNorm127(index, defVal) {
        if (!root.mfx || index < 0)
            return defVal
        const lo = root.mfx.paramMin(index)
        const hi = root.mfx.paramMax(index)
        const v = saneVal(index, Math.round((lo + hi) / 2))
        if (hi <= lo)
            return defVal
        return Math.round(((v - lo) / (hi - lo)) * 127)
    }

    function fromNorm127(index, n) {
        if (!root.mfx || index < 0)
            return
        const lo = root.mfx.paramMin(index)
        const hi = root.mfx.paramMax(index)
        root.mfx.setParamValue(index, Math.round(lo + (Math.max(0, Math.min(127, n)) / 127) * (hi - lo)))
    }

    RowLayout {
        Layout.fillWidth: true
        Label {
            text: (root.mfx ? root.mfx.typeName : "Filter") + "  ·  frequency"
            color: LogicTheme.textPrimary
            font.bold: true
            font.pixelSize: LogicTheme.fontSize
            Layout.fillWidth: true
        }
        Label {
            text: root.cutoffIdx >= 0 ? "Drag graph or edit below" : "Edit parameters below"
            color: LogicTheme.textMuted
            font.pixelSize: LogicTheme.fontSizeSmall
        }
    }

    FilterResponseGraph {
        Layout.fillWidth: true
        Layout.preferredHeight: 130
        Layout.maximumHeight: 140
        visible: root.cutoffIdx >= 0
        enabled: root.editorEnabled && root.cutoffIdx >= 0
        cutoff: root.toNorm127(root.cutoffIdx, 64)
        resonance: root.toNorm127(root.resIdx, 40)
        filterTypeName: {
            // Touch paramValues for NOTIFY
            const _ = root.paramValues
            if (root.typeIdx >= 0 && root.mfx && root.mfx.paramHasEnum(root.typeIdx)) {
                const names = root.mfx.paramEnumNames(root.typeIdx)
                const lo = root.mfx.paramMin(root.typeIdx)
                const v = root.saneVal(root.typeIdx, lo) - lo
                return names[Math.max(0, Math.min(names.length - 1, v))] || "LPF"
            }
            // Auto Wah Filter Type is often 0=LPF / 1=BPF without enum names in catalog
            if (root.typeIdx >= 0 && root.mfx) {
                const v = root.saneVal(root.typeIdx, 0)
                return v > 0 ? "BPF" : "LPF"
            }
            return "LPF"
        }
        onCutoffChangedByUser: (v) => root.fromNorm127(root.cutoffIdx, v)
        onResonanceChangedByUser: (v) => root.fromNorm127(root.resIdx, v)
    }

    MfxParamGrid {
        Layout.fillWidth: true
        mfxModel: root.mfx
        enabled: root.editorEnabled
    }
}
