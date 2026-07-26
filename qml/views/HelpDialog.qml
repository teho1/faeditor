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
    height: Math.min(580, Overlay.overlay ? Overlay.overlay.height - 40 : 580)

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
                    + "3. Tab 1 Sets & Tones: choose a User/Preset from the FA Set dropdown (opens that Temporary set). "
                    + "Select a part, click a tone to assign (instrument icon previews). Library projects sit in the right panel.\n"
                    + "4. Pull Temp / Push Temp for a full Temporary read/write. Permanent User store is Write on the FA — SysEx only edits Temporary."
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
                text: "FA Set dropdown & Scan"
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
                    "Picking a slot in the FA Set list loads it into Temporary. "
                    + "Scan reads Studio Set names from the FA (can take several minutes); results are cached in a local JSON file so you only need to scan once. "
                    + "Scanning again overwrites the cache. Opening the dropdown refreshes the current slot’s name from Temporary if you renamed it on the FA."
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
                    "New creates a local JSON project (suggested name = current Studio Set). "
                    + "Save and Dup appear only when a library file is open. "
                    + "Click a row to load it (you’ll be warned if there are unsaved edits; delete asks for confirmation). "
                    + "Save Studio Sets locally (parts, zones, studio FX, IFX, pads, audio FX…) and push back to Temporary. "
                    + "A library file is a Temporary Studio Set plus System Audio FX / Master EQ snapshot — not a per-tone MFX designer backup; User Write stays on the FA. "
                    + "In the tone list, use the star next to Search to show favourites only, or star individual tones. "
                    + "Library files live in the app’s Application Support folder."
            }

            Label {
                text: "Mixer"
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
                    "Channel strips stay on the left. The Tone row opens a tone picker that replaces part details in the inspector; use ^ to close. "
                    + "Chorus/Reverb sends and Main/Sub output are edited on the Effects tab."
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
                    "Tab 3 is an FA-style EFFECTS EDIT diagram: Part → Chorus/Reverb → Master Comp → Output, "
                    + "plus Audio Input → NS → TFX → MIC Reverb (TFX moves between Input and Main paths from TFX Location). "
                    + "Click a block to edit it. Part picker and Output Main/Sub are on the diagram. "
                    + "Input Gain is on Audio Input. MFX is routing context only (not SysEx yet — edit on the FA). "
                    + "Some USB Audio options are not SysEx-controllable "
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
                    "Temporary Studio Set + System Audio FX / Master EQ via SysEx — not a full tone or MFX designer. "
                    + "Permanent User store is Write on the FA. Always keep a backup of important User Studio Sets on the instrument or SD card."
            }
        }
    }
}
