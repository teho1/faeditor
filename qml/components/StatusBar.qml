import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    height: LogicTheme.statusHeight
    color: LogicTheme.panelBg

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: LogicTheme.hairline
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10

        FaIcon {
            icon: FaIcons.circle
            size: LogicTheme.fontSizeSmall - 1
            iconColor: App.midi.connected ? LogicTheme.success : LogicTheme.textMuted
        }
        Label {
            text: App.workflowHint.length ? App.workflowHint : App.midi.statusText
            color: App.midi.connected ? LogicTheme.textSecondary : LogicTheme.textMuted
            font.pixelSize: LogicTheme.fontSizeSmall
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        Item { Layout.fillWidth: true }

        Label {
            visible: App.studioSet.busy
            text: "Busy…"
            color: LogicTheme.warning
            font.pixelSize: LogicTheme.fontSizeSmall
        }

        Label {
            visible: App.studioSet.lastError.length > 0
            text: App.studioSet.lastError
            color: LogicTheme.danger
            font.pixelSize: LogicTheme.fontSizeSmall
            elide: Text.ElideRight
            Layout.maximumWidth: 400
        }

        Label {
            text: App.studioSet.dirty ? "● Edited" : "Saved"
            color: App.studioSet.dirty ? LogicTheme.warning : LogicTheme.textMuted
            font.pixelSize: LogicTheme.fontSizeSmall
        }
    }
}
