#pragma once

#include <QObject>
#include <memory>
#include "midi/MidiDeviceModel.h"
#include "model/FantomSceneModel.h"

class SysexEngine;
class RolandFantomPlatform;

class FantomAppController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString productName READ productName CONSTANT)
    Q_PROPERTY(QString deviceProfile READ deviceProfile NOTIFY deviceProfileChanged)
    Q_PROPERTY(QString safetyNotice READ safetyNotice CONSTANT)
    Q_PROPERTY(MidiDeviceModel* midi READ midi CONSTANT)
    Q_PROPERTY(FantomSceneModel* scene READ scene CONSTANT)
public:
    explicit FantomAppController(QObject *parent=nullptr);
    ~FantomAppController() override;
    QString productName() const{return QStringLiteral("Fantom Editor");}
    QString deviceProfile() const;
    QString safetyNotice() const{return QStringLiteral("All Push and live edits target Temporary memory only. Save on the FANTOM to make changes permanent.");}
    MidiDeviceModel *midi() const{return m_midi.get();}
    FantomSceneModel *scene() const{return m_scene.get();}
    Q_INVOKABLE void previewNote(int zone,int note,int velocity,bool on);
signals:
    void deviceProfileChanged();
private:
    std::unique_ptr<SysexEngine> m_engine;
    std::unique_ptr<RolandFantomPlatform> m_platform;
    std::unique_ptr<MidiDeviceModel> m_midi;
    std::unique_ptr<FantomSceneModel> m_scene;
};
