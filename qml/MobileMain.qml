import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import FAEditor

ApplicationWindow {
    id: root
    width: 1024
    height: 768
    visible: true
    title: "FA Editor · iOS"
    color: LogicTheme.windowBg
    font.pixelSize: 15
    readonly property bool compact: width < 700
    property bool showLibrary: false
    property bool openingConnection: false
    readonly property int currentPage: showLibrary ? 4 : App.mainTab

    function selectPage(index) {
        showLibrary = index === 4
        if (!showLibrary) App.mainTab = index
    }

    function applyStoreScreenshotView() {
        const view = App.storeScreenshotView
        if (!view.length)
            return
        inspector.close()
        if (view === "library")
            selectPage(4)
        else if (view === "mixer" || view === "mixer-part") {
            selectPage(1)
            if (view === "mixer-part")
                Qt.callLater(function() { inspector.open() })
        }         else if (view === "effects")
            selectPage(2)
        else if (view === "tone" || view === "tone-filter" || view === "tone-amp")
            selectPage(3)
        else
            selectPage(0)
    }

    Component.onCompleted: applyStoreScreenshotView()
    Connections {
        target: App
        function onStoreScreenshotViewChanged() { root.applyStoreScreenshotView() }
    }

    header: ToolBar {
        ColumnLayout {
            anchors.fill: parent
            spacing: 0
            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 8
                spacing: 8
                Label {
                    text: App.fantomDevice ? App.scene.name : App.studioSet.name
                    font.bold: true
                    font.pixelSize: 18
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                }
                Button {
                    text: App.midi.connected ? "MIDI ✓" : "Connect"
                    implicitHeight: 44
                    onClicked: { root.openingConnection = !App.midi.connected; App.openMidiDialog() }
                }
                Button {
                    text: "Pull"
                    implicitHeight: 44
                    enabled: App.midi.connected
                    onClicked: App.pull()
                }
                ToolButton { text: "•••"; implicitWidth: 44; implicitHeight: 44; onClicked: actions.open() }
                Menu {
                    id: actions
                    MenuItem { text: "Push Temporary to FA"; enabled: App.midi.connected; onTriggered: App.push() }
                    MenuItem { text: "Save local copy"; onTriggered: { App.saveToLibrary(); root.showLibrary = true } }
                    MenuItem { text: "Import Roland SVD…"; onTriggered: svdFileDialog.open() }
                    MenuItem { text: "Export Studio Set MIDI…"; onTriggered: midiFileDialog.open() }
                    MenuItem { text: "Export tone names…"; onTriggered: namesFileDialog.open() }
                    MenuSeparator {}
                    MenuItem { text: "Disconnect"; enabled: App.midi.connected; onTriggered: App.disconnectInstrument() }
                    MenuItem { text: "About"; onTriggered: aboutDialog.open() }
                }
            }
            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 12; Layout.rightMargin: 12; Layout.bottomMargin: 8
                text: App.workflowHint
                color: LogicTheme.textSecondary
                font.pixelSize: 12
                elide: Text.ElideRight
            }
        }
    }

    footer: TabBar {
        visible: !App.fantomDevice
        currentIndex: root.currentPage
        Repeater {
            model: ["Sets", "Mixer", "Effects", "Tone", "Library"]
            TabButton {
                required property int index
                required property string modelData
                text: modelData
                implicitHeight: 52
                onClicked: root.selectPage(index)
            }
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: App.fantomDevice ? 5 : root.currentPage
        MobileSetsView {}
        Item {
            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                RowLayout {
                    Layout.fillWidth: true
                    Layout.margins: 8
                    Label {
                        text: "Part " + (App.studioSet.selectedPart + 1)
                        font.bold: true
                        Layout.fillWidth: true
                    }
                    Button { text: "Part controls"; implicitHeight: 44; onClicked: inspector.open() }
                    Button { text: "Change tone"; implicitHeight: 44; onClicked: root.selectPage(0) }
                }
                MixerView { Layout.fillWidth: true; Layout.fillHeight: true }
            }
        }
        MobileEditorViewport {
            editor: Component { EffectsEditView {} }
        }
        MobileEditorViewport {
            minimumEditorWidth: 820
            editor: Component { ToneEditView {} }
        }
        MobileEditorViewport {
            minimumEditorWidth: 360
            editor: Component { LibraryView { embedded: true } }
        }
        MobileEditorViewport {
            editor: Component { FantomSceneView {} }
        }
    }

    Drawer {
        id: inspector
        edge: Qt.RightEdge
        width: Math.min(root.width, 380)
        height: root.height
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            RowLayout {
                Layout.fillWidth: true
                Label { text: "Part " + (App.studioSet.selectedPart + 1); font.bold: true; Layout.fillWidth: true }
                Button { text: "Done"; implicitHeight: 44; onClicked: inspector.close() }
            }
            PartInspector {
                Layout.fillWidth: true
                Layout.fillHeight: true
                onTonePickerRequested: { inspector.close(); root.selectPage(0) }
            }
            ZoneKeyboard { Layout.fillWidth: true; Layout.preferredHeight: 100 }
        }
    }
    MidiConnectDialog {
        onDismissed: {
            if (root.openingConnection && App.midi.connected) Qt.callLater(function() { App.pull() })
            root.openingConnection = false
        }
    }
    AboutDialog { id: aboutDialog; appVersion: Qt.application.version; githubUrl: "https://github.com/teholapp/faeditor" }
    FileDialog {
        id: svdFileDialog
        title: "Import Roland FA Backup"
        nameFilters: ["Roland backup (*.SVD *.svd)"]
        onAccepted: {
            if (App.svdImport.loadFile(selectedFile)) importDrawer.open()
            else { importError.text = App.svdImport.lastError; importError.open() }
        }
    }
    MessageDialog { id: importError; title: "Could not import SVD" }
    Drawer {
        id: importDrawer
        edge: Qt.RightEdge
        width: Math.min(root.width, 600)
        height: root.height
        SvdImportPanel { anchors.fill: parent }
    }
    FileDialog {
        id: midiFileDialog
        title: "Export Studio Set MIDI"
        fileMode: FileDialog.SaveFile
        nameFilters: ["MIDI (*.mid)"]
        defaultSuffix: "mid"
        onAccepted: App.exportStudioSetMidi(selectedFile)
    }
    FileDialog {
        id: namesFileDialog
        title: "Export tone names"
        fileMode: FileDialog.SaveFile
        nameFilters: ["MIDI Name Document (*.midnam)"]
        defaultSuffix: "midnam"
        onAccepted: App.exportToneNamesMidnam(selectedFile)
    }
}
