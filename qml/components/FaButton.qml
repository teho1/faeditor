import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

// Custom chip button — macOS Quick Controls style forbids contentItem overrides on Button.
Item {
    id: root
    property string glyph: ""
    property string text: ""
    property string secondaryText: ""
    property color glyphColor: enabled ? LogicTheme.textPrimary : LogicTheme.textMuted
    property bool checkable: false
    property bool checked: false
    readonly property bool down: enabled && (checked || mouse.pressed)
    readonly property bool hovered: mouse.containsMouse
    signal clicked()
    signal toggled()

    implicitWidth: Math.max(LogicTheme.mobile ? 44 : 36, contentCol.implicitWidth + 14)
    implicitHeight: Math.max(LogicTheme.mobile ? 44 : 26, contentCol.implicitHeight + 8)
    opacity: enabled ? 1 : 0.45

    Rectangle {
        id: face
        anchors.fill: parent
        anchors.topMargin: root.down ? 1 : 0
        anchors.bottomMargin: root.down ? 0 : 1
        radius: 5
        color: {
            if (!root.enabled)
                return LogicTheme.panelBg
            if (root.down)
                return LogicTheme.selectedBg
            return LogicTheme.panelBgRaised
        }
        border.color: root.down ? LogicTheme.accent : LogicTheme.hairline
        border.width: root.down ? 2 : 1
    }

    Column {
        id: contentCol
        anchors.centerIn: parent
        anchors.verticalCenterOffset: root.down ? 1 : 0
        spacing: 1

        Row {
            id: row
            anchors.horizontalCenter: parent.horizontalCenter
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

        Label {
            visible: root.secondaryText.length > 0
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.secondaryText
            color: root.enabled ? LogicTheme.textSecondary : LogicTheme.textMuted
            font.pixelSize: LogicTheme.fontSizeSmall
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        enabled: root.enabled
        hoverEnabled: true
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: {
            if (root.checkable) {
                root.checked = !root.checked
                root.toggled()
            }
            root.clicked()
        }
    }
}
