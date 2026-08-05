#pragma once

#include "midi/AddressMap.h"
#include "model/MfxModel.h"
#include "model/MfxPresetStore.h"
#include "model/PcmSynthToneModel.h"
#include "model/SnAcousticToneModel.h"
#include "model/SnSynthToneModel.h"

#include <QJsonObject>
#include <QObject>
#include <QTimer>

class SysexEngine;
class InstrumentPlatform;
class StudioSetModel;

/** Selected part's Temporary Tone — MFX (all engines) + SN-S / PCM / SN-A body. */
class TemporaryToneModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int partIndex READ partIndex NOTIFY partChanged)
    Q_PROPERTY(QString engineName READ engineName NOTIFY partChanged)
    Q_PROPERTY(int engine READ engineInt NOTIFY partChanged)
    Q_PROPERTY(bool isSnSynth READ isSnSynth NOTIFY partChanged)
    Q_PROPERTY(bool isPcmSynth READ isPcmSynth NOTIFY partChanged)
    Q_PROPERTY(bool isSnAcoustic READ isSnAcoustic NOTIFY partChanged)
    Q_PROPERTY(bool supportsMfx READ supportsMfx NOTIFY partChanged)
    Q_PROPERTY(QString toneName READ toneName NOTIFY partChanged)
    Q_PROPERTY(int bankMsb READ bankMsb NOTIFY partChanged)
    Q_PROPERTY(QString selectedStage READ selectedStage WRITE setSelectedStage NOTIFY selectedStageChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(MfxModel *mfx READ mfx CONSTANT)
    Q_PROPERTY(SnSynthToneModel *snSynth READ snSynth CONSTANT)
    Q_PROPERTY(PcmSynthToneModel *pcmSynth READ pcmSynth CONSTANT)
    Q_PROPERTY(SnAcousticToneModel *snAcoustic READ snAcoustic CONSTANT)
    Q_PROPERTY(MfxPresetStore *presets READ presets CONSTANT)

public:
    explicit TemporaryToneModel(SysexEngine *engine, InstrumentPlatform *platform, StudioSetModel *studioSet,
                                QObject *parent = nullptr);

    int partIndex() const { return m_partIndex; }
    QString engineName() const;
    int engineInt() const { return static_cast<int>(m_toneEngine); }
    bool isSnSynth() const { return m_toneEngine == roland::ToneEngine::SnSynth; }
    bool isPcmSynth() const { return m_toneEngine == roland::ToneEngine::PcmSynth; }
    bool isSnAcoustic() const { return m_toneEngine == roland::ToneEngine::SnAcoustic; }
    bool supportsMfx() const { return m_toneEngine != roland::ToneEngine::Unknown; }
    QString toneName() const;
    int bankMsb() const;
    QString selectedStage() const { return m_selectedStage; }
    void setSelectedStage(const QString &stage);
    bool busy() const { return m_busy; }
    QString lastError() const { return m_lastError; }

    MfxModel *mfx() const { return m_mfx; }
    SnSynthToneModel *snSynth() const { return m_sn; }
    PcmSynthToneModel *pcmSynth() const { return m_pcm; }
    SnAcousticToneModel *snAcoustic() const { return m_sna; }
    MfxPresetStore *presets() const { return m_presets; }

    Q_INVOKABLE bool pull();
    Q_INVOKABLE bool push();
    Q_INVOKABLE bool initSnSynth();
    Q_INVOKABLE void openMfxStage();
    Q_INVOKABLE bool applyMfxPreset(int row);

    /** Refresh tone blobs for library JSON (connected: live read). */
    QJsonObject toneBlobsJson(bool refreshFromDevice);
    void setToneBlobsJson(const QJsonObject &obj);
    /** Push stored tone blobs after Studio Set settle. */
    bool pushToneBlobs(const QJsonObject &blobs);

signals:
    void partChanged();
    void selectedStageChanged();
    void busyChanged();
    void lastErrorChanged();
    void toneChanged();

private:
    void syncFromStudioSet();
    void ensureStageForEngine();
    void setBusy(bool v);
    void setError(const QString &e);
    QJsonObject capturePartBlob(int partIndex, bool fromDevice);

    SysexEngine *m_sysex = nullptr;
    InstrumentPlatform *m_platform = nullptr;
    StudioSetModel *m_studioSet = nullptr;
    MfxModel *m_mfx = nullptr;
    SnSynthToneModel *m_sn = nullptr;
    PcmSynthToneModel *m_pcm = nullptr;
    SnAcousticToneModel *m_sna = nullptr;
    MfxPresetStore *m_presets = nullptr;
    QTimer m_partSettle;
    QJsonObject m_cachedBlobs;
    int m_partIndex = 0;
    roland::ToneEngine m_toneEngine = roland::ToneEngine::Unknown;
    QString m_selectedStage = QStringLiteral("osc");
    bool m_busy = false;
    QString m_lastError;
};
