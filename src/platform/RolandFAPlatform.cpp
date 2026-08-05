#include "platform/RolandFAPlatform.h"
#include "midi/SysexEngine.h"

using namespace roland;

bool RolandFAPlatform::isConnected() const { return m_engine && m_engine->isOpen(); }

Address RolandFAPlatform::address(int part, ToneEngine engine, ToneSection section, int index) const
{
    switch (section) {
    case ToneSection::Mfx: return addr::mfx(part, engine);
    case ToneSection::MfxSwitch: return addr::mfxSwitch(part, engine);
    case ToneSection::SnCommon: return addr::snSynthCommon(part);
    case ToneSection::SnMisc: return addr::snSynthMisc(part);
    case ToneSection::SnPartial: return addr::snSynthPartial(part, index);
    case ToneSection::PcmCommon: return addr::pcmSynthCommon(part);
    case ToneSection::PcmPmt: return addr::pcmSynthPmt(part);
    case ToneSection::PcmCommon2: return addr::pcmSynthCommon2(part);
    case ToneSection::PcmPartial: return addr::pcmSynthPartial(part, index);
    case ToneSection::SnAcousticCommon: return addr::snAcousticCommon(part);
    }
    return {};
}

bool RolandFAPlatform::readToneSection(int part, ToneEngine engine, ToneSection section,
                                       int index, int size, QByteArray *data, QString *error)
{
    return m_engine && m_engine->read(address(part, engine, section, index), size, data, error);
}

bool RolandFAPlatform::writeToneSection(int part, ToneEngine engine, ToneSection section,
                                        int index, const QByteArray &data, QString *error)
{
    return m_engine && m_engine->write(address(part, engine, section, index), data, error);
}

bool RolandFAPlatform::writeToneParameter(int part, ToneEngine engine, ToneSection section,
                                          int index, int offset, const QByteArray &data, QString *error)
{
    return m_engine && m_engine->writeParam(addOffset(address(part, engine, section, index), offset), data, error);
}
