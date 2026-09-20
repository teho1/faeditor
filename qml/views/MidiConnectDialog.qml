import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Popup {
    id: root
    parent: Overlay.overlay
    anchors.centerIn: parent
    modal: true
    focus: true
    width: Math.min(480, parent ? parent.width - 24 : 480)
    height: Math.min(body.implicitHeight + padding * 2, parent ? parent.height - 24 : body.implicitHeight + padding * 2)
    padding: 16
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    signal dismissed()

    // Drive open/close from App flag (visible binding alone is unreliable with Overlay).
    Connections {
        target: App
        function onConnectDialogOpenChanged() {
            if (App.connectDialogOpen)
                root.openDialog()
            else if (root.visible)
                root.close()
        }
    }

    onClosed: {
        if (App.connectDialogOpen)
            App.connectDialogOpen = false
        dismissed()
    }

    function openDialog() {
        App.midi.refresh()
        open()
    }

    background: Rectangle {
        color: LogicTheme.panelBg
        radius: 8
        border.color: LogicTheme.hairline
    }

    contentItem: ScrollView {
        clip: true
        contentWidth: availableWidth
        ColumnLayout {
        id: body
        width: parent.width
        spacing: 10

        Label {
            text: "MIDI Connection"
            color: LogicTheme.textPrimary
            font.pixelSize: LogicTheme.fontSizeTitle
            font.bold: true
        }

        Label {
            text: LogicTheme.mobile ? "FA USB Driver must be GENERIC (MIDI only), saved, then restarted. Choose music ports; ignore DAW CTRL and network sessions." : "Use the FA or FANTOM music ports — ignore DAW CTRL / Mackie Control. Device type is identified automatically after connecting."
            color: LogicTheme.textSecondary
            font.pixelSize: LogicTheme.fontSizeSmall
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Label {
                text: "Input"
                color: LogicTheme.textSecondary
                font.pixelSize: LogicTheme.fontSizeSmall
                Layout.preferredWidth: 60
            }
            ComboBox {
                implicitHeight: LogicTheme.mobile ? 44 : implicitContentHeight + topPadding + bottomPadding
                id: inBox
                Layout.fillWidth: true
                model: App.midi.inputPortNames
                currentIndex: Math.max(0, Math.min(count - 1, App.midi.selectedInput))
                onActivated: (index) => { App.midi.selectedInput = index }
            }
        }

        RowLayout {
            Label {
                text: "Output"
                color: LogicTheme.textSecondary
                font.pixelSize: LogicTheme.fontSizeSmall
                Layout.preferredWidth: 60
            }
            ComboBox {
                implicitHeight: LogicTheme.mobile ? 44 : implicitContentHeight + topPadding + bottomPadding
                id: outBox
                Layout.fillWidth: true
                model: App.midi.outputPortNames
                currentIndex: Math.max(0, Math.min(count - 1, App.midi.selectedOutput))
                onActivated: (index) => { App.midi.selectedOutput = index }
            }
        }

        Label {
            text: App.midi.statusText
            color: LogicTheme.textMuted
            font.pixelSize: LogicTheme.fontSizeSmall
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }

        Item { Layout.preferredHeight: 8 }

        Flow {
            Layout.fillWidth: true
            spacing: 8
            Button {
                implicitHeight: LogicTheme.mobile ? 44 : implicitContentHeight + topPadding + bottomPadding
                text: "Refresh"
                onClicked: App.midi.refresh()
            }
            Button {
                implicitHeight: LogicTheme.mobile ? 44 : implicitContentHeight + topPadding + bottomPadding
                text: "Auto"
                onClicked: {
                    if (App.midi.autoConnectInstrument())
                        root.close()
                }
            }
            Button {
                implicitHeight: LogicTheme.mobile ? 44 : implicitContentHeight + topPadding + bottomPadding
                text: "Cancel"
                onClicked: root.close()
            }
            Button {
                implicitHeight: LogicTheme.mobile ? 44 : implicitContentHeight + topPadding + bottomPadding
                text: "Connect"
                highlighted: true
                onClicked: {
                    App.midi.selectedInput = inBox.currentIndex
                    App.midi.selectedOutput = outBox.currentIndex
                    if (App.midi.connectSelected())
                        root.close()
                }
            }
        }
    }
    }
}
