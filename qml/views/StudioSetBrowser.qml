import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    id: root
    color: LogicTheme.windowBg

    // When true: single toolbar row for Sets & Tones (no tall slot list).
    property bool compact: false

    implicitHeight: root.compact
                    ? (compactBar.height + (App.studioSets.scanning ? compactProgress.implicitHeight + 8 : 0))
                    : 400

    // ── Compact: toolbar picker ─────────────────────────────────────────────
    ColumnLayout {
        id: compactRoot
        width: parent.width
        spacing: 0
        visible: root.compact

        Rectangle {
            id: compactBar
            Layout.fillWidth: true
            height: barRow.implicitHeight + 12
            color: LogicTheme.panelBg

            RowLayout {
                id: barRow
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 8

                Label {
                    text: "FA Set"
                    color: LogicTheme.textPrimary
                    font.pixelSize: LogicTheme.fontSize
                    font.bold: true
                }

                ComboBox {
                    id: groupCombo
                    model: ["All", "User", "Preset"]
                    currentIndex: Math.max(0, model.indexOf(App.studioSets.groupFilter))
                    onActivated: App.studioSets.groupFilter = currentText
                    Layout.preferredWidth: 84
                    font.pixelSize: LogicTheme.fontSizeSmall
                }

                ComboBox {
                    id: setCombo
                    Layout.fillWidth: true
                    Layout.minimumWidth: 160
                    Layout.preferredWidth: 280
                    model: App.studioSets
                    currentIndex: App.studioSets.currentRow
                    font.pixelSize: LogicTheme.fontSizeSmall
                    displayText: currentIndex < 0
                                 ? "Select User/Preset slot…"
                                 : App.studioSets.currentLabel

                    Connections {
                        target: App.studioSets
                        function onCurrentChanged() {
                            setCombo.currentIndex = App.studioSets.currentRow
                        }
                        function onFilterChanged() {
                            setCombo.currentIndex = App.studioSets.currentRow
                        }
                    }

                    // When the list opens, refresh this slot’s name from Temporary (device edits).
                    Connections {
                        target: setCombo.popup
                        function onOpened() {
                            App.studioSets.syncCurrentNameFromDevice()
                        }
                    }

                    onActivated: function (index) {
                        App.openStudioSet(index)
                    }

                    delegate: ItemDelegate {
                        required property int index
                        required property string label
                        required property string name
                        required property bool hasName

                        width: setCombo.width
                        highlighted: setCombo.highlightedIndex === index
                        font.pixelSize: LogicTheme.fontSizeSmall

                        contentItem: RowLayout {
                            spacing: 8
                            Label {
                                text: label
                                color: LogicTheme.textSecondary
                                font.pixelSize: LogicTheme.fontSizeSmall
                                Layout.preferredWidth: 72
                                elide: Text.ElideRight
                            }
                            Label {
                                text: name
                                color: hasName ? LogicTheme.textPrimary : LogicTheme.textMuted
                                font.pixelSize: LogicTheme.fontSizeSmall
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                        }
                    }
                }

                TextField {
                    Layout.preferredWidth: 110
                    placeholderText: "Filter…"
                    font.pixelSize: LogicTheme.fontSizeSmall
                    onTextChanged: App.studioSets.filterText = text
                }

                FaButton {
                    glyph: ""
                    text: App.studioSets.scanning ? "Cancel" : "Scan"
                    onClicked: {
                        if (App.studioSets.scanning)
                            App.studioSets.cancelScan()
                        else
                            scanConfirm.openFor(true)
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: LogicTheme.hairline
            }
        }

        ProgressBar {
            id: compactProgress
            Layout.fillWidth: true
            visible: App.studioSets.scanning
            from: 0
            to: Math.max(1, App.studioSets.scanTotal)
            value: App.studioSets.scanProgress
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            Layout.topMargin: 4
            Layout.bottomMargin: 4
        }
    }

    // ── Full browser (standalone / non-compact) ─────────────────────────────
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10
        visible: !root.compact

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
            font.pixelSize: LogicTheme.fontSizeSmall
            text: "Pick a User or Preset Studio Set to load it into the FA’s Temporary memory, then change instruments on its parts. Optional: Scan Names reads titles from the keyboard (takes a few minutes for all User slots)."
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            TextField {
                Layout.fillWidth: true
                placeholderText: "Filter by slot or name…"
                font.pixelSize: LogicTheme.fontSizeSmall
                onTextChanged: App.studioSets.filterText = text
            }

            ComboBox {
                model: ["All", "User", "Preset"]
                currentIndex: Math.max(0, model.indexOf(App.studioSets.groupFilter))
                onActivated: App.studioSets.groupFilter = currentText
                Layout.preferredWidth: 120
                font.pixelSize: LogicTheme.fontSizeSmall
            }

            Button {
                text: App.studioSets.scanning ? "Cancel Scan" : "Scan User Names"
                onClicked: {
                    if (App.studioSets.scanning)
                        App.studioSets.cancelScan()
                    else
                        scanConfirm.openFor(true)
                }
            }

            Button {
                text: "Scan All Names"
                enabled: !App.studioSets.scanning
                onClicked: scanConfirm.openFor(false)
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
                            elide: Text.ElideRight
                        }
                        Label {
                            text: name
                            color: hasName ? LogicTheme.textPrimary : LogicTheme.textMuted
                            font.pixelSize: LogicTheme.fontSizeSmall
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                        Label {
                            text: kind
                            color: LogicTheme.textMuted
                            font.pixelSize: LogicTheme.fontSizeSmall
                            Layout.preferredWidth: 50
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

    Dialog {
        id: scanConfirm
        property bool userOnly: true
        parent: Overlay.overlay
        modal: true
        anchors.centerIn: parent
        title: "Scan Studio Set Names"
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: 420

        function openFor(usersOnly) {
            userOnly = usersOnly
            open()
        }

        Label {
            width: parent ? parent.width : 380
            wrapMode: Text.WordWrap
            color: LogicTheme.textSecondary
            text: scanConfirm.userOnly
                  ? "Reading User Studio Set names from the FA can take several minutes (hundreds of slots). Names are saved locally so you only need to do this once; running Scan again overwrites the saved list."
                  : "Reading all User and Preset Studio Set names from the FA can take a long time. Names are saved locally so you only need to do this once; running Scan again overwrites the saved list."
        }

        onAccepted: App.studioSets.startScanNames(scanConfirm.userOnly)
    }
}
