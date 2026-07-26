import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

/**
 * Delay family router: type 36 → MfxTapDelayEditor; other delays → paramList faders.
 */
ColumnLayout {
    id: root
    spacing: 8

    required property MfxModel mfx
    property bool editorEnabled: true

    readonly property bool isMultiTap: mfx && mfx.type === 36

    MfxTapDelayEditor {
        Layout.fillWidth: true
        visible: root.isMultiTap
        mfx: root.mfx
        editorEnabled: root.editorEnabled
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 8
        visible: !root.isMultiTap
        enabled: root.editorEnabled && root.mfx

        Label {
            text: (root.mfx ? root.mfx.typeName : "Delay") + "  ·  parameters"
            color: LogicTheme.textPrimary
            font.bold: true
            font.pixelSize: LogicTheme.fontSize
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: LogicTheme.textMuted
            font.pixelSize: LogicTheme.fontSizeSmall
            text: "Rich multi-tap timeline is for 3Tap Pan Delay (type 36). Edit named params below."
        }

        MfxParamGrid {
            Layout.fillWidth: true
            mfxModel: root.mfx
            enabled: root.editorEnabled
        }
    }
}
