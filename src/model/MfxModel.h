#pragma once

#include "midi/AddressMap.h"
#include "model/MfxParamListModel.h"
#include "model/MfxTapDelayModel.h"

#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <QVariantList>

class InstrumentPlatform;

/** Shared Temporary Tone MFX block (145 B) + Switch. */
class MfxModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool mfxSwitch READ mfxSwitch WRITE setMfxSwitch NOTIFY mfxChanged)
    Q_PROPERTY(int type READ type WRITE setType NOTIFY mfxChanged)
    Q_PROPERTY(int chorusSend READ chorusSend WRITE setChorusSend NOTIFY mfxChanged)
    Q_PROPERTY(int reverbSend READ reverbSend WRITE setReverbSend NOTIFY mfxChanged)
    Q_PROPERTY(QStringList typeNames READ typeNames CONSTANT)
    Q_PROPERTY(QString typeName READ typeName NOTIFY mfxChanged)
    Q_PROPERTY(int paramCount READ paramCount NOTIFY mfxChanged)
    Q_PROPERTY(QVariantList paramCatalog READ paramCatalog NOTIFY mfxChanged)
    Q_PROPERTY(QVariantList paramValues READ paramValues NOTIFY mfxChanged)
    Q_PROPERTY(QVariantList paramMinValues READ paramMinValues NOTIFY mfxChanged)
    Q_PROPERTY(QVariantList paramMaxValues READ paramMaxValues NOTIFY mfxChanged)
    Q_PROPERTY(QString uiFamily READ uiFamily NOTIFY mfxChanged)
    Q_PROPERTY(bool usesVisualTemplate READ usesVisualTemplate NOTIFY mfxChanged)
    Q_PROPERTY(MfxTapDelayModel *tapDelay READ tapDelay CONSTANT)
    Q_PROPERTY(MfxParamListModel *paramList READ paramList CONSTANT)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit MfxModel(InstrumentPlatform *platform, QObject *parent = nullptr);

    bool mfxSwitch() const { return m_switch; }
    int type() const { return m_type; }
    int chorusSend() const { return m_chorusSend; }
    int reverbSend() const { return m_reverbSend; }
    QStringList typeNames() const;
    QString typeName() const;
    int paramCount() const;
    QVariantList paramCatalog() const;
    QVariantList paramValues() const;
    QVariantList paramMinValues() const;
    QVariantList paramMaxValues() const;
    QString uiFamily() const;
    bool usesVisualTemplate() const;
    MfxTapDelayModel *tapDelay() const { return m_tapDelay; }
    MfxParamListModel *paramList() const { return m_paramList; }
    QString lastError() const { return m_lastError; }

    void setMfxSwitch(bool v);
    void setType(int v);
    void setChorusSend(int v);
    void setReverbSend(int v);

    Q_INVOKABLE int paramValue(int index) const;
    Q_INVOKABLE void setParamValue(int index, int value);
    Q_INVOKABLE QString paramName(int index) const;
    Q_INVOKABLE int paramMin(int index) const;
    Q_INVOKABLE int paramMax(int index) const;
    Q_INVOKABLE QStringList paramEnumNames(int index) const;
    Q_INVOKABLE bool paramHasEnum(int index) const;
    Q_INVOKABLE int paramIndexByName(const QString &name) const;
    Q_INVOKABLE bool applyPreset(const QJsonObject &preset);

    void setContext(int partIndex, roland::ToneEngine engine);
    int partIndex() const { return m_partIndex; }
    roland::ToneEngine toneEngine() const { return m_engineType; }

    bool pullFromDevice();
    bool pushToDevice();
    void loadBytes(const QByteArray &mfx145, bool switchOn);
    QByteArray mfxBytes() const;
    bool switchByte() const { return m_switch; }

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &obj);

    void setFromDevice(bool v) { m_fromDevice = v; }

signals:
    void mfxChanged();
    void lastErrorChanged();

private:
    void writeSwitch(bool on);
    void writeByte(quint8 offset, int value);
    void writeParam(int index, int logicalValue);
    void setError(const QString &e);
    static QByteArray encodeParam(int logicalValue);
    static int decodeParam(const QByteArray &four);

    InstrumentPlatform *m_platform = nullptr;
    bool m_fromDevice = false;
    int m_partIndex = 0;
    roland::ToneEngine m_engineType = roland::ToneEngine::Unknown;
    QString m_lastError;
    QByteArray m_raw;
    bool m_switch = false;
    int m_type = 0;
    int m_chorusSend = 0;
    int m_reverbSend = 0;
    MfxTapDelayModel *m_tapDelay = nullptr;
    MfxParamListModel *m_paramList = nullptr;
};
