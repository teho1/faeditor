#pragma once

#include "platform/InstrumentPlatform.h"

class SysexEngine;

class RolandFAPlatform final : public InstrumentPlatform
{
public:
    explicit RolandFAPlatform(SysexEngine *engine) : m_engine(engine) {}
    bool isConnected() const override;
    bool readToneSection(int part, roland::ToneEngine engine, ToneSection section,
                         int sectionIndex, int size, QByteArray *data, QString *error) override;
    bool writeToneSection(int part, roland::ToneEngine engine, ToneSection section,
                          int sectionIndex, const QByteArray &data, QString *error) override;
    bool writeToneParameter(int part, roland::ToneEngine engine, ToneSection section,
                            int sectionIndex, int offset, const QByteArray &data, QString *error) override;

private:
    roland::Address address(int part, roland::ToneEngine engine, ToneSection section,
                            int sectionIndex) const;
    SysexEngine *m_engine = nullptr;
};
