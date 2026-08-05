#pragma once

#include "midi/AddressMap.h"

#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <array>

class InstrumentPlatform;
class WaveformCatalog;

/** PCM Synth Temporary Tone — Common + PMT + 4 partials (MVP fields). */
class PcmSynthToneModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString toneName READ toneName WRITE setToneName NOTIFY pcmChanged)
    Q_PROPERTY(int toneLevel READ toneLevel WRITE setToneLevel NOTIFY pcmChanged)
    Q_PROPERTY(int tonePan READ tonePan WRITE setTonePan NOTIFY pcmChanged)
    Q_PROPERTY(int selectedPartial READ selectedPartial WRITE setSelectedPartial NOTIFY selectedPartialChanged)

    Q_PROPERTY(int structureType12 READ structureType12 WRITE setStructureType12 NOTIFY pcmChanged)
    Q_PROPERTY(int structureType34 READ structureType34 WRITE setStructureType34 NOTIFY pcmChanged)
    Q_PROPERTY(bool partial1On READ partial1On WRITE setPartial1On NOTIFY pcmChanged)
    Q_PROPERTY(bool partial2On READ partial2On WRITE setPartial2On NOTIFY pcmChanged)
    Q_PROPERTY(bool partial3On READ partial3On WRITE setPartial3On NOTIFY pcmChanged)
    Q_PROPERTY(bool partial4On READ partial4On WRITE setPartial4On NOTIFY pcmChanged)

    Q_PROPERTY(int waveGroupType READ waveGroupType WRITE setWaveGroupType NOTIFY pcmChanged)
    Q_PROPERTY(int waveGroupId READ waveGroupId WRITE setWaveGroupId NOTIFY pcmChanged)
    /** UI bank: 0=INT-A, 1=INT-B, 2=SRX (maps Type + Group ID). */
    Q_PROPERTY(int waveGroupBank READ waveGroupBank WRITE setWaveGroupBank NOTIFY pcmChanged)
    Q_PROPERTY(int waveNumber READ waveNumber WRITE setWaveNumber NOTIFY pcmChanged)
    Q_PROPERTY(QString waveName READ waveName NOTIFY pcmChanged)
    Q_PROPERTY(int coarseTune READ coarseTune WRITE setCoarseTune NOTIFY pcmChanged)
    Q_PROPERTY(int fineTune READ fineTune WRITE setFineTune NOTIFY pcmChanged)

    Q_PROPERTY(int filterType READ filterType WRITE setFilterType NOTIFY pcmChanged)
    Q_PROPERTY(int filterCutoff READ filterCutoff WRITE setFilterCutoff NOTIFY pcmChanged)
    Q_PROPERTY(int filterResonance READ filterResonance WRITE setFilterResonance NOTIFY pcmChanged)
    Q_PROPERTY(int filterEnvAttack READ filterEnvAttack WRITE setFilterEnvAttack NOTIFY pcmChanged)
    Q_PROPERTY(int filterEnvDecay READ filterEnvDecay WRITE setFilterEnvDecay NOTIFY pcmChanged)
    Q_PROPERTY(int filterEnvSustain READ filterEnvSustain WRITE setFilterEnvSustain NOTIFY pcmChanged)
    Q_PROPERTY(int filterEnvRelease READ filterEnvRelease WRITE setFilterEnvRelease NOTIFY pcmChanged)

    Q_PROPERTY(int ampLevel READ ampLevel WRITE setAmpLevel NOTIFY pcmChanged)
    Q_PROPERTY(int ampPan READ ampPan WRITE setAmpPan NOTIFY pcmChanged)
    Q_PROPERTY(int ampEnvAttack READ ampEnvAttack WRITE setAmpEnvAttack NOTIFY pcmChanged)
    Q_PROPERTY(int ampEnvDecay READ ampEnvDecay WRITE setAmpEnvDecay NOTIFY pcmChanged)
    Q_PROPERTY(int ampEnvSustain READ ampEnvSustain WRITE setAmpEnvSustain NOTIFY pcmChanged)
    Q_PROPERTY(int ampEnvRelease READ ampEnvRelease WRITE setAmpEnvRelease NOTIFY pcmChanged)

    Q_PROPERTY(int lfoWaveform READ lfoWaveform WRITE setLfoWaveform NOTIFY pcmChanged)
    Q_PROPERTY(int lfoRate READ lfoRate WRITE setLfoRate NOTIFY pcmChanged)
    Q_PROPERTY(int lfoPitchDepth READ lfoPitchDepth WRITE setLfoPitchDepth NOTIFY pcmChanged)
    Q_PROPERTY(int lfoFilterDepth READ lfoFilterDepth WRITE setLfoFilterDepth NOTIFY pcmChanged)
    Q_PROPERTY(int lfoAmpDepth READ lfoAmpDepth WRITE setLfoAmpDepth NOTIFY pcmChanged)

    Q_PROPERTY(int matrix1Source READ matrix1Source WRITE setMatrix1Source NOTIFY pcmChanged)
    Q_PROPERTY(int matrix1Dest READ matrix1Dest WRITE setMatrix1Dest NOTIFY pcmChanged)
    Q_PROPERTY(int matrix1Sens READ matrix1Sens WRITE setMatrix1Sens NOTIFY pcmChanged)
    Q_PROPERTY(int matrix2Source READ matrix2Source WRITE setMatrix2Source NOTIFY pcmChanged)
    Q_PROPERTY(int matrix2Dest READ matrix2Dest WRITE setMatrix2Dest NOTIFY pcmChanged)
    Q_PROPERTY(int matrix2Sens READ matrix2Sens WRITE setMatrix2Sens NOTIFY pcmChanged)
    Q_PROPERTY(int matrix3Source READ matrix3Source WRITE setMatrix3Source NOTIFY pcmChanged)
    Q_PROPERTY(int matrix3Dest READ matrix3Dest WRITE setMatrix3Dest NOTIFY pcmChanged)
    Q_PROPERTY(int matrix3Sens READ matrix3Sens WRITE setMatrix3Sens NOTIFY pcmChanged)
    Q_PROPERTY(int matrix4Source READ matrix4Source WRITE setMatrix4Source NOTIFY pcmChanged)
    Q_PROPERTY(int matrix4Dest READ matrix4Dest WRITE setMatrix4Dest NOTIFY pcmChanged)
    Q_PROPERTY(int matrix4Sens READ matrix4Sens WRITE setMatrix4Sens NOTIFY pcmChanged)

    Q_PROPERTY(QStringList waveGroupTypeNames READ waveGroupTypeNames CONSTANT)
    Q_PROPERTY(QStringList waveGroupBankNames READ waveGroupBankNames CONSTANT)
    Q_PROPERTY(QStringList filterTypeNames READ filterTypeNames CONSTANT)
    Q_PROPERTY(QStringList lfoWaveformNames READ lfoWaveformNames CONSTANT)
    Q_PROPERTY(QStringList matrixSourceNames READ matrixSourceNames CONSTANT)
    Q_PROPERTY(QStringList matrixDestNames READ matrixDestNames CONSTANT)
    Q_PROPERTY(QStringList partialWaveLabels READ partialWaveLabels NOTIFY pcmChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit PcmSynthToneModel(InstrumentPlatform *platform, QObject *parent = nullptr);

    void setWaveformCatalog(WaveformCatalog *catalog);

    Q_INVOKABLE QString partialDisplayName(int index) const;
    Q_INVOKABLE QString partialWaveLabel(int index) const;

    QString toneName() const { return m_toneName; }
    int toneLevel() const { return m_toneLevel; }
    int tonePan() const { return m_tonePan; }
    int selectedPartial() const { return m_selectedPartial; }

    int structureType12() const;
    int structureType34() const;
    bool partial1On() const;
    bool partial2On() const;
    bool partial3On() const;
    bool partial4On() const;

    int waveGroupType() const;
    int waveGroupId() const;
    int waveGroupBank() const;
    int waveNumber() const;
    QString waveName() const;
    int coarseTune() const;
    int fineTune() const;
    int filterType() const;
    int filterCutoff() const;
    int filterResonance() const;
    int filterEnvAttack() const;
    int filterEnvDecay() const;
    int filterEnvSustain() const;
    int filterEnvRelease() const;
    int ampLevel() const;
    int ampPan() const;
    int ampEnvAttack() const;
    int ampEnvDecay() const;
    int ampEnvSustain() const;
    int ampEnvRelease() const;
    int lfoWaveform() const;
    int lfoRate() const;
    int lfoPitchDepth() const;
    int lfoFilterDepth() const;
    int lfoAmpDepth() const;

    int matrix1Source() const;
    int matrix1Dest() const;
    int matrix1Sens() const;
    int matrix2Source() const;
    int matrix2Dest() const;
    int matrix2Sens() const;
    int matrix3Source() const;
    int matrix3Dest() const;
    int matrix3Sens() const;
    int matrix4Source() const;
    int matrix4Dest() const;
    int matrix4Sens() const;

    QStringList waveGroupTypeNames() const;
    QStringList waveGroupBankNames() const;
    QStringList filterTypeNames() const;
    QStringList lfoWaveformNames() const;
    QStringList matrixSourceNames() const;
    QStringList matrixDestNames() const;
    QStringList partialWaveLabels() const;
    QString lastError() const { return m_lastError; }

    void setToneName(const QString &v);
    void setToneLevel(int v);
    void setTonePan(int v);
    void setSelectedPartial(int v);

    void setStructureType12(int v);
    void setStructureType34(int v);
    void setPartial1On(bool v);
    void setPartial2On(bool v);
    void setPartial3On(bool v);
    void setPartial4On(bool v);

    void setWaveGroupType(int v);
    void setWaveGroupId(int v);
    void setWaveGroupBank(int v);
    void setWaveNumber(int v);
    void setCoarseTune(int v);
    void setFineTune(int v);
    void setFilterType(int v);
    void setFilterCutoff(int v);
    void setFilterResonance(int v);
    void setFilterEnvAttack(int v);
    void setFilterEnvDecay(int v);
    void setFilterEnvSustain(int v);
    void setFilterEnvRelease(int v);
    void setAmpLevel(int v);
    void setAmpPan(int v);
    void setAmpEnvAttack(int v);
    void setAmpEnvDecay(int v);
    void setAmpEnvSustain(int v);
    void setAmpEnvRelease(int v);
    void setLfoWaveform(int v);
    void setLfoRate(int v);
    void setLfoPitchDepth(int v);
    void setLfoFilterDepth(int v);
    void setLfoAmpDepth(int v);

    void setMatrix1Source(int v);
    void setMatrix1Dest(int v);
    void setMatrix1Sens(int v);
    void setMatrix2Source(int v);
    void setMatrix2Dest(int v);
    void setMatrix2Sens(int v);
    void setMatrix3Source(int v);
    void setMatrix3Dest(int v);
    void setMatrix3Sens(int v);
    void setMatrix4Source(int v);
    void setMatrix4Dest(int v);
    void setMatrix4Sens(int v);

    void setPartIndex(int partIndex) { m_partIndex = partIndex; }
    int partIndex() const { return m_partIndex; }

    bool pullFromDevice();
    bool pushToDevice();
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &obj);

signals:
    void pcmChanged();
    void selectedPartialChanged();
    void lastErrorChanged();

private:
    struct Partial {
        QByteArray raw;
    };

    int partialByte(int offset) const;
    void setPartialByte(int offset, int value, int lo, int hi);
    int partialNibbleValue(int offset, int nibbleCount) const;
    void setPartialNibbleValue(int offset, int nibbleCount, int value, int lo, int hi);
    int pmtByte(quint8 offset) const;
    void setPmtByte(quint8 offset, int value, int lo, int hi);
    int commonByte(quint8 offset) const;
    void setCommonByte(quint8 offset, int value, int lo, int hi);
    int matrixSource(int index) const;
    int matrixDest(int index) const;
    int matrixSens(int index) const;
    void setMatrixSource(int index, int v);
    void setMatrixDest(int index, int v);
    void setMatrixSens(int index, int v);
    void writeCommonBytes(quint8 offset, const QByteArray &data);
    void writePmtBytes(quint8 offset, const QByteArray &data);
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
    QByteArray m_pmt;
    QByteArray m_common2;
    std::array<Partial, 4> m_partials;
    QString m_toneName;
    int m_toneLevel = 100;
    int m_tonePan = 64;
};
