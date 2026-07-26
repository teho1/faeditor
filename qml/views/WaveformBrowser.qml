import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    id: root
    color: LogicTheme.windowBg
    clip: true

    /// Currently assigned wave number (highlight + scroll target).
    property int selectedNumber: -1
    property bool showClose: true
    signal waveChosen(int number)
    signal closeRequested()

    readonly property int selectedRow: App.waveforms
        ? App.waveforms.indexOfWave(selectedNumber)
        : -1

    function scrollToSelected() {
        if (selectedRow < 0)
            return
        waveList.currentIndex = selectedRow
        waveList.positionViewAtIndex(selectedRow, ListView.Contain)
    }

    onSelectedNumberChanged: Qt.callLater(scrollToSelected)
    onSelectedRowChanged: Qt.callLater(scrollToSelected)
    onVisibleChanged: if (visible) Qt.callLater(scrollToSelected)
    onEnabledChanged: if (enabled) Qt.callLater(scrollToSelected)

    Connections {
        target: App.waveforms
        function onFilterChanged() { Qt.callLater(root.scrollToSelected) }
        function onCatalogChanged() { Qt.callLater(root.scrollToSelected) }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Waveforms"
                color: LogicTheme.textPrimary
                font.pixelSize: LogicTheme.fontSizeTitle
                font.bold: true
                Layout.fillWidth: true
            }
            FaIcon {
                visible: root.showClose
                icon: FaIcons.chevronUp
                size: LogicTheme.fontSize + 2
                iconColor: LogicTheme.textMuted
                Layout.preferredWidth: 20
                MouseArea {
                    anchors.fill: parent
                    anchors.margins: -6
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.closeRequested()
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            radius: 4
            color: LogicTheme.selectedBg
            border.width: 1
            border.color: LogicTheme.accent
            visible: root.selectedNumber >= 0

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                spacing: 6

                Label {
                    text: "Current"
                    color: LogicTheme.textSecondary
                    font.pixelSize: LogicTheme.fontSizeSmall
                }
                Label {
                    text: "#" + root.selectedNumber
                    color: LogicTheme.textSecondary
                    font.pixelSize: LogicTheme.fontSizeSmall
                    Layout.preferredWidth: 44
                }
                Label {
                    text: {
                        if (!App.waveforms)
                            return ""
                        if (App.waveforms.bank === "sn")
                            return App.waveforms.displaySn(root.selectedNumber)
                        if (App.waveforms.bank === "intA")
                            return App.waveforms.displayPcm(0, 1, root.selectedNumber)
                        if (App.waveforms.bank === "intB")
                            return App.waveforms.displayPcm(0, 2, root.selectedNumber)
                        return "#" + root.selectedNumber
                    }
                    color: LogicTheme.textPrimary
                    font.pixelSize: LogicTheme.fontSizeSmall
                    font.bold: true
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
            }
        }

        Label {
            text: App.waveforms ? App.waveforms.bankDisplayName : ""
            color: LogicTheme.accent
            font.pixelSize: LogicTheme.fontSizeSmall
            font.bold: true
        }

        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: "Search name or #…"
            font.pixelSize: LogicTheme.fontSize
            onTextChanged: if (App.waveforms) App.waveforms.filterText = text
        }

        Label {
            visible: App.waveforms && !App.waveforms.bankHasNames
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: "No named waves for this bank (SRX). Use the wave number spinner."
            color: LogicTheme.textSecondary
            font.pixelSize: LogicTheme.fontSizeSmall
        }

        ListView {
            id: waveList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            visible: App.waveforms && App.waveforms.bankHasNames
            model: App.waveforms
            spacing: 0
            currentIndex: root.selectedRow
            highlightFollowsCurrentItem: false

            delegate: Rectangle {
                id: waveRow
                required property int index
                required property int number
                required property string name

                readonly property bool assigned: number === root.selectedNumber

                width: ListView.view.width
                height: 28
                radius: 3
                color: {
                    if (assigned)
                        return LogicTheme.selectedBg
                    return index % 2 ? LogicTheme.panelBgRaised : "transparent"
                }
                border.width: assigned ? 1 : 0
                border.color: LogicTheme.accent

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.waveChosen(number)
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 8

                    Label {
                        text: number === 0 ? "OFF" : String(number)
                        color: LogicTheme.textSecondary
                        font.pixelSize: LogicTheme.fontSizeSmall
                        font.bold: waveRow.assigned
                        Layout.preferredWidth: 44
                    }
                    Label {
                        text: name
                        color: LogicTheme.textPrimary
                        font.pixelSize: LogicTheme.fontSizeSmall
                        font.bold: waveRow.assigned
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                }
            }
        }

        Label {
            text: {
                if (!App.waveforms || !App.waveforms.bankHasNames)
                    return ""
                const n = App.waveforms.filteredCount
                return n + (n === 1 ? " wave" : " waves")
            }
            color: LogicTheme.textMuted
            font.pixelSize: LogicTheme.fontSizeSmall
        }
    }
}
