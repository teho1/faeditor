#pragma once

#include "midi/AddressMap.h"

#include <QByteArray>
#include <QString>
#include <QVector>

class InstrumentPlatform
{
public:
    enum class FeatureAccess { Unavailable, ReadOnly, ReadWrite };
    enum class Workspace { MidiConnection, DeviceIdentity, StudioSets, ToneEditing, AudioFx, NotePreview, Library };
    enum class ToneEngineCapability { SuperNaturalSynth, PcmSynth, SuperNaturalAcoustic, PcmDrum, SuperNaturalDrum,
                                      ZCore, VirtualToneWheel, ExpansionSuperNatural, ModelTone };
    struct WorkspaceCapability { Workspace workspace; FeatureAccess access = FeatureAccess::Unavailable; };
    struct DeviceProfile {
        QString manufacturer;
        QString model;
        QVector<ToneEngineCapability> toneEngines;
        int partCount = 0;
        QVector<WorkspaceCapability> workspaces;
    };
    struct MidiPort { int index = -1; QString name; bool isDawControl = false; bool looksLikeFa = false; };
    enum class ToneSection { Mfx, MfxSwitch, SnCommon, SnMisc, SnPartial,
                             PcmCommon, PcmPmt, PcmCommon2, PcmPartial,
                             SnAcousticCommon };
    enum class StudioBlock { Common, Chorus, Reverb, Ifx, MasterComp, Controller,
                             PadCommon, Midi, Part, PartEq, Zone, Pad };
    enum class AudioBlock { SystemCommon, InputEfx, Tfx, SystemController };

    virtual ~InstrumentPlatform() = default;
    virtual DeviceProfile profile() const = 0;
    FeatureAccess workspaceAccess(Workspace workspace) const
    {
        for (const auto &capability : profile().workspaces)
            if (capability.workspace == workspace) return capability.access;
        return FeatureAccess::Unavailable;
    }
    bool supportsWorkspace(Workspace workspace, bool write = false) const
    {
        const auto access = workspaceAccess(workspace);
        return access == FeatureAccess::ReadWrite || (!write && access == FeatureAccess::ReadOnly);
    }
    virtual bool isConnected() const = 0;
    virtual bool discoverMidiPorts(QVector<MidiPort> *inputs, QVector<MidiPort> *outputs,
                                   QString *error = nullptr) = 0;
    virtual bool openMidiConnection(int inputIndex, int outputIndex,
                                    QString *error = nullptr) = 0;
    virtual void closeMidiConnection() = 0;
    virtual bool midiConnectionHealthy() const = 0;
    virtual bool detectDevice(quint8 *deviceId, int timeoutMs = 1500,
                              QString *error = nullptr) = 0;
    bool detectRolandFA(quint8 *deviceId, int timeoutMs = 1500, QString *error = nullptr)
    { return detectDevice(deviceId, timeoutMs, error); }
    virtual bool sendPreviewNote(int channel, int note, int velocity, bool noteOn,
                                 QString *error = nullptr) = 0;
    virtual bool recallPerformance(int bankMsb, int bankLsb, int program,
                                   QString *error = nullptr) = 0;
    virtual bool readTemporaryPerformanceName(QString *name, QString *error = nullptr) = 0;
    bool recallStudioSet(int bankMsb, int bankLsb, int program, QString *error = nullptr)
    { return recallPerformance(bankMsb, bankLsb, program, error); }
    bool readTemporaryStudioSetName(QString *name, QString *error = nullptr)
    { return readTemporaryPerformanceName(name, error); }
    virtual bool readStudioBlock(StudioBlock block, int index, int size, QByteArray *data,
                                 QString *error = nullptr) = 0;
    virtual bool writeStudioBlock(StudioBlock block, int index, const QByteArray &data,
                                  QString *error = nullptr) = 0;
    virtual bool writeStudioParameter(StudioBlock block, int index, int offset,
                                      const QByteArray &data, QString *error = nullptr) = 0;
    virtual bool readMasterEq(QByteArray *data, QString *error = nullptr) = 0;
    virtual bool writeMasterEq(const QByteArray &data, QString *error = nullptr) = 0;
    virtual bool readAudioBlock(AudioBlock block, int size, QByteArray *data,
                                QString *error = nullptr) = 0;
    virtual bool writeAudioBlock(AudioBlock block, const QByteArray &data,
                                 QString *error = nullptr) = 0;
    virtual bool writeAudioParameter(AudioBlock block, int offset, const QByteArray &data,
                                     QString *error = nullptr) = 0;
    virtual bool readToneSection(int part, roland::ToneEngine engine, ToneSection section,
                                 int sectionIndex, int size, QByteArray *data,
                                 QString *error = nullptr) = 0;
    virtual bool writeToneSection(int part, roland::ToneEngine engine, ToneSection section,
                                  int sectionIndex, const QByteArray &data,
                                  QString *error = nullptr) = 0;
    virtual bool writeToneParameter(int part, roland::ToneEngine engine, ToneSection section,
                                    int sectionIndex, int offset, const QByteArray &data,
                                    QString *error = nullptr) = 0;
};
