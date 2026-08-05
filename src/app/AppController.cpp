#include "app/AppController.h"
#include "model/PartModel.h"

#include <QThread>
#include <QTimer>

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    m_engine = new SysexEngine(this);
    m_platform = new RolandFAPlatform(m_engine);
    m_midi = new MidiDeviceModel(m_engine, this);
    m_undo = new UndoController(this);
    m_studioSet = new StudioSetModel(m_platform, m_undo, this);
    m_studioSets = new StudioSetBrowserModel(m_platform, m_studioSet, this);
    m_tones = new ToneBrowserModel(this);
    m_tones->loadCatalog();
    m_waveforms = new WaveformCatalog(this);
    // Bundled Sound List names — load every start so Tone Edit never depends on Waves.
    m_waveforms->loadCatalog();
    m_audioFx = new AudioFxModel(m_platform, this);
    m_tone = new TemporaryToneModel(m_engine, m_platform, m_studioSet, this);
    if (m_tone) {
        if (auto *sn = m_tone->snSynth())
            sn->setWaveformCatalog(m_waveforms);
        if (auto *pcm = m_tone->pcmSynth())
            pcm->setWaveformCatalog(m_waveforms);
    }
    m_library = new ProjectStore(m_studioSet, m_audioFx, m_tone, this);

    m_studioSet->setToneNameResolver([this](int msb, int lsb, int pc) {
        return m_tones->resolveName(msb, lsb, pc);
    });

    connect(m_studioSet, &StudioSetModel::autosaveRequested, this, [this]() {
        m_autosaveTimer.start(400);
    });
    m_autosaveTimer.setSingleShot(true);
    connect(&m_autosaveTimer, &QTimer::timeout, m_library, &ProjectStore::autosave);

    connect(m_studioSets, &StudioSetBrowserModel::recalled, this, [this](int) {
        setHint(QStringLiteral("Set loaded. Select a part, then click a tone to assign (icon previews)."));
        setMainTab(0); // Stay on Sets & Tones workspace
    });

    connect(m_studioSets, &StudioSetBrowserModel::statusTextChanged, this, [this]() {
        if (!m_studioSets->statusText().isEmpty())
            setHint(m_studioSets->statusText());
    });

    connect(m_midi, &MidiDeviceModel::connectedChanged, this, [this]() {
        if (!m_midi->connected())
            setHint(m_midi->statusText().isEmpty()
                        ? QStringLiteral("FA disconnected — cable unplugged or powered off")
                        : m_midi->statusText());
    });

    m_library->recoverAutosaveIfNeeded();
    setHint(QStringLiteral("Looking for FA…"));

    // After the UI is up, auto-connect and pull Temporary data.
    QTimer::singleShot(500, this, &AppController::startupConnect);
}

void AppController::setHint(const QString &h)
{
    if (m_workflowHint == h)
        return;
    m_workflowHint = h;
    emit workflowHintChanged();
}

void AppController::openMidiDialog()
{
    if (m_midi) {
        m_midi->refresh();
        // Stale "connected" after FA power-cycle / CoreMIDI renumber
        if (m_midi->connected() && m_engine && !m_engine->isOpen())
            m_midi->disconnectDevice();
    }
    // Force a rising edge so the Popup always opens, even if already flagged open.
    if (m_connectDialogOpen) {
        m_connectDialogOpen = false;
        emit connectDialogOpenChanged();
    }
    m_connectDialogOpen = true;
    emit connectDialogOpenChanged();
}

void AppController::startupConnect()
{
    if (!m_midi) {
        setHint(QStringLiteral("1) Connect MIDI  2) Open a Studio Set  3) Change instruments  4) Push"));
        return;
    }

    setHint(QStringLiteral("Looking for FA MIDI ports…"));
    if (!m_midi->autoConnectFa()) {
        setHint(QStringLiteral("No FA found — power on / plug in USB, then click MIDI ○ (or MIDI → Auto-connect FA)."));
        return;
    }

    setHint(QStringLiteral("Connected — pulling Temporary Studio Set…"));
    const bool pulled = pull();
    if (pulled && m_audioFx)
        m_audioFx->pullFromDevice();

    if (pulled) {
        setHint(QStringLiteral("Ready — “%1”. Open a Studio Set or change part tones, then Push.")
                    .arg(m_studioSet->name()));
    } else {
        setHint(QStringLiteral("MIDI connected, but pull failed — click Pull Temp (FA must be ready)."));
    }
}

void AppController::setMainTab(int v)
{
    v = qBound(0, v, 3);
    if (m_mainTab == v)
        return;
    m_mainTab = v;
    emit mainTabChanged();
    // 0 Sets & Tones (+ Library), 1 Mixer, 2 Effects, 3 Tone
    if (v == 2 && m_midi && m_midi->connected())
        m_audioFx->pullFromDevice();
    if (v == 3 && m_midi && m_midi->connected() && m_tone)
        m_tone->pull();
}

void AppController::setConnectDialogOpen(bool v)
{
    if (m_connectDialogOpen == v)
        return;
    m_connectDialogOpen = v;
    emit connectDialogOpenChanged();
}

void AppController::applyToneToSelectedPart(int toneRow)
{
    const auto tone = m_tones->toneAt(toneRow);
    if (tone.isEmpty())
        return;
    auto *p = m_studioSet->selectedPartModel();
    if (!p)
        return;
    p->setBankMsb(tone.value(QStringLiteral("bankMsb")).toInt());
    p->setBankLsb(tone.value(QStringLiteral("bankLsb")).toInt());
    p->setProgram(tone.value(QStringLiteral("program")).toInt());
    p->setToneName(tone.value(QStringLiteral("name")).toString());
    setHint(QStringLiteral("Part %1 → %2 (live on FA). Push to refresh full Temporary if needed; Write on FA to store User set.")
                .arg(p->partNumber())
                .arg(p->toneName()));
}

void AppController::previewTone(int toneRow)
{
    applyToneToSelectedPart(toneRow);
    if (!m_engine || !m_engine->isOpen())
        return;
    auto *p = m_studioSet->selectedPartModel();
    if (!p)
        return;
    const quint8 ch = static_cast<quint8>(p->receiveChannel() & 0x0F);
    QByteArray noteOn;
    noteOn.append(static_cast<char>(0x90 | ch));
    noteOn.append(char(60));
    noteOn.append(char(100));
    m_engine->sendMessage(noteOn);
    QTimer::singleShot(300, this, [this, ch]() {
        QByteArray noteOff;
        noteOff.append(static_cast<char>(0x80 | ch));
        noteOff.append(char(60));
        noteOff.append(char(0));
        m_engine->sendMessage(noteOff);
    });
}

bool AppController::pull()
{
    const bool ok = m_studioSet->pullFromDevice();
    if (ok)
        setHint(QStringLiteral("Pulled Temporary Studio Set “%1”. Select a part and change its tone.")
                    .arg(m_studioSet->name()));
    return ok;
}

bool AppController::push()
{
    // Blobs + typed studio overlays, then tone blobs, then System Audio FX + Master EQ.
    if (!m_studioSet->pushToDevice())
        return false;
    if (m_tone && m_library) {
        // Settle after Studio Set rewrite before Temporary Tone DT1.
        QThread::msleep(250);
        const auto blobs = m_library->toneBlobs();
        if (!blobs.isEmpty() && !m_tone->pushToneBlobs(blobs)) {
            setHint(QStringLiteral("Studio Set pushed, but tone blob push failed."));
            return false;
        }
    }
    if (m_audioFx && !m_audioFx->pushToDevice()) {
        setHint(QStringLiteral("Studio Set pushed, but Audio FX push failed."));
        return false;
    }
    if (!m_studioSet->pushSystemMasterEq()) {
        setHint(QStringLiteral("Studio Set + Audio FX pushed, but Master EQ push failed."));
        return false;
    }
    setHint(QStringLiteral("Pushed Temporary Studio Set, tones, Audio FX, and Master EQ. Permanent store: Write on the FA."));
    return true;
}

bool AppController::openStudioSet(int row)
{
    m_studioSets->setCurrentRow(row);
    return m_studioSets->recallRow(row, true);
}

void AppController::goChangeToneForPart(int partIndex)
{
    m_studioSet->setSelectedPart(partIndex);
    setMainTab(0); // Sets & Tones workspace
    setHint(QStringLiteral("Changing instrument for Part %1 — click a tone to assign, or the icon to preview.")
                .arg(partIndex + 1));
}

bool AppController::saveToLibrary()
{
    if (!m_library || !m_library->save())
        return false;
    setMainTab(0); // Sets & Tones (library panel)
    setHint(QStringLiteral("Saved “%1” to library.").arg(m_library->currentName()));
    return true;
}

bool AppController::loadLibrary(int row)
{
    if (!m_library) {
        setHint(QStringLiteral("Library is not available."));
        return false;
    }
    if (!m_library->load(row)) {
        const auto err = m_library->lastError();
        setHint(err.isEmpty() ? QStringLiteral("Failed to load library file.") : err);
        return false;
    }
    setHint(QStringLiteral("Loaded “%1” from library.").arg(m_library->currentName()));
    return true;
}

QString AppController::qtLicenseNotice() const
{
    // Same attribution pattern as righthere-app (Utils::licenseInfo / QtLegal::aboutQT).
    const QString caption = QStringLiteral(
        "<h3>Qt libraries</h3>"
        "<p>This software uses the Qt libraries, version %1.</p>"
        "<p>We have used a precompiled version of Qt (LGPLv3), which has been downloaded "
        "from the official website <a href=\"https://%2\">%2</a>.</p>")
        .arg(QLatin1String(QT_VERSION_STR), QStringLiteral("qt.io/download"));

    const QString body = QStringLiteral(
        "<p>Qt is a C++ toolkit for cross-platform application development.</p>"
        "<p>Qt provides single-source portability across all major desktop operating systems. "
        "It is also available for embedded Linux and other embedded and mobile operating systems.</p>"
        "<p>Qt is available under multiple licensing options designed to accommodate the needs "
        "of our various users.</p>"
        "<p>Qt licensed under our commercial license agreement is appropriate for development of "
        "proprietary/commercial software where you do not want to share any source code with third "
        "parties or otherwise cannot comply with the terms of GNU (L)GPL.</p>"
        "<p>Qt licensed under GNU (L)GPL is appropriate for the development of Qt applications "
        "provided you can comply with the terms and conditions of the respective licenses.</p>"
        "<p>Please see <a href=\"https://%2/\">%2</a> for an overview of Qt licensing.</p>"
        "<p>Copyright (C) %1 The Qt Company Ltd and other contributors.</p>"
        "<p>Qt and the Qt logo are trademarks of The Qt Company Ltd.</p>"
        "<p>Qt is The Qt Company Ltd product developed as an open source project. "
        "See <a href=\"https://%3/\">%3</a> for more information.</p>")
        .arg(QStringLiteral("2026"),
             QStringLiteral("qt.io/licensing"),
             QStringLiteral("qt.io"));

    return caption + body;
}
