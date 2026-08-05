#include "FakeInstrumentPlatform.h"
#include "midi/MidiDeviceModel.h"

#include <QtTest>

class MidiPlatformTests : public QObject
{
    Q_OBJECT
private slots:
    void connectionSucceeds();
    void connectionFailureIsReported();
    void identityReplyIsReported();
    void unhealthyPortDisconnects();
    void disconnectClosesPlatform();
    void previewMessageUsesPlatform();
    void unavailableCapabilityPreventsDiscovery();
};

void MidiPlatformTests::connectionSucceeds()
{
    FakeInstrumentPlatform fake; fake.connected=false; fake.identityDeviceId=0x21;
    MidiDeviceModel model(&fake);
    QVERIFY(model.autoConnectFa());
    QVERIFY(model.connected()); QVERIFY(fake.connected);
    QCOMPARE(model.connectedName(),QStringLiteral("FA-08"));
    QVERIFY(model.statusText().contains(QStringLiteral("21")));
}

void MidiPlatformTests::connectionFailureIsReported()
{
    FakeInstrumentPlatform fake; fake.connected=false; fake.openSucceeds=false;
    MidiDeviceModel model(&fake);
    QVERIFY(!model.autoConnectFa());
    QVERIFY(!model.connected()); QCOMPARE(model.statusText(),QStringLiteral("Open failed"));
}

void MidiPlatformTests::identityReplyIsReported()
{
    FakeInstrumentPlatform fake; fake.identityDeviceId=0x2a;
    MidiDeviceModel model(&fake);
    QVERIFY(model.probeIdentity());
    QCOMPARE(model.statusText(),QStringLiteral("FA Identity OK (device 2a)"));
}

void MidiPlatformTests::unhealthyPortDisconnects()
{
    FakeInstrumentPlatform fake; fake.connected=false;
    MidiDeviceModel model(&fake); QVERIFY(model.autoConnectFa());
    fake.healthy=false;
    QVERIFY(QMetaObject::invokeMethod(&model,"pollConnection",Qt::DirectConnection));
    QVERIFY(!model.connected()); QVERIFY(!fake.connected);
    QVERIFY(model.statusText().contains(QStringLiteral("disconnected")));
}

void MidiPlatformTests::disconnectClosesPlatform()
{
    FakeInstrumentPlatform fake; fake.connected=false;
    MidiDeviceModel model(&fake); QVERIFY(model.autoConnectFa());
    model.disconnectDevice();
    QVERIFY(!model.connected()); QVERIFY(!fake.connected);
    QCOMPARE(model.statusText(),QStringLiteral("Disconnected"));
}

void MidiPlatformTests::previewMessageUsesPlatform()
{
    FakeInstrumentPlatform fake;
    QVERIFY(fake.sendPreviewNote(3,60,100,true,nullptr));
    QVERIFY(fake.sendPreviewNote(3,60,0,false,nullptr));
    QCOMPARE(fake.previews.size(),2);
    QCOMPARE(fake.previews[0].channel,3); QCOMPARE(fake.previews[0].note,60);
    QCOMPARE(fake.previews[0].velocity,100); QVERIFY(fake.previews[0].noteOn);
    QCOMPARE(fake.previews[1].velocity,0); QVERIFY(!fake.previews[1].noteOn);
}

void MidiPlatformTests::unavailableCapabilityPreventsDiscovery()
{
    FakeInstrumentPlatform fake; fake.deviceProfile.workspaces.clear();
    MidiDeviceModel model(&fake);
    QCOMPARE(fake.discoveryCalls,0);
    QVERIFY(!model.autoConnectFa());
    QCOMPARE(fake.discoveryCalls,0);
    QVERIFY(model.statusText().contains(QStringLiteral("not supported")));
}

QTEST_GUILESS_MAIN(MidiPlatformTests)
#include "test_midi_platform.moc"
