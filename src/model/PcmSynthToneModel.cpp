#include "model/PcmSynthToneModel.h"
#include "model/WaveformCatalog.h"
#include "platform/InstrumentPlatform.h"

#include <QJsonArray>
#include <algorithm>

using namespace roland;

namespace {

QByteArray encodeNibbles(int value, int nibbleCount)
{
    QByteArray out(nibbleCount, '\0');
    for (int i = nibbleCount - 1; i >= 0; --i) {
        out[i] = static_cast<char>(value & 0x0F);
        value >>= 4;
    }
    return out;
}

int decodeNibbles(const QByteArray &raw, int offset, int nibbleCount)
{
    int value = 0;
    for (int i = 0; i < nibbleCount; ++i) {
        if (offset + i >= raw.size())
            break;
        value = (value << 4) | (static_cast<quint8>(raw.at(offset + i)) & 0x0F);
    }
    return value;
}

} // namespace

PcmSynthToneModel::PcmSynthToneModel(InstrumentPlatform *platform, QObject *parent)
    : QObject(parent)
    , m_platform(platform)
{
    m_common = QByteArray(pcmSynthOff::CommonSize, '\0');
    m_pmt = QByteArray(pcmSynthOff::PmtSize, '\0');
    m_common2 = QByteArray(pcmSynthOff::Common2Size, '\0');
    for (auto &p : m_partials)
        p.raw = QByteArray(pcmSynthOff::PartialSize, '\0');
}

void PcmSynthToneModel::setWaveformCatalog(WaveformCatalog *catalog)
{
    if (m_waves == catalog)
        return;
    if (m_waves)
        disconnect(m_waves, nullptr, this, nullptr);
    m_waves = catalog;
    if (m_waves) {
        connect(m_waves, &WaveformCatalog::catalogChanged, this, [this]() {
            emit pcmChanged();
        });
    }
    emit pcmChanged();
}

QStringList PcmSynthToneModel::waveGroupTypeNames() const
{
    return {
        QStringLiteral("INT"), QStringLiteral("SRX"),
        QStringLiteral("---"), QStringLiteral("---")
    };
}

QStringList PcmSynthToneModel::waveGroupBankNames() const
{
    return {
        QStringLiteral("INT-A"), QStringLiteral("INT-B"), QStringLiteral("SRX")
    };
}

QString PcmSynthToneModel::partialDisplayName(int index) const
{
    if (index < 0 || index > 3)
        return {};
    return QStringLiteral("Partial %1").arg(index + 1);
}

QString PcmSynthToneModel::partialWaveLabel(int index) const
{
    if (index < 0 || index > 3)
        return {};
    const auto &raw = m_partials.at(static_cast<size_t>(index)).raw;
    const int groupType = (pcmSynthOff::WaveGroupType < raw.size())
                              ? static_cast<quint8>(raw.at(pcmSynthOff::WaveGroupType))
                              : 0;
    const int groupId = decodeNibbles(raw, pcmSynthOff::WaveGroupId, 4);
    const int number = decodeNibbles(raw, pcmSynthOff::WaveNumberL, 4);
    const QString bank = m_waves ? m_waves->pcmBankLabel(groupType, groupId)
                                 : (groupType == 1 ? QStringLiteral("SRX") : QStringLiteral("INT"));
    if (number <= 0)
        return QStringLiteral("%1 OFF").arg(bank);
    if (m_waves) {
        const QString name = m_waves->resolvePcmName(groupType, groupId, number);
        if (!name.isEmpty())
            return QStringLiteral("%1 %2").arg(bank, name);
    }
    return QStringLiteral("%1 %2").arg(bank).arg(number);
}

QStringList PcmSynthToneModel::partialWaveLabels() const
{
    QStringList out;
    out.reserve(4);
    for (int i = 0; i < 4; ++i)
        out.append(partialWaveLabel(i));
    return out;
}

QStringList PcmSynthToneModel::filterTypeNames() const
{
    return {
        QStringLiteral("OFF"), QStringLiteral("LPF"), QStringLiteral("BPF"),
        QStringLiteral("HPF"), QStringLiteral("PKG"), QStringLiteral("LPF2"),
        QStringLiteral("LPF3")
    };
}

QStringList PcmSynthToneModel::lfoWaveformNames() const
{
    return {
        QStringLiteral("SIN"), QStringLiteral("TRI"), QStringLiteral("SAW-UP"),
        QStringLiteral("SAW-DW"), QStringLiteral("SQR"), QStringLiteral("RND"),
        QStringLiteral("BEND-UP"), QStringLiteral("BEND-DW"), QStringLiteral("TRP"),
        QStringLiteral("S&H"), QStringLiteral("CHS"), QStringLiteral("VSIN"),
        QStringLiteral("STEP")
    };
}

QStringList PcmSynthToneModel::matrixSourceNames() const
{
    // Compact labels for MVP (indices match MIDI Implementation 0–109).
    QStringList names;
    names.reserve(110);
    names.append(QStringLiteral("OFF"));
    for (int cc = 1; cc <= 31; ++cc)
        names.append(QStringLiteral("CC%1").arg(cc, 2, 10, QChar('0')));
    for (int cc = 33; cc <= 95; ++cc)
        names.append(QStringLiteral("CC%1").arg(cc, 2, 10, QChar('0')));
    names.append(QStringLiteral("BEND"));
    names.append(QStringLiteral("AFT"));
    names.append(QStringLiteral("CTRL1"));
    names.append(QStringLiteral("CTRL2"));
    names.append(QStringLiteral("CTRL3"));
    names.append(QStringLiteral("CTRL4"));
    names.append(QStringLiteral("VELOCITY"));
    names.append(QStringLiteral("KEYFOLLOW"));
    names.append(QStringLiteral("TEMPO"));
    names.append(QStringLiteral("LFO1"));
    names.append(QStringLiteral("LFO2"));
    names.append(QStringLiteral("PIT-ENV"));
    names.append(QStringLiteral("TVF-ENV"));
    names.append(QStringLiteral("TVA-ENV"));
    while (names.size() < 110)
        names.append(QStringLiteral("?%1").arg(names.size()));
    return names;
}

QStringList PcmSynthToneModel::matrixDestNames() const
{
    return {
        QStringLiteral("OFF"), QStringLiteral("PCH"), QStringLiteral("CUT"),
        QStringLiteral("RES"), QStringLiteral("LEV"), QStringLiteral("PAN"),
        QStringLiteral("DRY"), QStringLiteral("CHO"), QStringLiteral("REV"),
        QStringLiteral("PIT-LFO1"), QStringLiteral("PIT-LFO2"),
        QStringLiteral("TVF-LFO1"), QStringLiteral("TVF-LFO2"),
        QStringLiteral("TVA-LFO1"), QStringLiteral("TVA-LFO2"),
        QStringLiteral("PAN-LFO1"), QStringLiteral("PAN-LFO2"),
        QStringLiteral("LFO1-RATE"), QStringLiteral("LFO2-RATE"),
        QStringLiteral("PIT-ATK"), QStringLiteral("PIT-DCY"), QStringLiteral("PIT-REL"),
        QStringLiteral("TVF-ATK"), QStringLiteral("TVF-DCY"), QStringLiteral("TVF-REL"),
        QStringLiteral("TVA-ATK"), QStringLiteral("TVA-DCY"), QStringLiteral("TVA-REL"),
        QStringLiteral("PMT"), QStringLiteral("FXM"),
        QStringLiteral("---"), QStringLiteral("---"), QStringLiteral("---"),
        QStringLiteral("---")
    };
}

void PcmSynthToneModel::setError(const QString &e)
{
    if (m_lastError == e)
        return;
    m_lastError = e;
    emit lastErrorChanged();
}

void PcmSynthToneModel::setSelectedPartial(int v)
{
    v = std::clamp(v, 0, 3);
    if (m_selectedPartial == v)
        return;
    m_selectedPartial = v;
    emit selectedPartialChanged();
    emit pcmChanged();
}

int PcmSynthToneModel::partialByte(int offset) const
{
    const auto &raw = m_partials.at(static_cast<size_t>(m_selectedPartial)).raw;
    if (offset < 0 || offset >= raw.size())
        return 0;
    return static_cast<quint8>(raw.at(offset));
}

void PcmSynthToneModel::writeCommonBytes(quint8 offset, const QByteArray &data)
{
    if (m_fromDevice || !m_platform || !m_platform->isConnected() || data.isEmpty())
        return;
    QString err;
    if (!m_platform->writeToneParameter(m_partIndex, ToneEngine::PcmSynth, InstrumentPlatform::ToneSection::PcmCommon, 0, offset, data, &err))
        setError(err);
}

void PcmSynthToneModel::writePmtBytes(quint8 offset, const QByteArray &data)
{
    if (m_fromDevice || !m_platform || !m_platform->isConnected() || data.isEmpty())
        return;
    QString err;
    if (!m_platform->writeToneParameter(m_partIndex, ToneEngine::PcmSynth, InstrumentPlatform::ToneSection::PcmPmt, 0, offset, data, &err))
        setError(err);
}

void PcmSynthToneModel::writePartialBytes(int partial, int offset, const QByteArray &data)
{
    if (m_fromDevice || !m_platform || !m_platform->isConnected() || data.isEmpty())
        return;
    QString err;
    if (!m_platform->writeToneParameter(m_partIndex, ToneEngine::PcmSynth, InstrumentPlatform::ToneSection::PcmPartial,
                                        partial, offset, data, &err))
        setError(err);
}

void PcmSynthToneModel::setPartialByte(int offset, int value, int lo, int hi)
{
    value = std::clamp(value, lo, hi);
    auto &raw = m_partials.at(static_cast<size_t>(m_selectedPartial)).raw;
    if (offset < 0 || offset >= raw.size())
        return;
    if (static_cast<quint8>(raw.at(offset)) == value)
        return;
    raw[offset] = static_cast<char>(value);
    emit pcmChanged();
    writePartialBytes(m_selectedPartial, offset, QByteArray(1, static_cast<char>(value & 0x7F)));
}

int PcmSynthToneModel::partialNibbleValue(int offset, int nibbleCount) const
{
    const auto &raw = m_partials.at(static_cast<size_t>(m_selectedPartial)).raw;
    return decodeNibbles(raw, offset, nibbleCount);
}

void PcmSynthToneModel::setPartialNibbleValue(int offset, int nibbleCount, int value, int lo, int hi)
{
    value = std::clamp(value, lo, hi);
    auto &raw = m_partials.at(static_cast<size_t>(m_selectedPartial)).raw;
    if (offset < 0 || offset + nibbleCount > raw.size())
        return;
    if (decodeNibbles(raw, offset, nibbleCount) == value)
        return;
    const auto encoded = encodeNibbles(value, nibbleCount);
    for (int i = 0; i < nibbleCount; ++i)
        raw[offset + i] = encoded.at(i);
    emit pcmChanged();
    writePartialBytes(m_selectedPartial, offset, encoded);
}

int PcmSynthToneModel::pmtByte(quint8 offset) const
{
    if (offset >= m_pmt.size())
        return 0;
    return static_cast<quint8>(m_pmt.at(offset));
}

void PcmSynthToneModel::setPmtByte(quint8 offset, int value, int lo, int hi)
{
    value = std::clamp(value, lo, hi);
    if (offset >= m_pmt.size())
        return;
    if (static_cast<quint8>(m_pmt.at(offset)) == value)
        return;
    m_pmt[offset] = static_cast<char>(value);
    emit pcmChanged();
    writePmtBytes(offset, QByteArray(1, static_cast<char>(value & 0x7F)));
}

int PcmSynthToneModel::commonByte(quint8 offset) const
{
    if (offset >= m_common.size())
        return 0;
    return static_cast<quint8>(m_common.at(offset));
}

void PcmSynthToneModel::setCommonByte(quint8 offset, int value, int lo, int hi)
{
    value = std::clamp(value, lo, hi);
    if (offset >= m_common.size())
        return;
    if (static_cast<quint8>(m_common.at(offset)) == value)
        return;
    m_common[offset] = static_cast<char>(value);
    emit pcmChanged();
    writeCommonBytes(offset, QByteArray(1, static_cast<char>(value & 0x7F)));
}

void PcmSynthToneModel::syncCommonFromRaw()
{
    m_toneName.clear();
    for (int i = 0; i < pcmSynthOff::CommonNameLength && i < m_common.size(); ++i) {
        const char c = m_common.at(i);
        if (c >= 32 && c < 127)
            m_toneName.append(QChar(c));
    }
    m_toneName = m_toneName.trimmed();
    m_toneLevel = m_common.size() > pcmSynthOff::CommonToneLevel
                      ? static_cast<quint8>(m_common.at(pcmSynthOff::CommonToneLevel))
                      : 100;
    m_tonePan = m_common.size() > pcmSynthOff::CommonTonePan
                    ? static_cast<quint8>(m_common.at(pcmSynthOff::CommonTonePan))
                    : 64;
}

void PcmSynthToneModel::setToneName(const QString &v)
{
    QString name = v.left(pcmSynthOff::CommonNameLength);
    while (name.size() < pcmSynthOff::CommonNameLength)
        name.append(QLatin1Char(' '));
    if (m_toneName == name.trimmed())
        return;
    m_toneName = name.trimmed();
    QByteArray chunk(pcmSynthOff::CommonNameLength, ' ');
    for (int i = 0; i < pcmSynthOff::CommonNameLength; ++i) {
        const char c = name.at(i).toLatin1();
        m_common[i] = c;
        chunk[i] = c;
    }
    writeCommonBytes(0, chunk);
    emit pcmChanged();
}

void PcmSynthToneModel::setToneLevel(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_toneLevel == v)
        return;
    m_toneLevel = v;
    m_common[pcmSynthOff::CommonToneLevel] = static_cast<char>(v);
    emit pcmChanged();
    writeCommonBytes(pcmSynthOff::CommonToneLevel, QByteArray(1, static_cast<char>(v)));
}

void PcmSynthToneModel::setTonePan(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_tonePan == v)
        return;
    m_tonePan = v;
    m_common[pcmSynthOff::CommonTonePan] = static_cast<char>(v);
    emit pcmChanged();
    writeCommonBytes(pcmSynthOff::CommonTonePan, QByteArray(1, static_cast<char>(v)));
}

int PcmSynthToneModel::structureType12() const { return pmtByte(pcmSynthOff::PmtStructure12); }
int PcmSynthToneModel::structureType34() const { return pmtByte(pcmSynthOff::PmtStructure34); }
bool PcmSynthToneModel::partial1On() const { return pmtByte(pcmSynthOff::PmtPartial1Switch) != 0; }
bool PcmSynthToneModel::partial2On() const { return pmtByte(pcmSynthOff::PmtPartial2Switch) != 0; }
bool PcmSynthToneModel::partial3On() const { return pmtByte(pcmSynthOff::PmtPartial3Switch) != 0; }
bool PcmSynthToneModel::partial4On() const { return pmtByte(pcmSynthOff::PmtPartial4Switch) != 0; }

void PcmSynthToneModel::setStructureType12(int v) { setPmtByte(pcmSynthOff::PmtStructure12, v, 0, 9); }
void PcmSynthToneModel::setStructureType34(int v) { setPmtByte(pcmSynthOff::PmtStructure34, v, 0, 9); }
void PcmSynthToneModel::setPartial1On(bool v) { setPmtByte(pcmSynthOff::PmtPartial1Switch, v ? 1 : 0, 0, 1); }
void PcmSynthToneModel::setPartial2On(bool v) { setPmtByte(pcmSynthOff::PmtPartial2Switch, v ? 1 : 0, 0, 1); }
void PcmSynthToneModel::setPartial3On(bool v) { setPmtByte(pcmSynthOff::PmtPartial3Switch, v ? 1 : 0, 0, 1); }
void PcmSynthToneModel::setPartial4On(bool v) { setPmtByte(pcmSynthOff::PmtPartial4Switch, v ? 1 : 0, 0, 1); }

int PcmSynthToneModel::waveGroupType() const { return partialByte(pcmSynthOff::WaveGroupType); }
int PcmSynthToneModel::waveGroupId() const { return partialNibbleValue(pcmSynthOff::WaveGroupId, 4); }
int PcmSynthToneModel::waveGroupBank() const
{
    const int type = waveGroupType();
    const int id = waveGroupId();
    if (type == 1)
        return 2; // SRX
    if (type == 0 && id == 2)
        return 1; // INT-B
    return 0; // INT-A (Type INT + ID 0/1, or default)
}
int PcmSynthToneModel::waveNumber() const { return partialNibbleValue(pcmSynthOff::WaveNumberL, 4); }
QString PcmSynthToneModel::waveName() const
{
    if (!m_waves)
        return {};
    return m_waves->displayPcm(waveGroupType(), waveGroupId(), waveNumber());
}
int PcmSynthToneModel::coarseTune() const { return partialByte(pcmSynthOff::PartialCoarseTune); }
int PcmSynthToneModel::fineTune() const { return partialByte(pcmSynthOff::PartialFineTune); }
int PcmSynthToneModel::filterType() const { return partialByte(pcmSynthOff::TvfFilterType); }
int PcmSynthToneModel::filterCutoff() const { return partialByte(pcmSynthOff::TvfCutoff); }
int PcmSynthToneModel::filterResonance() const { return partialByte(pcmSynthOff::TvfResonance); }
int PcmSynthToneModel::filterEnvAttack() const { return partialByte(pcmSynthOff::TvfEnvTime1); }
int PcmSynthToneModel::filterEnvDecay() const { return partialByte(pcmSynthOff::TvfEnvTime2); }
int PcmSynthToneModel::filterEnvSustain() const { return partialByte(pcmSynthOff::TvfEnvLevel3); }
int PcmSynthToneModel::filterEnvRelease() const { return partialByte(pcmSynthOff::TvfEnvTime4); }
int PcmSynthToneModel::ampLevel() const { return partialByte(pcmSynthOff::PartialLevel); }
int PcmSynthToneModel::ampPan() const { return partialByte(pcmSynthOff::PartialPan); }
int PcmSynthToneModel::ampEnvAttack() const { return partialByte(pcmSynthOff::TvaEnvTime1); }
int PcmSynthToneModel::ampEnvDecay() const { return partialByte(pcmSynthOff::TvaEnvTime2); }
int PcmSynthToneModel::ampEnvSustain() const { return partialByte(pcmSynthOff::TvaEnvLevel3); }
int PcmSynthToneModel::ampEnvRelease() const { return partialByte(pcmSynthOff::TvaEnvTime4); }
int PcmSynthToneModel::lfoWaveform() const { return partialByte(pcmSynthOff::Lfo1Waveform); }
int PcmSynthToneModel::lfoRate() const { return partialNibbleValue(pcmSynthOff::Lfo1Rate, 2); }
int PcmSynthToneModel::lfoPitchDepth() const { return partialByte(pcmSynthOff::Lfo1PitchDepth); }
int PcmSynthToneModel::lfoFilterDepth() const { return partialByte(pcmSynthOff::Lfo1TvfDepth); }
int PcmSynthToneModel::lfoAmpDepth() const { return partialByte(pcmSynthOff::Lfo1TvaDepth); }

void PcmSynthToneModel::setWaveGroupType(int v) { setPartialByte(pcmSynthOff::WaveGroupType, v, 0, 3); }
void PcmSynthToneModel::setWaveGroupId(int v) { setPartialNibbleValue(pcmSynthOff::WaveGroupId, 4, v, 0, 16384); }
void PcmSynthToneModel::setWaveGroupBank(int v)
{
    v = std::clamp(v, 0, 2);
    if (v == 2) {
        setWaveGroupType(1);
        if (waveGroupId() == 0)
            setWaveGroupId(1);
        return;
    }
    setWaveGroupType(0);
    setWaveGroupId(v == 1 ? 2 : 1);
}
void PcmSynthToneModel::setWaveNumber(int v) { setPartialNibbleValue(pcmSynthOff::WaveNumberL, 4, v, 0, 16384); }
void PcmSynthToneModel::setCoarseTune(int v) { setPartialByte(pcmSynthOff::PartialCoarseTune, v, 16, 112); }
void PcmSynthToneModel::setFineTune(int v) { setPartialByte(pcmSynthOff::PartialFineTune, v, 14, 114); }
void PcmSynthToneModel::setFilterType(int v) { setPartialByte(pcmSynthOff::TvfFilterType, v, 0, 6); }
void PcmSynthToneModel::setFilterCutoff(int v) { setPartialByte(pcmSynthOff::TvfCutoff, v, 0, 127); }
void PcmSynthToneModel::setFilterResonance(int v) { setPartialByte(pcmSynthOff::TvfResonance, v, 0, 127); }
void PcmSynthToneModel::setFilterEnvAttack(int v) { setPartialByte(pcmSynthOff::TvfEnvTime1, v, 0, 127); }
void PcmSynthToneModel::setFilterEnvDecay(int v) { setPartialByte(pcmSynthOff::TvfEnvTime2, v, 0, 127); }
void PcmSynthToneModel::setFilterEnvSustain(int v) { setPartialByte(pcmSynthOff::TvfEnvLevel3, v, 0, 127); }
void PcmSynthToneModel::setFilterEnvRelease(int v) { setPartialByte(pcmSynthOff::TvfEnvTime4, v, 0, 127); }
void PcmSynthToneModel::setAmpLevel(int v) { setPartialByte(pcmSynthOff::PartialLevel, v, 0, 127); }
void PcmSynthToneModel::setAmpPan(int v) { setPartialByte(pcmSynthOff::PartialPan, v, 0, 127); }
void PcmSynthToneModel::setAmpEnvAttack(int v) { setPartialByte(pcmSynthOff::TvaEnvTime1, v, 0, 127); }
void PcmSynthToneModel::setAmpEnvDecay(int v) { setPartialByte(pcmSynthOff::TvaEnvTime2, v, 0, 127); }
void PcmSynthToneModel::setAmpEnvSustain(int v) { setPartialByte(pcmSynthOff::TvaEnvLevel3, v, 0, 127); }
void PcmSynthToneModel::setAmpEnvRelease(int v) { setPartialByte(pcmSynthOff::TvaEnvTime4, v, 0, 127); }
void PcmSynthToneModel::setLfoWaveform(int v) { setPartialByte(pcmSynthOff::Lfo1Waveform, v, 0, 12); }
void PcmSynthToneModel::setLfoRate(int v) { setPartialNibbleValue(pcmSynthOff::Lfo1Rate, 2, v, 0, 149); }
void PcmSynthToneModel::setLfoPitchDepth(int v) { setPartialByte(pcmSynthOff::Lfo1PitchDepth, v, 1, 127); }
void PcmSynthToneModel::setLfoFilterDepth(int v) { setPartialByte(pcmSynthOff::Lfo1TvfDepth, v, 1, 127); }
void PcmSynthToneModel::setLfoAmpDepth(int v) { setPartialByte(pcmSynthOff::Lfo1TvaDepth, v, 1, 127); }

int PcmSynthToneModel::matrixSource(int index) const
{
    return commonByte(static_cast<quint8>(pcmSynthOff::MatrixControl1Source
                                          + index * pcmSynthOff::MatrixControlStride));
}

int PcmSynthToneModel::matrixDest(int index) const
{
    return commonByte(static_cast<quint8>(pcmSynthOff::MatrixControl1Source
                                          + index * pcmSynthOff::MatrixControlStride + 1));
}

int PcmSynthToneModel::matrixSens(int index) const
{
    return commonByte(static_cast<quint8>(pcmSynthOff::MatrixControl1Source
                                          + index * pcmSynthOff::MatrixControlStride + 2));
}

void PcmSynthToneModel::setMatrixSource(int index, int v)
{
    setCommonByte(static_cast<quint8>(pcmSynthOff::MatrixControl1Source
                                      + index * pcmSynthOff::MatrixControlStride),
                  v, 0, 109);
}

void PcmSynthToneModel::setMatrixDest(int index, int v)
{
    setCommonByte(static_cast<quint8>(pcmSynthOff::MatrixControl1Source
                                      + index * pcmSynthOff::MatrixControlStride + 1),
                  v, 0, 33);
}

void PcmSynthToneModel::setMatrixSens(int index, int v)
{
    setCommonByte(static_cast<quint8>(pcmSynthOff::MatrixControl1Source
                                      + index * pcmSynthOff::MatrixControlStride + 2),
                  v, 1, 127);
}

int PcmSynthToneModel::matrix1Source() const { return matrixSource(0); }
int PcmSynthToneModel::matrix1Dest() const { return matrixDest(0); }
int PcmSynthToneModel::matrix1Sens() const { return matrixSens(0); }
int PcmSynthToneModel::matrix2Source() const { return matrixSource(1); }
int PcmSynthToneModel::matrix2Dest() const { return matrixDest(1); }
int PcmSynthToneModel::matrix2Sens() const { return matrixSens(1); }
int PcmSynthToneModel::matrix3Source() const { return matrixSource(2); }
int PcmSynthToneModel::matrix3Dest() const { return matrixDest(2); }
int PcmSynthToneModel::matrix3Sens() const { return matrixSens(2); }
int PcmSynthToneModel::matrix4Source() const { return matrixSource(3); }
int PcmSynthToneModel::matrix4Dest() const { return matrixDest(3); }
int PcmSynthToneModel::matrix4Sens() const { return matrixSens(3); }

void PcmSynthToneModel::setMatrix1Source(int v) { setMatrixSource(0, v); }
void PcmSynthToneModel::setMatrix1Dest(int v) { setMatrixDest(0, v); }
void PcmSynthToneModel::setMatrix1Sens(int v) { setMatrixSens(0, v); }
void PcmSynthToneModel::setMatrix2Source(int v) { setMatrixSource(1, v); }
void PcmSynthToneModel::setMatrix2Dest(int v) { setMatrixDest(1, v); }
void PcmSynthToneModel::setMatrix2Sens(int v) { setMatrixSens(1, v); }
void PcmSynthToneModel::setMatrix3Source(int v) { setMatrixSource(2, v); }
void PcmSynthToneModel::setMatrix3Dest(int v) { setMatrixDest(2, v); }
void PcmSynthToneModel::setMatrix3Sens(int v) { setMatrixSens(2, v); }
void PcmSynthToneModel::setMatrix4Source(int v) { setMatrixSource(3, v); }
void PcmSynthToneModel::setMatrix4Dest(int v) { setMatrixDest(3, v); }
void PcmSynthToneModel::setMatrix4Sens(int v) { setMatrixSens(3, v); }

bool PcmSynthToneModel::pullFromDevice()
{
    if (!m_platform || !m_platform->isConnected()) {
        setError(QStringLiteral("Not connected"));
        return false;
    }
    QString err;
    QByteArray common, pmt, common2;
    std::array<QByteArray, 4> partials;
    if (!m_platform->readToneSection(m_partIndex, ToneEngine::PcmSynth, InstrumentPlatform::ToneSection::PcmCommon, 0, pcmSynthOff::CommonSize, &common, &err)
        || !m_platform->readToneSection(m_partIndex, ToneEngine::PcmSynth, InstrumentPlatform::ToneSection::PcmPmt, 0, pcmSynthOff::PmtSize, &pmt, &err)
        || !m_platform->readToneSection(m_partIndex, ToneEngine::PcmSynth, InstrumentPlatform::ToneSection::PcmCommon2, 0, pcmSynthOff::Common2Size, &common2, &err)) {
        setError(err);
        return false;
    }
    for (int i = 0; i < 4; ++i) {
        if (!m_platform->readToneSection(m_partIndex, ToneEngine::PcmSynth, InstrumentPlatform::ToneSection::PcmPartial,
                                         i, pcmSynthOff::PartialSize, &partials[static_cast<size_t>(i)], &err)) {
            setError(err);
            return false;
        }
    }
    m_fromDevice = true;
    m_common = common;
    m_pmt = pmt;
    m_common2 = common2;
    for (int i = 0; i < 4; ++i)
        m_partials[static_cast<size_t>(i)].raw = partials[static_cast<size_t>(i)];
    syncCommonFromRaw();
    m_fromDevice = false;
    setError({});
    emit pcmChanged();
    return true;
}

bool PcmSynthToneModel::pushToDevice()
{
    if (!m_platform || !m_platform->isConnected()) {
        setError(QStringLiteral("Not connected"));
        return false;
    }
    QString err;
    if (!m_platform->writeToneSection(m_partIndex, ToneEngine::PcmSynth, InstrumentPlatform::ToneSection::PcmCommon, 0, m_common, &err)
        || !m_platform->writeToneSection(m_partIndex, ToneEngine::PcmSynth, InstrumentPlatform::ToneSection::PcmPmt, 0, m_pmt, &err)
        || !m_platform->writeToneSection(m_partIndex, ToneEngine::PcmSynth, InstrumentPlatform::ToneSection::PcmCommon2, 0, m_common2, &err)) {
        setError(err);
        return false;
    }
    for (int i = 0; i < 4; ++i) {
        if (!m_platform->writeToneSection(m_partIndex, ToneEngine::PcmSynth, InstrumentPlatform::ToneSection::PcmPartial,
                                          i, m_partials[static_cast<size_t>(i)].raw, &err)) {
            setError(err);
            return false;
        }
    }
    setError({});
    return true;
}

QJsonObject PcmSynthToneModel::toJson() const
{
    QJsonObject o;
    o.insert(QStringLiteral("common"), QString::fromLatin1(m_common.toBase64()));
    o.insert(QStringLiteral("pmt"), QString::fromLatin1(m_pmt.toBase64()));
    o.insert(QStringLiteral("common2"), QString::fromLatin1(m_common2.toBase64()));
    QJsonArray partials;
    for (const auto &p : m_partials)
        partials.append(QString::fromLatin1(p.raw.toBase64()));
    o.insert(QStringLiteral("partials"), partials);
    return o;
}

bool PcmSynthToneModel::fromJson(const QJsonObject &obj)
{
    if (obj.isEmpty())
        return false;
    m_fromDevice = true;
    m_common = QByteArray::fromBase64(obj.value(QStringLiteral("common")).toString().toLatin1());
    m_pmt = QByteArray::fromBase64(obj.value(QStringLiteral("pmt")).toString().toLatin1());
    m_common2 = QByteArray::fromBase64(obj.value(QStringLiteral("common2")).toString().toLatin1());
    if (m_common.size() < pcmSynthOff::CommonSize)
        m_common.resize(pcmSynthOff::CommonSize);
    if (m_pmt.size() < pcmSynthOff::PmtSize)
        m_pmt.resize(pcmSynthOff::PmtSize);
    if (m_common2.size() < pcmSynthOff::Common2Size)
        m_common2.resize(pcmSynthOff::Common2Size);
    const auto partials = obj.value(QStringLiteral("partials")).toArray();
    for (int i = 0; i < 4; ++i) {
        QByteArray raw;
        if (i < partials.size())
            raw = QByteArray::fromBase64(partials.at(i).toString().toLatin1());
        if (raw.size() < pcmSynthOff::PartialSize)
            raw.resize(pcmSynthOff::PartialSize);
        m_partials[static_cast<size_t>(i)].raw = raw;
    }
    syncCommonFromRaw();
    m_fromDevice = false;
    emit pcmChanged();
    return true;
}
