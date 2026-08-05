#include "platform/RolandFAPlatform.h"
#include "midi/SysexEngine.h"

using namespace roland;

bool RolandFAPlatform::isConnected() const { return m_engine && m_engine->isOpen(); }

bool RolandFAPlatform::recallStudioSet(int msb, int lsb, int program, QString *error)
{
    if (!isConnected()) { if (error) *error = QStringLiteral("Not connected"); return false; }
    if (!m_engine->writeParam(Address{{0x01, 0x00, 0x00, 0x00}}, QByteArray(1, char(1)), error))
        return false;
    QByteArray select; select.append(char(msb & 0x7f)); select.append(char(lsb & 0x7f)); select.append(char(program & 0x7f));
    return m_engine->write(Address{{0x01, 0x00, 0x00, 0x04}}, select, error);
}

bool RolandFAPlatform::readTemporaryStudioSetName(QString *name, QString *error)
{
    QByteArray data;
    if (!m_engine || !m_engine->read(addr::kStudioSetCommon, 16, &data, error, 2500)) return false;
    if (name) *name = QString::fromLatin1(data.constData(), qMin(16, data.size())).trimmed();
    return true;
}

Address RolandFAPlatform::studioAddress(StudioBlock b, int i) const
{
    switch (b) {
    case StudioBlock::Common: return addr::kStudioSetCommon; case StudioBlock::Chorus: return addr::kStudioSetChorus;
    case StudioBlock::Reverb: return addr::kStudioSetReverb; case StudioBlock::Ifx: return addr::kStudioSetIfx;
    case StudioBlock::MasterComp: return addr::kStudioSetMasterComp; case StudioBlock::Controller: return addr::kStudioSetController;
    case StudioBlock::PadCommon: return addr::kStudioSetPadCommon; case StudioBlock::Midi: return addr::midiChannel(i);
    case StudioBlock::Part: return addr::part(i); case StudioBlock::PartEq: return addr::partEq(i);
    case StudioBlock::Zone: return addr::zone(i); case StudioBlock::Pad: return addr::pad(i);
    } return {};
}
bool RolandFAPlatform::readStudioBlock(StudioBlock b,int i,int s,QByteArray*d,QString*e){return m_engine&&m_engine->read(studioAddress(b,i),s,d,e);}
bool RolandFAPlatform::writeStudioBlock(StudioBlock b,int i,const QByteArray&d,QString*e){return m_engine&&m_engine->write(studioAddress(b,i),d,e);}
bool RolandFAPlatform::writeStudioParameter(StudioBlock b,int i,int o,const QByteArray&d,QString*e){return m_engine&&m_engine->writeParam(addOffset(studioAddress(b,i),o),d,e);}
bool RolandFAPlatform::readMasterEq(QByteArray*d,QString*e){return m_engine&&m_engine->read(addr::kSystemMasterEq,sysOff::MasterEqSize,d,e);}
bool RolandFAPlatform::writeMasterEq(const QByteArray&d,QString*e){return m_engine&&m_engine->write(addr::kSystemMasterEq,d,e);}

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
