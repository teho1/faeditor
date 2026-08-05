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

    virtual ~InstrumentPlatform() = default;
    virtual bool isConnected() const = 0;
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
