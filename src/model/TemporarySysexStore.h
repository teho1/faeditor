#pragma once

#include "midi/AddressMap.h"
#include "platform/InstrumentPlatform.h"

#include <QByteArray>
#include <QJsonObject>
#include <QMap>
#include <QString>

class InstrumentPlatform;

/** Raw Temporary Studio Set SysEx dumps (base64 in library JSON under sysexBlobs). */
class TemporarySysexStore
{
public:
    bool pullFromDevice(InstrumentPlatform *platform, QString *error = nullptr);
    bool pushToDevice(InstrumentPlatform *platform, QString *error = nullptr) const;

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &obj);

    bool isEmpty() const { return m_blobs.isEmpty(); }
    void clear() { m_blobs.clear(); }

    QByteArray blob(const QString &key) const;
    void setBlob(const QString &key, const QByteArray &data);

    static QString partKey(int index);
    static QString zoneKey(int index);
    static QString partEqKey(int index);
    static QString midiKey(int index);
    static QString padKey(int index);

private:
    bool readBlock(InstrumentPlatform *, const QString &, InstrumentPlatform::StudioBlock, int, int, QString *);
    bool writeBlock(InstrumentPlatform *, const QString &, InstrumentPlatform::StudioBlock, int, QString *) const;

    QMap<QString, QByteArray> m_blobs;
};
