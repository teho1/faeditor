import QtQuick
import FAEditor

Text {
    id: root
    property string icon: FaIcons.music
    property int size: LogicTheme.fontSize
    property color iconColor: LogicTheme.textPrimary
    property string iconFamily: FaIcons.fontForGlyph(icon)

    text: icon
    color: iconColor
    font.family: iconFamily
    font.weight: iconFamily === FaIcons.faFamily ? Font.Black : Font.Normal
    font.pixelSize: size
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
}
