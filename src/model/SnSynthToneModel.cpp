#include "model/SnSynthToneModel.h"
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

SnSynthToneModel::SnSynthToneModel(InstrumentPlatform *platform, QObject *parent)
    : QObject(parent)
    , m_platform(platform)
{
    m_common = QByteArray(snSynthOff::CommonSize, '\0');
    m_misc = QByteArray(snSynthOff::MiscSize, '\0');
    for (auto &p : m_partials)
        p.raw = QByteArray(snSynthOff::PartialSize, '\0');
    loadInitTemplate();
}

void SnSynthToneModel::setWaveformCatalog(WaveformCatalog *catalog)
{
    if (m_waves == catalog)
        return;
    if (m_waves)
        disconnect(m_waves, nullptr, this, nullptr);
    m_waves = catalog;
    if (m_waves) {
        connect(m_waves, &WaveformCatalog::catalogChanged, this, [this]() {
            emit snChanged();
        });
    }
    emit snChanged();
}

QString SnSynthToneModel::partialDisplayName(int index) const
{
    if (index < 0 || index > 2)
        return {};
    return QStringLiteral("Partial %1").arg(index + 1);
}

bool SnSynthToneModel::partialEnabled(int index) const
{
    if (index < 0 || index >= snSynthOff::PartialCount)
        return false;
    const int offset = snSynthOff::CommonPartial1Switch + index * 2;
    return offset < m_common.size() && m_common.at(offset) != 0;
}

void SnSynthToneModel::setPartialEnabled(int index, bool enabled)
{
    if (index < 0 || index >= snSynthOff::PartialCount)
        return;
    const int offset = snSynthOff::CommonPartial1Switch + index * 2;
    const char value = enabled ? 1 : 0;
    if (offset >= m_common.size() || m_common.at(offset) == value)
        return;
    m_common[offset] = value;
    emit snChanged();
    writeCommonByte(static_cast<quint8>(offset), value);
}

QStringList SnSynthToneModel::oscWaveNames() const
{
    return {
        QStringLiteral("SAW"), QStringLiteral("SQR"), QStringLiteral("PW-SQR"),
        QStringLiteral("TRI"), QStringLiteral("SINE"), QStringLiteral("NOISE"),
        QStringLiteral("SUPER-SAW"), QStringLiteral("PCM")
    };
}

QStringList SnSynthToneModel::oscWaveGainNames() const
{
    return {
        QStringLiteral("-6 dB"), QStringLiteral("0 dB"),
        QStringLiteral("+6 dB"), QStringLiteral("+12 dB")
    };
}

QStringList SnSynthToneModel::filterModeNames() const
{
    return {
        QStringLiteral("BYPASS"), QStringLiteral("LPF"), QStringLiteral("HPF"),
        QStringLiteral("BPF"), QStringLiteral("PKG"), QStringLiteral("LPF2"),
        QStringLiteral("LPF3"), QStringLiteral("LPF4")
    };
}

QStringList SnSynthToneModel::lfoShapeNames() const
{
    return {
        QStringLiteral("TRI"), QStringLiteral("SIN"), QStringLiteral("SAW"),
        QStringLiteral("SQR"), QStringLiteral("S&H"), QStringLiteral("RND")
    };
}

void SnSynthToneModel::setError(const QString &e)
{
    if (m_lastError == e)
        return;
    m_lastError = e;
    emit lastErrorChanged();
}

void SnSynthToneModel::setSelectedPartial(int v)
{
    v = std::clamp(v, 0, 2);
    if (m_selectedPartial == v)
        return;
    m_selectedPartial = v;
    emit selectedPartialChanged();
    emit snChanged();
}

int SnSynthToneModel::partialByte(quint8 offset) const
{
    const auto &raw = m_partials.at(static_cast<size_t>(m_selectedPartial)).raw;
    if (offset >= raw.size())
        return 0;
    return static_cast<quint8>(raw.at(offset));
}

void SnSynthToneModel::writeCommonByte(quint8 offset, int value)
{
    if (m_fromDevice || !m_platform || !m_platform->isConnected())
        return;
    QByteArray d(1, static_cast<char>(value & 0x7F));
    QString err;
    if (!m_platform->writeToneParameter(m_partIndex, ToneEngine::SnSynth,
                                        InstrumentPlatform::ToneSection::SnCommon, 0, offset, d, &err))
        setError(err);
}

void SnSynthToneModel::writePartialByte(int partial, quint8 offset, int value)
{
    writePartialBytes(partial, offset, QByteArray(1, static_cast<char>(value & 0x7F)));
}

void SnSynthToneModel::writePartialBytes(int partial, int offset, const QByteArray &data)
{
    if (m_fromDevice || !m_platform || !m_platform->isConnected() || data.isEmpty())
        return;
    QString err;
    if (!m_platform->writeToneParameter(m_partIndex, ToneEngine::SnSynth,
                                        InstrumentPlatform::ToneSection::SnPartial,
                                        partial, offset, data, &err))
        setError(err);
}

void SnSynthToneModel::setPartialByte(quint8 offset, int value, int lo, int hi)
{
    value = std::clamp(value, lo, hi);
    auto &raw = m_partials.at(static_cast<size_t>(m_selectedPartial)).raw;
    if (offset >= raw.size())
        return;
    if (static_cast<quint8>(raw.at(offset)) == value)
        return;
    raw[offset] = static_cast<char>(value);
    emit snChanged();
    writePartialByte(m_selectedPartial, offset, value);
}

int SnSynthToneModel::partialNibbleValue(int offset, int nibbleCount) const
{
    const auto &raw = m_partials.at(static_cast<size_t>(m_selectedPartial)).raw;
    return decodeNibbles(raw, offset, nibbleCount);
}

void SnSynthToneModel::setPartialNibbleValue(int offset, int nibbleCount, int value, int lo, int hi)
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
    emit snChanged();
    writePartialBytes(m_selectedPartial, offset, encoded);
}

void SnSynthToneModel::syncCommonFromRaw()
{
    m_toneName.clear();
    for (int i = 0; i < snSynthOff::CommonNameLength && i < m_common.size(); ++i) {
        const char c = m_common.at(i);
        if (c >= 32 && c < 127)
            m_toneName.append(QChar(c));
    }
    m_toneName = m_toneName.trimmed();
    m_toneLevel = m_common.size() > snSynthOff::CommonToneLevel
                      ? static_cast<quint8>(m_common.at(snSynthOff::CommonToneLevel))
                      : 100;
    m_monoSwitch = m_common.size() > snSynthOff::CommonMonoSwitch
                   && m_common.at(snSynthOff::CommonMonoSwitch) != 0;
}

void SnSynthToneModel::setToneName(const QString &v)
{
    QString name = v.left(snSynthOff::CommonNameLength);
    while (name.size() < snSynthOff::CommonNameLength)
        name.append(QLatin1Char(' '));
    if (m_toneName == name.trimmed())
        return;
    m_toneName = name.trimmed();
    for (int i = 0; i < snSynthOff::CommonNameLength; ++i) {
        const char c = name.at(i).toLatin1();
        m_common[i] = c;
        writeCommonByte(static_cast<quint8>(i), static_cast<quint8>(c));
    }
    emit snChanged();
}

void SnSynthToneModel::setToneLevel(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_toneLevel == v)
        return;
    m_toneLevel = v;
    m_common[snSynthOff::CommonToneLevel] = static_cast<char>(v);
    emit snChanged();
    writeCommonByte(snSynthOff::CommonToneLevel, v);
}

void SnSynthToneModel::setMonoSwitch(bool v)
{
    if (m_monoSwitch == v)
        return;
    m_monoSwitch = v;
    m_common[snSynthOff::CommonMonoSwitch] = static_cast<char>(v ? 1 : 0);
    emit snChanged();
    writeCommonByte(snSynthOff::CommonMonoSwitch, v ? 1 : 0);
}

int SnSynthToneModel::oscWave() const { return partialByte(snSynthOff::OscWave); }
int SnSynthToneModel::oscWaveVariation() const { return partialByte(snSynthOff::OscWaveVariation); }
int SnSynthToneModel::oscPitch() const { return partialByte(snSynthOff::OscPitch); }
int SnSynthToneModel::oscDetune() const { return partialByte(snSynthOff::OscDetune); }
int SnSynthToneModel::oscPulseWidth() const { return partialByte(snSynthOff::OscPulseWidth); }
bool SnSynthToneModel::oscIsPcmWave() const { return oscWave() == 7; }
int SnSynthToneModel::oscWaveNumber() const
{
    return partialNibbleValue(snSynthOff::WaveNumberL, 4);
}
QString SnSynthToneModel::oscWaveName() const
{
    if (!m_waves)
        return oscWaveNumber() <= 0 ? QStringLiteral("OFF")
                                    : QStringLiteral("#%1").arg(oscWaveNumber());
    return m_waves->displaySn(oscWaveNumber());
}
int SnSynthToneModel::oscWaveGain() const { return partialByte(snSynthOff::WaveGain); }
int SnSynthToneModel::filterMode() const { return partialByte(snSynthOff::FilterMode); }
int SnSynthToneModel::filterCutoff() const { return partialByte(snSynthOff::FilterCutoff); }
int SnSynthToneModel::filterResonance() const { return partialByte(snSynthOff::FilterResonance); }
int SnSynthToneModel::filterEnvAttack() const { return partialByte(snSynthOff::FilterEnvAttack); }
int SnSynthToneModel::filterEnvDecay() const { return partialByte(snSynthOff::FilterEnvDecay); }
int SnSynthToneModel::filterEnvSustain() const { return partialByte(snSynthOff::FilterEnvSustain); }
int SnSynthToneModel::filterEnvRelease() const { return partialByte(snSynthOff::FilterEnvRelease); }
int SnSynthToneModel::ampLevel() const { return partialByte(snSynthOff::AmpLevel); }
int SnSynthToneModel::ampEnvAttack() const { return partialByte(snSynthOff::AmpEnvAttack); }
int SnSynthToneModel::ampEnvDecay() const { return partialByte(snSynthOff::AmpEnvDecay); }
int SnSynthToneModel::ampEnvSustain() const { return partialByte(snSynthOff::AmpEnvSustain); }
int SnSynthToneModel::ampEnvRelease() const { return partialByte(snSynthOff::AmpEnvRelease); }
int SnSynthToneModel::ampPan() const { return partialByte(snSynthOff::AmpPan); }
int SnSynthToneModel::lfoShape() const { return partialByte(snSynthOff::LfoShape); }
int SnSynthToneModel::lfoRate() const { return partialByte(snSynthOff::LfoRate); }
int SnSynthToneModel::lfoPitchDepth() const { return partialByte(snSynthOff::LfoPitchDepth); }
int SnSynthToneModel::lfoFilterDepth() const { return partialByte(snSynthOff::LfoFilterDepth); }
int SnSynthToneModel::lfoAmpDepth() const { return partialByte(snSynthOff::LfoAmpDepth); }

void SnSynthToneModel::setOscWave(int v) { setPartialByte(snSynthOff::OscWave, v, 0, 7); }
void SnSynthToneModel::setOscWaveVariation(int v) { setPartialByte(snSynthOff::OscWaveVariation, v, 0, 2); }
void SnSynthToneModel::setOscPitch(int v) { setPartialByte(snSynthOff::OscPitch, v, 40, 88); }
void SnSynthToneModel::setOscDetune(int v) { setPartialByte(snSynthOff::OscDetune, v, 14, 114); }
void SnSynthToneModel::setOscPulseWidth(int v) { setPartialByte(snSynthOff::OscPulseWidth, v, 0, 127); }
void SnSynthToneModel::setOscWaveNumber(int v)
{
    setPartialNibbleValue(snSynthOff::WaveNumberL, 4, v, 0, 16384);
}
void SnSynthToneModel::setOscWaveGain(int v) { setPartialByte(snSynthOff::WaveGain, v, 0, 3); }
void SnSynthToneModel::setFilterMode(int v) { setPartialByte(snSynthOff::FilterMode, v, 0, 7); }
void SnSynthToneModel::setFilterCutoff(int v) { setPartialByte(snSynthOff::FilterCutoff, v, 0, 127); }
void SnSynthToneModel::setFilterResonance(int v) { setPartialByte(snSynthOff::FilterResonance, v, 0, 127); }
void SnSynthToneModel::setFilterEnvAttack(int v) { setPartialByte(snSynthOff::FilterEnvAttack, v, 0, 127); }
void SnSynthToneModel::setFilterEnvDecay(int v) { setPartialByte(snSynthOff::FilterEnvDecay, v, 0, 127); }
void SnSynthToneModel::setFilterEnvSustain(int v) { setPartialByte(snSynthOff::FilterEnvSustain, v, 0, 127); }
void SnSynthToneModel::setFilterEnvRelease(int v) { setPartialByte(snSynthOff::FilterEnvRelease, v, 0, 127); }
void SnSynthToneModel::setAmpLevel(int v) { setPartialByte(snSynthOff::AmpLevel, v, 0, 127); }
void SnSynthToneModel::setAmpEnvAttack(int v) { setPartialByte(snSynthOff::AmpEnvAttack, v, 0, 127); }
void SnSynthToneModel::setAmpEnvDecay(int v) { setPartialByte(snSynthOff::AmpEnvDecay, v, 0, 127); }
void SnSynthToneModel::setAmpEnvSustain(int v) { setPartialByte(snSynthOff::AmpEnvSustain, v, 0, 127); }
void SnSynthToneModel::setAmpEnvRelease(int v) { setPartialByte(snSynthOff::AmpEnvRelease, v, 0, 127); }
void SnSynthToneModel::setAmpPan(int v) { setPartialByte(snSynthOff::AmpPan, v, 0, 127); }
void SnSynthToneModel::setLfoShape(int v) { setPartialByte(snSynthOff::LfoShape, v, 0, 5); }
void SnSynthToneModel::setLfoRate(int v) { setPartialByte(snSynthOff::LfoRate, v, 0, 127); }
void SnSynthToneModel::setLfoPitchDepth(int v) { setPartialByte(snSynthOff::LfoPitchDepth, v, 1, 127); }
void SnSynthToneModel::setLfoFilterDepth(int v) { setPartialByte(snSynthOff::LfoFilterDepth, v, 1, 127); }
void SnSynthToneModel::setLfoAmpDepth(int v) { setPartialByte(snSynthOff::LfoAmpDepth, v, 1, 127); }

void SnSynthToneModel::loadInitTemplate()
{
    m_fromDevice = true;
    m_common.fill('\0');
    m_misc.fill('\0');
    const QByteArray name = QByteArrayLiteral("Init SN-S   ");
    for (int i = 0; i < snSynthOff::CommonNameLength; ++i)
        m_common[i] = name.at(i);
    m_common[snSynthOff::CommonToneLevel] = 100;
    m_common[snSynthOff::CommonPartial1Switch] = 1;
    m_common[snSynthOff::CommonPartial1Switch + 1] = 1; // select
    m_common[snSynthOff::CommonMfxSwitch] = 0;

    for (int p = 0; p < 3; ++p) {
        auto &raw = m_partials[static_cast<size_t>(p)].raw;
        raw.fill('\0');
        raw[snSynthOff::OscWave] = 0; // SAW
        raw[snSynthOff::OscPitch] = 64; // 0
        raw[snSynthOff::OscDetune] = 64;
        raw[snSynthOff::OscPulseWidth] = 64;
        raw[snSynthOff::WaveGain] = 1; // 0 dB
        // Wave Number OFF (0) via 4 zero nibbles at WaveNumberL
        raw[snSynthOff::FilterMode] = 1; // LPF
        raw[snSynthOff::FilterCutoff] = 100;
        raw[snSynthOff::FilterResonance] = 0;
        raw[snSynthOff::FilterEnvAttack] = 0;
        raw[snSynthOff::FilterEnvDecay] = 40;
        raw[snSynthOff::FilterEnvSustain] = 80;
        raw[snSynthOff::FilterEnvRelease] = 40;
        raw[snSynthOff::AmpLevel] = (p == 0) ? 100 : 0;
        raw[snSynthOff::AmpEnvAttack] = 0;
        raw[snSynthOff::AmpEnvDecay] = 40;
        raw[snSynthOff::AmpEnvSustain] = 100;
        raw[snSynthOff::AmpEnvRelease] = 40;
        raw[snSynthOff::AmpPan] = 64;
        raw[snSynthOff::LfoShape] = 1; // SIN
        raw[snSynthOff::LfoRate] = 60;
        raw[snSynthOff::LfoPitchDepth] = 64;
        raw[snSynthOff::LfoFilterDepth] = 64;
        raw[snSynthOff::LfoAmpDepth] = 64;
    }
    syncCommonFromRaw();
    m_fromDevice = false;
    emit snChanged();
}

bool SnSynthToneModel::pullFromDevice()
{
    if (!m_platform || !m_platform->isConnected()) {
        setError(QStringLiteral("Not connected"));
        return false;
    }
    QString err;
    QByteArray common, misc;
    std::array<QByteArray, 3> partials;
    if (!m_platform->readToneSection(m_partIndex, ToneEngine::SnSynth, InstrumentPlatform::ToneSection::SnCommon,
                                     0, snSynthOff::CommonSize, &common, &err)
        || !m_platform->readToneSection(m_partIndex, ToneEngine::SnSynth, InstrumentPlatform::ToneSection::SnMisc,
                                        0, snSynthOff::MiscSize, &misc, &err)) {
        setError(err);
        return false;
    }
    for (int i = 0; i < 3; ++i) {
        if (!m_platform->readToneSection(m_partIndex, ToneEngine::SnSynth, InstrumentPlatform::ToneSection::SnPartial,
                                         i, snSynthOff::PartialSize, &partials[static_cast<size_t>(i)], &err)) {
            setError(err);
            return false;
        }
    }
    m_fromDevice = true;
    m_common = common;
    m_misc = misc;
    for (int i = 0; i < 3; ++i)
        m_partials[static_cast<size_t>(i)].raw = partials[static_cast<size_t>(i)];
    syncCommonFromRaw();
    m_fromDevice = false;
    setError({});
    emit snChanged();
    return true;
}

bool SnSynthToneModel::pushToDevice()
{
    if (!m_platform || !m_platform->isConnected()) {
        setError(QStringLiteral("Not connected"));
        return false;
    }
    QString err;
    if (!m_platform->writeToneSection(m_partIndex, ToneEngine::SnSynth, InstrumentPlatform::ToneSection::SnCommon,
                                      0, m_common, &err)
        || !m_platform->writeToneSection(m_partIndex, ToneEngine::SnSynth, InstrumentPlatform::ToneSection::SnMisc,
                                         0, m_misc, &err)) {
        setError(err);
        return false;
    }
    for (int i = 0; i < 3; ++i) {
        if (!m_platform->writeToneSection(m_partIndex, ToneEngine::SnSynth, InstrumentPlatform::ToneSection::SnPartial,
                                          i, m_partials[static_cast<size_t>(i)].raw, &err)) {
            setError(err);
            return false;
        }
    }
    setError({});
    return true;
}

QJsonObject SnSynthToneModel::toJson() const
{
    QJsonObject o;
    o.insert(QStringLiteral("common"), QString::fromLatin1(m_common.toBase64()));
    o.insert(QStringLiteral("misc"), QString::fromLatin1(m_misc.toBase64()));
    QJsonArray partials;
    for (const auto &p : m_partials)
        partials.append(QString::fromLatin1(p.raw.toBase64()));
    o.insert(QStringLiteral("partials"), partials);
    return o;
}

bool SnSynthToneModel::fromJson(const QJsonObject &obj)
{
    if (obj.isEmpty())
        return false;
    m_fromDevice = true;
    m_common = QByteArray::fromBase64(obj.value(QStringLiteral("common")).toString().toLatin1());
    m_misc = QByteArray::fromBase64(obj.value(QStringLiteral("misc")).toString().toLatin1());
    if (m_common.size() < snSynthOff::CommonSize)
        m_common.resize(snSynthOff::CommonSize);
    if (m_misc.size() < snSynthOff::MiscSize)
        m_misc.resize(snSynthOff::MiscSize);
    const auto partials = obj.value(QStringLiteral("partials")).toArray();
    for (int i = 0; i < 3; ++i) {
        QByteArray raw;
        if (i < partials.size())
            raw = QByteArray::fromBase64(partials.at(i).toString().toLatin1());
        if (raw.size() < snSynthOff::PartialSize)
            raw.resize(snSynthOff::PartialSize);
        m_partials[static_cast<size_t>(i)].raw = raw;
    }
    syncCommonFromRaw();
    m_fromDevice = false;
    emit snChanged();
    return true;
}
