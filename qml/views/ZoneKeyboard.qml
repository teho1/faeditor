import QtQuick
import QtQuick.Controls
import FAEditor

Item {
    id: root
    readonly property var part: App.studioSet.selectedPartModel

    Column {
        anchors.fill: parent
        spacing: 4

        Label {
            text: "Key Range"
            color: LogicTheme.textSecondary
            font.pixelSize: LogicTheme.fontSizeSmall
        }

        Item {
            id: keyboard
            width: parent.width
            height: 48

            property int low: part ? part.keyLow : 0
            property int high: part ? part.keyHigh : 127

            Row {
                anchors.fill: parent
                spacing: 0
                Repeater {
                    model: 128
                    Rectangle {
                        width: keyboard.width / 128
                        height: parent.height
                        color: {
                            const n = index % 12
                            const black = (n === 1 || n === 3 || n === 6 || n === 8 || n === 10)
                            if (index >= keyboard.low && index <= keyboard.high)
                                return LogicTheme.zoneFill
                            return black ? LogicTheme.keyBlack : LogicTheme.keyWhite
                        }
                        border.width: 0
                    }
                }
            }

            // Low handle
            Rectangle {
                x: (keyboard.low / 127) * keyboard.width - 4
                width: 8
                height: parent.height
                color: LogicTheme.accent
                opacity: 0.9
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.SizeHorCursor
                    drag.target: parent
                    drag.axis: Drag.XAxis
                    drag.minimumX: -4
                    drag.maximumX: keyboard.width - 4
                    onPositionChanged: {
                        if (!pressed || !part) return
                        const k = Math.round(((parent.x + 4) / keyboard.width) * 127)
                        part.keyLow = Math.min(k, part.keyHigh)
                    }
                }
            }

            // High handle
            Rectangle {
                x: (keyboard.high / 127) * keyboard.width - 4
                width: 8
                height: parent.height
                color: LogicTheme.accent
                opacity: 0.9
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.SizeHorCursor
                    drag.target: parent
                    drag.axis: Drag.XAxis
                    drag.minimumX: -4
                    drag.maximumX: keyboard.width - 4
                    onPositionChanged: {
                        if (!pressed || !part) return
                        const k = Math.round(((parent.x + 4) / keyboard.width) * 127)
                        part.keyHigh = Math.max(k, part.keyLow)
                    }
                }
            }
        }

        Label {
            text: (part ? part.keyLow : 0) + " — " + (part ? part.keyHigh : 127)
            color: LogicTheme.textPrimary
            font.pixelSize: LogicTheme.fontSizeSmall
        }
    }
}
