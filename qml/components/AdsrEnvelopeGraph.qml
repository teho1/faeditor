import QtQuick
import FAEditor

Item {
    id: root

    property int attack: 40
    property int decay: 40
    property int sustain: 80
    property int release: 40
    property string verticalLabel: "Level"
    property color curveColor: LogicTheme.accent
    property color fillColor: LogicTheme.dark ? "#5E9CE633" : "#007AFF22"

    signal attackChangedByUser(int value)
    signal decayChangedByUser(int value)
    signal sustainChangedByUser(int value)
    signal releaseChangedByUser(int value)

    implicitWidth: 320
    implicitHeight: 180

    readonly property real padL: 36
    readonly property real padR: 12
    readonly property real padT: 16
    readonly property real padB: 28
    readonly property real plotW: Math.max(1, width - padL - padR)
    readonly property real plotH: Math.max(1, height - padT - padB)

    readonly property real aw: 0.08 + (attack / 127) * 0.92
    readonly property real dw: 0.08 + (decay / 127) * 0.92
    readonly property real rw: 0.08 + (release / 127) * 0.92
    readonly property real sw: 0.55
    readonly property real segSum: aw + dw + sw + rw
    readonly property real segA: plotW * aw / segSum
    readonly property real segD: plotW * dw / segSum
    readonly property real segS: plotW * sw / segSum

    readonly property real ax: padL + segA
    readonly property real ay: padT
    readonly property real dx: padL + segA + segD
    readonly property real dy: padT + plotH * (1 - sustain / 127)
    readonly property real sx: padL + segA + segD + segS
    readonly property real sy: dy
    readonly property real rx: padL + plotW
    readonly property real ry: padT + plotH

    function clamp127(v) {
        return Math.max(0, Math.min(127, Math.round(v)))
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            const h = height

            ctx.fillStyle = LogicTheme.dark ? "#1A1A1C" : "#E8E8ED"
            ctx.strokeStyle = LogicTheme.hairline
            ctx.lineWidth = 1
            ctx.fillRect(root.padL, root.padT, root.plotW, root.plotH)
            ctx.strokeRect(root.padL, root.padT, root.plotW, root.plotH)

            ctx.strokeStyle = LogicTheme.dark ? "#2A2A2E" : "#D8D8DC"
            for (let i = 1; i < 4; ++i) {
                const y = root.padT + root.plotH * i / 4
                ctx.beginPath()
                ctx.moveTo(root.padL, y)
                ctx.lineTo(root.padL + root.plotW, y)
                ctx.stroke()
            }

            const x0 = root.padL
            const y0 = root.padT + root.plotH

            ctx.beginPath()
            ctx.moveTo(x0, y0)
            ctx.lineTo(root.ax, root.ay)
            ctx.lineTo(root.dx, root.dy)
            ctx.lineTo(root.sx, root.sy)
            ctx.lineTo(root.rx, root.ry)
            ctx.closePath()
            ctx.fillStyle = root.fillColor
            ctx.fill()

            ctx.beginPath()
            ctx.moveTo(x0, y0)
            ctx.lineTo(root.ax, root.ay)
            ctx.lineTo(root.dx, root.dy)
            ctx.lineTo(root.sx, root.sy)
            ctx.lineTo(root.rx, root.ry)
            ctx.strokeStyle = root.curveColor
            ctx.lineWidth = 2
            ctx.lineJoin = "round"
            ctx.stroke()

            ctx.fillStyle = LogicTheme.textMuted
            ctx.font = LogicTheme.fontSizeSmall + "px sans-serif"
            ctx.textAlign = "center"
            ctx.fillText("A", (x0 + root.ax) / 2, h - 8)
            ctx.fillText("D", (root.ax + root.dx) / 2, h - 8)
            ctx.fillText("S", (root.dx + root.sx) / 2, h - 8)
            ctx.fillText("R", (root.sx + root.rx) / 2, h - 8)

            ctx.save()
            ctx.translate(12, root.padT + root.plotH / 2)
            ctx.rotate(-Math.PI / 2)
            ctx.textAlign = "center"
            ctx.fillStyle = LogicTheme.textSecondary
            ctx.fillText(root.verticalLabel, 0, 0)
            ctx.restore()
        }

        Connections {
            target: root
            function onAttackChanged() { canvas.requestPaint() }
            function onDecayChanged() { canvas.requestPaint() }
            function onSustainChanged() { canvas.requestPaint() }
            function onReleaseChanged() { canvas.requestPaint() }
            function onVerticalLabelChanged() { canvas.requestPaint() }
            function onCurveColorChanged() { canvas.requestPaint() }
            function onWidthChanged() { canvas.requestPaint() }
            function onHeightChanged() { canvas.requestPaint() }
        }
    }

    component EnvHandle: Rectangle {
        id: handle
        property string role: "a"
        property real hx: 0
        property real hy: 0

        width: 14
        height: 14
        radius: 7
        z: 2
        x: hx - width / 2
        y: hy - height / 2
        color: LogicTheme.panelBgRaised
        border.color: root.curveColor
        border.width: 2

        MouseArea {
            anchors.fill: parent
            anchors.margins: -6
            cursorShape: Qt.PointingHandCursor
            preventStealing: true
            property real pressMouseX
            property real pressMouseY
            property int startA
            property int startD
            property int startS
            property int startR

            onPressed: (mouse) => {
                pressMouseX = mouse.x
                pressMouseY = mouse.y
                startA = root.attack
                startD = root.decay
                startS = root.sustain
                startR = root.release
            }
            onPositionChanged: (mouse) => {
                if (!pressed)
                    return
                const ddx = mouse.x - pressMouseX
                const ddy = mouse.y - pressMouseY
                const timeDelta = Math.round((ddx / Math.max(1, root.plotW)) * 180)
                const levelDelta = Math.round((-ddy / Math.max(1, root.plotH)) * 127)
                if (handle.role === "a")
                    root.attackChangedByUser(root.clamp127(startA + timeDelta))
                else if (handle.role === "d") {
                    root.decayChangedByUser(root.clamp127(startD + timeDelta))
                    root.sustainChangedByUser(root.clamp127(startS + levelDelta))
                } else if (handle.role === "s")
                    root.sustainChangedByUser(root.clamp127(startS + levelDelta))
                else if (handle.role === "r")
                    root.releaseChangedByUser(root.clamp127(startR - timeDelta))
            }
        }
    }

    EnvHandle { role: "a"; hx: root.ax; hy: root.ay }
    EnvHandle { role: "d"; hx: root.dx; hy: root.dy }
    EnvHandle { role: "s"; hx: root.sx; hy: root.sy }
    EnvHandle { role: "r"; hx: root.rx; hy: root.ry; opacity: 0.85 }
}
