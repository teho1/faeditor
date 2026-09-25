import QtQuick
import QtQuick.Controls
import FAEditor
import "FlickableGuard.js" as FlickableGuard

Item {
    id: root
    property real value: 100
    property real from: 0
    property real to: 127
    signal moved(real v)

    width: LogicTheme.mobile ? 44 : 28
    height: 160

    Rectangle {
        id: track
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 4
        width: 6
        radius: 3
        color: LogicTheme.faderTrack

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.height * ((root.value - root.from) / Math.max(1, root.to - root.from))
            radius: 3
            color: LogicTheme.faderFill
            opacity: 0.45
        }
    }

    Rectangle {
        id: thumb
        width: 22
        height: 14
        radius: 3
        color: LogicTheme.panelBgRaised
        border.color: LogicTheme.hairline
        anchors.horizontalCenter: parent.horizontalCenter
        y: track.height - (track.height * ((root.value - root.from) / Math.max(1, root.to - root.from))) - height / 2

        Rectangle {
            anchors.centerIn: parent
            width: parent.width - 6
            height: 2
            color: LogicTheme.accent
        }
    }

    property var _lockedFlickables: []

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.SizeVerCursor
        // Keep the pointer grab after Qt's drag threshold instead of letting
        // a parent Flickable cancel the fader.
        preventStealing: true
        onPositionChanged: (mouse) => {
            if (!pressed) return
            const t = 1 - Math.min(1, Math.max(0, mouse.y / height))
            root.value = Math.round(root.from + t * (root.to - root.from))
            root.moved(root.value)
        }
        onPressed: (mouse) => {
            root._lockedFlickables = FlickableGuard.lockFrom(root)
            const t = 1 - Math.min(1, Math.max(0, mouse.y / height))
            root.value = Math.round(root.from + t * (root.to - root.from))
            root.moved(root.value)
        }
        onReleased: {
            FlickableGuard.unlock(root._lockedFlickables)
            root._lockedFlickables = []
        }
        onCanceled: {
            FlickableGuard.unlock(root._lockedFlickables)
            root._lockedFlickables = []
        }
    }
}
