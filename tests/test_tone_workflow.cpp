#include <QtTest>

#include "midi/SysexEngine.h"
#include "model/StudioSetModel.h"
#include "model/TemporaryToneModel.h"
#include "undo/UndoController.h"
#include "platform/RolandFAPlatform.h"

class FakeSysexEngine final : public SysexEngine
{
public:
    struct Write { roland::Address address; QByteArray data; };
    bool open = true;
    QString failure;
    QVector<Write> writes;
    QHash<quint32, QByteArray> memory;
    bool isOpen() const override { return open; }
    bool read(const roland::Address &a, int size, QByteArray *out, QString *error, int) override
    {
        if (!failure.isEmpty()) { if (error) *error = failure; return false; }
        if (out) *out = memory.value(roland::addressToU32(a), QByteArray(size, '\0')).leftJustified(size, '\0');
        return true;
    }
    bool write(const roland::Address &a, const QByteArray &data, QString *error) override
    {
        if (!failure.isEmpty()) { if (error) *error = failure; return false; }
        writes.push_back({a, data}); memory.insert(roland::addressToU32(a), data); return true;
    }
    bool writeParam(const roland::Address &a, const QByteArray &data, QString *error) override
    { return write(a, data, error); }
};

class TestToneWorkflow : public QObject
{
    Q_OBJECT
private slots:
    void initSnSynthSetsPartModeAndPushesSelectedPart()
    {
        FakeSysexEngine fake; UndoController undo;
        RolandFAPlatform platform(&fake);
        StudioSetModel studio(&platform, &undo);
        studio.setSelectedPart(5);
        studio.selectedPartModel()->setBankMsb(87);
        fake.writes.clear();
        TemporaryToneModel tone(&fake, &platform, &studio);
        QVERIFY(tone.initSnSynth());
        QCOMPARE(studio.selectedPartModel()->bankMsb(), 95);
        QVERIFY(tone.isSnSynth());
        QVERIFY(!fake.writes.isEmpty());
        bool wroteSelectedTone = false;
        const auto base = roland::addressToU32(roland::addr::temporaryTone(5));
        const auto next = roland::addressToU32(roland::addr::temporaryTone(6));
        for (const auto &w : fake.writes) {
            const auto address = roland::addressToU32(w.address);
            wroteSelectedTone = wroteSelectedTone || (address >= base && address < next);
        }
        QVERIFY(wroteSelectedTone);
    }
    void pullPushAndErrorsAreHeadless()
    {
        FakeSysexEngine fake; UndoController undo;
        RolandFAPlatform platform(&fake);
        StudioSetModel studio(&platform, &undo);
        studio.setSelectedPart(2);
        studio.selectedPartModel()->setBankMsb(95);
        TemporaryToneModel tone(&fake, &platform, &studio);
        QVERIFY(tone.pull());
        QVERIFY(tone.push());
        QVERIFY(!fake.writes.isEmpty());
        fake.failure = QStringLiteral("simulated timeout");
        QVERIFY(!tone.pull());
        QCOMPARE(tone.lastError(), QStringLiteral("simulated timeout"));
    }
};

QTEST_APPLESS_MAIN(TestToneWorkflow)
#include "test_tone_workflow.moc"
