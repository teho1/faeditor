import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    id: root
    color: LogicTheme.windowBg

    readonly property var tone: App.tone
    readonly property var sn: App.tone.snSynth
    readonly property var pcm: App.tone.pcmSynth
    readonly property var sna: App.tone.snAcoustic
    readonly property var mfx: App.tone.mfx
    readonly property var part: App.studioSet.selectedPartModel

    readonly property string stage: tone.selectedStage
    readonly property bool bodyEditable: tone.isSnSynth || tone.isPcmSynth
    readonly property bool snaEditable: tone.isSnAcoustic
    readonly property int partialCount: tone.isPcmSynth ? 4 : 3
    readonly property var body: tone.isPcmSynth ? pcm : sn

    readonly property var flowStages: {
        if (tone.isSnAcoustic) {
            if (sna.isTwOrgan)
                return [
                    { id: "inst", title: "Inst" },
                    { id: "organ", title: "Organ" },
                    { id: "mfx", title: "MFX" }
                ]
            return [
                { id: "inst", title: "Inst" },
                { id: "modify", title: "Modify" },
                { id: "mfx", title: "MFX" }
            ]
        }
        if (tone.isPcmSynth)
            return [
                { id: "osc", title: "OSC" },
                { id: "filter", title: "Filter" },
                { id: "amp", title: "Amp" },
                { id: "lfo", title: "LFO" },
                { id: "matrix", title: "Matrix" },
                { id: "mfx", title: "MFX" }
            ]
        if (tone.isSnSynth)
            return [
                { id: "osc", title: "OSC" },
                { id: "filter", title: "Filter" },
                { id: "amp", title: "Amp" },
                { id: "lfo", title: "LFO" },
                { id: "mfx", title: "MFX" }
            ]
        return [ { id: "mfx", title: "MFX" } ]
    }

    readonly property var drawbarModel: [
        { label: "16'", prop: "bar16" },
        { label: "5⅓'", prop: "bar5_13" },
        { label: "8'", prop: "bar8" },
        { label: "4'", prop: "bar4" },
        { label: "2⅔'", prop: "bar2_23" },
        { label: "2'", prop: "bar2" },
        { label: "1⅗'", prop: "bar1_35" },
        { label: "1⅓'", prop: "bar1_13" },
        { label: "1'", prop: "bar1" }
    ]

    readonly property var snWaveIcons: [
        { name: "Saw", glyph: "╱╲" },
        { name: "Square", glyph: "⊓" },
        { name: "Pulse", glyph: "⊓₋" },
        { name: "Tri", glyph: "△" },
        { name: "Sine", glyph: "∿" },
        { name: "Noise", glyph: "▓" },
        { name: "Super", glyph: "≋" },
        { name: "PCM", glyph: "◈" }
    ]

    function selectStage(id) {
        tone.selectedStage = id
    }

    function stageTitle() {
        switch (stage) {
        case "osc": return "Oscillator"
        case "filter": return "Filter"
        case "amp": return "Amplifier"
        case "lfo": return "LFO"
        case "matrix": return "Matrix"
        case "inst": return "Instrument"
        case "modify": return "Modify"
        case "organ": return "TW Organ"
        case "mfx": return "MFX"
        case "common": return "Common"
        default: return "Tone"
        }
    }

    function partialDisplayName(index) {
        if (tone.isPcmSynth)
            return pcm.partialDisplayName(index)
        if (tone.isSnSynth)
            return sn.partialDisplayName(index)
        return "Partial " + (index + 1)
    }

    function stageSubtitle(id) {
        if (id === "mfx")
            return mfx.mfxSwitch ? mfx.typeName : "Off"
        if (id === "matrix")
            return "Ctrl 1–4"
        if (id === "inst")
            return sna.instrumentName || ("Inst " + sna.instNumberDisplay)
        if (id === "organ")
            return "Drawbars"
        if (id === "modify")
            return sna.instrumentName
                   ? (sna.modifyParamNamedCount + " params")
                   : "1–32"
        if (id === "osc" && tone.isPcmSynth)
            return "PCM Wave"
        if (root.bodyEditable)
            return root.partialDisplayName(root.body.selectedPartial)
        return tone.engineName
    }

    function drawbarValue(prop) {
        return sna[prop]
    }

    function setDrawbarValue(prop, value) {
        sna[prop] = value
    }

    function lfoShapeName() {
        if (tone.isSnSynth) {
            const n = sn.lfoShapeNames
            return (sn.lfoShape >= 0 && sn.lfoShape < n.length) ? n[sn.lfoShape] : ""
        }
        if (tone.isPcmSynth) {
            const n = pcm.lfoWaveformNames
            return (pcm.lfoWaveform >= 0 && pcm.lfoWaveform < n.length) ? n[pcm.lfoWaveform] : ""
        }
        return ""
    }

    function depthNorm(v) {
        // FA depths often center near 64; show activity from mid
        return Math.abs(v - 64) / 63
    }

    /// Right-side waveform browser (Tone Edit OSC) — shown when the bank has named waves.
    readonly property bool waveBrowserEligible: {
        if (stage !== "osc")
            return false
        if (tone.isSnSynth && sn && sn.oscWave === 7)
            return App.waveforms && App.waveforms.loaded
        if (tone.isPcmSynth && pcm)
            return pcm.waveGroupBank !== 2 && App.waveforms && App.waveforms.loaded
        return false
    }

    readonly property int currentWaveNumber: {
        if (tone.isSnSynth && sn)
            return sn.oscWaveNumber
        if (tone.isPcmSynth && pcm)
            return pcm.waveNumber
        return -1
    }

    function syncWaveBrowserBank() {
        if (!App.waveforms)
            return
        if (tone.isSnSynth) {
            App.waveforms.bank = "sn"
            return
        }
        if (tone.isPcmSynth && pcm) {
            if (pcm.waveGroupBank === 1)
                App.waveforms.bank = "intB"
            else if (pcm.waveGroupBank === 2)
                App.waveforms.bank = "srx"
            else
                App.waveforms.bank = "intA"
        }
    }

    function applyWaveFromBrowser(number) {
        if (tone.isSnSynth && sn) {
            sn.oscWaveNumber = number
            return
        }
        if (tone.isPcmSynth && pcm)
            pcm.waveNumber = number
    }

    onWaveBrowserEligibleChanged: {
        if (waveBrowserEligible)
            syncWaveBrowserBank()
    }

    Connections {
        target: pcm
        enabled: tone.isPcmSynth
        function onPcmChanged() {
            if (root.waveBrowserEligible)
                root.syncWaveBrowserBank()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        // —— Header ——
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Label {
                text: "Tone"
                color: LogicTheme.accent
                font.pixelSize: LogicTheme.fontSizeTitle
                font.bold: true
            }

            Rectangle { width: 1; height: 16; color: LogicTheme.hairline }

            ComboBox {
                id: partBox
                Layout.preferredWidth: 110
                model: 16
                displayText: "Part " + (currentIndex + 1)
                currentIndex: App.studioSet.selectedPart
                delegate: ItemDelegate {
                    width: partBox.width
                    text: "Part " + (index + 1)
                    highlighted: partBox.highlightedIndex === index
                }
                onActivated: (index) => { App.studioSet.selectedPart = index }
            }

            Label {
                text: part && part.toneName.length ? part.toneName : "(no tone)"
                color: LogicTheme.textPrimary
                font.pixelSize: LogicTheme.fontSize
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Rectangle {
                radius: 4
                color: LogicTheme.panelBgRaised
                border.color: LogicTheme.hairline
                border.width: 1
                implicitHeight: 26
                implicitWidth: engineLabel.implicitWidth + 16
                Label {
                    id: engineLabel
                    anchors.centerIn: parent
                    text: tone.engineName
                    color: LogicTheme.accent
                    font.pixelSize: LogicTheme.fontSizeSmall
                    font.bold: true
                }
            }

            Label {
                visible: tone.lastError.length > 0
                text: tone.lastError
                color: LogicTheme.danger
                font.pixelSize: LogicTheme.fontSizeSmall
                elide: Text.ElideRight
                Layout.maximumWidth: 200
            }

            FaButton { text: "Pull"; onClicked: tone.pull() }
            FaButton { text: "Push"; onClicked: tone.push() }
            FaButton { text: "Init SN-S"; onClicked: tone.initSnSynth() }
        }

        // —— Signal flow ——
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 72
            color: LogicTheme.panelBg
            radius: LogicTheme.radius
            border.color: LogicTheme.hairline
            border.width: 1

            Row {
                id: flowRow
                anchors.centerIn: parent
                spacing: 0

                Repeater {
                    model: root.flowStages

                    delegate: Item {
                        width: stageChip.width + (index < root.flowStages.length - 1 ? 28 : 0)
                        height: stageChip.height

                        Rectangle {
                            id: stageChip
                            width: Math.max(88, stageCol.implicitWidth + 20)
                            height: 48
                            radius: 6
                            color: root.stage === modelData.id
                                   ? LogicTheme.selectedBg
                                   : LogicTheme.panelBgRaised
                            border.color: root.stage === modelData.id
                                          ? LogicTheme.accent
                                          : LogicTheme.hairline
                            border.width: root.stage === modelData.id ? 2 : 1

                            Column {
                                id: stageCol
                                anchors.centerIn: parent
                                spacing: 2
                                Label {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: modelData.title
                                    color: LogicTheme.textPrimary
                                    font.pixelSize: LogicTheme.fontSize
                                    font.bold: true
                                }
                                Label {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: root.stageSubtitle(modelData.id)
                                    color: LogicTheme.textSecondary
                                    font.pixelSize: LogicTheme.fontSizeSmall
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.selectStage(modelData.id)
                            }
                        }

                        Canvas {
                            visible: index < root.flowStages.length - 1
                            anchors.left: stageChip.right
                            anchors.verticalCenter: stageChip.verticalCenter
                            width: 28
                            height: 14
                            onPaint: {
                                const ctx = getContext("2d")
                                ctx.reset()
                                ctx.strokeStyle = LogicTheme.dark ? "#6A8AAA" : "#7A9AB8"
                                ctx.fillStyle = ctx.strokeStyle
                                ctx.lineWidth = 1.5
                                ctx.lineCap = "round"
                                ctx.beginPath()
                                ctx.moveTo(2, 7)
                                ctx.lineTo(18, 7)
                                ctx.stroke()
                                ctx.beginPath()
                                ctx.moveTo(18, 7)
                                ctx.lineTo(12, 3)
                                ctx.lineTo(12, 11)
                                ctx.closePath()
                                ctx.fill()
                            }
                            Component.onCompleted: requestPaint()
                        }
                    }
                }
            }
        }

        // —— Editor pane (+ optional right-side waveform browser) ——
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: LogicTheme.panelBg
            radius: LogicTheme.radius
            border.color: LogicTheme.hairline
            border.width: 1
            clip: true

            RowLayout {
                anchors.fill: parent
                spacing: 0

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

            Flickable {
                anchors.fill: parent
                anchors.margins: 14
                contentHeight: editorCol.height
                clip: true
                flickableDirection: Flickable.VerticalFlick
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                ColumnLayout {
                    id: editorCol
                    width: parent.width
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        RowLayout {
                            spacing: 8
                            Label {
                                text: root.stageTitle()
                                color: LogicTheme.textPrimary
                                font.pixelSize: 16
                                font.bold: true
                            }
                            Label {
                                visible: root.stage === "osc" && tone.isPcmSynth
                                text: "PCM Wave"
                                color: LogicTheme.accent
                                font.pixelSize: LogicTheme.fontSizeSmall
                                font.bold: true
                            }
                        }
                        Item { Layout.fillWidth: true }

                        Row {
                            spacing: 6
                            visible: root.bodyEditable
                                     && root.stage !== "mfx" && root.stage !== "matrix"
                                     && root.stage !== "inst" && root.stage !== "modify"
                                     && root.stage !== "organ" && root.stage !== "common"
                            Repeater {
                                model: root.partialCount
                                Column {
                                    spacing: 2
                                    FaButton {
                                        text: root.partialDisplayName(index)
                                        secondaryText: tone.isPcmSynth
                                                       ? pcm.partialWaveLabels[index]
                                                       : ""
                                        enabled: root.bodyEditable
                                        checked: root.body.selectedPartial === index
                                        onClicked: root.body.selectedPartial = index
                                    }
                                    CheckBox {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        visible: tone.isSnSynth
                                        height: visible ? implicitHeight : 0
                                        text: "On"
                                        checked: tone.isSnSynth && sn.partialEnabled(index)
                                        onToggled: {
                                            if (tone.isSnSynth
                                                    && sn.partialEnabled(index) !== checked)
                                                sn.setPartialEnabled(index, checked)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Label {
                        visible: root.stage === "osc" && tone.isPcmSynth
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        text: "PCM Synth — each partial plays a ROM sample (wave), not a VA oscillator."
                        color: LogicTheme.textSecondary
                        font.pixelSize: LogicTheme.fontSizeSmall
                    }

                    Label {
                        visible: !root.bodyEditable && !root.snaEditable && root.stage !== "mfx"
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        color: LogicTheme.warning
                        font.pixelSize: LogicTheme.fontSize
                        text: "Body editing for this engine is not available yet. "
                              + "This part is " + tone.engineName + " — use MFX presets, "
                              + "or assign an SN-S / PCM / SN-A tone. Init SN-S writes an init Temporary SN-S body."
                    }

                    // ===================== OSC SN-S =====================
                    ColumnLayout {
                        id: snOscSection
                        visible: root.stage === "osc" && tone.isSnSynth
                        spacing: 14
                        Layout.fillWidth: true

                        // PCM is oscWave index 7 (same as waveform chip Repeater). Prefer
                        // oscWave===7 over oscIsPcmWave so visibility tracks the chip model
                        // even if a derived bool property fails to bind through `var sn`.
                        // Toggle whole layout blocks (not GridLayout cells): per-cell
                        // visible: inside GridLayout often fails to reflow in Qt Quick Layouts.
                        readonly property bool pcmWave: sn.oscWave === 7

                        Label {
                            text: "Waveform"
                            color: LogicTheme.textSecondary
                            font.pixelSize: LogicTheme.fontSizeSmall
                        }

                        Flow {
                            Layout.fillWidth: true
                            spacing: 8
                            Repeater {
                                model: sn.oscWaveNames
                                Rectangle {
                                    width: 72
                                    height: 56
                                    radius: 6
                                    color: sn.oscWave === index
                                           ? LogicTheme.selectedBg
                                           : LogicTheme.panelBgRaised
                                    border.color: sn.oscWave === index
                                                  ? LogicTheme.accent
                                                  : LogicTheme.hairline
                                    border.width: sn.oscWave === index ? 2 : 1

                                    Column {
                                        anchors.centerIn: parent
                                        spacing: 2
                                        Label {
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: index < root.snWaveIcons.length
                                                  ? root.snWaveIcons[index].glyph : "◦"
                                            color: LogicTheme.accent
                                            font.pixelSize: 18
                                        }
                                        Label {
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: index < root.snWaveIcons.length
                                                  ? root.snWaveIcons[index].name
                                                  : modelData
                                            color: LogicTheme.textPrimary
                                            font.pixelSize: LogicTheme.fontSizeSmall
                                            font.bold: true
                                        }
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: sn.oscWave = index
                                    }
                                }
                            }
                        }

                        GridLayout {
                            columns: 2
                            columnSpacing: 14
                            rowSpacing: 10
                            Layout.fillWidth: true

                            Label { text: "Variation"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            SpinBox { from: 0; to: 2; value: sn.oscWaveVariation; onValueModified: sn.oscWaveVariation = value }

                            Label { text: "Pitch"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            SpinBox { from: 40; to: 88; value: sn.oscPitch; onValueModified: sn.oscPitch = value }
                            Label { text: "Detune"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            SpinBox { from: 14; to: 114; value: sn.oscDetune; onValueModified: sn.oscDetune = value }
                        }

                        GridLayout {
                            visible: snOscSection.pcmWave
                            columns: 2
                            columnSpacing: 14
                            rowSpacing: 10
                            Layout.fillWidth: true

                            Label {
                                text: "Wave"
                                color: LogicTheme.textSecondary
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                radius: 4
                                color: LogicTheme.panelBgRaised
                                border.width: 1
                                border.color: root.waveBrowserEligible ? LogicTheme.accent : LogicTheme.hairline

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8
                                    anchors.rightMargin: 8
                                    spacing: 6
                                    Label {
                                        text: sn.oscWaveNumber === 0 ? "OFF" : ("#" + sn.oscWaveNumber)
                                        color: LogicTheme.textSecondary
                                        font.pixelSize: LogicTheme.fontSizeSmall
                                        Layout.preferredWidth: 44
                                    }
                                    Label {
                                        text: sn.oscWaveName
                                        color: LogicTheme.textPrimary
                                        font.pixelSize: LogicTheme.fontSizeSmall
                                        font.bold: true
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }
                                }
                            }
                            Label {
                                text: "Wave Gain"
                                color: LogicTheme.textSecondary
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                            ComboBox {
                                model: sn.oscWaveGainNames
                                currentIndex: sn.oscWaveGain
                                onActivated: (i) => { sn.oscWaveGain = i }
                                Layout.fillWidth: true
                            }
                        }

                        GridLayout {
                            visible: sn.oscWave === 2
                                     && !(sn.ringSwitch && sn.selectedPartial < 2)
                            columns: 2
                            columnSpacing: 14
                            rowSpacing: 10
                            Layout.fillWidth: true

                            Label {
                                text: "Pulse Width"
                                color: LogicTheme.textSecondary
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                Slider {
                                    from: 0; to: 127; stepSize: 1; value: sn.oscPulseWidth
                                    onMoved: sn.oscPulseWidth = Math.round(value)
                                    Layout.fillWidth: true
                                }
                                Label {
                                    text: String(sn.oscPulseWidth)
                                    color: LogicTheme.textPrimary
                                    font.pixelSize: LogicTheme.fontSizeSmall
                                    Layout.preferredWidth: 28
                                }
                            }
                        }

                        Label {
                            visible: !snOscSection.pcmWave
                                     && (sn.oscWave !== 2
                                         || (sn.ringSwitch && sn.selectedPartial < 2))
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: sn.ringSwitch && sn.selectedPartial < 2
                                  ? "Pulse Width is inactive for Partials 1–2 while Ring is on."
                                  : "Pulse Width is available when OSC Wave is PW-SQR."
                            color: LogicTheme.textSecondary
                            font.pixelSize: LogicTheme.fontSizeSmall
                        }
                    }

                    // ===================== OSC PCM =====================
                    ColumnLayout {
                        visible: root.stage === "osc" && tone.isPcmSynth
                        spacing: 14
                        Layout.fillWidth: true

                        GridLayout {
                            columns: 2
                            columnSpacing: 14
                            rowSpacing: 10
                            Layout.fillWidth: true

                            Label { text: "Wave Group"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            ComboBox {
                                model: pcm.waveGroupBankNames
                                currentIndex: pcm.waveGroupBank
                                onActivated: (i) => { pcm.waveGroupBank = i }
                                Layout.fillWidth: true
                            }
                            Label { text: "Wave"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8
                                // Named banks (INT-A / INT-B): pick from the right-side list.
                                Rectangle {
                                    visible: pcm.waveGroupBank !== 2
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 30
                                    radius: 4
                                    color: LogicTheme.panelBgRaised
                                    border.width: 1
                                    border.color: root.waveBrowserEligible ? LogicTheme.accent : LogicTheme.hairline

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 8
                                        anchors.rightMargin: 8
                                        spacing: 6
                                        Label {
                                            text: pcm.waveNumber === 0 ? "OFF" : ("#" + pcm.waveNumber)
                                            color: LogicTheme.textSecondary
                                            font.pixelSize: LogicTheme.fontSizeSmall
                                            Layout.preferredWidth: 44
                                        }
                                        Label {
                                            text: pcm.waveName
                                            color: LogicTheme.textPrimary
                                            font.pixelSize: LogicTheme.fontSizeSmall
                                            font.bold: true
                                            elide: Text.ElideRight
                                            Layout.fillWidth: true
                                        }
                                    }
                                }
                                // SRX: no named list — numeric wave number editor only.
                                SpinBox {
                                    visible: pcm.waveGroupBank === 2
                                    from: 0; to: 16384; value: pcm.waveNumber; editable: true
                                    onValueModified: pcm.waveNumber = value
                                }
                            }
                            Label {
                                visible: tone.isPcmSynth && pcm.waveGroupBank === 2
                                Layout.columnSpan: 2
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                                text: "SRX waves have no names in the Sound List — enter a wave number directly."
                                color: LogicTheme.textMuted
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                            Label { text: "Coarse Tune"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            SpinBox { from: 16; to: 112; value: pcm.coarseTune; onValueModified: pcm.coarseTune = value }
                            Label { text: "Fine Tune"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            SpinBox { from: 14; to: 114; value: pcm.fineTune; onValueModified: pcm.fineTune = value }
                            Label { text: "Tone Level"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            Slider {
                                from: 0; to: 127; value: pcm.toneLevel
                                onMoved: pcm.toneLevel = Math.round(value)
                                Layout.fillWidth: true
                            }
                            Label { text: "Tone Pan"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            SpinBox { from: 0; to: 127; value: pcm.tonePan; onValueModified: pcm.tonePan = value }
                            Label { text: "Structure 1–2 / 3–4"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            RowLayout {
                                SpinBox { from: 0; to: 9; value: pcm.structureType12; onValueModified: pcm.structureType12 = value }
                                SpinBox { from: 0; to: 9; value: pcm.structureType34; onValueModified: pcm.structureType34 = value }
                            }
                            Label { text: "Partials"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            Flow {
                                Layout.fillWidth: true
                                spacing: 8
                                CheckBox {
                                    text: pcm.partialDisplayName(0)
                                    checked: pcm.partial1On
                                    onToggled: pcm.partial1On = checked
                                }
                                CheckBox {
                                    text: pcm.partialDisplayName(1)
                                    checked: pcm.partial2On
                                    onToggled: pcm.partial2On = checked
                                }
                                CheckBox {
                                    text: pcm.partialDisplayName(2)
                                    checked: pcm.partial3On
                                    onToggled: pcm.partial3On = checked
                                }
                                CheckBox {
                                    text: pcm.partialDisplayName(3)
                                    checked: pcm.partial4On
                                    onToggled: pcm.partial4On = checked
                                }
                            }
                        }
                    }

                    // ===================== FILTER =====================
                    ColumnLayout {
                        visible: root.stage === "filter" && root.bodyEditable
                        spacing: 14
                        Layout.fillWidth: true

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            FilterResponseGraph {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 170
                                Layout.minimumWidth: 200
                                cutoff: root.body.filterCutoff
                                resonance: root.body.filterResonance
                                filterTypeName: {
                                    if (tone.isSnSynth)
                                        return sn.filterModeNames[sn.filterMode] || "LPF"
                                    return pcm.filterTypeNames[pcm.filterType] || "LPF"
                                }
                                onCutoffChangedByUser: (v) => { root.body.filterCutoff = v }
                                onResonanceChangedByUser: (v) => { root.body.filterResonance = v }
                            }

                            AdsrEnvelopeGraph {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 170
                                Layout.minimumWidth: 200
                                verticalLabel: "Cutoff"
                                attack: root.body.filterEnvAttack
                                decay: root.body.filterEnvDecay
                                sustain: root.body.filterEnvSustain
                                release: root.body.filterEnvRelease
                                onAttackChangedByUser: (v) => { root.body.filterEnvAttack = v }
                                onDecayChangedByUser: (v) => { root.body.filterEnvDecay = v }
                                onSustainChangedByUser: (v) => { root.body.filterEnvSustain = v }
                                onReleaseChangedByUser: (v) => { root.body.filterEnvRelease = v }
                            }
                        }

                        GridLayout {
                            columns: 4
                            columnSpacing: 12
                            rowSpacing: 8
                            Layout.fillWidth: true

                            Label {
                                text: tone.isSnSynth ? "Mode" : "Type"
                                color: LogicTheme.textSecondary
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                            ComboBox {
                                Layout.columnSpan: 3
                                Layout.fillWidth: true
                                model: tone.isSnSynth ? sn.filterModeNames : pcm.filterTypeNames
                                currentIndex: tone.isSnSynth ? sn.filterMode : pcm.filterType
                                onActivated: (i) => {
                                    if (tone.isSnSynth) sn.filterMode = i
                                    else pcm.filterType = i
                                }
                            }

                            Label { text: "Cutoff"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            Slider {
                                from: 0; to: 127; value: root.body.filterCutoff
                                onMoved: root.body.filterCutoff = Math.round(value)
                                Layout.fillWidth: true
                                Layout.columnSpan: 2
                            }
                            SpinBox {
                                from: 0; to: 127; value: root.body.filterCutoff
                                onValueModified: root.body.filterCutoff = value
                            }

                            Label { text: "Resonance"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            Slider {
                                from: 0; to: 127; value: root.body.filterResonance
                                onMoved: root.body.filterResonance = Math.round(value)
                                Layout.fillWidth: true
                                Layout.columnSpan: 2
                            }
                            SpinBox {
                                from: 0; to: 127; value: root.body.filterResonance
                                onValueModified: root.body.filterResonance = value
                            }

                            Label { text: "Attack"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            SpinBox { from: 0; to: 127; value: root.body.filterEnvAttack; onValueModified: root.body.filterEnvAttack = value }
                            Label { text: "Decay"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            SpinBox { from: 0; to: 127; value: root.body.filterEnvDecay; onValueModified: root.body.filterEnvDecay = value }
                            Label { text: "Sustain"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            SpinBox { from: 0; to: 127; value: root.body.filterEnvSustain; onValueModified: root.body.filterEnvSustain = value }
                            Label { text: "Release"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            SpinBox { from: 0; to: 127; value: root.body.filterEnvRelease; onValueModified: root.body.filterEnvRelease = value }
                        }
                    }

                    // ===================== AMP =====================
                    ColumnLayout {
                        visible: root.stage === "amp" && root.bodyEditable
                        spacing: 14
                        Layout.fillWidth: true

                        AdsrEnvelopeGraph {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 200
                            verticalLabel: "Volume"
                            curveColor: LogicTheme.success
                            fillColor: LogicTheme.dark ? "#30D15833" : "#34C75922"
                            attack: root.body.ampEnvAttack
                            decay: root.body.ampEnvDecay
                            sustain: root.body.ampEnvSustain
                            release: root.body.ampEnvRelease
                            onAttackChangedByUser: (v) => { root.body.ampEnvAttack = v }
                            onDecayChangedByUser: (v) => { root.body.ampEnvDecay = v }
                            onSustainChangedByUser: (v) => { root.body.ampEnvSustain = v }
                            onReleaseChangedByUser: (v) => { root.body.ampEnvRelease = v }
                        }

                        GridLayout {
                            columns: 4
                            columnSpacing: 12
                            rowSpacing: 8
                            Layout.fillWidth: true

                            Label { text: "Level"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            Slider {
                                from: 0; to: 127; value: root.body.ampLevel
                                onMoved: root.body.ampLevel = Math.round(value)
                                Layout.fillWidth: true
                                Layout.columnSpan: 2
                            }
                            SpinBox {
                                from: 0; to: 127; value: root.body.ampLevel
                                onValueModified: root.body.ampLevel = value
                            }

                            Label { text: "Pan"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            Slider {
                                from: 0; to: 127; value: root.body.ampPan
                                onMoved: root.body.ampPan = Math.round(value)
                                Layout.fillWidth: true
                                Layout.columnSpan: 2
                            }
                            SpinBox {
                                from: 0; to: 127; value: root.body.ampPan
                                onValueModified: root.body.ampPan = value
                            }

                            Label { text: "Attack"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            SpinBox { from: 0; to: 127; value: root.body.ampEnvAttack; onValueModified: root.body.ampEnvAttack = value }
                            Label { text: "Decay"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            SpinBox { from: 0; to: 127; value: root.body.ampEnvDecay; onValueModified: root.body.ampEnvDecay = value }
                            Label { text: "Sustain"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            SpinBox { from: 0; to: 127; value: root.body.ampEnvSustain; onValueModified: root.body.ampEnvSustain = value }
                            Label { text: "Release"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            SpinBox { from: 0; to: 127; value: root.body.ampEnvRelease; onValueModified: root.body.ampEnvRelease = value }
                        }
                    }

                    // ===================== LFO =====================
                    ColumnLayout {
                        visible: root.stage === "lfo" && root.bodyEditable
                        spacing: 14
                        Layout.fillWidth: true

                        LfoWaveformGraph {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 130
                            shape: tone.isSnSynth ? sn.lfoShape : pcm.lfoWaveform
                            shapeName: root.lfoShapeName()
                            rate: tone.isSnSynth ? sn.lfoRate : pcm.lfoRate
                            depth: Math.max(
                                       tone.isSnSynth ? sn.lfoPitchDepth : pcm.lfoPitchDepth,
                                       tone.isSnSynth ? sn.lfoFilterDepth : pcm.lfoFilterDepth,
                                       tone.isSnSynth ? sn.lfoAmpDepth : pcm.lfoAmpDepth)
                        }

                        GridLayout {
                            columns: 2
                            columnSpacing: 14
                            rowSpacing: 10
                            Layout.fillWidth: true

                            Label { text: "Shape"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            ComboBox {
                                Layout.fillWidth: true
                                model: tone.isSnSynth ? sn.lfoShapeNames : pcm.lfoWaveformNames
                                currentIndex: tone.isSnSynth ? sn.lfoShape : pcm.lfoWaveform
                                onActivated: (i) => {
                                    if (tone.isSnSynth) sn.lfoShape = i
                                    else pcm.lfoWaveform = i
                                }
                            }
                            Label { text: "Rate"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            Slider {
                                from: 0
                                to: tone.isPcmSynth ? 149 : 127
                                value: tone.isSnSynth ? sn.lfoRate : pcm.lfoRate
                                onMoved: {
                                    if (tone.isSnSynth) sn.lfoRate = Math.round(value)
                                    else pcm.lfoRate = Math.round(value)
                                }
                                Layout.fillWidth: true
                            }
                        }

                        Label {
                            text: "Routing"
                            color: LogicTheme.textPrimary
                            font.bold: true
                            font.pixelSize: LogicTheme.fontSize
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            // Pitch
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: pitchRoute.implicitHeight + 16
                                radius: 6
                                color: LogicTheme.panelBgRaised
                                border.color: LogicTheme.hairline
                                border.width: 1
                                ColumnLayout {
                                    id: pitchRoute
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.top: parent.top
                                    anchors.margins: 8
                                    spacing: 6
                                    RowLayout {
                                        Label { text: "LFO"; color: LogicTheme.textMuted; font.pixelSize: LogicTheme.fontSizeSmall }
                                        Label { text: "→"; color: LogicTheme.warning; font.bold: true }
                                        Label { text: "Pitch"; color: LogicTheme.textPrimary; font.bold: true; Layout.fillWidth: true }
                                        Rectangle {
                                            width: 8; height: 8; radius: 4; color: LogicTheme.warning
                                            opacity: 0.35 + root.depthNorm(tone.isSnSynth ? sn.lfoPitchDepth : pcm.lfoPitchDepth) * 0.65
                                        }
                                    }
                                    Slider {
                                        from: 1; to: 127
                                        value: tone.isSnSynth ? sn.lfoPitchDepth : pcm.lfoPitchDepth
                                        onMoved: {
                                            if (tone.isSnSynth) sn.lfoPitchDepth = Math.round(value)
                                            else pcm.lfoPitchDepth = Math.round(value)
                                        }
                                        Layout.fillWidth: true
                                    }
                                    Label {
                                        text: "Depth  " + (tone.isSnSynth ? sn.lfoPitchDepth : pcm.lfoPitchDepth)
                                        color: LogicTheme.textSecondary
                                        font.pixelSize: LogicTheme.fontSizeSmall
                                    }
                                }
                            }

                            // Filter
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: filterRoute.implicitHeight + 16
                                radius: 6
                                color: LogicTheme.panelBgRaised
                                border.color: LogicTheme.hairline
                                border.width: 1
                                ColumnLayout {
                                    id: filterRoute
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.top: parent.top
                                    anchors.margins: 8
                                    spacing: 6
                                    RowLayout {
                                        Label { text: "LFO"; color: LogicTheme.textMuted; font.pixelSize: LogicTheme.fontSizeSmall }
                                        Label { text: "→"; color: LogicTheme.accent; font.bold: true }
                                        Label { text: "Filter"; color: LogicTheme.textPrimary; font.bold: true; Layout.fillWidth: true }
                                        Rectangle {
                                            width: 8; height: 8; radius: 4; color: LogicTheme.accent
                                            opacity: 0.35 + root.depthNorm(tone.isSnSynth ? sn.lfoFilterDepth : pcm.lfoFilterDepth) * 0.65
                                        }
                                    }
                                    Slider {
                                        from: 1; to: 127
                                        value: tone.isSnSynth ? sn.lfoFilterDepth : pcm.lfoFilterDepth
                                        onMoved: {
                                            if (tone.isSnSynth) sn.lfoFilterDepth = Math.round(value)
                                            else pcm.lfoFilterDepth = Math.round(value)
                                        }
                                        Layout.fillWidth: true
                                    }
                                    Label {
                                        text: "Depth  " + (tone.isSnSynth ? sn.lfoFilterDepth : pcm.lfoFilterDepth)
                                        color: LogicTheme.textSecondary
                                        font.pixelSize: LogicTheme.fontSizeSmall
                                    }
                                }
                            }

                            // Amp
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: ampRoute.implicitHeight + 16
                                radius: 6
                                color: LogicTheme.panelBgRaised
                                border.color: LogicTheme.hairline
                                border.width: 1
                                ColumnLayout {
                                    id: ampRoute
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.top: parent.top
                                    anchors.margins: 8
                                    spacing: 6
                                    RowLayout {
                                        Label { text: "LFO"; color: LogicTheme.textMuted; font.pixelSize: LogicTheme.fontSizeSmall }
                                        Label { text: "→"; color: LogicTheme.success; font.bold: true }
                                        Label { text: "Amp"; color: LogicTheme.textPrimary; font.bold: true; Layout.fillWidth: true }
                                        Rectangle {
                                            width: 8; height: 8; radius: 4; color: LogicTheme.success
                                            opacity: 0.35 + root.depthNorm(tone.isSnSynth ? sn.lfoAmpDepth : pcm.lfoAmpDepth) * 0.65
                                        }
                                    }
                                    Slider {
                                        from: 1; to: 127
                                        value: tone.isSnSynth ? sn.lfoAmpDepth : pcm.lfoAmpDepth
                                        onMoved: {
                                            if (tone.isSnSynth) sn.lfoAmpDepth = Math.round(value)
                                            else pcm.lfoAmpDepth = Math.round(value)
                                        }
                                        Layout.fillWidth: true
                                    }
                                    Label {
                                        text: "Depth  " + (tone.isSnSynth ? sn.lfoAmpDepth : pcm.lfoAmpDepth)
                                        color: LogicTheme.textSecondary
                                        font.pixelSize: LogicTheme.fontSizeSmall
                                    }
                                }
                            }
                        }
                    }

                    // ===================== MATRIX PCM =====================
                    MatrixControlCards {
                        visible: root.stage === "matrix" && tone.isPcmSynth
                        Layout.fillWidth: true
                        pcm: root.pcm
                    }

                    // ===================== SN-A Inst =====================
                    GridLayout {
                        visible: root.stage === "inst" && tone.isSnAcoustic
                        columns: 2
                        columnSpacing: 12
                        rowSpacing: 8
                        Layout.fillWidth: true

                        Label { text: "Tone Name"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        TextField {
                            text: sna.toneName
                            onEditingFinished: sna.toneName = text
                            Layout.fillWidth: true
                        }
                        Label { text: "Instrument"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            ComboBox {
                                Layout.fillWidth: true
                                model: sna.instrumentNames
                                currentIndex: Math.max(0, Math.min(sna.instNumberDisplay - 1, count - 1))
                                onActivated: sna.instNumberDisplay = index + 1
                            }
                            Label {
                                text: sna.instrumentFamilyLabel
                                      + "  ·  panel " + sna.instNumberDisplay
                                      + "  ·  sysex " + sna.instNumber
                                      + (sna.isTwOrgan ? "  ·  TW Organ" : "")
                                color: LogicTheme.textMuted
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                        }
                        Label { text: "Inst Variation"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        RowLayout {
                            Layout.fillWidth: true
                            ComboBox {
                                Layout.fillWidth: true
                                visible: sna.instVariationNames.length > 1
                                model: sna.instVariationNames
                                currentIndex: Math.max(0, Math.min(sna.instVariation, count - 1))
                                onActivated: sna.instVariation = index
                            }
                            SpinBox {
                                visible: sna.instVariationNames.length <= 1
                                from: 0; to: 127; value: sna.instVariation; editable: true
                                onValueModified: sna.instVariation = value
                            }
                            Label {
                                visible: sna.instVariationNames.length > 1
                                text: "CC80 / S1–S3 style articulations"
                                color: LogicTheme.textMuted
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                        }
                        Label { text: "Tone Level"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127; value: sna.toneLevel
                            onMoved: sna.toneLevel = Math.round(value)
                            Layout.fillWidth: true
                        }
                        Label { text: "Poly"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Switch {
                            checked: sna.monoPoly
                            onToggled: sna.monoPoly = checked
                        }
                        Label { text: "Octave Shift"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        SpinBox {
                            from: 61; to: 67; value: sna.octaveShift
                            onValueModified: sna.octaveShift = value
                        }
                        Label { text: "Cutoff Offset"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127; value: sna.cutoffOffset
                            onMoved: sna.cutoffOffset = Math.round(value)
                            Layout.fillWidth: true
                        }
                        Label { text: "Resonance Offset"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        Slider {
                            from: 0; to: 127; value: sna.resonanceOffset
                            onMoved: sna.resonanceOffset = Math.round(value)
                            Layout.fillWidth: true
                        }
                        Label { text: "Attack / Release Off"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        RowLayout {
                            SpinBox { from: 0; to: 127; value: sna.attackTimeOffset; onValueModified: sna.attackTimeOffset = value }
                            SpinBox { from: 0; to: 127; value: sna.releaseTimeOffset; onValueModified: sna.releaseTimeOffset = value }
                        }
                        Label { text: "Vibrato Rate / Depth / Delay"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                        RowLayout {
                            SpinBox { from: 0; to: 127; value: sna.vibratoRate; onValueModified: sna.vibratoRate = value }
                            SpinBox { from: 0; to: 127; value: sna.vibratoDepth; onValueModified: sna.vibratoDepth = value }
                            SpinBox { from: 0; to: 127; value: sna.vibratoDelay; onValueModified: sna.vibratoDelay = value }
                        }
                    }

                    // ===================== SN-A Modify =====================
                    ColumnLayout {
                        visible: root.stage === "modify" && tone.isSnAcoustic && !sna.isTwOrgan
                        spacing: 12
                        Layout.fillWidth: true

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Label {
                                text: sna.instrumentName || ("Inst " + sna.instNumberDisplay)
                                color: LogicTheme.textPrimary
                                font.bold: true
                                font.pixelSize: LogicTheme.fontSize
                            }
                            Label {
                                text: sna.instrumentFamilyLabel + "  ·  Parameter Guide Modify map"
                                color: LogicTheme.textSecondary
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                        }

                        // —— Ac. Piano: resonance / hammer / stereo ——
                        ColumnLayout {
                            visible: sna.instrumentFamily === "acPiano"
                            Layout.fillWidth: true
                            spacing: 8
                            Label {
                                text: "Resonance & character"
                                color: LogicTheme.textPrimary
                                font.bold: true
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                            GridLayout {
                                columns: 3
                                columnSpacing: 14
                                rowSpacing: 10
                                Layout.fillWidth: true
                                Repeater {
                                    model: [0, 1, 2, 3, 4, 5]
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 4
                                        Label {
                                            text: sna.modifyParamNames[modelData] || ""
                                            color: LogicTheme.textSecondary
                                            font.pixelSize: LogicTheme.fontSizeSmall
                                            elide: Text.ElideRight
                                            Layout.fillWidth: true
                                        }
                                        ComboBox {
                                            visible: sna.modifyParamHasEnum(modelData)
                                            Layout.fillWidth: true
                                            model: sna.modifyParamEnumNames(modelData)
                                            currentIndex: Math.max(0, Math.min(sna.modifyParams[modelData], count - 1))
                                            onActivated: sna.setModifyParam(modelData, index)
                                        }
                                        RowLayout {
                                            visible: !sna.modifyParamHasEnum(modelData)
                                            Layout.fillWidth: true
                                            Slider {
                                                Layout.fillWidth: true
                                                from: sna.modifyParamMinValues[modelData]
                                                to: sna.modifyParamMaxValues[modelData]
                                                stepSize: 1
                                                value: sna.modifyParams[modelData]
                                                onMoved: sna.setModifyParam(modelData, Math.round(value))
                                            }
                                            Label {
                                                text: sna.modifyParamDisplayText(modelData)
                                                color: LogicTheme.accent
                                                font.pixelSize: LogicTheme.fontSizeSmall
                                                font.bold: true
                                                Layout.preferredWidth: 36
                                                horizontalAlignment: Text.AlignRight
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // —— E.Piano / Clav / Bass: noise ——
                        ColumnLayout {
                            visible: sna.instrumentFamily === "ePiano"
                                     || sna.instrumentFamily === "clav"
                                     || sna.instrumentFamily === "bass"
                            Layout.fillWidth: true
                            spacing: 8
                            Label {
                                text: sna.instrumentFamily === "bass" ? "Pickup / string noise"
                                                                     : "Key-off & hum noise"
                                color: LogicTheme.textPrimary
                                font.bold: true
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                Label {
                                    text: sna.modifyParamNames[0] || "Noise Level"
                                    color: LogicTheme.textSecondary
                                    font.pixelSize: LogicTheme.fontSizeSmall
                                    Layout.preferredWidth: 110
                                }
                                Slider {
                                    Layout.fillWidth: true
                                    from: 0; to: 127; stepSize: 1
                                    value: sna.modifyParams[0]
                                    onMoved: sna.setModifyParam(0, Math.round(value))
                                }
                                Label {
                                    text: sna.modifyParamDisplayText(0)
                                    color: LogicTheme.accent
                                    font.bold: true
                                    font.pixelSize: LogicTheme.fontSizeSmall
                                    Layout.preferredWidth: 40
                                    horizontalAlignment: Text.AlignRight
                                }
                            }
                            Label {
                                visible: sna.instrumentFamily === "bass"
                                text: "Articulations (Mute / Slap / Harmonics) are on Inst → Variation"
                                color: LogicTheme.textMuted
                                font.pixelSize: LogicTheme.fontSizeSmall
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }

                        // —— Guitar: mute noise / strum ——
                        ColumnLayout {
                            visible: sna.instrumentFamily === "guitar"
                            Layout.fillWidth: true
                            spacing: 8
                            Label {
                                text: "Noise & strum"
                                color: LogicTheme.textPrimary
                                font.bold: true
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                            GridLayout {
                                columns: 2
                                columnSpacing: 14
                                rowSpacing: 10
                                Layout.fillWidth: true
                                Repeater {
                                    model: [0, 1]
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 4
                                        Label {
                                            text: sna.modifyParamNames[modelData] || ""
                                            color: LogicTheme.textSecondary
                                            font.pixelSize: LogicTheme.fontSizeSmall
                                        }
                                        RowLayout {
                                            Layout.fillWidth: true
                                            Slider {
                                                Layout.fillWidth: true
                                                from: 0; to: 127; stepSize: 1
                                                value: sna.modifyParams[modelData]
                                                onMoved: sna.setModifyParam(modelData, Math.round(value))
                                            }
                                            Label {
                                                text: sna.modifyParamDisplayText(modelData)
                                                color: LogicTheme.accent
                                                font.bold: true
                                                font.pixelSize: LogicTheme.fontSizeSmall
                                                Layout.preferredWidth: 40
                                                horizontalAlignment: Text.AlignRight
                                            }
                                        }
                                    }
                                }
                                Label {
                                    text: "Strum Mode"
                                    color: LogicTheme.textSecondary
                                    font.pixelSize: LogicTheme.fontSizeSmall
                                }
                                Switch {
                                    checked: sna.modifyParams[3] !== 0
                                    onToggled: sna.setModifyParam(3, checked ? 1 : 0)
                                }
                            }
                            Label {
                                text: "Mute / Harmonics → Inst Variation"
                                color: LogicTheme.textMuted
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                        }

                        // —— Strings: hold legato + variation hint ——
                        ColumnLayout {
                            visible: sna.instrumentFamily === "strings"
                            Layout.fillWidth: true
                            spacing: 8
                            Label {
                                text: "Phrase / hold"
                                color: LogicTheme.textPrimary
                                font.bold: true
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: "Hold Legato Mode"
                                    color: LogicTheme.textSecondary
                                    font.pixelSize: LogicTheme.fontSizeSmall
                                    Layout.fillWidth: true
                                }
                                Switch {
                                    checked: sna.modifyParams[3] !== 0
                                    onToggled: sna.setModifyParam(3, checked ? 1 : 0)
                                }
                            }
                            Label {
                                text: "Staccato / Pizzicato / Tremolo → Inst Variation"
                                color: LogicTheme.textMuted
                                font.pixelSize: LogicTheme.fontSizeSmall
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }

                        // —— Fallback: unknown / named slots grid ——
                        GridLayout {
                            visible: sna.instrumentFamily === "unknown"
                                     || (sna.modifyParamNamedCount > 0
                                         && sna.instrumentFamily !== "acPiano"
                                         && sna.instrumentFamily !== "ePiano"
                                         && sna.instrumentFamily !== "clav"
                                         && sna.instrumentFamily !== "bass"
                                         && sna.instrumentFamily !== "guitar"
                                         && sna.instrumentFamily !== "strings")
                            columns: 4
                            columnSpacing: 10
                            rowSpacing: 8
                            Layout.fillWidth: true
                            Repeater {
                                model: 32
                                ColumnLayout {
                                    visible: {
                                        const n = sna.modifyParamNames[index]
                                        return n && n.length > 0
                                    }
                                    Label {
                                        text: sna.modifyParamNames[index] || ("M" + (index + 1))
                                        color: LogicTheme.textMuted
                                        font.pixelSize: LogicTheme.fontSizeSmall
                                        elide: Text.ElideRight
                                        Layout.preferredWidth: 100
                                    }
                                    ComboBox {
                                        visible: sna.modifyParamHasEnum(index)
                                        Layout.preferredWidth: 100
                                        model: sna.modifyParamEnumNames(index)
                                        currentIndex: Math.max(0, Math.min(sna.modifyParams[index], count - 1))
                                        onActivated: (i) => sna.setModifyParam(index, i)
                                    }
                                    SpinBox {
                                        visible: !sna.modifyParamHasEnum(index)
                                        from: sna.modifyParamMinValues[index]
                                        to: sna.modifyParamMaxValues[index]
                                        value: sna.modifyParams[index]
                                        editable: true
                                        textFromValue: function(v) {
                                            const off = sna.modifyParamDisplayOffsets[index] || 0
                                            return "" + (v - off)
                                        }
                                        valueFromText: function(t) {
                                            const off = sna.modifyParamDisplayOffsets[index] || 0
                                            return Number(t) + off
                                        }
                                        onValueModified: sna.setModifyParam(index, value)
                                        Layout.preferredWidth: 100
                                    }
                                }
                            }
                        }
                    }

                    // ===================== TW Organ =====================
                    ColumnLayout {
                        visible: (root.stage === "organ" || (root.stage === "modify" && sna.isTwOrgan))
                                 && tone.isSnAcoustic
                        spacing: 14
                        Layout.fillWidth: true

                        Label {
                            text: "Harmonic bars"
                            color: LogicTheme.textPrimary
                            font.bold: true
                            font.pixelSize: LogicTheme.fontSize
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 220
                            spacing: 8

                            Repeater {
                                model: root.drawbarModel
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    spacing: 4

                                    Label {
                                        text: String(root.drawbarValue(modelData.prop))
                                        color: LogicTheme.accent
                                        font.pixelSize: LogicTheme.fontSizeSmall
                                        font.bold: true
                                        horizontalAlignment: Text.AlignHCenter
                                        Layout.fillWidth: true
                                    }
                                    Slider {
                                        orientation: Qt.Vertical
                                        from: 0; to: 8; stepSize: 1
                                        value: root.drawbarValue(modelData.prop)
                                        onMoved: root.setDrawbarValue(modelData.prop, Math.round(value))
                                        Layout.fillHeight: true
                                        Layout.alignment: Qt.AlignHCenter
                                        Layout.preferredWidth: 36
                                    }
                                    Label {
                                        text: modelData.label
                                        color: LogicTheme.textSecondary
                                        font.pixelSize: LogicTheme.fontSizeSmall
                                        font.bold: true
                                        horizontalAlignment: Text.AlignHCenter
                                        Layout.fillWidth: true
                                    }
                                }
                            }
                        }

                        GridLayout {
                            columns: 2
                            columnSpacing: 12
                            rowSpacing: 8
                            Layout.fillWidth: true

                            Label { text: "Leakage"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            Slider {
                                from: 0; to: 127; value: sna.leakageLevel
                                onMoved: sna.leakageLevel = Math.round(value)
                                Layout.fillWidth: true
                            }
                            Label { text: "Percussion"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            Switch {
                                checked: sna.percussionSwitch
                                onToggled: sna.percussionSwitch = checked
                            }
                            Label { text: "Perc Soft"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            ComboBox {
                                model: sna.percussionSoftNames
                                currentIndex: sna.percussionSoft
                                onActivated: (i) => { sna.percussionSoft = i }
                                Layout.fillWidth: true
                            }
                            Label { text: "Perc Soft / Norm Level"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            RowLayout {
                                SpinBox { from: 0; to: 15; value: sna.percussionSoftLevel; onValueModified: sna.percussionSoftLevel = value }
                                SpinBox { from: 0; to: 15; value: sna.percussionNormalLevel; onValueModified: sna.percussionNormalLevel = value }
                            }
                            Label { text: "Perc Slow"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            ComboBox {
                                model: sna.percussionSlowNames
                                currentIndex: sna.percussionSlow
                                onActivated: (i) => { sna.percussionSlow = i }
                                Layout.fillWidth: true
                            }
                            Label { text: "Perc Slow / Fast Time"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            RowLayout {
                                SpinBox { from: 0; to: 127; value: sna.percussionSlowTime; onValueModified: sna.percussionSlowTime = value }
                                SpinBox { from: 0; to: 127; value: sna.percussionFastTime; onValueModified: sna.percussionFastTime = value }
                            }
                            Label { text: "Perc Harmonic"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            ComboBox {
                                model: sna.percussionHarmonicNames
                                currentIndex: sna.percussionHarmonic
                                onActivated: (i) => { sna.percussionHarmonic = i }
                                Layout.fillWidth: true
                            }
                            Label { text: "Recharge / Bar Level"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            RowLayout {
                                SpinBox { from: 0; to: 15; value: sna.percussionRechargeTime; onValueModified: sna.percussionRechargeTime = value }
                                SpinBox { from: 0; to: 127; value: sna.percussionHarmonicBarLevel; onValueModified: sna.percussionHarmonicBarLevel = value }
                            }
                            Label { text: "Key On / Off Click"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            RowLayout {
                                SpinBox { from: 0; to: 31; value: sna.keyOnClickLevel; onValueModified: sna.keyOnClickLevel = value }
                                SpinBox { from: 0; to: 31; value: sna.keyOffClickLevel; onValueModified: sna.keyOffClickLevel = value }
                            }
                        }
                    }

                    // ===================== MFX =====================
                    ColumnLayout {
                        visible: root.stage === "mfx"
                        spacing: 10
                        Layout.fillWidth: true

                        Label {
                            visible: !tone.supportsMfx
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            color: LogicTheme.warning
                            text: "MFX needs a known tone engine (assign a tone to this part first)."
                        }

                        Label {
                            text: "Presets"
                            color: LogicTheme.textPrimary
                            font.bold: true
                            font.pixelSize: LogicTheme.fontSize
                        }

                        Flow {
                            Layout.fillWidth: true
                            spacing: 8
                            Repeater {
                                model: tone.presets
                                FaButton {
                                    text: model.name
                                    enabled: tone.supportsMfx
                                    onClicked: tone.applyMfxPreset(index)
                                }
                            }
                        }

                        GridLayout {
                            columns: 2
                            columnSpacing: 12
                            rowSpacing: 8
                            Layout.fillWidth: true
                            enabled: tone.supportsMfx

                            Label { text: "MFX Switch"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            Switch {
                                checked: mfx.mfxSwitch
                                onToggled: mfx.mfxSwitch = checked
                            }
                            Label { text: "Type"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                            ComboBox {
                                model: mfx.typeNames
                                currentIndex: mfx.type
                                onActivated: (i) => { mfx.type = i }
                                Layout.fillWidth: true
                            }
                            // Sends stay here except for multi-tap delay (has them in Output).
                            Label {
                                visible: root.mfx && root.mfx.uiFamily !== MfxUiFamily.familyMultiTapDelay
                                text: "Chorus Send"
                                color: LogicTheme.textSecondary
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                            Slider {
                                visible: root.mfx && root.mfx.uiFamily !== MfxUiFamily.familyMultiTapDelay
                                from: 0; to: 127; value: root.mfx ? root.mfx.chorusSend : 0
                                onMoved: if (root.mfx) root.mfx.chorusSend = Math.round(value)
                                Layout.fillWidth: true
                            }
                            Label {
                                visible: root.mfx && root.mfx.uiFamily !== MfxUiFamily.familyMultiTapDelay
                                text: "Reverb Send"
                                color: LogicTheme.textSecondary
                                font.pixelSize: LogicTheme.fontSizeSmall
                            }
                            Slider {
                                visible: root.mfx && root.mfx.uiFamily !== MfxUiFamily.familyMultiTapDelay
                                from: 0; to: 127; value: root.mfx ? root.mfx.reverbSend : 0
                                onMoved: if (root.mfx) root.mfx.reverbSend = Math.round(value)
                                Layout.fillWidth: true
                            }
                        }

                        // Family router — must use root.mfx (never bare `mfx: mfx` self-bind).
                        MfxDelayEditor {
                            Layout.fillWidth: true
                            visible: root.mfx && (root.mfx.uiFamily === MfxUiFamily.familyMultiTapDelay
                                                  || root.mfx.uiFamily === MfxUiFamily.familyDelay)
                            mfx: root.mfx
                            editorEnabled: tone.supportsMfx
                        }
                        MfxEQEditor {
                            Layout.fillWidth: true
                            visible: root.mfx && root.mfx.uiFamily === MfxUiFamily.familyEq
                            mfx: root.mfx
                            editorEnabled: tone.supportsMfx
                        }
                        MfxFilterEditor {
                            Layout.fillWidth: true
                            visible: root.mfx && root.mfx.uiFamily === MfxUiFamily.familyFilter
                            mfx: root.mfx
                            editorEnabled: tone.supportsMfx
                        }
                        MfxModulationEditor {
                            Layout.fillWidth: true
                            visible: root.mfx && root.mfx.uiFamily === MfxUiFamily.familyModulation
                            mfx: root.mfx
                            editorEnabled: tone.supportsMfx
                        }
                        MfxCompressorEditor {
                            Layout.fillWidth: true
                            visible: root.mfx && root.mfx.uiFamily === MfxUiFamily.familyCompressor
                            mfx: root.mfx
                            editorEnabled: tone.supportsMfx
                        }
                        MfxAmpEditor {
                            Layout.fillWidth: true
                            visible: root.mfx && root.mfx.uiFamily === MfxUiFamily.familyAmp
                            mfx: root.mfx
                            editorEnabled: tone.supportsMfx
                        }
                        MfxRotaryEditor {
                            Layout.fillWidth: true
                            visible: root.mfx && root.mfx.uiFamily === MfxUiFamily.familyRotary
                            mfx: root.mfx
                            editorEnabled: tone.supportsMfx
                        }
                        MfxPipelineEditor {
                            Layout.fillWidth: true
                            visible: root.mfx && root.mfx.uiFamily === MfxUiFamily.familyPipeline
                            mfx: root.mfx
                            editorEnabled: tone.supportsMfx
                        }
                        MfxSlicerEditor {
                            Layout.fillWidth: true
                            visible: root.mfx && root.mfx.uiFamily === MfxUiFamily.familySlicer
                            mfx: root.mfx
                            editorEnabled: tone.supportsMfx
                        }
                        MfxGenericParameterEditor {
                            Layout.fillWidth: true
                            visible: root.mfx && (root.mfx.uiFamily === MfxUiFamily.familyGeneric
                                                  || root.mfx.uiFamily === MfxUiFamily.familyParamGrid)
                            mfx: root.mfx
                            editorEnabled: tone.supportsMfx
                        }

                        Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            color: LogicTheme.textMuted
                            font.pixelSize: LogicTheme.fontSizeSmall
                            text: "Edits write Temporary Tone SysEx. Use Write → User Tone on the FA to store permanently."
                        }
                    }
                }
            }
            } // Item (editor flickable host)

            Rectangle {
                width: 1
                Layout.fillHeight: true
                color: LogicTheme.hairline
                visible: root.waveBrowserEligible
            }

            WaveformBrowser {
                id: waveBrowser
                Layout.preferredWidth: LogicTheme.inspectorWidth
                Layout.fillHeight: true
                visible: root.waveBrowserEligible
                showClose: false
                selectedNumber: root.currentWaveNumber
                onWaveChosen: (n) => root.applyWaveFromBrowser(n)
            }
            } // RowLayout
        }
    }
}
