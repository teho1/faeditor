#include "model/TemporarySysexStore.h"
#include "platform/InstrumentPlatform.h"

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

bool TemporarySysexStore::readBlock(InstrumentPlatform *engine, const QString &key,
                                    InstrumentPlatform::StudioBlock block, int index, int size, QString *error)
{
    QByteArray data;
    if (!engine->readStudioBlock(block, index, size, &data, error))
        return false;
    m_blobs.insert(key, data);
    return true;
}

bool TemporarySysexStore::writeBlock(InstrumentPlatform *engine, const QString &key,
                                     InstrumentPlatform::StudioBlock block, int index, QString *error) const
{
    const auto data = m_blobs.value(key);
    if (data.isEmpty())
        return true;
    return engine->writeStudioBlock(block, index, data, error);
}

bool TemporarySysexStore::pullFromDevice(InstrumentPlatform *engine, QString *error)
{
    if (!engine || !engine->isConnected()) {
        if (error)
            *error = QStringLiteral("Not connected");
        return false;
    }

    using namespace roland;
    m_blobs.clear();

    using B=InstrumentPlatform::StudioBlock;
    if (!readBlock(engine,"common",B::Common,0,ssOff::CommonSize,error)||!readBlock(engine,"chorus",B::Chorus,0,ssOff::ChorusSize,error)||!readBlock(engine,"reverb",B::Reverb,0,ssOff::ReverbSize,error)||!readBlock(engine,"ifx",B::Ifx,0,ssOff::IfxSize,error)||!readBlock(engine,"masterComp",B::MasterComp,0,ssOff::MasterCompSize,error)||!readBlock(engine,"controller",B::Controller,0,ssOff::ControllerSize,error)||!readBlock(engine,"padCommon",B::PadCommon,0,ssOff::PadCommonSize,error)) {
        return false;
    }

    for (int i = 0; i < 16; ++i) {
        if (!readBlock(engine,midiKey(i),B::Midi,i,ssOff::MidiChSize,error)||!readBlock(engine,partKey(i),B::Part,i,partOff::PartSize,error)||!readBlock(engine,partEqKey(i),B::PartEq,i,ssOff::PartEqSize,error)||!readBlock(engine,zoneKey(i),B::Zone,i,zoneOff::ZoneSize,error)||!readBlock(engine,padKey(i),B::Pad,i,ssOff::PadSize,error)) {
            return false;
        }
    }
    return true;
}

bool TemporarySysexStore::pushToDevice(InstrumentPlatform *engine, QString *error) const
{
    if (!engine || !engine->isConnected()) {
        if (error)
            *error = QStringLiteral("Not connected");
        return false;
    }
    if (m_blobs.isEmpty())
        return true;

    using namespace roland;

    using B=InstrumentPlatform::StudioBlock;
    if (!writeBlock(engine,"common",B::Common,0,error)||!writeBlock(engine,"chorus",B::Chorus,0,error)||!writeBlock(engine,"reverb",B::Reverb,0,error)||!writeBlock(engine,"ifx",B::Ifx,0,error)||!writeBlock(engine,"masterComp",B::MasterComp,0,error)||!writeBlock(engine,"controller",B::Controller,0,error)||!writeBlock(engine,"padCommon",B::PadCommon,0,error)) {
        return false;
    }

    for (int i = 0; i < 16; ++i) {
        if (!writeBlock(engine,midiKey(i),B::Midi,i,error)||!writeBlock(engine,partKey(i),B::Part,i,error)||!writeBlock(engine,partEqKey(i),B::PartEq,i,error)||!writeBlock(engine,zoneKey(i),B::Zone,i,error)||!writeBlock(engine,padKey(i),B::Pad,i,error)) {
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
