#include "model/TemporaryToneModel.h"
#include "model/PartModel.h"
#include "model/StudioSetModel.h"
#include "midi/SysexEngine.h"

#include <QJsonArray>
#include <QThread>

using namespace roland;

TemporaryToneModel::TemporaryToneModel(SysexEngine *engine, StudioSetModel *studioSet,
                                       QObject *parent)
    : QObject(parent)
    , m_sysex(engine)
    , m_studioSet(studioSet)
{
    m_mfx = new MfxModel(engine, this);
    m_sn = new SnSynthToneModel(engine, this);
    m_pcm = new PcmSynthToneModel(engine, this);
    m_sna = new SnAcousticToneModel(engine, this);
    m_presets = new MfxPresetStore(this);

    connect(m_sna, &SnAcousticToneModel::snaChanged, this, [this]() {
        if (isSnAcoustic())
            ensureStageForEngine();
    });

    m_partSettle.setSingleShot(true);
    m_partSettle.setInterval(220);
    connect(&m_partSettle, &QTimer::timeout, this, [this]() {
        if (m_sysex && m_sysex->isOpen())
            pull();
    });

    if (m_studioSet) {
        connect(m_studioSet, &StudioSetModel::selectedPartChanged, this, [this]() {
            syncFromStudioSet();
            m_partSettle.start();
        });
        connect(m_studioSet, &StudioSetModel::studioSetLoaded, this, [this]() {
            syncFromStudioSet();
            m_partSettle.start();
        });
    }
    syncFromStudioSet();
}

QString TemporaryToneModel::engineName() const
{
    return QString::fromUtf8(toneEngineName(m_toneEngine));
}

QString TemporaryToneModel::toneName() const
{
    if (auto *p = m_studioSet ? m_studioSet->selectedPartModel() : nullptr)
        return p->toneName();
    return {};
}

int TemporaryToneModel::bankMsb() const
{
    if (auto *p = m_studioSet ? m_studioSet->selectedPartModel() : nullptr)
        return p->bankMsb();
    return 0;
}

void TemporaryToneModel::setSelectedStage(const QString &stage)
{
    if (m_selectedStage == stage)
        return;
    m_selectedStage = stage;
    emit selectedStageChanged();
}

void TemporaryToneModel::openMfxStage()
{
    setSelectedStage(QStringLiteral("mfx"));
}

void TemporaryToneModel::setBusy(bool v)
{
    if (m_busy == v)
        return;
    m_busy = v;
    emit busyChanged();
}

void TemporaryToneModel::setError(const QString &e)
{
    if (m_lastError == e)
        return;
    m_lastError = e;
    emit lastErrorChanged();
}

void TemporaryToneModel::ensureStageForEngine()
{
    const QString &s = m_selectedStage;
    if (isSnAcoustic()) {
        const bool tw = m_sna && m_sna->isTwOrgan();
        if (tw) {
            if (s == QLatin1String("modify"))
                setSelectedStage(QStringLiteral("organ"));
            else if (s != QLatin1String("inst") && s != QLatin1String("organ")
                     && s != QLatin1String("mfx"))
                setSelectedStage(QStringLiteral("inst"));
        } else {
            if (s == QLatin1String("organ"))
                setSelectedStage(QStringLiteral("modify"));
            else if (s != QLatin1String("inst") && s != QLatin1String("modify")
                     && s != QLatin1String("mfx"))
                setSelectedStage(QStringLiteral("inst"));
        }
        return;
    }
    if (isSnSynth()) {
        if (s == QLatin1String("matrix") || s == QLatin1String("inst")
            || s == QLatin1String("modify") || s == QLatin1String("organ"))
            setSelectedStage(QStringLiteral("osc"));
        return;
    }
    if (isPcmSynth()) {
        if (s == QLatin1String("inst") || s == QLatin1String("modify")
            || s == QLatin1String("organ"))
            setSelectedStage(QStringLiteral("osc"));
        return;
    }
    // Drums / unknown — MFX only
    if (s != QLatin1String("mfx"))
        setSelectedStage(QStringLiteral("mfx"));
}

void TemporaryToneModel::syncFromStudioSet()
{
    if (!m_studioSet)
        return;
    m_partIndex = m_studioSet->selectedPart();
    int msb = 87;
    if (auto *p = m_studioSet->selectedPartModel())
        msb = p->bankMsb();
    m_toneEngine = toneEngineFromBankMsb(msb);
    m_mfx->setContext(m_partIndex, m_toneEngine);
    m_sn->setPartIndex(m_partIndex);
    m_pcm->setPartIndex(m_partIndex);
    m_sna->setPartIndex(m_partIndex);
    ensureStageForEngine();
    emit partChanged();
}

bool TemporaryToneModel::pull()
{
    syncFromStudioSet();
    if (!m_sysex || !m_sysex->isOpen()) {
        setError(QStringLiteral("Not connected"));
        return false;
    }
    if (m_toneEngine == ToneEngine::Unknown) {
        setError(QStringLiteral("Unknown tone engine for this bank MSB"));
        return false;
    }
    setBusy(true);
    bool ok = m_mfx->pullFromDevice();
    if (ok && isSnSynth())
        ok = m_sn->pullFromDevice();
    if (ok && isPcmSynth())
        ok = m_pcm->pullFromDevice();
    if (ok && isSnAcoustic())
        ok = m_sna->pullFromDevice();
    setBusy(false);
    if (!ok) {
        QString err = m_mfx->lastError();
        if (err.isEmpty() && isSnSynth())
            err = m_sn->lastError();
        if (err.isEmpty() && isPcmSynth())
            err = m_pcm->lastError();
        if (err.isEmpty() && isSnAcoustic())
            err = m_sna->lastError();
        setError(err);
    } else {
        setError({});
    }
    emit toneChanged();
    return ok;
}

bool TemporaryToneModel::push()
{
    syncFromStudioSet();
    if (!m_sysex || !m_sysex->isOpen()) {
        setError(QStringLiteral("Not connected"));
        return false;
    }
    if (m_toneEngine == ToneEngine::Unknown) {
        setError(QStringLiteral("Unknown tone engine for this bank MSB"));
        return false;
    }
    setBusy(true);
    bool ok = m_mfx->pushToDevice();
    if (ok && isSnSynth())
        ok = m_sn->pushToDevice();
    if (ok && isPcmSynth())
        ok = m_pcm->pushToDevice();
    if (ok && isSnAcoustic())
        ok = m_sna->pushToDevice();
    setBusy(false);
    if (!ok) {
        QString err = m_mfx->lastError();
        if (err.isEmpty() && isSnSynth())
            err = m_sn->lastError();
        if (err.isEmpty() && isPcmSynth())
            err = m_pcm->lastError();
        if (err.isEmpty() && isSnAcoustic())
            err = m_sna->lastError();
        setError(err);
    } else {
        setError({});
    }
    return ok;
}

bool TemporaryToneModel::initSnSynth()
{
    syncFromStudioSet();
    m_sn->loadInitTemplate();
    m_mfx->loadBytes(QByteArray(mfxOff::MfxSize, '\0'), false);
    m_mfx->setContext(m_partIndex, ToneEngine::SnSynth);
    // Init always targets SN-S Temporary slot for the part.
    m_toneEngine = ToneEngine::SnSynth;
    if (m_sysex && m_sysex->isOpen()) {
        setBusy(true);
        const bool ok = m_sn->pushToDevice() && m_mfx->pushToDevice();
        setBusy(false);
        if (!ok) {
            setError(m_sn->lastError().isEmpty() ? m_mfx->lastError() : m_sn->lastError());
            return false;
        }
    }
    setSelectedStage(QStringLiteral("osc"));
    setError({});
    emit partChanged();
    emit toneChanged();
    return true;
}

bool TemporaryToneModel::applyMfxPreset(int row)
{
    const auto preset = m_presets->presetAt(row);
    if (preset.isEmpty())
        return false;
    syncFromStudioSet();
    if (m_toneEngine == ToneEngine::Unknown) {
        setError(QStringLiteral("Select a part with a known tone engine first"));
        return false;
    }
    setSelectedStage(QStringLiteral("mfx"));
    return m_mfx->applyPreset(preset);
}

QJsonObject TemporaryToneModel::capturePartBlob(int partIndex, bool fromDevice)
{
    QJsonObject blob;
    if (!m_studioSet || partIndex < 0 || partIndex >= 16)
        return blob;

    auto *part = m_studioSet->partAt(partIndex);
    if (!part)
        return blob;

    const int msb = part->bankMsb();
    const auto engine = toneEngineFromBankMsb(msb);
    blob.insert(QStringLiteral("toneType"), static_cast<int>(engine));
    blob.insert(QStringLiteral("bankMsb"), msb);
    blob.insert(QStringLiteral("bankLsb"), part->bankLsb());
    blob.insert(QStringLiteral("program"), part->program());

    if (engine == ToneEngine::Unknown)
        return blob;

    if (fromDevice && m_sysex && m_sysex->isOpen()) {
        MfxModel tmp(m_sysex);
        tmp.setContext(partIndex, engine);
        if (tmp.pullFromDevice())
            blob.insert(QStringLiteral("mfx"), tmp.toJson());
        blob.insert(QStringLiteral("mfxSwitch"), tmp.mfxSwitch());

        if (engine == ToneEngine::SnSynth) {
            SnSynthToneModel sn(m_sysex);
            sn.setPartIndex(partIndex);
            if (sn.pullFromDevice())
                blob.insert(QStringLiteral("snSynth"), sn.toJson());
        } else if (engine == ToneEngine::PcmSynth) {
            PcmSynthToneModel pcm(m_sysex);
            pcm.setPartIndex(partIndex);
            if (pcm.pullFromDevice())
                blob.insert(QStringLiteral("pcmSynth"), pcm.toJson());
        } else if (engine == ToneEngine::SnAcoustic) {
            SnAcousticToneModel sna(m_sysex);
            sna.setPartIndex(partIndex);
            if (sna.pullFromDevice())
                blob.insert(QStringLiteral("snAcoustic"), sna.toJson());
        }
    } else if (partIndex == m_partIndex) {
        blob.insert(QStringLiteral("mfx"), m_mfx->toJson());
        blob.insert(QStringLiteral("mfxSwitch"), m_mfx->mfxSwitch());
        if (engine == ToneEngine::SnSynth)
            blob.insert(QStringLiteral("snSynth"), m_sn->toJson());
        else if (engine == ToneEngine::PcmSynth)
            blob.insert(QStringLiteral("pcmSynth"), m_pcm->toJson());
        else if (engine == ToneEngine::SnAcoustic)
            blob.insert(QStringLiteral("snAcoustic"), m_sna->toJson());
    } else if (m_cachedBlobs.contains(QStringLiteral("part%1").arg(partIndex + 1, 2, 10, QChar('0')))) {
        return m_cachedBlobs.value(QStringLiteral("part%1").arg(partIndex + 1, 2, 10, QChar('0'))).toObject();
    }
    return blob;
}

QJsonObject TemporaryToneModel::toneBlobsJson(bool refreshFromDevice)
{
    QJsonObject root;
    // At minimum refresh the selected part; when connected + refresh, pull all parts' MFX.
    const int count = (refreshFromDevice && m_sysex && m_sysex->isOpen()) ? 16 : 16;
    for (int i = 0; i < count; ++i) {
        const auto key = QStringLiteral("part%1").arg(i + 1, 2, 10, QChar('0'));
        const bool live = refreshFromDevice && m_sysex && m_sysex->isOpen()
                          && (i == m_partIndex || refreshFromDevice);
        // Pulling 16 full tones is slow — refresh selected always; others keep cache or light MFX.
        QJsonObject blob;
        if (refreshFromDevice && m_sysex && m_sysex->isOpen()) {
            if (i == m_partIndex) {
                pull();
                blob = capturePartBlob(i, false);
            } else {
                // MFX + switch only for non-selected parts (faster library save).
                auto *part = m_studioSet ? m_studioSet->partAt(i) : nullptr;
                if (!part)
                    continue;
                const auto engine = toneEngineFromBankMsb(part->bankMsb());
                if (engine == ToneEngine::Unknown)
                    continue;
                MfxModel tmp(m_sysex);
                tmp.setContext(i, engine);
                if (tmp.pullFromDevice()) {
                    blob.insert(QStringLiteral("toneType"), static_cast<int>(engine));
                    blob.insert(QStringLiteral("bankMsb"), part->bankMsb());
                    blob.insert(QStringLiteral("mfx"), tmp.toJson());
                    blob.insert(QStringLiteral("mfxSwitch"), tmp.mfxSwitch());
                }
            }
        } else {
            blob = capturePartBlob(i, false);
        }
        if (!blob.isEmpty())
            root.insert(key, blob);
        Q_UNUSED(live);
    }
    m_cachedBlobs = root;
    return root;
}

void TemporaryToneModel::setToneBlobsJson(const QJsonObject &obj)
{
    m_cachedBlobs = obj;
    const auto key = QStringLiteral("part%1").arg(m_partIndex + 1, 2, 10, QChar('0'));
    const auto blob = obj.value(key).toObject();
    if (blob.isEmpty())
        return;
    if (blob.contains(QStringLiteral("mfx")))
        m_mfx->fromJson(blob.value(QStringLiteral("mfx")).toObject());
    if (blob.contains(QStringLiteral("snSynth")))
        m_sn->fromJson(blob.value(QStringLiteral("snSynth")).toObject());
    if (blob.contains(QStringLiteral("pcmSynth")))
        m_pcm->fromJson(blob.value(QStringLiteral("pcmSynth")).toObject());
    if (blob.contains(QStringLiteral("snAcoustic")))
        m_sna->fromJson(blob.value(QStringLiteral("snAcoustic")).toObject());
    emit toneChanged();
}

bool TemporaryToneModel::pushToneBlobs(const QJsonObject &blobs)
{
    if (!m_sysex || !m_sysex->isOpen() || blobs.isEmpty())
        return true;

    for (int i = 0; i < 16; ++i) {
        const auto key = QStringLiteral("part%1").arg(i + 1, 2, 10, QChar('0'));
        const auto blob = blobs.value(key).toObject();
        if (blob.isEmpty())
            continue;
        const auto engine = static_cast<ToneEngine>(blob.value(QStringLiteral("toneType")).toInt(-1));
        if (engine == ToneEngine::Unknown)
            continue;

        if (blob.contains(QStringLiteral("mfx"))) {
            MfxModel tmp(m_sysex);
            tmp.setContext(i, engine);
            tmp.fromJson(blob.value(QStringLiteral("mfx")).toObject());
            if (!tmp.pushToDevice()) {
                setError(tmp.lastError());
                return false;
            }
        }
        if (engine == ToneEngine::SnSynth && blob.contains(QStringLiteral("snSynth"))) {
            SnSynthToneModel sn(m_sysex);
            sn.setPartIndex(i);
            sn.fromJson(blob.value(QStringLiteral("snSynth")).toObject());
            if (!sn.pushToDevice()) {
                setError(sn.lastError());
                return false;
            }
        }
        if (engine == ToneEngine::PcmSynth && blob.contains(QStringLiteral("pcmSynth"))) {
            PcmSynthToneModel pcm(m_sysex);
            pcm.setPartIndex(i);
            pcm.fromJson(blob.value(QStringLiteral("pcmSynth")).toObject());
            if (!pcm.pushToDevice()) {
                setError(pcm.lastError());
                return false;
            }
        }
        if (engine == ToneEngine::SnAcoustic && blob.contains(QStringLiteral("snAcoustic"))) {
            SnAcousticToneModel sna(m_sysex);
            sna.setPartIndex(i);
            sna.fromJson(blob.value(QStringLiteral("snAcoustic")).toObject());
            if (!sna.pushToDevice()) {
                setError(sna.lastError());
                return false;
            }
        }
        QThread::msleep(15);
    }
    return true;
}
