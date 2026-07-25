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

    function openNameDialog(title, initialName, onAccept) {
        nameDialog.title = title
        nameField.text = initialName
        nameDialog._onAccept = onAccept
        nameDialog.open()
        nameField.forceActiveFocus()
        nameField.selectAll()
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
                glyph: FaIcons.save
                text: root.embedded ? "" : "Save"
                onClicked: App.saveToLibrary()
            }
            FaButton {
                glyph: FaIcons.copy
                text: root.embedded ? "" : "Save As…"
                onClicked: openNameDialog("Save As", App.studioSet.name + " Copy", function(n) {
                    App.library.saveAs(n)
                })
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
            text: App.library.currentPath.length ? ("Current: " + App.library.currentName) : "No project file"
            color: LogicTheme.textSecondary
            font.pixelSize: LogicTheme.fontSizeSmall
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: App.library
            spacing: root.embedded ? 1 : 2
            delegate: Rectangle {
                required property int index
                required property string name
                required property string modified
                required property string path

                width: ListView.view.width
                height: root.embedded ? 32 : 40
                color: LogicTheme.panelBg
                radius: root.embedded ? 3 : 4

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
                        onClicked: App.library.load(index)
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
                        glyph: FaIcons.copy
                        text: root.embedded ? "" : "Dup"
                        onClicked: App.library.duplicate(index)
                    }
                    FaButton {
                        glyph: FaIcons.trash
                        text: root.embedded ? "" : "Delete"
                        glyphColor: LogicTheme.danger
                        onClicked: App.library.remove(index)
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
        width: 360

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
}
