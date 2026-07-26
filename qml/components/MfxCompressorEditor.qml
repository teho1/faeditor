import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

/**
 * Compressor / Limiter / Gate: transfer curve + paramList-backed controls.
 * Graph reads via paramValues (NOTIFY mfxChanged); writes via paramList / setParamValue.
 */
ColumnLayout {
    id: root
    spacing: 8

    required property MfxModel mfx
    property bool editorEnabled: true

    function p(name) { return root.mfx ? root.mfx.paramIndexByName(name) : -1 }

    /** NOTIFY tick — any binding that reads this re-evaluates on mfxChanged. */
    readonly property var paramValues: root.mfx ? root.mfx.paramValues : []

    function firstIdx(names) {
        for (let i = 0; i < names.length; ++i) {
            const idx = root.p(names[i])
            if (idx >= 0) return idx
        }
        return -1
    }

    readonly property int threshIdx: root.firstIdx(["Threshold", "Threshold Level"])
    readonly property int ratioIdx: root.firstIdx(["Ratio"])
    readonly property int attackIdx: root.firstIdx(["Attack"])
    readonly property int releaseIdx: root.firstIdx(["Release"])
    readonly property int holdIdx: root.firstIdx(["Hold"])
    readonly property int kneeIdx: root.firstIdx(["Knee", "Soft Knee"])
    readonly property int makeupIdx: root.firstIdx(["Post Gain", "Output", "Makeup", "Gain"])
    readonly property int levelIdx: root.firstIdx(["Level"])
    readonly property int balanceIdx: root.firstIdx(["Balance"])
    readonly property int modeIdx: root.firstIdx(["Mode"])

    readonly property bool isGate: {
        const _ = root.paramValues
        const n = root.mfx ? String(root.mfx.typeName).toLowerCase() : ""
        return n.indexOf("gate") >= 0
    }

    function saneAt(index, defVal) {
        if (!root.mfx || index < 0) return defVal
        const v = Number(root.paramValues[index])
        const lo = root.mfx.paramMin(index)
        const hi = root.mfx.paramMax(index)
        if (v < lo || v > hi) return defVal
        return v
    }

    function toNorm127(index, defVal) {
        if (!root.mfx || index < 0) return defVal
        const lo = root.mfx.paramMin(index)
        const hi = root.mfx.paramMax(index)
        const v = saneAt(index, Math.round((lo + hi) / 2))
        if (hi <= lo) return defVal
        return Math.round(((v - lo) / (hi - lo)) * 127)
    }

    Label {
        text: (root.mfx ? root.mfx.typeName : "Compressor") + "  ·  dynamics"
        color: LogicTheme.textPrimary
        font.bold: true
        font.pixelSize: LogicTheme.fontSize
    }

    CompressorTransferGraph {
        Layout.fillWidth: true
        Layout.preferredHeight: 120
        Layout.maximumHeight: 130
        enabled: root.editorEnabled
        // Bound props — each reads paramValues so fader moves repaint immediately
        threshold: root.toNorm127(root.threshIdx, 64)
        knee: {
            if (root.kneeIdx >= 0)
                return root.toNorm127(root.kneeIdx, 0)
            // Approximate soft knee from Attack when catalog has no Knee
            return root.attackIdx >= 0 ? Math.round(root.toNorm127(root.attackIdx, 40) * 0.35) : 0
        }
        makeup: root.toNorm127(root.makeupIdx, 64)
        level: root.toNorm127(root.levelIdx, 100)
        attack: root.toNorm127(root.attackIdx, 40)
        release: root.toNorm127(root.releaseIdx, 64)
        hold: root.toNorm127(root.holdIdx, 0)
        balance: {
            if (root.balanceIdx < 0 || !root.mfx) return 100
            return Math.round(root.saneAt(root.balanceIdx, 100))
        }
        hasRatio: root.ratioIdx >= 0
        ratioRaw: root.ratioIdx >= 0 ? root.saneAt(root.ratioIdx, 0) : 0
        ratioMax: root.ratioIdx >= 0 && root.mfx ? root.mfx.paramMax(root.ratioIdx) : 3
        isGate: root.isGate
        gateMode: root.modeIdx >= 0 ? Math.round(root.saneAt(root.modeIdx, 0)) : 0
        running: root.editorEnabled && root.visible
    }

    MfxParamGrid {
        Layout.fillWidth: true
        mfxModel: root.mfx
        enabled: root.editorEnabled
    }
}
