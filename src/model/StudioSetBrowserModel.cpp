#include "model/StudioSetBrowserModel.h"
#include "platform/InstrumentPlatform.h"
#include "midi/AddressMap.h"
#include "model/StudioSetModel.h"

#include <QThread>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDateTime>

namespace {

StudioSetSlot makeUserSlot(int number1Based)
{
    StudioSetSlot s;
    s.kind = StudioSetSlot::Kind::User;
    s.number = number1Based;
    const int zero = number1Based - 1;
    s.bankMsb = 85;
    s.bankLsb = zero / 128;
    s.program = zero % 128;
    return s;
}

StudioSetSlot makePresetSlot(int number1Based)
{
    StudioSetSlot s;
    s.kind = StudioSetSlot::Kind::Preset;
    s.number = number1Based;
    s.bankMsb = 85;
    s.bankLsb = 64;
    s.program = number1Based - 1;
    return s;
}

QString defaultLabel(const StudioSetSlot &s)
{
    if (s.kind == StudioSetSlot::Kind::User)
        return QStringLiteral("User %1").arg(s.number, 3, 10, QChar('0'));
    return QStringLiteral("Preset %1").arg(s.number, 2, 10, QChar('0'));
}

QString slotKey(const StudioSetSlot &s)
{
    return (s.kind == StudioSetSlot::Kind::User ? QStringLiteral("U") : QStringLiteral("P"))
           + QString::number(s.number);
}

} // namespace

StudioSetBrowserModel::StudioSetBrowserModel(InstrumentPlatform *platform, StudioSetModel *studioSet,
                                             QObject *parent)
    : QAbstractListModel(parent)
    , m_platform(platform)
    , m_studioSet(studioSet)
{
    buildSlots();
    loadCachedNames();
    rebuildFiltered();
    m_scanTimer.setSingleShot(true);
    connect(&m_scanTimer, &QTimer::timeout, this, &StudioSetBrowserModel::scanStep);
}

void StudioSetBrowserModel::buildSlots()
{
    m_all.clear();
    m_all.reserve(512 + 69);
    for (int i = 1; i <= 512; ++i)
        m_all.push_back(makeUserSlot(i));
    for (int i = 1; i <= 69; ++i)
        m_all.push_back(makePresetSlot(i));
}

int StudioSetBrowserModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_filtered.size();
}

QVariant StudioSetBrowserModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filtered.size())
        return {};
    const auto &s = m_all.at(m_filtered.at(index.row()));
    switch (role) {
    case LabelRole:
        return defaultLabel(s);
    case NameRole:
        return s.name.isEmpty() ? QStringLiteral("—") : s.name;
    case KindRole:
        return s.kind == StudioSetSlot::Kind::User ? QStringLiteral("User")
                                                   : QStringLiteral("Preset");
    case NumberRole:
        return s.number;
    case SlotIdRole:
        return defaultLabel(s);
    case HasNameRole:
        return !s.name.isEmpty();
    default:
        return {};
    }
}

QHash<int, QByteArray> StudioSetBrowserModel::roleNames() const
{
    return {
        {LabelRole, "label"},
        {NameRole, "name"},
        {KindRole, "kind"},
        {NumberRole, "number"},
        {SlotIdRole, "slotId"},
        {HasNameRole, "hasName"}
    };
}

void StudioSetBrowserModel::setFilterText(const QString &t)
{
    if (m_filterText == t)
        return;
    m_filterText = t;
    rebuildFiltered();
    emit filterChanged();
}

void StudioSetBrowserModel::setGroupFilter(const QString &g)
{
    if (m_groupFilter == g)
        return;
    m_groupFilter = g;
    rebuildFiltered();
    emit filterChanged();
}

void StudioSetBrowserModel::rebuildFiltered()
{
    const int previousAbs = (m_currentRow >= 0 && m_currentRow < m_filtered.size())
                                ? m_filtered.at(m_currentRow)
                                : -1;
    beginResetModel();
    m_filtered.clear();
    const auto filter = m_filterText.trimmed();
    for (int i = 0; i < m_all.size(); ++i) {
        const auto &s = m_all.at(i);
        if (m_groupFilter == QLatin1String("User") && s.kind != StudioSetSlot::Kind::User)
            continue;
        if (m_groupFilter == QLatin1String("Preset") && s.kind != StudioSetSlot::Kind::Preset)
            continue;
        if (!filter.isEmpty()) {
            const auto label = defaultLabel(s);
            if (!label.contains(filter, Qt::CaseInsensitive)
                && !s.name.contains(filter, Qt::CaseInsensitive))
                continue;
        }
        m_filtered.push_back(i);
    }
    endResetModel();

    int newRow = -1;
    if (previousAbs >= 0) {
        for (int r = 0; r < m_filtered.size(); ++r) {
            if (m_filtered.at(r) == previousAbs) {
                newRow = r;
                break;
            }
        }
    }
    m_currentRow = newRow;
    emit currentChanged();
}

void StudioSetBrowserModel::setCurrentRow(int row)
{
    if (row < -1 || row >= m_filtered.size())
        return;
    if (m_currentRow == row)
        return;
    m_currentRow = row;
    emit currentChanged();
}

QString StudioSetBrowserModel::currentLabel() const
{
    if (m_currentRow < 0 || m_currentRow >= m_filtered.size())
        return QStringLiteral("None selected");
    const auto &s = m_all.at(m_filtered.at(m_currentRow));
    if (!s.name.isEmpty())
        return defaultLabel(s) + QStringLiteral(" — ") + s.name;
    return defaultLabel(s);
}

void StudioSetBrowserModel::setStatus(const QString &s)
{
    if (m_statusText == s)
        return;
    m_statusText = s;
    emit statusTextChanged();
}

int StudioSetBrowserModel::absoluteIndexFromFiltered(int row) const
{
    if (row < 0 || row >= m_filtered.size())
        return -1;
    return m_filtered.at(row);
}

QVariantMap StudioSetBrowserModel::slotAt(int row) const
{
    QVariantMap m;
    const int abs = absoluteIndexFromFiltered(row);
    if (abs < 0)
        return m;
    const auto &s = m_all.at(abs);
    m.insert(QStringLiteral("label"), defaultLabel(s));
    m.insert(QStringLiteral("name"), s.name);
    m.insert(QStringLiteral("kind"),
             s.kind == StudioSetSlot::Kind::User ? QStringLiteral("User")
                                                 : QStringLiteral("Preset"));
    m.insert(QStringLiteral("number"), s.number);
    m.insert(QStringLiteral("bankMsb"), s.bankMsb);
    m.insert(QStringLiteral("bankLsb"), s.bankLsb);
    m.insert(QStringLiteral("program"), s.program);
    return m;
}

bool StudioSetBrowserModel::writeSetupSelect(const StudioSetSlot &slot, QString *error)
{
    if (!m_platform || !m_platform->isConnected()) {
        if (error)
            *error = QStringLiteral("Not connected");
        return false;
    }

    return m_platform->recallStudioSet(slot.bankMsb, slot.bankLsb, slot.program, error);
}

bool StudioSetBrowserModel::readTemporaryName(QString *outName, QString *error)
{
    return m_platform && m_platform->readTemporaryStudioSetName(outName, error);
}

bool StudioSetBrowserModel::recallRow(int row, bool pullAfter)
{
    const int abs = absoluteIndexFromFiltered(row);
    if (abs < 0) {
        setStatus(QStringLiteral("Select a Studio Set first"));
        return false;
    }

    auto &slot = m_all[abs];
    QString err;
    setStatus(QStringLiteral("Loading %1…").arg(defaultLabel(slot)));
    if (!writeSetupSelect(slot, &err)) {
        setStatus(err);
        return false;
    }

    QThread::msleep(250);

    QString name;
    if (readTemporaryName(&name, &err)) {
        slot.name = name;
        const auto idx = index(row);
        emit dataChanged(idx, idx, {NameRole, HasNameRole});
        emit currentChanged();
        saveCachedNames();
    }

    if (pullAfter && m_studioSet) {
        if (!m_studioSet->pullFromDevice()) {
            setStatus(m_studioSet->lastError().isEmpty()
                          ? QStringLiteral("Loaded set but full pull failed")
                          : m_studioSet->lastError());
            emit recalled(row);
            return false;
        }
    }

    m_currentRow = row;
    emit currentChanged();
    setStatus(QStringLiteral("Loaded %1%2")
                  .arg(defaultLabel(slot),
                       slot.name.isEmpty() ? QString() : (QStringLiteral(" — ") + slot.name)));
    emit recalled(row);
    return true;
}

bool StudioSetBrowserModel::recallCurrent(bool pullAfter)
{
    return recallRow(m_currentRow, pullAfter);
}

void StudioSetBrowserModel::startScanNames(bool userOnly)
{
    if (m_scanning)
        return;
    if (!m_platform || !m_platform->isConnected()) {
        setStatus(QStringLiteral("Connect to the FA first"));
        return;
    }

    m_scanQueue.clear();
    for (int i = 0; i < m_all.size(); ++i) {
        if (userOnly && m_all.at(i).kind != StudioSetSlot::Kind::User)
            continue;
        m_scanQueue.push_back(i);
    }

    m_scanning = true;
    m_scanProgress = 0;
    m_scanTotal = m_scanQueue.size();
    m_scanIndex = 0;
    emit scanningChanged();
    setStatus(QStringLiteral("Scanning Studio Set names… 0/%1").arg(m_scanTotal));
    m_scanTimer.start(10);
}

void StudioSetBrowserModel::cancelScan()
{
    if (!m_scanning)
        return;
    m_scanTimer.stop();
    m_scanning = false;
    emit scanningChanged();
    saveCachedNames(); // keep whatever was read so far
    setStatus(QStringLiteral("Scan cancelled (%1/%2) — names saved").arg(m_scanProgress).arg(m_scanTotal));
}

void StudioSetBrowserModel::scanStep()
{
    if (!m_scanning)
        return;
    if (m_scanIndex >= m_scanQueue.size()) {
        m_scanning = false;
        emit scanningChanged();
        saveCachedNames();
        setStatus(QStringLiteral("Scan complete — %1 names (saved)").arg(m_scanProgress));
        rebuildFiltered();
        return;
    }

    const int abs = m_scanQueue.at(m_scanIndex);
    auto &slot = m_all[abs];
    QString err;
    if (writeSetupSelect(slot, &err)) {
        QThread::msleep(180);
        QString name;
        if (readTemporaryName(&name, &err))
            slot.name = name;
    }

    ++m_scanIndex;
    ++m_scanProgress;
    emit scanningChanged();
    if ((m_scanProgress % 5) == 0 || m_scanProgress == m_scanTotal) {
        setStatus(QStringLiteral("Scanning… %1/%2").arg(m_scanProgress).arg(m_scanTotal));
        if (!m_filtered.isEmpty())
            emit dataChanged(index(0), index(m_filtered.size() - 1), {NameRole, HasNameRole});
    }

    m_scanTimer.start(20);
}

bool StudioSetBrowserModel::hasCachedNames() const
{
    for (const auto &s : m_all) {
        if (!s.name.isEmpty())
            return true;
    }
    return false;
}

bool StudioSetBrowserModel::syncCurrentNameFromDevice()
{
    if (m_scanning)
        return false;
    if (!m_platform || !m_platform->isConnected())
        return false;
    const int abs = absoluteIndexFromFiltered(m_currentRow);
    if (abs < 0)
        return false;

    QString name;
    QString err;
    if (!readTemporaryName(&name, &err))
        return false;

    auto &slot = m_all[abs];
    if (slot.name == name) {
        // Still refresh editor Temporary name if the model drifted.
        if (m_studioSet && m_studioSet->name() != name.left(16))
            m_studioSet->setName(name.left(16));
        return true;
    }

    slot.name = name;
    const auto idx = index(m_currentRow);
    emit dataChanged(idx, idx, {NameRole, HasNameRole});
    emit currentChanged();
    if (m_studioSet)
        m_studioSet->setName(name.left(16));
    saveCachedNames();
    setStatus(QStringLiteral("Updated %1 — %2").arg(defaultLabel(slot), name));
    return true;
}

QString StudioSetBrowserModel::namesCachePath() const
{
    const auto dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/studio_set_names.json");
}

void StudioSetBrowserModel::loadCachedNames()
{
    QFile f(namesCachePath());
    if (!f.open(QIODevice::ReadOnly))
        return;
    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject())
        return;
    const auto arr = doc.object().value(QStringLiteral("slots")).toArray();
    QHash<QString, QString> byKey;
    byKey.reserve(arr.size());
    for (const auto &v : arr) {
        const auto o = v.toObject();
        const auto kind = o.value(QStringLiteral("kind")).toString();
        const int number = o.value(QStringLiteral("number")).toInt();
        const auto name = o.value(QStringLiteral("name")).toString().trimmed();
        if (name.isEmpty() || number < 1)
            continue;
        const auto key = (kind == QLatin1String("Preset") ? QStringLiteral("P") : QStringLiteral("U"))
                         + QString::number(number);
        byKey.insert(key, name);
    }
    if (byKey.isEmpty())
        return;
    for (auto &s : m_all) {
        const auto it = byKey.constFind(slotKey(s));
        if (it != byKey.cend())
            s.name = it.value();
    }
    setStatus(QStringLiteral("Loaded %1 cached Studio Set names").arg(byKey.size()));
}

void StudioSetBrowserModel::saveCachedNames() const
{
    QJsonArray arr;
    for (const auto &s : m_all) {
        if (s.name.isEmpty())
            continue;
        QJsonObject o;
        o.insert(QStringLiteral("kind"),
                 s.kind == StudioSetSlot::Kind::User ? QStringLiteral("User")
                                                     : QStringLiteral("Preset"));
        o.insert(QStringLiteral("number"), s.number);
        o.insert(QStringLiteral("name"), s.name);
        arr.append(o);
    }
    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("savedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    root.insert(QStringLiteral("slots"), arr);
    QFile f(namesCachePath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
}
