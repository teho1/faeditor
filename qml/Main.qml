import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

ApplicationWindow {
    id: root
    width: 2880
    height: 1800
    visible: true
    title: "FA Editor — " + (App.studioSet.name || "Studio Set")
    color: LogicTheme.windowBg

    readonly property bool _editingText: {
        const f = activeFocusItem
        if (!f)
            return false
        if (f instanceof TextInput || f instanceof TextEdit)
            return true
        if (f.contentItem && (f.contentItem instanceof TextInput || f.contentItem instanceof TextEdit))
            return true
        return false
    }

    function clearTextFocus() {
        // StackLayout keeps hidden pages alive — search fields can keep focus and
        // block digit tab shortcuts. Park focus on the tab bar instead.
        tabs.forceActiveFocus()
    }

    Connections {
        target: App
        function onMainTabChanged() { root.clearTextFocus() }
    }

    menuBar: MenuBar {
        Menu {
            title: "File"
            Action { text: "Save Local Copy"; shortcut: "Ctrl+S"; onTriggered: App.saveToLibrary() }
            Action { text: "Save As…"; onTriggered: App.library.saveAs(App.studioSet.name) }
            Action { text: "Refresh Library"; onTriggered: App.library.refresh() }
            MenuSeparator {}
            Action { text: "Quit"; shortcut: "Ctrl+Q"; onTriggered: Qt.quit() }
        }
        Menu {
            title: "MIDI"
            Action { text: "Connect…"; onTriggered: App.openMidiDialog() }
            Action { text: "Auto-connect FA"; onTriggered: App.startupConnect() }
            Action { text: "Disconnect"; onTriggered: App.midi.disconnectDevice() }
            MenuSeparator {}
            Action { text: "Pull Temporary"; shortcut: "Ctrl+R"; onTriggered: App.pull() }
            Action { text: "Push Temporary"; shortcut: "Ctrl+P"; onTriggered: App.push() }
        }
        Menu {
            title: "Help"
            Action { text: "FA Editor Help"; shortcut: "Ctrl+/"; onTriggered: helpDialog.open() }
            MenuSeparator {}
            Action { text: "About FA Editor"; onTriggered: aboutDialog.open() }
        }
    }

    header: Toolbar {
        onConnectClicked: App.openMidiDialog()
        onPullClicked: App.pull()
        onPushClicked: App.push()
        onSaveClicked: App.saveToLibrary()
    }

    footer: StatusBar {}

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            height: 28
            color: LogicTheme.panelBgRaised
            Label {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                verticalAlignment: Text.AlignVCenter
                text: App.workflowHint
                color: LogicTheme.textSecondary
                font.pixelSize: LogicTheme.fontSizeSmall
                elide: Text.ElideRight
            }
        }

        TabBar {
            id: tabs
            Layout.fillWidth: true
            focusPolicy: Qt.StrongFocus
            TabButton { text: "1. Studio Sets"; width: implicitWidth }
            TabButton { text: "2. Change Tone"; width: implicitWidth }
            TabButton { text: "3. Mixer"; width: implicitWidth }
            TabButton { text: "4. Audio FX"; width: implicitWidth }
            TabButton { text: "5. Studio FX"; width: implicitWidth }
            TabButton { text: "6. Library"; width: implicitWidth }
            currentIndex: App.mainTab
            onCurrentIndexChanged: {
                App.mainTab = currentIndex
                root.clearTextFocus()
            }
        }

        // Digits 1–6 switch tabs (skipped while a text field has focus)
        Shortcut { sequence: "1"; enabled: !root._editingText; onActivated: { App.mainTab = 0; root.clearTextFocus() } }
        Shortcut { sequence: "2"; enabled: !root._editingText; onActivated: { App.mainTab = 1; root.clearTextFocus() } }
        Shortcut { sequence: "3"; enabled: !root._editingText; onActivated: { App.mainTab = 2; root.clearTextFocus() } }
        Shortcut { sequence: "4"; enabled: !root._editingText; onActivated: { App.mainTab = 3; root.clearTextFocus() } }
        Shortcut { sequence: "5"; enabled: !root._editingText; onActivated: { App.mainTab = 4; root.clearTextFocus() } }
        Shortcut { sequence: "6"; enabled: !root._editingText; onActivated: { App.mainTab = 5; root.clearTextFocus() } }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: App.mainTab

                StudioSetBrowser {}
                ChangeToneView {}
                MixerView {}
                AudioFxPanel {}
                EffectsPanel {}
                LibraryView {}
            }

            Rectangle {
                Layout.preferredWidth: LogicTheme.inspectorWidth
                Layout.fillHeight: true
                color: LogicTheme.panelBg
                visible: App.mainTab === 1 || App.mainTab === 2

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    Label {
                        text: "Part " + (App.studioSet.selectedPart + 1)
                        color: LogicTheme.textPrimary
                        font.pixelSize: LogicTheme.fontSizeTitle
                        font.bold: true
                    }

                    PartInspector {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    ZoneKeyboard {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 90
                    }
                }
            }
        }
    }

    MidiConnectDialog {
        id: midiDialog
    }

    HelpDialog {
        id: helpDialog
    }

    AboutDialog {
        id: aboutDialog
        appVersion: "0.1.0"
        githubUrl: "https://github.com/teho1/faeditor"
    }
}
