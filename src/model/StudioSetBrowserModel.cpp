#include "model/StudioSetBrowserModel.h"
#include "midi/SysexEngine.h"
#include "midi/AddressMap.h"
#include "model/StudioSetModel.h"

#include <QThread>

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

} // namespace

StudioSetBrowserModel::StudioSetBrowserModel(SysexEngine *engine, StudioSetModel *studioSet,
                                             QObject *parent)
    : QAbstractListModel(parent)
    , m_engine(engine)
    , m_studioSet(studioSet)
{
    buildSlots();
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
    if (!m_engine || !m_engine->isOpen()) {
        if (error)
            *error = QStringLiteral("Not connected");
        return false;
    }

    QByteArray mode(1, char(1)); // Sound Mode = STUDIO
    if (!m_engine->writeParam(roland::Address{{0x01, 0x00, 0x00, 0x00}}, mode, error))
        return false;

    QByteArray select;
    select.append(char(slot.bankMsb & 0x7F));
    select.append(char(slot.bankLsb & 0x7F));
    select.append(char(slot.program & 0x7F));
    return m_engine->write(roland::Address{{0x01, 0x00, 0x00, 0x04}}, select, error);
}

bool StudioSetBrowserModel::readTemporaryName(QString *outName, QString *error)
{
    QByteArray data;
    if (!m_engine->read(roland::addr::kStudioSetCommon, 16, &data, error, 2500))
        return false;
    if (outName)
        *outName = QString::fromLatin1(data.constData(), qMin(16, data.size())).trimmed();
    return true;
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
    if (!m_engine || !m_engine->isOpen()) {
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
    setStatus(QStringLiteral("Scan cancelled (%1/%2)").arg(m_scanProgress).arg(m_scanTotal));
}

void StudioSetBrowserModel::scanStep()
{
    if (!m_scanning)
        return;
    if (m_scanIndex >= m_scanQueue.size()) {
        m_scanning = false;
        emit scanningChanged();
        setStatus(QStringLiteral("Scan complete — %1 names").arg(m_scanProgress));
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
