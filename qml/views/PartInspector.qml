import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Flickable {
    id: root
    contentHeight: col.height
    clip: true
    flickableDirection: Flickable.VerticalFlick

    readonly property var part: App.studioSet.selectedPartModel

    ColumnLayout {
        id: col
        width: root.width
        spacing: 8

        GridLayout {
            columns: 2
            columnSpacing: 8
            rowSpacing: 6
            Layout.fillWidth: true

            Label { text: "Tone"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            Label { text: part ? part.toneName : ""; color: LogicTheme.textPrimary; font.pixelSize: LogicTheme.fontSize; elide: Text.ElideRight; Layout.fillWidth: true }

            Label { text: "Bank MSB"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            SpinBox {
                from: 0; to: 127
                value: part ? part.bankMsb : 0
                onValueModified: if (part) part.bankMsb = value
                editable: true
            }

            Label { text: "Bank LSB"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            SpinBox {
                from: 0; to: 127
                value: part ? part.bankLsb : 0
                onValueModified: if (part) part.bankLsb = value
                editable: true
            }

            Label { text: "Program"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            SpinBox {
                from: 0; to: 127
                value: part ? part.program : 0
                onValueModified: if (part) part.program = value
                editable: true
            }

            Label { text: "MIDI Ch"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            SpinBox {
                from: 1; to: 16
                value: part ? part.receiveChannel + 1 : 1
                onValueModified: if (part) part.receiveChannel = value - 1
            }

            Label { text: "Volume"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            Slider {
                from: 0; to: 127
                value: part ? part.level : 100
                onMoved: if (part) part.level = Math.round(value)
                Layout.fillWidth: true
            }

            Label { text: "Pan"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            Slider {
                from: 0; to: 127
                value: part ? part.pan : 64
                onMoved: if (part) part.pan = Math.round(value)
                Layout.fillWidth: true
            }

            Label { text: "Octave"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            SpinBox {
                from: -3; to: 3
                value: part ? (part.octaveShift - 64) : 0
                onValueModified: if (part) part.octaveShift = value
            }

            Label { text: "Transpose"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            SpinBox {
                from: 16; to: 112
                value: part ? part.coarseTune : 64
                textFromValue: (v) => (v - 64).toString()
                valueFromText: (t) => parseInt(t) + 64
                onValueModified: if (part) part.coarseTune = value
            }

            Label { text: "Chorus"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            Slider {
                from: 0; to: 127
                value: part ? part.chorusSend : 0
                onMoved: if (part) part.chorusSend = Math.round(value)
                Layout.fillWidth: true
            }

            Label { text: "Reverb"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            Slider {
                from: 0; to: 127
                value: part ? part.reverbSend : 0
                onMoved: if (part) part.reverbSend = Math.round(value)
                Layout.fillWidth: true
            }

            Label { text: "Output"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            ComboBox {
                model: ["Main", "Sub"]
                currentIndex: part ? part.outputAssign : 0
                onActivated: if (part) part.outputAssign = currentIndex
                Layout.fillWidth: true
            }

            Label { text: "Part Rx"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            Switch {
                checked: part ? part.partSwitch : true
                onToggled: if (part) part.partSwitch = checked
            }

            Label { text: "Keyboard"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            Switch {
                checked: part ? part.keyboardSwitch : true
                onToggled: if (part) part.keyboardSwitch = checked
            }

            Label { text: "Mute"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            Switch {
                checked: part ? part.mute : false
                onToggled: if (part) part.mute = checked
            }

            Label { text: "Vel Low"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            SpinBox {
                from: 1; to: 127
                value: part ? part.velocityLow : 1
                onValueModified: if (part) part.velocityLow = value
            }

            Label { text: "Vel High"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
            SpinBox {
                from: 0; to: 127
                value: part ? part.velocityHigh : 127
                onValueModified: if (part) part.velocityHigh = value
            }
        }
    }
}
