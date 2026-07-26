#include "model/TemporarySysexStore.h"
#include "midi/SysexEngine.h"

#include <QJsonObject>

namespace {

QString paddedKey(const char *prefix, int index1Based)
{
    return QStringLiteral("%1%2").arg(QLatin1String(prefix)).arg(index1Based, 2, 10, QLatin1Char('0'));
}

} // namespace

QString TemporarySysexStore::partKey(int index)
{
    return paddedKey("part", index + 1);
}

QString TemporarySysexStore::zoneKey(int index)
{
    return paddedKey("zone", index + 1);
}

QString TemporarySysexStore::partEqKey(int index)
{
    return paddedKey("partEq", index + 1);
}

QString TemporarySysexStore::midiKey(int index)
{
    return paddedKey("midi", index + 1);
}

QString TemporarySysexStore::padKey(int index)
{
    return paddedKey("pad", index + 1);
}

QByteArray TemporarySysexStore::blob(const QString &key) const
{
    return m_blobs.value(key);
}

void TemporarySysexStore::setBlob(const QString &key, const QByteArray &data)
{
    if (data.isEmpty())
        m_blobs.remove(key);
    else
        m_blobs.insert(key, data);
}

bool TemporarySysexStore::readBlock(SysexEngine *engine, const QString &key,
                                    const roland::Address &address, int size, QString *error)
{
    QByteArray data;
    if (!engine->read(address, size, &data, error))
        return false;
    m_blobs.insert(key, data);
    return true;
}

bool TemporarySysexStore::writeBlock(SysexEngine *engine, const QString &key,
                                     const roland::Address &address, QString *error) const
{
    const auto data = m_blobs.value(key);
    if (data.isEmpty())
        return true;
    return engine->write(address, data, error);
}

bool TemporarySysexStore::pullFromDevice(SysexEngine *engine, QString *error)
{
    if (!engine || !engine->isOpen()) {
        if (error)
            *error = QStringLiteral("Not connected");
        return false;
    }

    using namespace roland;
    m_blobs.clear();

    if (!readBlock(engine, QStringLiteral("common"), addr::kStudioSetCommon, ssOff::CommonSize, error)
        || !readBlock(engine, QStringLiteral("chorus"), addr::kStudioSetChorus, ssOff::ChorusSize, error)
        || !readBlock(engine, QStringLiteral("reverb"), addr::kStudioSetReverb, ssOff::ReverbSize, error)
        || !readBlock(engine, QStringLiteral("ifx"), addr::kStudioSetIfx, ssOff::IfxSize, error)
        || !readBlock(engine, QStringLiteral("masterComp"), addr::kStudioSetMasterComp, ssOff::MasterCompSize, error)
        || !readBlock(engine, QStringLiteral("controller"), addr::kStudioSetController, ssOff::ControllerSize, error)
        || !readBlock(engine, QStringLiteral("padCommon"), addr::kStudioSetPadCommon, ssOff::PadCommonSize, error)) {
        return false;
    }

    for (int i = 0; i < 16; ++i) {
        if (!readBlock(engine, midiKey(i), addr::midiChannel(i), ssOff::MidiChSize, error)
            || !readBlock(engine, partKey(i), addr::part(i), partOff::PartSize, error)
            || !readBlock(engine, partEqKey(i), addr::partEq(i), ssOff::PartEqSize, error)
            || !readBlock(engine, zoneKey(i), addr::zone(i), zoneOff::ZoneSize, error)
            || !readBlock(engine, padKey(i), addr::pad(i), ssOff::PadSize, error)) {
            return false;
        }
    }
    return true;
}

bool TemporarySysexStore::pushToDevice(SysexEngine *engine, QString *error) const
{
    if (!engine || !engine->isOpen()) {
        if (error)
            *error = QStringLiteral("Not connected");
        return false;
    }
    if (m_blobs.isEmpty())
        return true;

    using namespace roland;

    if (!writeBlock(engine, QStringLiteral("common"), addr::kStudioSetCommon, error)
        || !writeBlock(engine, QStringLiteral("chorus"), addr::kStudioSetChorus, error)
        || !writeBlock(engine, QStringLiteral("reverb"), addr::kStudioSetReverb, error)
        || !writeBlock(engine, QStringLiteral("ifx"), addr::kStudioSetIfx, error)
        || !writeBlock(engine, QStringLiteral("masterComp"), addr::kStudioSetMasterComp, error)
        || !writeBlock(engine, QStringLiteral("controller"), addr::kStudioSetController, error)
        || !writeBlock(engine, QStringLiteral("padCommon"), addr::kStudioSetPadCommon, error)) {
        return false;
    }

    for (int i = 0; i < 16; ++i) {
        if (!writeBlock(engine, midiKey(i), addr::midiChannel(i), error)
            || !writeBlock(engine, partKey(i), addr::part(i), error)
            || !writeBlock(engine, partEqKey(i), addr::partEq(i), error)
            || !writeBlock(engine, zoneKey(i), addr::zone(i), error)
            || !writeBlock(engine, padKey(i), addr::pad(i), error)) {
            return false;
        }
    }
    return true;
}

QJsonObject TemporarySysexStore::toJson() const
{
    QJsonObject obj;
    for (auto it = m_blobs.constBegin(); it != m_blobs.constEnd(); ++it) {
        if (!it.value().isEmpty())
            obj.insert(it.key(), QString::fromLatin1(it.value().toBase64()));
    }
    return obj;
}

void TemporarySysexStore::fromJson(const QJsonObject &obj)
{
    m_blobs.clear();
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        const QByteArray raw = QByteArray::fromBase64(it.value().toString().toLatin1());
        if (!raw.isEmpty())
            m_blobs.insert(it.key(), raw);
    }
}
