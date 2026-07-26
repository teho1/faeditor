import QtQuick
import FAEditor

Item {
    id: root

    property int cutoff: 80
    property int resonance: 40
    /// Filter type label from model (e.g. LPF, BPF, HPF, PKG, OFF, BYPASS, LPF2…)
    property string filterTypeName: "LPF"
    property color curveColor: LogicTheme.accent
    property color fillColor: LogicTheme.dark ? "#5E9CE628" : "#007AFF18"

    // Live drag overrides so the curve moves immediately; cleared on release.
    property int _dragCutoff: -1
    property int _dragResonance: -1
    readonly property int effectiveCutoff: _dragCutoff >= 0 ? _dragCutoff : cutoff
    readonly property int effectiveResonance: _dragResonance >= 0 ? _dragResonance : resonance

    signal cutoffChangedByUser(int value)
    signal resonanceChangedByUser(int value)

    implicitWidth: 320
    implicitHeight: 160

    readonly property real padL: 12
    readonly property real padR: 12
    readonly property real padT: 14
    readonly property real padB: 22
    readonly property real plotW: Math.max(1, width - padL - padR)
    readonly property real plotH: Math.max(1, height - padT - padB)

    readonly property string filterKind: {
        const n = filterTypeName.toUpperCase()
        if (n === "OFF" || n === "BYPASS" || n.length === 0)
            return "OFF"
        if (n.indexOf("BPF") >= 0)
            return "BPF"
        if (n.indexOf("HPF") >= 0)
            return "HPF"
        if (n.indexOf("PKG") >= 0 || n.indexOf("PEAK") >= 0)
            return "PKG"
        if (n.indexOf("LPF") >= 0) {
            if (n.indexOf("4") >= 0) return "LPF4"
            if (n.indexOf("3") >= 0) return "LPF3"
            if (n.indexOf("2") >= 0) return "LPF2"
            return "LPF"
        }
        return "LPF"
    }

    function clamp127(v) {
        return Math.max(0, Math.min(127, Math.round(v)))
    }

    function peakX() {
        return padL + (effectiveCutoff / 127) * plotW
    }

    function peakY() {
        const boost = effectiveResonance / 127
        const kind = filterKind
        if (kind === "OFF")
            return padT + plotH * 0.45
        if (kind === "PKG")
            return padT + plotH * (0.45 - boost * 0.38)
        if (kind === "BPF")
            return padT + plotH * (0.35 - boost * 0.28)
        // LPF / HPF family
        return padT + plotH * (0.55 - boost * 0.48)
    }

    function sampleMag(x, cx, res) {
        const kind = filterKind
        const w = Math.max(1, plotW)
        const nx = (x - cx) / Math.max(8, w * 0.12)

        if (kind === "OFF")
            return 0.48

        if (kind === "BPF") {
            const width = w * (0.14 - res * 0.06)
            const bell = Math.exp(-Math.pow((x - cx) / Math.max(6, width), 2))
            return 0.08 + (0.55 + res * 0.4) * bell
        }

        if (kind === "PKG") {
            const width = w * (0.12 - res * 0.04)
            const bell = Math.exp(-Math.pow((x - cx) / Math.max(6, width), 2))
            return 0.48 + res * 0.5 * bell
        }

        if (kind === "HPF") {
            // Mirror of resonant LPF
            if (x >= cx) {
                const rise = Math.exp(-Math.pow((x - cx) / (w * 0.35), 2) * 0.15)
                return 0.55 + rise * 0.1 + res * 0.35 * Math.exp(-Math.pow(nx, 2))
            }
            const roll = 1 / (1 + Math.pow((cx - x) / (w * 0.08), 2.2))
            return (0.55 + res * 0.4) * roll
        }

        // LPF / LPF2 / LPF3 / LPF4 — steeper slope for higher orders
        let rollExp = 2.2
        let rollScale = 0.08
        if (kind === "LPF2") { rollExp = 2.8; rollScale = 0.07 }
        else if (kind === "LPF3") { rollExp = 3.4; rollScale = 0.055 }
        else if (kind === "LPF4") { rollExp = 4.0; rollScale = 0.045 }

        if (x <= cx) {
            const rise = Math.exp(-Math.pow((cx - x) / (w * 0.35), 2) * 0.15)
            return 0.55 + rise * 0.1 + res * 0.35 * Math.exp(-Math.pow(nx, 2))
        }
        const roll = 1 / (1 + Math.pow((x - cx) / (w * rollScale), rollExp))
        return (0.55 + res * 0.4) * roll
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()

            ctx.fillStyle = LogicTheme.dark ? "#1A1A1C" : "#E8E8ED"
            ctx.strokeStyle = LogicTheme.hairline
            ctx.lineWidth = 1
            ctx.fillRect(root.padL, root.padT, root.plotW, root.plotH)
            ctx.strokeRect(root.padL, root.padT, root.plotW, root.plotH)

            // Light grid
            ctx.strokeStyle = LogicTheme.dark ? "#2A2A2E" : "#D8D8DC"
            for (let g = 1; g < 4; ++g) {
                const y = root.padT + root.plotH * g / 4
                ctx.beginPath()
                ctx.moveTo(root.padL, y)
                ctx.lineTo(root.padL + root.plotW, y)
                ctx.stroke()
            }

            const cx = root.peakX()
            const res = root.effectiveResonance / 127
            const off = root.filterKind === "OFF"
            const samples = 96
            const pts = []
            for (let i = 0; i <= samples; ++i) {
                const t = i / samples
                const x = root.padL + t * root.plotW
                let mag = root.sampleMag(x, cx, res)
                mag = Math.max(0.02, Math.min(1.05, mag))
                const y = root.padT + root.plotH * (1 - mag * 0.92)
                pts.push({ x: x, y: y })
            }

            const strokeCol = off ? LogicTheme.textMuted : root.curveColor
            const fillCol = off
                ? (LogicTheme.dark ? "#FFFFFF10" : "#0000000C")
                : root.fillColor

            ctx.beginPath()
            ctx.moveTo(pts[0].x, root.padT + root.plotH)
            for (let i = 0; i < pts.length; ++i)
                ctx.lineTo(pts[i].x, pts[i].y)
            ctx.lineTo(pts[pts.length - 1].x, root.padT + root.plotH)
            ctx.closePath()
            ctx.fillStyle = fillCol
            ctx.fill()

            ctx.beginPath()
            ctx.moveTo(pts[0].x, pts[0].y)
            for (let i = 1; i < pts.length; ++i)
                ctx.lineTo(pts[i].x, pts[i].y)
            ctx.strokeStyle = strokeCol
            ctx.lineWidth = 2
            ctx.lineJoin = "round"
            ctx.stroke()

            // Peak / cutoff marker (subtle when filter is off)
            const py = root.peakY()
            ctx.beginPath()
            ctx.arc(cx, py, off ? 3.5 : 5, 0, Math.PI * 2)
            ctx.fillStyle = strokeCol
            ctx.fill()
            ctx.strokeStyle = LogicTheme.panelBgRaised
            ctx.lineWidth = 1.5
            ctx.stroke()

            ctx.fillStyle = LogicTheme.textMuted
            ctx.font = LogicTheme.fontSizeSmall + "px sans-serif"
            ctx.textAlign = "left"
            ctx.fillText("Cutoff", root.padL, height - 6)
            ctx.textAlign = "right"
            ctx.fillText("Resonance", root.padL + root.plotW, height - 6)
        }

        Connections {
            target: root
            function onCutoffChanged() { canvas.requestPaint() }
            function onResonanceChanged() { canvas.requestPaint() }
            function onEffectiveCutoffChanged() { canvas.requestPaint() }
            function onEffectiveResonanceChanged() { canvas.requestPaint() }
            function onFilterTypeNameChanged() { canvas.requestPaint() }
            function onWidthChanged() { canvas.requestPaint() }
            function onHeightChanged() { canvas.requestPaint() }
        }
    }

    MouseArea {
        id: drag
        anchors.fill: parent
        anchors.leftMargin: root.padL
        anchors.rightMargin: root.padR
        anchors.topMargin: root.padT
        anchors.bottomMargin: root.padB
        cursorShape: Qt.PointingHandCursor
        preventStealing: true

        property bool draggingCutoff: true
        property int startCut
        property int startRes
        property real startX
        property real startY

        onPressed: (mouse) => {
            startCut = root.effectiveCutoff
            startRes = root.effectiveResonance
            startX = mouse.x
            startY = mouse.y
            const nearPeak = Math.abs((root.padL + mouse.x) - root.peakX()) < 28
            draggingCutoff = !nearPeak
            if (nearPeak)
                draggingCutoff = false
        }
        onPositionChanged: (mouse) => {
            if (!pressed)
                return
            const dx = mouse.x - startX
            const dy = mouse.y - startY
            if (Math.abs(dx) > Math.abs(dy) * 1.2)
                draggingCutoff = true
            else if (Math.abs(dy) > Math.abs(dx) * 1.2)
                draggingCutoff = false

            if (draggingCutoff) {
                const cut = root.clamp127((mouse.x / Math.max(1, width)) * 127)
                root._dragCutoff = cut
                root.cutoffChangedByUser(cut)
            } else {
                const res = root.clamp127(startRes + (-dy / Math.max(1, height)) * 140)
                root._dragResonance = res
                root.resonanceChangedByUser(res)
            }
        }
        onReleased: {
            root._dragCutoff = -1
            root._dragResonance = -1
        }
        onCanceled: {
            root._dragCutoff = -1
            root._dragResonance = -1
        }
    }
}
