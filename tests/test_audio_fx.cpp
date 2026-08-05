#include "FakeInstrumentPlatform.h"
#include "model/AudioFxModel.h"

#include <QtTest>

class AudioFxTests : public QObject
{
    Q_OBJECT
private slots:
    void pullUsesAudioBlocks();
    void pushWritesBlocks();
    void liveSetterWritesParameter();
    void controllerAssignmentTargetsSystemController();
    void injectedFailureIsReported();
    void unavailableCapabilityPreventsIo();
};

void AudioFxTests::pullUsesAudioBlocks()
{
    FakeInstrumentPlatform fake;
    QByteArray input(0x0B, '\0');
    input[0]=1; input[1]=3; input[2]=70; input[3]=50; input[4]=1; input[5]=41; input[6]=42;
    QByteArray tfx(0x26, '\0');
    tfx[0]=1; tfx[1]=29; tfx[2]=11; tfx[3]=22; tfx[4]=33; tfx[5]=44;
    QByteArray common(0x30, '\0'); common[0x28]=1; common[0x29]=5;
    fake.seedAudio(InstrumentPlatform::AudioBlock::InputEfx,input);
    fake.seedAudio(InstrumentPlatform::AudioBlock::Tfx,tfx);
    fake.seedAudio(InstrumentPlatform::AudioBlock::SystemCommon,common);
    AudioFxModel model(&fake);

    QVERIFY(model.pullFromDevice());
    QCOMPARE(model.inputReverbType(),3); QCOMPARE(model.nsRelease(),42);
    QCOMPARE(model.tfxType(),28); QCOMPARE(model.tfxParamD(),44);
    QCOMPARE(model.tfxLocation(),1); QCOMPARE(model.tfxInputGain(),5);
    QCOMPARE(fake.audioRequests.size(),3);
}

void AudioFxTests::pushWritesBlocks()
{
    FakeInstrumentPlatform fake; AudioFxModel model(&fake);
    model.fromJson({{QStringLiteral("inputReverbType"),4},{QStringLiteral("tfxType"),7},
                    {QStringLiteral("tfxParamA"),88},{QStringLiteral("tfxLocation"),0},
                    {QStringLiteral("tfxInputGain"),3}});
    QVERIFY(model.pushToDevice());
    const auto input=fake.storedAudio(InstrumentPlatform::AudioBlock::InputEfx);
    const auto tfx=fake.storedAudio(InstrumentPlatform::AudioBlock::Tfx);
    const auto common=fake.storedAudio(InstrumentPlatform::AudioBlock::SystemCommon);
    QCOMPARE(input.size(),0x0B); QCOMPARE(quint8(input[1]),quint8(4));
    QCOMPARE(tfx.size(),0x26); QCOMPARE(quint8(tfx[1]),quint8(8)); QCOMPARE(quint8(tfx[2]),quint8(88));
    QCOMPARE(quint8(common[0x28]),quint8(0)); QCOMPARE(quint8(common[0x29]),quint8(3));
}

void AudioFxTests::liveSetterWritesParameter()
{
    FakeInstrumentPlatform fake; AudioFxModel model(&fake);
    model.setTfxParamB(77);
    QCOMPARE(fake.audioRequests.size(),1);
    const auto request=fake.audioRequests.constFirst();
    QVERIFY(request.write); QCOMPARE(request.block,InstrumentPlatform::AudioBlock::Tfx);
    QCOMPARE(request.offset,3); QCOMPARE(quint8(request.data[0]),quint8(77));
}

void AudioFxTests::controllerAssignmentTargetsSystemController()
{
    FakeInstrumentPlatform fake; AudioFxModel model(&fake);
    QVERIFY(model.assignDeviceControlsToTfx());
    QCOMPARE(fake.audioRequests.size(),6); // five controller writes plus the switch default
    const QList<int> offsets{0x22,0x19,0x1A,0x1B,0x14};
    const QList<int> values{0,98,99,100,101};
    for (int i=0;i<5;++i) {
        const auto request=fake.audioRequests.at(i);
        QCOMPARE(request.block,InstrumentPlatform::AudioBlock::SystemController);
        QCOMPARE(request.offset,offsets.at(i)); QCOMPARE(quint8(request.data[0]),quint8(values.at(i)));
    }
}

void AudioFxTests::injectedFailureIsReported()
{
    FakeInstrumentPlatform fake; fake.failure=QStringLiteral("simulated audio timeout");
    AudioFxModel model(&fake);
    QVERIFY(!model.pullFromDevice());
    QCOMPARE(model.lastError(),QStringLiteral("simulated audio timeout"));
    QVERIFY(!model.busy());
}

void AudioFxTests::unavailableCapabilityPreventsIo()
{
    FakeInstrumentPlatform fake; fake.deviceProfile.workspaces.clear();
    AudioFxModel model(&fake);
    QVERIFY(!model.pullFromDevice());
    QVERIFY(fake.audioRequests.isEmpty());
    QVERIFY(model.lastError().contains(QStringLiteral("not supported")));
}

QTEST_GUILESS_MAIN(AudioFxTests)
#include "test_audio_fx.moc"
