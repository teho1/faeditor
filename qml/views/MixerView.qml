import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    color: LogicTheme.windowBg

    OverflowFlickable {
        id: mixerFlick
        anchors.fill: parent
        flickableDirection: Flickable.HorizontalFlick
        contentWidth: strips.width
        contentHeight: height
        ScrollBar.horizontal: ScrollBar {
            policy: mixerFlick.contentWidth > mixerFlick.width + 1
                    ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff
        }

        Row {
            id: strips
            height: mixerFlick.height
            spacing: 0
            Repeater {
                model: 16
                ChannelStrip {
                    partIndex: index
                    height: strips.height
                }
            }
        }
    }
}
