#pragma once

#include "midi/AddressMap.h"

#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <QVariantList>

class SysexEngine;

/** SuperNATURAL Acoustic Temporary Tone — Common (Inst + Modify 1–32). */
class SnAcousticToneModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString toneName READ toneName WRITE setToneName NOTIFY snaChanged)
    Q_PROPERTY(int toneLevel READ toneLevel WRITE setToneLevel NOTIFY snaChanged)
    Q_PROPERTY(bool monoPoly READ monoPoly WRITE setMonoPoly NOTIFY snaChanged)
    Q_PROPERTY(int portamentoTimeOffset READ portamentoTimeOffset WRITE setPortamentoTimeOffset NOTIFY snaChanged)
    Q_PROPERTY(int cutoffOffset READ cutoffOffset WRITE setCutoffOffset NOTIFY snaChanged)
    Q_PROPERTY(int resonanceOffset READ resonanceOffset WRITE setResonanceOffset NOTIFY snaChanged)
    Q_PROPERTY(int attackTimeOffset READ attackTimeOffset WRITE setAttackTimeOffset NOTIFY snaChanged)
    Q_PROPERTY(int releaseTimeOffset READ releaseTimeOffset WRITE setReleaseTimeOffset NOTIFY snaChanged)
    Q_PROPERTY(int vibratoRate READ vibratoRate WRITE setVibratoRate NOTIFY snaChanged)
    Q_PROPERTY(int vibratoDepth READ vibratoDepth WRITE setVibratoDepth NOTIFY snaChanged)
    Q_PROPERTY(int vibratoDelay READ vibratoDelay WRITE setVibratoDelay NOTIFY snaChanged)
    Q_PROPERTY(int octaveShift READ octaveShift WRITE setOctaveShift NOTIFY snaChanged)
    Q_PROPERTY(int instVariation READ instVariation WRITE setInstVariation NOTIFY snaChanged)
    Q_PROPERTY(int instNumber READ instNumber WRITE setInstNumber NOTIFY snaChanged)
    Q_PROPERTY(int instNumberDisplay READ instNumberDisplay WRITE setInstNumberDisplay NOTIFY snaChanged)
    Q_PROPERTY(QString instrumentName READ instrumentName NOTIFY snaChanged)
    Q_PROPERTY(QString instrumentFamily READ instrumentFamily NOTIFY snaChanged)
    Q_PROPERTY(QString instrumentFamilyLabel READ instrumentFamilyLabel NOTIFY snaChanged)
    Q_PROPERTY(QStringList instrumentNames READ instrumentNames CONSTANT)
    Q_PROPERTY(QStringList instVariationNames READ instVariationNames NOTIFY snaChanged)
    Q_PROPERTY(bool isTwOrgan READ isTwOrgan NOTIFY snaChanged)
    Q_PROPERTY(QVariantList modifyParams READ modifyParams NOTIFY snaChanged)
    Q_PROPERTY(QStringList modifyParamNames READ modifyParamNames NOTIFY snaChanged)
    Q_PROPERTY(QVariantList modifyParamMaxValues READ modifyParamMaxValues NOTIFY snaChanged)
    Q_PROPERTY(QVariantList modifyParamMinValues READ modifyParamMinValues NOTIFY snaChanged)
    Q_PROPERTY(QVariantList modifyParamDisplayOffsets READ modifyParamDisplayOffsets NOTIFY snaChanged)
    Q_PROPERTY(int modifyParamNamedCount READ modifyParamNamedCount NOTIFY snaChanged)

    // TW Organ drawbars / percussion / click (Modify map when isTwOrgan)
    Q_PROPERTY(int bar16 READ bar16 WRITE setBar16 NOTIFY snaChanged)
    Q_PROPERTY(int bar5_13 READ bar5_13 WRITE setBar5_13 NOTIFY snaChanged)
    Q_PROPERTY(int bar8 READ bar8 WRITE setBar8 NOTIFY snaChanged)
    Q_PROPERTY(int bar4 READ bar4 WRITE setBar4 NOTIFY snaChanged)
    Q_PROPERTY(int bar2_23 READ bar2_23 WRITE setBar2_23 NOTIFY snaChanged)
    Q_PROPERTY(int bar2 READ bar2 WRITE setBar2 NOTIFY snaChanged)
    Q_PROPERTY(int bar1_35 READ bar1_35 WRITE setBar1_35 NOTIFY snaChanged)
    Q_PROPERTY(int bar1_13 READ bar1_13 WRITE setBar1_13 NOTIFY snaChanged)
    Q_PROPERTY(int bar1 READ bar1 WRITE setBar1 NOTIFY snaChanged)
    Q_PROPERTY(int leakageLevel READ leakageLevel WRITE setLeakageLevel NOTIFY snaChanged)
    Q_PROPERTY(bool percussionSwitch READ percussionSwitch WRITE setPercussionSwitch NOTIFY snaChanged)
    Q_PROPERTY(int percussionSoft READ percussionSoft WRITE setPercussionSoft NOTIFY snaChanged)
    Q_PROPERTY(int percussionSoftLevel READ percussionSoftLevel WRITE setPercussionSoftLevel NOTIFY snaChanged)
    Q_PROPERTY(int percussionNormalLevel READ percussionNormalLevel WRITE setPercussionNormalLevel NOTIFY snaChanged)
    Q_PROPERTY(int percussionSlow READ percussionSlow WRITE setPercussionSlow NOTIFY snaChanged)
    Q_PROPERTY(int percussionSlowTime READ percussionSlowTime WRITE setPercussionSlowTime NOTIFY snaChanged)
    Q_PROPERTY(int percussionFastTime READ percussionFastTime WRITE setPercussionFastTime NOTIFY snaChanged)
    Q_PROPERTY(int percussionHarmonic READ percussionHarmonic WRITE setPercussionHarmonic NOTIFY snaChanged)
    Q_PROPERTY(int percussionRechargeTime READ percussionRechargeTime WRITE setPercussionRechargeTime NOTIFY snaChanged)
    Q_PROPERTY(int percussionHarmonicBarLevel READ percussionHarmonicBarLevel WRITE setPercussionHarmonicBarLevel NOTIFY snaChanged)
    Q_PROPERTY(int keyOnClickLevel READ keyOnClickLevel WRITE setKeyOnClickLevel NOTIFY snaChanged)
    Q_PROPERTY(int keyOffClickLevel READ keyOffClickLevel WRITE setKeyOffClickLevel NOTIFY snaChanged)

    Q_PROPERTY(QStringList percussionSoftNames READ percussionSoftNames CONSTANT)
    Q_PROPERTY(QStringList percussionSlowNames READ percussionSlowNames CONSTANT)
    Q_PROPERTY(QStringList percussionHarmonicNames READ percussionHarmonicNames CONSTANT)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit SnAcousticToneModel(SysexEngine *engine, QObject *parent = nullptr);

    QString toneName() const { return m_toneName; }
    int toneLevel() const { return m_toneLevel; }
    bool monoPoly() const { return m_monoPoly; }
    int portamentoTimeOffset() const;
    int cutoffOffset() const;
    int resonanceOffset() const;
    int attackTimeOffset() const;
    int releaseTimeOffset() const;
    int vibratoRate() const;
    int vibratoDepth() const;
    int vibratoDelay() const;
    int octaveShift() const;
    int instVariation() const { return m_instVariation; }
    int instNumber() const { return m_instNumber; }
    int instNumberDisplay() const;
    QString instrumentName() const;
    QString instrumentFamily() const;
    QString instrumentFamilyLabel() const;
    QStringList instrumentNames() const;
    QStringList instVariationNames() const;
    bool isTwOrgan() const;

    QVariantList modifyParams() const;
    QStringList modifyParamNames() const;
    QVariantList modifyParamMaxValues() const;
    QVariantList modifyParamMinValues() const;
    QVariantList modifyParamDisplayOffsets() const;
    int modifyParamNamedCount() const;

    int bar16() const;
    int bar5_13() const;
    int bar8() const;
    int bar4() const;
    int bar2_23() const;
    int bar2() const;
    int bar1_35() const;
    int bar1_13() const;
    int bar1() const;
    int leakageLevel() const;
    bool percussionSwitch() const;
    int percussionSoft() const;
    int percussionSoftLevel() const;
    int percussionNormalLevel() const;
    int percussionSlow() const;
    int percussionSlowTime() const;
    int percussionFastTime() const;
    int percussionHarmonic() const;
    int percussionRechargeTime() const;
    int percussionHarmonicBarLevel() const;
    int keyOnClickLevel() const;
    int keyOffClickLevel() const;

    QStringList percussionSoftNames() const;
    QStringList percussionSlowNames() const;
    QStringList percussionHarmonicNames() const;
    QString lastError() const { return m_lastError; }

    void setToneName(const QString &v);
    void setToneLevel(int v);
    void setMonoPoly(bool v);
    void setPortamentoTimeOffset(int v);
    void setCutoffOffset(int v);
    void setResonanceOffset(int v);
    void setAttackTimeOffset(int v);
    void setReleaseTimeOffset(int v);
    void setVibratoRate(int v);
    void setVibratoDepth(int v);
    void setVibratoDelay(int v);
    void setOctaveShift(int v);
    void setInstVariation(int v);
    void setInstNumber(int v);
    void setInstNumberDisplay(int panelInstNo);

    void setBar16(int v);
    void setBar5_13(int v);
    void setBar8(int v);
    void setBar4(int v);
    void setBar2_23(int v);
    void setBar2(int v);
    void setBar1_35(int v);
    void setBar1_13(int v);
    void setBar1(int v);
    void setLeakageLevel(int v);
    void setPercussionSwitch(bool v);
    void setPercussionSoft(int v);
    void setPercussionSoftLevel(int v);
    void setPercussionNormalLevel(int v);
    void setPercussionSlow(int v);
    void setPercussionSlowTime(int v);
    void setPercussionFastTime(int v);
    void setPercussionHarmonic(int v);
    void setPercussionRechargeTime(int v);
    void setPercussionHarmonicBarLevel(int v);
    void setKeyOnClickLevel(int v);
    void setKeyOffClickLevel(int v);

    Q_INVOKABLE int modifyParam(int index) const;
    Q_INVOKABLE void setModifyParam(int index, int value);
    Q_INVOKABLE QStringList modifyParamEnumNames(int index) const;
    Q_INVOKABLE bool modifyParamHasEnum(int index) const;
    Q_INVOKABLE QString modifyParamDisplayText(int index) const;

    void setPartIndex(int partIndex) { m_partIndex = partIndex; }
    int partIndex() const { return m_partIndex; }

    bool pullFromDevice();
    bool pushToDevice();
    void loadInitTemplate();
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &obj);

signals:
    void snaChanged();
    void lastErrorChanged();

private:
    int commonByte(quint8 offset) const;
    void setCommonByte(quint8 offset, int value, int lo, int hi);
    void writeCommonByte(quint8 offset, int value);
    void syncFromRaw();
    void setError(const QString &e);

    SysexEngine *m_engine = nullptr;
    bool m_fromDevice = false;
    int m_partIndex = 0;
    QString m_lastError;
    QByteArray m_common;
    QString m_toneName;
    int m_toneLevel = 100;
    bool m_monoPoly = true; // POLY
    int m_instVariation = 0;
    int m_instNumber = 0;
};
