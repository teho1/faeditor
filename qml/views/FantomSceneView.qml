import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Item {
    ColumnLayout { anchors.fill: parent; anchors.margins: 10; spacing: 8
        RowLayout { Layout.fillWidth: true
            Label { text: "SCENE  " + App.scene.name; color: LogicTheme.textPrimary; font.pixelSize: 20; font.bold: true }
            Item { Layout.fillWidth: true }
            Label { text: "Temporary memory only — save on FANTOM to keep changes"; color: LogicTheme.warning }
        }
        TabBar { id: ftabs; Layout.fillWidth: true
            TabButton { text: "Scene / Zones" }
            TabButton { text: "Tone assignment" }
            TabButton { text: "Effects & routing" }
        }
        StackLayout { Layout.fillWidth: true; Layout.fillHeight: true; currentIndex: ftabs.currentIndex
            ScrollView { clip: true
                ListView { id: zones; model: App.scene; spacing: 5
                    delegate: Frame {
                        required property int index; required property int zoneNumber; required property int toneMsb; required property int toneLsb; required property int toneProgram
                        required property int midiChannel; required property bool muted; required property bool receiveEnabled; required property bool keyboardEnabled; required property int level; required property int pan
                        width: zones.width-16; height: 82; padding: 7
                        background: Rectangle { color: LogicTheme.panelBg; border.color: muted ? "#e05a5a" : LogicTheme.border; radius: 5 }
                        RowLayout { anchors.fill: parent; spacing: 8
                            Label { text: "ZONE\n"+zoneNumber; color: LogicTheme.accent; font.bold: true; horizontalAlignment: Text.AlignHCenter; Layout.preferredWidth: 52 }
                            Label { text: toneMsb+":"+toneLsb+":"+toneProgram+"\nCh "+midiChannel; color: LogicTheme.textPrimary; Layout.preferredWidth: 105 }
                            CheckBox { text:"KBD"; checked:keyboardEnabled; onToggled:App.scene.setZoneValue(index,"keyboard",checked) }
                            CheckBox { text:"RX"; checked:receiveEnabled; onToggled:App.scene.setZoneValue(index,"receive",checked) }
                            CheckBox { text:"Mute"; checked:muted; onToggled:App.scene.setZoneValue(index,"mute",checked) }
                            ColumnLayout { Layout.fillWidth: true; Label{text:"Level "+level;color:LogicTheme.textSecondary} Slider{Layout.fillWidth:true;from:0;to:127;value:level;onMoved:App.scene.setZoneValue(index,"level",Math.round(value))} }
                            ColumnLayout { Layout.fillWidth: true; Label{text:"Pan "+pan;color:LogicTheme.textSecondary} Slider{Layout.fillWidth:true;from:-64;to:63;value:pan;onMoved:App.scene.setZoneValue(index,"pan",Math.round(value))} }
                            Button { text:"Test"; onPressed:App.previewFantomZone(index,60,100,true);onReleased:App.previewFantomZone(index,60,0,false) }
                        }
                    }
                }
            }
            Item { ColumnLayout { anchors.fill: parent; anchors.margins: 20
                Label { text:"Assign tone to Temporary Zone";color:LogicTheme.textPrimary;font.pixelSize:20;font.bold:true }
                GridLayout { columns:5
                    Label{text:"Zone"} SpinBox{id:z;from:1;to:16}
                    Label{text:"MSB"} SpinBox{id:msb;from:0;to:127;value:87} Item{}
                    Label{text:"LSB"} SpinBox{id:lsb;from:0;to:127;value:64}
                    Label{text:"Program"} SpinBox{id:pc;from:1;to:128;value:1}
                    Button{text:"Assign";onClicked:App.scene.assignTone(z.value-1,msb.value,lsb.value,pc.value)}
                }
                Label{text:"Use bank values from the official FANTOM-06/07/08 Sound List. Z-Core and other engine-specific visual editors are enabled as their shared parameter adapters are completed.";color:LogicTheme.textSecondary;wrapMode:Text.WordWrap;Layout.fillWidth:true}
                Item{Layout.fillHeight:true}
            }}
            ScrollView { GridLayout { width:parent.width;columns:2
                Repeater { model:App.scene; delegate:Frame {
                    required property int zoneNumber;required property int chorusSend;required property int reverbSend;required property int outputAssign;required property bool eqEnabled
                    Layout.fillWidth:true
                    RowLayout{anchors.fill:parent;Label{text:"Zone "+zoneNumber;color:LogicTheme.accent;font.bold:true}Item{Layout.fillWidth:true}Label{text:"Chorus "+chorusSend+"   Reverb "+reverbSend+"   "+["MAIN","IFX1","IFX2","SUB"][outputAssign]+(eqEnabled?" · EQ":"");color:LogicTheme.textPrimary}}
                }}
            }}
        }
    }
}
