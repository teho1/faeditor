#pragma once

#include <QObject>

class MfxModel;

/**
 * Typed R/W view of MFX type 36 (3Tap Pan Delay) params.
 * Individual properties + NOTIFY — reliable QML bindings (SN-A / PCM style).
 * Writes go through MfxModel::setParamValue / chorusSend / reverbSend.
 */
class MfxTapDelayModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)

    Q_PROPERTY(int leftTime READ leftTime WRITE setLeftTime NOTIFY leftTimeChanged)
    Q_PROPERTY(int rightTime READ rightTime WRITE setRightTime NOTIFY rightTimeChanged)
    Q_PROPERTY(int centerTime READ centerTime WRITE setCenterTime NOTIFY centerTimeChanged)
    Q_PROPERTY(int leftLevel READ leftLevel WRITE setLeftLevel NOTIFY leftLevelChanged)
    Q_PROPERTY(int rightLevel READ rightLevel WRITE setRightLevel NOTIFY rightLevelChanged)
    Q_PROPERTY(int centerLevel READ centerLevel WRITE setCenterLevel NOTIFY centerLevelChanged)

    Q_PROPERTY(int feedback READ feedback WRITE setFeedback NOTIFY feedbackChanged)
    Q_PROPERTY(int hfDamp READ hfDamp WRITE setHfDamp NOTIFY hfDampChanged)
    Q_PROPERTY(bool hfDampBypass READ hfDampBypass WRITE setHfDampBypass NOTIFY hfDampBypassChanged)
    Q_PROPERTY(qreal hfDampNorm READ hfDampNorm NOTIFY hfDampChanged)

    Q_PROPERTY(int lowGain READ lowGain WRITE setLowGain NOTIFY lowGainChanged)
    Q_PROPERTY(int highGain READ highGain WRITE setHighGain NOTIFY highGainChanged)
    Q_PROPERTY(int balance READ balance WRITE setBalance NOTIFY balanceChanged)
    Q_PROPERTY(int level READ level WRITE setLevel NOTIFY levelChanged)
    Q_PROPERTY(int chorusSend READ chorusSend WRITE setChorusSend NOTIFY chorusSendChanged)
    Q_PROPERTY(int reverbSend READ reverbSend WRITE setReverbSend NOTIFY reverbSendChanged)

    Q_PROPERTY(QString feedbackLabel READ feedbackLabel NOTIFY feedbackChanged)
    Q_PROPERTY(QString hfDampLabel READ hfDampLabel NOTIFY hfDampChanged)
    Q_PROPERTY(QString balanceLabel READ balanceLabel NOTIFY balanceChanged)

public:
    explicit MfxTapDelayModel(MfxModel *mfx, QObject *parent = nullptr);

    bool active() const;

    int leftTime() const;
    int rightTime() const;
    int centerTime() const;
    int leftLevel() const;
    int rightLevel() const;
    int centerLevel() const;
    int feedback() const;
    int hfDamp() const;
    bool hfDampBypass() const;
    qreal hfDampNorm() const;
    int lowGain() const;
    int highGain() const;
    int balance() const;
    int level() const;
    int chorusSend() const;
    int reverbSend() const;

    QString feedbackLabel() const;
    QString hfDampLabel() const;
    QString balanceLabel() const;

    void setLeftTime(int v);
    void setRightTime(int v);
    void setCenterTime(int v);
    void setLeftLevel(int v);
    void setRightLevel(int v);
    void setCenterLevel(int v);
    void setFeedback(int v);
    void setHfDamp(int v);
    void setHfDampBypass(bool on);
    void setLowGain(int v);
    void setHighGain(int v);
    void setBalance(int v);
    void setLevel(int v);
    void setChorusSend(int v);
    void setReverbSend(int v);

    Q_INVOKABLE int timeOf(int tap) const;
    Q_INVOKABLE int levelOf(int tap) const;
    Q_INVOKABLE void setTime(int tap, int ms);
    Q_INVOKABLE void setLevelAt(int tap, int level);
    Q_INVOKABLE int spinTimeValue(int tap) const;
    Q_INVOKABLE bool isDelayNote(int tap) const;
    Q_INVOKABLE QString formatTime(int tap) const;
    Q_INVOKABLE qreal hfDampCurveMagnitude(qreal tNorm) const;
    Q_INVOKABLE void setHfDampFromNorm(qreal n);
    Q_INVOKABLE int delayTimeForAxis(int v) const;
    Q_INVOKABLE int timeFromAxisNorm(qreal nx) const;
    Q_INVOKABLE int levelFromAxisNorm(qreal ny) const;

signals:
    void activeChanged();
    void leftTimeChanged();
    void rightTimeChanged();
    void centerTimeChanged();
    void leftLevelChanged();
    void rightLevelChanged();
    void centerLevelChanged();
    void feedbackChanged();
    void hfDampChanged();
    void hfDampBypassChanged();
    void lowGainChanged();
    void highGainChanged();
    void balanceChanged();
    void levelChanged();
    void chorusSendChanged();
    void reverbSendChanged();
    void tapChanged();

private:
    void syncFromMfx();
    void maybeSeedDemoDefaults();
    int rawParam(int index) const;
    void writeParam(int index, int value);

    MfxModel *m_mfx = nullptr;
    bool m_inSync = false;
    bool m_active = false;
    int m_leftTime = 0;
    int m_rightTime = 0;
    int m_centerTime = 0;
    int m_leftLevel = 0;
    int m_rightLevel = 0;
    int m_centerLevel = 0;
    int m_feedback = 0;
    int m_hfDamp = 8001;
    int m_lowGain = 0;
    int m_highGain = 0;
    int m_balance = 50;
    int m_level = 0;
    int m_chorusSend = 0;
    int m_reverbSend = 0;
};
