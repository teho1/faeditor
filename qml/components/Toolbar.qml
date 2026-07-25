import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    id: root
    signal connectClicked()
    signal pullClicked()
    signal pushClicked()
    signal saveClicked()

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
            text: "FA Editor"
            color: LogicTheme.textPrimary
            font.pixelSize: LogicTheme.fontSizeTitle
            font.bold: true
        }

        TextField {
            Layout.preferredWidth: 180
            text: App.studioSet.name
            font.pixelSize: LogicTheme.fontSize
            onEditingFinished: App.studioSet.name = text
        }

        Item { Layout.fillWidth: true }

        FaButton {
            glyph: FaIcons.cable
            glyphColor: App.midi.connected ? LogicTheme.success : LogicTheme.textMuted
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
        FaButton {
            glyph: FaIcons.save
            text: "Save"
            onClicked: root.saveClicked()
        }
    }
}
