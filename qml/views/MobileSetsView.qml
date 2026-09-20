import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Item {
    id: root
    readonly property bool wide: width >= 700
    ColumnLayout {
        anchors.fill: parent
        spacing: 8
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 12
            ComboBox {
                model: ["All", "User", "Preset"]
                currentIndex: Math.max(0, model.indexOf(App.studioSets.groupFilter))
                onActivated: App.studioSets.groupFilter = currentText
                implicitHeight: 44
                Layout.preferredWidth: 100
            }
            ComboBox {
                id: sets
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                implicitHeight: 44
                model: App.studioSets
                textRole: "label"
                currentIndex: App.studioSets.currentRow
                displayText: currentIndex < 0 ? "Choose a Studio Set…" : App.studioSets.currentLabel
                onActivated: index => App.openStudioSet(index)
                delegate: ItemDelegate {
                    required property int index
                    required property string label
                    required property string name
                    width: sets.width
                    implicitHeight: 48
                    text: label + "  " + name
                    highlighted: sets.highlightedIndex === index
                }
            }
        }
        ComboBox {
            visible: !root.wide
            Layout.fillWidth: true
            Layout.leftMargin: 12; Layout.rightMargin: 12
            implicitHeight: 44
            model: App.studioSet
            textRole: "toneName"
            currentIndex: App.studioSet.selectedPart
            displayText: "Part " + (currentIndex + 1) + " · " + (App.studioSet.selectedPartModel ? App.studioSet.selectedPartModel.toneName : "")
            onActivated: index => App.studioSet.selectedPart = index
            delegate: ItemDelegate {
                required property int index
                required property string toneName
                width: parent ? parent.width : 300
                implicitHeight: 48
                text: (index + 1) + " · " + toneName
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 12; Layout.rightMargin: 12; Layout.bottomMargin: 12
            spacing: 16
            ListView {
                visible: root.wide
                Layout.preferredWidth: Math.min(300, root.width * 0.32)
                Layout.fillHeight: true
                clip: true
                model: App.studioSet
                spacing: 4
                ScrollBar.vertical: ScrollBar {}
                delegate: ItemDelegate {
                    required property int index
                    required property string toneName
                    width: ListView.view.width
                    implicitHeight: 48
                    text: (index + 1) + " · " + toneName
                    highlighted: App.studioSet.selectedPart === index
                    onClicked: App.studioSet.selectedPart = index
                }
            }
            ToneBrowser {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 0
                narrow: true
            }
        }
    }
}
