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
    width: 480
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

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Label {
            text: "MIDI Connection"
            color: LogicTheme.textPrimary
            font.pixelSize: LogicTheme.fontSizeTitle
            font.bold: true
        }

        Label {
            text: "Use FA music ports — ignore DAW CTRL / Mackie Control. After power-cycling the FA, click Refresh or Auto FA."
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

        RowLayout {
            Layout.fillWidth: true
            Button {
                text: "Refresh"
                onClicked: App.midi.refresh()
            }
            Button {
                text: "Auto FA"
                onClicked: {
                    if (App.midi.autoConnectFa())
                        root.close()
                }
            }
            Item { Layout.fillWidth: true }
            Button {
                text: "Cancel"
                onClicked: root.close()
            }
            Button {
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
