import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    color: LogicTheme.windowBg

    Component.onCompleted: {
        if (App.midi.connected)
            App.audioFx.pullFromDevice()
    }

    Flickable {
        anchors.fill: parent
        anchors.margins: 16
        contentHeight: col.height
        clip: true

        ColumnLayout {
            id: col
            width: parent.width
            spacing: 14

            RowLayout {
                Layout.fillWidth: true
                Label {
                    text: "Audio Input FX"
                    color: LogicTheme.textPrimary
                    font.pixelSize: LogicTheme.fontSizeTitle
                    font.bold: true
                    Layout.fillWidth: true
                }
                Button {
                    text: "Pull from FA"
                    onClicked: App.audioFx.pullFromDevice()
                }
                Button {
                    text: "Push to FA"
                    onClicked: App.audioFx.pushToDevice()
                }
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: LogicTheme.warning
                font.pixelSize: LogicTheme.fontSizeSmall
                text: "Not controllable via SysEx (set on the FA): USB Audio Output Select, USB Audio Input Destination, USB Driver Mode — MENU → System → [3] System Effects → USB Audio (and USB Driver for driver mode)."
            }

            Label {
                visible: App.audioFx.lastError.length > 0
                text: App.audioFx.lastError
                color: LogicTheme.danger
                font.pixelSize: LogicTheme.fontSizeSmall
            }

            GroupBox {
                title: "Input path"
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    anchors.fill: parent
                    columnSpacing: 12
                    rowSpacing: 8

                    Label { text: "TFX Location"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    ComboBox {
                        model: ["MAIN (keyboard)", "INPUT (guitar/line)"]
                        currentIndex: App.audioFx.tfxLocation
                        onActivated: (index) => { App.audioFx.tfxLocation = index }
                        Layout.fillWidth: true
                    }

                    Label { text: "TFX Input Gain"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    ComboBox {
                        model: App.audioFx.tfxGainNames
                        currentIndex: App.audioFx.tfxInputGain
                        onActivated: (index) => { App.audioFx.tfxInputGain = index }
                        Layout.fillWidth: true
                    }

                    Label { text: "Input Reverb"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    Switch {
                        checked: App.audioFx.inputReverbSwitch
                        onToggled: App.audioFx.inputReverbSwitch = checked
                    }

                    Label { text: "Reverb Type"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    ComboBox {
                        model: App.audioFx.inputReverbTypeNames
                        currentIndex: App.audioFx.inputReverbType
                        onActivated: (index) => { App.audioFx.inputReverbType = index }
                        Layout.fillWidth: true
                    }

                    Label { text: "Reverb Time"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    Slider {
                        from: 0; to: 127
                        value: App.audioFx.inputReverbTime
                        onMoved: App.audioFx.inputReverbTime = Math.round(value)
                        Layout.fillWidth: true
                    }

                    Label { text: "Reverb Level"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    Slider {
                        from: 0; to: 127
                        value: App.audioFx.inputReverbLevel
                        onMoved: App.audioFx.inputReverbLevel = Math.round(value)
                        Layout.fillWidth: true
                    }

                    Label { text: "Noise Suppressor"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    Switch {
                        checked: App.audioFx.nsSwitch
                        onToggled: App.audioFx.nsSwitch = checked
                    }

                    Label { text: "NS Threshold"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    Slider {
                        from: 0; to: 127
                        value: App.audioFx.nsThreshold
                        onMoved: App.audioFx.nsThreshold = Math.round(value)
                        Layout.fillWidth: true
                    }

                    Label { text: "NS Release"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    Slider {
                        from: 0; to: 127
                        value: App.audioFx.nsRelease
                        onMoved: App.audioFx.nsRelease = Math.round(value)
                        Layout.fillWidth: true
                    }
                }
            }

            GroupBox {
                title: "TFX (Total FX)"
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    anchors.fill: parent
                    columnSpacing: 12
                    rowSpacing: 8

                    Label { text: "TFX Switch"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    Switch {
                        checked: App.audioFx.tfxSwitch
                        onToggled: App.audioFx.tfxSwitch = checked
                    }

                    Label { text: "Type"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    ComboBox {
                        model: App.audioFx.tfxTypeNames
                        currentIndex: App.audioFx.tfxType
                        onActivated: (index) => { App.audioFx.tfxType = index }
                        Layout.fillWidth: true
                    }

                    Label { text: "Ctrl 1 (Cutoff / …)"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    Slider {
                        from: 0; to: 127
                        value: App.audioFx.tfxParamA
                        onMoved: App.audioFx.tfxParamA = Math.round(value)
                        Layout.fillWidth: true
                    }

                    Label { text: "Ctrl 2 (Reso / …)"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    Slider {
                        from: 0; to: 127
                        value: App.audioFx.tfxParamB
                        onMoved: App.audioFx.tfxParamB = Math.round(value)
                        Layout.fillWidth: true
                    }

                    Label { text: "Ctrl 3 (Drive / …)"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    Slider {
                        from: 0; to: 127
                        value: App.audioFx.tfxParamC
                        onMoved: App.audioFx.tfxParamC = Math.round(value)
                        Layout.fillWidth: true
                    }
                }
            }

        }
    }
}
