#include "model/SvdImportModel.h"

#include "midi/AddressMap.h"
#include "model/MfxModel.h"
#include "model/PartModel.h"
#include "model/SnAcousticToneModel.h"
#include "model/StudioSetModel.h"
#include "platform/InstrumentPlatform.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QThread>
#include <QUrl>

namespace {

quint32 be32(const QByteArray &data, int offset)
{
    const auto *p = reinterpret_cast<const uchar *>(data.constData() + offset);
    return (quint32(p[0]) << 24) | (quint32(p[1]) << 16) | (quint32(p[2]) << 8) | p[3];
}

class BitReader
{
public:
    explicit BitReader(const QByteArray &data) : m_data(data) {}
    int read(int width)
    {
        int value = 0;
        for (int i = 0; i < width; ++i) {
            value = (value << 1) | ((quint8(m_data.at(m_bit / 8)) >> (7 - m_bit % 8)) & 1);
            ++m_bit;
        }
        return value;
    }
    void skip(int width) { m_bit += width; }
    int position() const { return m_bit; }
private:
    QByteArray m_data;
    int m_bit = 0;
};

void appendNibbles(QByteArray &out, int value)
{
    out.append(char((value >> 12) & 0x0f));
    out.append(char((value >> 8) & 0x0f));
    out.append(char((value >> 4) & 0x0f));
    out.append(char(value & 0x0f));
}

} // namespace

SvdImportModel::SvdImportModel(InstrumentPlatform *platform, StudioSetModel *studioSet,
                               QObject *parent)
    : QAbstractListModel(parent), m_platform(platform), m_studioSet(studioSet)
{
}

int SvdImportModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_tones.size();
}

QVariant SvdImportModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tones.size())
        return {};
    const auto &tone = m_tones.at(index.row());
    if (role == NameRole) return tone.name;
    if (role == SlotRole) return index.row() + 1;
    if (role == CategoryRole) return QStringLiteral("SN-A");
    return {};
}

QHash<int, QByteArray> SvdImportModel::roleNames() const
{
    return {{NameRole, "name"}, {SlotRole, "slot"}, {CategoryRole, "category"}};
}

void SvdImportModel::setError(const QString &error)
{
    if (m_lastError == error) return;
    m_lastError = error;
    emit lastErrorChanged();
}

bool SvdImportModel::loadFile(const QUrl &url)
{
    const QString path = url.isLocalFile() ? url.toLocalFile() : url.toString();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(QStringLiteral("Could not open SVD file."));
        return false;
    }
    const QByteArray bytes = file.readAll();
    if (bytes.size() < 16 || bytes.mid(2, 4) != QByteArrayLiteral("SVD1")) {
        setError(QStringLiteral("Unsupported backup: expected Roland SVD1."));
        return false;
    }
    const int followingHeaderLength = (quint8(bytes[0]) << 8) | quint8(bytes[1]);
    const int firstArea = 2 + followingHeaderLength;
    if (followingHeaderLength < 14 || firstArea > bytes.size()
        || (firstArea - 16) % 16 != 0) {
        setError(QStringLiteral("Invalid SVD header length."));
        return false;
    }

    quint32 areaOffset = 0;
    quint32 areaLength = 0;
    for (int pos = 16; pos + 16 <= firstArea; pos += 16) {
        if (bytes.mid(pos, 8) == QByteArrayLiteral("SNTaMI73")) {
            areaOffset = be32(bytes, pos + 8);
            areaLength = be32(bytes, pos + 12);
            break;
        }
    }
    if (!areaOffset || areaOffset + areaLength > quint32(bytes.size()) || areaLength < 16) {
        setError(QStringLiteral("This backup has no FA MI73 SN-A tone area."));
        return false;
    }
    const quint32 count = be32(bytes, int(areaOffset));
    const quint32 entrySize = be32(bytes, int(areaOffset) + 4);
    const quint32 entryOffset = be32(bytes, int(areaOffset) + 8);
    if (count > 128 || entrySize != 138 || entryOffset < 16
        || quint64(entryOffset) + quint64(count) * entrySize > areaLength) {
        setError(QStringLiteral("Invalid FA SN-A area dimensions."));
        return false;
    }

    QVector<Tone> tones;
    tones.reserve(int(count));
    for (quint32 i = 0; i < count; ++i) {
        const int pos = int(areaOffset + entryOffset + i * entrySize);
        const QByteArray packed = bytes.mid(pos, int(entrySize));
        QByteArray common, mfx;
        QString error;
        if (!decodeSnAcoustic(packed, &common, &mfx, &error)) {
            setError(QStringLiteral("Tone %1: %2").arg(i + 1).arg(error));
            return false;
        }
        tones.push_back({QString::fromLatin1(common.left(12)).trimmed(), packed});
    }

    beginResetModel();
    m_tones = std::move(tones);
    m_sourceName = QFileInfo(path).fileName();
    endResetModel();
    setError({});
    emit sourceChanged();
    return true;
}

bool SvdImportModel::decodeSnAcoustic(const QByteArray &packed, QByteArray *common,
                                      QByteArray *mfx, QString *error)
{
    if (!common || !mfx || packed.size() != 138) {
        if (error) *error = QStringLiteral("SN-A entry must be exactly 138 bytes.");
        return false;
    }
    BitReader r(packed);
    QByteArray c(roland::snAcousticOff::CommonSize, '\0');
    for (int i = 0; i < 16; ++i) c[i] = char(r.read(7));
    c[0x10] = char(r.read(7));
    c[0x11] = char(r.read(1));
    for (int i = 0x12; i <= 0x19; ++i) c[i] = char(r.read(7));
    c[0x1a] = char(r.read(3) + 60);
    c[0x1b] = char(r.read(7));
    const int phrase = r.read(8);
    c[0x1c] = char((phrase >> 4) & 0x0f);
    c[0x1d] = char(phrase & 0x0f);
    c[0x1e] = char(r.read(3) + 60);
    c[0x1f] = char(r.read(1));
    c[0x20] = char(r.read(7));
    c[0x21] = char(r.read(7) + 1);
    for (int i = 0x22; i <= 0x41; ++i) c[i] = char(r.read(7));
    c[0x42] = char(r.read(2));
    c[0x43] = char(r.read(5));
    for (int i = 0x44; i <= 0x46; ++i) c[i] = char(r.read(7));
    r.skip(16);
    if (r.position() != 480) {
        if (error) *error = QStringLiteral("Internal Common layout mismatch.");
        return false;
    }

    QByteArray fx;
    fx.reserve(145);
    for (int i = 0; i < 4; ++i) fx.append(char(r.read(7)));
    fx.append(char(r.read(2)));
    for (int i = 0; i < 8; ++i) fx.append(char(r.read(7)));
    for (int i = 0; i < 4; ++i) fx.append(char(r.read(5)));
    for (int i = 0; i < 32; ++i) appendNibbles(fx, r.read(16));
    r.skip(6);
    if (r.position() != 1104 || fx.size() != 145) {
        if (error) *error = QStringLiteral("Internal MFX layout mismatch.");
        return false;
    }
    *common = c;
    *mfx = fx;
    return true;
}

bool SvdImportModel::pushTone(int row)
{
    if (row < 0 || row >= m_tones.size()) {
        setError(QStringLiteral("Select an imported tone first."));
        return false;
    }
    if (!m_platform || !m_platform->isConnected() || !m_studioSet) {
        setError(QStringLiteral("Connect the FA before pushing an imported tone."));
        return false;
    }
    auto *part = m_studioSet->selectedPartModel();
    if (!part) {
        setError(QStringLiteral("Select a Studio Set part first."));
        return false;
    }
    QByteArray common, mfx;
    QString error;
    if (!decodeSnAcoustic(m_tones.at(row).packed, &common, &mfx, &error)) {
        setError(error);
        return false;
    }

    // Select a known preset SN-A tone only to establish the correct Temporary
    // Tone engine. Do not depend on the same-numbered User slot: the imported
    // backup is not necessarily installed on this FA, and an empty/late User
    // recall can otherwise overwrite the Temporary data below.
    part->setBankMsb(89);
    part->setBankLsb(64);
    part->setProgram(0);
    QThread::msleep(250);
    part->setToneName(m_tones.at(row).name);

    SnAcousticToneModel sna(m_platform);
    sna.setPartIndex(part->partNumber() - 1);
    sna.fromJson({{QStringLiteral("common"), QString::fromLatin1(common.toBase64())}});
    MfxModel fx(m_platform);
    fx.setContext(part->partNumber() - 1, roland::ToneEngine::SnAcoustic);
    fx.fromJson({{QStringLiteral("raw"), QString::fromLatin1(mfx.toBase64())},
                 {QStringLiteral("switch"), bool(quint8(common[0x1f]))}});
    if (!sna.pushToDevice() || !fx.pushToDevice()) {
        setError(sna.lastError().isEmpty() ? fx.lastError() : sna.lastError());
        return false;
    }
    setError({});
    emit tonePushed(m_tones.at(row).name, part->partNumber());
    return true;
}
