import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    id: root
    color: LogicTheme.windowBg
    property bool svdImportVisible: false
    signal browseSvdRequested()

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Device User/Preset recall — compact toolbar (not a left-column list)
        StudioSetBrowser {
            compact: true
            Layout.fillWidth: true
            Layout.preferredHeight: implicitHeight
            Layout.maximumHeight: implicitHeight
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Left — parts list (equal third with Tones / Library)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Layout.minimumWidth: 0
                Layout.fillHeight: true
                clip: true
                color: LogicTheme.windowBg

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4
                    width: parent.width

                    Label {
                        text: "Parts"
                        color: LogicTheme.textPrimary
                        font.pixelSize: LogicTheme.fontSizeTitle
                        font.bold: true
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        elide: Text.ElideRight
                        color: LogicTheme.textSecondary
                        font.pixelSize: LogicTheme.fontSizeSmall
                        text: "“" + App.studioSet.name + "” — select, then pick a tone"
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: App.studioSet
                        spacing: 1
                        delegate: Rectangle {
                            required property int index
                            required property int partNumber
                            required property string toneName
                            required property int bankMsb
                            required property int bankLsb
                            required property int program

                            width: ListView.view.width
                            height: 28
                            radius: 3
                            color: App.studioSet.selectedPart === index ? LogicTheme.selectedBg : LogicTheme.panelBg
                            border.color: LogicTheme.hairline

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 6
                                anchors.rightMargin: 6
                                spacing: 4

                                Label {
                                    text: partNumber
                                    color: LogicTheme.textMuted
                                    font.pixelSize: LogicTheme.fontSizeSmall
                                    Layout.preferredWidth: 16
                                }
                                FaIcon {
                                    icon: FaIcons.forCategory(App.tones.resolveCategory(bankMsb, bankLsb, program))
                                    size: LogicTheme.fontSize
                                    iconColor: LogicTheme.textSecondary
                                    Layout.preferredWidth: 16
                                }
                                Label {
                                    text: toneName
                                    color: LogicTheme.textPrimary
                                    font.pixelSize: LogicTheme.fontSizeSmall
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: App.studioSet.selectedPart = index
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

            // Center — tone browser for selected part
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Layout.minimumWidth: 0
                Layout.fillHeight: true
                clip: true
                color: LogicTheme.windowBg

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 6
                    width: parent.width

                    Label {
                        text: "Tones"
                        color: LogicTheme.textPrimary
                        font.pixelSize: LogicTheme.fontSizeTitle
                        font.bold: true
                    }

                    ToneBrowser {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumWidth: 0
                        narrow: true
                    }
                }
            }

            Rectangle {
                width: 1
                Layout.fillHeight: true
                color: LogicTheme.hairline
            }

            // Right — local studio-set library (store / load)
            LibraryView {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Layout.minimumWidth: 0
                Layout.fillHeight: true
                embedded: true
            }

            Rectangle {
                visible: root.svdImportVisible
                width: visible ? 1 : 0
                Layout.fillHeight: true
                color: LogicTheme.hairline
            }

            SvdImportPanel {
                visible: root.svdImportVisible
                Layout.fillWidth: visible
                Layout.preferredWidth: visible ? 1 : 0
                Layout.minimumWidth: 0
                Layout.fillHeight: true
                onBrowseRequested: root.browseSvdRequested()
                onCloseRequested: root.svdImportVisible = false
            }
        }
    }
}
