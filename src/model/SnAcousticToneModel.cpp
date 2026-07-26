#include "model/SnAcousticToneModel.h"
#include "model/SnAcousticInstCatalog.h"
#include "midi/SysexEngine.h"

#include <algorithm>

using namespace roland;

namespace {

QVector<SnAcousticInstCatalog::ModifySlot> currentModifySlots(const SnAcousticToneModel *self)
{
    const int panel = self->isTwOrgan() ? 24 : self->instNumberDisplay();
    return SnAcousticInstCatalog::modifySlotsForInst(panel);
}

} // namespace

SnAcousticToneModel::SnAcousticToneModel(SysexEngine *engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
{
    m_common = QByteArray(snAcousticOff::CommonSize, '\0');
    loadInitTemplate();
}

QStringList SnAcousticToneModel::percussionSoftNames() const
{
    return {QStringLiteral("NORM"), QStringLiteral("SOFT")};
}

QStringList SnAcousticToneModel::percussionSlowNames() const
{
    return {QStringLiteral("FAST"), QStringLiteral("SLOW")};
}

QStringList SnAcousticToneModel::percussionHarmonicNames() const
{
    return {QStringLiteral("2ND"), QStringLiteral("3RD")};
}

void SnAcousticToneModel::setError(const QString &e)
{
    if (m_lastError == e)
        return;
    m_lastError = e;
    emit lastErrorChanged();
}

int SnAcousticToneModel::commonByte(quint8 offset) const
{
    if (offset >= m_common.size())
        return 0;
    return static_cast<quint8>(m_common.at(offset));
}

void SnAcousticToneModel::writeCommonByte(quint8 offset, int value)
{
    if (m_fromDevice || !m_engine || !m_engine->isOpen())
        return;
    QByteArray d(1, static_cast<char>(value & 0x7F));
    QString err;
    if (!m_engine->writeParam(addOffset(addr::snAcousticCommon(m_partIndex), offset), d, &err))
        setError(err);
}

void SnAcousticToneModel::setCommonByte(quint8 offset, int value, int lo, int hi)
{
    value = std::clamp(value, lo, hi);
    if (offset >= m_common.size())
        return;
    if (static_cast<quint8>(m_common.at(offset)) == value)
        return;
    m_common[offset] = static_cast<char>(value);
    emit snaChanged();
    writeCommonByte(offset, value);
}

void SnAcousticToneModel::syncFromRaw()
{
    m_toneName.clear();
    for (int i = 0; i < snAcousticOff::NameLength && i < m_common.size(); ++i) {
        const char c = m_common.at(i);
        if (c >= 32 && c < 127)
            m_toneName.append(QChar(c));
    }
    m_toneName = m_toneName.trimmed();
    m_toneLevel = commonByte(snAcousticOff::ToneLevel);
    m_monoPoly = commonByte(snAcousticOff::MonoPoly) != 0; // 0=MONO, 1=POLY
    m_instVariation = commonByte(snAcousticOff::InstVariation);
    m_instNumber = commonByte(snAcousticOff::InstNumber);
}

int SnAcousticToneModel::portamentoTimeOffset() const { return commonByte(snAcousticOff::PortamentoTimeOffset); }
int SnAcousticToneModel::cutoffOffset() const { return commonByte(snAcousticOff::CutoffOffset); }
int SnAcousticToneModel::resonanceOffset() const { return commonByte(snAcousticOff::ResonanceOffset); }
int SnAcousticToneModel::attackTimeOffset() const { return commonByte(snAcousticOff::AttackTimeOffset); }
int SnAcousticToneModel::releaseTimeOffset() const { return commonByte(snAcousticOff::ReleaseTimeOffset); }
int SnAcousticToneModel::vibratoRate() const { return commonByte(snAcousticOff::VibratoRate); }
int SnAcousticToneModel::vibratoDepth() const { return commonByte(snAcousticOff::VibratoDepth); }
int SnAcousticToneModel::vibratoDelay() const { return commonByte(snAcousticOff::VibratoDelay); }
int SnAcousticToneModel::octaveShift() const { return commonByte(snAcousticOff::OctaveShift); }

int SnAcousticToneModel::instNumberDisplay() const
{
    return SnAcousticInstCatalog::panelInstFromSysex(m_instNumber);
}

void SnAcousticToneModel::setInstNumberDisplay(int panelInstNo)
{
    setInstNumber(SnAcousticInstCatalog::sysexInstFromPanel(panelInstNo));
}

QString SnAcousticToneModel::instrumentName() const
{
    return SnAcousticInstCatalog::instrumentName(instNumberDisplay());
}

QString SnAcousticToneModel::instrumentFamily() const
{
    return SnAcousticInstCatalog::familyId(
        SnAcousticInstCatalog::familyForInst(instNumberDisplay()));
}

QString SnAcousticToneModel::instrumentFamilyLabel() const
{
    return SnAcousticInstCatalog::familyLabel(
        SnAcousticInstCatalog::familyForInst(instNumberDisplay()));
}

QStringList SnAcousticToneModel::instrumentNames() const
{
    return SnAcousticInstCatalog::instrumentNames();
}

QStringList SnAcousticToneModel::instVariationNames() const
{
    return SnAcousticInstCatalog::variationNamesForInst(instNumberDisplay());
}

bool SnAcousticToneModel::isTwOrgan() const
{
    using namespace snAcousticOff;
    if (m_instNumber == TwOrganInstNumber || m_instNumber == TwOrganInstNumberAlt)
        return true;
    // Preset names like "B3 Jazz 1" are TW Organ under the hood.
    if (m_toneName.contains(QStringLiteral("B3"), Qt::CaseInsensitive)
        || m_toneName.contains(QStringLiteral("TW Organ"), Qt::CaseInsensitive))
        return true;
    return false;
}

QVariantList SnAcousticToneModel::modifyParams() const
{
    QVariantList list;
    list.reserve(snAcousticOff::ModifyCount);
    for (int i = 0; i < snAcousticOff::ModifyCount; ++i)
        list.append(modifyParam(i));
    return list;
}

QStringList SnAcousticToneModel::modifyParamNames() const
{
    const int panel = isTwOrgan() ? 24 : instNumberDisplay();
    QStringList names = SnAcousticInstCatalog::modifyNamesForInst(panel);
    const bool known = SnAcousticInstCatalog::familyForInst(panel)
                       != SnAcousticInstCatalog::Family::Unknown;
    if (!known) {
        names.clear();
        names.reserve(snAcousticOff::ModifyCount);
        for (int i = 0; i < snAcousticOff::ModifyCount; ++i)
            names.append(QStringLiteral("Modify %1").arg(i + 1));
        return names;
    }
    // Pad to 32 if needed
    while (names.size() < snAcousticOff::ModifyCount)
        names.append(QString());
    return names;
}

QVariantList SnAcousticToneModel::modifyParamMaxValues() const
{
    const auto modSlots = currentModifySlots(this);
    QVariantList list;
    list.reserve(snAcousticOff::ModifyCount);
    for (int i = 0; i < snAcousticOff::ModifyCount; ++i) {
        if (i < modSlots.size() && !modSlots[i].name.isEmpty())
            list.append(modSlots[i].maxValue);
        else
            list.append(127);
    }
    return list;
}

QVariantList SnAcousticToneModel::modifyParamMinValues() const
{
    const auto modSlots = currentModifySlots(this);
    QVariantList list;
    list.reserve(snAcousticOff::ModifyCount);
    for (int i = 0; i < snAcousticOff::ModifyCount; ++i) {
        if (i < modSlots.size() && !modSlots[i].name.isEmpty())
            list.append(modSlots[i].minValue);
        else
            list.append(0);
    }
    return list;
}

QVariantList SnAcousticToneModel::modifyParamDisplayOffsets() const
{
    const auto modSlots = currentModifySlots(this);
    QVariantList list;
    list.reserve(snAcousticOff::ModifyCount);
    for (int i = 0; i < snAcousticOff::ModifyCount; ++i) {
        if (i < modSlots.size())
            list.append(modSlots[i].displayOffset);
        else
            list.append(0);
    }
    return list;
}

int SnAcousticToneModel::modifyParamNamedCount() const
{
    int n = 0;
    for (const QString &name : modifyParamNames()) {
        if (!name.isEmpty())
            ++n;
    }
    return n;
}

int SnAcousticToneModel::modifyParam(int index) const
{
    if (index < 0 || index >= snAcousticOff::ModifyCount)
        return 0;
    return commonByte(static_cast<quint8>(snAcousticOff::ModifyParameter1 + index));
}

void SnAcousticToneModel::setModifyParam(int index, int value)
{
    if (index < 0 || index >= snAcousticOff::ModifyCount)
        return;
    int lo = 0;
    int hi = 127;
    const auto modSlots = currentModifySlots(this);
    if (index < modSlots.size() && !modSlots[index].name.isEmpty()) {
        lo = modSlots[index].minValue;
        hi = modSlots[index].maxValue;
    } else if (isTwOrgan()) {
        using namespace snAcousticOff;
        if (index <= TwBar1)
            hi = 8;
        else if (index == TwPercSwitch || index == TwPercSoft
                 || index == TwPercSlow || index == TwPercHarmonic)
            hi = 1;
        else if (index == TwPercSoftLevel || index == TwPercNormalLevel
                 || index == TwPercRecharge)
            hi = 15;
        else if (index == TwKeyOnClick || index == TwKeyOffClick)
            hi = 31;
    }
    setCommonByte(static_cast<quint8>(snAcousticOff::ModifyParameter1 + index), value, lo, hi);
}

QStringList SnAcousticToneModel::modifyParamEnumNames(int index) const
{
    const auto modSlots = currentModifySlots(this);
    if (index < 0 || index >= modSlots.size())
        return {};
    return modSlots[index].enumNames;
}

bool SnAcousticToneModel::modifyParamHasEnum(int index) const
{
    return !modifyParamEnumNames(index).isEmpty();
}

QString SnAcousticToneModel::modifyParamDisplayText(int index) const
{
    const int raw = modifyParam(index);
    const auto enums = modifyParamEnumNames(index);
    if (!enums.isEmpty()) {
        if (raw >= 0 && raw < enums.size())
            return enums.at(raw);
        return QString::number(raw);
    }
    const auto modSlots = currentModifySlots(this);
    const int offset = (index >= 0 && index < modSlots.size()) ? modSlots[index].displayOffset : 0;
    if (offset != 0)
        return QString::number(raw - offset);
    return QString::number(raw);
}

int SnAcousticToneModel::bar16() const { return modifyParam(snAcousticOff::TwBar16); }
int SnAcousticToneModel::bar5_13() const { return modifyParam(snAcousticOff::TwBar5_13); }
int SnAcousticToneModel::bar8() const { return modifyParam(snAcousticOff::TwBar8); }
int SnAcousticToneModel::bar4() const { return modifyParam(snAcousticOff::TwBar4); }
int SnAcousticToneModel::bar2_23() const { return modifyParam(snAcousticOff::TwBar2_23); }
int SnAcousticToneModel::bar2() const { return modifyParam(snAcousticOff::TwBar2); }
int SnAcousticToneModel::bar1_35() const { return modifyParam(snAcousticOff::TwBar1_35); }
int SnAcousticToneModel::bar1_13() const { return modifyParam(snAcousticOff::TwBar1_13); }
int SnAcousticToneModel::bar1() const { return modifyParam(snAcousticOff::TwBar1); }
int SnAcousticToneModel::leakageLevel() const { return modifyParam(snAcousticOff::TwLeakage); }
bool SnAcousticToneModel::percussionSwitch() const { return modifyParam(snAcousticOff::TwPercSwitch) != 0; }
int SnAcousticToneModel::percussionSoft() const { return modifyParam(snAcousticOff::TwPercSoft); }
int SnAcousticToneModel::percussionSoftLevel() const { return modifyParam(snAcousticOff::TwPercSoftLevel); }
int SnAcousticToneModel::percussionNormalLevel() const { return modifyParam(snAcousticOff::TwPercNormalLevel); }
int SnAcousticToneModel::percussionSlow() const { return modifyParam(snAcousticOff::TwPercSlow); }
int SnAcousticToneModel::percussionSlowTime() const { return modifyParam(snAcousticOff::TwPercSlowTime); }
int SnAcousticToneModel::percussionFastTime() const { return modifyParam(snAcousticOff::TwPercFastTime); }
int SnAcousticToneModel::percussionHarmonic() const { return modifyParam(snAcousticOff::TwPercHarmonic); }
int SnAcousticToneModel::percussionRechargeTime() const { return modifyParam(snAcousticOff::TwPercRecharge); }
int SnAcousticToneModel::percussionHarmonicBarLevel() const { return modifyParam(snAcousticOff::TwPercBarLevel); }
int SnAcousticToneModel::keyOnClickLevel() const { return modifyParam(snAcousticOff::TwKeyOnClick); }
int SnAcousticToneModel::keyOffClickLevel() const { return modifyParam(snAcousticOff::TwKeyOffClick); }

void SnAcousticToneModel::setBar16(int v) { setModifyParam(snAcousticOff::TwBar16, v); }
void SnAcousticToneModel::setBar5_13(int v) { setModifyParam(snAcousticOff::TwBar5_13, v); }
void SnAcousticToneModel::setBar8(int v) { setModifyParam(snAcousticOff::TwBar8, v); }
void SnAcousticToneModel::setBar4(int v) { setModifyParam(snAcousticOff::TwBar4, v); }
void SnAcousticToneModel::setBar2_23(int v) { setModifyParam(snAcousticOff::TwBar2_23, v); }
void SnAcousticToneModel::setBar2(int v) { setModifyParam(snAcousticOff::TwBar2, v); }
void SnAcousticToneModel::setBar1_35(int v) { setModifyParam(snAcousticOff::TwBar1_35, v); }
void SnAcousticToneModel::setBar1_13(int v) { setModifyParam(snAcousticOff::TwBar1_13, v); }
void SnAcousticToneModel::setBar1(int v) { setModifyParam(snAcousticOff::TwBar1, v); }
void SnAcousticToneModel::setLeakageLevel(int v) { setModifyParam(snAcousticOff::TwLeakage, v); }
void SnAcousticToneModel::setPercussionSwitch(bool v) { setModifyParam(snAcousticOff::TwPercSwitch, v ? 1 : 0); }
void SnAcousticToneModel::setPercussionSoft(int v) { setModifyParam(snAcousticOff::TwPercSoft, v); }
void SnAcousticToneModel::setPercussionSoftLevel(int v) { setModifyParam(snAcousticOff::TwPercSoftLevel, v); }
void SnAcousticToneModel::setPercussionNormalLevel(int v) { setModifyParam(snAcousticOff::TwPercNormalLevel, v); }
void SnAcousticToneModel::setPercussionSlow(int v) { setModifyParam(snAcousticOff::TwPercSlow, v); }
void SnAcousticToneModel::setPercussionSlowTime(int v) { setModifyParam(snAcousticOff::TwPercSlowTime, v); }
void SnAcousticToneModel::setPercussionFastTime(int v) { setModifyParam(snAcousticOff::TwPercFastTime, v); }
void SnAcousticToneModel::setPercussionHarmonic(int v) { setModifyParam(snAcousticOff::TwPercHarmonic, v); }
void SnAcousticToneModel::setPercussionRechargeTime(int v) { setModifyParam(snAcousticOff::TwPercRecharge, v); }
void SnAcousticToneModel::setPercussionHarmonicBarLevel(int v) { setModifyParam(snAcousticOff::TwPercBarLevel, v); }
void SnAcousticToneModel::setKeyOnClickLevel(int v) { setModifyParam(snAcousticOff::TwKeyOnClick, v); }
void SnAcousticToneModel::setKeyOffClickLevel(int v) { setModifyParam(snAcousticOff::TwKeyOffClick, v); }

void SnAcousticToneModel::setToneName(const QString &v)
{
    QString name = v.left(snAcousticOff::NameLength);
    while (name.size() < snAcousticOff::NameLength)
        name.append(QLatin1Char(' '));
    if (m_toneName == name.trimmed())
        return;
    m_toneName = name.trimmed();
    for (int i = 0; i < snAcousticOff::NameLength; ++i) {
        const char c = name.at(i).toLatin1();
        m_common[i] = c;
        writeCommonByte(static_cast<quint8>(i), static_cast<quint8>(c));
    }
    emit snaChanged();
}

void SnAcousticToneModel::setToneLevel(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_toneLevel == v)
        return;
    m_toneLevel = v;
    m_common[snAcousticOff::ToneLevel] = static_cast<char>(v);
    emit snaChanged();
    writeCommonByte(snAcousticOff::ToneLevel, v);
}

void SnAcousticToneModel::setMonoPoly(bool v)
{
    if (m_monoPoly == v)
        return;
    m_monoPoly = v;
    m_common[snAcousticOff::MonoPoly] = static_cast<char>(v ? 1 : 0);
    emit snaChanged();
    writeCommonByte(snAcousticOff::MonoPoly, v ? 1 : 0);
}

void SnAcousticToneModel::setPortamentoTimeOffset(int v)
{
    setCommonByte(snAcousticOff::PortamentoTimeOffset, v, 0, 127);
}
void SnAcousticToneModel::setCutoffOffset(int v)
{
    setCommonByte(snAcousticOff::CutoffOffset, v, 0, 127);
}
void SnAcousticToneModel::setResonanceOffset(int v)
{
    setCommonByte(snAcousticOff::ResonanceOffset, v, 0, 127);
}
void SnAcousticToneModel::setAttackTimeOffset(int v)
{
    setCommonByte(snAcousticOff::AttackTimeOffset, v, 0, 127);
}
void SnAcousticToneModel::setReleaseTimeOffset(int v)
{
    setCommonByte(snAcousticOff::ReleaseTimeOffset, v, 0, 127);
}
void SnAcousticToneModel::setVibratoRate(int v)
{
    setCommonByte(snAcousticOff::VibratoRate, v, 0, 127);
}
void SnAcousticToneModel::setVibratoDepth(int v)
{
    setCommonByte(snAcousticOff::VibratoDepth, v, 0, 127);
}
void SnAcousticToneModel::setVibratoDelay(int v)
{
    setCommonByte(snAcousticOff::VibratoDelay, v, 0, 127);
}
void SnAcousticToneModel::setOctaveShift(int v)
{
    setCommonByte(snAcousticOff::OctaveShift, v, 61, 67);
}

void SnAcousticToneModel::setInstVariation(int v)
{
    const QStringList vars = instVariationNames();
    const int hi = vars.size() > 1 ? vars.size() - 1 : 127;
    v = std::clamp(v, 0, hi);
    if (m_instVariation == v)
        return;
    m_instVariation = v;
    m_common[snAcousticOff::InstVariation] = static_cast<char>(v);
    emit snaChanged();
    writeCommonByte(snAcousticOff::InstVariation, v);
}

void SnAcousticToneModel::setInstNumber(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_instNumber == v)
        return;
    m_instNumber = v;
    m_common[snAcousticOff::InstNumber] = static_cast<char>(v);
    emit snaChanged();
    writeCommonByte(snAcousticOff::InstNumber, v);
}

void SnAcousticToneModel::loadInitTemplate()
{
    m_fromDevice = true;
    m_common.fill('\0');
    const QByteArray name = QByteArrayLiteral("Init SN-A   ");
    for (int i = 0; i < snAcousticOff::NameLength; ++i)
        m_common[i] = name.at(i);
    m_common[snAcousticOff::ToneLevel] = 100;
    m_common[snAcousticOff::MonoPoly] = 1; // POLY
    m_common[snAcousticOff::PortamentoTimeOffset] = 64;
    m_common[snAcousticOff::CutoffOffset] = 64;
    m_common[snAcousticOff::ResonanceOffset] = 64;
    m_common[snAcousticOff::AttackTimeOffset] = 64;
    m_common[snAcousticOff::ReleaseTimeOffset] = 64;
    m_common[snAcousticOff::VibratoRate] = 64;
    m_common[snAcousticOff::VibratoDepth] = 64;
    m_common[snAcousticOff::VibratoDelay] = 64;
    m_common[snAcousticOff::OctaveShift] = 64;
    m_common[snAcousticOff::PhraseOctaveShift] = 64;
    m_common[snAcousticOff::MfxSwitch] = 0;
    m_common[snAcousticOff::InstVariation] = 0;
    m_common[snAcousticOff::InstNumber] = 0;
    // Sensible TW Organ drawbar default if user switches Inst later
    for (int i = 0; i < 9; ++i)
        m_common[snAcousticOff::ModifyParameter1 + i] = (i == 2) ? 8 : 0; // 8' full
    syncFromRaw();
    m_fromDevice = false;
    emit snaChanged();
}

bool SnAcousticToneModel::pullFromDevice()
{
    if (!m_engine || !m_engine->isOpen()) {
        setError(QStringLiteral("Not connected"));
        return false;
    }
    QString err;
    QByteArray common;
    if (!m_engine->read(addr::snAcousticCommon(m_partIndex), snAcousticOff::CommonSize, &common, &err)) {
        setError(err);
        return false;
    }
    m_fromDevice = true;
    m_common = common;
    if (m_common.size() < snAcousticOff::CommonSize)
        m_common.resize(snAcousticOff::CommonSize);
    syncFromRaw();
    m_fromDevice = false;
    setError({});
    emit snaChanged();
    return true;
}

bool SnAcousticToneModel::pushToDevice()
{
    if (!m_engine || !m_engine->isOpen()) {
        setError(QStringLiteral("Not connected"));
        return false;
    }
    QString err;
    if (!m_engine->write(addr::snAcousticCommon(m_partIndex), m_common, &err)) {
        setError(err);
        return false;
    }
    setError({});
    return true;
}

QJsonObject SnAcousticToneModel::toJson() const
{
    QJsonObject o;
    o.insert(QStringLiteral("common"), QString::fromLatin1(m_common.toBase64()));
    return o;
}

bool SnAcousticToneModel::fromJson(const QJsonObject &obj)
{
    if (obj.isEmpty())
        return false;
    m_fromDevice = true;
    m_common = QByteArray::fromBase64(obj.value(QStringLiteral("common")).toString().toLatin1());
    if (m_common.size() < snAcousticOff::CommonSize)
        m_common.resize(snAcousticOff::CommonSize);
    syncFromRaw();
    m_fromDevice = false;
    emit snaChanged();
    return true;
}
