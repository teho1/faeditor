import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

ColumnLayout {
    id: root
    spacing: 10

    property var pcm: null

    readonly property var slots: pcm ? [
        {
            index: 0,
            source: pcm.matrix1Source,
            dest: pcm.matrix1Dest,
            sens: pcm.matrix1Sens
        },
        {
            index: 1,
            source: pcm.matrix2Source,
            dest: pcm.matrix2Dest,
            sens: pcm.matrix2Sens
        },
        {
            index: 2,
            source: pcm.matrix3Source,
            dest: pcm.matrix3Dest,
            sens: pcm.matrix3Sens
        },
        {
            index: 3,
            source: pcm.matrix4Source,
            dest: pcm.matrix4Dest,
            sens: pcm.matrix4Sens
        }
    ] : []

    function sourceName(i) {
        if (!pcm) return "OFF"
        const names = pcm.matrixSourceNames
        return (i >= 0 && i < names.length) ? names[i] : "?"
    }

    function destName(i) {
        if (!pcm) return "OFF"
        const names = pcm.matrixDestNames
        return (i >= 0 && i < names.length) ? names[i] : "?"
    }

    function friendlySource(i) {
        const n = sourceName(i)
        if (n === "OFF") return "Off"
        if (n === "BEND") return "Pitch Bend"
        if (n === "AFT") return "Aftertouch"
        if (n === "VELOCITY") return "Velocity"
        if (n === "KEYFOLLOW") return "Key Follow"
        if (n === "TEMPO") return "Tempo"
        if (n.indexOf("CTRL") === 0) return "Control " + n.slice(4)
        if (n.indexOf("CC") === 0) return "CC " + n.slice(2)
        if (n === "LFO1") return "LFO 1"
        if (n === "LFO2") return "LFO 2"
        if (n === "PIT-ENV") return "Pitch Env"
        if (n === "TVF-ENV") return "Filter Env"
        if (n === "TVA-ENV") return "Amp Env"
        return n
    }

    function friendlyDest(i) {
        const n = destName(i)
        const map = {
            "OFF": "Off",
            "PCH": "Pitch",
            "CUT": "Cutoff",
            "RES": "Resonance",
            "LEV": "Level",
            "PAN": "Pan",
            "DRY": "Dry",
            "CHO": "Chorus",
            "REV": "Reverb",
            "PIT-LFO1": "Pitch LFO 1",
            "PIT-LFO2": "Pitch LFO 2",
            "TVF-LFO1": "Filter LFO 1",
            "TVF-LFO2": "Filter LFO 2",
            "TVA-LFO1": "Amp LFO 1",
            "TVA-LFO2": "Amp LFO 2",
            "PAN-LFO1": "Pan LFO 1",
            "PAN-LFO2": "Pan LFO 2",
            "LFO1-RATE": "LFO 1 Rate",
            "LFO2-RATE": "LFO 2 Rate",
            "PIT-ATK": "Pitch Attack",
            "PIT-DCY": "Pitch Decay",
            "PIT-REL": "Pitch Release",
            "TVF-ATK": "Filter Attack",
            "TVF-DCY": "Filter Decay",
            "TVF-REL": "Filter Release",
            "TVA-ATK": "Amp Attack",
            "TVA-DCY": "Amp Decay",
            "TVA-REL": "Amp Release",
            "PMT": "PMT",
            "FXM": "FXM"
        }
        return map[n] || n
    }

    function categoryColor(destIndex) {
        const n = destName(destIndex)
        if (n === "PCH" || n.indexOf("PIT") === 0) return LogicTheme.warning
        if (n === "CUT" || n === "RES" || n.indexOf("TVF") === 0) return LogicTheme.accent
        if (n === "LEV" || n === "PAN" || n.indexOf("TVA") === 0) return LogicTheme.success
        if (n === "CHO" || n === "REV" || n === "DRY") return "#BF5AF2"
        return LogicTheme.textSecondary
    }

    function amountFromSens(sens) {
        return sens - 64
    }

    function sensFromAmount(amount) {
        return Math.max(1, Math.min(127, Math.round(amount) + 64))
    }

    function setSource(slot, v) {
        if (!pcm) return
        if (slot === 0) pcm.matrix1Source = v
        else if (slot === 1) pcm.matrix2Source = v
        else if (slot === 2) pcm.matrix3Source = v
        else pcm.matrix4Source = v
    }

    function setDest(slot, v) {
        if (!pcm) return
        if (slot === 0) pcm.matrix1Dest = v
        else if (slot === 1) pcm.matrix2Dest = v
        else if (slot === 2) pcm.matrix3Dest = v
        else pcm.matrix4Dest = v
    }

    function setSens(slot, v) {
        if (!pcm) return
        if (slot === 0) pcm.matrix1Sens = v
        else if (slot === 1) pcm.matrix2Sens = v
        else if (slot === 2) pcm.matrix3Sens = v
        else pcm.matrix4Sens = v
    }

    function clearSlot(slot) {
        setSource(slot, 0)
        setDest(slot, 0)
        setSens(slot, 64)
    }

    function firstOffSlot() {
        for (let i = 0; i < 4; ++i) {
            const s = slots[i]
            if (s && s.source === 0)
                return i
        }
        return -1
    }

    Label {
        text: "Modulation"
        color: LogicTheme.textPrimary
        font.pixelSize: LogicTheme.fontSizeTitle
        font.bold: true
    }

    Label {
        text: "Route a source into a destination. Amount is bipolar (−63 … +63)."
        color: LogicTheme.textSecondary
        font.pixelSize: LogicTheme.fontSizeSmall
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
    }

    Flow {
        Layout.fillWidth: true
        spacing: 10

        Repeater {
            model: 4
            delegate: Rectangle {
                id: card
                required property int index
                readonly property int src: {
                    if (!root.pcm) return 0
                    if (index === 0) return root.pcm.matrix1Source
                    if (index === 1) return root.pcm.matrix2Source
                    if (index === 2) return root.pcm.matrix3Source
                    return root.pcm.matrix4Source
                }
                readonly property int dst: {
                    if (!root.pcm) return 0
                    if (index === 0) return root.pcm.matrix1Dest
                    if (index === 1) return root.pcm.matrix2Dest
                    if (index === 2) return root.pcm.matrix3Dest
                    return root.pcm.matrix4Dest
                }
                readonly property int sens: {
                    if (!root.pcm) return 64
                    if (index === 0) return root.pcm.matrix1Sens
                    if (index === 1) return root.pcm.matrix2Sens
                    if (index === 2) return root.pcm.matrix3Sens
                    return root.pcm.matrix4Sens
                }
                readonly property bool active: src !== 0

                width: 280
                height: col.implicitHeight + 20
                radius: LogicTheme.radius
                color: LogicTheme.panelBgRaised
                border.color: active ? root.categoryColor(dst) : LogicTheme.hairline
                border.width: active ? 1.5 : 1
                opacity: active ? 1 : 0.55

                ColumnLayout {
                    id: col
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 10
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        Rectangle {
                            width: 8
                            height: 8
                            radius: 4
                            color: card.active ? root.categoryColor(card.dst) : LogicTheme.textMuted
                        }
                        Label {
                            text: "Slot " + (card.index + 1)
                            color: LogicTheme.textPrimary
                            font.bold: true
                            font.pixelSize: LogicTheme.fontSize
                            Layout.fillWidth: true
                        }
                        FaButton {
                            text: "Clear"
                            enabled: card.active
                            onClicked: root.clearSlot(card.index)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        ComboBox {
                            Layout.fillWidth: true
                            model: root.pcm ? root.pcm.matrixSourceNames : []
                            currentIndex: card.src
                            onActivated: (i) => root.setSource(card.index, i)
                            displayText: root.friendlySource(currentIndex)
                        }
                        Label {
                            text: "→"
                            color: LogicTheme.accent
                            font.bold: true
                            font.pixelSize: 16
                        }
                        ComboBox {
                            Layout.fillWidth: true
                            model: root.pcm ? root.pcm.matrixDestNames : []
                            currentIndex: card.dst
                            onActivated: (i) => root.setDest(card.index, i)
                            displayText: root.friendlyDest(currentIndex)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label {
                            text: "Amount"
                            color: LogicTheme.textSecondary
                            font.pixelSize: LogicTheme.fontSizeSmall
                        }
                        Slider {
                            id: amtSlider
                            Layout.fillWidth: true
                            from: -63
                            to: 63
                            stepSize: 1
                            value: root.amountFromSens(card.sens)
                            onMoved: root.setSens(card.index, root.sensFromAmount(value))
                        }
                        Label {
                            text: (amtSlider.value >= 0 ? "+" : "") + Math.round(amtSlider.value)
                            color: LogicTheme.textPrimary
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

    FaButton {
        text: "+ Add modulation"
        enabled: root.pcm && root.firstOffSlot() >= 0
        onClicked: {
            const i = root.firstOffSlot()
            if (i < 0)
                return
            const names = root.pcm.matrixSourceNames
            let src = 1
            for (let k = 0; k < names.length; ++k) {
                if (names[k] === "VELOCITY") { src = k; break }
            }
            let dst = 2
            const dn = root.pcm.matrixDestNames
            for (let k = 0; k < dn.length; ++k) {
                if (dn[k] === "CUT") { dst = k; break }
            }
            root.setSource(i, src)
            root.setDest(i, dst)
            root.setSens(i, 80)
        }
    }
}
