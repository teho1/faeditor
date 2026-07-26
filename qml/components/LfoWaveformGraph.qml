import QtQuick
import FAEditor

Item {
    id: root

    property int shape: 1
    property int rate: 64
    property int depth: 64
    property color curveColor: LogicTheme.accent
    property bool running: true

    implicitWidth: 320
    implicitHeight: 120

    readonly property real padL: 10
    readonly property real padR: 10
    readonly property real padT: 10
    readonly property real padB: 10
    property real phase: 0

    // Map FA shape indices (SN-S: TRI SIN SAW SQR S&H RND / PCM varies) via friendly override
    property string shapeName: ""

    function sample(t) {
        // t in 0..1 for one cycle
        const name = shapeName.length ? shapeName.toUpperCase() : ""
        const s = shape
        let kind = "SIN"
        if (name.indexOf("TRI") >= 0) kind = "TRI"
        else if (name.indexOf("SIN") >= 0) kind = "SIN"
        else if (name.indexOf("SAW") >= 0)
            // SAW2 / SAWD = falling; SAW / SAW1 = rising
            kind = (name.indexOf("DW") >= 0 || name.indexOf("SAW2") >= 0) ? "SAWD" : "SAW"
        else if (name.indexOf("SQR") >= 0) kind = "SQR"
        else if (name.indexOf("TRP") >= 0) kind = "TRP"
        else if (name.indexOf("S&H") >= 0 || name.indexOf("STEP") >= 0) kind = "SH"
        else if (name.indexOf("RND") >= 0 || name.indexOf("CHS") >= 0) kind = "RND"
        else {
            // SN-S defaults by index
            const sn = ["TRI", "SIN", "SAW", "SQR", "SH", "RND"]
            kind = sn[Math.max(0, Math.min(sn.length - 1, s))] || "SIN"
        }

        if (kind === "TRI")
            return t < 0.5 ? (t * 4 - 1) : (3 - t * 4)
        if (kind === "SAW")
            return t * 2 - 1
        if (kind === "SAWD")
            return 1 - t * 2
        if (kind === "SQR")
            return t < 0.5 ? 1 : -1
        if (kind === "TRP") {
            // Trapezoid: flat top/bottom with linear ramps
            if (t < 0.2) return -1 + t * 10
            if (t < 0.5) return 1
            if (t < 0.7) return 1 - (t - 0.5) * 10
            return -1
        }
        if (kind === "SH") {
            const steps = 8
            const idx = Math.floor(t * steps)
            // deterministic pseudo steps from phase bucket
            const u = Math.sin((idx + 1) * 12.9898) * 43758.5453
            return (u - Math.floor(u)) * 2 - 1
        }
        if (kind === "RND") {
            const u = Math.sin((Math.floor(t * 32) + phase * 10) * 78.233) * 43758.5453
            return (u - Math.floor(u)) * 2 - 1
        }
        // SIN
        return Math.sin(t * Math.PI * 2)
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            const x0 = root.padL
            const y0 = root.padT
            const w = width - root.padL - root.padR
            const h = height - root.padT - root.padB
            const mid = y0 + h / 2
            const amp = (h * 0.42) * (0.25 + 0.75 * (root.depth / 127))

            ctx.fillStyle = LogicTheme.dark ? "#1A1A1C" : "#E8E8ED"
            ctx.strokeStyle = LogicTheme.hairline
            ctx.lineWidth = 1
            ctx.fillRect(x0, y0, w, h)
            ctx.strokeRect(x0, y0, w, h)

            ctx.strokeStyle = LogicTheme.dark ? "#2A2A2E" : "#D8D8DC"
            ctx.beginPath()
            ctx.moveTo(x0, mid)
            ctx.lineTo(x0 + w, mid)
            ctx.stroke()

            const cycles = 2.2
            const n = 120
            ctx.beginPath()
            for (let i = 0; i <= n; ++i) {
                const u = i / n
                const t = (u * cycles + root.phase) % 1
                const y = mid - root.sample(t) * amp
                const x = x0 + u * w
                if (i === 0)
                    ctx.moveTo(x, y)
                else
                    ctx.lineTo(x, y)
            }
            ctx.strokeStyle = root.curveColor
            ctx.lineWidth = 2
            ctx.lineJoin = "round"
            ctx.stroke()
        }

        Connections {
            target: root
            function onShapeChanged() { canvas.requestPaint() }
            function onRateChanged() { canvas.requestPaint() }
            function onDepthChanged() { canvas.requestPaint() }
            function onShapeNameChanged() { canvas.requestPaint() }
            function onPhaseChanged() { canvas.requestPaint() }
            function onWidthChanged() { canvas.requestPaint() }
            function onHeightChanged() { canvas.requestPaint() }
        }
    }

    Timer {
        interval: 33
        running: root.running && root.visible
        repeat: true
        onTriggered: {
            const speed = 0.008 + (root.rate / 127) * 0.045
            root.phase = (root.phase + speed) % 1
        }
    }
}
