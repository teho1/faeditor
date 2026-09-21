#pragma once

#include "platform/InstrumentPlatform.h"
#include "platform/RolandFAPlatform.h"
#include "platform/RolandFantomPlatform.h"

class SysexEngine;

/** One shared platform endpoint that selects FA or FANTOM-0 after Universal Identity Reply. */
class AutoDetectRolandPlatform final : public InstrumentPlatform
{
public:
    enum class Family { Unknown, FA, Fantom0 };
    explicit AutoDetectRolandPlatform(SysexEngine *engine);
    Family family() const{return m_family;}
    bool isFantom0() const{return m_family==Family::Fantom0;}
    DeviceProfile profile() const override;
    bool isConnected() const override;
    bool discoverMidiPorts(QVector<MidiPort>*,QVector<MidiPort>*,QString*) override;
    bool openMidiConnection(int,int,QString*) override;
    void closeMidiConnection() override;
    void resetMidiHost() override;
    bool midiConnectionHealthy()const override;
    bool detectDevice(quint8*,int,QString*) override;
    bool sendPreviewNote(int a,int b,int c,bool d,QString*e)override{return active()->sendPreviewNote(a,b,c,d,e);}
    bool recallPerformance(int a,int b,int c,QString*e)override{return active()->recallPerformance(a,b,c,e);}
    bool readTemporaryPerformanceName(QString*a,QString*e)override{return active()->readTemporaryPerformanceName(a,e);}
    bool readStudioBlock(StudioBlock a,int b,int c,QByteArray*d,QString*e)override{return active()->readStudioBlock(a,b,c,d,e);}
    bool writeStudioBlock(StudioBlock a,int b,const QByteArray&c,QString*e)override{return active()->writeStudioBlock(a,b,c,e);}
    bool writeStudioParameter(StudioBlock a,int b,int c,const QByteArray&d,QString*e)override{return active()->writeStudioParameter(a,b,c,d,e);}
    bool readMasterEq(QByteArray*a,QString*e)override{return active()->readMasterEq(a,e);}
    bool writeMasterEq(const QByteArray&a,QString*e)override{return active()->writeMasterEq(a,e);}
    bool readAudioBlock(AudioBlock a,int b,QByteArray*c,QString*e)override{return active()->readAudioBlock(a,b,c,e);}
    bool writeAudioBlock(AudioBlock a,const QByteArray&b,QString*e)override{return active()->writeAudioBlock(a,b,e);}
    bool writeAudioParameter(AudioBlock a,int b,const QByteArray&c,QString*e)override{return active()->writeAudioParameter(a,b,c,e);}
    bool readToneSection(int a,roland::ToneEngine b,ToneSection c,int d,int f,QByteArray*g,QString*e)override{return active()->readToneSection(a,b,c,d,f,g,e);}
    bool writeToneSection(int a,roland::ToneEngine b,ToneSection c,int d,const QByteArray&f,QString*e)override{return active()->writeToneSection(a,b,c,d,f,e);}
    bool writeToneParameter(int a,roland::ToneEngine b,ToneSection c,int d,int f,const QByteArray&g,QString*e)override{return active()->writeToneParameter(a,b,c,d,f,g,e);}
private:
    InstrumentPlatform *active(); const InstrumentPlatform *active()const;
    void select(Family);
    SysexEngine *m_engine;
    RolandFAPlatform m_fa;
    RolandFantomPlatform m_fantom;
    Family m_family=Family::Unknown;
    QVector<MidiPort> m_lastInputs,m_lastOutputs;
};
