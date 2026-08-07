import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FantomEditor

ApplicationWindow {
    id: root
    width: 1380; height: 820; minimumWidth: 980; minimumHeight: 620
    visible: true; title: "Fantom Editor — " + App.scene.name
    color: LogicTheme.windowBg

    header: ToolBar {
        contentHeight: 52
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16; spacing: 10
            Label { text: "FANTOM"; color: LogicTheme.accent; font.pixelSize: 20; font.bold: true }
            Label { text: App.scene.name; color: LogicTheme.textPrimary; font.bold: true }
            Item { Layout.fillWidth: true }
            Label { text: App.midi.connected ? App.deviceProfile : App.midi.statusText; color: App.midi.connected ? "#39d98a" : LogicTheme.textSecondary }
            Button { text: App.midi.connected ? "Disconnect" : "MIDI"; onClicked: App.midi.connected ? App.midi.disconnectDevice() : midiDialog.open() }
            Button { text: "Pull"; enabled: App.midi.connected && !App.scene.busy; onClicked: App.scene.pull() }
            Button { text: "Push Temporary"; enabled: App.midi.connected && !App.scene.busy; onClicked: App.scene.push() }
        }
    }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 14; spacing: 10
        Frame {
            Layout.fillWidth: true; padding: 10
            background: Rectangle { color: LogicTheme.panelBg; border.color: LogicTheme.border; radius: 6 }
            RowLayout {
                anchors.fill: parent
                Label { text: App.safetyNotice; color: LogicTheme.warning; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                Label { text: App.scene.edited ? "● Edited" : "● Synced"; color: App.scene.edited ? "#ffd60a" : "#39d98a" }
            }
        }

        TabBar { id: tabs; Layout.fillWidth: true
            TabButton { text: "Scene / Zones" }
            TabButton { text: "Tone assignment" }
            TabButton { text: "Effects & routing" }
        }

        StackLayout {
            currentIndex: tabs.currentIndex; Layout.fillWidth: true; Layout.fillHeight: true
            Item {
                ScrollView { anchors.fill: parent; clip: true
                    ListView {
                        id: zoneList; model: App.scene; spacing: 6
                        delegate: Frame {
                            required property int index
                            required property int zoneNumber
                            required property int toneMsb
                            required property int toneLsb
                            required property int toneProgram
                            required property int midiChannel
                            required property bool muted
                            required property bool receiveEnabled
                            required property int level
                            required property int pan
                            required property bool keyboardEnabled
                            width: zoneList.width - 18; height: 92; padding: 9
                            background: Rectangle { color: LogicTheme.panelBg; border.color: muted ? "#e05a5a" : LogicTheme.border; radius: 6 }
                            RowLayout { anchors.fill: parent; spacing: 10
                                Label { text: "ZONE\n" + zoneNumber; color: LogicTheme.accent; font.bold: true; horizontalAlignment: Text.AlignHCenter; Layout.preferredWidth: 58 }
                                ColumnLayout { Layout.preferredWidth: 150
                                    Label { text: "Tone " + toneMsb + ":" + toneLsb + ":" + toneProgram; color: LogicTheme.textPrimary; font.bold: true }
                                    Label { text: "MIDI Ch " + midiChannel; color: LogicTheme.textSecondary }
                                }
                                CheckBox { text: "KBD"; checked: keyboardEnabled; onToggled: App.scene.setZoneValue(index,"keyboard",checked) }
                                CheckBox { text: "RX"; checked: receiveEnabled; onToggled: App.scene.setZoneValue(index,"receive",checked) }
                                CheckBox { text: "Mute"; checked: muted; onToggled: App.scene.setZoneValue(index,"mute",checked) }
                                ColumnLayout { Layout.fillWidth: true
                                    Label { text: "Level  " + level; color: LogicTheme.textSecondary }
                                    Slider { Layout.fillWidth: true; from: 0; to: 127; value: level; onMoved: App.scene.setZoneValue(index,"level",Math.round(value)) }
                                }
                                ColumnLayout { Layout.preferredWidth: 210
                                    Label { text: "Pan  " + (pan === 0 ? "C" : pan < 0 ? "L" + -pan : "R" + pan); color: LogicTheme.textSecondary }
                                    Slider { Layout.fillWidth: true; from: -64; to: 63; value: pan; onMoved: App.scene.setZoneValue(index,"pan",Math.round(value)) }
                                }
                                Button { text: "Test"; onPressed: App.previewNote(index,60,100,true); onReleased: App.previewNote(index,60,0,false) }
                            }
                        }
                    }
                }
            }
            Item {
                ColumnLayout { anchors.fill: parent; spacing: 10
                    Label { text: "Assign a documented FANTOM tone bank to Temporary Scene"; color: LogicTheme.textPrimary; font.pixelSize: 20; font.bold: true }
                    Label { text: "Use the MSB/LSB/Program values from the Roland FANTOM-06/07/08 Sound List. The selected tone is loaded into the zone's temporary buffer."; color: LogicTheme.textSecondary; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    GridLayout { columns: 5; columnSpacing: 12; rowSpacing: 8
                        Label { text: "Zone"; color: LogicTheme.textSecondary }
                        SpinBox { id: assignZone; from: 1; to: 16; value: 1 }
                        Label { text: "Bank MSB"; color: LogicTheme.textSecondary }
                        SpinBox { id: assignMsb; from: 0; to: 127; value: 87 }
                        Item { width: 1; height: 1 }
                        Label { text: "Bank LSB"; color: LogicTheme.textSecondary }
                        SpinBox { id: assignLsb; from: 0; to: 127; value: 64 }
                        Label { text: "Program"; color: LogicTheme.textSecondary }
                        SpinBox { id: assignPc; from: 1; to: 128; value: 1 }
                        Button { text: "Assign Temporary"; enabled: App.midi.connected; onClicked: App.scene.assignTone(assignZone.value-1,assignMsb.value,assignLsb.value,assignPc.value) }
                    }
                    Rectangle { Layout.fillWidth: true; height: 1; color: LogicTheme.border }
                    Label { text: "Tone-engine editors"; color: LogicTheme.textPrimary; font.pixelSize: 18; font.bold: true }
                    Label { text: "Z-Core, SN-A, VTW, EXSN and Model Tone temporary address spaces are enabled in the adapter. Visual parameter panels are being exposed engine-by-engine; no permanent user-memory command is used."; color: LogicTheme.warning; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Item { Layout.fillHeight: true }
                }
            }
            Item {
                GridLayout { anchors.fill: parent; columns: 2; columnSpacing: 12; rowSpacing: 12
                    Repeater { model: App.scene
                        delegate: Frame {
                            required property int index; required property int zoneNumber; required property int chorusSend; required property int reverbSend; required property int outputAssign; required property bool eqEnabled
                            Layout.fillWidth: true; padding: 10
                            background: Rectangle { color: LogicTheme.panelBg; border.color: LogicTheme.border; radius: 6 }
                            GridLayout { anchors.fill: parent; columns: 4
                                Label { text: "Zone " + zoneNumber; color: LogicTheme.accent; font.bold: true }
                                Label { text: "Chorus " + chorusSend; color: LogicTheme.textSecondary }
                                Label { text: "Reverb " + reverbSend; color: LogicTheme.textSecondary }
                                Label { text: "Output " + ["MAIN","IFX1","IFX2","SUB"][outputAssign] + (eqEnabled ? " · EQ" : ""); color: LogicTheme.textPrimary }
                            }
                        }
                    }
                }
            }
        }
        Label { Layout.fillWidth: true; text: App.scene.status; color: LogicTheme.textSecondary; elide: Text.ElideRight }
    }

    Dialog {
        id: midiDialog; title: "Connect FANTOM-06/07/08"; modal: true; anchors.centerIn: parent; width: 520
        standardButtons: Dialog.Close
        onOpened: App.midi.refresh()
        ColumnLayout { anchors.fill: parent; spacing: 10
            Label { text: "MIDI input" }
            ComboBox { Layout.fillWidth: true; model: App.midi.inputPortNames; currentIndex: App.midi.selectedInput; onActivated: App.midi.selectedInput=currentIndex }
            Label { text: "MIDI output" }
            ComboBox { Layout.fillWidth: true; model: App.midi.outputPortNames; currentIndex: App.midi.selectedOutput; onActivated: App.midi.selectedOutput=currentIndex }
            RowLayout { Button { text: "Auto-connect FANTOM"; onClicked: { if(App.midi.autoConnectInstrument()) midiDialog.close() } }
                        Button { text: "Connect selected"; onClicked: { if(App.midi.connectSelected()) midiDialog.close() } } }
            Label { text: App.midi.statusText; color: LogicTheme.textSecondary; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        }
    }
}
