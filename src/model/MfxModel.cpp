#include "model/MfxModel.h"
#include "model/MfxParamCatalog.h"
#include "model/MfxUiHelpers.h"
#include "midi/SysexEngine.h"

#include <QJsonArray>
#include <QVariantMap>
#include <algorithm>

using namespace roland;

MfxModel::MfxModel(SysexEngine *engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
{
    m_raw = QByteArray(mfxOff::MfxSize, '\0');
    MfxParamCatalog::ensureLoaded();
    m_tapDelay = new MfxTapDelayModel(this, this);
    m_paramList = new MfxParamListModel(this, this);
}

QStringList MfxModel::typeNames() const
{
    // Parameter Guide MFX Parameters (types 0–68).
    return {
        QStringLiteral("00 Thru"),
        QStringLiteral("01 Equalizer"),
        QStringLiteral("02 Spectrum"),
        QStringLiteral("03 Low Boost"),
        QStringLiteral("04 Step Filter"),
        QStringLiteral("05 Enhancer"),
        QStringLiteral("06 Auto Wah"),
        QStringLiteral("07 Humanizer"),
        QStringLiteral("08 Speaker Simulator"),
        QStringLiteral("09 Phaser 1"),
        QStringLiteral("10 Phaser 2"),
        QStringLiteral("11 Phaser 3"),
        QStringLiteral("12 Step Phaser"),
        QStringLiteral("13 Multi Stage Phaser"),
        QStringLiteral("14 Infinite Phaser"),
        QStringLiteral("15 Ring Modulator"),
        QStringLiteral("16 Tremolo"),
        QStringLiteral("17 Auto Pan"),
        QStringLiteral("18 Slicer"),
        QStringLiteral("19 Rotary 1"),
        QStringLiteral("20 Rotary 2"),
        QStringLiteral("21 Rotary 3"),
        QStringLiteral("22 Chorus"),
        QStringLiteral("23 Flanger"),
        QStringLiteral("24 Step Flanger"),
        QStringLiteral("25 Hexa-Chorus"),
        QStringLiteral("26 Tremolo Chorus"),
        QStringLiteral("27 Space-D"),
        QStringLiteral("28 Overdrive"),
        QStringLiteral("29 Distortion"),
        QStringLiteral("30 Guitar Amp Simulator"),
        QStringLiteral("31 Compressor"),
        QStringLiteral("32 Limiter"),
        QStringLiteral("33 Gate"),
        QStringLiteral("34 Delay"),
        QStringLiteral("35 Modulation Delay"),
        QStringLiteral("36 3Tap Pan Delay"),
        QStringLiteral("37 4Tap Pan Delay"),
        QStringLiteral("38 Multi Tap Delay"),
        QStringLiteral("39 Reverse Delay"),
        QStringLiteral("40 Time Ctrl Delay"),
        QStringLiteral("41 LOFI Compress"),
        QStringLiteral("42 Bit Crusher"),
        QStringLiteral("43 Pitch Shifter"),
        QStringLiteral("44 2Voice Pitch Shifter"),
        QStringLiteral("45 Overdrive → Chorus"),
        QStringLiteral("46 Overdrive → Flanger"),
        QStringLiteral("47 Overdrive → Delay"),
        QStringLiteral("48 Distortion → Chorus"),
        QStringLiteral("49 Distortion → Flanger"),
        QStringLiteral("50 Distortion → Delay"),
        QStringLiteral("51 OD/DS → TouchWah"),
        QStringLiteral("52 OD/DS → AutoWah"),
        QStringLiteral("53 GuitarAmpSim → Chorus"),
        QStringLiteral("54 GuitarAmpSim → Flanger"),
        QStringLiteral("55 GuitarAmpSim → Phaser"),
        QStringLiteral("56 GuitarAmpSim → Delay"),
        QStringLiteral("57 EP AmpSim → Tremolo"),
        QStringLiteral("58 EP AmpSim → Chorus"),
        QStringLiteral("59 EP AmpSim → Flanger"),
        QStringLiteral("60 EP AmpSim → Phaser"),
        QStringLiteral("61 EP AmpSim → Delay"),
        QStringLiteral("62 Enhancer → Chorus"),
        QStringLiteral("63 Enhancer → Flanger"),
        QStringLiteral("64 Enhancer → Delay"),
        QStringLiteral("65 Chorus → Delay"),
        QStringLiteral("66 Flanger → Delay"),
        QStringLiteral("67 Chorus → Flanger"),
        QStringLiteral("68 Vocoder")
    };
}

QString MfxModel::typeName() const
{
    const auto names = typeNames();
    if (m_type >= 0 && m_type < names.size())
        return names.at(m_type);
    return QStringLiteral("?");
}

int MfxModel::paramCount() const
{
    const int n = MfxParamCatalog::paramCountForType(m_type);
    // Unknown / missing catalog entry: keep previous 8-slot UI affordance.
    if (n == 0 && m_type != 0)
        return 8;
    return n;
}

QVariantList MfxModel::paramCatalog() const
{
    QVariantList list;
    const auto entries = MfxParamCatalog::paramsForType(m_type);
    if (entries.isEmpty() && m_type != 0) {
        list.reserve(8);
        for (int i = 0; i < 8; ++i) {
            QVariantMap m;
            m.insert(QStringLiteral("name"), QStringLiteral("Parameter %1").arg(i + 1));
            m.insert(QStringLiteral("min"), MfxParamCatalog::DefaultMin);
            m.insert(QStringLiteral("max"), MfxParamCatalog::DefaultMax);
            list.append(m);
        }
        return list;
    }
    list.reserve(entries.size());
    for (const auto &s : entries) {
        QVariantMap m;
        m.insert(QStringLiteral("name"), s.name);
        m.insert(QStringLiteral("min"), s.minValue);
        m.insert(QStringLiteral("max"), s.maxValue);
        if (!s.enumNames.isEmpty())
            m.insert(QStringLiteral("enum"), s.enumNames);
        list.append(m);
    }
    return list;
}

QVariantList MfxModel::paramValues() const
{
    QVariantList list;
    list.reserve(mfxOff::ParamCount);
    for (int i = 0; i < mfxOff::ParamCount; ++i)
        list.append(paramValue(i));
    return list;
}

QVariantList MfxModel::paramMinValues() const
{
    const int n = paramCount();
    QVariantList list;
    list.reserve(n);
    for (int i = 0; i < n; ++i)
        list.append(paramMin(i));
    return list;
}

QVariantList MfxModel::paramMaxValues() const
{
    const int n = paramCount();
    QVariantList list;
    list.reserve(n);
    for (int i = 0; i < n; ++i)
        list.append(paramMax(i));
    return list;
}

QString MfxModel::uiFamily() const
{
    return MfxUiHelpers::instance()->familyForType(m_type);
}

bool MfxModel::usesVisualTemplate() const
{
    return MfxUiHelpers::instance()->usesVisualTemplate(m_type);
}

QString MfxModel::paramName(int index) const
{
    const auto entries = MfxParamCatalog::paramsForType(m_type);
    if (index >= 0 && index < entries.size())
        return entries.at(index).name;
    return MfxParamCatalog::paramName(m_type, index);
}

int MfxModel::paramMin(int index) const
{
    return MfxParamCatalog::paramMin(m_type, index);
}

int MfxModel::paramMax(int index) const
{
    return MfxParamCatalog::paramMax(m_type, index);
}

QStringList MfxModel::paramEnumNames(int index) const
{
    return MfxParamCatalog::paramEnumNames(m_type, index);
}

bool MfxModel::paramHasEnum(int index) const
{
    return MfxParamCatalog::paramHasEnum(m_type, index);
}

int MfxModel::paramIndexByName(const QString &name) const
{
    const auto entries = MfxParamCatalog::paramsForType(m_type);
    for (int i = 0; i < entries.size(); ++i) {
        if (entries.at(i).name.compare(name, Qt::CaseInsensitive) == 0)
            return i;
    }
    return -1;
}

void MfxModel::setError(const QString &e)
{
    if (m_lastError == e)
        return;
    m_lastError = e;
    emit lastErrorChanged();
}

void MfxModel::setContext(int partIndex, ToneEngine engine)
{
    m_partIndex = partIndex;
    m_engineType = engine;
}

QByteArray MfxModel::encodeParam(int logicalValue)
{
    logicalValue = std::clamp(logicalValue, -20000, 20000);
    const int packed = logicalValue + 32768;
    QByteArray out(4, '\0');
    out[0] = static_cast<char>((packed >> 12) & 0x0F);
    out[1] = static_cast<char>((packed >> 8) & 0x0F);
    out[2] = static_cast<char>((packed >> 4) & 0x0F);
    out[3] = static_cast<char>(packed & 0x0F);
    return out;
}

int MfxModel::decodeParam(const QByteArray &four)
{
    if (four.size() < 4)
        return 0;
    const int packed = ((static_cast<quint8>(four[0]) & 0x0F) << 12)
                     | ((static_cast<quint8>(four[1]) & 0x0F) << 8)
                     | ((static_cast<quint8>(four[2]) & 0x0F) << 4)
                     | (static_cast<quint8>(four[3]) & 0x0F);
    return packed - 32768;
}

void MfxModel::writeSwitch(bool on)
{
    if (m_fromDevice || !m_engine || !m_engine->isOpen()
        || m_engineType == ToneEngine::Unknown)
        return;
    QByteArray d(1, static_cast<char>(on ? 1 : 0));
    QString err;
    if (!m_engine->writeParam(addr::mfxSwitch(m_partIndex, m_engineType), d, &err))
        setError(err);
}

void MfxModel::writeByte(quint8 offset, int value)
{
    if (m_fromDevice || !m_engine || !m_engine->isOpen()
        || m_engineType == ToneEngine::Unknown)
        return;
    QByteArray d(1, static_cast<char>(value & 0x7F));
    QString err;
    if (!m_engine->writeParam(addOffset(addr::mfx(m_partIndex, m_engineType), offset), d, &err))
        setError(err);
}

void MfxModel::writeParam(int index, int logicalValue)
{
    if (m_fromDevice || !m_engine || !m_engine->isOpen()
        || m_engineType == ToneEngine::Unknown)
        return;
    if (index < 0 || index >= mfxOff::ParamCount)
        return;
    const auto data = encodeParam(logicalValue);
    const auto address = addOffset(addr::mfx(m_partIndex, m_engineType),
                                   mfxOff::Param1 + index * mfxOff::ParamBytes);
    QString err;
    if (!m_engine->writeParam(address, data, &err))
        setError(err);
}

void MfxModel::setMfxSwitch(bool v)
{
    if (m_switch == v)
        return;
    m_switch = v;
    emit mfxChanged();
    writeSwitch(v);
}

void MfxModel::setType(int v)
{
    v = std::clamp(v, 0, 68);
    if (m_type == v)
        return;
    m_type = v;
    if (m_raw.size() >= mfxOff::MfxSize)
        m_raw[mfxOff::Type] = static_cast<char>(v);
    emit mfxChanged();
    writeByte(mfxOff::Type, v);
}

void MfxModel::setChorusSend(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_chorusSend == v)
        return;
    m_chorusSend = v;
    if (m_raw.size() >= mfxOff::MfxSize)
        m_raw[mfxOff::ChorusSend] = static_cast<char>(v);
    emit mfxChanged();
    writeByte(mfxOff::ChorusSend, v);
}

void MfxModel::setReverbSend(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_reverbSend == v)
        return;
    m_reverbSend = v;
    if (m_raw.size() >= mfxOff::MfxSize)
        m_raw[mfxOff::ReverbSend] = static_cast<char>(v);
    emit mfxChanged();
    writeByte(mfxOff::ReverbSend, v);
}

int MfxModel::paramValue(int index) const
{
    if (index < 0 || index >= mfxOff::ParamCount || m_raw.size() < mfxOff::MfxSize)
        return 0;
    const int off = mfxOff::Param1 + index * mfxOff::ParamBytes;
    return decodeParam(m_raw.mid(off, 4));
}

void MfxModel::setParamValue(int index, int value)
{
    if (index < 0 || index >= mfxOff::ParamCount)
        return;
    value = std::clamp(value, -20000, 20000);
    if (paramValue(index) == value)
        return;
    if (m_raw.size() < mfxOff::MfxSize)
        m_raw = QByteArray(mfxOff::MfxSize, '\0');
    const int off = mfxOff::Param1 + index * mfxOff::ParamBytes;
    const auto enc = encodeParam(value);
    for (int i = 0; i < 4; ++i)
        m_raw[off + i] = enc[i];
    emit mfxChanged();
    writeParam(index, value);
}

bool MfxModel::applyPreset(const QJsonObject &preset)
{
    if (preset.isEmpty())
        return false;
    setFromDevice(true);
    setMfxSwitch(preset.value(QStringLiteral("switch")).toBool(true));
    setType(preset.value(QStringLiteral("type")).toInt(0));
    if (preset.contains(QStringLiteral("chorusSend")))
        setChorusSend(preset.value(QStringLiteral("chorusSend")).toInt(0));
    if (preset.contains(QStringLiteral("reverbSend")))
        setReverbSend(preset.value(QStringLiteral("reverbSend")).toInt(0));
    const auto params = preset.value(QStringLiteral("params")).toArray();
    for (int i = 0; i < params.size() && i < mfxOff::ParamCount; ++i)
        setParamValue(i, params.at(i).toInt());
    setFromDevice(false);

    // Live-write switch + full MFX block when connected.
    if (m_engine && m_engine->isOpen() && m_engineType != ToneEngine::Unknown) {
        writeSwitch(m_switch);
        QString err;
        if (!m_engine->write(addr::mfx(m_partIndex, m_engineType), m_raw, &err)) {
            setError(err);
            return false;
        }
    }
    emit mfxChanged();
    return true;
}

void MfxModel::loadBytes(const QByteArray &mfx145, bool switchOn)
{
    m_fromDevice = true;
    m_raw = mfx145;
    if (m_raw.size() < mfxOff::MfxSize)
        m_raw.resize(mfxOff::MfxSize);
    m_switch = switchOn;
    m_type = static_cast<quint8>(m_raw.at(mfxOff::Type));
    m_chorusSend = static_cast<quint8>(m_raw.at(mfxOff::ChorusSend));
    m_reverbSend = static_cast<quint8>(m_raw.at(mfxOff::ReverbSend));
    m_fromDevice = false;
    emit mfxChanged();
}

QByteArray MfxModel::mfxBytes() const
{
    return m_raw.size() >= mfxOff::MfxSize ? m_raw.left(mfxOff::MfxSize)
                                           : QByteArray(mfxOff::MfxSize, '\0');
}

bool MfxModel::pullFromDevice()
{
    if (!m_engine || !m_engine->isOpen() || m_engineType == ToneEngine::Unknown) {
        setError(QStringLiteral("Not connected or unknown tone engine"));
        return false;
    }
    QByteArray mfxData;
    QByteArray sw;
    QString err;
    if (!m_engine->read(addr::mfx(m_partIndex, m_engineType), mfxOff::MfxSize, &mfxData, &err)) {
        setError(err);
        return false;
    }
    if (!m_engine->read(addr::mfxSwitch(m_partIndex, m_engineType), 1, &sw, &err)) {
        setError(err);
        return false;
    }
    loadBytes(mfxData, !sw.isEmpty() && sw.at(0) != 0);
    setError({});
    return true;
}

bool MfxModel::pushToDevice()
{
    if (!m_engine || !m_engine->isOpen() || m_engineType == ToneEngine::Unknown) {
        setError(QStringLiteral("Not connected or unknown tone engine"));
        return false;
    }
    QString err;
    writeSwitch(m_switch);
    if (!m_engine->write(addr::mfx(m_partIndex, m_engineType), mfxBytes(), &err)) {
        setError(err);
        return false;
    }
    setError({});
    return true;
}

QJsonObject MfxModel::toJson() const
{
    QJsonObject o;
    o.insert(QStringLiteral("switch"), m_switch);
    o.insert(QStringLiteral("type"), m_type);
    o.insert(QStringLiteral("chorusSend"), m_chorusSend);
    o.insert(QStringLiteral("reverbSend"), m_reverbSend);
    o.insert(QStringLiteral("raw"), QString::fromLatin1(mfxBytes().toBase64()));
    return o;
}

bool MfxModel::fromJson(const QJsonObject &obj)
{
    if (obj.isEmpty())
        return false;
    const auto raw = QByteArray::fromBase64(obj.value(QStringLiteral("raw")).toString().toLatin1());
    loadBytes(raw.isEmpty() ? QByteArray(mfxOff::MfxSize, '\0') : raw,
              obj.value(QStringLiteral("switch")).toBool(false));
    if (obj.contains(QStringLiteral("type")))
        m_type = obj.value(QStringLiteral("type")).toInt(m_type);
    if (obj.contains(QStringLiteral("chorusSend")))
        m_chorusSend = obj.value(QStringLiteral("chorusSend")).toInt(m_chorusSend);
    if (obj.contains(QStringLiteral("reverbSend")))
        m_reverbSend = obj.value(QStringLiteral("reverbSend")).toInt(m_reverbSend);
    emit mfxChanged();
    return true;
}
