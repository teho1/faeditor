#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>

class PartModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int partNumber READ partNumber CONSTANT)
    Q_PROPERTY(int receiveChannel READ receiveChannel WRITE setReceiveChannel NOTIFY partChanged)
    Q_PROPERTY(bool partSwitch READ partSwitch WRITE setPartSwitch NOTIFY partChanged)
    Q_PROPERTY(int bankMsb READ bankMsb WRITE setBankMsb NOTIFY partChanged)
    Q_PROPERTY(int bankLsb READ bankLsb WRITE setBankLsb NOTIFY partChanged)
    Q_PROPERTY(int program READ program WRITE setProgram NOTIFY partChanged)
    Q_PROPERTY(int level READ level WRITE setLevel NOTIFY partChanged)
    Q_PROPERTY(int pan READ pan WRITE setPan NOTIFY partChanged)
    Q_PROPERTY(int coarseTune READ coarseTune WRITE setCoarseTune NOTIFY partChanged)
    Q_PROPERTY(int octaveShift READ octaveShift WRITE setOctaveShift NOTIFY partChanged)
    Q_PROPERTY(int velocityLow READ velocityLow WRITE setVelocityLow NOTIFY partChanged)
    Q_PROPERTY(int velocityHigh READ velocityHigh WRITE setVelocityHigh NOTIFY partChanged)
    Q_PROPERTY(bool mute READ mute WRITE setMute NOTIFY partChanged)
    Q_PROPERTY(int chorusSend READ chorusSend WRITE setChorusSend NOTIFY partChanged)
    Q_PROPERTY(int reverbSend READ reverbSend WRITE setReverbSend NOTIFY partChanged)
    Q_PROPERTY(int outputAssign READ outputAssign WRITE setOutputAssign NOTIFY partChanged)
    Q_PROPERTY(int keyLow READ keyLow WRITE setKeyLow NOTIFY partChanged)
    Q_PROPERTY(int keyHigh READ keyHigh WRITE setKeyHigh NOTIFY partChanged)
    Q_PROPERTY(bool keyboardSwitch READ keyboardSwitch WRITE setKeyboardSwitch NOTIFY partChanged)
    Q_PROPERTY(QString toneName READ toneName WRITE setToneName NOTIFY partChanged)
    Q_PROPERTY(bool solo READ solo WRITE setSolo NOTIFY partChanged)

public:
    explicit PartModel(int partIndex, QObject *parent = nullptr);

    int partNumber() const { return m_index + 1; }
    int index() const { return m_index; }

    int receiveChannel() const { return m_receiveChannel; }
    bool partSwitch() const { return m_partSwitch; }
    int bankMsb() const { return m_bankMsb; }
    int bankLsb() const { return m_bankLsb; }
    int program() const { return m_program; }
    int level() const { return m_level; }
    int pan() const { return m_pan; }
    int coarseTune() const { return m_coarseTune; }
    int octaveShift() const { return m_octaveShift; }
    int velocityLow() const { return m_velocityLow; }
    int velocityHigh() const { return m_velocityHigh; }
    bool mute() const { return m_mute; }
    int chorusSend() const { return m_chorusSend; }
    int reverbSend() const { return m_reverbSend; }
    int outputAssign() const { return m_outputAssign; }
    int keyLow() const { return m_keyLow; }
    int keyHigh() const { return m_keyHigh; }
    bool keyboardSwitch() const { return m_keyboardSwitch; }
    QString toneName() const { return m_toneName; }
    bool solo() const { return m_solo; }

    void setReceiveChannel(int v);
    void setPartSwitch(bool v);
    void setBankMsb(int v);
    void setBankLsb(int v);
    void setProgram(int v);
    void setLevel(int v);
    void setPan(int v);
    void setCoarseTune(int v);
    void setOctaveShift(int v);
    void setVelocityLow(int v);
    void setVelocityHigh(int v);
    void setMute(bool v);
    void setChorusSend(int v);
    void setReverbSend(int v);
    void setOutputAssign(int v);
    void setKeyLow(int v);
    void setKeyHigh(int v);
    void setKeyboardSwitch(bool v);
    void setToneName(const QString &v);
    void setSolo(bool v);

    void loadFromPartBytes(const QByteArray &data);
    void loadFromZoneBytes(const QByteArray &data);
    /** Keep deep SysEx bytes from library blobs without overwriting typed UI fields. */
    void setRawPartBytes(const QByteArray &data);
    void setRawZoneBytes(const QByteArray &data);
    /** Overlay modeled fields onto last pulled raw block (never zero-wipe Receive Src etc.). */
    QByteArray toPartBytes() const;
    QByteArray toZoneBytes() const;

    /** Apply without emitting write-to-device (used when loading from FA). */
    void setFromDevice(bool enabled) { m_fromDevice = enabled; }

signals:
    void partChanged();
    void parameterEdited(int partIndex, const QString &param, int value);

private:
    void emitEdit(const QString &param, int value);
    QByteArray defaultPartTemplate() const;
    QByteArray defaultZoneTemplate() const;

    int m_index = 0;
    bool m_fromDevice = false;
    QByteArray m_rawPart;
    QByteArray m_rawZone;

    int m_receiveChannel = 0;
    bool m_partSwitch = true;
    int m_bankMsb = 87;
    int m_bankLsb = 64;
    int m_program = 0;
    int m_level = 100;
    int m_pan = 64;
    int m_coarseTune = 64; // stored raw 16-112 => -48..+48 with 64 center-ish; FA uses 16-112
    int m_octaveShift = 64; // raw 61-67 => -3..+3 with 64 = 0
    int m_velocityLow = 1;
    int m_velocityHigh = 127;
    bool m_mute = false;
    int m_chorusSend = 0;
    int m_reverbSend = 0;
    int m_outputAssign = 0;
    int m_keyLow = 0;
    int m_keyHigh = 127;
    bool m_keyboardSwitch = true;
    QString m_toneName;
    bool m_solo = false;
};
