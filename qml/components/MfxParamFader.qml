import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor
import "FlickableGuard.js" as FlickableGuard

/**
 * Compact mixer-style vertical fader for MFX params.
 * Does not assign to `value` while dragging — emit `moved` and let the model
 * NOTIFY refresh the binding (avoids broken bindings / loops).
 */
ColumnLayout {
    id: root
    spacing: 2

    property string label: ""
    property int value: 0
    property int from: 0
    property int to: 127
    property real faderHeight: 72
    property real faderWidth: 28
    property bool showValue: true
    /** True while the thumb is being dragged — parents may pause Flickable. */
    property bool dragging: false

    signal moved(int v)

    property var _lockedFlickables: []

    function _pauseParentFlick() {
        root._lockedFlickables = FlickableGuard.lockFrom(root)
    }

    function _resumeParentFlick() {
        FlickableGuard.unlock(root._lockedFlickables)
        root._lockedFlickables = []
    }

    Layout.preferredWidth: Math.max(root.faderWidth + 8, 44)
    Layout.maximumWidth: 56
    Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

    readonly property real _span: Math.max(1, root.to - root.from)
    readonly property real _t: Math.max(0, Math.min(1, (root.value - root.from) / root._span))

    Item {
        id: faderHost
        Layout.preferredWidth: root.faderWidth
        Layout.preferredHeight: root.faderHeight
        Layout.alignment: Qt.AlignHCenter

        Rectangle {
            id: track
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 2
            width: 5
            radius: 2
            color: LogicTheme.faderTrack

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: parent.height * root._t
                radius: 2
                color: LogicTheme.faderFill
                opacity: 0.45
            }
        }

        Rectangle {
            id: thumb
            width: 18
            height: 11
            radius: 2
            color: LogicTheme.panelBgRaised
            border.color: LogicTheme.hairline
            anchors.horizontalCenter: parent.horizontalCenter
            y: track.height * (1 - root._t) - height / 2

            Rectangle {
                anchors.centerIn: parent
                width: parent.width - 5
                height: 2
                color: LogicTheme.accent
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.SizeVerCursor
            enabled: root.enabled
            // Win over parent VerticalFlick (Tone Edit ScrollView) — mixer faders
            // sit in a horizontal ScrollView so they never hit this conflict.
            preventStealing: true

            function applyAt(my) {
                const t = 1 - Math.min(1, Math.max(0, my / Math.max(1, height)))
                const v = Math.round(root.from + t * root._span)
                root.moved(v)
            }

            onPositionChanged: (mouse) => {
                if (pressed)
                    applyAt(mouse.y)
            }
            onPressed: (mouse) => {
                root.dragging = true
                root._pauseParentFlick()
                applyAt(mouse.y)
            }
            onReleased: {
                root.dragging = false
                root._resumeParentFlick()
            }
            onCanceled: {
                root.dragging = false
                root._resumeParentFlick()
            }
        }
    }

    Label {
        visible: root.showValue
        text: String(root.value)
        color: LogicTheme.textSecondary
        font.pixelSize: LogicTheme.fontSizeSmall
        horizontalAlignment: Text.AlignHCenter
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
    }

    Label {
        text: root.label
        color: LogicTheme.textMuted
        font.pixelSize: LogicTheme.fontSizeSmall
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        elide: Text.ElideRight
        Layout.fillWidth: true
        Layout.preferredWidth: Math.max(root.faderWidth + 8, 44)
        Layout.alignment: Qt.AlignHCenter
    }
}
