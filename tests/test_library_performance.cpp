#include "model/ToneBrowserModel.h"
#include "model/WaveformCatalog.h"

#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

class LibraryPerformanceTests : public QObject
{
    Q_OBJECT
private slots:
    void toneCatalogLoadAndRepeatedFilters();
    void waveformLoadSortAndRepeatedFilters();
};

void LibraryPerformanceTests::toneCatalogLoadAndRepeatedFilters()
{
    QTemporaryDir dir; QVERIFY(dir.isValid());
    const QString path=dir.filePath(QStringLiteral("tones.json"));
    QJsonArray tones;
    for(int i=0;i<5000;++i) tones.append(QJsonObject{{QStringLiteral("name"),QStringLiteral("Tone %1 Pad").arg(i)},
        {QStringLiteral("category"),QStringLiteral("Category %1").arg(i%24)},
        {QStringLiteral("bankMsb"),i/16384},{QStringLiteral("bankLsb"),(i/128)%128},{QStringLiteral("program"),i%128}});
    QFile file(path); QVERIFY(file.open(QIODevice::WriteOnly)); file.write(QJsonDocument(tones).toJson(QJsonDocument::Compact)); file.close();

    ToneBrowserModel model; QElapsedTimer timer; timer.start();
    QVERIFY(model.loadCatalog(path));
    const qint64 loadMs=timer.elapsed();
    QCOMPARE(model.rowCount(),5000);
    QVERIFY2(loadMs<3000,qPrintable(QStringLiteral("Tone catalog load took %1 ms").arg(loadMs)));

    timer.restart();
    for(int i=0;i<200;++i) {
        model.setFilterText(QStringLiteral("Tone %1").arg(i%100));
        model.setCategory(i%3?QStringLiteral("All"):QStringLiteral("Category %1").arg(i%24));
    }
    const qint64 filterMs=timer.elapsed();
    qInfo() << "Library performance: tone load" << loadMs << "ms, 200 filter/category updates" << filterMs << "ms";
    QVERIFY2(filterMs<2500,qPrintable(QStringLiteral("Tone filtering took %1 ms").arg(filterMs)));
}

void LibraryPerformanceTests::waveformLoadSortAndRepeatedFilters()
{
    QTemporaryDir dir; QVERIFY(dir.isValid());
    const QString path=dir.filePath(QStringLiteral("waves.json"));
    auto table=[](int count){QJsonObject o;for(int i=count;i>=1;--i)o.insert(QString::number(i),QStringLiteral("Wave %1 Bright").arg(i));return o;};
    QJsonObject root{{QStringLiteral("snSynthPcm"),table(2000)},
                     {QStringLiteral("pcmIntA"),table(2000)},
                     {QStringLiteral("pcmIntB"),table(2000)}};
    QFile file(path); QVERIFY(file.open(QIODevice::WriteOnly)); file.write(QJsonDocument(root).toJson(QJsonDocument::Compact)); file.close();

    WaveformCatalog model; QElapsedTimer timer; timer.start();
    QVERIFY(model.loadCatalog(path));
    const qint64 loadMs=timer.elapsed();
    QCOMPARE(model.snCount(),2000); QCOMPARE(model.rowCount(),2001);
    QVERIFY2(loadMs<3000,qPrintable(QStringLiteral("Waveform load/sort took %1 ms").arg(loadMs)));

    timer.restart();
    const QStringList banks{QStringLiteral("sn"),QStringLiteral("intA"),QStringLiteral("intB")};
    for(int i=0;i<200;++i) { model.setBank(banks.at(i%3)); model.setFilterText(QString::number(i%100)); }
    const qint64 filterMs=timer.elapsed();
    qInfo() << "Library performance: waveform load/sort" << loadMs << "ms, 200 bank/filter updates" << filterMs << "ms";
    QVERIFY2(filterMs<2500,qPrintable(QStringLiteral("Waveform filtering took %1 ms").arg(filterMs)));
}

QTEST_GUILESS_MAIN(LibraryPerformanceTests)
#include "test_library_performance.moc"
