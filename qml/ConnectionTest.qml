import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    width: 390
    height: 844
    visible: true
    title: "FA Connection Test"
    palette.window: "#151922"
    palette.windowText: "#eef2f8"
    palette.text: "#eef2f8"
    palette.base: "#242b38"
    palette.button: "#303b4d"
    palette.buttonText: "#eef2f8"
    palette.highlight: "#478ce8"
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        ColumnLayout {
            width: parent.width
            spacing: 14
            Label { text: "FA Connection Test"; font.pixelSize: 24; font.bold: true; Layout.margins: 16 }
            Label {
                text: "Connect your Roland to the iPhone’s USB port. On FA: set USB Driver to GENERIC (MIDI only), save and restart. Choose the music ports, not DAW CTRL."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                Layout.leftMargin: 16; Layout.rightMargin: 16
            }
            Label { text: "MIDI input"; Layout.leftMargin: 16 }
            ComboBox { id: input; model: Probe.inputs; enabled: !Probe.busy; Layout.fillWidth: true; Layout.leftMargin: 16; Layout.rightMargin: 16; implicitHeight: 48 }
            Label { text: "MIDI output"; Layout.leftMargin: 16 }
            ComboBox { id: output; model: Probe.outputs; enabled: !Probe.busy; Layout.fillWidth: true; Layout.leftMargin: 16; Layout.rightMargin: 16; implicitHeight: 48 }
            RowLayout {
                Layout.fillWidth: true; Layout.leftMargin: 16; Layout.rightMargin: 16
                Button { text: "Refresh"; enabled: !Probe.busy; onClicked: Probe.refresh(); implicitHeight: 48 }
                Button {
                    text: Probe.busy ? "Testing…" : "Test connection"
                    enabled: !Probe.busy && input.currentIndex >= 0 && output.currentIndex >= 0
                    highlighted: true; Layout.fillWidth: true; implicitHeight: 48
                    onClicked: Probe.test(input.currentIndex, output.currentIndex)
                }
            }
            Label {
                text: "Reads instrument identity and the current set/scene name. Does not change settings. Keep the app open during the test."
                wrapMode: Text.WordWrap; Layout.fillWidth: true; Layout.leftMargin: 16; Layout.rightMargin: 16
            }
            Button { text: "Copy diagnostic log"; onClicked: Probe.copyLog(); Layout.leftMargin: 16; implicitHeight: 48 }
            TextArea {
                text: Probe.log; readOnly: true; wrapMode: TextEdit.WrapAnywhere
                font.family: "monospace"; font.pixelSize: 13; selectByMouse: true
                Layout.fillWidth: true; Layout.leftMargin: 16; Layout.rightMargin: 16; Layout.bottomMargin: 24
            }
        }
    }
}
