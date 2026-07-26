#include "model/SnAcousticInstCatalog.h"

#include "midi/AddressMap.h"

#include <algorithm>

namespace SnAcousticInstCatalog {
namespace {

ModifySlot named(const QString &name, int minV, int maxV, int displayOffset = 0,
                 const QStringList &enums = {})
{
    return {name, minV, maxV, displayOffset, enums};
}

ModifySlot unused()
{
    return {};
}

QVector<ModifySlot> emptySlots()
{
    return QVector<ModifySlot>(ModifyCount);
}

QVector<ModifySlot> fillNamed(std::initializer_list<ModifySlot> namedSlots)
{
    QVector<ModifySlot> modSlots = emptySlots();
    int i = 0;
    for (const ModifySlot &s : namedSlots) {
        if (i >= ModifyCount)
            break;
        modSlots[i++] = s;
    }
    return modSlots;
}

/** Place named params at specific 0-based Modify indices (CC16→0 … CC19→3). */
QVector<ModifySlot> atIndices(std::initializer_list<std::pair<int, ModifySlot>> entries)
{
    QVector<ModifySlot> modSlots = emptySlots();
    for (const auto &e : entries) {
        if (e.first >= 0 && e.first < ModifyCount)
            modSlots[e.first] = e.second;
    }
    return modSlots;
}

const QStringList &kInstrumentNames()
{
    static const QStringList names = {
        QStringLiteral("Concert Grand"),
        QStringLiteral("Grand Piano1"),
        QStringLiteral("Grand Piano2"),
        QStringLiteral("Grand Piano3"),
        QStringLiteral("Mellow Piano"),
        QStringLiteral("Bright Piano"),
        QStringLiteral("Upright Piano"),
        QStringLiteral("Concert Mono"),
        QStringLiteral("Honky-tonk"),
        QStringLiteral("Pure Vintage EP1"),
        QStringLiteral("Pure Vintage EP2"),
        QStringLiteral("Pure Wurly"),
        QStringLiteral("Pure Vintage EP3"),
        QStringLiteral("Old Hammer EP"),
        QStringLiteral("Dyno Piano"),
        QStringLiteral("Clav CB Flat"),
        QStringLiteral("Clav CA Flat"),
        QStringLiteral("Clav CB Medium"),
        QStringLiteral("Clav CA Medium"),
        QStringLiteral("Clav CB Brillia"),
        QStringLiteral("Clav CA Brillia"),
        QStringLiteral("Clav CB Combo"),
        QStringLiteral("Clav CA Combo"),
        QStringLiteral("TW Organ"),
        QStringLiteral("Nylon Guitar"),
        QStringLiteral("SteelStr Guitar"),
        QStringLiteral("Acoustic Bass"),
        QStringLiteral("Fingered Bass"),
        QStringLiteral("Picked Bass"),
        QStringLiteral("Strings"),
        QStringLiteral("Marcato Strings"),
    };
    return names;
}

QVector<ModifySlot> acPianoSlots()
{
    // Parameter Guide order → Modify 1–6
    return fillNamed({
        named(QStringLiteral("String Resonance"), 0, 127),
        named(QStringLiteral("Key Off Resonance"), 0, 127),
        named(QStringLiteral("Hammer Noise"), 0, 4, 2), // −2…+2
        named(QStringLiteral("Stereo Width"), 0, 63),
        named(QStringLiteral("Nuance"), 0, 2, 0,
              {QStringLiteral("Type1"), QStringLiteral("Type2"), QStringLiteral("Type3")}),
        named(QStringLiteral("Tone Character"), 0, 10, 5), // −5…+5
    });
}

QVector<ModifySlot> noiseOnlySlots()
{
    // E.Piano / Clav / Bass — CC#16 Noise Level (−64…+63)
    return atIndices({
        {0, named(QStringLiteral("Noise Level"), 0, 127, 64)},
    });
}

QVector<ModifySlot> twOrganSlots()
{
    return fillNamed({
        named(QStringLiteral("16'"), 0, 8),
        named(QStringLiteral("5⅓'"), 0, 8),
        named(QStringLiteral("8'"), 0, 8),
        named(QStringLiteral("4'"), 0, 8),
        named(QStringLiteral("2⅔'"), 0, 8),
        named(QStringLiteral("2'"), 0, 8),
        named(QStringLiteral("1⅗'"), 0, 8),
        named(QStringLiteral("1⅓'"), 0, 8),
        named(QStringLiteral("1'"), 0, 8),
        named(QStringLiteral("Leakage"), 0, 127),
        named(QStringLiteral("Perc Sw"), 0, 1, 0, {QStringLiteral("OFF"), QStringLiteral("ON")}),
        named(QStringLiteral("Perc Soft"), 0, 1, 0, {QStringLiteral("NORM"), QStringLiteral("SOFT")}),
        named(QStringLiteral("Perc Soft Lv"), 0, 15),
        named(QStringLiteral("Perc Norm Lv"), 0, 15),
        named(QStringLiteral("Perc Slow"), 0, 1, 0, {QStringLiteral("FAST"), QStringLiteral("SLOW")}),
        named(QStringLiteral("Perc Slow T"), 0, 127),
        named(QStringLiteral("Perc Fast T"), 0, 127),
        named(QStringLiteral("Perc Harm"), 0, 1, 0, {QStringLiteral("2ND"), QStringLiteral("3RD")}),
        named(QStringLiteral("Perc Recharge"), 0, 15),
        named(QStringLiteral("Perc Bar Lv"), 0, 127),
        named(QStringLiteral("Key On Click"), 0, 31),
        named(QStringLiteral("Key Off Click"), 0, 31),
    });
}

QVector<ModifySlot> guitarSlots()
{
    // CC16 Noise, CC17 Strum Speed, CC19 Strum Mode → Modify 1, 2, 4
    return atIndices({
        {0, named(QStringLiteral("Noise Level"), 0, 127, 64)},
        {1, named(QStringLiteral("Strum Speed"), 0, 127, 64)},
        {3, named(QStringLiteral("Strum Mode"), 0, 1, 0,
                  {QStringLiteral("OFF"), QStringLiteral("ON")})},
    });
}

QVector<ModifySlot> stringsSlots()
{
    // CC19 Hold Legato Mode → Modify 4
    return atIndices({
        {3, named(QStringLiteral("Hold Legato Mode"), 0, 1, 0,
                  {QStringLiteral("OFF"), QStringLiteral("ON")})},
    });
}

} // namespace

QStringList instrumentNames()
{
    return kInstrumentNames();
}

QString instrumentName(int panelInstNo)
{
    if (panelInstNo < 1 || panelInstNo > PanelInstCount)
        return {};
    return kInstrumentNames().at(panelInstNo - 1);
}

Family familyForInst(int panelInstNo)
{
    if (panelInstNo >= 1 && panelInstNo <= 9)
        return Family::AcPiano;
    if (panelInstNo >= 10 && panelInstNo <= 15)
        return Family::EPiano;
    if (panelInstNo >= 16 && panelInstNo <= 23)
        return Family::Clav;
    if (panelInstNo == 24)
        return Family::TwOrgan;
    if (panelInstNo >= 25 && panelInstNo <= 26)
        return Family::Guitar;
    if (panelInstNo >= 27 && panelInstNo <= 29)
        return Family::Bass;
    if (panelInstNo >= 30 && panelInstNo <= 31)
        return Family::Strings;
    return Family::Unknown;
}

QString familyLabel(Family family)
{
    switch (family) {
    case Family::AcPiano: return QStringLiteral("Ac. Piano");
    case Family::EPiano: return QStringLiteral("E. Piano");
    case Family::Clav: return QStringLiteral("Clav");
    case Family::TwOrgan: return QStringLiteral("TW Organ");
    case Family::Guitar: return QStringLiteral("Ac. Guitar");
    case Family::Bass: return QStringLiteral("Bass");
    case Family::Strings: return QStringLiteral("Strings");
    default: return QStringLiteral("Instrument");
    }
}

QString familyId(Family family)
{
    switch (family) {
    case Family::AcPiano: return QStringLiteral("acPiano");
    case Family::EPiano: return QStringLiteral("ePiano");
    case Family::Clav: return QStringLiteral("clav");
    case Family::TwOrgan: return QStringLiteral("twOrgan");
    case Family::Guitar: return QStringLiteral("guitar");
    case Family::Bass: return QStringLiteral("bass");
    case Family::Strings: return QStringLiteral("strings");
    default: return QStringLiteral("unknown");
    }
}

QVector<ModifySlot> modifySlotsForInst(int panelInstNo)
{
    switch (familyForInst(panelInstNo)) {
    case Family::AcPiano: return acPianoSlots();
    case Family::EPiano:
    case Family::Clav:
    case Family::Bass: return noiseOnlySlots();
    case Family::TwOrgan: return twOrganSlots();
    case Family::Guitar: return guitarSlots();
    case Family::Strings: return stringsSlots();
    default: return emptySlots();
    }
}

QStringList modifyNamesForInst(int panelInstNo)
{
    const QVector<ModifySlot> modSlots = modifySlotsForInst(panelInstNo);
    QStringList names;
    names.reserve(modSlots.size());
    for (const ModifySlot &s : modSlots)
        names.append(s.name);
    return names;
}

QStringList variationNamesForInst(int panelInstNo)
{
    // Inst Variation 0 = Normal; 1… = Performance Variation Sounds (p.29)
    switch (panelInstNo) {
    case 25: // Nylon Guitar
    case 26: // SteelStr Guitar
        return {QStringLiteral("Normal"), QStringLiteral("Mute"), QStringLiteral("Harmonics")};
    case 27: // Acoustic Bass
        return {QStringLiteral("Normal"), QStringLiteral("Staccato"), QStringLiteral("Harmonics")};
    case 28: // Fingered Bass
        return {QStringLiteral("Normal"), QStringLiteral("Slap"), QStringLiteral("Harmonics")};
    case 29: // Picked Bass
        return {QStringLiteral("Normal"), QStringLiteral("Bridge Mute"), QStringLiteral("Harmonics")};
    case 30: // Strings
    case 31: // Marcato Strings
        return {QStringLiteral("Normal"), QStringLiteral("Staccato"), QStringLiteral("Pizzicato"),
                QStringLiteral("Tremolo")};
    default:
        return {QStringLiteral("Normal")};
    }
}

int panelInstFromSysex(int sysexInstNumber)
{
    using namespace roland::snAcousticOff;
    if (sysexInstNumber == 0)
        return 1;
    if (sysexInstNumber == TwOrganInstNumberAlt || sysexInstNumber == TwOrganInstNumber)
        return 24;
    if (sysexInstNumber >= 1 && sysexInstNumber <= 23)
        return sysexInstNumber + 1;
    if (sysexInstNumber >= 25 && sysexInstNumber <= 31)
        return sysexInstNumber;
    // Already panel-like 1–31 (except ambiguous 24 handled above)
    if (sysexInstNumber >= 1 && sysexInstNumber <= PanelInstCount)
        return sysexInstNumber;
    return std::clamp(sysexInstNumber, 1, PanelInstCount);
}

int sysexInstFromPanel(int panelInstNo)
{
    panelInstNo = std::clamp(panelInstNo, 1, PanelInstCount);
    if (panelInstNo == 1)
        return 0; // 0-based Concert Grand
    if (panelInstNo == 24)
        return roland::snAcousticOff::TwOrganInstNumber; // 23
    if (panelInstNo <= 23)
        return panelInstNo - 1;
    return panelInstNo; // 25–31 stored as panel number
}

} // namespace SnAcousticInstCatalog
