#include "model/SvdImportModel.h"

#include "midi/AddressMap.h"
#include "model/MfxModel.h"
#include "model/PartModel.h"
#include "model/SnAcousticToneModel.h"
#include "model/SnSynthToneModel.h"
#include "model/StudioSetModel.h"
#include "platform/InstrumentPlatform.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
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

bool decodeMfx(BitReader &r, QByteArray *mfx)
{
    QByteArray fx;
    fx.reserve(roland::mfxOff::MfxSize);
    for (int i = 0; i < 4; ++i) fx.append(char(r.read(7)));
    fx.append(char(r.read(2)));
    for (int i = 0; i < 8; ++i) fx.append(char(r.read(7)));
    for (int i = 0; i < 4; ++i) fx.append(char(r.read(5)));
    for (int i = 0; i < 32; ++i) appendNibbles(fx, r.read(16));
    r.skip(6);
    if (fx.size() != roland::mfxOff::MfxSize)
        return false;
    *mfx = fx;
    return true;
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
    if (role == SlotRole) return tone.slot;
    if (role == CategoryRole)
        return tone.engine == Engine::SnSynth ? QStringLiteral("SN-S") : QStringLiteral("SN-A");
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

    struct Area { QByteArray name; quint32 offset; quint32 length; };
    QVector<Area> areas;
    for (int pos = 16; pos + 16 <= firstArea; pos += 16) {
        const QByteArray name = bytes.mid(pos, 8);
        if (name == QByteArrayLiteral("SNTaMI73") || name == QByteArrayLiteral("SHPaMI73"))
            areas.push_back({name, be32(bytes, pos + 8), be32(bytes, pos + 12)});
    }
    if (areas.isEmpty()) {
        setError(QStringLiteral("This backup has no supported FA MI73 tone area."));
        return false;
    }

    QVector<Tone> tones;
    for (const auto &area : areas) {
        if (!area.offset || area.offset + area.length > quint32(bytes.size()) || area.length < 16) {
            setError(QStringLiteral("Invalid FA tone area bounds."));
            return false;
        }
        const quint32 count = be32(bytes, int(area.offset));
        const quint32 entrySize = be32(bytes, int(area.offset) + 4);
        const quint32 entryOffset = be32(bytes, int(area.offset) + 8);
        const bool synth = area.name == QByteArrayLiteral("SHPaMI73");
        const quint32 expectedCount = synth ? 512u : 128u;
        const quint32 expectedSize = synth ? 280u : 138u;
        if (count > expectedCount || entrySize != expectedSize || entryOffset < 16
            || quint64(entryOffset) + quint64(count) * entrySize > area.length) {
            setError(QStringLiteral("Invalid FA %1 area dimensions.")
                         .arg(synth ? QStringLiteral("SN-S") : QStringLiteral("SN-A")));
            return false;
        }
        tones.reserve(tones.size() + int(count));
        for (quint32 i = 0; i < count; ++i) {
            const int pos = int(area.offset + entryOffset + i * entrySize);
            const QByteArray packed = bytes.mid(pos, int(entrySize));
            QByteArray common, mfx, misc;
            std::array<QByteArray, 3> partials;
            QString error;
            const bool ok = synth
                ? decodeSnSynth(packed, &common, &mfx, &partials, &misc, &error)
                : decodeSnAcoustic(packed, &common, &mfx, &error);
            if (!ok) {
                setError(QStringLiteral("%1 tone %2: %3")
                             .arg(synth ? QStringLiteral("SN-S") : QStringLiteral("SN-A"))
                             .arg(i + 1).arg(error));
                return false;
            }
            tones.push_back({QString::fromLatin1(common.left(12)).trimmed(), packed,
                             synth ? Engine::SnSynth : Engine::SnAcoustic, int(i + 1)});
        }
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
    // SVD and FA SysEx both store the internal instrument selector. The
    // one-based number shown on the FA panel is presentation only.
    c[0x21] = char(r.read(7));
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
    if (!decodeMfx(r, &fx) || r.position() != 1104) {
        if (error) *error = QStringLiteral("Internal MFX layout mismatch.");
        return false;
    }
    *common = c;
    *mfx = fx;
    return true;
}

bool SvdImportModel::decodeSnSynth(const QByteArray &packed, QByteArray *common,
                                   QByteArray *mfx, std::array<QByteArray, 3> *partials,
                                   QByteArray *misc, QString *error)
{
    if (!common || !mfx || !partials || !misc || packed.size() != 280) {
        if (error) *error = QStringLiteral("SN-S entry must be exactly 280 bytes.");
        return false;
    }
    BitReader r(packed);
    QByteArray c(roland::snSynthOff::CommonSize, '\0');
    for (int i = 0; i < 12; ++i) c[i] = char(r.read(7));
    c[0x0c] = char(r.read(7));
    for (int i = 0x0d; i <= 0x0f; ++i) c[i] = char(r.read(4));
    c[0x10] = char(r.read(1));
    c[0x11] = char(r.read(1));
    c[0x12] = char(r.read(1));
    c[0x13] = char(r.read(7));
    c[0x14] = char(r.read(2));
    c[0x15] = char(r.read(3) + 60);
    c[0x16] = char(r.read(5));
    c[0x17] = char(r.read(5));
    c[0x18] = char(r.read(3));
    for (int i = 0x19; i <= 0x1e; ++i) c[i] = char(r.read(1));
    c[0x1f] = char(r.read(2));
    c[0x20] = char(r.read(1));
    for (int i = 0x21; i <= 0x26; ++i) c[i] = char(r.read(2));
    for (int i = 0x27; i <= 0x2d; ++i) c[i] = char(r.read(1));
    for (int i = 0x2e; i <= 0x33; ++i) c[i] = char(r.read(1));
    for (int i = 0x34; i <= 0x36; ++i) c[i] = char(r.read(7));
    const int phrase = r.read(16);
    c[0x37] = char((phrase >> 12) & 0x0f);
    c[0x38] = char((phrase >> 8) & 0x0f);
    c[0x39] = char((phrase >> 4) & 0x0f);
    c[0x3a] = char(phrase & 0x0f);
    c[0x3b] = char(r.read(3) + 60);
    c[0x3c] = char(r.read(2));
    for (int i = 0x3d; i <= 0x3f; ++i) c[i] = char(r.read(7));
    r.skip(12);
    if (r.position() != 240) {
        if (error) *error = QStringLiteral("Internal SN-S Common layout mismatch.");
        return false;
    }

    QByteArray fx;
    if (!decodeMfx(r, &fx) || r.position() != 864) {
        if (error) *error = QStringLiteral("Internal SN-S MFX layout mismatch.");
        return false;
    }

    std::array<QByteArray, 3> ps;
    for (auto &p : ps) {
        p = QByteArray(roland::snSynthOff::PartialSize, '\0');
        p[0x00] = char(r.read(3));
        p[0x01] = char(r.read(6));
        p[0x02] = char(r.read(2));
        p[0x03] = char(r.read(6) + 32);
        for (int i = 0x04; i <= 0x09; ++i) p[i] = char(r.read(7));
        p[0x0a] = char(r.read(3));
        p[0x0b] = char(r.read(1));
        p[0x0c] = char(r.read(7));
        p[0x0d] = char(r.read(6) + 32);
        for (int i = 0x0e; i <= 0x1b; ++i) p[i] = char(r.read(7));
        p[0x1c] = char(r.read(3));
        p[0x1d] = char(r.read(7));
        p[0x1e] = char(r.read(1));
        p[0x1f] = char(r.read(5));
        p[0x20] = char(r.read(7));
        p[0x21] = char(r.read(1));
        for (int i = 0x22; i <= 0x25; ++i) p[i] = char(r.read(7));
        p[0x26] = char(r.read(3));
        p[0x27] = char(r.read(7));
        p[0x28] = char(r.read(1));
        p[0x29] = char(r.read(5));
        p[0x2a] = char(r.read(7));
        p[0x2b] = char(r.read(1));
        for (int i = 0x2c; i <= 0x33; ++i) p[i] = char(r.read(7));
        p[0x34] = char(r.read(2));
        const int wave = r.read(16);
        p[0x35] = char((wave >> 12) & 0x0f);
        p[0x36] = char((wave >> 8) & 0x0f);
        p[0x37] = char((wave >> 4) & 0x0f);
        p[0x38] = char(wave & 0x0f);
        for (int i = 0x39; i <= 0x3b; ++i) p[i] = char(r.read(7));
        p[0x3c] = char(r.read(5) + 48);
        r.skip(18);
    }
    if (r.position() != 1968) {
        if (error) *error = QStringLiteral("Internal SN-S Partial layout mismatch.");
        return false;
    }

    QByteArray mi(roland::snSynthOff::MiscSize, '\0');
    for (int i = 0; i < mi.size(); ++i) mi[i] = char(r.read(7));
    r.skip(13);
    if (r.position() != 2240) {
        if (error) *error = QStringLiteral("Internal SN-S Misc layout mismatch.");
        return false;
    }
    *common = c;
    *mfx = fx;
    *partials = ps;
    *misc = mi;
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
    const auto &tone = m_tones.at(row);
    QByteArray common, mfx, misc;
    std::array<QByteArray, 3> partials;
    QString error;
    const bool synth = tone.engine == Engine::SnSynth;
    const bool decoded = synth
        ? decodeSnSynth(tone.packed, &common, &mfx, &partials, &misc, &error)
        : decodeSnAcoustic(tone.packed, &common, &mfx, &error);
    if (!decoded) {
        setError(error);
        return false;
    }

    // Select a known preset SN-A tone only to establish the correct Temporary
    // Tone engine. Do not depend on the same-numbered User slot: the imported
    // backup is not necessarily installed on this FA, and an empty/late User
    // recall can otherwise overwrite the Temporary data below.
    part->setBankMsb(synth ? 95 : 89);
    part->setBankLsb(64);
    part->setProgram(0);
    QThread::msleep(250);
    part->setToneName(tone.name);

    bool toneOk = false;
    QString toneError;
    if (synth) {
        SnSynthToneModel sns(m_platform);
        sns.setPartIndex(part->partNumber() - 1);
        QJsonArray partialJson;
        for (const auto &p : partials)
            partialJson.append(QString::fromLatin1(p.toBase64()));
        sns.fromJson({{QStringLiteral("common"), QString::fromLatin1(common.toBase64())},
                      {QStringLiteral("misc"), QString::fromLatin1(misc.toBase64())},
                      {QStringLiteral("partials"), partialJson}});
        toneOk = sns.pushToDevice();
        toneError = sns.lastError();
    } else {
        SnAcousticToneModel sna(m_platform);
        sna.setPartIndex(part->partNumber() - 1);
        sna.fromJson({{QStringLiteral("common"), QString::fromLatin1(common.toBase64())}});
        toneOk = sna.pushToDevice();
        toneError = sna.lastError();
    }
    MfxModel fx(m_platform);
    fx.setContext(part->partNumber() - 1,
                  synth ? roland::ToneEngine::SnSynth : roland::ToneEngine::SnAcoustic);
    fx.fromJson({{QStringLiteral("raw"), QString::fromLatin1(mfx.toBase64())},
                 {QStringLiteral("switch"), bool(quint8(common[synth ? 0x20 : 0x1f]))}});
    if (!toneOk || !fx.pushToDevice()) {
        setError(toneError.isEmpty() ? fx.lastError() : toneError);
        return false;
    }
    setError({});
    emit tonePushed(tone.name, part->partNumber());
    return true;
}
