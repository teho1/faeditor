#include <QtTest>
#include <QSignalSpy>

#include "model/MfxModel.h"
#include "model/MfxTapDelayModel.h"
#include "model/MfxUiHelpers.h"

class TestMfxTap : public QObject
{
    Q_OBJECT
private slots:
    void sanitizeUninitializedRawSeedsDemo()
    {
        MfxModel mfx(nullptr);
        mfx.setType(36);
        auto *tap = mfx.tapDelay();
        QVERIFY(tap);
        // All-zero nibble decode is −32768; seed a spread so markers are not piled at 0.
        QCOMPARE(tap->leftTime(), 200);
        QCOMPARE(tap->centerTime(), 400);
        QCOMPARE(tap->rightTime(), 600);
        QCOMPARE(tap->leftLevel(), 100);
        QCOMPARE(tap->centerLevel(), 110);
        QCOMPARE(tap->feedback(), 32);
    }

    void intentionalZeroStaysZero()
    {
        MfxModel mfx(nullptr);
        mfx.setType(36);
        auto *tap = mfx.tapDelay();
        tap->setLeftTime(0);
        tap->setLeftLevel(0);
        QCOMPARE(tap->leftTime(), 0);
        QCOMPARE(tap->leftLevel(), 0);
        // Re-sync must not re-seed once raw encodes a real 0 ms.
        mfx.setChorusSend(mfx.chorusSend()); // emits mfxChanged
        QCOMPARE(tap->leftTime(), 0);
    }

    void writeRoundTripThroughMfxRaw()
    {
        MfxModel mfx(nullptr);
        mfx.setType(36);
        auto *tap = mfx.tapDelay();
        tap->setLeftTime(800);
        tap->setCenterTime(1200);
        tap->setRightTime(400);
        tap->setLeftLevel(100);
        tap->setCenterLevel(90);
        tap->setRightLevel(80);
        tap->setFeedback(40);
        tap->setHfDamp(2000);
        tap->setBalance(60);
        tap->setLevel(110);

        QCOMPARE(tap->leftTime(), 800);
        QCOMPARE(tap->centerTime(), 1200);
        QCOMPARE(tap->rightTime(), 400);
        QCOMPARE(mfx.paramValue(0), 800);
        QCOMPARE(mfx.paramValue(2), 1200);
        QCOMPARE(mfx.paramValue(5), 100);
        QCOMPARE(tap->feedback(), 40);
        QCOMPARE(tap->hfDamp(), 2000);
        QCOMPARE(tap->balance(), 60);
        QCOMPARE(tap->level(), 110);
    }

    void leftTime400MovesProperty()
    {
        MfxModel mfx(nullptr);
        mfx.setType(36);
        auto *tap = mfx.tapDelay();
        QSignalSpy spy(tap, &MfxTapDelayModel::leftTimeChanged);
        tap->setLeftTime(400);
        QCOMPARE(tap->leftTime(), 400);
        QCOMPARE(mfx.paramValue(0), 400);
        QVERIFY(spy.count() >= 1);
        // Axis mapping: mid-ish drag must leave 0.
        QCOMPARE(tap->timeFromAxisNorm(0.25), 650);
        QVERIFY(tap->delayTimeForAxis(400) == 400);
    }

    void hfDampBypassAndFamily()
    {
        MfxModel mfx(nullptr);
        mfx.setType(36);
        auto *tap = mfx.tapDelay();
        tap->setHfDamp(1500);
        QVERIFY(!tap->hfDampBypass());
        tap->setHfDampBypass(true);
        QVERIFY(tap->hfDampBypass());
        QCOMPARE(tap->hfDamp(), 8001);

        QCOMPARE(mfx.uiFamily(), QStringLiteral("multiTapDelay"));
        QVERIFY(mfx.usesVisualTemplate());
        QCOMPARE(MfxUiHelpers::instance()->familyForType(36),
                 QStringLiteral("multiTapDelay"));
        // Without app QRC, catalog may be empty → MfxModel falls back to 8 slots.
        QVERIFY(mfx.paramList()->rowCount() >= 8);
    }
};

QTEST_APPLESS_MAIN(TestMfxTap)
#include "test_mfx_tap.moc"
