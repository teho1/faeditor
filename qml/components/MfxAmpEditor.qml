import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

/** Amp / OD / Dist / Speaker: paramList-backed faders. */
ColumnLayout {
    id: root
    spacing: 8

    required property MfxModel mfx
    property bool editorEnabled: true

    Label {
        text: (root.mfx ? root.mfx.typeName : "Amp") + "  ·  amp"
        color: LogicTheme.textPrimary
        font.bold: true
        font.pixelSize: LogicTheme.fontSize
    }

    MfxParamGrid {
        Layout.fillWidth: true
        mfxModel: root.mfx
        enabled: root.editorEnabled
    }
}
