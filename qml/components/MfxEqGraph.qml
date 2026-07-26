import QtQuick
import QtQuick.Controls
import FAEditor

/**
 * FabFilter-style EQ response graph: nodes are the primary editors.
 * Parent feeds band descriptors (Hz / dB / Q) and handles param writes.
 *
 * Band object fields:
 *   label, color, kind ("shelfLow"|"bell"|"shelfHigh"|"fixed"),
 *   freqHz, gainDb, q, hasQ, hasFreq, dimmed, bypassed
 */
Item {
    id: root

    property var bands: []
    property int selectedBand: -1
    property bool editorEnabled: true

    /// Axis
    readonly property real minHz: 20
    readonly property real maxHz: 20000
    readonly property real minDb: -15
    readonly property real maxDb: 15

    readonly property real padL: 36
    readonly property real padR: 12
    readonly property real padT: 12
    readonly property real padB: 22
    readonly property real plotW: Math.max(1, width - padL - padR)
    readonly property real plotH: Math.max(1, height - padT - padB)

    signal bandMoved(int index, real freqHz, real gainDb)
    signal bandQChanged(int index, real q)
    signal bandResetRequested(int index)
    signal bandSoloRequested(int index)
    signal bandBypassRequested(int index)
    signal bandSelected(int index)

    implicitWidth: 360
    implicitHeight: 200

    function clamp(v, lo, hi) { return Math.max(lo, Math.min(hi, v)) }

    function hzToX(hz) {
        const h = clamp(hz, root.minHz, root.maxHz)
        const t = Math.log(h / root.minHz) / Math.log(root.maxHz / root.minHz)
        return root.padL + t * root.plotW
    }

    function xToHz(x) {
        const t = clamp((x - root.padL) / root.plotW, 0, 1)
        return root.minHz * Math.pow(root.maxHz / root.minHz, t)
    }

    function dbToY(db) {
        const n = clamp(db, root.minDb, root.maxDb)
        const t = (n - root.minDb) / (root.maxDb - root.minDb)
        return root.padT + root.plotH * (1 - t)
    }

    function yToDb(y) {
        const t = 1 - clamp((y - root.padT) / root.plotH, 0, 1)
        return root.minDb + t * (root.maxDb - root.minDb)
    }

    function peakMag(hz, centerHz, q, gainDb) {
        if (Math.abs(gainDb) < 0.001)
            return 0
        const t = Math.log(Math.max(1, hz))
        const c = Math.log(Math.max(1, centerHz))
        // Higher Q → narrower bell (Roland Q: 0.5 … 8)
        const width = 0.72 / Math.max(0.35, q)
        const d = (t - c) / width
        return gainDb * Math.exp(-0.5 * d * d)
    }

    function shelfLow(hz, edgeHz, gainDb) {
        if (Math.abs(gainDb) < 0.001)
            return 0
        const t = Math.log(Math.max(1, hz))
        const e = Math.log(Math.max(1, edgeHz))
        const k = clamp(1 - (t - e) / 0.85, 0, 1)
        return gainDb * k * k
    }

    function shelfHigh(hz, edgeHz, gainDb) {
        if (Math.abs(gainDb) < 0.001)
            return 0
        const t = Math.log(Math.max(1, hz))
        const e = Math.log(Math.max(1, edgeHz))
        const k = clamp((t - e + 0.35) / 0.85, 0, 1)
        return gainDb * k * k
    }

    function bandContribution(band, hz) {
        if (!band || band.bypassed)
            return 0
        const g = Number(band.gainDb) || 0
        const f = Number(band.freqHz) || 1000
        const q = Number(band.q) || 1
        const kind = band.kind || "bell"
        if (kind === "shelfLow")
            return shelfLow(hz, f, g)
        if (kind === "shelfHigh")
            return shelfHigh(hz, f, g)
        return peakMag(hz, f, q, g)
    }

    function responseDb(hz) {
        const list = root.bands || []
        let sum = 0
        let anySolo = false
        for (let i = 0; i < list.length; ++i) {
            if (list[i] && list[i].solo)
                anySolo = true
        }
        for (let i = 0; i < list.length; ++i) {
            const b = list[i]
            if (!b)
                continue
            if (anySolo && !b.solo)
                continue
            sum += bandContribution(b, hz)
        }
        return clamp(sum, -18, 18)
    }

    function hitTest(mx, my) {
        const list = root.bands || []
        let best = -1
        let bestD = 22 * 22
        for (let i = 0; i < list.length; ++i) {
            const b = list[i]
            if (!b)
                continue
            const x = hzToX(Number(b.freqHz) || 1000)
            const y = dbToY(Number(b.gainDb) || 0)
            const dx = mx - x
            const dy = my - y
            const d = dx * dx + dy * dy
            if (d < bestD) {
                bestD = d
                best = i
            }
        }
        return best
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            const w = width, h = height
            if (w < 8 || h < 8)
                return

            const pl = root.padL, pt = root.padT, pw = root.plotW, ph = root.plotH

            ctx.fillStyle = LogicTheme.dark ? "#1A1A1C" : "#E8E8ED"
            ctx.fillRect(0, 0, w, h)
            ctx.strokeStyle = LogicTheme.hairline
            ctx.strokeRect(pl, pt, pw, ph)

            // dB grid
            ctx.strokeStyle = LogicTheme.dark ? "#2A2A2E" : "#D8D8DC"
            ctx.lineWidth = 1
            for (let g = -12; g <= 12; g += 6) {
                const y = root.dbToY(g)
                ctx.beginPath()
                ctx.moveTo(pl, y)
                ctx.lineTo(pl + pw, y)
                ctx.stroke()
            }
            ctx.strokeStyle = LogicTheme.hairline
            ctx.beginPath()
            ctx.moveTo(pl, root.dbToY(0))
            ctx.lineTo(pl + pw, root.dbToY(0))
            ctx.stroke()

            // dB labels
            ctx.fillStyle = LogicTheme.textMuted
            ctx.font = LogicTheme.fontSizeSmall + "px sans-serif"
            ctx.textAlign = "right"
            ctx.fillText("+15", pl - 4, root.dbToY(15) + 3)
            ctx.fillText("0", pl - 4, root.dbToY(0) + 3)
            ctx.fillText("−15", pl - 4, root.dbToY(-15) + 3)

            // Log frequency ticks / labels
            const freqLabels = [
                { hz: 20, text: "20Hz" },
                { hz: 100, text: "100" },
                { hz: 500, text: "500" },
                { hz: 1000, text: "1k" },
                { hz: 5000, text: "5k" },
                { hz: 10000, text: "10k" },
                { hz: 20000, text: "20k" }
            ]
            ctx.textAlign = "center"
            for (let i = 0; i < freqLabels.length; ++i) {
                const x = root.hzToX(freqLabels[i].hz)
                ctx.strokeStyle = LogicTheme.dark ? "#2A2A2E" : "#D8D8DC"
                ctx.beginPath()
                ctx.moveTo(x, pt)
                ctx.lineTo(x, pt + ph)
                ctx.stroke()
                ctx.fillStyle = LogicTheme.textMuted
                ctx.fillText(freqLabels[i].text, x, h - 5)
            }

            const list = root.bands || []
            const steps = 96

            // Per-band ghost curves
            for (let bi = 0; bi < list.length; ++bi) {
                const b = list[bi]
                if (!b || b.bypassed)
                    continue
                const dim = !!b.dimmed
                ctx.beginPath()
                for (let i = 0; i <= steps; ++i) {
                    const t = i / steps
                    const hz = root.minHz * Math.pow(root.maxHz / root.minHz, t)
                    const x = pl + t * pw
                    const y = root.dbToY(root.clamp(root.bandContribution(b, hz), -18, 18))
                    if (i === 0)
                        ctx.moveTo(x, y)
                    else
                        ctx.lineTo(x, y)
                }
                ctx.strokeStyle = b.color || LogicTheme.accent
                ctx.globalAlpha = dim ? 0.18 : 0.35
                ctx.lineWidth = 1.25
                ctx.stroke()
                ctx.globalAlpha = 1
            }

            // Sum response
            ctx.beginPath()
            for (let i = 0; i <= steps; ++i) {
                const t = i / steps
                const hz = root.minHz * Math.pow(root.maxHz / root.minHz, t)
                const x = pl + t * pw
                const y = root.dbToY(root.responseDb(hz))
                if (i === 0)
                    ctx.moveTo(x, y)
                else
                    ctx.lineTo(x, y)
            }
            ctx.strokeStyle = LogicTheme.dark ? "#F5F5F7" : "#1D1D1F"
            ctx.lineWidth = 2
            ctx.lineJoin = "round"
            ctx.stroke()

            // Nodes
            for (let bi = 0; bi < list.length; ++bi) {
                const b = list[bi]
                if (!b)
                    continue
                const x = root.hzToX(Number(b.freqHz) || 1000)
                const y = root.dbToY(Number(b.gainDb) || 0)
                const dim = !!b.dimmed
                const sel = bi === root.selectedBand
                const col = b.color || LogicTheme.accent

                ctx.globalAlpha = dim ? 0.35 : (b.bypassed ? 0.45 : 1)
                ctx.beginPath()
                ctx.arc(x, y, sel ? 7 : 5.5, 0, Math.PI * 2)
                ctx.fillStyle = b.bypassed ? LogicTheme.textMuted : col
                ctx.fill()
                ctx.strokeStyle = LogicTheme.panelBgRaised
                ctx.lineWidth = sel ? 2.25 : 1.5
                ctx.stroke()

                if (b.bypassed) {
                    ctx.beginPath()
                    ctx.moveTo(x - 4, y - 4)
                    ctx.lineTo(x + 4, y + 4)
                    ctx.strokeStyle = LogicTheme.textPrimary
                    ctx.lineWidth = 1.5
                    ctx.stroke()
                }

                ctx.fillStyle = LogicTheme.textSecondary
                ctx.font = LogicTheme.fontSizeSmall + "px sans-serif"
                ctx.textAlign = "center"
                ctx.fillText(b.label || "", x, y - 11)
                ctx.globalAlpha = 1
            }
        }

        function repaint() { requestPaint() }

        Connections {
            target: root
            function onBandsChanged() { canvas.repaint() }
            function onSelectedBandChanged() { canvas.repaint() }
            function onWidthChanged() { canvas.repaint() }
            function onHeightChanged() { canvas.repaint() }
        }
        Component.onCompleted: requestPaint()
    }

    MouseArea {
        id: drag
        anchors.fill: parent
        enabled: root.editorEnabled
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        preventStealing: true
        cursorShape: containsMouse || pressed ? Qt.PointingHandCursor : Qt.ArrowCursor

        property int bandIndex: -1
        property real pressX
        property real pressY
        property real startFreq
        property real startGain
        property real startQ
        property bool qMode: false

        onPressed: (mouse) => {
            if (mouse.button === Qt.RightButton) {
                const hi = root.hitTest(mouse.x, mouse.y)
                if (hi >= 0) {
                    root.selectedBand = hi
                    root.bandSelected(hi)
                    bandMenu.bandIndex = hi
                    bandMenu.popup(drag, mouse.x, mouse.y)
                }
                return
            }
            const hi = root.hitTest(mouse.x, mouse.y)
            bandIndex = hi
            if (hi < 0)
                return
            root.selectedBand = hi
            root.bandSelected(hi)
            const b = root.bands[hi]
            pressX = mouse.x
            pressY = mouse.y
            startFreq = Number(b.freqHz) || 1000
            startGain = Number(b.gainDb) || 0
            startQ = Number(b.q) || 1
            qMode = !!(mouse.modifiers & Qt.ShiftModifier) && !!b.hasQ
        }

        onPositionChanged: (mouse) => {
            if (!pressed || bandIndex < 0 || mouse.buttons !== Qt.LeftButton)
                return
            const b = root.bands[bandIndex]
            if (!b)
                return

            const shiftQ = !!(mouse.modifiers & Qt.ShiftModifier) && !!b.hasQ
            if (shiftQ || qMode) {
                qMode = true
                const dy = mouse.y - pressY
                // Drag up → higher Q (narrower)
                const q = root.clamp(startQ * Math.pow(2, -dy / 48), 0.5, 8)
                root.bandQChanged(bandIndex, q)
                return
            }

            let freq = startFreq
            let gain = startGain
            if (b.hasFreq !== false)
                freq = root.xToHz(mouse.x)
            gain = root.yToDb(mouse.y)
            root.bandMoved(bandIndex, freq, gain)
        }

        onDoubleClicked: (mouse) => {
            const hi = root.hitTest(mouse.x, mouse.y)
            if (hi >= 0)
                root.bandResetRequested(hi)
        }

        onWheel: (wheel) => {
            const hi = root.hitTest(wheel.x, wheel.y)
            if (hi < 0)
                return
            const b = root.bands[hi]
            if (!b || !b.hasQ)
                return
            wheel.accepted = true
            const step = wheel.angleDelta.y > 0 ? 1 : -1
            // Discrete-ish: multiply by √2 per notch toward Roland Q table
            const q = Number(b.q) || 1
            const next = root.clamp(q * (step > 0 ? 1.41421356 : 1 / 1.41421356), 0.5, 8)
            root.selectedBand = hi
            root.bandQChanged(hi, next)
        }

        onReleased: {
            bandIndex = -1
            qMode = false
        }
        onCanceled: {
            bandIndex = -1
            qMode = false
        }

        ToolTip.visible: drag.containsMouse && root.hitTest(drag.mouseX, drag.mouseY) >= 0
        ToolTip.delay: 600
        ToolTip.text: {
            const hi = root.hitTest(drag.mouseX, drag.mouseY)
            if (hi < 0 || !root.bands || !root.bands[hi])
                return ""
            const b = root.bands[hi]
            const hz = Math.round(Number(b.freqHz) || 0)
            const g = Number(b.gainDb) || 0
            const gTxt = (g >= 0 ? "+" : "") + g.toFixed(0) + " dB"
            const hzTxt = hz >= 1000
                ? ((hz / 1000).toFixed(hz % 1000 === 0 ? 0 : 1) + "k")
                : String(hz)
            let tip = (b.label || "Band") + "  ·  " + hzTxt + " Hz  ·  " + gTxt
            if (b.hasQ)
                tip += "  ·  Q " + (Number(b.q) || 1).toFixed(1)
            if (b.bypassed)
                tip += "\nSoft bypass (gain → 0 dB; no bypass SysEx)"
            if (b.solo)
                tip += "\nUI-only solo (dims others; no solo SysEx)"
            tip += "\nDrag: freq/gain · Wheel/Shift-drag: Q · Double-click: reset"
            return tip
        }
    }

    Menu {
        id: bandMenu
        property int bandIndex: -1

        MenuItem {
            text: "Reset Band"
            onTriggered: root.bandResetRequested(bandMenu.bandIndex)
        }
        MenuItem {
            text: "Solo Band"
            onTriggered: root.bandSoloRequested(bandMenu.bandIndex)
        }
        MenuItem {
            text: "Bypass Band"
            onTriggered: root.bandBypassRequested(bandMenu.bandIndex)
        }
    }
}
