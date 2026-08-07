#pragma once

#include "platform/InstrumentPlatform.h"

class SysexEngine;

/** Roland FANTOM-06/07/08 adapter. All writes target documented temporary areas. */
class RolandFantomPlatform final : public InstrumentPlatform
{
public:
    explicit RolandFantomPlatform(SysexEngine *engine=nullptr);

    DeviceProfile profile() const override;
    bool isConnected() const override;
    bool discoverMidiPorts(QVector<MidiPort> *, QVector<MidiPort> *, QString *) override;
    bool openMidiConnection(int, int, QString *) override;
    void closeMidiConnection() override;
    bool midiConnectionHealthy() const override;
    bool detectDevice(quint8 *, int, QString *) override;
    bool sendPreviewNote(int, int, int, bool, QString *) override;
    bool recallPerformance(int, int, int, QString *) override;
    bool readTemporaryPerformanceName(QString *, QString *) override;
    bool readStudioBlock(StudioBlock, int, int, QByteArray *, QString *) override;
    bool writeStudioBlock(StudioBlock, int, const QByteArray &, QString *) override;
    bool writeStudioParameter(StudioBlock, int, int, const QByteArray &, QString *) override;
    bool readMasterEq(QByteArray *, QString *) override;
    bool writeMasterEq(const QByteArray &, QString *) override;
    bool readAudioBlock(AudioBlock, int, QByteArray *, QString *) override;
    bool writeAudioBlock(AudioBlock, const QByteArray &, QString *) override;
    bool writeAudioParameter(AudioBlock, int, const QByteArray &, QString *) override;
    bool readToneSection(int, roland::ToneEngine, ToneSection, int, int, QByteArray *, QString *) override;
    bool writeToneSection(int, roland::ToneEngine, ToneSection, int, const QByteArray &, QString *) override;
    bool writeToneParameter(int, roland::ToneEngine, ToneSection, int, int, const QByteArray &, QString *) override;

    static roland::Address sceneAddress(StudioBlock block, int index = 0);
    static roland::Address zCoreAddress(int zone, ToneSection section, int sectionIndex = 0);
    QString detectedModel() const { return m_detectedModel; }

private:
    static bool nameIsDawControl(const QString &);
    static bool nameLooksLikeFantom0(const QString &);
    roland::Address audioAddress(AudioBlock) const;
    SysexEngine *m_engine = nullptr;
    QString m_detectedModel = QStringLiteral("FANTOM-06/07/08");
};
