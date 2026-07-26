#include "model/AudioFxModel.h"
#include "midi/SysexEngine.h"

#include <QJsonObject>
#include <algorithm>

AudioFxModel::AudioFxModel(SysexEngine *engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
{
}

QStringList AudioFxModel::inputReverbTypeNames() const
{
    return {
        QStringLiteral("Room 1"), QStringLiteral("Room 2"),
        QStringLiteral("Stage 1"), QStringLiteral("Stage 2"),
        QStringLiteral("Hall 1"), QStringLiteral("Hall 2"),
        QStringLiteral("Delay"), QStringLiteral("Pan Delay")
    };
}

QStringList AudioFxModel::tfxGainNames() const
{
    return {
        QStringLiteral("-18 dB"), QStringLiteral("-15 dB"), QStringLiteral("-12 dB"),
        QStringLiteral("-9 dB"), QStringLiteral("-6 dB"), QStringLiteral("-3 dB"),
        QStringLiteral("0 dB")
    };
}

QStringList AudioFxModel::tfxTypeNames() const
{
    // Official FA TFX list (Parameter Guide p.20): panel types 01–29.
    // SysEx Type is stored as that panel number (1–29), not 0–28.
    return {
        QStringLiteral("01 Filter + Drive"),
        QStringLiteral("02 Isolator"),
        QStringLiteral("03 DJFX Looper"),
        QStringLiteral("04 BPM Looper"),
        QStringLiteral("05 Bit Crush"),
        QStringLiteral("06 Wah"),
        QStringLiteral("07 Reverb"),
        QStringLiteral("08 Delay"),
        QStringLiteral("09 Tape Echo"),
        QStringLiteral("10 Pitch Shifter"),
        QStringLiteral("11 Voice Trans"),
        QStringLiteral("12 Flanger"),
        QStringLiteral("13 Slicer + Flg"),
        QStringLiteral("14 Phaser"),
        QStringLiteral("15 Chorus"),
        QStringLiteral("16 Tremolo / Pan"),
        QStringLiteral("17 Overdrive"),
        QStringLiteral("18 Distortion"),
        QStringLiteral("19 Fuzz"),
        QStringLiteral("20 Octave"),
        QStringLiteral("21 Subsonic"),
        QStringLiteral("22 Ring Mod"),
        QStringLiteral("23 Chromatic PS"),
        QStringLiteral("24 C. Canceler"),
        QStringLiteral("25 Vinyl Sim"),
        QStringLiteral("26 Radio Tuning"),
        QStringLiteral("27 Noise Gen"),
        QStringLiteral("28 Comp"),
        QStringLiteral("29 Equalizer")
    };
}

void AudioFxModel::setError(const QString &e)
{
    if (m_lastError == e)
        return;
    m_lastError = e;
    emit lastErrorChanged();
}

void AudioFxModel::setBusy(bool v)
{
    if (m_busy == v)
        return;
    m_busy = v;
    emit busyChanged();
}

void AudioFxModel::setControllerStatus(const QString &s)
{
    if (m_controllerStatus == s)
        return;
    m_controllerStatus = s;
    emit controllerStatusChanged();
}

void AudioFxModel::writeByte(const roland::Address &address, int value)
{
    if (m_fromDevice || !m_engine || !m_engine->isOpen())
        return;
    QByteArray d(1, static_cast<char>(value & 0x7F));
    QString err;
    if (!m_engine->writeParam(address, d, &err))
        setError(err);
}

#define AUDIO_SETTER_BOOL(name, member, addrExpr) \
void AudioFxModel::set##name(bool v) \
{ \
    if (m_##member == v) return; \
    m_##member = v; \
    emit audioFxChanged(); \
    writeByte(addrExpr, v ? 1 : 0); \
}

#define AUDIO_SETTER_INT(name, member, lo, hi, addrExpr) \
void AudioFxModel::set##name(int v) \
{ \
    v = std::clamp(v, lo, hi); \
    if (m_##member == v) return; \
    m_##member = v; \
    emit audioFxChanged(); \
    writeByte(addrExpr, v); \
}

AUDIO_SETTER_BOOL(InputReverbSwitch, inputReverbSwitch, roland::addr::inputEfxParam(0x00))
AUDIO_SETTER_INT(InputReverbType, inputReverbType, 0, 7, roland::addr::inputEfxParam(0x01))
AUDIO_SETTER_INT(InputReverbTime, inputReverbTime, 0, 127, roland::addr::inputEfxParam(0x02))
AUDIO_SETTER_INT(InputReverbLevel, inputReverbLevel, 0, 127, roland::addr::inputEfxParam(0x03))
AUDIO_SETTER_BOOL(NsSwitch, nsSwitch, roland::addr::inputEfxParam(0x04))
AUDIO_SETTER_INT(NsThreshold, nsThreshold, 0, 127, roland::addr::inputEfxParam(0x05))
AUDIO_SETTER_INT(NsRelease, nsRelease, 0, 127, roland::addr::inputEfxParam(0x06))
AUDIO_SETTER_BOOL(TfxSwitch, tfxSwitch, roland::addr::tfxParam(0x00))
AUDIO_SETTER_INT(TfxParamA, tfxParamA, 0, 127, roland::addr::tfxParam(0x02))
AUDIO_SETTER_INT(TfxParamB, tfxParamB, 0, 127, roland::addr::tfxParam(0x03))
AUDIO_SETTER_INT(TfxParamC, tfxParamC, 0, 127, roland::addr::tfxParam(0x04))
AUDIO_SETTER_INT(TfxParamD, tfxParamD, 0, 127, roland::addr::tfxParam(0x05))
AUDIO_SETTER_INT(TfxLocation, tfxLocation, 0, 1, roland::addr::systemCommonParam(roland::sysOff::TfxLocation))
AUDIO_SETTER_INT(TfxInputGain, tfxInputGain, 0, 6, roland::addr::systemCommonParam(roland::sysOff::TfxInputGain))

void AudioFxModel::setTfxType(int v)
{
    v = std::clamp(v, 0, 28);
    if (m_tfxType == v)
        return;
    m_tfxType = v;
    emit audioFxChanged();
    // Device stores panel type numbers 01–29 (SysEx 1–29), not list index 0–28.
    writeByte(roland::addr::tfxParam(0x01), v + 1);
}

#undef AUDIO_SETTER_BOOL
#undef AUDIO_SETTER_INT

bool AudioFxModel::pullFromDevice()
{
    if (!m_engine || !m_engine->isOpen()) {
        setError(QStringLiteral("Not connected"));
        return false;
    }
    setBusy(true);
    setError({});
    QString err;

    QByteArray input;
    if (!m_engine->read(roland::addr::kSystemInputEfx, roland::sysOff::InputEfxSize, &input, &err)) {
        setError(err);
        setBusy(false);
        return false;
    }
    QByteArray tfx;
    if (!m_engine->read(roland::addr::kSystemTfx, roland::sysOff::TfxSize, &tfx, &err)) {
        setError(err);
        setBusy(false);
        return false;
    }
    QByteArray common;
    m_engine->read(roland::addr::kSystemCommon, 0x30, &common, &err);

    m_fromDevice = true;
    auto at = [](const QByteArray &d, int off, int def) {
        return (off < d.size()) ? static_cast<quint8>(d.at(off)) : def;
    };
    m_inputReverbSwitch = at(input, 0, 0) != 0;
    m_inputReverbType = at(input, 1, 0);
    m_inputReverbTime = at(input, 2, 64);
    m_inputReverbLevel = at(input, 3, 40);
    m_nsSwitch = at(input, 4, 0) != 0;
    m_nsThreshold = at(input, 5, 40);
    m_nsRelease = at(input, 6, 40);

    m_tfxSwitch = at(tfx, 0, 0) != 0;
    {
        const int raw = at(tfx, 1, 1);
        // SysEx holds panel numbers 1–29; map to list index 0–28 (0 → treat as 01).
        m_tfxType = std::clamp(raw >= 1 ? raw - 1 : 0, 0, 28);
    }
    m_tfxParamA = at(tfx, 2, 64);
    m_tfxParamB = at(tfx, 3, 64);
    m_tfxParamC = at(tfx, 4, 64);
    m_tfxParamD = at(tfx, 5, 64);

    if (common.size() > roland::sysOff::TfxLocation)
        m_tfxLocation = at(common, roland::sysOff::TfxLocation, 1) & 0x01;
    if (common.size() > roland::sysOff::TfxInputGain)
        m_tfxInputGain = std::clamp(at(common, roland::sysOff::TfxInputGain, 6), 0, 6);

    m_fromDevice = false;
    setBusy(false);
    emit audioFxChanged();
    setControllerStatus(QStringLiteral("Pulled Input FX + TFX from FA"));
    return true;
}

bool AudioFxModel::pushToDevice()
{
    if (!m_engine || !m_engine->isOpen()) {
        setError(QStringLiteral("Not connected"));
        return false;
    }
    setBusy(true);
    QString err;

    QByteArray input(roland::sysOff::InputEfxSize, char(0));
    input[0] = char(m_inputReverbSwitch ? 1 : 0);
    input[1] = char(m_inputReverbType);
    input[2] = char(m_inputReverbTime);
    input[3] = char(m_inputReverbLevel);
    input[4] = char(m_nsSwitch ? 1 : 0);
    input[5] = char(m_nsThreshold);
    input[6] = char(m_nsRelease);

    QByteArray tfx(roland::sysOff::TfxSize, char(0));
    tfx[0] = char(m_tfxSwitch ? 1 : 0);
    tfx[1] = char(m_tfxType + 1);
    tfx[2] = char(m_tfxParamA);
    tfx[3] = char(m_tfxParamB);
    tfx[4] = char(m_tfxParamC);
    tfx[5] = char(m_tfxParamD);

    if (!m_engine->write(roland::addr::kSystemInputEfx, input, &err)
        || !m_engine->write(roland::addr::kSystemTfx, tfx, &err)) {
        setError(err);
        setBusy(false);
        return false;
    }
    writeByte(roland::addr::systemCommonParam(roland::sysOff::TfxLocation), m_tfxLocation);
    writeByte(roland::addr::systemCommonParam(roland::sysOff::TfxInputGain), m_tfxInputGain);

    setBusy(false);
    setControllerStatus(QStringLiteral("Pushed Input FX + TFX to FA"));
    return true;
}

QJsonObject AudioFxModel::toJson() const
{
    QJsonObject o;
    o.insert(QStringLiteral("inputReverbSwitch"), m_inputReverbSwitch);
    o.insert(QStringLiteral("inputReverbType"), m_inputReverbType);
    o.insert(QStringLiteral("inputReverbTime"), m_inputReverbTime);
    o.insert(QStringLiteral("inputReverbLevel"), m_inputReverbLevel);
    o.insert(QStringLiteral("nsSwitch"), m_nsSwitch);
    o.insert(QStringLiteral("nsThreshold"), m_nsThreshold);
    o.insert(QStringLiteral("nsRelease"), m_nsRelease);
    o.insert(QStringLiteral("tfxSwitch"), m_tfxSwitch);
    o.insert(QStringLiteral("tfxType"), m_tfxType);
    o.insert(QStringLiteral("tfxParamA"), m_tfxParamA);
    o.insert(QStringLiteral("tfxParamB"), m_tfxParamB);
    o.insert(QStringLiteral("tfxParamC"), m_tfxParamC);
    o.insert(QStringLiteral("tfxParamD"), m_tfxParamD);
    o.insert(QStringLiteral("tfxLocation"), m_tfxLocation);
    o.insert(QStringLiteral("tfxInputGain"), m_tfxInputGain);
    return o;
}

bool AudioFxModel::fromJson(const QJsonObject &obj)
{
    if (obj.isEmpty())
        return true;
    m_fromDevice = true;
    m_inputReverbSwitch = obj.value(QStringLiteral("inputReverbSwitch")).toBool(m_inputReverbSwitch);
    m_inputReverbType = obj.value(QStringLiteral("inputReverbType")).toInt(m_inputReverbType);
    m_inputReverbTime = obj.value(QStringLiteral("inputReverbTime")).toInt(m_inputReverbTime);
    m_inputReverbLevel = obj.value(QStringLiteral("inputReverbLevel")).toInt(m_inputReverbLevel);
    m_nsSwitch = obj.value(QStringLiteral("nsSwitch")).toBool(m_nsSwitch);
    m_nsThreshold = obj.value(QStringLiteral("nsThreshold")).toInt(m_nsThreshold);
    m_nsRelease = obj.value(QStringLiteral("nsRelease")).toInt(m_nsRelease);
    m_tfxSwitch = obj.value(QStringLiteral("tfxSwitch")).toBool(m_tfxSwitch);
    m_tfxType = std::clamp(obj.value(QStringLiteral("tfxType")).toInt(m_tfxType), 0, 28);
    m_tfxParamA = obj.value(QStringLiteral("tfxParamA")).toInt(m_tfxParamA);
    m_tfxParamB = obj.value(QStringLiteral("tfxParamB")).toInt(m_tfxParamB);
    m_tfxParamC = obj.value(QStringLiteral("tfxParamC")).toInt(m_tfxParamC);
    m_tfxParamD = obj.value(QStringLiteral("tfxParamD")).toInt(m_tfxParamD);
    m_tfxLocation = obj.value(QStringLiteral("tfxLocation")).toInt(m_tfxLocation) & 0x01;
    m_tfxInputGain = std::clamp(obj.value(QStringLiteral("tfxInputGain")).toInt(m_tfxInputGain), 0, 6);
    m_fromDevice = false;
    emit audioFxChanged();
    return true;
}

bool AudioFxModel::assignDeviceControlsToTfx()
{
    if (!m_engine || !m_engine->isOpen()) {
        setError(QStringLiteral("Not connected"));
        return false;
    }

    // Knob assign source = SYS (0)
    writeByte(roland::addr::systemControllerParam(roland::ctrlOff::KnobAssignSource), 0);
    // Sound Modify knobs 1–3 → TFX_PRM1–3
    writeByte(roland::addr::systemControllerParam(roland::ctrlOff::SoundModifyKnob1),
              roland::ctrlOff::KnobTfxPrm1);
    writeByte(roland::addr::systemControllerParam(static_cast<quint8>(roland::ctrlOff::SoundModifyKnob1 + 1)),
              roland::ctrlOff::KnobTfxPrm2);
    writeByte(roland::addr::systemControllerParam(static_cast<quint8>(roland::ctrlOff::SoundModifyKnob1 + 2)),
              roland::ctrlOff::KnobTfxPrm3);
    // S1 → TFX switch
    writeByte(roland::addr::systemControllerParam(roland::ctrlOff::SwitchS1Assign),
              roland::ctrlOff::SwitchTfxSw);

    // Guitar-friendly defaults
    if (m_tfxLocation != 1)
        setTfxLocation(1);
    if (!m_tfxSwitch)
        setTfxSwitch(true);

    setControllerStatus(
        QStringLiteral("Knobs 1–3 → TFX params · S1 → TFX on/off · TFX Location = INPUT"));
    return true;
}
