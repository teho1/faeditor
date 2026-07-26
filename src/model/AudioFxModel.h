#pragma once

#include "midi/AddressMap.h"

#include <QJsonObject>
#include <QObject>
#include <QStringList>

class SysexEngine;

class AudioFxModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool inputReverbSwitch READ inputReverbSwitch WRITE setInputReverbSwitch NOTIFY audioFxChanged)
    Q_PROPERTY(int inputReverbType READ inputReverbType WRITE setInputReverbType NOTIFY audioFxChanged)
    Q_PROPERTY(int inputReverbTime READ inputReverbTime WRITE setInputReverbTime NOTIFY audioFxChanged)
    Q_PROPERTY(int inputReverbLevel READ inputReverbLevel WRITE setInputReverbLevel NOTIFY audioFxChanged)
    Q_PROPERTY(bool nsSwitch READ nsSwitch WRITE setNsSwitch NOTIFY audioFxChanged)
    Q_PROPERTY(int nsThreshold READ nsThreshold WRITE setNsThreshold NOTIFY audioFxChanged)
    Q_PROPERTY(int nsRelease READ nsRelease WRITE setNsRelease NOTIFY audioFxChanged)

    Q_PROPERTY(bool tfxSwitch READ tfxSwitch WRITE setTfxSwitch NOTIFY audioFxChanged)
    Q_PROPERTY(int tfxType READ tfxType WRITE setTfxType NOTIFY audioFxChanged)
    Q_PROPERTY(int tfxParamA READ tfxParamA WRITE setTfxParamA NOTIFY audioFxChanged)
    Q_PROPERTY(int tfxParamB READ tfxParamB WRITE setTfxParamB NOTIFY audioFxChanged)
    Q_PROPERTY(int tfxParamC READ tfxParamC WRITE setTfxParamC NOTIFY audioFxChanged)
    Q_PROPERTY(int tfxParamD READ tfxParamD WRITE setTfxParamD NOTIFY audioFxChanged)
    Q_PROPERTY(int tfxLocation READ tfxLocation WRITE setTfxLocation NOTIFY audioFxChanged)
    Q_PROPERTY(int tfxInputGain READ tfxInputGain WRITE setTfxInputGain NOTIFY audioFxChanged)

    Q_PROPERTY(QStringList tfxTypeNames READ tfxTypeNames CONSTANT)
    Q_PROPERTY(QStringList inputReverbTypeNames READ inputReverbTypeNames CONSTANT)
    Q_PROPERTY(QStringList tfxGainNames READ tfxGainNames CONSTANT)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString controllerStatus READ controllerStatus NOTIFY controllerStatusChanged)

public:
    explicit AudioFxModel(SysexEngine *engine, QObject *parent = nullptr);

    bool inputReverbSwitch() const { return m_inputReverbSwitch; }
    int inputReverbType() const { return m_inputReverbType; }
    int inputReverbTime() const { return m_inputReverbTime; }
    int inputReverbLevel() const { return m_inputReverbLevel; }
    bool nsSwitch() const { return m_nsSwitch; }
    int nsThreshold() const { return m_nsThreshold; }
    int nsRelease() const { return m_nsRelease; }
    bool tfxSwitch() const { return m_tfxSwitch; }
    int tfxType() const { return m_tfxType; }
    int tfxParamA() const { return m_tfxParamA; }
    int tfxParamB() const { return m_tfxParamB; }
    int tfxParamC() const { return m_tfxParamC; }
    int tfxParamD() const { return m_tfxParamD; }
    int tfxLocation() const { return m_tfxLocation; }
    int tfxInputGain() const { return m_tfxInputGain; }
    QStringList tfxTypeNames() const;
    QStringList inputReverbTypeNames() const;
    QStringList tfxGainNames() const;
    QString lastError() const { return m_lastError; }
    bool busy() const { return m_busy; }
    QString controllerStatus() const { return m_controllerStatus; }

    void setInputReverbSwitch(bool v);
    void setInputReverbType(int v);
    void setInputReverbTime(int v);
    void setInputReverbLevel(int v);
    void setNsSwitch(bool v);
    void setNsThreshold(int v);
    void setNsRelease(int v);
    void setTfxSwitch(bool v);
    void setTfxType(int v);
    void setTfxParamA(int v);
    void setTfxParamB(int v);
    void setTfxParamC(int v);
    void setTfxParamD(int v);
    void setTfxLocation(int v);
    void setTfxInputGain(int v);

    Q_INVOKABLE bool pullFromDevice();
    Q_INVOKABLE bool pushToDevice();
    Q_INVOKABLE bool assignDeviceControlsToTfx();

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &obj);

signals:
    void audioFxChanged();
    void lastErrorChanged();
    void busyChanged();
    void controllerStatusChanged();

private:
    void writeByte(const roland::Address &address, int value);
    void setError(const QString &e);
    void setBusy(bool v);
    void setControllerStatus(const QString &s);

    SysexEngine *m_engine = nullptr;
    bool m_fromDevice = false;
    bool m_busy = false;
    QString m_lastError;
    QString m_controllerStatus;

    bool m_inputReverbSwitch = false;
    int m_inputReverbType = 0;
    int m_inputReverbTime = 64;
    int m_inputReverbLevel = 40;
    bool m_nsSwitch = false;
    int m_nsThreshold = 40;
    int m_nsRelease = 40;
    bool m_tfxSwitch = false;
    int m_tfxType = 0;
    int m_tfxParamA = 64;
    int m_tfxParamB = 64;
    int m_tfxParamC = 64;
    int m_tfxParamD = 64;
    int m_tfxLocation = 1;
    int m_tfxInputGain = 6;
};
