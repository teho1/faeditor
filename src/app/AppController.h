#pragma once

#include "midi/MidiDeviceModel.h"
#include "model/StudioSetModel.h"
#include "model/StudioSetBrowserModel.h"
#include "model/ToneBrowserModel.h"
#include "model/WaveformCatalog.h"
#include "model/AudioFxModel.h"
#include "model/TemporaryToneModel.h"
#include "model/SvdImportModel.h"
#include "model/FantomSceneModel.h"
#include "project/ProjectStore.h"
#include "undo/UndoController.h"
#include "midi/SysexEngine.h"
#include "platform/AutoDetectRolandPlatform.h"

#include <QObject>
#include <QTimer>
#include <QUrl>

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
    Q_PROPERTY(SvdImportModel *svdImport READ svdImport CONSTANT)
    Q_PROPERTY(UndoController *undo READ undo CONSTANT)
    Q_PROPERTY(FantomSceneModel *scene READ scene CONSTANT)
    Q_PROPERTY(bool fantomDevice READ fantomDevice NOTIFY deviceFamilyChanged)
    Q_PROPERTY(QString performanceLabel READ performanceLabel NOTIFY deviceFamilyChanged)
    Q_PROPERTY(QString partLabel READ partLabel NOTIFY deviceFamilyChanged)
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
    SvdImportModel *svdImport() const { return m_svdImport; }
    UndoController *undo() const { return m_undo; }
    FantomSceneModel *scene() const{return m_scene;}
    bool fantomDevice() const;
    QString performanceLabel() const{return fantomDevice()?QStringLiteral("Scene"):QStringLiteral("Studio Set");}
    QString partLabel() const{return fantomDevice()?QStringLiteral("Zone"):QStringLiteral("Part");}

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
    /** User-requested disconnect (does not auto-reconnect after lock/sleep). */
    Q_INVOKABLE void disconnectInstrument();
    /** Auto-connect FA and pull Temporary + Audio FX. */
    Q_INVOKABLE void startupConnect();
    Q_INVOKABLE bool exportStudioSetMidi(const QUrl &url);
    Q_INVOKABLE bool exportToneNamesMidnam(const QUrl &url);
    Q_INVOKABLE void previewFantomZone(int zone,int note,int velocity,bool on);

signals:
    void mainTabChanged();
    void connectDialogOpenChanged();
    void workflowHintChanged();
    void deviceFamilyChanged();

private:
    void setHint(const QString &h);
    bool isMobileUi() const;
    void handleApplicationState(Qt::ApplicationState state);
    void startForegroundReconnect();
    void tryForegroundReconnect();
    void finishSessionAfterMidiConnect();

    SysexEngine *m_engine = nullptr;
    AutoDetectRolandPlatform *m_platform = nullptr;
    MidiDeviceModel *m_midi = nullptr;
    UndoController *m_undo = nullptr;
    StudioSetModel *m_studioSet = nullptr;
    StudioSetBrowserModel *m_studioSets = nullptr;
    ToneBrowserModel *m_tones = nullptr;
    WaveformCatalog *m_waveforms = nullptr;
    AudioFxModel *m_audioFx = nullptr;
    TemporaryToneModel *m_tone = nullptr;
    ProjectStore *m_library = nullptr;
    SvdImportModel *m_svdImport = nullptr;
    FantomSceneModel *m_scene = nullptr;
    QTimer m_autosaveTimer;
    QTimer m_foregroundReconnectTimer;
    int m_foregroundReconnectAttempt = 0;
    int m_mainTab = 0;
    bool m_connectDialogOpen = false;
    bool m_keepInstrumentConnection = false;
    bool m_reconnectingForeground = false;
    QString m_workflowHint;
};
