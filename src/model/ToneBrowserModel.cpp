#include "model/ToneBrowserModel.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>

ToneBrowserModel::ToneBrowserModel(QObject *parent)
    : QAbstractListModel(parent)
{
    loadFavorites();
}

int ToneBrowserModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_filtered.size();
}

QVariant ToneBrowserModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filtered.size())
        return {};
    const auto &t = m_all.at(m_filtered.at(index.row()));
    switch (role) {
    case NameRole:
        return t.name;
    case CategoryRole:
        return t.category;
    case BankMsbRole:
        return t.bankMsb;
    case BankLsbRole:
        return t.bankLsb;
    case ProgramRole:
        return t.program;
    case FavoriteRole:
        return m_favorites.contains(favoriteKey(t.bankMsb, t.bankLsb, t.program));
    default:
        return {};
    }
}

QHash<int, QByteArray> ToneBrowserModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {CategoryRole, "category"},
        {BankMsbRole, "bankMsb"},
        {BankLsbRole, "bankLsb"},
        {ProgramRole, "program"},
        {FavoriteRole, "favorite"}
    };
}

void ToneBrowserModel::setFilterText(const QString &t)
{
    if (m_filterText == t)
        return;
    m_filterText = t;
    rebuildFiltered();
    emit filterChanged();
}

void ToneBrowserModel::setCategory(const QString &c)
{
    if (m_category == c)
        return;
    m_category = c;
    rebuildFiltered();
    emit filterChanged();
}

QString ToneBrowserModel::favoriteKey(int msb, int lsb, int pc) const
{
    return QStringLiteral("%1-%2-%3").arg(msb).arg(lsb).arg(pc);
}

QString ToneBrowserModel::favoritesPath() const
{
    const auto dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/favorites.json");
}

bool ToneBrowserModel::loadCatalog(const QString &path)
{
    QString filePath = path;
    if (filePath.isEmpty())
        filePath = QStringLiteral(":/qt/qml/FAEditor/resources/tones/soundlist.json");

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        // Fallback relative
        f.setFileName(QStringLiteral("resources/tones/soundlist.json"));
        if (!f.open(QIODevice::ReadOnly))
            return false;
    }
    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isArray() && !doc.isObject())
        return false;

    beginResetModel();
    m_all.clear();
    QSet<QString> cats;
    cats.insert(QStringLiteral("All"));
    cats.insert(QStringLiteral("Favorites"));

    QJsonArray arr;
    if (doc.isArray())
        arr = doc.array();
    else
        arr = doc.object().value(QStringLiteral("tones")).toArray();

    for (const auto &v : arr) {
        const auto o = v.toObject();
        ToneEntry e;
        e.name = o.value(QStringLiteral("name")).toString();
        e.category = o.value(QStringLiteral("category")).toString(QStringLiteral("Synth"));
        e.bankMsb = o.value(QStringLiteral("bankMsb")).toInt();
        e.bankLsb = o.value(QStringLiteral("bankLsb")).toInt();
        e.program = o.value(QStringLiteral("program")).toInt();
        m_all.push_back(e);
        cats.insert(e.category);
    }
    m_categories = cats.values();
    m_categories.sort();
    m_categories.removeAll(QStringLiteral("All"));
    m_categories.removeAll(QStringLiteral("Favorites"));
    m_categories.prepend(QStringLiteral("Favorites"));
    m_categories.prepend(QStringLiteral("All"));

    m_filtered.clear();
    for (int i = 0; i < m_all.size(); ++i)
        m_filtered.push_back(i);

    endResetModel();
    emit catalogLoaded();
    return true;
}

void ToneBrowserModel::rebuildFiltered()
{
    beginResetModel();
    m_filtered.clear();
    const auto filter = m_filterText.trimmed();
    for (int i = 0; i < m_all.size(); ++i) {
        const auto &t = m_all.at(i);
        if (m_category == QLatin1String("Favorites")) {
            if (!m_favorites.contains(favoriteKey(t.bankMsb, t.bankLsb, t.program)))
                continue;
        } else if (m_category != QLatin1String("All") && t.category != m_category) {
            continue;
        }
        if (!filter.isEmpty() && !t.name.contains(filter, Qt::CaseInsensitive))
            continue;
        m_filtered.push_back(i);
    }
    endResetModel();
}

void ToneBrowserModel::toggleFavorite(int row)
{
    if (row < 0 || row >= m_filtered.size())
        return;
    const auto &t = m_all.at(m_filtered.at(row));
    const auto key = favoriteKey(t.bankMsb, t.bankLsb, t.program);
    if (m_favorites.contains(key))
        m_favorites.remove(key);
    else
        m_favorites.insert(key);
    saveFavorites();
    // Favorites filter membership changed — rebuild that view
    if (m_category == QLatin1String("Favorites")) {
        rebuildFiltered();
        return;
    }
    const auto idx = index(row);
    emit dataChanged(idx, idx, {FavoriteRole});
}

bool ToneBrowserModel::isFavorite(int bankMsb, int bankLsb, int program) const
{
    return m_favorites.contains(favoriteKey(bankMsb, bankLsb, program));
}

void ToneBrowserModel::loadFavorites()
{
    QFile f(favoritesPath());
    if (!f.open(QIODevice::ReadOnly))
        return;
    const auto doc = QJsonDocument::fromJson(f.readAll());
    m_favorites.clear();
    for (const auto &v : doc.array())
        m_favorites.insert(v.toString());
}

void ToneBrowserModel::saveFavorites()
{
    QJsonArray arr;
    for (const auto &k : m_favorites)
        arr.append(k);
    QFile f(favoritesPath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(QJsonDocument(arr).toJson());
}

QString ToneBrowserModel::resolveName(int bankMsb, int bankLsb, int program) const
{
    for (const auto &t : m_all) {
        if (t.bankMsb == bankMsb && t.bankLsb == bankLsb && t.program == program)
            return t.name;
    }
    return QStringLiteral("%1:%2:%3").arg(bankMsb).arg(bankLsb).arg(program + 1);
}

QString ToneBrowserModel::resolveCategory(int bankMsb, int bankLsb, int program) const
{
    for (const auto &t : m_all) {
        if (t.bankMsb == bankMsb && t.bankLsb == bankLsb && t.program == program)
            return t.category;
    }
    return {};
}

QVariantMap ToneBrowserModel::toneAt(int row) const
{
    QVariantMap m;
    if (row < 0 || row >= m_filtered.size())
        return m;
    const auto &t = m_all.at(m_filtered.at(row));
    m.insert(QStringLiteral("name"), t.name);
    m.insert(QStringLiteral("category"), t.category);
    m.insert(QStringLiteral("bankMsb"), t.bankMsb);
    m.insert(QStringLiteral("bankLsb"), t.bankLsb);
    m.insert(QStringLiteral("program"), t.program);
    return m;
}
