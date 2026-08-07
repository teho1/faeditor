#include "model/FantomSceneModel.h"

FantomSceneModel::FantomSceneModel(InstrumentPlatform *platform,QObject *parent)
    : QAbstractListModel(parent),m_platform(platform) {}

int FantomSceneModel::rowCount(const QModelIndex &parent) const{return parent.isValid()?0:16;}
int FantomSceneModel::byte(const QByteArray &v,int i){return i>=0&&i<v.size()?quint8(v[i]):0;}

QVariant FantomSceneModel::data(const QModelIndex &idx,int role) const
{
    if(!idx.isValid()||idx.row()<0||idx.row()>=16)return {};
    const auto &z=m_zones[size_t(idx.row())];
    switch(role){
    case NumberRole:return idx.row()+1;case ToneMsbRole:return byte(z.main,0);case ToneLsbRole:return byte(z.main,1);
    case ToneProgramRole:return byte(z.main,2)+1;case ChannelRole:return byte(z.main,3)+1;case ReceiveRole:return byte(z.main,4)!=0;
    case MuteRole:return byte(z.main,5)!=0;case LevelRole:return byte(z.main,7);case PanRole:return byte(z.main,8)-64;
    case ChorusRole:return byte(z.main,0x23);case ReverbRole:return byte(z.main,0x24);case OutputRole:return byte(z.main,0x25);
    case KeyboardRole:return byte(z.control,0)!=0;case KeyLowRole:return byte(z.control,4);case KeyHighRole:return byte(z.control,5);
    case VelocityLowRole:return byte(z.control,6);case VelocityHighRole:return byte(z.control,7);case EqEnabledRole:return byte(z.eq,8)!=0;
    default:return {};
    }
}

QHash<int,QByteArray> FantomSceneModel::roleNames() const
{
    return {{NumberRole,"zoneNumber"},{ToneMsbRole,"toneMsb"},{ToneLsbRole,"toneLsb"},{ToneProgramRole,"toneProgram"},
            {ChannelRole,"midiChannel"},{ReceiveRole,"receiveEnabled"},{MuteRole,"muted"},{LevelRole,"level"},{PanRole,"pan"},
            {ChorusRole,"chorusSend"},{ReverbRole,"reverbSend"},{OutputRole,"outputAssign"},{KeyboardRole,"keyboardEnabled"},
            {KeyLowRole,"keyLow"},{KeyHighRole,"keyHigh"},{VelocityLowRole,"velocityLow"},{VelocityHighRole,"velocityHigh"},
            {EqEnabledRole,"eqEnabled"}};
}
Qt::ItemFlags FantomSceneModel::flags(const QModelIndex &i)const{return QAbstractListModel::flags(i)|Qt::ItemIsEditable;}

void FantomSceneModel::setStatus(QString v){if(m_status==v)return;m_status=std::move(v);emit statusChanged();}
void FantomSceneModel::setBusy(bool v){if(m_busy==v)return;m_busy=v;emit busyChanged();}

bool FantomSceneModel::pull()
{
    if(!m_platform||!m_platform->isConnected()){setStatus(QStringLiteral("Connect the FANTOM first"));return false;}
    setBusy(true);QString error;QByteArray common;
    if(!m_platform->readStudioBlock(InstrumentPlatform::StudioBlock::Common,0,150,&common,&error)){setBusy(false);setStatus(error);return false;}
    std::array<Zone,16> pulled;
    for(int i=0;i<16;++i){
        if(!m_platform->readStudioBlock(InstrumentPlatform::StudioBlock::Part,i,73,&pulled[size_t(i)].main,&error)
           ||!m_platform->readStudioBlock(InstrumentPlatform::StudioBlock::PartEq,i,9,&pulled[size_t(i)].eq,&error)
           ||!m_platform->readStudioBlock(InstrumentPlatform::StudioBlock::Zone,i,112,&pulled[size_t(i)].control,&error)){
            setBusy(false);setStatus(QStringLiteral("Zone %1: %2").arg(i+1).arg(error));return false;
        }
    }
    beginResetModel();m_common=common;m_zones=std::move(pulled);endResetModel();
    QByteArray nameBytes=m_common.left(16);const int nul=nameBytes.indexOf('\0');if(nul>=0)nameBytes.truncate(nul);
    m_name=QString::fromLatin1(nameBytes).trimmed();emit nameChanged();
    if(m_edited){m_edited=false;emit editedChanged();}setBusy(false);setStatus(QStringLiteral("Temporary Scene pulled — edits remain temporary until saved on the instrument"));return true;
}

bool FantomSceneModel::push()
{
    if(!m_platform||!m_platform->isConnected()){setStatus(QStringLiteral("Connect the FANTOM first"));return false;}
    setBusy(true);QString error;
    if(!m_platform->writeStudioBlock(InstrumentPlatform::StudioBlock::Common,0,m_common,&error)){setBusy(false);setStatus(error);return false;}
    for(int i=0;i<16;++i){const auto &z=m_zones[size_t(i)];
        if(!m_platform->writeStudioBlock(InstrumentPlatform::StudioBlock::Part,i,z.main,&error)
           ||!m_platform->writeStudioBlock(InstrumentPlatform::StudioBlock::PartEq,i,z.eq,&error)
           ||!m_platform->writeStudioBlock(InstrumentPlatform::StudioBlock::Zone,i,z.control,&error)){
            setBusy(false);setStatus(QStringLiteral("Zone %1: %2").arg(i+1).arg(error));return false;}}
    if(m_edited){m_edited=false;emit editedChanged();}setBusy(false);setStatus(QStringLiteral("Scene pushed to Temporary memory — use Save on FANTOM to keep it"));return true;
}

bool FantomSceneModel::liveWrite(int zone,InstrumentPlatform::StudioBlock block,int offset,int value)
{
    if(zone<0||zone>=16||!m_platform||!m_platform->isConnected()){setStatus(QStringLiteral("Connect the FANTOM first"));return false;}
    QString error;QByteArray d(1,char(qBound(0,value,127)));
    if(!m_platform->writeStudioParameter(block,zone,offset,d,&error)){setStatus(error);return false;}
    if(!m_edited){m_edited=true;emit editedChanged();}emit dataChanged(index(zone),index(zone));return true;
}

bool FantomSceneModel::setZoneValue(int zone,const QString &p,int value)
{
    if(zone<0||zone>=16)return false;auto &z=m_zones[size_t(zone)];int off=-1;auto block=InstrumentPlatform::StudioBlock::Part;int raw=value;
    if(p==QStringLiteral("level"))off=7;else if(p==QStringLiteral("pan")){off=8;raw=value+64;}else if(p==QStringLiteral("mute"))off=5;
    else if(p==QStringLiteral("channel")){off=3;raw=value-1;}else if(p==QStringLiteral("receive"))off=4;
    else if(p==QStringLiteral("chorus"))off=0x23;else if(p==QStringLiteral("reverb"))off=0x24;
    else if(p==QStringLiteral("output"))off=0x25;
    else {block=InstrumentPlatform::StudioBlock::Zone;if(p==QStringLiteral("keyboard"))off=0;else if(p==QStringLiteral("keyLow"))off=4;
          else if(p==QStringLiteral("keyHigh"))off=5;else if(p==QStringLiteral("velocityLow"))off=6;else if(p==QStringLiteral("velocityHigh"))off=7;}
    if(off<0)return false;QByteArray &bytes=block==InstrumentPlatform::StudioBlock::Part?z.main:z.control;if(bytes.size()<=off)bytes.resize(off+1);bytes[off]=char(qBound(0,raw,127));
    return liveWrite(zone,block,off,raw);
}

bool FantomSceneModel::assignTone(int zone,int msb,int lsb,int program)
{
    if(zone<0||zone>=16)return false;auto &v=m_zones[size_t(zone)].main;v[0]=char(msb&127);v[1]=char(lsb&127);v[2]=char(qBound(1,program,128)-1);
    QString error;if(!m_platform||!m_platform->writeStudioParameter(InstrumentPlatform::StudioBlock::Part,zone,0,v.left(3),&error)){setStatus(error.isEmpty()?QStringLiteral("Connect the FANTOM first"):error);return false;}
    if(!m_edited){m_edited=true;emit editedChanged();}emit dataChanged(index(zone),index(zone));return true;
}

bool FantomSceneModel::setData(const QModelIndex &idx,const QVariant &value,int role)
{
    if(!idx.isValid())return false;
    switch(role){case LevelRole:return setZoneValue(idx.row(),QStringLiteral("level"),value.toInt());case PanRole:return setZoneValue(idx.row(),QStringLiteral("pan"),value.toInt());case MuteRole:return setZoneValue(idx.row(),QStringLiteral("mute"),value.toBool());default:return false;}
}
