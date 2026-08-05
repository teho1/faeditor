import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FantomEditor

ApplicationWindow {
    width: 1000
    height: 680
    minimumWidth: 760
    minimumHeight: 520
    visible: true
    title: App.productName
    color: LogicTheme.windowBg

    menuBar: MenuBar {
        Menu {
            title: "File"
            Action { text: "Quit"; shortcut: "Ctrl+Q"; onTriggered: Qt.quit() }
        }
        Menu {
            title: "Help"
            Action { text: "About Fantom Editor"; onTriggered: aboutDialog.open() }
        }
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            Label {
                text: "FANTOM"
                color: LogicTheme.accent
                font.pixelSize: 20
                font.bold: true
            }
            Item { Layout.fillWidth: true }
            Label { text: App.deviceProfile; color: LogicTheme.textSecondary }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 20

        Label {
            text: "Fantom workspace"
            color: LogicTheme.textPrimary
            font.pixelSize: 30
            font.bold: true
        }
        Label {
            Layout.fillWidth: true
            text: "A dedicated product shell for Roland Fantom workflows."
            color: LogicTheme.textSecondary
            font.pixelSize: 16
            wrapMode: Text.WordWrap
        }

        Frame {
            Layout.fillWidth: true
            padding: 22
            background: Rectangle {
                color: LogicTheme.panelBg
                border.color: LogicTheme.border
                radius: 8
            }
            ColumnLayout {
                anchors.fill: parent
                spacing: 12
                Label {
                    text: "Device editing"
                    color: LogicTheme.textPrimary
                    font.pixelSize: 20
                    font.bold: true
                }
                Label {
                    Layout.fillWidth: true
                    text: App.implementationStatus
                    color: LogicTheme.warning
                    wrapMode: Text.WordWrap
                }
                Label {
                    Layout.fillWidth: true
                    text: "Verified Fantom identity, scene, zone, tone, and effects capabilities will appear here as documented adapters are added."
                    color: LogicTheme.textSecondary
                    wrapMode: Text.WordWrap
                }
            }
        }

        Frame {
            Layout.fillWidth: true
            padding: 22
            background: Rectangle {
                color: LogicTheme.panelBg
                border.color: LogicTheme.border
                radius: 8
            }
            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                Label {
                    text: "Backup library"
                    color: LogicTheme.textPrimary
                    font.pixelSize: 20
                    font.bold: true
                }
                Label {
                    Layout.fillWidth: true
                    text: App.localLibraryAvailable
                          ? "The common local-library capability is reserved for Fantom backup workflows. Device import and restore are not implemented."
                          : "Backup workflows are unavailable."
                    color: LogicTheme.textSecondary
                    wrapMode: Text.WordWrap
                }
            }
        }
        Item { Layout.fillHeight: true }
    }

    Dialog {
        id: aboutDialog
        anchors.centerIn: parent
        title: "About Fantom Editor"
        standardButtons: Dialog.Ok
        Label {
            width: 420
            text: "Fantom Editor " + Qt.application.version + "\n\nShared FAEditor infrastructure with a separate Fantom product workflow. Device editing is intentionally disabled until implemented from verified documentation."
            color: LogicTheme.textPrimary
            wrapMode: Text.WordWrap
        }
    }
}
