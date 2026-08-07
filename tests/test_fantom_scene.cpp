#include <QtTest>
#include "FakeInstrumentPlatform.h"
#include "model/FantomSceneModel.h"
#include "platform/RolandFantomPlatform.h"

class TestFantomScene : public QObject
{
    Q_OBJECT
private slots:
    void documentedAddressMap()
    {
        using B=InstrumentPlatform::StudioBlock;
        QCOMPARE(RolandFantomPlatform::sceneAddress(B::Common),roland::Address({{0x02,0x00,0x00,0x00}}));
        QCOMPARE(RolandFantomPlatform::sceneAddress(B::Part,15),roland::Address({{0x02,0x00,0x1f,0x00}}));
        QCOMPARE(RolandFantomPlatform::sceneAddress(B::PartEq,8),roland::Address({{0x02,0x00,0x28,0x00}}));
        QCOMPARE(RolandFantomPlatform::sceneAddress(B::Zone,3),roland::Address({{0x02,0x00,0x33,0x00}}));
        QCOMPARE(RolandFantomPlatform::sceneAddress(B::Ifx,1),roland::Address({{0x02,0x00,0x06,0x00}}));
        QCOMPARE(RolandFantomPlatform::zCoreAddress(0,InstrumentPlatform::ToneSection::PcmCommon),roland::Address({{0x02,0x10,0x00,0x00}}));
        QCOMPARE(RolandFantomPlatform::zCoreAddress(15,InstrumentPlatform::ToneSection::PcmPartial,3),roland::Address({{0x02,0x1f,0x23,0x00}}));
    }
    void pullDecodesSceneAndZones()
    {
        FakeInstrumentPlatform fake;using B=InstrumentPlatform::StudioBlock;
        QByteArray common(150,'\0');common.replace(0,10,"TEST SCENE");fake.seedStudio(B::Common,0,common);
        for(int i=0;i<16;++i){QByteArray z(73,'\0');z[0]=87;z[1]=64;z[2]=char(i);z[3]=char(i);z[7]=char(100+i);z[8]=64;fake.seedStudio(B::Part,i,z);fake.seedStudio(B::PartEq,i,QByteArray(9,'\0'));fake.seedStudio(B::Zone,i,QByteArray(112,'\0'));}
        FantomSceneModel model(&fake);QVERIFY(model.pull());QCOMPARE(model.name(),QStringLiteral("TEST SCENE"));QCOMPARE(model.rowCount(),16);
        QCOMPARE(model.data(model.index(4),FantomSceneModel::ToneProgramRole).toInt(),5);
        QCOMPARE(model.data(model.index(4),FantomSceneModel::LevelRole).toInt(),104);
    }
    void liveZoneAndToneWritesPreserveRawBlock()
    {
        FakeInstrumentPlatform fake;using B=InstrumentPlatform::StudioBlock;
        fake.seedStudio(B::Common,0,QByteArray(150,'\0'));
        for(int i=0;i<16;++i){fake.seedStudio(B::Part,i,QByteArray(73,char(i)));fake.seedStudio(B::PartEq,i,QByteArray(9,'\0'));fake.seedStudio(B::Zone,i,QByteArray(112,'\0'));}
        FantomSceneModel model(&fake);QVERIFY(model.pull());QVERIFY(model.setZoneValue(2,QStringLiteral("level"),111));
        QCOMPARE(quint8(fake.storedStudio(B::Part,2)[7]),111);QCOMPARE(quint8(fake.storedStudio(B::Part,2)[8]),2);
        QVERIFY(model.assignTone(2,87,65,128));auto z=fake.storedStudio(B::Part,2);QCOMPARE(quint8(z[0]),87);QCOMPARE(quint8(z[1]),65);QCOMPARE(quint8(z[2]),127);
        QVERIFY(model.edited());QVERIFY(model.push());QVERIFY(!model.edited());
    }
    void injectedFailureIsVisible()
    {
        FakeInstrumentPlatform fake;fake.failure=QStringLiteral("simulated FANTOM timeout");FantomSceneModel model(&fake);
        QVERIFY(!model.pull());QCOMPARE(model.status(),QStringLiteral("simulated FANTOM timeout"));
    }
};
QTEST_APPLESS_MAIN(TestFantomScene)
#include "test_fantom_scene.moc"
