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
            id: partPicker
            visible: !root.wide
            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            implicitHeight: 44
            model: App.studioSet
            textRole: "toneName"
            displayText: {
                const n = App.studioSet.selectedPart + 1
                const p = App.studioSet.selectedPartModel
                const name = p && p.toneName ? p.toneName : ""
                return name.length ? ("Part " + n + " · " + name) : ("Part " + n)
            }
            Component.onCompleted: currentIndex = App.studioSet.selectedPart
            Connections {
                target: App.studioSet
                function onSelectedPartChanged() {
                    partPicker.currentIndex = App.studioSet.selectedPart
                }
            }
            delegate: ItemDelegate {
                required property int index
                required property string toneName
                width: partPicker.width
                implicitHeight: 48
                text: (index + 1) + " · " + toneName
                highlighted: partPicker.highlightedIndex === index
            }
            onActivated: (index) => App.studioSet.selectPart(index)
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
                    onClicked: App.studioSet.selectPart(index)
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
