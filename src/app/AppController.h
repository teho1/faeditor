#pragma once

#include "midi/MidiDeviceModel.h"
#include "model/StudioSetModel.h"
#include "model/StudioSetBrowserModel.h"
#include "model/ToneBrowserModel.h"
#include "model/WaveformCatalog.h"
#include "model/AudioFxModel.h"
#include "model/TemporaryToneModel.h"
#include "project/ProjectStore.h"
#include "undo/UndoController.h"
#include "midi/SysexEngine.h"

#include <QObject>
#include <QTimer>

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MidiDeviceModel *midi READ midi CONSTANT)
    Q_PROPERTY(StudioSetModel *studioSet READ studioSet CONSTANT)
    Q_PROPERTY(StudioSetBrowserModel *studioSets READ studioSets CONSTANT)
    Q_PROPERTY(ToneBrowserModel *tones READ tones CONSTANT)
    Q_PROPERTY(WaveformCatalog *waveforms READ waveforms CONSTANT)
    Q_PROPERTY(AudioFxModel *audioFx READ audioFx CONSTANT)
    Q_PROPERTY(TemporaryToneModel *tone READ tone CONSTANT)
    Q_PROPERTY(ProjectStore *library READ library CONSTANT)
    Q_PROPERTY(UndoController *undo READ undo CONSTANT)
    Q_PROPERTY(int mainTab READ mainTab WRITE setMainTab NOTIFY mainTabChanged)
    Q_PROPERTY(bool connectDialogOpen READ connectDialogOpen WRITE setConnectDialogOpen NOTIFY connectDialogOpenChanged)
    Q_PROPERTY(QString workflowHint READ workflowHint NOTIFY workflowHintChanged)
    Q_PROPERTY(QString qtLicenseNotice READ qtLicenseNotice CONSTANT)

public:
    explicit AppController(QObject *parent = nullptr);

    MidiDeviceModel *midi() const { return m_midi; }
    StudioSetModel *studioSet() const { return m_studioSet; }
    StudioSetBrowserModel *studioSets() const { return m_studioSets; }
    ToneBrowserModel *tones() const { return m_tones; }
    WaveformCatalog *waveforms() const { return m_waveforms; }
    AudioFxModel *audioFx() const { return m_audioFx; }
    TemporaryToneModel *tone() const { return m_tone; }
    ProjectStore *library() const { return m_library; }
    UndoController *undo() const { return m_undo; }

    int mainTab() const { return m_mainTab; }
    void setMainTab(int v);
    bool connectDialogOpen() const { return m_connectDialogOpen; }
    void setConnectDialogOpen(bool v);
    QString workflowHint() const { return m_workflowHint; }
    /** Qt LGPLv3 liability / attribution notice (same text pattern as righthere-app). */
    QString qtLicenseNotice() const;

    Q_INVOKABLE void applyToneToSelectedPart(int toneRow);
    Q_INVOKABLE void previewTone(int toneRow);
    Q_INVOKABLE bool pull();
    Q_INVOKABLE bool push();
    Q_INVOKABLE bool openStudioSet(int row);
    Q_INVOKABLE void goChangeToneForPart(int partIndex);
    /** Save current Studio Set to the local library and show Sets & Tones. */
    Q_INVOKABLE bool saveToLibrary();
    /** Load a library row into the editor; updates workflow hint on success/failure. */
    Q_INVOKABLE bool loadLibrary(int row);
    /** Open MIDI dialog (refreshes ports; clears stale connection). */
    Q_INVOKABLE void openMidiDialog();
    /** Auto-connect FA and pull Temporary + Audio FX. */
    Q_INVOKABLE void startupConnect();

signals:
    void mainTabChanged();
    void connectDialogOpenChanged();
    void workflowHintChanged();

private:
    void setHint(const QString &h);

    SysexEngine *m_engine = nullptr;
    MidiDeviceModel *m_midi = nullptr;
    UndoController *m_undo = nullptr;
    StudioSetModel *m_studioSet = nullptr;
    StudioSetBrowserModel *m_studioSets = nullptr;
    ToneBrowserModel *m_tones = nullptr;
    WaveformCatalog *m_waveforms = nullptr;
    AudioFxModel *m_audioFx = nullptr;
    TemporaryToneModel *m_tone = nullptr;
    ProjectStore *m_library = nullptr;
    QTimer m_autosaveTimer;
    int m_mainTab = 0;
    bool m_connectDialogOpen = false;
    QString m_workflowHint;
};
