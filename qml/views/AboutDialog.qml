import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Dialog {
    id: root
    parent: Overlay.overlay
    anchors.centerIn: parent
    modal: true
    title: "About FA Editor"
    standardButtons: Dialog.Ok
    width: Math.min(560, Overlay.overlay ? Overlay.overlay.width - 40 : 560)
    height: Math.min(560, Overlay.overlay ? Overlay.overlay.height - 40 : 560)

    property string appVersion: "0.1.0"
    property string githubUrl: "https://github.com/teho1/faeditor"

    contentItem: ScrollView {
        clip: true
        ColumnLayout {
            width: root.availableWidth
            spacing: 12

            RowLayout {
                spacing: 12
                Layout.fillWidth: true
                Image {
                    source: "qrc:/qt/qml/FAEditor/resources/icons/appicon.png"
                    sourceSize.width: 64
                    sourceSize.height: 64
                    Layout.preferredWidth: 64
                    Layout.preferredHeight: 64
                    fillMode: Image.PreserveAspectFit
                }
                ColumnLayout {
                    spacing: 2
                    Label {
                        text: "FA Editor"
                        font.pixelSize: LogicTheme.fontSizeTitle + 4
                        font.bold: true
                        color: LogicTheme.textPrimary
                    }
                    Label {
                        text: "Version " + root.appVersion
                        color: LogicTheme.textSecondary
                        font.pixelSize: LogicTheme.fontSizeSmall
                    }
                    Label {
                        text: "Studio Set editor for Roland FA-06 / FA-07 / FA-08"
                        color: LogicTheme.textSecondary
                        font.pixelSize: LogicTheme.fontSizeSmall
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: LogicTheme.textPrimary
                font.pixelSize: LogicTheme.fontSize
                text: "Unofficial third-party software. Not affiliated with, endorsed by, or connected to Roland Corporation. “Roland”, “FA-06”, “FA-07”, and “FA-08” are trademarks of their respective owners."
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: LogicTheme.textSecondary
                font.pixelSize: LogicTheme.fontSizeSmall
                text: "Source code is published on GitHub (MIT License). The macOS App Store build is offered as a paid binary (USD $4.99) to support ongoing development. Purchasing the App Store build does not grant ownership of Roland sound data or hardware."
            }

            Button {
                text: "Open GitHub repository"
                onClicked: Qt.openUrlExternally(root.githubUrl)
            }

            Label {
                text: "Qt libraries"
                font.bold: true
                color: LogicTheme.textPrimary
                Layout.topMargin: 4
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
                color: LogicTheme.textSecondary
                font.pixelSize: LogicTheme.fontSizeSmall
                text: App.qtLicenseNotice
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }

            Label {
                text: "Other open source components"
                font.bold: true
                color: LogicTheme.textPrimary
                Layout.topMargin: 4
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: LogicTheme.textSecondary
                font.pixelSize: LogicTheme.fontSizeSmall
                text:
                    "• RtMidi — © Gary P. Scavone (MIT-style license).\n\n"
                    + "• Font Awesome Free — icons/fonts under the Font Awesome Free License (CC BY 4.0 for icons, SIL OFL 1.1 for fonts, MIT for code).\n\n"
                    + "• Material Icons — © Google LLC (Apache License 2.0).\n\n"
                    + "• Tone names/catalog derived from Roland’s publicly documented Sound List (reference data for FA-06/07/08). Parameter editing uses Roland’s published MIDI / SysEx documentation.\n\n"
                    + "See `LICENSE` and `THIRD_PARTY_NOTICES.md` in the GitHub repository for full texts."
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: LogicTheme.textMuted
                font.pixelSize: LogicTheme.fontSizeSmall
                text: "FA Editor is provided without warranty; use at your own risk."
            }
        }
    }
}
