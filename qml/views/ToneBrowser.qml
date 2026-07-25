import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    color: LogicTheme.windowBg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            TextField {
                Layout.fillWidth: true
                placeholderText: "Search tones…"
                font.pixelSize: LogicTheme.fontSize
                onTextChanged: App.tones.filterText = text
            }
            ComboBox {
                model: App.tones.categories
                currentIndex: Math.max(0, model.indexOf(App.tones.category))
                onActivated: App.tones.category = currentText
                Layout.preferredWidth: 160
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: App.tones
            spacing: 1
            delegate: Rectangle {
                required property int index
                required property string name
                required property string category
                required property int bankMsb
                required property int bankLsb
                required property int program
                required property bool favorite

                width: ListView.view.width
                height: 32
                color: index % 2 ? LogicTheme.panelBg : LogicTheme.windowBg

                MouseArea {
                    anchors.fill: parent
                    z: -1
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onDoubleClicked: App.applyToneToSelectedPart(index)
                    onClicked: (mouse) => {
                        if (mouse.button === Qt.RightButton)
                            App.previewTone(index)
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 8

                    FaIcon {
                        icon: favorite ? FaIcons.star : FaIcons.starEmpty
                        size: LogicTheme.fontSize + 1
                        iconColor: favorite ? LogicTheme.warning : LogicTheme.textMuted
                        opacity: favorite ? 1 : 0.35
                        Layout.preferredWidth: 22
                        MouseArea {
                            anchors.fill: parent
                            anchors.margins: -6
                            cursorShape: Qt.PointingHandCursor
                            onClicked: App.tones.toggleFavorite(index)
                        }
                    }
                    FaIcon {
                        icon: FaIcons.forCategory(category)
                        size: LogicTheme.fontSize
                        iconColor: LogicTheme.textSecondary
                        Layout.preferredWidth: 18
                    }
                    Label {
                        text: name
                        color: LogicTheme.textPrimary
                        font.pixelSize: LogicTheme.fontSize
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                    Label {
                        text: category
                        color: LogicTheme.textSecondary
                        font.pixelSize: LogicTheme.fontSizeSmall
                        Layout.preferredWidth: 90
                        elide: Text.ElideRight
                    }
                    Label {
                        text: bankMsb + ":" + bankLsb + ":" + (program + 1)
                        color: LogicTheme.textMuted
                        font.pixelSize: LogicTheme.fontSizeSmall
                        Layout.preferredWidth: 90
                    }
                }
            }
        }

        Label {
            text: "Star to favorite · Double-click to assign · Right-click to preview · Filter: Favorites"
            color: LogicTheme.textMuted
            font.pixelSize: LogicTheme.fontSizeSmall
        }
    }
}
