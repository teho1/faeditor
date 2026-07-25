import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    color: LogicTheme.windowBg

    ScrollView {
        anchors.fill: parent
        clip: true

        Row {
            spacing: 0
            Repeater {
                model: 16
                ChannelStrip {
                    partIndex: index
                    height: Math.max(420, parent.parent ? parent.parent.height : 420)
                }
            }
        }
    }
}
