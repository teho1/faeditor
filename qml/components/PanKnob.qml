import QtQuick
import FAEditor

Item {
    id: root
    property real value: 64
    signal moved(real v)
    width: 36
    height: 36

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: LogicTheme.panelBgRaised
        border.color: LogicTheme.hairline

        Rectangle {
            width: 2
            height: parent.height * 0.35
            radius: 1
            color: LogicTheme.accent
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.verticalCenter
            transformOrigin: Item.Bottom
            rotation: (root.value - 64) * (135 / 64)
        }
    }

    MouseArea {
        anchors.fill: parent
        // Horizontal drags otherwise get stolen by the mixer ScrollView after
        // the platform drag threshold (about 10 px).
        preventStealing: true
        property real startX
        property real startVal
        onPressed: (mouse) => { startX = mouse.x; startVal = root.value }
        onPositionChanged: (mouse) => {
            if (!pressed) return
            const delta = mouse.x - startX
            root.value = Math.round(Math.min(127, Math.max(0, startVal + delta)))
            root.moved(root.value)
        }
    }
}
