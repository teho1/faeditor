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
                    + "3. Tab 1 Studio Sets: Open a User/Preset slot into Temporary (or scan User names).\n"
                    + "4. Tab 2 Change Tone: pick a part → Change → double-click a tone (live SysEx to Temporary).\n"
                    + "5. Use Pull Temp / Push Temp for a full Temporary read/write. To store a User slot permanently, use Write on the FA — SysEx only edits Temporary."
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
                    "1 Studio Sets · 2 Change Tone · 3 Mixer · 4 Audio FX · 5 Studio FX · 6 Library\n"
                    + "Keys 1–6 switch tabs (disabled while typing in a text field).\n"
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
                    "Save stores the current Temporary Studio Set as a local JSON project and opens the Library tab. "
                    + "Star tones in the browser to keep favourites. Library files live in the app’s Application Support folder."
            }

            Label {
                text: "Audio input FX"
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
                    "TFX Location → INPUT for guitar/line. Some USB Audio routing options are not SysEx-controllable and must be set on the FA "
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
