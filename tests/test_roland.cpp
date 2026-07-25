#include <QtTest>
#include "midi/RolandChecksum.h"
#include "midi/AddressMap.h"

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
};

QTEST_APPLESS_MAIN(TestRoland)
#include "test_roland.moc"
