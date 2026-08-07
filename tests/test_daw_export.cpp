#include "export/DawExporter.h"
#include "model/PartModel.h"
#include "model/StudioSetModel.h"
#include "model/ToneBrowserModel.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QXmlStreamReader>
#include <QtTest>

class DawExportTests : public QObject
{
    Q_OBJECT
private slots:
    void studioSetMidiContainsSixteenNamedPatchTracks();
    void midnamContainsCatalogBanksAndEscapedNames();
};

void DawExportTests::studioSetMidiContainsSixteenNamedPatchTracks()
{
    QTemporaryDir dir; QVERIFY(dir.isValid());
    StudioSetModel set(nullptr, nullptr);
    set.setName(QStringLiteral("Live Set"));
    auto *part = set.partAt(0); QVERIFY(part);
    part->setToneName(QStringLiteral("Full Grand 1"));
    part->setBankMsb(89); part->setBankLsb(64); part->setProgram(7);
    part->setReceiveChannel(2); part->setLevel(101); part->setPan(63);
    QString error;
    const auto path = dir.filePath(QStringLiteral("setup.mid"));
    QVERIFY2(DawExporter::writeStudioSetMidi(set, path, &error), qPrintable(error));
    QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly)); const auto bytes=file.readAll();
    QCOMPARE(bytes.left(4), QByteArray("MThd"));
    QCOMPARE(quint8(bytes.at(9)), quint8(1));
    QCOMPARE((quint8(bytes.at(10)) << 8) | quint8(bytes.at(11)), 17);
    QCOMPARE(bytes.count(QByteArray("MTrk")), 17);
    QVERIFY(bytes.contains("01 Full Grand 1"));
    QVERIFY(bytes.contains(QByteArray::fromHex("00b2005900b2204000c20700b2076500b20a3f")));
}

void DawExportTests::midnamContainsCatalogBanksAndEscapedNames()
{
    QTemporaryDir dir; QVERIFY(dir.isValid());
    const auto catalogPath=dir.filePath(QStringLiteral("tones.json"));
    QJsonArray tones{
        QJsonObject{{"name","Piano & Bell"},{"category","Piano"},{"bankMsb",89},{"bankLsb",64},{"program",0}},
        QJsonObject{{"name","Wide Pad"},{"category","Synth"},{"bankMsb",95},{"bankLsb",65},{"program",12}}
    };
    QFile catalog(catalogPath); QVERIFY(catalog.open(QIODevice::WriteOnly));
    catalog.write(QJsonDocument(tones).toJson()); catalog.close();
    ToneBrowserModel model; QVERIFY(model.loadCatalog(catalogPath));
    QString error; const auto path=dir.filePath(QStringLiteral("Roland_FA.midnam"));
    QVERIFY2(DawExporter::writeMidnam(model, path, &error), qPrintable(error));
    QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly)); const auto bytes=file.readAll();
    QVERIFY(bytes.contains("MIDINameDocument"));
    QVERIFY(bytes.contains("Piano &amp; Bell"));
    QVERIFY(bytes.contains("Control=\"0\" Value=\"89\""));
    QVERIFY(bytes.contains("Control=\"32\" Value=\"65\""));
    QVERIFY(bytes.contains("Number=\"1\" Name=\"Piano &amp; Bell\" ProgramChange=\"0\""));
    QXmlStreamReader xml(bytes); while (!xml.atEnd()) xml.readNext();
    QVERIFY2(!xml.hasError(), qPrintable(xml.errorString()));
}

QTEST_GUILESS_MAIN(DawExportTests)
#include "test_daw_export.moc"
