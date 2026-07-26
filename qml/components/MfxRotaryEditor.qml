import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

/** Rotary speaker: rotor viz + paramList-backed faders. */
ColumnLayout {
    id: root
    spacing: 8

    required property MfxModel mfx
    property bool editorEnabled: true

    function p(name) { return root.mfx ? root.mfx.paramIndexByName(name) : -1 }
    readonly property var paramValues: root.mfx ? root.mfx.paramValues : []
    readonly property int speedIdx: root.p("Speed")
    readonly property bool fast: {
        if (root.speedIdx < 0 || !root.mfx) return false
        const v = Number(root.paramValues[root.speedIdx])
        return v > 0
    }

    Label {
        text: (root.mfx ? root.mfx.typeName : "Rotary") + "  ·  rotor"
        color: LogicTheme.textPrimary
        font.bold: true
        font.pixelSize: LogicTheme.fontSize
    }

    Item {
        Layout.fillWidth: true
        Layout.preferredHeight: 120
        Layout.maximumHeight: 130

        Rectangle {
            id: rotor
            width: 90
            height: 90
            radius: 45
            anchors.centerIn: parent
            color: LogicTheme.dark ? "#2A2A2E" : "#D8D8DE"
            border.color: LogicTheme.accent
            border.width: 2

            Rectangle {
                width: 8
                height: 32
                radius: 2
                color: LogicTheme.accent
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 10
            }

            RotationAnimation on rotation {
                from: 0
                to: 360
                loops: Animation.Infinite
                duration: root.fast ? 700 : 2200
                running: root.editorEnabled && root.visible
            }
        }

        Label {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.fast ? "Fast" : "Slow"
            color: LogicTheme.textSecondary
            font.pixelSize: LogicTheme.fontSizeSmall
        }
    }

    MfxParamGrid {
        Layout.fillWidth: true
        mfxModel: root.mfx
        enabled: root.editorEnabled
    }
}
