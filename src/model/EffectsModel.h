#pragma once

#include <QObject>
#include <QByteArray>
#include <QStringList>

class EffectsModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int chorusType READ chorusType WRITE setChorusType NOTIFY effectsChanged)
    Q_PROPERTY(int chorusLevel READ chorusLevel WRITE setChorusLevel NOTIFY effectsChanged)
    Q_PROPERTY(int reverbType READ reverbType WRITE setReverbType NOTIFY effectsChanged)
    Q_PROPERTY(int reverbLevel READ reverbLevel WRITE setReverbLevel NOTIFY effectsChanged)
    Q_PROPERTY(bool masterCompSwitch READ masterCompSwitch WRITE setMasterCompSwitch NOTIFY effectsChanged)
    Q_PROPERTY(int masterCompAttack READ masterCompAttack WRITE setMasterCompAttack NOTIFY effectsChanged)
    Q_PROPERTY(int masterCompRelease READ masterCompRelease WRITE setMasterCompRelease NOTIFY effectsChanged)
    Q_PROPERTY(int masterCompThreshold READ masterCompThreshold WRITE setMasterCompThreshold NOTIFY effectsChanged)
    Q_PROPERTY(int masterCompRatio READ masterCompRatio WRITE setMasterCompRatio NOTIFY effectsChanged)
    Q_PROPERTY(int masterCompGain READ masterCompGain WRITE setMasterCompGain NOTIFY effectsChanged)
    Q_PROPERTY(QStringList chorusTypeNames READ chorusTypeNames CONSTANT)
    Q_PROPERTY(QStringList reverbTypeNames READ reverbTypeNames CONSTANT)

public:
    explicit EffectsModel(QObject *parent = nullptr);

    int chorusType() const { return m_chorusType; }
    int chorusLevel() const { return m_chorusLevel; }
    int reverbType() const { return m_reverbType; }
    int reverbLevel() const { return m_reverbLevel; }
    bool masterCompSwitch() const { return m_masterCompSwitch; }
    int masterCompAttack() const { return m_masterCompAttack; }
    int masterCompRelease() const { return m_masterCompRelease; }
    int masterCompThreshold() const { return m_masterCompThreshold; }
    int masterCompRatio() const { return m_masterCompRatio; }
    int masterCompGain() const { return m_masterCompGain; }
    QStringList chorusTypeNames() const;
    QStringList reverbTypeNames() const;

    void setChorusType(int v);
    void setChorusLevel(int v);
    void setReverbType(int v);
    void setReverbLevel(int v);
    void setMasterCompSwitch(bool v);
    void setMasterCompAttack(int v);
    void setMasterCompRelease(int v);
    void setMasterCompThreshold(int v);
    void setMasterCompRatio(int v);
    void setMasterCompGain(int v);

    void loadChorus(const QByteArray &data);
    void loadReverb(const QByteArray &data);
    void loadMasterComp(const QByteArray &data);
    /** Keep deep SysEx bytes from library blobs without overwriting typed UI fields. */
    void setRawChorus(const QByteArray &data);
    void setRawReverb(const QByteArray &data);
    void setRawMasterComp(const QByteArray &data);
    QByteArray chorusBytes() const;
    QByteArray reverbBytes() const;
    QByteArray masterCompBytes() const;

    void setFromDevice(bool v) { m_fromDevice = v; }

signals:
    void effectsChanged();
    void parameterEdited(const QString &section, const QString &param, int value);

private:
    void emitEdit(const QString &section, const QString &param, int value);
    bool m_fromDevice = false;
    QByteArray m_rawChorus;
    QByteArray m_rawReverb;
    QByteArray m_rawMasterComp;
    int m_chorusType = 0;
    int m_chorusLevel = 0;
    int m_reverbType = 0;
    int m_reverbLevel = 0;
    bool m_masterCompSwitch = false;
    int m_masterCompAttack = 0;
    int m_masterCompRelease = 0;
    int m_masterCompThreshold = 0;
    int m_masterCompRatio = 0;
    int m_masterCompGain = 0;
};
