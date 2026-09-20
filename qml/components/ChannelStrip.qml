import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    id: root
    property int partIndex: 0
    property bool selected: App.studioSet.selectedPart === partIndex
    width: LogicTheme.stripWidth
    color: selected ? LogicTheme.selectedBg : LogicTheme.panelBg
    border.color: LogicTheme.hairline
    border.width: 1

    readonly property var part: App.studioSet.part(partIndex)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: (partIndex + 1).toString()
            color: LogicTheme.textSecondary
            font.pixelSize: LogicTheme.fontSizeSmall
        }

        Label {
            Layout.fillWidth: true
            text: part ? part.toneName : ""
            color: LogicTheme.textPrimary
            font.pixelSize: LogicTheme.fontSizeSmall
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
        }

        PanKnob {
            Layout.alignment: Qt.AlignHCenter
            value: part ? part.pan : 64
            onMoved: (v) => { if (part) part.pan = v }
        }

        Fader {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            value: part ? part.level : 100
            onMoved: (v) => { if (part) part.level = v }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 4

            Button {
                text: "M"
                Layout.preferredWidth: LogicTheme.mobile ? 44 : 24
                Layout.preferredHeight: LogicTheme.mobile ? 44 : 22
                checkable: true
                checked: part ? part.mute : false
                onClicked: if (part) part.mute = checked
                background: Rectangle {
                    color: parent.checked ? LogicTheme.muteRed : LogicTheme.panelBgRaised
                    radius: 3
                    border.color: LogicTheme.hairline
                }
                contentItem: Label {
                    text: parent.text
                    color: LogicTheme.textPrimary
                    font.pixelSize: LogicTheme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                text: "S"
                Layout.preferredWidth: LogicTheme.mobile ? 44 : 24
                Layout.preferredHeight: LogicTheme.mobile ? 44 : 22
                checkable: true
                checked: part ? part.solo : false
                onClicked: if (part) part.solo = checked
                background: Rectangle {
                    color: parent.checked ? LogicTheme.soloYellow : LogicTheme.panelBgRaised
                    radius: 3
                    border.color: LogicTheme.hairline
                }
                contentItem: Label {
                    text: parent.text
                    color: parent.checked ? "#1C1C1E" : LogicTheme.textPrimary
                    font.pixelSize: LogicTheme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        z: -1
        onClicked: App.studioSet.selectedPart = partIndex
    }
}
