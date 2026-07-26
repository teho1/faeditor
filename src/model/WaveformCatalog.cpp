#include "model/WaveformCatalog.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

namespace {

QHash<int, QString> parseTable(const QJsonObject &obj)
{
    QHash<int, QString> out;
    out.reserve(obj.size());
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        bool ok = false;
        const int n = it.key().toInt(&ok);
        if (!ok || n <= 0)
            continue;
        const QString name = it.value().toString().trimmed();
        if (!name.isEmpty())
            out.insert(n, name);
    }
    return out;
}

} // namespace

WaveformCatalog::WaveformCatalog(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVector<WaveformCatalog::WaveEntry> WaveformCatalog::sortedEntries(const QHash<int, QString> &table)
{
    QVector<WaveEntry> out;
    out.reserve(table.size() + 1);
    out.append({0, QStringLiteral("OFF")});
    QList<int> keys = table.keys();
    std::sort(keys.begin(), keys.end());
    for (int n : keys)
        out.append({n, table.value(n)});
    return out;
}

int WaveformCatalog::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_filtered.size();
}

QVariant WaveformCatalog::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filtered.size())
        return {};
    const auto &e = m_filtered.at(index.row());
    switch (role) {
    case NumberRole:
        return e.number;
    case NameRole:
        return e.name;
    default:
        return {};
    }
}

QHash<int, QByteArray> WaveformCatalog::roleNames() const
{
    return {
        {NumberRole, "number"},
        {NameRole, "name"}
    };
}

void WaveformCatalog::setBank(const QString &b)
{
    QString normalized = b.trimmed().toLower();
    if (normalized == QLatin1String("inta") || normalized == QLatin1String("int-a"))
        normalized = QStringLiteral("intA");
    else if (normalized == QLatin1String("intb") || normalized == QLatin1String("int-b"))
        normalized = QStringLiteral("intB");
    else if (normalized == QLatin1String("srx"))
        normalized = QStringLiteral("srx");
    else
        normalized = QStringLiteral("sn");

    if (m_bank == normalized)
        return;
    m_bank = normalized;
    rebuildFiltered();
    emit filterChanged();
}

void WaveformCatalog::setFilterText(const QString &t)
{
    if (m_filterText == t)
        return;
    m_filterText = t;
    rebuildFiltered();
    emit filterChanged();
}

bool WaveformCatalog::bankHasNames() const
{
    if (m_bank == QLatin1String("srx"))
        return false;
    const auto *table = activeTable();
    return table && table->size() > 1; // more than OFF
}

QString WaveformCatalog::bankDisplayName() const
{
    if (m_bank == QLatin1String("intA"))
        return QStringLiteral("PCM INT-A");
    if (m_bank == QLatin1String("intB"))
        return QStringLiteral("PCM INT-B");
    if (m_bank == QLatin1String("srx"))
        return QStringLiteral("SRX");
    return QStringLiteral("SN-S PCM");
}

const QVector<WaveformCatalog::WaveEntry> *WaveformCatalog::activeTable() const
{
    if (m_bank == QLatin1String("intA"))
        return &m_pcmIntAList;
    if (m_bank == QLatin1String("intB"))
        return &m_pcmIntBList;
    if (m_bank == QLatin1String("srx"))
        return nullptr;
    return &m_snList;
}

void WaveformCatalog::rebuildSortedTables()
{
    m_snList = sortedEntries(m_sn);
    m_pcmIntAList = sortedEntries(m_pcmIntA);
    m_pcmIntBList = sortedEntries(m_pcmIntB);
}

void WaveformCatalog::rebuildFiltered()
{
    beginResetModel();
    m_filtered.clear();
    const auto *table = activeTable();
    if (!table) {
        endResetModel();
        return;
    }

    const QString needle = m_filterText.trimmed();
    if (needle.isEmpty()) {
        m_filtered = *table;
    } else {
        m_filtered.reserve(table->size());
        for (const auto &e : *table) {
            if (e.name.contains(needle, Qt::CaseInsensitive)
                || QString::number(e.number).contains(needle)) {
                m_filtered.append(e);
            }
        }
    }
    endResetModel();
}

bool WaveformCatalog::loadCatalog(const QString &path)
{
    setBusy(true);
    setError({});

    QString filePath = path;
    if (filePath.isEmpty())
        filePath = QStringLiteral(":/qt/qml/FAEditor/resources/waves/waveforms.json");

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        const QStringList alts = {
            QStringLiteral(":/FAEditor/resources/waves/waveforms.json"),
            QStringLiteral(":/resources/waves/waveforms.json"),
            QStringLiteral("resources/waves/waveforms.json"),
            filePath
        };
        bool opened = false;
        for (const auto &alt : alts) {
            f.setFileName(alt);
            if (f.open(QIODevice::ReadOnly)) {
                opened = true;
                break;
            }
        }
        if (!opened) {
            setBusy(false);
            setError(QStringLiteral("Waveform list not found"));
            setStatus(QStringLiteral("Waveform names unavailable"));
            return false;
        }
    }

    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) {
        setBusy(false);
        setError(QStringLiteral("Invalid waveforms.json"));
        setStatus(QStringLiteral("Waveform names failed to load"));
        return false;
    }

    const QJsonObject root = doc.object();
    m_sn = parseTable(root.value(QStringLiteral("snSynthPcm")).toObject());
    m_pcmIntA = parseTable(root.value(QStringLiteral("pcmIntA")).toObject());
    m_pcmIntB = parseTable(root.value(QStringLiteral("pcmIntB")).toObject());
    m_loaded = !m_sn.isEmpty() || !m_pcmIntA.isEmpty() || !m_pcmIntB.isEmpty();

    rebuildSortedTables();
    rebuildFiltered();

    setBusy(false);
    if (!m_loaded) {
        setError(QStringLiteral("Waveform list empty"));
        setStatus(QStringLiteral("No waveform names loaded"));
        qWarning() << "WaveformCatalog: empty tables from" << f.fileName();
        emit catalogChanged();
        emit filterChanged();
        return false;
    }

    setStatus(QStringLiteral("Waveforms: SN-S %1 · INT-A %2 · INT-B %3")
                  .arg(m_sn.size())
                  .arg(m_pcmIntA.size())
                  .arg(m_pcmIntB.size()));
    qInfo() << "WaveformCatalog loaded:" << m_statusText << "from" << f.fileName();
    emit catalogChanged();
    emit filterChanged();
    return true;
}

bool WaveformCatalog::fetchWaveformNames()
{
    // Same path as startup load — optional manual reload of bundled JSON.
    setStatus(QStringLiteral("Loading waveform names…"));
    return loadCatalog();
}

QString WaveformCatalog::resolveSnName(int waveNumber) const
{
    if (waveNumber <= 0)
        return {};
    return m_sn.value(waveNumber);
}

WaveformCatalog::PcmBank WaveformCatalog::pcmBank(int waveGroupType, int waveGroupId) const
{
    if (waveGroupType == 1)
        return PcmBank::Srx;
    if (waveGroupType != 0)
        return PcmBank::Unknown;
    // MIDI: Wave Group ID 0 = OFF, 1–…; FA panel INT-A / INT-B map to ID 1 / 2.
    if (waveGroupId <= 1)
        return PcmBank::IntA;
    if (waveGroupId == 2)
        return PcmBank::IntB;
    return PcmBank::Unknown;
}

QString WaveformCatalog::resolvePcmName(int waveGroupType, int waveGroupId, int waveNumber) const
{
    if (waveNumber <= 0)
        return {};
    switch (pcmBank(waveGroupType, waveGroupId)) {
    case PcmBank::IntA:
        return m_pcmIntA.value(waveNumber);
    case PcmBank::IntB:
        return m_pcmIntB.value(waveNumber);
    default:
        return {};
    }
}

QString WaveformCatalog::displaySn(int waveNumber) const
{
    if (waveNumber <= 0)
        return QStringLiteral("OFF");
    const QString name = resolveSnName(waveNumber);
    if (!name.isEmpty())
        return name;
    return QStringLiteral("#%1").arg(waveNumber);
}

QString WaveformCatalog::displayPcm(int waveGroupType, int waveGroupId, int waveNumber) const
{
    if (waveNumber <= 0)
        return QStringLiteral("OFF");
    const QString name = resolvePcmName(waveGroupType, waveGroupId, waveNumber);
    if (!name.isEmpty())
        return name;
    return QStringLiteral("#%1").arg(waveNumber);
}

QString WaveformCatalog::pcmBankLabel(int waveGroupType, int waveGroupId) const
{
    switch (pcmBank(waveGroupType, waveGroupId)) {
    case PcmBank::IntA:
        return QStringLiteral("INT-A");
    case PcmBank::IntB:
        return QStringLiteral("INT-B");
    case PcmBank::Srx:
        return QStringLiteral("SRX");
    default:
        if (waveGroupType == 0)
            return QStringLiteral("INT");
        return QStringLiteral("—");
    }
}

int WaveformCatalog::indexOfWave(int waveNumber) const
{
    for (int i = 0; i < m_filtered.size(); ++i) {
        if (m_filtered.at(i).number == waveNumber)
            return i;
    }
    return -1;
}

QVariantMap WaveformCatalog::waveAt(int row) const
{
    QVariantMap m;
    if (row < 0 || row >= m_filtered.size())
        return m;
    const auto &e = m_filtered.at(row);
    m.insert(QStringLiteral("number"), e.number);
    m.insert(QStringLiteral("name"), e.name);
    return m;
}

void WaveformCatalog::setStatus(const QString &s)
{
    if (m_statusText == s)
        return;
    m_statusText = s;
    emit statusTextChanged();
}

void WaveformCatalog::setError(const QString &e)
{
    if (m_lastError == e)
        return;
    m_lastError = e;
    emit lastErrorChanged();
}

void WaveformCatalog::setBusy(bool v)
{
    if (m_busy == v)
        return;
    m_busy = v;
    emit busyChanged();
}
