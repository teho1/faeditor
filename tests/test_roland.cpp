#include <QtTest>
#include "midi/RolandChecksum.h"
#include "midi/AddressMap.h"
#include <QFile>

class TestRoland : public QObject
{
    Q_OBJECT
private slots:
    void reverbTypeExampleFromManual()
    {
        const quint8 bytes[] = {0x18, 0x00, 0x02, 0x01, 0x02};
        QCOMPARE(roland::computeChecksum(bytes, 5), quint8(0x63));
    }

    void temporaryStudioSetBase()
    {
        QCOMPARE(roland::addr::kTemporaryStudioSet[0], quint8(0x18));
    }

    void partAddresses()
    {
        QCOMPARE(roland::addr::part(0)[2], quint8(0x20));
        QCOMPARE(roland::addr::part(15)[2], quint8(0x2F));
    }

    void zoneAddresses()
    {
        QCOMPARE(roland::addr::zone(0)[2], quint8(0x40));
        QCOMPARE(roland::addr::zone(15)[2], quint8(0x4F));
    }

    void sizeRoundTrip()
    {
        QCOMPARE(roland::sizeToInt(roland::sizeFromInt(0x4C)), 0x4C);
    }

    void partParamAddress()
    {
        const auto a = roland::addr::partParam(0, 0x09);
        QCOMPARE(a[3], quint8(0x09));
    }

    void mixerControlsRetainPointerGrab()
    {
        for (const auto &name : {QStringLiteral("Fader.qml"),QStringLiteral("PanKnob.qml")}) {
            QFile file(QStringLiteral(FAEDITOR_SOURCE_DIR "/qml/components/") + name);
            QVERIFY2(file.open(QIODevice::ReadOnly),qPrintable(file.errorString()));
            QVERIFY2(file.readAll().contains("preventStealing: true"),qPrintable(name));
        }
    }

    void channelStripHasNoFakeLevelMeter()
    {
        QFile file(QStringLiteral(FAEDITOR_SOURCE_DIR "/qml/components/ChannelStrip.qml"));
        QVERIFY(file.open(QIODevice::ReadOnly));
        const auto source=file.readAll();
        QVERIFY(!source.contains("Meter placeholder"));
        QVERIFY(!source.contains("opacity: 0.5"));
    }

    void knobsUseLogicStyleVerticalDrag()
    {
        QFile file(QStringLiteral(FAEDITOR_SOURCE_DIR "/qml/components/PanKnob.qml"));
        QVERIFY(file.open(QIODevice::ReadOnly));
        const auto source=file.readAll();
        QVERIFY(source.contains("lastY - mouse.y"));
        QVERIFY(source.contains("Qt.ShiftModifier"));
        QVERIFY(source.contains("fineScale"));
        QVERIFY(source.contains("onDoubleClicked"));
        QVERIFY(source.contains("defaultValue: 64"));
        QVERIFY(source.contains("preventStealing: true"));
        QVERIFY(!source.contains("mouse.x - startX"));
    }

    void releaseVersionIsConsistent()
    {
        const auto read=[](const QString &path){QFile f(path);if(!f.open(QIODevice::ReadOnly))return QByteArray();return f.readAll();};
        const auto root=QStringLiteral(FAEDITOR_SOURCE_DIR "/");
        QVERIFY(read(root+QStringLiteral("CMakeLists.txt")).contains("project(FAEditor VERSION 1.1.0"));
        QVERIFY(read(root+QStringLiteral("CMakeLists.txt")).contains("MACOSX_BUNDLE_BUNDLE_VERSION 4"));
        const auto products=read(root+QStringLiteral("cmake/Products.cmake"));
        QVERIFY(products.contains("FAEditor;FA Editor;com.righthere.faeditor;1.1;4;FAEditor;Main"));
        QVERIFY(read(root+QStringLiteral("qml/views/AboutDialog.qml")).contains("appVersion: \"1.1\""));
        QVERIFY(read(root+QStringLiteral("qml/Main.qml")).contains("appVersion: Qt.application.version"));
    }
};

QTEST_APPLESS_MAIN(TestRoland)
#include "test_roland.moc"
