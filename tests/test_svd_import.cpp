#include <QtTest>

#include "FakeInstrumentPlatform.h"
#include "model/SvdImportModel.h"
#include "model/StudioSetModel.h"
#include "undo/UndoController.h"

#include <QTemporaryFile>

namespace {

class BitWriter
{
public:
    void write(int value, int width)
    {
        for (int bit = width - 1; bit >= 0; --bit) {
            if (m_bit % 8 == 0) m_data.append('\0');
            if ((value >> bit) & 1)
                m_data[m_bit / 8] = char(quint8(m_data[m_bit / 8]) | (1 << (7 - m_bit % 8)));
            ++m_bit;
        }
    }
    QByteArray data() const { return m_data; }
private:
    QByteArray m_data;
    int m_bit = 0;
};

QByteArray sampleEntry()
{
    BitWriter w;
    const QByteArray name = QByteArrayLiteral("Test Grand  ");
    for (char c : name) w.write(quint8(c), 7);
    for (int i = name.size(); i < 12; ++i) w.write(' ', 7);
    for (int i = 0; i < 4; ++i) w.write(' ', 7);
    w.write(100, 7); w.write(1, 1);
    for (int i = 0; i < 8; ++i) w.write(64, 7);
    w.write(4, 3); w.write(3, 7); w.write(42, 8); w.write(4, 3); w.write(1, 1);
    w.write(5, 7); w.write(23, 7);
    for (int i = 0; i < 32; ++i) w.write(i, 7);
    w.write(0, 2); w.write(0, 5);
    for (int i = 0; i < 3; ++i) w.write(0, 7);
    w.write(0, 16);
    w.write(21, 7); w.write(127, 7); w.write(100, 7); w.write(90, 7); w.write(0, 2);
    const int sources[4] = {18, 19, 0, 80};
    const int senses[4] = {96, 32, 64, 127};
    for (int i = 0; i < 4; ++i) { w.write(sources[i], 7); w.write(senses[i], 7); }
    for (int i = 0; i < 4; ++i) w.write(i + 1, 5);
    for (int i = 0; i < 32; ++i) w.write(32768 + i, 16);
    w.write(0, 6);
    const auto result = w.data();
    Q_ASSERT(result.size() == 138);
    return result;
}

void writeMfx(BitWriter &w)
{
    w.write(21, 7); w.write(127, 7); w.write(100, 7); w.write(90, 7); w.write(0, 2);
    const int sources[4] = {18, 19, 0, 80};
    const int senses[4] = {96, 32, 64, 127};
    for (int i = 0; i < 4; ++i) { w.write(sources[i], 7); w.write(senses[i], 7); }
    for (int i = 0; i < 4; ++i) w.write(i + 1, 5);
    for (int i = 0; i < 32; ++i) w.write(32768 + i, 16);
    w.write(0, 6);
}

QByteArray sampleSnSynthEntry()
{
    BitWriter w;
    const QByteArray name = QByteArrayLiteral("Test SNSynth");
    for (char c : name) w.write(quint8(c), 7);
    w.write(110, 7);
    for (int i = 0; i < 3; ++i) w.write(0, 4);
    w.write(0, 1); w.write(0, 1);
    w.write(1, 1); w.write(25, 7); w.write(1, 2); w.write(4, 3);
    w.write(12, 5); w.write(12, 5); w.write(0, 3);
    for (int i = 0; i < 6; ++i) w.write(i % 2, 1);
    w.write(2, 2); w.write(1, 1);
    for (int i = 0; i < 6; ++i) w.write(0, 2);
    for (int i = 0; i < 7; ++i) w.write(0, 1);
    w.write(1, 1); w.write(0, 1); w.write(0, 1); w.write(1, 1); w.write(1, 1); w.write(0, 1);
    w.write(12, 7); w.write(34, 7); w.write(22, 7); w.write(321, 16);
    w.write(4, 3); w.write(2, 2);
    for (int i = 0; i < 3; ++i) w.write(64, 7);
    w.write(0, 12);
    writeMfx(w);

    for (int partial = 0; partial < 3; ++partial) {
        w.write(partial, 3); w.write(partial, 6); w.write(0, 2); w.write(32, 6);
        w.write(64, 7); w.write(1, 7); w.write(64, 7); w.write(10, 7); w.write(20, 7); w.write(64, 7);
        w.write(1, 3); w.write(0, 1); w.write(100, 7); w.write(32, 6);
        for (int i = 0x0e; i <= 0x1b; ++i) w.write((i + partial) & 0x7f, 7);
        w.write(1, 3); w.write(60, 7); w.write(0, 1); w.write(5, 5); w.write(40, 7); w.write(1, 1);
        for (int i = 0; i < 4; ++i) w.write(64 + i, 7);
        w.write(2, 3); w.write(61, 7); w.write(0, 1); w.write(6, 5); w.write(65, 7); w.write(0, 1);
        for (int i = 0; i < 8; ++i) w.write(50 + i, 7);
        w.write(1, 2); w.write(123 + partial, 16);
        w.write(70, 7); w.write(71, 7); w.write(72, 7); w.write(16, 5);
        w.write(0, 18);
    }
    for (int i = 0; i < 37; ++i) w.write(i, 7);
    w.write(0, 13);
    const auto result = w.data();
    Q_ASSERT(result.size() == 280);
    return result;
}

void putBe32(QByteArray &data, int pos, quint32 value)
{
    data[pos] = char(value >> 24); data[pos + 1] = char(value >> 16);
    data[pos + 2] = char(value >> 8); data[pos + 3] = char(value);
}

QByteArray sampleSvdFor(const QByteArray &areaName, const QByteArray &entry)
{
    QByteArray result(32 + 16 + entry.size(), '\0');
    result[1] = char(30); // first area is 2 + 30 = 32
    result.replace(2, 4, QByteArrayLiteral("SVD1"));
    result.replace(16, 8, areaName);
    putBe32(result, 24, 32);
    putBe32(result, 28, quint32(16 + entry.size()));
    putBe32(result, 32, 1);
    putBe32(result, 36, quint32(entry.size()));
    putBe32(result, 40, 16);
    result.replace(48, entry.size(), entry);
    return result;
}

QByteArray sampleSvd()
{
    return sampleSvdFor(QByteArrayLiteral("SNTaMI73"), sampleEntry());
}

} // namespace

class TestSvdImport : public QObject
{
    Q_OBJECT
private slots:
    void decodesPackedSnAcousticToDeviceBytes()
    {
        QByteArray common, mfx;
        QString error;
        QVERIFY2(SvdImportModel::decodeSnAcoustic(sampleEntry(), &common, &mfx, &error), qPrintable(error));
        QCOMPARE(common.size(), 71);
        QCOMPARE(common.left(12), QByteArrayLiteral("Test Grand  "));
        QCOMPARE(quint8(common[0x10]), quint8(100));
        QCOMPARE(quint8(common[0x1a]), quint8(64));
        QCOMPARE(quint8(common[0x1b]), quint8(3));
        QCOMPARE(quint8(common[0x1c]), quint8(2));
        QCOMPARE(quint8(common[0x1d]), quint8(10));
        QCOMPARE(quint8(common[0x20]), quint8(5));
        QCOMPARE(quint8(common[0x21]), quint8(23));
        QCOMPARE(mfx.size(), 145);
        QCOMPARE(quint8(mfx[0]), quint8(21));
        QCOMPARE(quint8(mfx[5]), quint8(18));
        QCOMPARE(quint8(mfx[13]), quint8(1));
        QCOMPARE(mfx.mid(17, 4), QByteArray::fromHex("08000000"));
    }

    void loadsAndPushesSelectedToneThroughPlatform()
    {
        QTemporaryFile file;
        QVERIFY(file.open());
        QCOMPARE(file.write(sampleSvd()), qint64(sampleSvd().size()));
        file.flush();

        FakeInstrumentPlatform platform;
        UndoController undo;
        StudioSetModel studio(&platform, &undo);
        studio.setSelectedPart(4);
        SvdImportModel model(&platform, &studio);
        QVERIFY2(model.loadFile(QUrl::fromLocalFile(file.fileName())), qPrintable(model.lastError()));
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.data(model.index(0), SvdImportModel::NameRole).toString(), QStringLiteral("Test Grand"));
        QVERIFY2(model.pushTone(0), qPrintable(model.lastError()));
        QCOMPARE(studio.selectedPartModel()->bankMsb(), 89);
        QCOMPARE(studio.selectedPartModel()->bankLsb(), 64);
        QCOMPARE(studio.selectedPartModel()->program(), 0);
        QCOMPARE(platform.stored(4, roland::ToneEngine::SnAcoustic,
                                 InstrumentPlatform::ToneSection::SnAcousticCommon, 0).size(), 71);
        QCOMPARE(platform.stored(4, roland::ToneEngine::SnAcoustic,
                                 InstrumentPlatform::ToneSection::Mfx, 0).size(), 145);
    }

    void decodesPackedSnSynthToDeviceBlocks()
    {
        QByteArray common, mfx, misc;
        std::array<QByteArray, 3> partials;
        QString error;
        QVERIFY2(SvdImportModel::decodeSnSynth(sampleSnSynthEntry(), &common, &mfx,
                                               &partials, &misc, &error), qPrintable(error));
        QCOMPARE(common.size(), 64);
        QCOMPARE(common.left(12), QByteArrayLiteral("Test SNSynth"));
        QCOMPARE(quint8(common[0x0c]), quint8(110));
        QCOMPARE(quint8(common[0x15]), quint8(64));
        QCOMPARE(quint8(common[0x20]), quint8(1));
        QCOMPARE(common.mid(0x37, 4), QByteArray::fromHex("00010401"));
        QCOMPARE(mfx.size(), 145);
        QCOMPARE(partials[0].size(), 61);
        QCOMPARE(quint8(partials[0][0x03]), quint8(64));
        QCOMPARE(partials[0].mid(0x35, 4), QByteArray::fromHex("0000070b"));
        QCOMPARE(quint8(partials[0][0x3c]), quint8(64));
        QCOMPARE(misc.size(), 37);
        QCOMPARE(quint8(misc[36]), quint8(36));
    }

    void loadsAndPushesSnSynthThroughPlatform()
    {
        QTemporaryFile file;
        QVERIFY(file.open());
        const QByteArray svd = sampleSvdFor(QByteArrayLiteral("SHPaMI73"), sampleSnSynthEntry());
        QCOMPARE(file.write(svd), qint64(svd.size()));
        file.flush();

        FakeInstrumentPlatform platform;
        UndoController undo;
        StudioSetModel studio(&platform, &undo);
        studio.setSelectedPart(2);
        SvdImportModel model(&platform, &studio);
        QVERIFY2(model.loadFile(QUrl::fromLocalFile(file.fileName())), qPrintable(model.lastError()));
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.data(model.index(0), SvdImportModel::CategoryRole).toString(), QStringLiteral("SN-S"));
        QCOMPARE(model.data(model.index(0), SvdImportModel::NameRole).toString(), QStringLiteral("Test SNSynth"));
        QVERIFY2(model.pushTone(0), qPrintable(model.lastError()));
        QCOMPARE(studio.selectedPartModel()->bankMsb(), 95);
        QCOMPARE(platform.stored(2, roland::ToneEngine::SnSynth,
                                 InstrumentPlatform::ToneSection::SnCommon, 0).size(), 64);
        QCOMPARE(platform.stored(2, roland::ToneEngine::SnSynth,
                                 InstrumentPlatform::ToneSection::SnPartial, 2).size(), 61);
        QCOMPARE(platform.stored(2, roland::ToneEngine::SnSynth,
                                 InstrumentPlatform::ToneSection::SnMisc, 0).size(), 37);
        QCOMPARE(platform.stored(2, roland::ToneEngine::SnSynth,
                                 InstrumentPlatform::ToneSection::Mfx, 0).size(), 145);
    }

    void rejectsWrongVersionAndTruncation()
    {
        FakeInstrumentPlatform platform;
        UndoController undo;
        StudioSetModel studio(&platform, &undo);
        SvdImportModel model(&platform, &studio);
        QTemporaryFile file;
        QVERIFY(file.open());
        file.write("SVD0"); file.flush();
        QVERIFY(!model.loadFile(QUrl::fromLocalFile(file.fileName())));
        QVERIFY(model.lastError().contains(QStringLiteral("SVD1")));

        QByteArray truncated = sampleSvd(); truncated.chop(1);
        file.resize(0); file.write(truncated); file.flush();
        QVERIFY(!model.loadFile(QUrl::fromLocalFile(file.fileName())));
    }
};

QTEST_APPLESS_MAIN(TestSvdImport)
#include "test_svd_import.moc"
