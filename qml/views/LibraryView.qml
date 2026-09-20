import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    id: root
    color: LogicTheme.windowBg
    clip: true
    // When embedded in equal thirds, do not let action-row content inflate preferred width.
    implicitWidth: embedded ? 0 : implicitContentWidth

    /// Compact chrome when embedded beside Parts / Tones on Sets & Tones.
    property bool embedded: false
    readonly property real implicitContentWidth: contentColumn.implicitWidth
        + (embedded ? 16 : 24)
    readonly property bool hasProject: App.library.currentPath.length > 0

    property int _pendingLoadRow: -1
    property int _pendingDeleteRow: -1
    property string _pendingDeleteName: ""

    function proposedNewName() {
        const base = App.studioSet.name && App.studioSet.name.length
                     ? App.studioSet.name
                     : "Untitled"
        return base
    }

    function openNameDialog(title, initialName, onAccept) {
        nameDialog.title = title
        nameField.text = initialName
        nameDialog._onAccept = onAccept
        nameDialog.open()
        nameField.forceActiveFocus()
        nameField.selectAll()
    }

    function requestLoad(row) {
        if (row < 0)
            return
        _pendingLoadRow = row
        if (App.studioSet.dirty) {
            unsavedDialog.open()
            return
        }
        finishLoad()
    }

    function finishLoad() {
        const row = _pendingLoadRow
        _pendingLoadRow = -1
        if (row < 0)
            return
        if (App.loadLibrary(row)) {
            renameError.text = ""
            pushAfterLoadDialog.open()
        } else {
            renameError.text = App.library.lastError.length
                               ? App.library.lastError
                               : "Could not load library file."
        }
    }

    function cancelPendingLoad() {
        _pendingLoadRow = -1
    }

    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        anchors.margins: root.embedded ? 8 : 12
        spacing: 8
        width: parent.width

        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width
            spacing: 6
            Label {
                text: root.embedded ? "Library" : "Studio Set Library"
                color: LogicTheme.textPrimary
                font.pixelSize: LogicTheme.fontSizeTitle
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
            FaButton {
                glyph: FaIcons.add
                text: root.embedded ? "" : "New"
                onClicked: openNameDialog("New Library File", root.proposedNewName(), function(n) {
                    App.library.saveAs(n)
                })
            }
            FaButton {
                visible: root.hasProject
                glyph: FaIcons.save
                text: root.embedded ? "" : "Save"
                onClicked: App.saveToLibrary()
            }
            FaButton {
                visible: root.hasProject
                glyph: FaIcons.copy
                text: root.embedded ? "" : "Dup"
                onClicked: App.library.duplicateCurrent()
            }
            FaButton {
                glyph: FaIcons.refresh
                text: root.embedded ? "" : "Refresh"
                onClicked: App.library.refresh()
            }
        }

        Label {
            Layout.fillWidth: true
            elide: Text.ElideMiddle
            text: {
                if (!root.hasProject)
                    return "No project file" + (App.studioSet.dirty ? " · unsaved edits" : "")
                return "Current: " + App.library.currentName
                       + (App.studioSet.dirty ? " · unsaved" : "")
            }
            color: App.studioSet.dirty ? LogicTheme.warning : LogicTheme.textSecondary
            font.pixelSize: LogicTheme.fontSizeSmall
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: App.library
            spacing: root.embedded ? 1 : 2
            delegate: Rectangle {
                id: rowRoot
                required property int index
                required property string name
                required property string modified
                required property string path

                readonly property bool isCurrent: root.hasProject && path === App.library.currentPath

                width: ListView.view.width
                height: LogicTheme.mobile ? 48 : (root.embedded ? 32 : 40)
                color: isCurrent ? LogicTheme.selectedBg : LogicTheme.panelBg
                radius: root.embedded ? 3 : 4
                border.width: isCurrent ? 1 : 0
                border.color: LogicTheme.accent

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: root.embedded ? 6 : 8
                    anchors.rightMargin: root.embedded ? 4 : 8
                    anchors.topMargin: root.embedded ? 4 : 8
                    anchors.bottomMargin: root.embedded ? 4 : 8
                    spacing: root.embedded ? 4 : 8
                    clip: true

                    FaIcon {
                        icon: FaIcons.folder
                        size: LogicTheme.fontSize
                        iconColor: LogicTheme.textSecondary
                        Layout.preferredWidth: 16
                    }
                    Label {
                        text: name
                        color: LogicTheme.textPrimary
                        font.pixelSize: root.embedded ? LogicTheme.fontSizeSmall : LogicTheme.fontSize
                        font.bold: rowRoot.isCurrent
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        elide: Text.ElideRight
                    }
                    Label {
                        visible: !root.embedded
                        text: modified
                        color: LogicTheme.textMuted
                        font.pixelSize: LogicTheme.fontSizeSmall
                        Layout.preferredWidth: implicitWidth
                        elide: Text.ElideRight
                    }
                    FaButton {
                        glyph: FaIcons.download
                        text: root.embedded ? "" : "Load"
                        onClicked: root.requestLoad(index)
                    }
                    FaButton {
                        glyph: FaIcons.edit
                        text: root.embedded ? "" : "Rename"
                        onClicked: {
                            const row = index
                            openNameDialog("Rename", name, function(n) {
                                if (!App.library.rename(row, n))
                                    renameError.text = "Could not rename (name may already exist)."
                                else
                                    renameError.text = ""
                            })
                        }
                    }
                    FaButton {
                        glyph: FaIcons.trash
                        text: root.embedded ? "" : "Delete"
                        glyphColor: LogicTheme.danger
                        onClicked: {
                            root._pendingDeleteRow = index
                            root._pendingDeleteName = name
                            deleteConfirmDialog.open()
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    z: -1
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (rowRoot.isCurrent)
                            return
                        root.requestLoad(index)
                    }
                }
            }
        }

        Label {
            id: renameError
            color: LogicTheme.danger
            font.pixelSize: LogicTheme.fontSizeSmall
            Layout.fillWidth: true
            visible: text.length > 0
        }
    }

    Dialog {
        id: nameDialog
        property var _onAccept: null
        parent: Overlay.overlay
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: Math.min(360, Overlay.overlay ? Overlay.overlay.width - 24 : 360)

        ColumnLayout {
            anchors.fill: parent
            spacing: 8
            Label {
                text: "Name"
                color: LogicTheme.textSecondary
            }
            TextField {
                id: nameField
                Layout.fillWidth: true
                selectByMouse: true
                onAccepted: nameDialog.accept()
            }
        }

        onAccepted: {
            const n = nameField.text.trim()
            if (n.length && typeof _onAccept === "function")
                _onAccept(n)
        }
    }

    Dialog {
        id: unsavedDialog
        parent: Overlay.overlay
        modal: true
        anchors.centerIn: parent
        title: "Unsaved changes"
        width: Math.min(420, Overlay.overlay ? Overlay.overlay.width - 24 : 420)
        standardButtons: Dialog.NoButton

        ColumnLayout {
            anchors.fill: parent
            spacing: 14

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: LogicTheme.textSecondary
                text: root.hasProject
                      ? "You have unsaved changes to “" + App.library.currentName + "”. Save before loading another project?"
                      : "You have unsaved edits in the editor. Discard them and load this library project?"
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 8

                FaButton {
                    text: "Cancel"
                    onClicked: {
                        root.cancelPendingLoad()
                        unsavedDialog.close()
                    }
                }
                FaButton {
                    text: "Discard"
                    onClicked: {
                        unsavedDialog.close()
                        root.finishLoad()
                    }
                }
                FaButton {
                    visible: root.hasProject
                    text: "Save"
                    onClicked: {
                        if (App.saveToLibrary()) {
                            unsavedDialog.close()
                            root.finishLoad()
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: deleteConfirmDialog
        parent: Overlay.overlay
        modal: true
        anchors.centerIn: parent
        title: "Delete library file?"
        standardButtons: Dialog.Yes | Dialog.No
        width: Math.min(400, Overlay.overlay ? Overlay.overlay.width - 24 : 400)

        contentItem: Label {
            wrapMode: Text.WordWrap
            color: LogicTheme.textSecondary
            text: "Delete “" + root._pendingDeleteName + "”? This cannot be undone."
        }

        onAccepted: {
            if (root._pendingDeleteRow >= 0)
                App.library.remove(root._pendingDeleteRow)
            root._pendingDeleteRow = -1
            root._pendingDeleteName = ""
        }
        onRejected: {
            root._pendingDeleteRow = -1
            root._pendingDeleteName = ""
        }
    }

    Dialog {
        id: pushAfterLoadDialog
        parent: Overlay.overlay
        modal: true
        anchors.centerIn: parent
        title: "Push to FA?"
        standardButtons: Dialog.Yes | Dialog.No
        width: Math.min(400, Overlay.overlay ? Overlay.overlay.width - 24 : 400)

        contentItem: Label {
            wrapMode: Text.WordWrap
            color: LogicTheme.textSecondary
            text: "Library project “" + App.library.currentName + "” is loaded in the editor. Push Temporary Studio Set, tone blobs, Audio FX, and Master EQ to the FA now?"
        }

        onAccepted: App.push()
    }
}
