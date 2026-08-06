import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Dialog {
    id: root
    parent: Overlay.overlay
    anchors.centerIn: parent
    modal: true
    title: "Import tones from " + App.svdImport.sourceName
    width: 560
    height: Math.min(720, parent ? parent.height - 80 : 720)
    standardButtons: Dialog.Close
    property int selectedRow: -1

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            Layout.fillWidth: true
            text: "Select an SN-A tone and push it to the selected part's Temporary Tone memory. This does not overwrite a permanent User tone."
            wrapMode: Text.WordWrap
            color: LogicTheme.textSecondary
        }

        ListView {
            id: toneList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: App.svdImport
            currentIndex: root.selectedRow
            delegate: ItemDelegate {
                required property int index
                required property int slot
                required property string name
                required property string category
                width: ListView.view.width
                highlighted: ListView.isCurrentItem
                text: slot.toString().padStart(3, "0") + "   " + name + "   · " + category
                onClicked: root.selectedRow = index
            }
        }

        Label {
            Layout.fillWidth: true
            visible: App.svdImport.lastError.length > 0
            text: App.svdImport.lastError
            color: LogicTheme.danger
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button {
                text: "Push to selected Part"
                enabled: root.selectedRow >= 0 && App.midi.connected
                onClicked: App.svdImport.pushTone(root.selectedRow)
            }
        }
    }
}
