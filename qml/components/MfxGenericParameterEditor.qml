import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

/** Fallback MFX editor: catalog-named vertical fader row (paramList roles). */
ColumnLayout {
    id: root
    spacing: 8

    required property MfxModel mfx
    property bool editorEnabled: true
    property string title: "Parameters"

    Label {
        text: root.mfx && root.mfx.paramCount > 0
              ? (root.title + "  ·  " + root.mfx.paramCount)
              : (root.title + "  ·  none (Thru)")
        color: LogicTheme.textSecondary
        font.pixelSize: LogicTheme.fontSizeSmall
    }

    MfxParamGrid {
        Layout.fillWidth: true
        visible: root.mfx && root.mfx.paramCount > 0
        mfxModel: root.mfx
        enabled: root.editorEnabled
    }
}
