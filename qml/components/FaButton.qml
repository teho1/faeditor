import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

// Custom chip button — macOS Quick Controls style forbids contentItem overrides on Button.
Item {
    id: root
    property string glyph: ""
    property string text: ""
    property color glyphColor: enabled ? LogicTheme.textPrimary : LogicTheme.textMuted
    signal clicked()

    implicitWidth: Math.max(36, row.implicitWidth + 14)
    implicitHeight: Math.max(26, row.implicitHeight + 8)
    opacity: enabled ? 1 : 0.45

    Rectangle {
        anchors.fill: parent
        radius: 5
        color: root.enabled && mouse.pressed
               ? LogicTheme.selectedBg
               : (root.enabled && mouse.containsMouse ? LogicTheme.panelBgRaised : LogicTheme.panelBg)
        border.color: LogicTheme.hairline
        border.width: 1
    }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 6
        FaIcon {
            visible: root.glyph.length > 0
            icon: root.glyph
            size: LogicTheme.fontSize
            iconColor: root.glyphColor
            anchors.verticalCenter: parent.verticalCenter
        }
        Label {
            text: root.text
            color: root.enabled ? LogicTheme.textPrimary : LogicTheme.textMuted
            font.pixelSize: LogicTheme.fontSize
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        enabled: root.enabled
        hoverEnabled: true
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.clicked()
    }
}
