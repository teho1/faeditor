import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Rectangle {
    id: root
    // Compact (Mixer inspector) shares the panel surface; full / narrow use window bg.
    color: root.compact ? "transparent" : LogicTheme.windowBg
    clip: true
    // Narrow (equal-column) mode must not inflate parent preferred width.
    implicitWidth: narrow ? 0 : implicitContentWidth

    /// Mixer side panel: close chrome, inspector-matched rows.
    property bool compact: false
    /// Sets & Tones column: tighter list, no category column / footer tip.
    property bool narrow: false
    signal closeRequested()

    readonly property bool _tight: root.compact || root.narrow
    readonly property real implicitContentWidth: toneColumn.implicitWidth
        + (compact ? 16 : (narrow ? 0 : 24))

    readonly property var part: App.studioSet.selectedPartModel
    readonly property string assignedName: part ? part.toneName : ""
    readonly property int assignedMsb: part ? part.bankMsb : -1
    readonly property int assignedLsb: part ? part.bankLsb : -1
    readonly property int assignedProgram: part ? part.program : -1
    readonly property int assignedRow: (part && App.tones)
        ? App.tones.indexOfTone(assignedMsb, assignedLsb, assignedProgram)
        : -1
    /// Category to restore when turning off the favorites star filter.
    property string categoryBeforeFavorites: "All"

    readonly property bool favoritesFilterOn: App.tones.category === "Favorites"

    function toggleFavoritesFilter() {
        if (favoritesFilterOn) {
            App.tones.category = categoryBeforeFavorites.length ? categoryBeforeFavorites : "All"
        } else {
            if (App.tones.category !== "Favorites")
                categoryBeforeFavorites = App.tones.category
            App.tones.category = "Favorites"
        }
    }

    function isAssignedTone(msb, lsb, pc) {
        return part && msb === assignedMsb && lsb === assignedLsb && pc === assignedProgram
    }

    function scrollToAssigned() {
        if (assignedRow < 0)
            return
        toneList.currentIndex = assignedRow
        toneList.positionViewAtIndex(assignedRow, ListView.Contain)
    }

    onAssignedRowChanged: Qt.callLater(scrollToAssigned)
    onAssignedMsbChanged: Qt.callLater(scrollToAssigned)
    onAssignedLsbChanged: Qt.callLater(scrollToAssigned)
    onAssignedProgramChanged: Qt.callLater(scrollToAssigned)
    onEnabledChanged: if (enabled) Qt.callLater(scrollToAssigned)
    onVisibleChanged: if (visible) Qt.callLater(scrollToAssigned)

    Connections {
        target: App.tones
        function onFilterChanged() { Qt.callLater(root.scrollToAssigned) }
    }

    ColumnLayout {
        id: toneColumn
        anchors.fill: parent
        // When nested under an outer “Tones” header (narrow), skip extra margins.
        anchors.margins: root.compact ? 8 : (root.narrow ? 0 : 12)
        spacing: 8
        width: parent.width

        RowLayout {
            Layout.fillWidth: true
            visible: root.compact
            Label {
                text: "Select Tone"
                color: LogicTheme.textPrimary
                font.pixelSize: LogicTheme.fontSizeTitle
                font.bold: true
                Layout.fillWidth: true
            }
            FaIcon {
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

        // Always-visible current assignment (Mixer + Sets & Tones)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: root._tight ? 30 : 34
            visible: root.part !== null && root.assignedName.length > 0
            radius: 4
            color: LogicTheme.selectedBg
            border.width: 1
            border.color: LogicTheme.accent

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
                FaIcon {
                    icon: FaIcons.forCategory(App.tones.resolveCategory(root.assignedMsb, root.assignedLsb, root.assignedProgram))
                    size: LogicTheme.fontSize
                    iconColor: LogicTheme.textPrimary
                    Layout.preferredWidth: 18
                }
                Label {
                    text: root.assignedName
                    color: LogicTheme.textPrimary
                    font.pixelSize: root._tight ? LogicTheme.fontSizeSmall : LogicTheme.fontSize
                    font.bold: true
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width
            spacing: 6
            TextField {
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                placeholderText: "Search…"
                font.pixelSize: LogicTheme.fontSize
                onTextChanged: App.tones.filterText = text
            }
            FaIcon {
                icon: root.favoritesFilterOn ? FaIcons.star : FaIcons.starEmpty
                size: LogicTheme.fontSize + 4
                iconColor: root.favoritesFilterOn ? LogicTheme.warning : LogicTheme.textMuted
                opacity: root.favoritesFilterOn ? 1 : 0.55
                Layout.preferredWidth: 22
                Layout.preferredHeight: 22
                ToolTip.visible: favoritesStarHover.containsMouse
                ToolTip.text: root.favoritesFilterOn ? "Show all categories" : "Show favorites only"
                ToolTip.delay: 400
                MouseArea {
                    id: favoritesStarHover
                    anchors.fill: parent
                    anchors.margins: -4
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.toggleFavoritesFilter()
                }
            }
            ComboBox {
                model: App.tones.categories
                currentIndex: Math.max(0, model.indexOf(App.tones.category))
                onActivated: {
                    App.tones.category = currentText
                    if (currentText !== "Favorites")
                        root.categoryBeforeFavorites = currentText
                }
                Layout.preferredWidth: root._tight ? 110 : 160
                Layout.minimumWidth: root._tight ? 80 : 120
            }
        }

        ListView {
            id: toneList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: App.tones
            spacing: root._tight ? 0 : 1
            currentIndex: root.assignedRow
            highlightFollowsCurrentItem: false

            delegate: Rectangle {
                id: toneRow
                required property int index
                required property string name
                required property string category
                required property bool favorite
                required property int bankMsb
                required property int bankLsb
                required property int program

                readonly property bool assigned: root.isAssignedTone(bankMsb, bankLsb, program)

                width: ListView.view.width
                height: root._tight ? 28 : 32
                color: {
                    if (assigned)
                        return LogicTheme.selectedBg
                    if (root.compact)
                        return index % 2 ? LogicTheme.panelBgRaised : "transparent"
                    return index % 2 ? LogicTheme.panelBg : LogicTheme.windowBg
                }
                radius: root._tight ? 3 : 0
                border.width: assigned ? 1 : 0
                border.color: LogicTheme.accent

                MouseArea {
                    anchors.fill: parent
                    z: -1
                    onClicked: App.applyToneToSelectedPart(index)
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: root._tight ? 6 : 8
                    anchors.rightMargin: root._tight ? 6 : 8
                    spacing: root._tight ? 6 : 8

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
                        MouseArea {
                            anchors.fill: parent
                            anchors.margins: -6
                            cursorShape: Qt.PointingHandCursor
                            onClicked: App.previewTone(index)
                        }
                    }
                    Label {
                        text: name
                        color: LogicTheme.textPrimary
                        font.pixelSize: root._tight ? LogicTheme.fontSizeSmall : LogicTheme.fontSize
                        font.bold: toneRow.assigned
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                    Label {
                        visible: !root._tight
                        text: category
                        color: LogicTheme.textSecondary
                        font.pixelSize: LogicTheme.fontSizeSmall
                        Layout.preferredWidth: 90
                        elide: Text.ElideRight
                    }
                }
            }
        }

        Label {
            visible: !root._tight
            text: "Click a tone to assign · Click the instrument icon to preview · Star to favorite · Filter: Favorites"
            color: LogicTheme.textMuted
            font.pixelSize: LogicTheme.fontSizeSmall
        }
    }
}
