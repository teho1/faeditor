#include "app/FantomAppController.h"
#include "midi/SysexEngine.h"
#include "midi/MidiDeviceModel.h"
#include "model/FantomSceneModel.h"
#include "platform/RolandFantomPlatform.h"

FantomAppController::FantomAppController(QObject *parent):QObject(parent),
    m_engine(std::make_unique<SysexEngine>()),m_platform(std::make_unique<RolandFantomPlatform>(m_engine.get())),
    m_midi(std::make_unique<MidiDeviceModel>(m_platform.get())),m_scene(std::make_unique<FantomSceneModel>(m_platform.get()))
{
    connect(m_midi.get(),&MidiDeviceModel::statusTextChanged,this,&FantomAppController::deviceProfileChanged);
}
FantomAppController::~FantomAppController()=default;
QString FantomAppController::deviceProfile()const{return m_platform->profile().model;}
void FantomAppController::previewNote(int zone,int note,int velocity,bool on)
{
    QString error;m_platform->sendPreviewNote(qBound(0,zone,15),qBound(0,note,127),qBound(0,velocity,127),on,&error);
}
