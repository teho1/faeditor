import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FAEditor

Dialog {
    id: root
    parent: Overlay.overlay
    anchors.centerIn: parent
    modal: true
    title: "Help"
    standardButtons: Dialog.Ok
    width: Math.min(560, Overlay.overlay ? Overlay.overlay.width - 40 : 560)
    height: Math.min(560, Overlay.overlay ? Overlay.overlay.height - 40 : 560)

    contentItem: ScrollView {
        clip: true
        ColumnLayout {
            width: root.availableWidth
            spacing: 10

            Label {
                text: "Quick start"
                font.bold: true
                color: LogicTheme.textPrimary
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: LogicTheme.textSecondary
                font.pixelSize: LogicTheme.fontSize
                text:
                    "1. On the FA: System → USB Driver → Vendor (MIDI+Audio), and install Roland’s USB driver if needed.\n"
                    + "2. In FA Editor: MIDI → Auto-connect FA (or pick ports — skip DAW CTRL).\n"
                    + "3. Tab 1 Sets & Tones: pick a User/Preset slot from the FA Set dropdown (top), select a part, click a tone to assign (instrument icon previews). Local projects live in the Library panel on the same tab.\n"
                    + "4. Use Pull Temp / Push Temp for a full Temporary read/write. To store a User slot permanently, use Write on the FA — SysEx only edits Temporary."
            }

            Label {
                text: "Tabs & shortcuts"
                font.bold: true
                color: LogicTheme.textPrimary
                Layout.topMargin: 6
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: LogicTheme.textSecondary
                font.pixelSize: LogicTheme.fontSize
                text:
                    "1 Sets & Tones · 2 Mixer · 3 Effects\n"
                    + "Keys 1–3 switch tabs (disabled while typing in a text field).\n"
                    + "Ctrl+S save to library · Ctrl+R pull Temporary · Ctrl+P push Temporary."
            }

            Label {
                text: "Library & favourites"
                font.bold: true
                color: LogicTheme.textPrimary
                Layout.topMargin: 6
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: LogicTheme.textSecondary
                font.pixelSize: LogicTheme.fontSize
                text:
                    "Save stores the current Temporary Studio Set as a local JSON project and shows it in the Library panel on Sets & Tones. "
                    + "Star tones in the browser to keep favourites. Library files live in the app’s Application Support folder."
            }

            Label {
                text: "Effects"
                font.bold: true
                color: LogicTheme.textPrimary
                Layout.topMargin: 6
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: LogicTheme.textSecondary
                font.pixelSize: LogicTheme.fontSize
                text:
                    "Tab 3 shows the FA EFFECTS EDIT signal flow: Part → Chorus/Reverb → Master Comp, plus Audio Input → NS → TFX → MIC Reverb. "
                    + "Click a block to edit. Pick a part from the Part box dropdown. Input Gain lives on Audio Input; TFX Location on the TFX block. "
                    + "MFX is shown for routing only (not SysEx yet — edit on the FA). "
                    + "Some USB Audio routing options are not SysEx-controllable "
                    + "(System → System Effects → USB Audio / USB Driver)."
            }

            Label {
                text: "Limitations"
                font.bold: true
                color: LogicTheme.textPrimary
                Layout.topMargin: 6
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: LogicTheme.textSecondary
                font.pixelSize: LogicTheme.fontSize
                text:
                    "Studio Set / Temporary editing only — not a full tone designer. Relies on documented Roland SysEx. "
                    + "Always keep a backup of important User Studio Sets on the instrument or SD card."
            }
        }
    }
}
