#include "model/EffectsModel.h"

#include <algorithm>

EffectsModel::EffectsModel(QObject *parent)
    : QObject(parent)
{
}

QStringList EffectsModel::chorusTypeNames() const
{
    // FA Parameter Guide — Studio Set Chorus Type (panel 00–03).
    return {
        QStringLiteral("00: OFF"),
        QStringLiteral("01: Chorus"),
        QStringLiteral("02: Delay"),
        QStringLiteral("03: GM2 Chorus")
    };
}

QStringList EffectsModel::reverbTypeNames() const
{
    // FA Parameter Guide — Studio Set Reverb Type (panel 00–06).
    return {
        QStringLiteral("00: OFF"),
        QStringLiteral("01: Room 1"),
        QStringLiteral("02: Room 2"),
        QStringLiteral("03: Hall 1"),
        QStringLiteral("04: Hall 2"),
        QStringLiteral("05: Plate"),
        QStringLiteral("06: GM2 Reverb")
    };
}

void EffectsModel::emitEdit(const QString &section, const QString &param, int value)
{
    emit effectsChanged();
    if (!m_fromDevice)
        emit parameterEdited(section, param, value);
}

void EffectsModel::setChorusType(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_chorusType == v)
        return;
    m_chorusType = v;
    emitEdit(QStringLiteral("chorus"), QStringLiteral("type"), v);
}

void EffectsModel::setChorusLevel(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_chorusLevel == v)
        return;
    m_chorusLevel = v;
    emitEdit(QStringLiteral("chorus"), QStringLiteral("level"), v);
}

void EffectsModel::setReverbType(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_reverbType == v)
        return;
    m_reverbType = v;
    emitEdit(QStringLiteral("reverb"), QStringLiteral("type"), v);
}

void EffectsModel::setReverbLevel(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_reverbLevel == v)
        return;
    m_reverbLevel = v;
    emitEdit(QStringLiteral("reverb"), QStringLiteral("level"), v);
}

void EffectsModel::setMasterCompSwitch(bool v)
{
    if (m_masterCompSwitch == v)
        return;
    m_masterCompSwitch = v;
    emitEdit(QStringLiteral("masterComp"), QStringLiteral("switch"), v ? 1 : 0);
}

void EffectsModel::setMasterCompAttack(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_masterCompAttack == v)
        return;
    m_masterCompAttack = v;
    emitEdit(QStringLiteral("masterComp"), QStringLiteral("attack"), v);
}

void EffectsModel::setMasterCompRelease(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_masterCompRelease == v)
        return;
    m_masterCompRelease = v;
    emitEdit(QStringLiteral("masterComp"), QStringLiteral("release"), v);
}

void EffectsModel::setMasterCompThreshold(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_masterCompThreshold == v)
        return;
    m_masterCompThreshold = v;
    emitEdit(QStringLiteral("masterComp"), QStringLiteral("threshold"), v);
}

void EffectsModel::setMasterCompRatio(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_masterCompRatio == v)
        return;
    m_masterCompRatio = v;
    emitEdit(QStringLiteral("masterComp"), QStringLiteral("ratio"), v);
}

void EffectsModel::setMasterCompGain(int v)
{
    v = std::clamp(v, 0, 127);
    if (m_masterCompGain == v)
        return;
    m_masterCompGain = v;
    emitEdit(QStringLiteral("masterComp"), QStringLiteral("gain"), v);
}

void EffectsModel::loadChorus(const QByteArray &data)
{
    m_rawChorus = data;
    m_fromDevice = true;
    if (data.size() > 0)
        m_chorusType = static_cast<quint8>(data[0]);
    if (data.size() > 1)
        m_chorusLevel = static_cast<quint8>(data[1]);
    m_fromDevice = false;
    emit effectsChanged();
}

void EffectsModel::loadReverb(const QByteArray &data)
{
    m_rawReverb = data;
    m_fromDevice = true;
    // Reverb Type is at offset 01 in Studio Set Reverb block per doc example (18 00 02 01)
    if (data.size() > 1)
        m_reverbType = static_cast<quint8>(data[1]);
    else if (data.size() > 0)
        m_reverbType = static_cast<quint8>(data[0]);
    if (data.size() > 2)
        m_reverbLevel = static_cast<quint8>(data[2]);
    m_fromDevice = false;
    emit effectsChanged();
}

void EffectsModel::loadMasterComp(const QByteArray &data)
{
    m_rawMasterComp = data;
    m_fromDevice = true;
    if (data.size() > 0)
        m_masterCompSwitch = static_cast<quint8>(data[0]) != 0;
    if (data.size() > 1)
        m_masterCompAttack = static_cast<quint8>(data[1]);
    if (data.size() > 2)
        m_masterCompRelease = static_cast<quint8>(data[2]);
    if (data.size() > 3)
        m_masterCompThreshold = static_cast<quint8>(data[3]);
    if (data.size() > 4)
        m_masterCompRatio = static_cast<quint8>(data[4]);
    if (data.size() > 5)
        m_masterCompGain = static_cast<quint8>(data[5]);
    m_fromDevice = false;
    emit effectsChanged();
}

QByteArray EffectsModel::chorusBytes() const
{
    QByteArray d = m_rawChorus.isEmpty() ? QByteArray(0x20, char(0)) : m_rawChorus;
    if (d.size() < 2)
        d.resize(2);
    d[0] = static_cast<char>(m_chorusType);
    d[1] = static_cast<char>(m_chorusLevel);
    return d;
}

QByteArray EffectsModel::reverbBytes() const
{
    QByteArray d = m_rawReverb.isEmpty() ? QByteArray(0x20, char(0)) : m_rawReverb;
    if (d.size() < 3)
        d.resize(3);
    d[1] = static_cast<char>(m_reverbType);
    d[2] = static_cast<char>(m_reverbLevel);
    return d;
}

QByteArray EffectsModel::masterCompBytes() const
{
    QByteArray d = m_rawMasterComp.isEmpty() ? QByteArray(0x20, char(0)) : m_rawMasterComp;
    if (d.size() < 6)
        d.resize(6);
    d[0] = static_cast<char>(m_masterCompSwitch ? 1 : 0);
    d[1] = static_cast<char>(m_masterCompAttack);
    d[2] = static_cast<char>(m_masterCompRelease);
    d[3] = static_cast<char>(m_masterCompThreshold);
    d[4] = static_cast<char>(m_masterCompRatio);
    d[5] = static_cast<char>(m_masterCompGain);
    return d;
}
