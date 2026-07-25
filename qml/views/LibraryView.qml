import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    color: LogicTheme.windowBg

    function openNameDialog(title, initialName, onAccept) {
        nameDialog.title = title
        nameField.text = initialName
        nameDialog._onAccept = onAccept
        nameDialog.open()
        nameField.forceActiveFocus()
        nameField.selectAll()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Studio Set Library"
                color: LogicTheme.textPrimary
                font.pixelSize: LogicTheme.fontSizeTitle; font.bold: true
                Layout.fillWidth: true
            }
            FaButton {
                glyph: FaIcons.save
                text: "Save"
                onClicked: App.saveToLibrary()
            }
            FaButton {
                glyph: FaIcons.copy
                text: "Save As…"
                onClicked: openNameDialog("Save As", App.studioSet.name + " Copy", function(n) {
                    App.library.saveAs(n)
                })
            }
            FaButton {
                glyph: FaIcons.refresh
                text: "Refresh"
                onClicked: App.library.refresh()
            }
        }

        Label {
            text: App.library.currentPath.length ? ("Current: " + App.library.currentName) : "No project file"
            color: LogicTheme.textSecondary
            font.pixelSize: LogicTheme.fontSizeSmall
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: App.library
            spacing: 2
            delegate: Rectangle {
                required property int index
                required property string name
                required property string modified
                required property string path

                width: ListView.view.width
                height: 40
                color: LogicTheme.panelBg
                radius: 4

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    FaIcon {
                        icon: FaIcons.folder
                        size: LogicTheme.fontSize
                        iconColor: LogicTheme.textSecondary
                    }
                    Label {
                        text: name
                        color: LogicTheme.textPrimary
                        font.pixelSize: LogicTheme.fontSize
                        Layout.fillWidth: true
                    }
                    Label {
                        text: modified
                        color: LogicTheme.textMuted
                        font.pixelSize: LogicTheme.fontSizeSmall
                    }
                    FaButton {
                        glyph: FaIcons.download
                        text: "Load"
                        onClicked: App.library.load(index)
                    }
                    FaButton {
                        glyph: FaIcons.edit
                        text: "Rename"
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
                        text: "Dup"
                        onClicked: App.library.duplicate(index)
                    }
                    FaButton {
                        glyph: FaIcons.trash
                        text: "Delete"
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
