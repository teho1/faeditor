#pragma once

#include "midi/AddressMap.h"

#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <array>

class InstrumentPlatform;
class WaveformCatalog;

/** SuperNATURAL Synth Temporary Tone — Common + 3 partials (MVP fields). */
class SnSynthToneModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString toneName READ toneName WRITE setToneName NOTIFY snChanged)
    Q_PROPERTY(int toneLevel READ toneLevel WRITE setToneLevel NOTIFY snChanged)
    Q_PROPERTY(bool monoSwitch READ monoSwitch WRITE setMonoSwitch NOTIFY snChanged)
    Q_PROPERTY(int selectedPartial READ selectedPartial WRITE setSelectedPartial NOTIFY selectedPartialChanged)

    Q_PROPERTY(int oscWave READ oscWave WRITE setOscWave NOTIFY snChanged)
    Q_PROPERTY(int oscWaveVariation READ oscWaveVariation WRITE setOscWaveVariation NOTIFY snChanged)
    Q_PROPERTY(int oscPitch READ oscPitch WRITE setOscPitch NOTIFY snChanged)
    Q_PROPERTY(int oscDetune READ oscDetune WRITE setOscDetune NOTIFY snChanged)
    Q_PROPERTY(int oscPulseWidth READ oscPulseWidth WRITE setOscPulseWidth NOTIFY snChanged)
    /** OSC Wave == PCM (7): sample select. No Wave Group on SN-S partials. */
    Q_PROPERTY(bool oscIsPcmWave READ oscIsPcmWave NOTIFY snChanged)
    Q_PROPERTY(int oscWaveNumber READ oscWaveNumber WRITE setOscWaveNumber NOTIFY snChanged)
    Q_PROPERTY(QString oscWaveName READ oscWaveName NOTIFY snChanged)
    Q_PROPERTY(int oscWaveGain READ oscWaveGain WRITE setOscWaveGain NOTIFY snChanged)
    Q_PROPERTY(QStringList oscWaveGainNames READ oscWaveGainNames CONSTANT)

    Q_PROPERTY(int filterMode READ filterMode WRITE setFilterMode NOTIFY snChanged)
    Q_PROPERTY(int filterCutoff READ filterCutoff WRITE setFilterCutoff NOTIFY snChanged)
    Q_PROPERTY(int filterResonance READ filterResonance WRITE setFilterResonance NOTIFY snChanged)
    Q_PROPERTY(int filterEnvAttack READ filterEnvAttack WRITE setFilterEnvAttack NOTIFY snChanged)
    Q_PROPERTY(int filterEnvDecay READ filterEnvDecay WRITE setFilterEnvDecay NOTIFY snChanged)
    Q_PROPERTY(int filterEnvSustain READ filterEnvSustain WRITE setFilterEnvSustain NOTIFY snChanged)
    Q_PROPERTY(int filterEnvRelease READ filterEnvRelease WRITE setFilterEnvRelease NOTIFY snChanged)

    Q_PROPERTY(int ampLevel READ ampLevel WRITE setAmpLevel NOTIFY snChanged)
    Q_PROPERTY(int ampEnvAttack READ ampEnvAttack WRITE setAmpEnvAttack NOTIFY snChanged)
    Q_PROPERTY(int ampEnvDecay READ ampEnvDecay WRITE setAmpEnvDecay NOTIFY snChanged)
    Q_PROPERTY(int ampEnvSustain READ ampEnvSustain WRITE setAmpEnvSustain NOTIFY snChanged)
    Q_PROPERTY(int ampEnvRelease READ ampEnvRelease WRITE setAmpEnvRelease NOTIFY snChanged)
    Q_PROPERTY(int ampPan READ ampPan WRITE setAmpPan NOTIFY snChanged)

    Q_PROPERTY(int lfoShape READ lfoShape WRITE setLfoShape NOTIFY snChanged)
    Q_PROPERTY(int lfoRate READ lfoRate WRITE setLfoRate NOTIFY snChanged)
    Q_PROPERTY(int lfoPitchDepth READ lfoPitchDepth WRITE setLfoPitchDepth NOTIFY snChanged)
    Q_PROPERTY(int lfoFilterDepth READ lfoFilterDepth WRITE setLfoFilterDepth NOTIFY snChanged)
    Q_PROPERTY(int lfoAmpDepth READ lfoAmpDepth WRITE setLfoAmpDepth NOTIFY snChanged)

    Q_PROPERTY(QStringList oscWaveNames READ oscWaveNames CONSTANT)
    Q_PROPERTY(QStringList filterModeNames READ filterModeNames CONSTANT)
    Q_PROPERTY(QStringList lfoShapeNames READ lfoShapeNames CONSTANT)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit SnSynthToneModel(InstrumentPlatform *platform, QObject *parent = nullptr);

    void setWaveformCatalog(WaveformCatalog *catalog);

    Q_INVOKABLE QString partialDisplayName(int index) const;

    QString toneName() const { return m_toneName; }
    int toneLevel() const { return m_toneLevel; }
    bool monoSwitch() const { return m_monoSwitch; }
    int selectedPartial() const { return m_selectedPartial; }

    int oscWave() const;
    int oscWaveVariation() const;
    int oscPitch() const;
    int oscDetune() const;
    int oscPulseWidth() const;
    bool oscIsPcmWave() const;
    int oscWaveNumber() const;
    QString oscWaveName() const;
    int oscWaveGain() const;
    int filterMode() const;
    int filterCutoff() const;
    int filterResonance() const;
    int filterEnvAttack() const;
    int filterEnvDecay() const;
    int filterEnvSustain() const;
    int filterEnvRelease() const;
    int ampLevel() const;
    int ampEnvAttack() const;
    int ampEnvDecay() const;
    int ampEnvSustain() const;
    int ampEnvRelease() const;
    int ampPan() const;
    int lfoShape() const;
    int lfoRate() const;
    int lfoPitchDepth() const;
    int lfoFilterDepth() const;
    int lfoAmpDepth() const;

    QStringList oscWaveNames() const;
    QStringList oscWaveGainNames() const;
    QStringList filterModeNames() const;
    QStringList lfoShapeNames() const;
    QString lastError() const { return m_lastError; }

    void setToneName(const QString &v);
    void setToneLevel(int v);
    void setMonoSwitch(bool v);
    void setSelectedPartial(int v);

    void setOscWave(int v);
    void setOscWaveVariation(int v);
    void setOscPitch(int v);
    void setOscDetune(int v);
    void setOscPulseWidth(int v);
    void setOscWaveNumber(int v);
    void setOscWaveGain(int v);
    void setFilterMode(int v);
    void setFilterCutoff(int v);
    void setFilterResonance(int v);
    void setFilterEnvAttack(int v);
    void setFilterEnvDecay(int v);
    void setFilterEnvSustain(int v);
    void setFilterEnvRelease(int v);
    void setAmpLevel(int v);
    void setAmpEnvAttack(int v);
    void setAmpEnvDecay(int v);
    void setAmpEnvSustain(int v);
    void setAmpEnvRelease(int v);
    void setAmpPan(int v);
    void setLfoShape(int v);
    void setLfoRate(int v);
    void setLfoPitchDepth(int v);
    void setLfoFilterDepth(int v);
    void setLfoAmpDepth(int v);

    void setPartIndex(int partIndex) { m_partIndex = partIndex; }
    int partIndex() const { return m_partIndex; }

    bool pullFromDevice();
    bool pushToDevice();
    void loadInitTemplate();
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &obj);

signals:
    void snChanged();
    void selectedPartialChanged();
    void lastErrorChanged();

private:
    struct Partial {
        QByteArray raw;
    };

    int partialByte(quint8 offset) const;
    void setPartialByte(quint8 offset, int value, int lo, int hi);
    int partialNibbleValue(int offset, int nibbleCount) const;
    void setPartialNibbleValue(int offset, int nibbleCount, int value, int lo, int hi);
    void writeCommonByte(quint8 offset, int value);
    void writePartialByte(int partial, quint8 offset, int value);
    void writePartialBytes(int partial, int offset, const QByteArray &data);
    void syncCommonFromRaw();
    void setError(const QString &e);

    InstrumentPlatform *m_platform = nullptr;
    WaveformCatalog *m_waves = nullptr;
    bool m_fromDevice = false;
    int m_partIndex = 0;
    int m_selectedPartial = 0;
    QString m_lastError;
    QByteArray m_common;
    QByteArray m_misc;
    std::array<Partial, 3> m_partials;
    QString m_toneName;
    int m_toneLevel = 100;
    bool m_monoSwitch = false;
};
