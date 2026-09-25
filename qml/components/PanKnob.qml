import QtQuick
import FAEditor
import "FlickableGuard.js" as FlickableGuard

Item {
    id: root
    property real value: 64
    property real from: 0
    property real to: 127
    property real defaultValue: 64
    property real dragSensitivity: 1
    property real fineScale: 0.2
    signal moved(real v)
    width: LogicTheme.mobile ? 44 : 36
    height: width

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: LogicTheme.panelBgRaised
        border.color: LogicTheme.hairline

        // Fixed centre detent: pan centre remains visible even away from centre.
        Rectangle {
            width: 3
            height: 3
            radius: 1.5
            color: root.value === root.defaultValue ? LogicTheme.accent : LogicTheme.textMuted
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 3
        }

        Rectangle {
            width: 2
            height: parent.height * 0.35
            radius: 1
            color: LogicTheme.accent
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.verticalCenter
            transformOrigin: Item.Bottom
            rotation: root.value >= root.defaultValue
                      ? 135 * (root.value - root.defaultValue) / Math.max(1, root.to - root.defaultValue)
                      : -135 * (root.defaultValue - root.value) / Math.max(1, root.defaultValue - root.from)
        }
    }

    property var _lockedFlickables: []

    MouseArea {
        anchors.fill: parent
        // Horizontal drags otherwise get stolen by the mixer Flickable after
        // the platform drag threshold (about 10 px).
        preventStealing: true
        cursorShape: Qt.SizeVerCursor
        property real lastY
        property real dragValue
        onPressed: (mouse) => {
            root._lockedFlickables = FlickableGuard.lockFrom(root)
            lastY = mouse.y
            dragValue = root.value
        }
        onReleased: {
            FlickableGuard.unlock(root._lockedFlickables)
            root._lockedFlickables = []
        }
        onCanceled: {
            FlickableGuard.unlock(root._lockedFlickables)
            root._lockedFlickables = []
        }
        onPositionChanged: (mouse) => {
            if (!pressed) return
            const fine = (mouse.modifiers & Qt.ShiftModifier) !== 0
            dragValue += (lastY - mouse.y) * root.dragSensitivity * (fine ? root.fineScale : 1)
            lastY = mouse.y
            dragValue = Math.min(root.to, Math.max(root.from, dragValue))
            const next = Math.round(dragValue)
            if (next !== root.value) {
                root.value = next
                root.moved(next)
            }
        }
        onDoubleClicked: {
            const reset = Math.round(Math.min(root.to, Math.max(root.from, root.defaultValue)))
            if (reset !== root.value) {
                root.value = reset
                root.moved(reset)
            }
        }
    }
}
