#pragma once

#include "midi/AddressMap.h"

#include <QByteArray>
#include <QJsonObject>
#include <QMap>
#include <QString>

class SysexEngine;

/** Raw Temporary Studio Set SysEx dumps (base64 in library JSON under sysexBlobs). */
class TemporarySysexStore
{
public:
    bool pullFromDevice(SysexEngine *engine, QString *error = nullptr);
    bool pushToDevice(SysexEngine *engine, QString *error = nullptr) const;

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
    bool readBlock(SysexEngine *engine, const QString &key,
                   const roland::Address &address, int size, QString *error);
    bool writeBlock(SysexEngine *engine, const QString &key,
                    const roland::Address &address, QString *error) const;

    QMap<QString, QByteArray> m_blobs;
};
