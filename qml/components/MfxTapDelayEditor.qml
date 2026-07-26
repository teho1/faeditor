import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

/**
 * Multi-tap delay designer for MFX type 36 (3Tap Pan Delay).
 * All edit state comes from mfx.tapDelay (C++ NOTIFY props) — no paramValues indexing.
 */
ColumnLayout {
    id: root
    spacing: 12

    // One-way from parent only — never bind mfx: mfx (self-loop via name shadowing).
    required property MfxModel mfx
    property bool editorEnabled: true

    readonly property MfxTapDelayModel tap: mfx ? mfx.tapDelay : null

    property int selectedTap: 1 // 0=L, 1=C, 2=R — center default (feedback path)
    property bool draggingTap: false

    readonly property string selectedTapLabel: selectedTap === 0 ? "L"
                                              : selectedTap === 2 ? "R" : "C"
    readonly property int selectedTime: !tap ? 0
                                        : selectedTap === 0 ? tap.leftTime
                                        : selectedTap === 2 ? tap.rightTime
                                        : tap.centerTime
    readonly property int selectedLevel: !tap ? 0
                                         : selectedTap === 0 ? tap.leftLevel
                                         : selectedTap === 2 ? tap.rightLevel
                                         : tap.centerLevel

    RowLayout {
        Layout.fillWidth: true
        spacing: 10
        Label {
            text: "3-Tap Pan Delay"
            color: LogicTheme.textPrimary
            font.bold: true
            font.pixelSize: LogicTheme.fontSize
        }
        Label {
            text: "Drag ↔ time · ↕ level"
            color: LogicTheme.textMuted
            font.pixelSize: LogicTheme.fontSizeSmall
            Layout.fillWidth: true
        }
        Label {
            text: root.selectedTapLabel + "  ·  "
                  + MfxUiFamily.formatDelayTime(root.selectedTime)
                  + "  ·  Lv " + root.selectedLevel
            color: root.draggingTap ? LogicTheme.accent : LogicTheme.textSecondary
            font.pixelSize: LogicTheme.fontSizeSmall
            font.bold: root.draggingTap
        }
    }

    // ---- Timeline ----
    Item {
        id: timelineHost
        Layout.fillWidth: true
        Layout.preferredHeight: 200
        enabled: root.editorEnabled && root.tap
        clip: false

        readonly property real padL: 36
        readonly property real padR: 14
        readonly property real padT: 18
        readonly property real padB: 28
        readonly property real plotW: Math.max(1, width - padL - padR)
        readonly property real plotH: Math.max(1, height - padT - padB)

        function xForTime(t) {
            if (!root.tap) return padL
            const ms = root.tap.delayTimeForAxis(t)
            return padL + (ms / MfxUiFamily.delayMsMax) * plotW
        }
        function yForLevel(lv) {
            return padT + plotH * (1 - Math.max(0, Math.min(127, lv)) / 127)
        }
        function applyPointer(mx, my) {
            if (!root.tap) return
            const nx = (mx - padL) / Math.max(1, plotW)
            const ny = 1 - (my - padT) / Math.max(1, plotH)
            root.tap.setTime(root.selectedTap, root.tap.timeFromAxisNorm(nx))
            root.tap.setLevelAt(root.selectedTap, root.tap.levelFromAxisNorm(ny))
        }

        Canvas {
            id: timelineCanvas
            anchors.fill: parent
            antialiasing: true

            onPaint: {
                const ctx = getContext("2d")
                ctx.reset()
                if (!root.tap) return

                ctx.fillStyle = LogicTheme.dark ? "#1A1A1C" : "#E8E8ED"
                ctx.strokeStyle = LogicTheme.hairline
                ctx.lineWidth = 1
                ctx.fillRect(timelineHost.padL, timelineHost.padT, timelineHost.plotW, timelineHost.plotH)
                ctx.strokeRect(timelineHost.padL, timelineHost.padT, timelineHost.plotW, timelineHost.plotH)

                ctx.strokeStyle = LogicTheme.dark ? "#2A2A2E" : "#D8D8DC"
                for (let g = 1; g < 4; ++g) {
                    const y = timelineHost.padT + timelineHost.plotH * g / 4
                    ctx.beginPath()
                    ctx.moveTo(timelineHost.padL, y)
                    ctx.lineTo(timelineHost.padL + timelineHost.plotW, y)
                    ctx.stroke()
                }
                const ticks = [0, 650, 1300, 1950, 2600]
                ctx.fillStyle = LogicTheme.textMuted
                ctx.font = LogicTheme.fontSizeSmall + "px sans-serif"
                ctx.textAlign = "center"
                for (let i = 0; i < ticks.length; ++i) {
                    const x = timelineHost.xForTime(ticks[i])
                    ctx.strokeStyle = LogicTheme.dark ? "#2A2A2E" : "#D8D8DC"
                    ctx.beginPath()
                    ctx.moveTo(x, timelineHost.padT)
                    ctx.lineTo(x, timelineHost.padT + timelineHost.plotH)
                    ctx.stroke()
                    ctx.fillText(ticks[i] + (i === ticks.length - 1 ? " ms" : ""), x, height - 8)
                }

                const cx = timelineHost.xForTime(root.tap.centerTime)
                const cy = timelineHost.yForLevel(root.tap.centerLevel)
                const fb = Math.abs(root.tap.feedback) / 98
                if (fb > 0.02) {
                    ctx.strokeStyle = LogicTheme.warning
                    ctx.globalAlpha = 0.25 + fb * 0.55
                    ctx.lineWidth = 1.5
                    ctx.beginPath()
                    const r = 18 + fb * 22
                    ctx.arc(cx - r * 0.35, cy, r, Math.PI * 0.15, Math.PI * 1.65, false)
                    ctx.stroke()
                    ctx.beginPath()
                    ctx.moveTo(cx - r * 0.35 + r * Math.cos(Math.PI * 0.15),
                               cy + r * Math.sin(Math.PI * 0.15))
                    ctx.lineTo(cx - 4, cy + 10)
                    ctx.lineTo(cx - 14, cy + 2)
                    ctx.stroke()
                    ctx.globalAlpha = 1
                }

                const taps = [
                    { t: root.tap.leftTime, l: root.tap.leftLevel, col: "#5AC8FA" },
                    { t: root.tap.centerTime, l: root.tap.centerLevel, col: LogicTheme.accent },
                    { t: root.tap.rightTime, l: root.tap.rightLevel, col: "#FF9F0A" }
                ]
                for (let i = 0; i < taps.length; ++i) {
                    const x = timelineHost.xForTime(taps[i].t)
                    const y = timelineHost.yForLevel(taps[i].l)
                    const sel = root.selectedTap === i
                    ctx.strokeStyle = taps[i].col
                    ctx.globalAlpha = sel ? 0.85 : 0.4
                    ctx.lineWidth = sel ? 2 : 1
                    ctx.beginPath()
                    ctx.moveTo(x, timelineHost.padT + timelineHost.plotH)
                    ctx.lineTo(x, y)
                    ctx.stroke()
                    ctx.globalAlpha = 1
                }

                ctx.fillStyle = LogicTheme.textMuted
                ctx.font = LogicTheme.fontSizeSmall + "px sans-serif"
                ctx.save()
                ctx.translate(12, timelineHost.padT + timelineHost.plotH / 2)
                ctx.rotate(-Math.PI / 2)
                ctx.textAlign = "center"
                ctx.fillText("Level", 0, 0)
                ctx.restore()
            }

            function repaint() { requestPaint() }

            Connections {
                target: root.tap
                ignoreUnknownSignals: true
                function onTapChanged() { timelineCanvas.repaint() }
                function onLeftTimeChanged() { timelineCanvas.repaint() }
                function onRightTimeChanged() { timelineCanvas.repaint() }
                function onCenterTimeChanged() { timelineCanvas.repaint() }
                function onLeftLevelChanged() { timelineCanvas.repaint() }
                function onRightLevelChanged() { timelineCanvas.repaint() }
                function onCenterLevelChanged() { timelineCanvas.repaint() }
                function onFeedbackChanged() { timelineCanvas.repaint() }
            }
            Connections {
                target: root
                function onSelectedTapChanged() { timelineCanvas.repaint() }
            }
            Connections {
                target: timelineHost
                function onWidthChanged() { timelineCanvas.repaint() }
                function onHeightChanged() { timelineCanvas.repaint() }
            }
            Component.onCompleted: requestPaint()
        }

        MouseArea {
            anchors.fill: parent
            anchors.leftMargin: timelineHost.padL
            anchors.rightMargin: timelineHost.padR
            anchors.topMargin: timelineHost.padT
            anchors.bottomMargin: timelineHost.padB
            z: 1
            acceptedButtons: Qt.LeftButton
            onClicked: (mouse) => {
                if (!root.tap) return
                const mx = mouse.x + timelineHost.padL
                const my = mouse.y + timelineHost.padT
                let best = 0
                let bestD = 1e9
                for (let i = 0; i < 3; ++i) {
                    const x = timelineHost.xForTime(root.tap.timeOf(i))
                    const y = timelineHost.yForLevel(root.tap.levelOf(i))
                    const d = (x - mx) * (x - mx) + (y - my) * (y - my)
                    if (d < bestD) { bestD = d; best = i }
                }
                root.selectedTap = best
            }
        }

        component TapHandle: Item {
            id: handle
            property int tapIndex: 0
            property color tapColor: LogicTheme.accent
            property string label: "C"

            readonly property int tVal: {
                if (!root.tap) return 0
                if (tapIndex === 0) return root.tap.leftTime
                if (tapIndex === 2) return root.tap.rightTime
                return root.tap.centerTime
            }
            readonly property int lVal: {
                if (!root.tap) return 0
                if (tapIndex === 0) return root.tap.leftLevel
                if (tapIndex === 2) return root.tap.rightLevel
                return root.tap.centerLevel
            }
            x: timelineHost.xForTime(tVal) - width / 2
            y: timelineHost.yForLevel(lVal) - height / 2
            width: 18
            height: 18
            z: (root.draggingTap && root.selectedTap === tapIndex) ? 5
               : (root.selectedTap === tapIndex ? 4 : 3)

            Rectangle {
                anchors.centerIn: parent
                width: root.selectedTap === tapIndex ? 16 : 13
                height: width
                radius: width / 2
                color: LogicTheme.panelBgRaised
                border.color: handle.tapColor
                border.width: root.selectedTap === tapIndex ? 2.5 : 2
            }

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.top
                anchors.bottomMargin: 2
                width: panLab.implicitWidth + 8
                height: 14
                radius: 3
                color: LogicTheme.panelBgRaised
                border.color: handle.tapColor
                border.width: 1
                Label {
                    id: panLab
                    anchors.centerIn: parent
                    text: handle.label
                    color: handle.tapColor
                    font.pixelSize: 9
                    font.bold: true
                }
            }

            MouseArea {
                anchors.fill: parent
                anchors.margins: -14
                cursorShape: Qt.SizeAllCursor
                preventStealing: true
                acceptedButtons: Qt.LeftButton

                function applyAt(mouse) {
                    const p = mapToItem(timelineHost, mouse.x, mouse.y)
                    root.selectedTap = handle.tapIndex
                    timelineHost.applyPointer(p.x, p.y)
                }

                onPressed: (mouse) => {
                    root.selectedTap = handle.tapIndex
                    root.draggingTap = true
                    applyAt(mouse)
                }
                onPositionChanged: (mouse) => {
                    if (!pressed) return
                    applyAt(mouse)
                }
                onReleased: root.draggingTap = false
                onCanceled: root.draggingTap = false
            }
        }

        TapHandle { tapIndex: 0; tapColor: "#5AC8FA"; label: "L" }
        TapHandle { tapIndex: 1; tapColor: LogicTheme.accent; label: "C" }
        TapHandle { tapIndex: 2; tapColor: "#FF9F0A"; label: "R" }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 8
        Label {
            text: "Pan"
            color: LogicTheme.textSecondary
            font.pixelSize: LogicTheme.fontSizeSmall
        }
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            Rectangle {
                anchors.fill: parent
                radius: LogicTheme.radius
                color: LogicTheme.dark ? "#1A1A1C" : "#E8E8ED"
                border.color: LogicTheme.hairline
            }
            Repeater {
                model: [
                    { tap: 0, x: 0.12, c: "#5AC8FA", lab: "L" },
                    { tap: 1, x: 0.5, c: LogicTheme.accent, lab: "C" },
                    { tap: 2, x: 0.88, c: "#FF9F0A", lab: "R" }
                ]
                Rectangle {
                    required property var modelData
                    width: root.selectedTap === modelData.tap ? 22 : 16
                    height: width
                    radius: width / 2
                    color: LogicTheme.panelBgRaised
                    border.color: modelData.c
                    border.width: root.selectedTap === modelData.tap ? 2.5 : 1.5
                    x: parent.width * modelData.x - width / 2
                    anchors.verticalCenter: parent.verticalCenter
                    Label {
                        anchors.centerIn: parent
                        text: modelData.lab
                        color: modelData.c
                        font.pixelSize: 9
                        font.bold: true
                    }
                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -6
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.selectedTap = modelData.tap
                    }
                }
            }
        }
        Label {
            text: "fixed L / C / R"
            color: LogicTheme.textMuted
            font.pixelSize: LogicTheme.fontSizeSmall
        }
    }

    GridLayout {
        Layout.fillWidth: true
        columns: 3
        columnSpacing: 12
        rowSpacing: 6
        enabled: root.editorEnabled && root.tap

        Repeater {
            model: [
                { tap: 0, title: "LEFT", color: "#5AC8FA" },
                { tap: 1, title: "CENTER", color: LogicTheme.accent },
                { tap: 2, title: "RIGHT", color: "#FF9F0A" }
            ]
            ColumnLayout {
                required property var modelData
                Layout.fillWidth: true
                spacing: 4

                Label {
                    text: modelData.title
                    color: modelData.color
                    font.bold: true
                    font.pixelSize: LogicTheme.fontSizeSmall
                }
                Label {
                    text: "Time"
                    color: LogicTheme.textMuted
                    font.pixelSize: LogicTheme.fontSizeSmall
                }
                SpinBox {
                    Layout.fillWidth: true
                    from: 0
                    to: MfxUiFamily.delayMsMax
                    readonly property int tapTime: !root.tap ? 0
                        : modelData.tap === 0 ? root.tap.leftTime
                        : modelData.tap === 2 ? root.tap.rightTime
                        : root.tap.centerTime
                    value: MfxUiFamily.isDelayNote(tapTime)
                           ? MfxUiFamily.delayMsMax
                           : Math.max(0, Math.min(MfxUiFamily.delayMsMax, tapTime))
                    editable: true
                    textFromValue: (v) => MfxUiFamily.formatDelayTime(
                        tapTime > MfxUiFamily.delayMsMax ? tapTime : v)
                    valueFromText: (text) => {
                        const n = parseInt(text)
                        return isNaN(n) ? 0 : n
                    }
                    onValueModified: if (root.tap) root.tap.setTime(modelData.tap, value)
                }
                Label {
                    readonly property int tapTime: !root.tap ? 0
                        : modelData.tap === 0 ? root.tap.leftTime
                        : modelData.tap === 2 ? root.tap.rightTime
                        : root.tap.centerTime
                    visible: MfxUiFamily.isDelayNote(tapTime)
                    text: MfxUiFamily.formatDelayTime(tapTime) + " (edit → ms)"
                    color: LogicTheme.warning
                    font.pixelSize: 10
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
                Label {
                    text: "Level"
                    color: LogicTheme.textMuted
                    font.pixelSize: LogicTheme.fontSizeSmall
                }
                SpinBox {
                    Layout.fillWidth: true
                    from: 0
                    to: 127
                    value: !root.tap ? 0
                           : modelData.tap === 0 ? root.tap.leftLevel
                           : modelData.tap === 2 ? root.tap.rightLevel
                           : root.tap.centerLevel
                    editable: true
                    onValueModified: if (root.tap) root.tap.setLevelAt(modelData.tap, value)
                }
                Label {
                    text: "Pan  ·  " + modelData.title.charAt(0)
                    color: LogicTheme.textMuted
                    font.pixelSize: LogicTheme.fontSizeSmall
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 16
        enabled: root.editorEnabled && root.tap

        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            spacing: 6

            Label {
                text: "Feedback"
                color: LogicTheme.textPrimary
                font.bold: true
                font.pixelSize: LogicTheme.fontSize
            }
            Label {
                text: (root.tap ? root.tap.feedbackLabel : "+0 %") + "  ·  center tap loop"
                color: LogicTheme.textSecondary
                font.pixelSize: LogicTheme.fontSizeSmall
            }
            Slider {
                id: fbSlider
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                from: -98
                to: 98
                stepSize: 1
                value: root.tap ? root.tap.feedback : 0
                onMoved: if (root.tap) root.tap.feedback = Math.round(value)

                background: Item {
                    x: fbSlider.leftPadding
                    y: fbSlider.topPadding + fbSlider.availableHeight / 2 - height / 2
                    width: fbSlider.availableWidth
                    height: 6
                    Rectangle {
                        anchors.fill: parent
                        radius: 3
                        color: LogicTheme.faderTrack
                    }
                    Rectangle {
                        width: 2
                        height: parent.height + 8
                        anchors.verticalCenter: parent.verticalCenter
                        x: parent.width * 0.5 - 1
                        color: LogicTheme.textMuted
                    }
                    Rectangle {
                        width: Math.abs(fbSlider.visualPosition - 0.5) * parent.width
                        height: parent.height
                        radius: 3
                        color: LogicTheme.warning
                        opacity: 0.85
                        x: fbSlider.visualPosition < 0.5
                           ? fbSlider.visualPosition * parent.width
                           : parent.width * 0.5
                    }
                }
                handle: Rectangle {
                    x: fbSlider.leftPadding + fbSlider.visualPosition * (fbSlider.availableWidth - width)
                    y: fbSlider.topPadding + fbSlider.availableHeight / 2 - height / 2
                    width: 18
                    height: 18
                    radius: 9
                    color: LogicTheme.panelBgRaised
                    border.color: LogicTheme.warning
                    border.width: 2
                }
            }
            SpinBox {
                from: -98
                to: 98
                value: root.tap ? root.tap.feedback : 0
                editable: true
                textFromValue: (v) => MfxUiFamily.formatFeedback(v)
                valueFromText: (text) => {
                    const n = parseInt(text)
                    return isNaN(n) ? 0 : n
                }
                onValueModified: if (root.tap) root.tap.feedback = value
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            spacing: 6

            Label {
                text: "HF Damp"
                color: LogicTheme.textPrimary
                font.bold: true
                font.pixelSize: LogicTheme.fontSize
            }
            Label {
                text: root.tap ? root.tap.hfDampLabel : "BYPASS"
                color: LogicTheme.textSecondary
                font.pixelSize: LogicTheme.fontSizeSmall
            }

            Item {
                id: dampHost
                Layout.fillWidth: true
                Layout.preferredHeight: 72

                Canvas {
                    id: dampCanvas
                    anchors.fill: parent
                    antialiasing: true

                    onPaint: {
                        const ctx = getContext("2d")
                        ctx.reset()
                        if (!root.tap) return
                        const pad = 6
                        const w = width - pad * 2
                        const h = height - pad * 2
                        ctx.fillStyle = LogicTheme.dark ? "#1A1A1C" : "#E8E8ED"
                        ctx.strokeStyle = LogicTheme.hairline
                        ctx.fillRect(pad, pad, w, h)
                        ctx.strokeRect(pad, pad, w, h)

                        const bypass = root.tap.hfDampBypass
                        const cutN = root.tap.hfDampNorm
                        const samples = 64
                        ctx.beginPath()
                        for (let i = 0; i <= samples; ++i) {
                            const t = i / samples
                            const x = pad + t * w
                            const mag = root.tap.hfDampCurveMagnitude(t)
                            const y = pad + h * (1 - mag)
                            if (i === 0) ctx.moveTo(x, y)
                            else ctx.lineTo(x, y)
                        }
                        ctx.strokeStyle = bypass ? LogicTheme.textMuted : LogicTheme.accent
                        ctx.lineWidth = 2
                        ctx.stroke()

                        if (!bypass) {
                            const cx = pad + cutN * w
                            ctx.beginPath()
                            ctx.arc(cx, pad + h * 0.15, 4, 0, Math.PI * 2)
                            ctx.fillStyle = LogicTheme.accent
                            ctx.fill()
                        }

                        ctx.fillStyle = LogicTheme.textMuted
                        ctx.font = "10px sans-serif"
                        ctx.textAlign = "left"
                        ctx.fillText("200", pad, height - 2)
                        ctx.textAlign = "right"
                        ctx.fillText(bypass ? "BYPASS" : "8k", pad + w, height - 2)
                    }

                    Connections {
                        target: root.tap
                        ignoreUnknownSignals: true
                        function onHfDampChanged() { dampCanvas.requestPaint() }
                    }
                    Connections {
                        target: dampHost
                        function onWidthChanged() { dampCanvas.requestPaint() }
                        function onHeightChanged() { dampCanvas.requestPaint() }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onPositionChanged: (mouse) => {
                        if (!pressed || !root.tap) return
                        const pad = 6
                        const n = Math.max(0, Math.min(1, (mouse.x - pad) / Math.max(1, width - pad * 2)))
                        root.tap.setHfDampFromNorm(n)
                    }
                    onClicked: (mouse) => { positionChanged(mouse) }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                SpinBox {
                    Layout.fillWidth: true
                    from: MfxUiFamily.hfDampMinHz
                    to: MfxUiFamily.hfDampMaxHz
                    value: {
                        if (!root.tap || root.tap.hfDampBypass)
                            return MfxUiFamily.hfDampMaxHz
                        return root.tap.hfDamp
                    }
                    editable: true
                    textFromValue: (v) => v + " Hz"
                    valueFromText: (text) => {
                        const n = parseInt(text)
                        return isNaN(n) ? MfxUiFamily.hfDampMinHz : n
                    }
                    onValueModified: if (root.tap) root.tap.hfDamp = value
                }
                CheckBox {
                    text: "BYPASS"
                    checked: root.tap ? root.tap.hfDampBypass : true
                    onToggled: if (root.tap) root.tap.hfDampBypass = checked
                }
            }
        }
    }

    GroupBox {
        Layout.fillWidth: true
        title: "Output"
        enabled: root.editorEnabled && root.tap

        background: Rectangle {
            y: parent.topPadding - parent.bottomPadding
            width: parent.width
            height: parent.height - parent.topPadding + parent.bottomPadding
            color: "transparent"
            border.color: LogicTheme.hairline
            radius: LogicTheme.radius
        }
        label: Label {
            text: "Output"
            color: LogicTheme.textPrimary
            font.bold: true
            font.pixelSize: LogicTheme.fontSize
            leftPadding: 4
        }

        RowLayout {
            anchors.fill: parent
            spacing: 12

            MfxParamFader {
                label: "Low Gain"
                from: -15; to: 15
                value: root.tap ? root.tap.lowGain : 0
                onMoved: (v) => { if (root.tap) root.tap.lowGain = v }
            }
            MfxParamFader {
                label: "High Gain"
                from: -15; to: 15
                value: root.tap ? root.tap.highGain : 0
                onMoved: (v) => { if (root.tap) root.tap.highGain = v }
            }
            MfxParamFader {
                label: "Balance"
                from: 0; to: 100
                value: root.tap ? root.tap.balance : 50
                onMoved: (v) => { if (root.tap) root.tap.balance = v }
            }
            MfxParamFader {
                label: "Level"
                from: 0; to: 127
                value: root.tap ? root.tap.level : 0
                onMoved: (v) => { if (root.tap) root.tap.level = v }
            }
            MfxParamFader {
                label: "Chorus"
                from: 0; to: 127
                value: root.tap ? root.tap.chorusSend : 0
                onMoved: (v) => { if (root.tap) root.tap.chorusSend = v }
            }
            MfxParamFader {
                label: "Reverb"
                from: 0; to: 127
                value: root.tap ? root.tap.reverbSend : 0
                onMoved: (v) => { if (root.tap) root.tap.reverbSend = v }
            }
            Item { Layout.fillWidth: true }
        }
    }
}
