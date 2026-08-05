#pragma once

#include "platform/InstrumentPlatform.h"

#include <QHash>
#include <QVector>

class FakeInstrumentPlatform final : public InstrumentPlatform
{
public:
    struct Request { bool write; int part; roland::ToneEngine engine; ToneSection section; int index; int offset; QByteArray data; };
    struct AudioRequest { bool write; AudioBlock block; int offset; QByteArray data; };
    bool connected = true;
    QString failure;
    QVector<Request> requests;
    QVector<AudioRequest> audioRequests;
    int recalledMsb = -1, recalledLsb = -1, recalledProgram = -1;
    QString studioSetName = QStringLiteral("Fake Set");

    QString key(int part, roland::ToneEngine engine, ToneSection section, int index) const
    { return QStringLiteral("%1:%2:%3:%4").arg(part).arg(int(engine)).arg(int(section)).arg(index); }
    void seed(int part, roland::ToneEngine engine, ToneSection section, int index, const QByteArray &data)
    { memory.insert(key(part, engine, section, index), data); }
    QByteArray stored(int part, roland::ToneEngine engine, ToneSection section, int index) const
    { return memory.value(key(part, engine, section, index)); }
    void seedStudio(StudioBlock b,int i,const QByteArray&d){studio.insert(QStringLiteral("%1:%2").arg(int(b)).arg(i),d);}
    QByteArray storedStudio(StudioBlock b,int i)const{return studio.value(QStringLiteral("%1:%2").arg(int(b)).arg(i));}
    bool isConnected() const override { return connected; }
    bool recallStudioSet(int msb, int lsb, int program, QString *error) override
    { if (!check(error)) return false; recalledMsb=msb; recalledLsb=lsb; recalledProgram=program; return true; }
    bool readTemporaryStudioSetName(QString *name, QString *error) override
    { if (!check(error)) return false; if (name) *name=studioSetName; return true; }
    bool readStudioBlock(StudioBlock b,int i,int s,QByteArray*d,QString*e) override { if(!check(e))return false; if(d)*d=studio.value(QStringLiteral("%1:%2").arg(int(b)).arg(i),QByteArray(s,'\0')).leftJustified(s,'\0'); return true; }
    bool writeStudioBlock(StudioBlock b,int i,const QByteArray&d,QString*e) override { if(!check(e))return false; studio.insert(QStringLiteral("%1:%2").arg(int(b)).arg(i),d); return true; }
    bool writeStudioParameter(StudioBlock b,int i,int o,const QByteArray&d,QString*e) override { if(!check(e))return false; auto v=studio.value(QStringLiteral("%1:%2").arg(int(b)).arg(i)); if(v.size()<o+d.size())v.resize(o+d.size()); v.replace(o,d.size(),d); studio.insert(QStringLiteral("%1:%2").arg(int(b)).arg(i),v); return true; }
    bool readMasterEq(QByteArray*d,QString*e) override { if(!check(e))return false; if(d)*d=masterEq; return true; }
    bool writeMasterEq(const QByteArray&d,QString*e) override { if(!check(e))return false; masterEq=d; return true; }
    void seedAudio(AudioBlock b,const QByteArray&d){audio.insert(int(b),d);}
    QByteArray storedAudio(AudioBlock b)const{return audio.value(int(b));}
    bool readAudioBlock(AudioBlock b,int s,QByteArray*d,QString*e) override { audioRequests.push_back({false,b,0,{}}); if(!check(e)||!d)return false; *d=audio.value(int(b)).left(s).leftJustified(s,'\0'); return true; }
    bool writeAudioBlock(AudioBlock b,const QByteArray&d,QString*e) override { audioRequests.push_back({true,b,0,d}); if(!check(e))return false; audio.insert(int(b),d); return true; }
    bool writeAudioParameter(AudioBlock b,int o,const QByteArray&d,QString*e) override { audioRequests.push_back({true,b,o,d}); if(!check(e))return false; auto v=audio.value(int(b)); if(v.size()<o+d.size())v.resize(o+d.size()); v.replace(o,d.size(),d); audio.insert(int(b),v); return true; }
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
    QHash<QString, QByteArray> studio;
    QByteArray masterEq;
    QHash<int, QByteArray> audio;
};
