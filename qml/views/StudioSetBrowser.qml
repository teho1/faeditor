import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    color: LogicTheme.windowBg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label {
            text: "Studio Sets on the FA"
            color: LogicTheme.textPrimary
            font.pixelSize: LogicTheme.fontSizeTitle
            font.bold: true
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: LogicTheme.textSecondary
            font.pixelSize: LogicTheme.fontSize
            text: "Pick a User or Preset Studio Set to load it into the FA’s Temporary memory, then change instruments on its parts. Optional: Scan Names reads titles from the keyboard (takes a few minutes for all User slots)."
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                Layout.fillWidth: true
                placeholderText: "Filter by slot or name…"
                font.pixelSize: LogicTheme.fontSize
                onTextChanged: App.studioSets.filterText = text
            }

            ComboBox {
                model: ["All", "User", "Preset"]
                currentIndex: Math.max(0, model.indexOf(App.studioSets.groupFilter))
                onActivated: App.studioSets.groupFilter = currentText
                Layout.preferredWidth: 120
            }

            Button {
                text: App.studioSets.scanning ? "Cancel Scan" : "Scan User Names"
                onClicked: {
                    if (App.studioSets.scanning)
                        App.studioSets.cancelScan()
                    else
                        App.studioSets.startScanNames(true)
                }
            }

            Button {
                text: "Scan All Names"
                enabled: !App.studioSets.scanning
                onClicked: App.studioSets.startScanNames(false)
            }
        }

        ProgressBar {
            Layout.fillWidth: true
            visible: App.studioSets.scanning
            from: 0
            to: Math.max(1, App.studioSets.scanTotal)
            value: App.studioSets.scanProgress
        }

        Label {
            text: App.studioSets.statusText
            color: LogicTheme.accent
            font.pixelSize: LogicTheme.fontSizeSmall
            visible: App.studioSets.statusText.length > 0
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: LogicTheme.panelBg
            radius: 4
            border.color: LogicTheme.hairline

            ListView {
                id: list
                anchors.fill: parent
                anchors.margins: 1
                clip: true
                model: App.studioSets
                currentIndex: App.studioSets.currentRow
                spacing: 1

                delegate: Rectangle {
                    required property int index
                    required property string label
                    required property string name
                    required property string kind
                    required property int number
                    required property bool hasName

                    width: ListView.view.width
                    height: 36
                    color: App.studioSets.currentRow === index ? LogicTheme.selectedBg
                          : (index % 2 ? LogicTheme.panelBg : LogicTheme.windowBg)

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 12

                        Label {
                            text: label
                            color: LogicTheme.textSecondary
                            font.pixelSize: LogicTheme.fontSizeSmall
                            Layout.preferredWidth: 90
                        }
                        Label {
                            text: name
                            color: hasName ? LogicTheme.textPrimary : LogicTheme.textMuted
                            font.pixelSize: LogicTheme.fontSize
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                        Label {
                            text: kind
                            color: LogicTheme.textMuted
                            font.pixelSize: LogicTheme.fontSizeSmall
                            Layout.preferredWidth: 50
                        }
                        Button {
                            text: "Open"
                            onClicked: App.openStudioSet(index)
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        z: -1
                        onClicked: App.studioSets.currentRow = index
                        onDoubleClicked: App.openStudioSet(index)
                    }
                }
            }
        }

        Label {
            text: "Current: " + App.studioSets.currentLabel + "  ·  Loaded Temporary: “" + App.studioSet.name + "”"
            color: LogicTheme.textSecondary
            font.pixelSize: LogicTheme.fontSizeSmall
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
    }
}
