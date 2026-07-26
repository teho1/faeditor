#include "project/ProjectStore.h"
#include "model/StudioSetModel.h"
#include "model/AudioFxModel.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDateTime>
#include <QUuid>
#include <QRegularExpression>
#include <QLocale>

ProjectStore::ProjectStore(StudioSetModel *studioSet, AudioFxModel *audioFx, QObject *parent)
    : QAbstractListModel(parent)
    , m_studioSet(studioSet)
    , m_audioFx(audioFx)
{
    QDir().mkpath(libraryDir());
    refresh();
}

QString ProjectStore::libraryDir() const
{
    const auto base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const auto dir = base + QStringLiteral("/library");
    QDir().mkpath(dir);
    return dir;
}

QString ProjectStore::autosavePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
           + QStringLiteral("/autosave.json");
}

QString ProjectStore::sanitizeName(const QString &name) const
{
    QString n = name.trimmed();
    if (n.isEmpty())
        n = QStringLiteral("Untitled");
    n.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_\\- ]")), QStringLiteral("_"));
    return n.left(64);
}

QJsonObject ProjectStore::buildLibraryRoot(const QString &name, bool refreshFromDevice)
{
    if (refreshFromDevice && m_studioSet) {
        m_studioSet->refreshLibraryBlobs();
        if (m_audioFx)
            m_audioFx->pullFromDevice();
    }

    QJsonObject root;
    root.insert(QStringLiteral("name"), name);
    root.insert(QStringLiteral("created"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    root.insert(QStringLiteral("modified"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    root.insert(QStringLiteral("studioSet"), m_studioSet ? m_studioSet->toJson() : QJsonObject());
    root.insert(QStringLiteral("audioFx"), m_audioFx ? m_audioFx->toJson() : QJsonObject());
    root.insert(QStringLiteral("sysexBlobs"), m_studioSet ? m_studioSet->sysexBlobsJson() : QJsonObject());

    QJsonObject systemBlobs;
    if (m_studioSet) {
        systemBlobs.insert(QStringLiteral("masterEq"),
                           QString::fromLatin1(m_studioSet->systemMasterEq().toBase64()));
    }
    root.insert(QStringLiteral("systemBlobs"), systemBlobs);
    return root;
}

bool ProjectStore::applyLibraryRoot(const QJsonObject &root)
{
    if (!m_studioSet || !m_audioFx)
        return false;

    const auto studio = root.value(QStringLiteral("studioSet")).toObject();
    const auto audioFx = root.value(QStringLiteral("audioFx")).toObject();
    const auto sysexBlobs = root.value(QStringLiteral("sysexBlobs")).toObject();
    const auto systemBlobs = root.value(QStringLiteral("systemBlobs")).toObject();
    const auto masterEqB64 = systemBlobs.value(QStringLiteral("masterEq")).toString();

    if (studio.isEmpty() || audioFx.isEmpty() || sysexBlobs.isEmpty() || masterEqB64.isEmpty())
        return false;

    if (!m_studioSet->fromJson(studio))
        return false;
    if (!m_audioFx->fromJson(audioFx))
        return false;
    m_studioSet->setSysexBlobsJson(sysexBlobs);
    m_studioSet->setSystemMasterEq(QByteArray::fromBase64(masterEqB64.toLatin1()));
    return true;
}

int ProjectStore::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_entries.size();
}

QVariant ProjectStore::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};
    const auto &e = m_entries.at(index.row());
    switch (role) {
    case NameRole:
        return e.name;
    case PathRole:
        return e.path;
    case ModifiedRole:
        return e.modified;
    case IdRole:
        return e.id;
    default:
        return {};
    }
}

QHash<int, QByteArray> ProjectStore::roleNames() const
{
    return {
        {NameRole, "name"},
        {PathRole, "path"},
        {ModifiedRole, "modified"},
        {IdRole, "id"}
    };
}

void ProjectStore::refresh()
{
    beginResetModel();
    m_entries.clear();
    QDir dir(libraryDir());
    const auto files = dir.entryInfoList({QStringLiteral("*.json")}, QDir::Files, QDir::Time);
    for (const auto &fi : files) {
        LibraryEntry e;
        e.path = fi.absoluteFilePath();
        e.id = fi.completeBaseName();
        e.modified = QLocale::system().toString(fi.lastModified(), QLocale::ShortFormat);
        QFile f(e.path);
        if (f.open(QIODevice::ReadOnly)) {
            const auto doc = QJsonDocument::fromJson(f.readAll());
            e.name = doc.object().value(QStringLiteral("name")).toString(e.id);
        } else {
            e.name = e.id;
        }
        m_entries.push_back(e);
    }
    endResetModel();
    emit libraryChanged();
}

bool ProjectStore::save(const QString &name)
{
    if (!m_currentPath.isEmpty()) {
        const QString displayName = name.isEmpty() ? m_currentName : name;
        const auto root = buildLibraryRoot(displayName, true);
        QFile f(m_currentPath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            return false;
        f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        m_studioSet->markClean();
        refresh();
        return true;
    }
    return saveAs(name.isEmpty() ? m_studioSet->name() : name);
}

bool ProjectStore::saveAs(const QString &name)
{
    const auto safe = sanitizeName(name);
    const auto path = libraryDir() + QLatin1Char('/') + safe + QStringLiteral(".json");
    m_currentPath = path;
    m_currentName = safe;
    emit currentPathChanged();
    return save(safe);
}

bool ProjectStore::load(int row)
{
    if (row < 0 || row >= m_entries.size())
        return false;
    return loadPath(m_entries.at(row).path);
}

bool ProjectStore::loadPath(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    const auto doc = QJsonDocument::fromJson(f.readAll());
    const auto root = doc.object();
    if (!applyLibraryRoot(root))
        return false;
    m_currentPath = path;
    m_currentName = root.value(QStringLiteral("name")).toString(QFileInfo(path).completeBaseName());
    emit currentPathChanged();
    m_studioSet->markClean();
    return true;
}

bool ProjectStore::duplicate(int row)
{
    if (row < 0 || row >= m_entries.size())
        return false;
    const auto src = m_entries.at(row);
    QString newName = sanitizeName(src.name + QStringLiteral(" Copy"));
    auto dest = libraryDir() + QLatin1Char('/') + newName + QStringLiteral(".json");
    int n = 2;
    while (QFile::exists(dest)) {
        newName = sanitizeName(src.name + QStringLiteral(" Copy %1").arg(n++));
        dest = libraryDir() + QLatin1Char('/') + newName + QStringLiteral(".json");
    }
    if (!QFile::copy(src.path, dest))
        return false;
    // Update embedded display name without touching the open editor project
    QFile f(dest);
    if (f.open(QIODevice::ReadWrite)) {
        auto doc = QJsonDocument::fromJson(f.readAll());
        auto root = doc.object();
        root.insert(QStringLiteral("name"), newName);
        root.insert(QStringLiteral("modified"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        f.resize(0);
        f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    }
    refresh();
    return true;
}

bool ProjectStore::duplicateCurrent()
{
    if (m_currentPath.isEmpty())
        return false;
    const QFileInfo cur(m_currentPath);
    for (int i = 0; i < m_entries.size(); ++i) {
        if (QFileInfo(m_entries.at(i).path) == cur)
            return duplicate(i);
    }
    if (!cur.exists())
        return false;

    QString baseName = m_currentName.isEmpty() ? cur.completeBaseName() : m_currentName;
    QString newName = sanitizeName(baseName + QStringLiteral(" Copy"));
    auto dest = libraryDir() + QLatin1Char('/') + newName + QStringLiteral(".json");
    int n = 2;
    while (QFile::exists(dest)) {
        newName = sanitizeName(baseName + QStringLiteral(" Copy %1").arg(n++));
        dest = libraryDir() + QLatin1Char('/') + newName + QStringLiteral(".json");
    }
    if (!QFile::copy(m_currentPath, dest))
        return false;
    QFile f(dest);
    if (f.open(QIODevice::ReadWrite)) {
        auto doc = QJsonDocument::fromJson(f.readAll());
        auto root = doc.object();
        root.insert(QStringLiteral("name"), newName);
        root.insert(QStringLiteral("modified"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        f.resize(0);
        f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    }
    refresh();
    return true;
}

bool ProjectStore::rename(int row, const QString &newName)
{
    if (row < 0 || row >= m_entries.size())
        return false;
    const auto e = m_entries.at(row);
    const auto safe = sanitizeName(newName);
    if (safe.isEmpty())
        return false;

    const auto dest = libraryDir() + QLatin1Char('/') + safe + QStringLiteral(".json");
    const bool samePath = QFileInfo(e.path) == QFileInfo(dest);
    if (!samePath) {
        if (QFile::exists(dest))
            return false; // name already taken
        if (!QFile::rename(e.path, dest))
            return false;
    }

    const QString path = samePath ? e.path : dest;
    QFile f(path);
    if (!f.open(QIODevice::ReadWrite))
        return false;
    auto doc = QJsonDocument::fromJson(f.readAll());
    auto root = doc.object();
    root.insert(QStringLiteral("name"), safe);
    root.insert(QStringLiteral("modified"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    f.resize(0);
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    f.close();

    if (m_currentPath == e.path || QFileInfo(m_currentPath) == QFileInfo(path)) {
        m_currentPath = path;
        m_currentName = safe;
        emit currentPathChanged();
        // Keep Temporary name on FA/editor in sync when renaming the open project
        if (m_studioSet)
            m_studioSet->setName(safe.left(16));
    }

    refresh();
    return true;
}

bool ProjectStore::remove(int row)
{
    if (row < 0 || row >= m_entries.size())
        return false;
    const auto path = m_entries.at(row).path;
    if (!QFile::remove(path))
        return false;
    if (m_currentPath == path) {
        m_currentPath.clear();
        m_currentName.clear();
        emit currentPathChanged();
    }
    refresh();
    return true;
}

bool ProjectStore::importFile(const QString &path)
{
    QFileInfo fi(path);
    const auto dest = libraryDir() + QLatin1Char('/') + sanitizeName(fi.completeBaseName()) + QStringLiteral(".json");
    if (!QFile::copy(path, dest)) {
        // overwrite
        QFile::remove(dest);
        if (!QFile::copy(path, dest))
            return false;
    }
    refresh();
    return loadPath(dest);
}

bool ProjectStore::exportFile(int row, const QString &destPath)
{
    if (row < 0 || row >= m_entries.size())
        return false;
    QFile::remove(destPath);
    return QFile::copy(m_entries.at(row).path, destPath);
}

void ProjectStore::autosave()
{
    if (!m_studioSet)
        return;
    auto root = buildLibraryRoot(m_studioSet->name(), false);
    root.insert(QStringLiteral("crashRecovery"), true);
    QFile f(autosavePath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

bool ProjectStore::recoverAutosaveIfNeeded()
{
    QFile f(autosavePath());
    if (!f.exists())
        return false;
    if (!f.open(QIODevice::ReadOnly))
        return false;
    const auto doc = QJsonDocument::fromJson(f.readAll());
    const auto root = doc.object();
    if (!root.value(QStringLiteral("crashRecovery")).toBool())
        return false;
    return applyLibraryRoot(root);
}
