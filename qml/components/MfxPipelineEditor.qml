import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

/**
 * Multi-stage / Lo-Fi / Ring Mod / chain (A→B) pipeline editor.
 * Stage chips select which stage’s params are shown; Level/Balance stay shared.
 * Controls via paramList roles (NOTIFY); vertical faders for continuous params.
 */
ColumnLayout {
    id: root
    spacing: 8

    required property MfxModel mfx
    property bool editorEnabled: true

    property int selectedStageIndex: 0

    readonly property int mfxType: root.mfx ? root.mfx.type : -1
    readonly property var paramValues: root.mfx ? root.mfx.paramValues : []

    readonly property var stageLabels: {
        if (!root.mfx) return ["Effect"]
        const t = root.mfx.type
        if (t === 41)
            return ["Pre Filter / Comp", "Lo-Fi", "Post Filter", "2-Band EQ"]
        if (t === 42)
            return ["Bit Crusher", "2-Band EQ"]
        if (t === 15)
            return ["Carrier", "Ring Mod", "EQ"]
        const name = root.mfx.typeName || ""
        if (name.indexOf("→") > 0) {
            return name.replace(/^\d+\s*/, "").split("→").map(s => s.trim()).filter(s => s.length)
        }
        return [name.replace(/^\d+\s*/, "") || "Effect"]
    }

    readonly property bool multiStage: root.stageLabels.length > 1

    /** Shared output params always visible with any selected stage. */
    readonly property var sharedOutputNames: {
        const t = root.mfxType
        // LoFi / Bit Crusher / Ring Mod: Low/High Gain belong to EQ stage, not shared.
        if (t === 15 || t === 41 || t === 42)
            return ["Balance", "Level"]
        // OD/DS wah chains expose a post EQ without a dedicated stage chip.
        if (t === 51 || t === 52)
            return ["Low Gain", "High Gain", "Level"]
        return ["Level"]
    }

    readonly property var selectedStageRules: {
        if (!root.multiStage)
            return { prefixes: [], exact: [] }
        const label = root.stageLabels[root.selectedStageIndex] || ""
        return root.rulesForStage(label)
    }

    onMfxTypeChanged: selectedStageIndex = 0
    onStageLabelsChanged: {
        if (selectedStageIndex >= root.stageLabels.length)
            selectedStageIndex = Math.max(0, root.stageLabels.length - 1)
    }

    /**
     * Map stage chip label → param name match rules.
     * Catalog uses prefixes like "Distortion Drive", "Touch Wah Sens", "Pre Amp Sw".
     */
    function rulesForStage(label) {
        switch (label) {
        case "Overdrive":
            return { prefixes: ["Overdrive"], exact: [] }
        case "Distortion":
            return { prefixes: ["Distortion"], exact: [] }
        case "Chorus":
            return { prefixes: ["Chorus"], exact: [] }
        case "Flanger":
            return { prefixes: ["Flanger"], exact: [] }
        case "Delay":
            return { prefixes: ["Delay"], exact: ["Acceleration"] }
        case "Enhancer":
            return { prefixes: ["Enhancer"], exact: [] }
        case "OD/DS":
            // Drive Switch/Type/Drive, Tone, Amp Sw/Type — not "OD/DS …"
            return { prefixes: ["Drive", "Amp"], exact: ["Tone"] }
        case "TouchWah":
            return { prefixes: ["Touch Wah"], exact: [] }
        case "AutoWah":
            return { prefixes: ["Auto Wah"], exact: [] }
        case "GuitarAmpSim":
            return { prefixes: ["Pre Amp", "Speaker"], exact: [] }
        case "EP AmpSim":
            return { prefixes: ["OD"], exact: ["Type", "Bass", "Treble", "Speaker Type"] }
        case "Tremolo":
            return { prefixes: ["Tremolo"], exact: [] }
        case "Phaser":
            return { prefixes: ["Phaser"], exact: [] }
        case "Pre Filter / Comp":
            return { prefixes: ["Pre Filter"], exact: [] }
        case "Lo-Fi":
            return { prefixes: ["LoFi"], exact: [] }
        case "Post Filter":
            return { prefixes: ["Post Filter"], exact: [] }
        case "2-Band EQ":
        case "EQ":
            return { prefixes: [], exact: ["Low Gain", "High Gain"] }
        case "Bit Crusher":
            return { prefixes: [], exact: ["Sample Rate", "Bit Down", "Filter"] }
        case "Carrier":
            return { prefixes: [], exact: ["Frequency"] }
        case "Ring Mod":
            return { prefixes: [], exact: ["Sens", "Polarity"] }
        default:
            // Fallback: treat the chip label as a param-name prefix.
            return { prefixes: label.length ? [label] : [], exact: [] }
        }
    }

    /** Optional Switch / Sw param index for a stage (enable indicator + click-to-enable). */
    function stageSwitchIndex(stageIndex) {
        if (!root.mfx || !root.multiStage)
            return -1
        const label = root.stageLabels[stageIndex] || ""
        const rules = root.rulesForStage(label)
        const candidates = []
        if (label === "OD/DS")
            candidates.push("Drive Switch")
        else if (label === "GuitarAmpSim")
            candidates.push("Pre Amp Sw")
        else if (label === "EP AmpSim")
            candidates.push("OD Switch")
        const prefixes = rules.prefixes || []
        for (let i = 0; i < prefixes.length; ++i) {
            candidates.push(prefixes[i] + " Switch")
            candidates.push(prefixes[i] + " Sw")
        }
        for (let c = 0; c < candidates.length; ++c) {
            const idx = root.mfx.paramIndexByName(candidates[c])
            if (idx >= 0)
                return idx
        }
        return -1
    }

    function stageEnabled(stageIndex) {
        const idx = root.stageSwitchIndex(stageIndex)
        if (idx < 0)
            return true // no dedicated switch → treat as always on
        if (!root.mfx)
            return true
        const v = Number(root.paramValues[idx])
        const lo = root.mfx.paramMin(idx)
        // FA switches are typically 0=off, 1=on (or min/max).
        return v > lo
    }

    function stripStagePrefix(name) {
        const rules = root.selectedStageRules
        const prefixes = rules.prefixes || []
        for (let i = 0; i < prefixes.length; ++i) {
            const p = prefixes[i]
            if (name === p)
                return name
            if (name.indexOf(p + " ") === 0)
                return name.substring(p.length + 1)
        }
        return name
    }

    function selectStage(index) {
        if (index < 0 || index >= root.stageLabels.length)
            return
        root.selectedStageIndex = index
        // Selecting a stage shows its controls; if it has a Switch and is off,
        // turn it on so the stage is audibly enabled (FA chain semantics).
        const sw = root.stageSwitchIndex(index)
        if (sw >= 0 && root.mfx && !root.stageEnabled(index)) {
            const hi = root.mfx.paramMax(sw)
            root.mfx.paramList.setValue(sw, hi)
        }
    }

    Label {
        text: (root.mfx ? root.mfx.typeName : "Pipeline") + "  ·  stages"
        color: LogicTheme.textPrimary
        font.bold: true
        font.pixelSize: LogicTheme.fontSize
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 6
        Repeater {
            model: root.stageLabels
            RowLayout {
                required property int index
                required property string modelData
                spacing: 6

                readonly property bool selected: index === root.selectedStageIndex
                readonly property bool enabledStage: root.stageEnabled(index)

                Rectangle {
                    id: chip
                    Layout.preferredHeight: 28
                    Layout.preferredWidth: Math.max(64, stageLabel.implicitWidth + 20)
                    radius: 6
                    color: parent.selected
                           ? (LogicTheme.dark ? "#2E3540" : "#DCE6F5")
                           : (LogicTheme.dark ? "#2A2A2E" : "#E4E4EA")
                    border.color: parent.selected ? LogicTheme.accent : LogicTheme.hairline
                    border.width: parent.selected ? 1.5 : 1
                    opacity: parent.enabledStage || parent.selected ? 1.0 : 0.55

                    Label {
                        id: stageLabel
                        anchors.centerIn: parent
                        text: modelData
                        color: chip.parent.selected ? LogicTheme.accent : LogicTheme.textPrimary
                        font.pixelSize: LogicTheme.fontSizeSmall
                        font.bold: chip.parent.selected
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        enabled: root.editorEnabled
                        onClicked: root.selectStage(index)
                    }
                }
                Label {
                    visible: index < root.stageLabels.length - 1
                    text: "→"
                    color: LogicTheme.textMuted
                }
            }
        }
        Item { Layout.fillWidth: true }
    }

    Label {
        visible: root.mfx && root.mfx.type === 41
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: LogicTheme.textMuted
        font.pixelSize: LogicTheme.fontSizeSmall
        text: "LoFi Type is 1–9 (increasing degradation)."
    }

    MfxParamGrid {
        Layout.fillWidth: true
        mfxModel: root.mfx
        enabled: root.editorEnabled
        showNamePrefixes: root.multiStage ? (root.selectedStageRules.prefixes || []) : []
        showExactNames: root.multiStage ? (root.selectedStageRules.exact || []) : []
        alwaysShowNames: root.multiStage ? root.sharedOutputNames : []
        formatLabel: root.multiStage ? ((name) => root.stripStagePrefix(name)) : null
    }
}
