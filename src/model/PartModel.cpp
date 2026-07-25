#include "model/PartModel.h"
#include "midi/AddressMap.h"

#include <algorithm>

PartModel::PartModel(int partIndex, QObject *parent)
    : QObject(parent)
    , m_index(partIndex)
    , m_receiveChannel(partIndex)
    , m_toneName(QStringLiteral("Part %1").arg(partIndex + 1))
{
}

void PartModel::emitEdit(const QString &param, int value)
{
    emit partChanged();
    if (!m_fromDevice)
        emit parameterEdited(m_index, param, value);
}

void PartModel::setReceiveChannel(int v)
{
    v = std::clamp(v, 0, 15);
    if (m_receiveChannel == v)
        return;
    m_receiveChannel = v;
    emitEdit(QStringLiteral("receiveChannel"), v);
}

void PartModel::setPartSwitch(bool v)
{
    if (m_partSwitch == v)
        return;
    m_partSwitch = v;
    emitEdit(QStringLiteral("partSwitch"), v ? 1 : 0);
}

void PartModel::setBankMsb(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_bankMsb == v)
        return;
    m_bankMsb = v;
    emitEdit(QStringLiteral("bankMsb"), v);
}

void PartModel::setBankLsb(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_bankLsb == v)
        return;
    m_bankLsb = v;
    emitEdit(QStringLiteral("bankLsb"), v);
}

void PartModel::setProgram(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_program == v)
        return;
    m_program = v;
    emitEdit(QStringLiteral("program"), v);
}

void PartModel::setLevel(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_level == v)
        return;
    m_level = v;
    emitEdit(QStringLiteral("level"), v);
}

void PartModel::setPan(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_pan == v)
        return;
    m_pan = v;
    emitEdit(QStringLiteral("pan"), v);
}

void PartModel::setCoarseTune(int v)
{
    v = std::clamp(v, 16, 112);
    if (m_coarseTune == v)
        return;
    m_coarseTune = v;
    emitEdit(QStringLiteral("coarseTune"), v);
}

void PartModel::setOctaveShift(int v)
{
    // UI uses -3..+3; store as raw 61..67
    if (v >= -3 && v <= 3)
        v = 64 + v;
    v = std::clamp(v, 61, 67);
    if (m_octaveShift == v)
        return;
    m_octaveShift = v;
    emitEdit(QStringLiteral("octaveShift"), v);
}

void PartModel::setVelocityLow(int v)
{
    v = std::clamp(v, 1, 127);
    if (m_velocityLow == v)
        return;
    m_velocityLow = v;
    emitEdit(QStringLiteral("velocityLow"), v);
}

void PartModel::setVelocityHigh(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_velocityHigh == v)
        return;
    m_velocityHigh = v;
    emitEdit(QStringLiteral("velocityHigh"), v);
}

void PartModel::setMute(bool v)
{
    if (m_mute == v)
        return;
    m_mute = v;
    emitEdit(QStringLiteral("mute"), v ? 1 : 0);
}

void PartModel::setChorusSend(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_chorusSend == v)
        return;
    m_chorusSend = v;
    emitEdit(QStringLiteral("chorusSend"), v);
}

void PartModel::setReverbSend(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_reverbSend == v)
        return;
    m_reverbSend = v;
    emitEdit(QStringLiteral("reverbSend"), v);
}

void PartModel::setOutputAssign(int v)
{
    v = std::clamp(v, 0, 1);
    if (m_outputAssign == v)
        return;
    m_outputAssign = v;
    emitEdit(QStringLiteral("outputAssign"), v);
}

void PartModel::setKeyLow(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_keyLow == v)
        return;
    m_keyLow = v;
    emitEdit(QStringLiteral("keyLow"), v);
}

void PartModel::setKeyHigh(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_keyHigh == v)
        return;
    m_keyHigh = v;
    emitEdit(QStringLiteral("keyHigh"), v);
}

void PartModel::setKeyboardSwitch(bool v)
{
    if (m_keyboardSwitch == v)
        return;
    m_keyboardSwitch = v;
    emitEdit(QStringLiteral("keyboardSwitch"), v ? 1 : 0);
}

void PartModel::setToneName(const QString &v)
{
    if (m_toneName == v)
        return;
    m_toneName = v;
    emit partChanged();
}

void PartModel::setSolo(bool v)
{
    if (m_solo == v)
        return;
    m_solo = v;
    emitEdit(QStringLiteral("solo"), v ? 1 : 0);
}

QByteArray PartModel::defaultPartTemplate() const
{
    // Sensible FA defaults for a playable part when we never pulled raw bytes.
    QByteArray data(roland::partOff::PartSize, char(0));
    auto set = [&](int off, int v) {
        if (off >= 0 && off < data.size())
            data[off] = static_cast<char>(v & 0x7F);
    };
    set(roland::partOff::ReceiveChannel, m_index);
    set(roland::partOff::PartSwitch, 1);
    set(roland::partOff::ReceiveSrc1, 1);
    set(roland::partOff::ReceiveSrc2, 1);
    set(roland::partOff::ReceiveSrc3, 1);
    set(roland::partOff::ReceiveSrc4, 1);
    set(roland::partOff::ToneBankMsb, 87);
    set(roland::partOff::ToneBankLsb, 64);
    set(roland::partOff::ToneProgram, 0);
    set(roland::partOff::PartLevel, 100);
    set(roland::partOff::PartPan, 64);
    set(roland::partOff::PartCoarseTune, 64);
    set(roland::partOff::PartFineTune, 64);
    set(roland::partOff::PartMonoPoly, 2); // TONE
    for (int off = 0x13; off <= 0x1A; ++off)
        set(off, 64); // cutoff/reso/env/vib offsets center
    set(roland::partOff::PartOctaveShift, 64);
    set(roland::partOff::PartVelocitySens, 64);
    set(roland::partOff::VelocityRangeLower, 1);
    set(roland::partOff::VelocityRangeUpper, 127);
    for (int off = 0x31; off <= 0x3C; ++off)
        set(off, 64); // scale tune centers
    for (int off = 0x3D; off <= 0x4B; ++off)
        set(off, 1); // receive MIDI filters ON (where applicable)
    return data;
}

QByteArray PartModel::defaultZoneTemplate() const
{
    QByteArray data(roland::zoneOff::ZoneSize, char(0));
    data[roland::zoneOff::KeyRangeLower] = char(0);
    data[roland::zoneOff::KeyRangeUpper] = char(127);
    data[roland::zoneOff::KeyboardSwitch] = char(1);
    // Control enables typically ON from factory
    for (int off = 0x03; off <= 0x0C; ++off) {
        if (off == 0x05 || off == 0x0D)
            continue;
        data[off] = char(1);
    }
    return data;
}

void PartModel::loadFromPartBytes(const QByteArray &data)
{
    m_rawPart = data;
    if (m_rawPart.size() < roland::partOff::PartSize)
        m_rawPart.resize(roland::partOff::PartSize);

    m_fromDevice = true;
    auto at = [&](int off, int def) -> int {
        return (off < data.size()) ? static_cast<quint8>(data.at(off)) : def;
    };
    m_receiveChannel = at(roland::partOff::ReceiveChannel, m_index);
    m_partSwitch = at(roland::partOff::PartSwitch, 1) != 0;
    m_bankMsb = at(roland::partOff::ToneBankMsb, 87);
    m_bankLsb = at(roland::partOff::ToneBankLsb, 64);
    m_program = at(roland::partOff::ToneProgram, 0);
    m_level = at(roland::partOff::PartLevel, 100);
    m_pan = at(roland::partOff::PartPan, 64);
    m_coarseTune = at(roland::partOff::PartCoarseTune, 64);
    m_octaveShift = at(roland::partOff::PartOctaveShift, 64);
    m_velocityLow = at(roland::partOff::VelocityRangeLower, 1);
    m_velocityHigh = at(roland::partOff::VelocityRangeUpper, 127);
    m_mute = at(roland::partOff::MuteSwitch, 0) != 0;
    m_chorusSend = at(roland::partOff::ChorusSend, 0);
    m_reverbSend = at(roland::partOff::ReverbSend, 0);
    m_outputAssign = at(roland::partOff::OutputAssign, 0);
    m_fromDevice = false;
    emit partChanged();
}

void PartModel::loadFromZoneBytes(const QByteArray &data)
{
    m_rawZone = data;
    if (m_rawZone.size() < roland::zoneOff::ZoneSize)
        m_rawZone.resize(roland::zoneOff::ZoneSize);

    m_fromDevice = true;
    auto at = [&](int off, int def) -> int {
        return (off < data.size()) ? static_cast<quint8>(data.at(off)) : def;
    };
    m_keyLow = at(roland::zoneOff::KeyRangeLower, 0);
    m_keyHigh = at(roland::zoneOff::KeyRangeUpper, 127);
    m_keyboardSwitch = at(roland::zoneOff::KeyboardSwitch, 1) != 0;
    m_fromDevice = false;
    emit partChanged();
}

QByteArray PartModel::toPartBytes() const
{
    QByteArray data = m_rawPart.size() >= roland::partOff::PartSize
                          ? m_rawPart.left(roland::partOff::PartSize)
                          : defaultPartTemplate();
    if (data.size() < roland::partOff::PartSize)
        data.resize(roland::partOff::PartSize);

    auto set = [&](int off, int v) {
        if (off < data.size())
            data[off] = static_cast<char>(v & 0x7F);
    };
    set(roland::partOff::ReceiveChannel, m_receiveChannel);
    set(roland::partOff::PartSwitch, m_partSwitch ? 1 : 0);
    // Never leave Receive Src off — that is what silenced the FA after Push.
    if (static_cast<quint8>(data[roland::partOff::ReceiveSrc1]) == 0
        && static_cast<quint8>(data[roland::partOff::ReceiveSrc2]) == 0
        && static_cast<quint8>(data[roland::partOff::ReceiveSrc3]) == 0
        && static_cast<quint8>(data[roland::partOff::ReceiveSrc4]) == 0) {
        set(roland::partOff::ReceiveSrc1, 1);
        set(roland::partOff::ReceiveSrc2, 1);
        set(roland::partOff::ReceiveSrc3, 1);
        set(roland::partOff::ReceiveSrc4, 1);
    }
    set(roland::partOff::ToneBankMsb, m_bankMsb);
    set(roland::partOff::ToneBankLsb, m_bankLsb);
    set(roland::partOff::ToneProgram, m_program);
    set(roland::partOff::PartLevel, m_level);
    set(roland::partOff::PartPan, m_pan);
    set(roland::partOff::PartCoarseTune, m_coarseTune);
    set(roland::partOff::PartOctaveShift, m_octaveShift);
    set(roland::partOff::VelocityRangeLower, m_velocityLow);
    set(roland::partOff::VelocityRangeUpper, m_velocityHigh);
    set(roland::partOff::MuteSwitch, m_mute ? 1 : 0);
    set(roland::partOff::ChorusSend, m_chorusSend);
    set(roland::partOff::ReverbSend, m_reverbSend);
    set(roland::partOff::OutputAssign, m_outputAssign);
    return data;
}

QByteArray PartModel::toZoneBytes() const
{
    QByteArray data = m_rawZone.size() >= roland::zoneOff::ZoneSize
                          ? m_rawZone.left(roland::zoneOff::ZoneSize)
                          : defaultZoneTemplate();
    if (data.size() < roland::zoneOff::ZoneSize)
        data.resize(roland::zoneOff::ZoneSize);
    data[roland::zoneOff::KeyRangeLower] = static_cast<char>(m_keyLow);
    data[roland::zoneOff::KeyRangeUpper] = static_cast<char>(m_keyHigh);
    data[roland::zoneOff::KeyboardSwitch] = static_cast<char>(m_keyboardSwitch ? 1 : 0);
    return data;
}
