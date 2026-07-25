import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    color: LogicTheme.windowBg

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Parts list — pick which part to revoice
        Rectangle {
            Layout.preferredWidth: parent.width * 0.42
            Layout.fillHeight: true
            color: LogicTheme.windowBg

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                Label {
                    text: "Parts — pick one to change"
                    color: LogicTheme.textPrimary
                    font.pixelSize: LogicTheme.fontSizeTitle
                    font.bold: true
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    color: LogicTheme.textSecondary
                    font.pixelSize: LogicTheme.fontSizeSmall
                    text: "Studio Set: “" + App.studioSet.name + "”. Click Change on a part, then double-click a tone on the right."
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: App.studioSet
                    spacing: 2
                    delegate: Rectangle {
                        required property int index
                        required property int partNumber
                        required property string toneName
                        required property int bankMsb
                        required property int bankLsb
                        required property int program
                        required property bool keyboardSwitch

                        width: ListView.view.width
                        height: 44
                        radius: 4
                        color: App.studioSet.selectedPart === index ? LogicTheme.selectedBg : LogicTheme.panelBg
                        border.color: LogicTheme.hairline

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 8

                            Label {
                                text: partNumber
                                color: LogicTheme.textMuted
                                font.pixelSize: LogicTheme.fontSizeSmall
                                Layout.preferredWidth: 22
                            }
                            FaIcon {
                                icon: FaIcons.forCategory(App.tones.resolveCategory(bankMsb, bankLsb, program))
                                size: LogicTheme.fontSize + 2
                                iconColor: LogicTheme.textSecondary
                                Layout.preferredWidth: 22
                            }
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                Label {
                                    text: toneName
                                    color: LogicTheme.textPrimary
                                    font.pixelSize: LogicTheme.fontSize
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                                Label {
                                    text: bankMsb + ":" + bankLsb + ":" + (program + 1)
                                          + (keyboardSwitch ? "" : "  · KB off")
                                    color: LogicTheme.textMuted
                                    font.pixelSize: LogicTheme.fontSizeSmall
                                }
                            }
                            FaButton {
                                glyph: FaIcons.exchange
                                text: "Change"
                                onClicked: App.goChangeToneForPart(index)
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            z: -1
                            onClicked: App.studioSet.selectedPart = index
                            onDoubleClicked: App.goChangeToneForPart(index)
                        }
                    }
                }
            }
        }

        Rectangle {
            width: 1
            Layout.fillHeight: true
            color: LogicTheme.hairline
        }

        // Tone browser for selected part
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            ToneBrowser {
                anchors.fill: parent
            }
        }
    }
}
