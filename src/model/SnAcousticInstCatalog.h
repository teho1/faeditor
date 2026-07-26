#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

/** Parameter Guide Inst No. 1–31 → names, Modify maps, Variation labels. */
namespace SnAcousticInstCatalog {

inline constexpr int PanelInstCount = 31;
inline constexpr int ModifyCount = 32;

enum class Family {
    Unknown = 0,
    AcPiano,
    EPiano,
    Clav,
    TwOrgan,
    Guitar,
    Bass,
    Strings,
};

struct ModifySlot {
    QString name;           // empty → unused / hide in UI
    int minValue = 0;
    int maxValue = 127;
    int displayOffset = 0;  // display = raw - offset (64 for ±63, 2 for Hammer −2…+2, etc.)
    QStringList enumNames;  // non-empty → ComboBox
};

QStringList instrumentNames(); // index 0 = panel Inst 1
QString instrumentName(int panelInstNo);
Family familyForInst(int panelInstNo);
QString familyLabel(Family family);
QString familyId(Family family); // qml-friendly: "acPiano", …

/** Named Modify slots for panel Inst 1–31; length always 32. Empty name = unused. */
QVector<ModifySlot> modifySlotsForInst(int panelInstNo);
QStringList modifyNamesForInst(int panelInstNo);

/** Performance Variation Sounds (Inst Variation 0 = Normal / Off, then named). */
QStringList variationNamesForInst(int panelInstNo);

int panelInstFromSysex(int sysexInstNumber);
int sysexInstFromPanel(int panelInstNo);

} // namespace SnAcousticInstCatalog
