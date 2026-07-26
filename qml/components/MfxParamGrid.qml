import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

/**
 * Catalog-driven named parameter row/flow.
 * Binds to mfx.paramList (C++ QAbstractListModel) — not QVariantList indexing.
 * Continuous/int params → vertical MfxParamFader; named enums → ComboBox.
 */
Item {
    id: root

    // Do not redeclare Item.enabled — that triggers "overrides a member of the base object".
    required property var mfxModel
    property real faderHeight: 72
    property real faderWidth: 28
    property real cellWidth: 48
    property real enumWidth: 108
    /** Hide params whose name starts with this prefix (e.g. "Step " for slicer). */
    property string hideNamePrefix: ""
    /**
     * When non-empty (with showExactNames), only show matching params.
     * A param matches if its name equals a prefix or starts with "prefix ".
     */
    property var showNamePrefixes: []
    /** Exact param names to show when stage filtering is active. */
    property var showExactNames: []
    /** Exact names always shown alongside a stage filter (e.g. Level). */
    property var alwaysShowNames: []
    /** Optional (name) => display string. */
    property var formatLabel: null

    // Legacy API (ignored layout-wise; kept so call sites don't break).
    property int columns: 4
    property real cellMinWidth: 110

    readonly property bool stageFilterActive: {
        const p = root.showNamePrefixes
        const e = root.showExactNames
        return (p && p.length > 0) || (e && e.length > 0)
    }

    implicitHeight: flow.implicitHeight
    implicitWidth: 320

    function displayName(name) {
        if (root.formatLabel)
            return root.formatLabel(name)
        return name
    }

    function nameMatchesPrefix(name, prefix) {
        if (!prefix || !prefix.length)
            return false
        return name === prefix || name.indexOf(prefix + " ") === 0
    }

    function listHasName(list, name) {
        if (!list || !list.length)
            return false
        for (let i = 0; i < list.length; ++i) {
            if (list[i] === name)
                return true
        }
        return false
    }

    function paramShown(name) {
        if (root.hideNamePrefix.length > 0 && name.indexOf(root.hideNamePrefix) === 0)
            return false
        if (!root.stageFilterActive)
            return true
        if (root.listHasName(root.alwaysShowNames, name))
            return true
        if (root.listHasName(root.showExactNames, name))
            return true
        const prefixes = root.showNamePrefixes || []
        for (let i = 0; i < prefixes.length; ++i) {
            if (root.nameMatchesPrefix(name, prefixes[i]))
                return true
        }
        return false
    }

    Flow {
        id: flow
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 10
        enabled: root.enabled && root.mfxModel

        Repeater {
            model: root.mfxModel ? root.mfxModel.paramList : null

            Item {
                id: cell
                required property int index
                required property string name
                required property int min
                required property int max
                required property int paramValue
                required property int paramIndex
                required property bool hasEnum
                required property var enumNames

                readonly property bool isEnum: cell.hasEnum && cell.enumNames && cell.enumNames.length > 0
                readonly property bool shown: root.paramShown(cell.name)

                visible: cell.shown
                width: cell.shown ? (cell.isEnum ? root.enumWidth : root.cellWidth) : 0
                height: cell.shown ? col.implicitHeight : 0

                ColumnLayout {
                    id: col
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 2

                    MfxParamFader {
                        visible: !cell.isEnum
                        Layout.alignment: Qt.AlignHCenter
                        label: root.displayName(cell.name)
                        value: cell.paramValue
                        from: cell.min
                        to: cell.max
                        faderHeight: root.faderHeight
                        faderWidth: root.faderWidth
                        onMoved: (v) => {
                            if (root.mfxModel)
                                root.mfxModel.paramList.setValue(cell.paramIndex, v)
                        }
                    }

                    ComboBox {
                        visible: cell.isEnum
                        Layout.fillWidth: true
                        Layout.preferredWidth: root.enumWidth
                        model: cell.enumNames || []
                        currentIndex: {
                            if (!cell.isEnum)
                                return -1
                            return Math.max(0, Math.min(cell.enumNames.length - 1, cell.paramValue - cell.min))
                        }
                        onActivated: (i) => {
                            if (root.mfxModel)
                                root.mfxModel.paramList.setValue(cell.paramIndex, cell.min + i)
                        }
                    }

                    Label {
                        visible: cell.isEnum
                        text: root.displayName(cell.name)
                        color: LogicTheme.textMuted
                        font.pixelSize: LogicTheme.fontSizeSmall
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
