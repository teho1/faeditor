#pragma once

#include "platform/InstrumentPlatform.h"

class SysexEngine;

class RolandFAPlatform final : public InstrumentPlatform
{
public:
    explicit RolandFAPlatform(SysexEngine *engine) : m_engine(engine) {}
    bool isConnected() const override;
    bool recallStudioSet(int bankMsb, int bankLsb, int program, QString *error) override;
    bool readTemporaryStudioSetName(QString *name, QString *error) override;
    bool readStudioBlock(StudioBlock, int, int, QByteArray *, QString *) override;
    bool writeStudioBlock(StudioBlock, int, const QByteArray &, QString *) override;
    bool writeStudioParameter(StudioBlock, int, int, const QByteArray &, QString *) override;
    bool readMasterEq(QByteArray *, QString *) override;
    bool writeMasterEq(const QByteArray &, QString *) override;
    bool readAudioBlock(AudioBlock, int, QByteArray *, QString *) override;
    bool writeAudioBlock(AudioBlock, const QByteArray &, QString *) override;
    bool writeAudioParameter(AudioBlock, int, const QByteArray &, QString *) override;
    bool readToneSection(int part, roland::ToneEngine engine, ToneSection section,
                         int sectionIndex, int size, QByteArray *data, QString *error) override;
    bool writeToneSection(int part, roland::ToneEngine engine, ToneSection section,
                          int sectionIndex, const QByteArray &data, QString *error) override;
    bool writeToneParameter(int part, roland::ToneEngine engine, ToneSection section,
                            int sectionIndex, int offset, const QByteArray &data, QString *error) override;

private:
    roland::Address address(int part, roland::ToneEngine engine, ToneSection section,
                            int sectionIndex) const;
    roland::Address studioAddress(StudioBlock block, int index) const;
    roland::Address audioAddress(AudioBlock block) const;
    SysexEngine *m_engine = nullptr;
};
