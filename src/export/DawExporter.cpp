#include "export/DawExporter.h"

#include "model/PartModel.h"
#include "model/StudioSetModel.h"
#include "model/ToneBrowserModel.h"

#include <QFileInfo>
#include <QSaveFile>
#include <QXmlStreamWriter>
#include <algorithm>
#include <tuple>

namespace {

void appendBe16(QByteArray &out, quint16 value)
{
    out.append(char((value >> 8) & 0xff));
    out.append(char(value & 0xff));
}

void appendBe32(QByteArray &out, quint32 value)
{
    out.append(char((value >> 24) & 0xff));
    out.append(char((value >> 16) & 0xff));
    out.append(char((value >> 8) & 0xff));
    out.append(char(value & 0xff));
}

void appendVariableLength(QByteArray &out, quint32 value)
{
    quint32 buffer = value & 0x7f;
    while ((value >>= 7) != 0)
        buffer = (buffer << 8) | ((value & 0x7f) | 0x80);
    for (;;) {
        out.append(char(buffer & 0xff));
        if (buffer & 0x80)
            buffer >>= 8;
        else
            break;
    }
}

void appendMetaText(QByteArray &track, quint8 type, const QString &text)
{
    const auto utf8 = text.toUtf8();
    appendVariableLength(track, 0);
    track.append(char(0xff));
    track.append(char(type));
    appendVariableLength(track, quint32(utf8.size()));
    track.append(utf8);
}

void appendTrack(QByteArray &file, const QByteArray &track)
{
    file.append("MTrk", 4);
    appendBe32(file, quint32(track.size()));
    file.append(track);
}

QString withSuffix(QString path, const QString &suffix)
{
    if (!path.endsWith(suffix, Qt::CaseInsensitive))
        path += suffix;
    return path;
}

bool commitFile(QSaveFile &file, QString *error)
{
    if (file.commit())
        return true;
    if (error)
        *error = file.errorString();
    return false;
}

}

bool DawExporter::writeStudioSetMidi(const StudioSetModel &studioSet, const QString &requestedPath, QString *error)
{
    const auto path = withSuffix(requestedPath, QStringLiteral(".mid"));
    QByteArray midi;
    midi.append("MThd", 4);
    appendBe32(midi, 6);
    appendBe16(midi, 1); // Standard MIDI File type 1
    appendBe16(midi, 17); // conductor + all 16 FA parts
    appendBe16(midi, 480);

    QByteArray conductor;
    appendMetaText(conductor, 0x03, studioSet.name());
    appendVariableLength(conductor, 0);
    conductor.append("\xff\x2f\x00", 3);
    appendTrack(midi, conductor);

    for (int index = 0; index < 16; ++index) {
        const auto *part = studioSet.partAt(index);
        if (!part) {
            if (error)
                *error = QStringLiteral("Studio Set is missing Part %1.").arg(index + 1);
            return false;
        }
        QByteArray track;
        appendMetaText(track, 0x03, QStringLiteral("%1 %2").arg(index + 1, 2, 10, QLatin1Char('0')).arg(part->toneName()));
        const quint8 channel = quint8(std::clamp(part->receiveChannel(), 0, 15));
        const auto cc = [&](quint8 controller, int value) {
            appendVariableLength(track, 0);
            track.append(char(0xb0 | channel));
            track.append(char(controller));
            track.append(char(std::clamp(value, 0, 127)));
        };
        cc(0, part->bankMsb());
        cc(32, part->bankLsb());
        appendVariableLength(track, 0);
        track.append(char(0xc0 | channel));
        track.append(char(std::clamp(part->program(), 0, 127)));
        cc(7, part->level());
        cc(10, part->pan());
        cc(91, part->reverbSend());
        cc(93, part->chorusSend());
        appendVariableLength(track, 0);
        track.append("\xff\x2f\x00", 3);
        appendTrack(midi, track);
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }
    if (file.write(midi) != midi.size()) {
        if (error)
            *error = file.errorString();
        file.cancelWriting();
        return false;
    }
    return commitFile(file, error);
}

bool DawExporter::writeMidnam(const ToneBrowserModel &tones, const QString &requestedPath, QString *error)
{
    const auto path = withSuffix(requestedPath, QStringLiteral(".midnam"));
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeDTD(QStringLiteral("<!DOCTYPE MIDINameDocument PUBLIC \"-//MIDI Manufacturers Association//DTD MIDINameDocument 1.0//EN\" \"http://www.midi.org/dtds/MIDINameDocument10.dtd\">"));
    xml.writeStartElement(QStringLiteral("MIDINameDocument"));
    xml.writeTextElement(QStringLiteral("Author"), QStringLiteral("FA Editor"));
    xml.writeStartElement(QStringLiteral("MasterDeviceNames"));
    xml.writeTextElement(QStringLiteral("Manufacturer"), QStringLiteral("Roland"));
    for (const auto &model : {QStringLiteral("FA-06"), QStringLiteral("FA-07"), QStringLiteral("FA-08")})
        xml.writeTextElement(QStringLiteral("Model"), model);
    xml.writeStartElement(QStringLiteral("CustomDeviceMode"));
    xml.writeAttribute(QStringLiteral("Name"), QStringLiteral("Default"));
    xml.writeStartElement(QStringLiteral("ChannelNameSetAssignments"));
    for (int channel = 1; channel <= 16; ++channel) {
        xml.writeEmptyElement(QStringLiteral("ChannelNameSetAssign"));
        xml.writeAttribute(QStringLiteral("Channel"), QString::number(channel));
        xml.writeAttribute(QStringLiteral("NameSet"), QStringLiteral("Roland FA Tones"));
    }
    xml.writeEndElement();
    xml.writeEndElement();

    xml.writeStartElement(QStringLiteral("ChannelNameSet"));
    xml.writeAttribute(QStringLiteral("Name"), QStringLiteral("Roland FA Tones"));
    xml.writeStartElement(QStringLiteral("AvailableForChannels"));
    for (int channel = 1; channel <= 16; ++channel) {
        xml.writeEmptyElement(QStringLiteral("AvailableChannel"));
        xml.writeAttribute(QStringLiteral("Channel"), QString::number(channel));
    }
    xml.writeEndElement();

    auto catalog = tones.catalogEntries();
    std::sort(catalog.begin(), catalog.end(), [](const ToneEntry &a, const ToneEntry &b) {
        return std::tie(a.bankMsb, a.bankLsb, a.program, a.name)
             < std::tie(b.bankMsb, b.bankLsb, b.program, b.name);
    });
    int pos = 0;
    while (pos < catalog.size()) {
        const int msb = catalog.at(pos).bankMsb;
        const int lsb = catalog.at(pos).bankLsb;
        const int begin = pos;
        while (pos < catalog.size() && catalog.at(pos).bankMsb == msb && catalog.at(pos).bankLsb == lsb)
            ++pos;
        xml.writeStartElement(QStringLiteral("PatchBank"));
        xml.writeAttribute(QStringLiteral("Name"), QStringLiteral("MSB %1 / LSB %2").arg(msb).arg(lsb));
        xml.writeStartElement(QStringLiteral("MIDICommands"));
        for (const auto pair : {qMakePair(0, msb), qMakePair(32, lsb)}) {
            xml.writeEmptyElement(QStringLiteral("ControlChange"));
            xml.writeAttribute(QStringLiteral("Control"), QString::number(pair.first));
            xml.writeAttribute(QStringLiteral("Value"), QString::number(pair.second));
        }
        xml.writeEndElement();
        xml.writeStartElement(QStringLiteral("PatchNameList"));
        for (int i = begin; i < pos; ++i) {
            xml.writeEmptyElement(QStringLiteral("Patch"));
            xml.writeAttribute(QStringLiteral("Number"), QString::number(catalog.at(i).program + 1));
            xml.writeAttribute(QStringLiteral("Name"), catalog.at(i).name);
            xml.writeAttribute(QStringLiteral("ProgramChange"), QString::number(catalog.at(i).program));
        }
        xml.writeEndElement();
        xml.writeEndElement();
    }
    xml.writeEndElement(); // ChannelNameSet
    xml.writeEndElement(); // MasterDeviceNames
    xml.writeEndElement(); // MIDINameDocument
    xml.writeEndDocument();

    if (xml.hasError()) {
        if (error)
            *error = QStringLiteral("Could not generate MIDI Name Document.");
        file.cancelWriting();
        return false;
    }
    return commitFile(file, error);
}
