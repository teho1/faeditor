#pragma once

#include "platform/InstrumentPlatform.h"

// Safe base for documented adapter skeletons. No protocol is guessed: every
// hardware operation fails explicitly until a verified implementation exists.
class UnsupportedInstrumentPlatform : public InstrumentPlatform
{
public:
    bool isConnected() const override { return false; }
    bool discoverMidiPorts(QVector<MidiPort> *, QVector<MidiPort> *, QString *e) override { return unsupported(e); }
    bool openMidiConnection(int, int, QString *e) override { return unsupported(e); }
    void closeMidiConnection() override {}
    bool midiConnectionHealthy() const override { return false; }
    bool detectDevice(quint8 *, int, QString *e) override { return unsupported(e); }
    bool sendPreviewNote(int, int, int, bool, QString *e) override { return unsupported(e); }
    bool recallPerformance(int,int,int,QString*e) override{return unsupported(e);}
    bool readTemporaryPerformanceName(QString*,QString*e) override{return unsupported(e);}
    bool readStudioBlock(StudioBlock,int,int,QByteArray*,QString*e) override{return unsupported(e);}
    bool writeStudioBlock(StudioBlock,int,const QByteArray&,QString*e) override{return unsupported(e);}
    bool writeStudioParameter(StudioBlock,int,int,const QByteArray&,QString*e) override{return unsupported(e);}
    bool readMasterEq(QByteArray*,QString*e) override{return unsupported(e);}
    bool writeMasterEq(const QByteArray&,QString*e) override{return unsupported(e);}
    bool readAudioBlock(AudioBlock,int,QByteArray*,QString*e) override{return unsupported(e);}
    bool writeAudioBlock(AudioBlock,const QByteArray&,QString*e) override{return unsupported(e);}
    bool writeAudioParameter(AudioBlock,int,const QByteArray&,QString*e) override{return unsupported(e);}
    bool readToneSection(int,roland::ToneEngine,ToneSection,int,int,QByteArray*,QString*e) override{return unsupported(e);}
    bool writeToneSection(int,roland::ToneEngine,ToneSection,int,const QByteArray&,QString*e) override{return unsupported(e);}
    bool writeToneParameter(int,roland::ToneEngine,ToneSection,int,int,const QByteArray&,QString*e) override{return unsupported(e);}

protected:
    static bool unsupported(QString *error)
    { if(error)*error=QStringLiteral("Not implemented for this instrument profile"); return false; }
};
