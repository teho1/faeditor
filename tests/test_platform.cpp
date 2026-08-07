#include <QtTest>
#include "FakeInstrumentPlatform.h"
#include "model/TemporarySysexStore.h"
#include "platform/RolandFantomPlatform.h"
#include "platform/YamahaPlatform.h"

class TestPlatform : public QObject
{
    Q_OBJECT
private slots:
    void pullPushAndPartIsolation()
    {
        FakeInstrumentPlatform fake;
        using S = InstrumentPlatform::ToneSection;
        fake.seed(0, roland::ToneEngine::SnSynth, S::SnCommon, 0, QByteArrayLiteral("part-one"));
        fake.seed(1, roland::ToneEngine::SnSynth, S::SnCommon, 0, QByteArrayLiteral("part-two"));
        QByteArray data; QString error;
        QVERIFY(fake.readToneSection(1, roland::ToneEngine::SnSynth, S::SnCommon, 0, 8, &data, &error));
        QCOMPARE(data, QByteArrayLiteral("part-two"));
        QVERIFY(fake.writeToneSection(1, roland::ToneEngine::SnSynth, S::SnCommon, 0, QByteArrayLiteral("changed"), &error));
        QCOMPARE(fake.stored(0, roland::ToneEngine::SnSynth, S::SnCommon, 0), QByteArrayLiteral("part-one"));
        QCOMPARE(fake.stored(1, roland::ToneEngine::SnSynth, S::SnCommon, 0), QByteArrayLiteral("changed"));
    }
    void snSynthModeIsRecordedSemantically()
    {
        FakeInstrumentPlatform fake; QString error;
        using S = InstrumentPlatform::ToneSection;
        QVERIFY(fake.writeToneSection(7, roland::ToneEngine::SnSynth, S::MfxSwitch, 0, QByteArray(1, 1), &error));
        QCOMPARE(fake.requests.constLast().part, 7);
        QCOMPARE(fake.requests.constLast().engine, roland::ToneEngine::SnSynth);
        QCOMPARE(fake.requests.constLast().section, S::MfxSwitch);
    }
    void deterministicFailure()
    {
        FakeInstrumentPlatform fake; fake.failure = QStringLiteral("simulated timeout");
        QByteArray data; QString error;
        QVERIFY(!fake.readToneSection(0, roland::ToneEngine::SnSynth, InstrumentPlatform::ToneSection::Mfx, 0, 4, &data, &error));
        QCOMPARE(error, QStringLiteral("simulated timeout"));
        QCOMPARE(fake.requests.size(), 1);
    }
    void studioStoreRoundTripAndPartIsolation()
    {
        FakeInstrumentPlatform fake; using B=InstrumentPlatform::StudioBlock;
        fake.seedStudio(B::Part,3,QByteArray(roland::partOff::PartSize,char(3)));
        fake.seedStudio(B::Part,4,QByteArray(roland::partOff::PartSize,char(4)));
        TemporarySysexStore store; QString error;
        QVERIFY(store.pullFromDevice(&fake,&error));
        QCOMPARE(store.blob(TemporarySysexStore::partKey(3)).at(0),char(3));
        QCOMPARE(store.blob(TemporarySysexStore::partKey(4)).at(0),char(4));
        store.setBlob(TemporarySysexStore::partKey(3),QByteArray(roland::partOff::PartSize,char(9)));
        QVERIFY(store.pushToDevice(&fake,&error));
        QCOMPARE(fake.storedStudio(B::Part,3).at(0),char(9));
        QCOMPARE(fake.storedStudio(B::Part,4).at(0),char(4));
    }
    void studioStorePropagatesFailure()
    {
        FakeInstrumentPlatform fake; fake.failure=QStringLiteral("studio failure");
        TemporarySysexStore store; QString error;
        QVERIFY(!store.pullFromDevice(&fake,&error)); QCOMPARE(error,QStringLiteral("studio failure"));
    }
    void capabilityNegotiation()
    {
        FakeInstrumentPlatform fake;
        fake.deviceProfile.workspaces={{InstrumentPlatform::Workspace::Library,InstrumentPlatform::FeatureAccess::ReadOnly}};
        QVERIFY(fake.supportsWorkspace(InstrumentPlatform::Workspace::Library));
        QVERIFY(!fake.supportsWorkspace(InstrumentPlatform::Workspace::Library,true));
        QCOMPARE(fake.workspaceAccess(InstrumentPlatform::Workspace::AudioFx),InstrumentPlatform::FeatureAccess::Unavailable);
    }
    void capabilityAdaptersReflectImplementedSupport()
    {
        RolandFantomPlatform fantom; YamahaPlatform yamaha; QString error;
        QCOMPARE(fantom.profile().manufacturer,QStringLiteral("Roland"));
        QCOMPARE(yamaha.profile().manufacturer,QStringLiteral("Yamaha"));
        QVERIFY(fantom.supportsWorkspace(InstrumentPlatform::Workspace::ToneEditing,true));
        QVERIFY(yamaha.supportsWorkspace(InstrumentPlatform::Workspace::Library));
        QVERIFY(!yamaha.supportsWorkspace(InstrumentPlatform::Workspace::Library,true));
        QVERIFY(!fantom.openMidiConnection(0,0,&error));
    }
};
QTEST_APPLESS_MAIN(TestPlatform)
#include "test_platform.moc"
