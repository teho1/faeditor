import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    color: LogicTheme.windowBg

    OverflowFlickable {
        anchors.fill: parent
        anchors.margins: 16
        contentHeight: col.height
        flickableDirection: Flickable.VerticalFlick

        ColumnLayout {
            id: col
            width: parent.width
            spacing: 16

            Label {
                text: "Studio Set Effects"
                color: LogicTheme.textPrimary
                font.pixelSize: LogicTheme.fontSizeTitle; font.bold: true
            }

            GroupBox {
                title: "Chorus"
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    anchors.fill: parent
                    Label { text: "Type"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    SpinBox {
                        from: 0; to: 127
                        value: App.studioSet.effects.chorusType
                        onValueModified: App.studioSet.effects.chorusType = value
                    }
                    Label { text: "Level"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    FaSlider {
                        from: 0; to: 127
                        value: App.studioSet.effects.chorusLevel
                        onMoved: App.studioSet.effects.chorusLevel = Math.round(value)
                        Layout.fillWidth: true
                    }
                }
            }

            GroupBox {
                title: "Reverb"
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    anchors.fill: parent
                    Label { text: "Type"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    SpinBox {
                        from: 0; to: 127
                        value: App.studioSet.effects.reverbType
                        onValueModified: App.studioSet.effects.reverbType = value
                    }
                    Label { text: "Level"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    FaSlider {
                        from: 0; to: 127
                        value: App.studioSet.effects.reverbLevel
                        onMoved: App.studioSet.effects.reverbLevel = Math.round(value)
                        Layout.fillWidth: true
                    }
                }
            }

            GroupBox {
                title: "Master Compressor"
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    anchors.fill: parent
                    Label { text: "Switch"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    Switch {
                        checked: App.studioSet.effects.masterCompSwitch
                        onToggled: App.studioSet.effects.masterCompSwitch = checked
                    }
                    Label { text: "Attack"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    FaSlider {
                        from: 0; to: 127
                        value: App.studioSet.effects.masterCompAttack
                        onMoved: App.studioSet.effects.masterCompAttack = Math.round(value)
                        Layout.fillWidth: true
                    }
                    Label { text: "Release"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    FaSlider {
                        from: 0; to: 127
                        value: App.studioSet.effects.masterCompRelease
                        onMoved: App.studioSet.effects.masterCompRelease = Math.round(value)
                        Layout.fillWidth: true
                    }
                    Label { text: "Threshold"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    FaSlider {
                        from: 0; to: 127
                        value: App.studioSet.effects.masterCompThreshold
                        onMoved: App.studioSet.effects.masterCompThreshold = Math.round(value)
                        Layout.fillWidth: true
                    }
                    Label { text: "Ratio"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    FaSlider {
                        from: 0; to: 127
                        value: App.studioSet.effects.masterCompRatio
                        onMoved: App.studioSet.effects.masterCompRatio = Math.round(value)
                        Layout.fillWidth: true
                    }
                    Label { text: "Gain"; color: LogicTheme.textSecondary; font.pixelSize: LogicTheme.fontSizeSmall }
                    FaSlider {
                        from: 0; to: 127
                        value: App.studioSet.effects.masterCompGain
                        onMoved: App.studioSet.effects.masterCompGain = Math.round(value)
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
