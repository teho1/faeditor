import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    id: root
    color: LogicTheme.windowBg

    property string selectedBlock: "reverb"

    readonly property var part: App.studioSet.selectedPartModel
    readonly property var fx: App.studioSet.effects
    readonly property var audio: App.audioFx

    readonly property string partLabel: part
        ? ("Part " + (App.studioSet.selectedPart + 1))
        : "Part —"
    readonly property string toneLabel: part && part.toneName.length
        ? part.toneName
        : "(no tone)"

    // audio.tfxLocation: 0 = MAIN (keyboard), 1 = INPUT (guitar/line)
    readonly property bool tfxOnInput: audio.tfxLocation === 1

    function selectBlock(id) {
        selectedBlock = id
    }

    function pad2(n) {
        return (n < 10 ? "0" : "") + n
    }

    function tfxTypeLabel() {
        const names = audio.tfxTypeNames
        const i = audio.tfxType
        if (i >= 0 && i < names.length)
            return names[i]
        return "—"
    }

    function micReverbLabel() {
        const names = audio.inputReverbTypeNames
        const i = audio.inputReverbType
        if (i >= 0 && i < names.length)
            return pad2(i + 1) + ":" + names[i]
        return "—"
    }

    function chorusLabel() {
        const names = fx.chorusTypeNames
        const i = fx.chorusType
        if (i >= 0 && i < names.length)
            return names[i]
        return pad2(i) + ":?"
    }

    function reverbLabel() {
        const names = fx.reverbTypeNames
        const i = fx.reverbType
        if (i >= 0 && i < names.length)
            return names[i]
        return pad2(i) + ":?"
    }

    Component.onCompleted: {
        if (App.midi.connected)
            App.audioFx.pullFromDevice()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        // Header — FA EFFECTS EDIT style context
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Label {
                text: "EFFECTS EDIT"
                color: LogicTheme.accent
                font.pixelSize: LogicTheme.fontSizeTitle
                font.bold: true
            }

            Rectangle {
                width: 1
                height: 16
                color: LogicTheme.hairline
            }

            Label {
                text: root.partLabel
                color: LogicTheme.textPrimary
                font.pixelSize: LogicTheme.fontSize
                font.bold: true
            }

            Label {
                text: root.toneLabel
                color: LogicTheme.textSecondary
                font.pixelSize: LogicTheme.fontSize
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Label {
                visible: audio.lastError.length > 0
                text: audio.lastError
                color: LogicTheme.danger
                font.pixelSize: LogicTheme.fontSizeSmall
                elide: Text.ElideRight
                Layout.maximumWidth: 220
            }

            Button {
                text: "Pull Audio FX"
                onClicked: App.audioFx.pullFromDevice()
            }
            Button {
                text: "Push Audio FX"
                onClicked: App.audioFx.pushToDevice()
            }
        }

        // Signal-flow diagram (centered content; TFX placement follows tfxLocation)
        Rectangle {
            id: diagramFrame
            Layout.fillWidth: true
            Layout.preferredHeight: 280
            Layout.minimumHeight: 240
            color: LogicTheme.panelBg
            radius: LogicTheme.radius
            border.color: LogicTheme.hairline
            border.width: 1
            clip: true

            // Fixed-width graph, horizontally centered so Output is not pinned to the frame edge.
            Item {
                id: diagramContent
                width: root.tfxOnInput ? 670 : 790
                height: parent.height
                x: Math.max(12, Math.round((parent.width - width) / 2))
                y: 0

                Canvas {
                    id: wires
                    anchors.fill: parent
                    z: -1
                    // Geometry only — must not steal clicks from Part / Output ComboBoxes.

                    onPaint: {
                        const ctx = getContext("2d")
                        ctx.reset()
                        ctx.strokeStyle = LogicTheme.dark ? "#5A7A9A" : "#7A9AB8"
                        ctx.lineWidth = 1.5
                        ctx.lineJoin = "round"
                        ctx.lineCap = "round"

                        function mid(item) {
                            return {
                                x: item.x + item.width / 2,
                                y: item.y + item.height / 2,
                                r: item.x + item.width,
                                l: item.x,
                                t: item.y,
                                b: item.y + item.height
                            }
                        }

                        function line(x1, y1, x2, y2) {
                            ctx.beginPath()
                            ctx.moveTo(x1, y1)
                            ctx.lineTo(x2, y2)
                            ctx.stroke()
                        }

                        function elbow(x1, y1, x2, y2, midX) {
                            const mx = midX !== undefined ? midX : (x1 + x2) / 2
                            ctx.beginPath()
                            ctx.moveTo(x1, y1)
                            ctx.lineTo(mx, y1)
                            ctx.lineTo(mx, y2)
                            ctx.lineTo(x2, y2)
                            ctx.stroke()
                        }

                        const part = mid(partBlock)
                        const cho = mid(chorusBlock)
                        const rev = mid(reverbBlock)
                        const mcomp = mid(masterCompBlock)
                        const out = mid(outputBlock)
                        const ain = mid(audioInBlock)
                        const ns = mid(nsBlock)
                        const tfx = mid(tfxBlock)
                        const mic = mid(micReverbBlock)
                        const onInput = root.tfxOnInput

                        // Part → chorus / reverb
                        line(part.r, cho.y, cho.l, cho.y)
                        line(part.r, rev.y, rev.l, rev.y)

                        if (onInput) {
                            // Chorus / Reverb → Master Comp → Output
                            const busX = cho.r + 24
                            line(cho.r, cho.y, busX, cho.y)
                            line(rev.r, rev.y, busX, rev.y)
                            line(busX, cho.y, busX, rev.y)
                            line(busX, mcomp.y, mcomp.l, mcomp.y)
                            line(mcomp.r, mcomp.y, out.l, out.y)

                            // Audio Input → NS → TFX → MIC Reverb → Output
                            line(ain.r, ain.y, ns.l, ns.y)
                            line(ns.r, ns.y, tfx.l, tfx.y)
                            line(tfx.r, tfx.y, mic.l, mic.y)
                            elbow(mic.r, mic.y, out.l, out.y, mcomp.r + 20)
                        } else {
                            // Chorus / Reverb → TFX (MAIN) → Master Comp → Output
                            const busX = cho.r + 20
                            line(cho.r, cho.y, busX, cho.y)
                            line(rev.r, rev.y, busX, rev.y)
                            line(busX, cho.y, busX, rev.y)
                            line(busX, tfx.y, tfx.l, tfx.y)

                            line(tfx.r, tfx.y, mcomp.l, mcomp.y)
                            line(mcomp.r, mcomp.y, out.l, out.y)

                            // Audio Input → NS → MIC Reverb → Output (TFX on main)
                            line(ain.r, ain.y, ns.l, ns.y)
                            line(ns.r, ns.y, mic.l, mic.y)
                            elbow(mic.r, mic.y, out.l, out.y, mcomp.r + 20)
                        }
                    }

                    Connections {
                        target: diagramContent
                        function onWidthChanged() { wires.requestPaint() }
                        function onHeightChanged() { wires.requestPaint() }
                        function onXChanged() { wires.requestPaint() }
                    }
                    Connections {
                        target: root
                        function onTfxOnInputChanged() { wires.requestPaint() }
                    }
                    Connections {
                        target: audio
                        function onAudioFxChanged() { wires.requestPaint() }
                    }
                }

                // —— Part path (top): part picker ——
                Rectangle {
                    id: partBlock
                    x: 0
                    // Span Chorus→Reverb wire midpoints so both send lines meet this box.
                    y: 18
                    width: 148
                    height: 112
                    z: 2
                    radius: 4
                    color: LogicTheme.panelBgRaised
                    border.color: LogicTheme.hairline
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 6
                        spacing: 4

                        Label {
                            text: "Part"
                            color: LogicTheme.textPrimary
                            font.pixelSize: LogicTheme.fontSizeSmall
                            font.bold: true
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: partPicker
                            Layout.fillWidth: true
                            Layout.preferredHeight: 28
                            Layout.alignment: Qt.AlignTop
                            model: App.studioSet
                            textRole: "toneName"
                            // Avoid a two-way binding fight that can block opening/choosing.
                            Component.onCompleted: currentIndex = App.studioSet.selectedPart
                            displayText: {
                                const p = App.studioSet.part(currentIndex)
                                if (!p)
                                    return "Part " + (currentIndex + 1)
                                return (currentIndex + 1) + " · " + p.toneName
                            }
                            delegate: ItemDelegate {
                                required property int index
                                required property int partNumber
                                required property string toneName
                                width: ListView.view ? ListView.view.width : partPicker.width
                                text: partNumber + " · " + toneName
                                highlighted: partPicker.highlightedIndex === index
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                            Connections {
                                target: App.studioSet
                                function onSelectedPartChanged() {
                                    if (partPicker.currentIndex !== App.studioSet.selectedPart)
                                        partPicker.currentIndex = App.studioSet.selectedPart
                                }
                            }
                            onActivated: (index) => {
                                App.studioSet.selectedPart = index
                                currentIndex = index
                            }
                        }

                        Button {
                            text: "MFX → Tone"
                            Layout.fillWidth: true
                            Layout.preferredHeight: 26
                            font.pixelSize: LogicTheme.fontSizeSmall
                            onClicked: {
                                root.selectBlock("mfx")
                                App.tone.openMfxStage()
                                App.mainTab = 3
                            }
                        }
                    }

                    // Clicking empty chrome opens the part list (same idea as Output).
                    MouseArea {
                        anchors.fill: parent
                        z: -1
                        cursorShape: Qt.PointingHandCursor
                        onClicked: partPicker.popup.open()
                    }

                    onXChanged: wires.requestPaint()
                    onYChanged: wires.requestPaint()
                    onWidthChanged: wires.requestPaint()
                    onHeightChanged: wires.requestPaint()
                }

                Label {
                    id: chorusSendLabel
                    z: 3
                    // Center in the Part→Chorus gap (above the wire).
                    x: partBlock.x + partBlock.width
                       + Math.max(2, Math.round((chorusBlock.x - partBlock.x - partBlock.width - implicitWidth) / 2))
                    y: chorusBlock.y + (chorusBlock.height - height) / 2 - 12
                    text: part ? String(part.chorusSend) : "—"
                    color: LogicTheme.danger
                    font.pixelSize: LogicTheme.fontSizeSmall
                    font.bold: true
                }

                Label {
                    id: reverbSendLabel
                    z: 3
                    x: partBlock.x + partBlock.width
                       + Math.max(2, Math.round((reverbBlock.x - partBlock.x - partBlock.width - implicitWidth) / 2))
                    y: reverbBlock.y + (reverbBlock.height - height) / 2 - 12
                    text: part ? String(part.reverbSend) : "—"
                    color: LogicTheme.danger
                    font.pixelSize: LogicTheme.fontSizeSmall
                    font.bold: true
                }

                FxNode {
                    id: chorusBlock
                    // Leave room for send amounts between Part and Chorus/Reverb.
                    x: 198
                    y: 18
                    width: 130
                    height: 52
                    title: "Chorus"
                    detail: root.chorusLabel()
                    editable: true
                    selected: root.selectedBlock === "chorus"
                    ledOn: fx.chorusLevel > 0
                    onClicked: root.selectBlock("chorus")
                }

                FxNode {
                    id: reverbBlock
                    x: 198
                    y: 78
                    width: 130
                    height: 52
                    title: "Reverb"
                    detail: root.reverbLabel()
                    editable: true
                    selected: root.selectedBlock === "reverb"
                    ledOn: fx.reverbLevel > 0
                    onClicked: root.selectBlock("reverb")
                }

                // TFX: on input path (bottom) or main path (between Cho/Rev and Master)
                FxNode {
                    id: tfxBlock
                    x: root.tfxOnInput ? 202 : 360
                    y: root.tfxOnInput ? 160 : 48
                    width: 130
                    height: 52
                    title: "TFX"
                    detail: root.tfxTypeLabel()
                    editable: true
                    selected: root.selectedBlock === "tfx"
                    ledOn: audio.tfxSwitch
                    onXChanged: wires.requestPaint()
                    onYChanged: wires.requestPaint()
                    onClicked: root.selectBlock("tfx")
                }

                FxNode {
                    id: masterCompBlock
                    x: root.tfxOnInput ? 378 : 520
                    y: 48
                    width: 120
                    height: 52
                    title: "Master Comp"
                    detail: fx.masterCompSwitch ? "ON" : "OFF"
                    editable: true
                    selected: root.selectedBlock === "masterComp"
                    ledOn: fx.masterCompSwitch
                    onXChanged: wires.requestPaint()
                    onYChanged: wires.requestPaint()
                    onClicked: root.selectBlock("masterComp")
                }

                // Output — same unselected/selected chrome as other FxNode effect blocks
                Rectangle {
                    id: outputBlock
                    x: root.tfxOnInput ? 538 : 680
                    y: 48
                    width: 122
                    height: 72
                    radius: 4
                    color: root.selectedBlock === "output"
                           ? LogicTheme.selectedBg
                           : LogicTheme.panelBgRaised
                    border.color: root.selectedBlock === "output"
                                  ? LogicTheme.accent
                                  : LogicTheme.hairline
                    border.width: root.selectedBlock === "output" ? 2 : 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 6
                        spacing: 4

                        Label {
                            text: "Output"
                            color: LogicTheme.textPrimary
                            font.pixelSize: LogicTheme.fontSizeSmall
                            font.bold: true
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: outputAssignBox
                            Layout.fillWidth: true
                            Layout.preferredHeight: 28
                            model: ["Main", "Sub"]
                            enabled: !!part
                            currentIndex: part ? part.outputAssign : 0
                            onActivated: (index) => {
                                if (part)
                                    part.outputAssign = index
                                root.selectBlock("output")
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        z: -1
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.selectBlock("output")
                    }

                    onXChanged: wires.requestPaint()
                }

                // —— Audio input path (bottom); TFX only when Location = INPUT ——
                Rectangle {
                    id: audioInBlock
                    x: 0
                    y: 160
                    width: 118
                    height: 64
                    radius: 4
                    color: root.selectedBlock === "audioIn"
                           ? LogicTheme.selectedBg
                           : LogicTheme.panelBgRaised
                    border.color: root.selectedBlock === "audioIn"
                                  ? LogicTheme.accent
                                  : LogicTheme.hairline
                    border.width: root.selectedBlock === "audioIn" ? 2 : 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 6
                        spacing: 2

                        Label {
                            text: "Audio Input"
                            color: LogicTheme.textPrimary
                            font.pixelSize: LogicTheme.fontSizeSmall
                            font.bold: true
                            Layout.fillWidth: true
                        }

                        ComboBox {
                            id: inputGainBox
                            Layout.fillWidth: true
                            Layout.preferredHeight: 26
                            model: audio.tfxGainNames
                            currentIndex: audio.tfxInputGain
                            onActivated: (index) => {
                                audio.tfxInputGain = index
                                root.selectBlock("audioIn")
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        z: -1
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.selectBlock("audioIn")
                    }

                    onXChanged: wires.requestPaint()
                    onYChanged: wires.requestPaint()
                }

                FxNode {
                    id: nsBlock
                    x: 130
                    y: 172
                    width: 56
                    height: 40
                    title: "NS"
                    detail: audio.nsSwitch ? "ON" : "OFF"
                    editable: true
                    selected: root.selectedBlock === "ns"
                    ledOn: audio.nsSwitch
                    onClicked: root.selectBlock("ns")
                }

                FxNode {
                    id: micReverbBlock
                    // After TFX on input path; after NS when TFX is on MAIN
                    x: root.tfxOnInput ? 378 : 202
                    y: 160
                    width: 120
                    height: 52
                    title: "MIC Reverb"
                    detail: root.micReverbLabel()
                    editable: true
                    selected: root.selectedBlock === "micReverb"
                    ledOn: audio.inputReverbSwitch
                    onXChanged: wires.requestPaint()
                    onClicked: root.selectBlock("micReverb")
                }

                Label {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.margins: 4
                    text: root.tfxOnInput
                          ? "TFX Location: INPUT · Click a block to edit"
                          : "TFX Location: MAIN · Click a block to edit"
                    color: LogicTheme.textMuted
                    font.pixelSize: LogicTheme.fontSizeSmall
                }
            }

            // Repaint wires when layout settles / resizes / TFX location flips
            onWidthChanged: wires.requestPaint()
            onHeightChanged: wires.requestPaint()
            Timer {
                id: wireSettle
                interval: 16
                running: true
                repeat: false
                onTriggered: wires.requestPaint()
            }
            Connections {
                target: root
                function onTfxOnInputChanged() {
                    wireSettle.restart()
                }
            }
        }

        // Parameter editor for selected block
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: LogicTheme.panelBg
            radius: LogicTheme.radius
            border.color: LogicTheme.hairline
            border.width: 1
            clip: true

            Flickable {
                anchors.fill: parent
                anchors.margins: 12
                contentHeight: editorCol.height
                clip: true
                flickableDirection: Flickable.VerticalFlick

                ColumnLayout {
                    id: editorCol
                    width: parent.width
                    spacing: 12

                    Label {
                        text: {
                            switch (root.selectedBlock) {
                            case "mfx": return "MFX"
                            case "chorus": return "Chorus"
                            case "reverb": return "Reverb"
                            case "masterComp": return "Master Compressor"
                            case "tfx": return "TFX (Total FX)"
                            case "micReverb": return "MIC Reverb (Input Reverb)"
                            case "ns": return "Noise Suppressor"
                            case "audioIn": return "Audio Input"
                            case "output": return "Output"
                            default: return "Effects"
                            }
                        }
                        color: LogicTheme.textPrimary
                        font.pixelSize: LogicTheme.fontSizeTitle
                        font.bold: true
                    }

                    // —— MFX deep-link to Tone Edit ——
                    ColumnLayout {
                        visible: root.selectedBlock === "mfx"
                        spacing: 10
                        Layout.fillWidth: true

                        Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            color: LogicTheme.textSecondary
                            font.pixelSize: LogicTheme.fontSize
                            text: "Part MFX (amp models, delays, chorus…) lives in Temporary Tone. Open Tone Edit for presets and parameters."
                        }
                        Button {
                            text: "Open in Tone Edit"
                            onClicked: {
                                App.tone.openMfxStage()
                                App.mainTab = 3
                            }
                        }
                    }

                    // —— Audio Input (gain) ——
                    GridLayout {
                        visible: root.selectedBlock === "audioIn"
                        columns: 2
                        columnSpacing: 12
                        rowSpacing: 8
                        Layout.fillWidth: true

                        Label {
                            text: "Input Gain"
                            color: LogicTheme.textSecondary
                            font.pixelSize: LogicTheme.fontSizeSmall
                        }
                        ComboBox {
                            model: audio.tfxGainNames
                            currentIndex: audio.tfxInputGain
                            onActivated: (index) => { audio.tfxInputGain = index }
                            Layout.fillWidth: true
                        }

                        Label {
                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            color: LogicTheme.textSecondary
                            font.pixelSize: LogicTheme.fontSize
                            text: "Input gain for the audio path (guitar/line). TFX Location still lives on the TFX block. USB Audio options remain on the FA."
                        }
                    }

                    // —— Output (part Main / Sub) ——
                    GridLayout {
                        visible: root.selectedBlock === "output"
                        columns: 2
                        columnSpacing: 12
                        rowSpacing: 8
                        Layout.fillWidth: true

                        Label {
                            text: "Part output"
                            color: LogicTheme.textSecondary
                            font.pixelSize: LogicTheme.fontSizeSmall
                        }
                        ComboBox {
                            model: ["Main", "Sub"]
                            enabled: !!part
                            currentIndex: part ? part.outputAssign : 0
                            onActivated: (index) => { if (part) part.outputAssign = index }
                            Layout.fillWidth: true
                        }

                        Label {
                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            color: LogicTheme.textSecondary
                            font.pixelSize: LogicTheme.fontSize
                            text: "Routes this part to Main or Sub (same as Mixer / Part Inspector). PAD and USB Audio are hardware destinations after Output — configure USB on the FA if needed."
                        }
                    }

                    // —— Chorus ——
                    GridLayout {
                        visible: root.selectedBlock === "chorus"
                        columns: 2
                        columnSpacing: 12
                        rowSpacing: 8
                        Layout.fillWidth: true

                        Label { text: "Type"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        ComboBox {
                            model: fx.chorusTypeNames
                            currentIndex: fx.chorusType
                            onActivated: (index) => { fx.chorusType = index }
                            Layout.fillWidth: true
                        }
                        Label { text: "Level"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        RowLayout {
                            Layout.fillWidth: true
                            Slider {
                                from: 0; to: 127
                                value: fx.chorusLevel
                                onMoved: fx.chorusLevel = Math.round(value)
                                Layout.fillWidth: true
                            }
                            Label {
                                text: String(fx.chorusLevel)
                                color: LogicTheme.textPrimary
                                Layout.preferredWidth: 28
                            }
                        }
                        Label { text: "Part send"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        RowLayout {
                            Layout.fillWidth: true
                            Slider {
                                from: 0; to: 127
                                value: part ? part.chorusSend : 0
                                enabled: !!part
                                onMoved: if (part) part.chorusSend = Math.round(value)
                                Layout.fillWidth: true
                            }
                            Label {
                                text: part ? String(part.chorusSend) : "—"
                                color: LogicTheme.textPrimary
                                Layout.preferredWidth: 28
                            }
                        }
                    }

                    // —— Reverb ——
                    GridLayout {
                        visible: root.selectedBlock === "reverb"
                        columns: 2
                        columnSpacing: 12
                        rowSpacing: 8
                        Layout.fillWidth: true

                        Label { text: "Type"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        ComboBox {
                            model: fx.reverbTypeNames
                            currentIndex: fx.reverbType
                            onActivated: (index) => { fx.reverbType = index }
                            Layout.fillWidth: true
                        }
                        Label { text: "Level"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        RowLayout {
                            Layout.fillWidth: true
                            Slider {
                                from: 0; to: 127
                                value: fx.reverbLevel
                                onMoved: fx.reverbLevel = Math.round(value)
                                Layout.fillWidth: true
                            }
                            Label {
                                text: String(fx.reverbLevel)
                                color: LogicTheme.textPrimary
                                Layout.preferredWidth: 28
                            }
                        }
                        Label { text: "Part send"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        RowLayout {
                            Layout.fillWidth: true
                            Slider {
                                from: 0; to: 127
                                value: part ? part.reverbSend : 0
                                enabled: !!part
                                onMoved: if (part) part.reverbSend = Math.round(value)
                                Layout.fillWidth: true
                            }
                            Label {
                                text: part ? String(part.reverbSend) : "—"
                                color: LogicTheme.textPrimary
                                Layout.preferredWidth: 28
                            }
                        }
                    }

                    // —— Master Comp ——
                    GridLayout {
                        visible: root.selectedBlock === "masterComp"
                        columns: 2
                        columnSpacing: 12
                        rowSpacing: 8
                        Layout.fillWidth: true

                        Label { text: "Switch"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Switch {
                            checked: fx.masterCompSwitch
                            onToggled: fx.masterCompSwitch = checked
                        }
                        Label { text: "Attack"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127
                            value: fx.masterCompAttack
                            onMoved: fx.masterCompAttack = Math.round(value)
                            Layout.fillWidth: true
                        }
                        Label { text: "Release"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127
                            value: fx.masterCompRelease
                            onMoved: fx.masterCompRelease = Math.round(value)
                            Layout.fillWidth: true
                        }
                        Label { text: "Threshold"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127
                            value: fx.masterCompThreshold
                            onMoved: fx.masterCompThreshold = Math.round(value)
                            Layout.fillWidth: true
                        }
                        Label { text: "Ratio"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127
                            value: fx.masterCompRatio
                            onMoved: fx.masterCompRatio = Math.round(value)
                            Layout.fillWidth: true
                        }
                        Label { text: "Gain"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127
                            value: fx.masterCompGain
                            onMoved: fx.masterCompGain = Math.round(value)
                            Layout.fillWidth: true
                        }
                    }

                    // —— TFX ——
                    GridLayout {
                        visible: root.selectedBlock === "tfx"
                        columns: 2
                        columnSpacing: 12
                        rowSpacing: 8
                        Layout.fillWidth: true

                        Label { text: "TFX Switch"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Switch {
                            checked: audio.tfxSwitch
                            onToggled: audio.tfxSwitch = checked
                        }
                        Label { text: "Type"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        ComboBox {
                            model: audio.tfxTypeNames
                            currentIndex: audio.tfxType
                            onActivated: (index) => { audio.tfxType = index }
                            Layout.fillWidth: true
                        }
                        Label { text: "TFX Location"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        ComboBox {
                            model: ["MAIN (keyboard)", "INPUT (guitar/line)"]
                            currentIndex: audio.tfxLocation
                            onActivated: (index) => { audio.tfxLocation = index }
                            Layout.fillWidth: true
                        }
                        Label { text: "Ctrl 1 (Cutoff / …)"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127
                            value: audio.tfxParamA
                            onMoved: audio.tfxParamA = Math.round(value)
                            Layout.fillWidth: true
                        }
                        Label { text: "Ctrl 2 (Reso / …)"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127
                            value: audio.tfxParamB
                            onMoved: audio.tfxParamB = Math.round(value)
                            Layout.fillWidth: true
                        }
                        Label { text: "Ctrl 3 (Drive / …)"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127
                            value: audio.tfxParamC
                            onMoved: audio.tfxParamC = Math.round(value)
                            Layout.fillWidth: true
                        }
                    }

                    // —— MIC Reverb ——
                    GridLayout {
                        visible: root.selectedBlock === "micReverb"
                        columns: 2
                        columnSpacing: 12
                        rowSpacing: 8
                        Layout.fillWidth: true

                        Label { text: "Input Reverb"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Switch {
                            checked: audio.inputReverbSwitch
                            onToggled: audio.inputReverbSwitch = checked
                        }
                        Label { text: "Reverb Type"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        ComboBox {
                            model: audio.inputReverbTypeNames
                            currentIndex: audio.inputReverbType
                            onActivated: (index) => { audio.inputReverbType = index }
                            Layout.fillWidth: true
                        }
                        Label { text: "Reverb Time"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127
                            value: audio.inputReverbTime
                            onMoved: audio.inputReverbTime = Math.round(value)
                            Layout.fillWidth: true
                        }
                        Label { text: "Reverb Level"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127
                            value: audio.inputReverbLevel
                            onMoved: audio.inputReverbLevel = Math.round(value)
                            Layout.fillWidth: true
                        }
                    }

                    // —— NS ——
                    GridLayout {
                        visible: root.selectedBlock === "ns"
                        columns: 2
                        columnSpacing: 12
                        rowSpacing: 8
                        Layout.fillWidth: true

                        Label { text: "Noise Suppressor"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Switch {
                            checked: audio.nsSwitch
                            onToggled: audio.nsSwitch = checked
                        }
                        Label { text: "NS Threshold"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127
                            value: audio.nsThreshold
                            onMoved: audio.nsThreshold = Math.round(value)
                            Layout.fillWidth: true
                        }
                        Label { text: "NS Release"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127
                            value: audio.nsRelease
                            onMoved: audio.nsRelease = Math.round(value)
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }

    component FxNode: Rectangle {
        id: node
        property string title: ""
        property string subtitle: ""
        property string detail: ""
        property bool editable: true
        property bool selected: false
        property bool ledOn: false
        property bool structural: false
        property bool accentDetail: false
        signal clicked()

        radius: 4
        color: {
            if (selected)
                return LogicTheme.selectedBg
            if (structural)
                return LogicTheme.dark ? "#252528" : "#E8E8EA"
            return LogicTheme.panelBgRaised
        }
        border.color: selected ? LogicTheme.accent : LogicTheme.hairline
        border.width: selected ? 2 : 1
        opacity: editable || structural ? 1 : 0.85

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 6
            spacing: 2

            RowLayout {
                Layout.fillWidth: true
                spacing: 5

                Rectangle {
                    width: 7
                    height: 7
                    radius: 3.5
                    visible: !node.structural || node.ledOn
                    color: node.ledOn ? LogicTheme.success
                                      : (LogicTheme.dark ? "#3A3A3C" : "#C0C0C4")
                    border.color: LogicTheme.hairline
                    border.width: 1
                }

                Label {
                    text: node.title
                    color: node.structural ? LogicTheme.textMuted : LogicTheme.textPrimary
                    font.pixelSize: LogicTheme.fontSizeSmall
                    font.bold: true
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
            }

            Label {
                visible: node.subtitle.length > 0
                text: node.subtitle
                color: node.structural ? LogicTheme.textMuted : LogicTheme.textSecondary
                font.pixelSize: 10
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            Label {
                visible: node.detail.length > 0
                text: node.detail
                color: node.accentDetail ? LogicTheme.warning
                                         : (LogicTheme.dark ? "#7EB6FF" : LogicTheme.accentDim)
                font.pixelSize: 10
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }

        MouseArea {
            anchors.fill: parent
            // Structural stubs are non-interactive unless explicitly enabled (e.g. Audio Input).
            enabled: node.editable || node.title === "Audio Input"
            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: node.clicked()
        }
    }
}
