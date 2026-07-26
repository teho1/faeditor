#pragma once

#include <QAbstractListModel>
#include <QJsonObject>
#include <QStringList>

class StudioSetModel;
class AudioFxModel;

struct LibraryEntry {
    QString id;
    QString name;
    QString path;
    QString modified;
};

class ProjectStore : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentPathChanged)
    Q_PROPERTY(QString currentName READ currentName NOTIFY currentPathChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        PathRole,
        ModifiedRole,
        IdRole
    };

    explicit ProjectStore(StudioSetModel *studioSet, AudioFxModel *audioFx,
                          QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString currentPath() const { return m_currentPath; }
    QString currentName() const { return m_currentName; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool save(const QString &name = {});
    Q_INVOKABLE bool saveAs(const QString &name);
    Q_INVOKABLE bool load(int row);
    Q_INVOKABLE bool loadPath(const QString &path);
    Q_INVOKABLE bool duplicate(int row);
    /** Duplicate the currently open library file on disk (does not switch the editor to the copy). */
    Q_INVOKABLE bool duplicateCurrent();
    Q_INVOKABLE bool rename(int row, const QString &newName);
    Q_INVOKABLE bool remove(int row);
    Q_INVOKABLE bool importFile(const QString &path);
    Q_INVOKABLE bool exportFile(int row, const QString &destPath);
    Q_INVOKABLE void autosave();
    Q_INVOKABLE bool recoverAutosaveIfNeeded();

signals:
    void currentPathChanged();
    void libraryChanged();

private:
    QString libraryDir() const;
    QString autosavePath() const;
    QString sanitizeName(const QString &name) const;
    QJsonObject buildLibraryRoot(const QString &name, bool refreshFromDevice);
    bool applyLibraryRoot(const QJsonObject &root);

    StudioSetModel *m_studioSet = nullptr;
    AudioFxModel *m_audioFx = nullptr;
    QVector<LibraryEntry> m_entries;
    QString m_currentPath;
    QString m_currentName;
};
