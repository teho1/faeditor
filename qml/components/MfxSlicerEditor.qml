import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

/** Slicer: 16-step grid (paramValues NOTIFY) + paramList faders (non-step). */
ColumnLayout {
    id: root
    spacing: 8

    required property MfxModel mfx
    property bool editorEnabled: true

    readonly property var paramValues: root.mfx ? root.mfx.paramValues : []

    function stepIdx(n) {
        if (!root.mfx) return -1
        const padded = n < 10 ? ("0" + n) : ("" + n)
        let i = root.mfx.paramIndexByName("Step " + padded)
        if (i >= 0) return i
        return root.mfx.paramIndexByName("Step " + n)
    }

    Label {
        text: (root.mfx ? root.mfx.typeName : "Slicer") + "  ·  steps"
        color: LogicTheme.textPrimary
        font.bold: true
        font.pixelSize: LogicTheme.fontSize
    }

    GridLayout {
        columns: 8
        columnSpacing: 4
        rowSpacing: 6
        Layout.fillWidth: true
        enabled: root.editorEnabled

        Repeater {
            model: 16
            ColumnLayout {
                required property int index
                spacing: 2
                Layout.fillWidth: true

                Label {
                    text: (index + 1)
                    color: LogicTheme.textMuted
                    font.pixelSize: LogicTheme.fontSizeSmall
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    radius: 4
                    color: LogicTheme.dark ? "#1A1A1C" : "#E8E8ED"
                    border.color: LogicTheme.hairline

                    readonly property int pIdx: root.stepIdx(index + 1)
                    readonly property int lo: pIdx >= 0 && root.mfx ? root.mfx.paramMin(pIdx) : 0
                    readonly property int hi: pIdx >= 0 && root.mfx ? root.mfx.paramMax(pIdx) : 127
                    readonly property int val: {
                        if (pIdx < 0 || !root.mfx) return 0
                        const v = Number(root.paramValues[pIdx])
                        if (v < lo || v > hi) return lo
                        return v
                    }
                    readonly property real norm: {
                        if (hi <= lo) return 0.5
                        return (val - lo) / (hi - lo)
                    }

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.margins: 3
                        height: Math.max(4, (parent.height - 6) * parent.norm)
                        radius: 2
                        color: LogicTheme.accent
                        opacity: 0.85
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: parent.pIdx >= 0
                        preventStealing: true
                        onPositionChanged: (mouse) => {
                            if (!pressed || !root.mfx) return
                            const n = 1 - Math.max(0, Math.min(1, mouse.y / height))
                            const v = Math.round(parent.lo + n * (parent.hi - parent.lo))
                            root.mfx.setParamValue(parent.pIdx, v)
                        }
                        onClicked: (mouse) => {
                            if (!root.mfx) return
                            const n = 1 - Math.max(0, Math.min(1, mouse.y / height))
                            const v = Math.round(parent.lo + n * (parent.hi - parent.lo))
                            root.mfx.setParamValue(parent.pIdx, v)
                        }
                    }
                }
            }
        }
    }

    MfxParamGrid {
        Layout.fillWidth: true
        mfxModel: root.mfx
        enabled: root.editorEnabled
        hideNamePrefix: "Step "
    }
}
