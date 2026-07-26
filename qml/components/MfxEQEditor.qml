import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

/**
 * EQ family (1 Equalizer, 2 Spectrum, 3 Low Boost).
 * Graph is the primary editor (FabFilter-style); faders mirror the same params.
 *
 * Solo: UI-only (dims other bands / filters sum curve). No SysEx solo.
 * Bypass: soft bypass — writes gain to 0 dB (stash/restore). No bypass SysEx.
 */
ColumnLayout {
    id: root
    spacing: 8

    // One-way from parent only — never bind mfx: mfx (name-shadowing loop).
    required property MfxModel mfx
    property bool editorEnabled: true

    readonly property int mfxType: root.mfx ? root.mfx.type : -1
    readonly property int paramCount: root.mfx ? root.mfx.paramCount : 0

    /** NOTIFY tick — any binding that reads this re-evaluates on mfxChanged. */
    readonly property var paramValues: root.mfx ? root.mfx.paramValues : []

    /// UI-only solo (−1 = none).
    property int soloBand: -1
    /// Soft-bypass flags + stashed gains (parallel to eqGraph.bands).
    property var bypassFlags: []
    property var bypassStash: []

    // --- FA Parameter Guide discrete tables (type 1) ---
    readonly property var midFreqTable: [
        200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600,
        2000, 2500, 3150, 4000, 5000, 6300, 8000
    ]
    readonly property var lowFreqTable: [200, 400]
    readonly property var highFreqTable: [2000, 4000, 8000]
    readonly property var qTable: [0.5, 1.0, 2.0, 4.0, 8.0]
    readonly property var spectrumFreqs: [250, 500, 1000, 1250, 2000, 3150, 4000, 8000]

    readonly property color colLow: LogicTheme.success
    readonly property color colMid1: LogicTheme.soloYellow
    readonly property color colMid2: LogicTheme.warning
    readonly property color colHigh: LogicTheme.danger

    function shortLabel(name) {
        const m = name.match(/\(([^)]+)\)/)
        if (m && name.indexOf("Band") === 0)
            return m[1]
        return name
    }

    function sanitizeGain(v) {
        return MfxUiFamily.sanitizeGainDb(v)
    }

    function saneAt(index, defVal) {
        if (!root.mfx || index < 0 || index >= root.paramCount)
            return defVal
        const v = Number(root.paramValues[index])
        const lo = root.mfx.paramMin(index)
        const hi = root.mfx.paramMax(index)
        if (v < lo || v > hi)
            return defVal
        return v
    }

    function writeParam(index, value) {
        if (!root.mfx || index < 0 || index >= root.paramCount)
            return
        const lo = root.mfx.paramMin(index)
        const hi = root.mfx.paramMax(index)
        root.mfx.setParamValue(index, Math.round(Math.max(lo, Math.min(hi, value))))
    }

    function nearestIndex(table, hz) {
        let best = 0
        let bestD = Infinity
        for (let i = 0; i < table.length; ++i) {
            const d = Math.abs(Math.log(table[i]) - Math.log(Math.max(1, hz)))
            if (d < bestD) {
                bestD = d
                best = i
            }
        }
        return best
    }

    function qToIndex(q) {
        let best = 1
        let bestD = Infinity
        for (let i = 0; i < root.qTable.length; ++i) {
            const d = Math.abs(root.qTable[i] - q)
            if (d < bestD) {
                bestD = d
                best = i
            }
        }
        return best
    }

    function hzFromLowFreq(idx) {
        return root.lowFreqTable[Math.max(0, Math.min(root.lowFreqTable.length - 1, idx))]
    }
    function hzFromMidFreq(idx) {
        return root.midFreqTable[Math.max(0, Math.min(root.midFreqTable.length - 1, idx))]
    }
    function hzFromHighFreq(idx) {
        return root.highFreqTable[Math.max(0, Math.min(root.highFreqTable.length - 1, idx))]
    }
    function qFromIndex(idx) {
        return root.qTable[Math.max(0, Math.min(root.qTable.length - 1, idx))]
    }

    function bandColor(i, count) {
        if (count === 4) {
            if (i === 0) return root.colLow
            if (i === 1) return root.colMid1
            if (i === 2) return root.colMid2
            return root.colHigh
        }
        // Spectrum / other: green → yellow → orange → red
        const t = count <= 1 ? 0 : i / (count - 1)
        if (t < 0.33) return root.colLow
        if (t < 0.55) return root.colMid1
        if (t < 0.78) return root.colMid2
        return root.colHigh
    }

    readonly property int bandCount: {
        if (root.mfxType === 2) return 8
        if (root.mfxType === 3) return 3
        if (root.mfxType === 1) return 4
        return 0
    }

    function isBypassed(i) {
        return i >= 0 && i < root.bypassFlags.length && !!root.bypassFlags[i]
    }

    function setBypassFlag(i, on, stashGain) {
        const n = Math.max(root.bandCount, i + 1)
        const flags = []
        const stash = []
        for (let k = 0; k < n; ++k) {
            flags.push(k < root.bypassFlags.length ? !!root.bypassFlags[k] : false)
            stash.push(k < root.bypassStash.length ? Number(root.bypassStash[k]) : 0)
        }
        flags[i] = !!on
        if (on)
            stash[i] = stashGain
        root.bypassFlags = flags
        root.bypassStash = stash
    }

    function syncBypassArrays() {
        const n = root.bandCount
        if (n <= 0) {
            root.bypassFlags = []
            root.bypassStash = []
            return
        }
        if (root.bypassFlags.length === n && root.bypassStash.length === n)
            return
        const flags = []
        const stash = []
        for (let i = 0; i < n; ++i) {
            flags.push(i < root.bypassFlags.length ? !!root.bypassFlags[i] : false)
            stash.push(i < root.bypassStash.length ? Number(root.bypassStash[i]) : 0)
        }
        root.bypassFlags = flags
        root.bypassStash = stash
    }

    onBandCountChanged: syncBypassArrays()
    Component.onCompleted: syncBypassArrays()

    /** Build band descriptors for the graph from live paramValues. */
    readonly property var graphBands: {
        const _ = root.paramValues
        const type = root.mfxType
        if (!root.mfx || type < 1)
            return []

        if (type === 2) {
            // Spectrum: 8 fixed bands + shared Q
            const q = qFromIndex(saneAt(8, 1))
            const bands = []
            for (let i = 0; i < 8; ++i) {
                const gain = sanitizeGain(saneAt(i, 0))
                const solo = root.soloBand === i
                const dim = root.soloBand >= 0 && !solo
                bands.push({
                    label: i === 0 ? "250" : (root.spectrumFreqs[i] >= 1000
                           ? (root.spectrumFreqs[i] / 1000) + "k" : String(root.spectrumFreqs[i])),
                    color: bandColor(i, 8),
                    kind: "bell",
                    freqHz: root.spectrumFreqs[i],
                    gainDb: gain,
                    q: q,
                    hasQ: true,
                    hasFreq: false,
                    dimmed: dim,
                    bypassed: isBypassed(i),
                    solo: solo
                })
            }
            return bands
        }

        if (type === 3) {
            // Low Boost: boost peak + low/high shelves (fixed shelf edges)
            const lo = root.mfx.paramMin(0)
            const hi = root.mfx.paramMax(0)
            let boostHz = 80
            if (hi - lo > 0 && hi - lo < 500) {
                // Likely discrete / narrow range → map to 50–125 Hz
                const n = (saneAt(0, lo) - lo) / (hi - lo)
                boostHz = 50 + n * 75
            } else if (saneAt(0, 80) >= 50 && saneAt(0, 80) <= 125) {
                boostHz = saneAt(0, 80)
            } else {
                const n = Math.max(0, Math.min(1, (saneAt(0, 64) - lo) / Math.max(1, hi - lo)))
                boostHz = 50 + n * 75
            }
            const widthIdx = saneAt(2, 1)
            const q = widthIdx <= 0 ? 0.7 : (widthIdx === 1 ? 1.4 : 2.8)
            const defs = [
                {
                    label: "Boost",
                    color: colLow,
                    kind: "bell",
                    freqHz: boostHz,
                    gainDb: Math.max(0, Math.min(12, saneAt(1, 0))),
                    q: q,
                    hasQ: true,
                    hasFreq: true,
                    gainIdx: 1,
                    freqIdx: 0,
                    qIdx: 2
                },
                {
                    label: "Low Shelf",
                    color: colMid1,
                    kind: "shelfLow",
                    freqHz: 200,
                    gainDb: sanitizeGain(saneAt(3, 0)),
                    q: 1,
                    hasQ: false,
                    hasFreq: false,
                    gainIdx: 3
                },
                {
                    label: "High Shelf",
                    color: colHigh,
                    kind: "shelfHigh",
                    freqHz: 4000,
                    gainDb: sanitizeGain(saneAt(4, 0)),
                    q: 1,
                    hasQ: false,
                    hasFreq: false,
                    gainIdx: 4
                }
            ]
            for (let i = 0; i < defs.length; ++i) {
                const solo = root.soloBand === i
                defs[i].dimmed = root.soloBand >= 0 && !solo
                defs[i].bypassed = isBypassed(i)
                defs[i].solo = solo
            }
            return defs
        }

        // Type 1 Equalizer
        const bands = [
            {
                label: "Low Shelf",
                color: colLow,
                kind: "shelfLow",
                freqHz: hzFromLowFreq(saneAt(0, 0)),
                gainDb: sanitizeGain(saneAt(1, 0)),
                q: 1,
                hasQ: false,
                hasFreq: true
            },
            {
                label: "Bell",
                color: colMid1,
                kind: "bell",
                freqHz: hzFromMidFreq(saneAt(2, 7)),
                gainDb: sanitizeGain(saneAt(3, 0)),
                q: qFromIndex(saneAt(4, 1)),
                hasQ: true,
                hasFreq: true
            },
            {
                label: "Bell",
                color: colMid2,
                kind: "bell",
                freqHz: hzFromMidFreq(saneAt(5, 13)),
                gainDb: sanitizeGain(saneAt(6, 0)),
                q: qFromIndex(saneAt(7, 1)),
                hasQ: true,
                hasFreq: true
            },
            {
                label: "High Shelf",
                color: colHigh,
                kind: "shelfHigh",
                freqHz: hzFromHighFreq(saneAt(8, 1)),
                gainDb: sanitizeGain(saneAt(9, 0)),
                q: 1,
                hasQ: false,
                hasFreq: true
            }
        ]
        for (let i = 0; i < bands.length; ++i) {
            const solo = root.soloBand === i
            bands[i].dimmed = root.soloBand >= 0 && !solo
            bands[i].bypassed = isBypassed(i)
            bands[i].solo = solo
        }
        return bands
    }

    function clearBypassIfGainChanged(bandIndex, newGain) {
        if (!isBypassed(bandIndex))
            return
        if (Math.round(newGain) !== 0)
            setBypassFlag(bandIndex, false, 0)
    }

    function onBandMoved(index, freqHz, gainDb) {
        if (!root.mfx || index < 0)
            return
        const g = Math.round(Math.max(-15, Math.min(15, gainDb)))
        clearBypassIfGainChanged(index, g)

        if (root.mfxType === 2) {
            if (index >= 0 && index < 8)
                writeParam(index, g)
            return
        }

        if (root.mfxType === 3) {
            if (index === 0) {
                writeParam(1, Math.max(0, Math.min(12, g)))
                const lo = root.mfx.paramMin(0)
                const hi = root.mfx.paramMax(0)
                const n = Math.max(0, Math.min(1, (freqHz - 50) / 75))
                writeParam(0, Math.round(lo + n * (hi - lo)))
            } else if (index === 1) {
                writeParam(3, g)
            } else if (index === 2) {
                writeParam(4, g)
            }
            return
        }

        // Type 1
        if (index === 0) {
            writeParam(0, nearestIndex(root.lowFreqTable, freqHz))
            writeParam(1, g)
        } else if (index === 1) {
            writeParam(2, nearestIndex(root.midFreqTable, freqHz))
            writeParam(3, g)
        } else if (index === 2) {
            writeParam(5, nearestIndex(root.midFreqTable, freqHz))
            writeParam(6, g)
        } else if (index === 3) {
            writeParam(8, nearestIndex(root.highFreqTable, freqHz))
            writeParam(9, g)
        }
    }

    function onBandQChanged(index, q) {
        if (!root.mfx || index < 0)
            return
        const qi = qToIndex(q)

        if (root.mfxType === 2) {
            writeParam(8, qi)
            return
        }
        if (root.mfxType === 3) {
            if (index === 0) {
                // Boost Width: 0 WIDE, 1 MID, 2 NARROW — inverse of Q
                const w = qi <= 0 ? 0 : (qi <= 2 ? 1 : 2)
                writeParam(2, w)
            }
            return
        }
        // Type 1 mids
        if (index === 1)
            writeParam(4, qi)
        else if (index === 2)
            writeParam(7, qi)
    }

    function resetBand(index) {
        if (!root.mfx || index < 0)
            return
        setBypassFlag(index, false, 0)
        if (root.soloBand === index)
            root.soloBand = -1

        if (root.mfxType === 2) {
            writeParam(index, 0)
            return
        }
        if (root.mfxType === 3) {
            if (index === 0) {
                writeParam(1, 0)
                writeParam(2, 1) // MID width
                // leave frequency
            } else if (index === 1) {
                writeParam(3, 0)
            } else if (index === 2) {
                writeParam(4, 0)
            }
            return
        }
        // Type 1 defaults: 0 dB, mid Q (1.0), sensible freqs
        if (index === 0) {
            writeParam(0, 0)   // 200 Hz
            writeParam(1, 0)
        } else if (index === 1) {
            writeParam(2, 7)   // 1000 Hz
            writeParam(3, 0)
            writeParam(4, 1)   // Q 1.0
        } else if (index === 2) {
            writeParam(5, 13)  // 4000 Hz
            writeParam(6, 0)
            writeParam(7, 1)
        } else if (index === 3) {
            writeParam(8, 1)   // 4000 Hz
            writeParam(9, 0)
        }
    }

    function toggleSolo(index) {
        root.soloBand = (root.soloBand === index) ? -1 : index
    }

    function toggleBypass(index) {
        if (!root.mfx || index < 0)
            return
        const bands = root.graphBands
        if (index >= bands.length)
            return
        const curGain = Number(bands[index].gainDb) || 0

        if (isBypassed(index)) {
            const restore = Number(root.bypassStash[index]) || 0
            setBypassFlag(index, false, 0)
            // Write restored gain
            if (root.mfxType === 2) {
                writeParam(index, restore)
            } else if (root.mfxType === 3) {
                if (index === 0) writeParam(1, Math.max(0, Math.min(12, restore)))
                else if (index === 1) writeParam(3, restore)
                else if (index === 2) writeParam(4, restore)
            } else {
                const gainIdx = [1, 3, 6, 9][index]
                writeParam(gainIdx, restore)
            }
        } else {
            setBypassFlag(index, true, curGain)
            if (root.mfxType === 2) {
                writeParam(index, 0)
            } else if (root.mfxType === 3) {
                if (index === 0) writeParam(1, 0)
                else if (index === 1) writeParam(3, 0)
                else if (index === 2) writeParam(4, 0)
            } else {
                const gainIdx = [1, 3, 6, 9][index]
                writeParam(gainIdx, 0)
            }
        }
    }

    // Reset UI solo/bypass when effect type changes
    onMfxTypeChanged: {
        root.soloBand = -1
        root.bypassFlags = []
        root.bypassStash = []
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 8
        Label {
            text: (root.mfx ? root.mfx.typeName : "EQ") + "  ·  frequency"
            color: LogicTheme.textPrimary
            font.bold: true
            font.pixelSize: LogicTheme.fontSize
            Layout.fillWidth: true
        }
        Label {
            text: root.soloBand >= 0 ? "Solo (UI-only)" : "Drag nodes · sliders mirror"
            color: root.soloBand >= 0 ? LogicTheme.soloYellow : LogicTheme.textMuted
            font.pixelSize: LogicTheme.fontSizeSmall
        }
    }

    MfxEqGraph {
        id: eqGraph
        Layout.fillWidth: true
        Layout.preferredHeight: 200
        Layout.minimumHeight: 160
        Layout.maximumHeight: 240
        editorEnabled: root.editorEnabled
        bands: root.graphBands

        onBandMoved: (index, freqHz, gainDb) => root.onBandMoved(index, freqHz, gainDb)
        onBandQChanged: (index, q) => root.onBandQChanged(index, q)
        onBandResetRequested: (index) => root.resetBand(index)
        onBandSoloRequested: (index) => root.toggleSolo(index)
        onBandBypassRequested: (index) => root.toggleBypass(index)
    }

    Label {
        text: {
            if (root.mfxType === 2)
                return "Spectrum bands — wheel/Shift-drag any node for shared Q · soft bypass = gain 0"
            if (root.mfxType === 3)
                return "Low Boost — drag boost node for freq/gain · width via Q · soft bypass = gain 0"
            return "Low / Mid / Mid / High — wheel or Shift-drag Mid for Q · soft bypass = gain 0 (no SysEx bypass/solo)"
        }
        color: LogicTheme.textSecondary
        font.pixelSize: LogicTheme.fontSizeSmall
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
    }

    // Mirror faders (same params the graph writes)
    MfxParamGrid {
        id: primaryGrid
        Layout.fillWidth: true
        mfxModel: root.mfx
        enabled: root.editorEnabled
        formatLabel: (name) => {
            const s = root.shortLabel(name)
            return (name.indexOf("Gain") >= 0 || name.indexOf("Band") === 0) ? (s + "  dB") : s
        }
    }
}
