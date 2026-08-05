#pragma once

#include "platform/InstrumentPlatform.h"

#include <QHash>
#include <QVector>

class FakeInstrumentPlatform final : public InstrumentPlatform
{
public:
    struct Request { bool write; int part; roland::ToneEngine engine; ToneSection section; int index; int offset; QByteArray data; };
    bool connected = true;
    QString failure;
    QVector<Request> requests;
    int recalledMsb = -1, recalledLsb = -1, recalledProgram = -1;
    QString studioSetName = QStringLiteral("Fake Set");

    QString key(int part, roland::ToneEngine engine, ToneSection section, int index) const
    { return QStringLiteral("%1:%2:%3:%4").arg(part).arg(int(engine)).arg(int(section)).arg(index); }
    void seed(int part, roland::ToneEngine engine, ToneSection section, int index, const QByteArray &data)
    { memory.insert(key(part, engine, section, index), data); }
    QByteArray stored(int part, roland::ToneEngine engine, ToneSection section, int index) const
    { return memory.value(key(part, engine, section, index)); }
    bool isConnected() const override { return connected; }
    bool recallStudioSet(int msb, int lsb, int program, QString *error) override
    { if (!check(error)) return false; recalledMsb=msb; recalledLsb=lsb; recalledProgram=program; return true; }
    bool readTemporaryStudioSetName(QString *name, QString *error) override
    { if (!check(error)) return false; if (name) *name=studioSetName; return true; }
    bool readToneSection(int part, roland::ToneEngine engine, ToneSection section, int index,
                         int size, QByteArray *data, QString *error) override
    {
        requests.push_back({false, part, engine, section, index, 0, {}});
        if (!check(error) || !data) return false;
        *data = memory.value(key(part, engine, section, index)).left(size);
        return true;
    }
    bool writeToneSection(int part, roland::ToneEngine engine, ToneSection section, int index,
                          const QByteArray &data, QString *error) override
    {
        requests.push_back({true, part, engine, section, index, 0, data});
        if (!check(error)) return false;
        memory.insert(key(part, engine, section, index), data); return true;
    }
    bool writeToneParameter(int part, roland::ToneEngine engine, ToneSection section, int index,
                            int offset, const QByteArray &data, QString *error) override
    {
        requests.push_back({true, part, engine, section, index, offset, data});
        if (!check(error)) return false;
        auto value = memory.value(key(part, engine, section, index));
        if (value.size() < offset + data.size()) value.resize(offset + data.size());
        value.replace(offset, data.size(), data); memory.insert(key(part, engine, section, index), value); return true;
    }
private:
    bool check(QString *error) const { if (!connected || !failure.isEmpty()) { if (error) *error = failure.isEmpty() ? QStringLiteral("Not connected") : failure; return false; } return true; }
    QHash<QString, QByteArray> memory;
};
