import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    id: root
    color: LogicTheme.windowBg
    clip: true
    property int selectedRow: -1
    signal browseRequested()
    signal closeRequested()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "SVD Tones"
                color: LogicTheme.textPrimary
                font.pixelSize: LogicTheme.fontSizeTitle
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
            Button {
                text: "Open…"
                onClicked: root.browseRequested()
            }
            Button {
                text: "×"
                Accessible.name: "Close SVD tones"
                onClicked: root.closeRequested()
            }
        }

        Label {
            Layout.fillWidth: true
            text: App.svdImport.sourceName + " · " + App.svdImport.toneCount + " SN-A tones"
            color: LogicTheme.textSecondary
            font.pixelSize: LogicTheme.fontSizeSmall
            elide: Text.ElideMiddle
        }

        Label {
            Layout.fillWidth: true
            text: "Select a tone, then push it to the selected Part's Temporary Tone memory."
            wrapMode: Text.WordWrap
            color: LogicTheme.textMuted
            font.pixelSize: LogicTheme.fontSizeSmall
        }

        ListView {
            id: toneList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 1
            model: App.svdImport
            currentIndex: root.selectedRow
            delegate: Rectangle {
                required property int index
                required property int slot
                required property string name
                required property string category
                width: ListView.view.width
                height: 28
                radius: 3
                color: ListView.isCurrentItem ? LogicTheme.selectedBg : LogicTheme.panelBg
                border.color: LogicTheme.hairline

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 6
                    anchors.rightMargin: 6
                    spacing: 5
                    Label {
                        text: slot
                        color: LogicTheme.textMuted
                        font.pixelSize: LogicTheme.fontSizeSmall
                        Layout.preferredWidth: 24
                    }
                    Label {
                        text: name
                        color: LogicTheme.textPrimary
                        font.pixelSize: LogicTheme.fontSizeSmall
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                    Label {
                        text: category
                        color: LogicTheme.textMuted
                        font.pixelSize: LogicTheme.fontSizeSmall
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: root.selectedRow = index
                    onDoubleClicked: {
                        root.selectedRow = index
                        if (App.midi.connected)
                            App.svdImport.pushTone(index)
                    }
                }
            }
        }

        Label {
            Layout.fillWidth: true
            visible: App.svdImport.lastError.length > 0
            text: App.svdImport.lastError
            color: LogicTheme.danger
            wrapMode: Text.WordWrap
            font.pixelSize: LogicTheme.fontSizeSmall
        }

        Button {
            Layout.fillWidth: true
            text: "Push to selected Part"
            enabled: root.selectedRow >= 0 && App.midi.connected
            onClicked: App.svdImport.pushTone(root.selectedRow)
        }
    }
}
