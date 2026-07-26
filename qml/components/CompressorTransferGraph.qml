import QtQuick
import FAEditor

/**
 * Dynamics transfer curve (in → out) for Compressor / Limiter / Gate.
 * Bound int/real props drive paint — parent should feed via paramValues NOTIFY.
 */
Item {
    id: root

    /** Threshold kink along the diagonal, 0–127 (higher = later/higher threshold). */
    property int threshold: 64
    /** Soft-knee width 0–127 (0 = hard). */
    property int knee: 0
    /** Makeup / Post Gain 0–127 — vertical shift of the curve. */
    property int makeup: 64
    /** Output Level 0–127 — scales the transfer slightly. */
    property int level: 100
    /** Attack / Release / Hold 0–127 — envelope sparkline timing. */
    property int attack: 40
    property int release: 64
    property int hold: 0
    /** Gate floor / wet-dry style balance 0–100. */
    property int balance: 100
    /** Limiter Ratio raw catalog value (e.g. 0–3). Ignored when hasRatio is false. */
    property real ratioRaw: 0
    property real ratioMax: 3
    property bool hasRatio: false
    /** true → gate/expander curve (attenuate below threshold). */
    property bool isGate: false
    /** Gate Mode: 0 = hard gate, 1 = expander-ish. */
    property int gateMode: 0

    property color curveColor: LogicTheme.accent
    property bool running: true

    implicitWidth: 320
    implicitHeight: 120

    readonly property real pad: 14
    /** Envelope phase 0–1 for Attack/Hold/Release animation. */
    property real envPhase: 0

    /** Above-threshold slope (out/in). Lower = more compression. */
    function slopeAbove() {
        if (root.hasRatio) {
            const max = Math.max(1, Math.round(root.ratioMax))
            if (max <= 3) {
                const slopes = [1 / 1.5, 1 / 2, 1 / 4, 1 / 100]
                const i = Math.max(0, Math.min(3, Math.round(root.ratioRaw)))
                return slopes[i]
            }
            const n = Math.max(0, Math.min(1, root.ratioRaw / Math.max(1e-6, root.ratioMax)))
            const r = 1 + n * 19
            return 1 / r
        }
        // FA Compressor has no Ratio — fixed ~3.5:1 sketch
        return 1 / 3.5
    }

    function transfer(xin) {
        const thr = Math.max(0, Math.min(1, root.threshold / 127))
        const kneeW = Math.max(0, Math.min(0.35, (root.knee / 127) * 0.35))
        const makeup = ((root.makeup / 127) - 0.5) * 0.28
        const lvl = 0.7 + 0.3 * (root.level / 127)
        let y = xin

        if (root.isGate) {
            const floor = Math.max(0, Math.min(1, root.balance / 100)) * 0.15
            if (root.gateMode > 0) {
                // Expander: gentle attenuation below threshold
                if (xin < thr) {
                    const t = thr > 1e-6 ? xin / thr : 0
                    y = floor + (thr - floor) * Math.pow(Math.max(0, t), 1.6)
                } else {
                    y = xin
                }
            } else {
                // Hard gate with optional knee
                if (xin < thr - kneeW) {
                    y = floor
                } else if (xin < thr + kneeW && kneeW > 1e-6) {
                    const u = (xin - (thr - kneeW)) / (2 * kneeW)
                    y = floor + (thr - floor) * u * u * (3 - 2 * u)
                } else {
                    y = xin
                }
            }
        } else {
            const slope = root.slopeAbove()
            if (kneeW > 1e-6 && xin > thr - kneeW && xin < thr + kneeW) {
                // Soft knee blend between 1:1 and compressed slope
                const u = (xin - (thr - kneeW)) / (2 * kneeW)
                const hard = xin <= thr ? xin : thr + (xin - thr) * slope
                const soft = thr + (xin - thr) * (1 + (slope - 1) * u)
                y = hard * (1 - u) + soft * u
            } else if (xin > thr) {
                y = thr + (xin - thr) * slope
            } else {
                y = xin
            }
        }

        y = y * lvl + makeup
        return Math.max(0, Math.min(1.15, y))
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            const w = width, h = height
            const pad = root.pad
            const plotW = w - 2 * pad
            const plotH = h - 2 * pad

            ctx.fillStyle = LogicTheme.dark ? "#1A1A1C" : "#E8E8ED"
            ctx.fillRect(0, 0, w, h)

            // Unity reference
            ctx.strokeStyle = LogicTheme.hairline
            ctx.lineWidth = 1
            ctx.beginPath()
            ctx.moveTo(pad, h - pad)
            ctx.lineTo(w - pad, pad)
            ctx.stroke()

            // Threshold guide
            const thrN = Math.max(0, Math.min(1, root.threshold / 127))
            const tx = pad + thrN * plotW
            ctx.strokeStyle = LogicTheme.textMuted
            ctx.setLineDash([4, 3])
            ctx.beginPath()
            ctx.moveTo(tx, pad)
            ctx.lineTo(tx, h - pad)
            ctx.stroke()
            ctx.setLineDash([])

            // Transfer curve
            ctx.strokeStyle = root.curveColor
            ctx.lineWidth = 2.2
            ctx.lineJoin = "round"
            ctx.beginPath()
            const steps = 64
            for (let i = 0; i <= steps; ++i) {
                const xin = i / steps
                const yout = root.transfer(xin)
                const x = pad + xin * plotW
                const y = h - pad - Math.min(1, yout) * plotH
                if (i === 0) ctx.moveTo(x, y)
                else ctx.lineTo(x, y)
            }
            ctx.stroke()

            // Attack / Hold / Release sparkline (bottom strip)
            const envH = 16
            const envY = h - pad - envH
            const aN = 0.08 + 0.42 * (1 - root.attack / 127)
            const hN = root.hold > 0 ? 0.05 + 0.25 * (root.hold / 127) : 0.08
            const rN = 0.08 + 0.42 * (root.release / 127)
            const sum = aN + hN + rN
            const aW = (aN / sum) * plotW
            const hW = (hN / sum) * plotW
            const rW = plotW - aW - hW

            ctx.strokeStyle = LogicTheme.dark ? "#3A3A40" : "#C8C8D0"
            ctx.lineWidth = 1.4
            ctx.beginPath()
            ctx.moveTo(pad, envY + envH)
            ctx.lineTo(pad + aW, envY)
            ctx.lineTo(pad + aW + hW, envY)
            ctx.lineTo(pad + aW + hW + rW, envY + envH)
            ctx.stroke()

            // Moving envelope cursor
            const ph = root.envPhase
            let cx = pad
            let cy = envY + envH
            if (ph < aN / sum) {
                const u = ph / (aN / sum)
                cx = pad + u * aW
                cy = envY + envH * (1 - u)
            } else if (ph < (aN + hN) / sum) {
                const u = (ph - aN / sum) / Math.max(1e-6, hN / sum)
                cx = pad + aW + u * hW
                cy = envY
            } else {
                const u = (ph - (aN + hN) / sum) / Math.max(1e-6, rN / sum)
                cx = pad + aW + hW + u * rW
                cy = envY + u * envH
            }
            ctx.fillStyle = root.curveColor
            ctx.beginPath()
            ctx.arc(cx, cy, 2.5, 0, Math.PI * 2)
            ctx.fill()
        }

        Connections {
            target: root
            function onThresholdChanged() { canvas.requestPaint() }
            function onKneeChanged() { canvas.requestPaint() }
            function onMakeupChanged() { canvas.requestPaint() }
            function onLevelChanged() { canvas.requestPaint() }
            function onAttackChanged() { canvas.requestPaint() }
            function onReleaseChanged() { canvas.requestPaint() }
            function onHoldChanged() { canvas.requestPaint() }
            function onBalanceChanged() { canvas.requestPaint() }
            function onRatioRawChanged() { canvas.requestPaint() }
            function onRatioMaxChanged() { canvas.requestPaint() }
            function onHasRatioChanged() { canvas.requestPaint() }
            function onIsGateChanged() { canvas.requestPaint() }
            function onGateModeChanged() { canvas.requestPaint() }
            function onEnvPhaseChanged() { canvas.requestPaint() }
            function onWidthChanged() { canvas.requestPaint() }
            function onHeightChanged() { canvas.requestPaint() }
        }
        Component.onCompleted: requestPaint()
    }

    Timer {
        interval: 33
        running: root.running && root.visible
        repeat: true
        onTriggered: {
            // Faster attack / slower release → different cursor speeds (visual only)
            const atk = 0.012 + (1 - root.attack / 127) * 0.04
            const rel = 0.008 + (root.release / 127) * 0.028
            const hold = root.hold > 0 ? 0.004 + (1 - root.hold / 127) * 0.01 : 0.01
            const speed = (atk + rel + hold) / 3
            root.envPhase = (root.envPhase + speed) % 1
        }
    }
}
