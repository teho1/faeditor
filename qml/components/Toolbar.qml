import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    id: root
    signal connectClicked()
    signal pullClicked()
    signal pushClicked()

    height: LogicTheme.toolbarHeight
    color: LogicTheme.panelBg

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: LogicTheme.hairline
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 6

        Label {
            text: App.fantomDevice ? "FA Editor · FANTOM" : "FA Editor"
            color: LogicTheme.textPrimary
            font.pixelSize: LogicTheme.fontSizeTitle
            font.bold: true
        }

        TextField {
            Layout.preferredWidth: 180
            text: App.fantomDevice ? App.scene.name : App.studioSet.name
            readOnly: App.fantomDevice
            font.pixelSize: LogicTheme.fontSize
            onEditingFinished: if (!App.fantomDevice) App.studioSet.name = text
        }

        Item { Layout.fillWidth: true }

        FaButton {
            glyph: FaIcons.cable
            glyphColor: App.midi.connected ? LogicTheme.success : LogicTheme.danger
            text: "MIDI"
            onClicked: root.connectClicked()
        }
        FaButton {
            glyph: FaIcons.download
            text: "Pull"
            onClicked: root.pullClicked()
        }
        FaButton {
            glyph: FaIcons.upload
            text: "Push"
            onClicked: root.pushClicked()
        }
    }
}
