#pragma once

#include "midi/AddressMap.h"

#include <QByteArray>
#include <QString>

class InstrumentPlatform
{
public:
    enum class ToneSection { Mfx, MfxSwitch, SnCommon, SnMisc, SnPartial,
                             PcmCommon, PcmPmt, PcmCommon2, PcmPartial,
                             SnAcousticCommon };
    enum class StudioBlock { Common, Chorus, Reverb, Ifx, MasterComp, Controller,
                             PadCommon, Midi, Part, PartEq, Zone, Pad };
    enum class AudioBlock { SystemCommon, InputEfx, Tfx, SystemController };

    virtual ~InstrumentPlatform() = default;
    virtual bool isConnected() const = 0;
    virtual bool recallStudioSet(int bankMsb, int bankLsb, int program,
                                 QString *error = nullptr) = 0;
    virtual bool readTemporaryStudioSetName(QString *name, QString *error = nullptr) = 0;
    virtual bool readStudioBlock(StudioBlock block, int index, int size, QByteArray *data,
                                 QString *error = nullptr) = 0;
    virtual bool writeStudioBlock(StudioBlock block, int index, const QByteArray &data,
                                  QString *error = nullptr) = 0;
    virtual bool writeStudioParameter(StudioBlock block, int index, int offset,
                                      const QByteArray &data, QString *error = nullptr) = 0;
    virtual bool readMasterEq(QByteArray *data, QString *error = nullptr) = 0;
    virtual bool writeMasterEq(const QByteArray &data, QString *error = nullptr) = 0;
    virtual bool readAudioBlock(AudioBlock block, int size, QByteArray *data,
                                QString *error = nullptr) = 0;
    virtual bool writeAudioBlock(AudioBlock block, const QByteArray &data,
                                 QString *error = nullptr) = 0;
    virtual bool writeAudioParameter(AudioBlock block, int offset, const QByteArray &data,
                                     QString *error = nullptr) = 0;
    virtual bool readToneSection(int part, roland::ToneEngine engine, ToneSection section,
                                 int sectionIndex, int size, QByteArray *data,
                                 QString *error = nullptr) = 0;
    virtual bool writeToneSection(int part, roland::ToneEngine engine, ToneSection section,
                                  int sectionIndex, const QByteArray &data,
                                  QString *error = nullptr) = 0;
    virtual bool writeToneParameter(int part, roland::ToneEngine engine, ToneSection section,
                                    int sectionIndex, int offset, const QByteArray &data,
                                    QString *error = nullptr) = 0;
};
