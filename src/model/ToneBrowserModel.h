#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QSet>
#include <QJsonArray>

struct ToneEntry {
    QString name;
    QString category;
    int bankMsb = 0;
    int bankLsb = 0;
    int program = 0;
};

class ToneBrowserModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterChanged)
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY filterChanged)
    Q_PROPERTY(QStringList categories READ categories NOTIFY catalogLoaded)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        CategoryRole,
        BankMsbRole,
        BankLsbRole,
        ProgramRole,
        FavoriteRole
    };

    explicit ToneBrowserModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString filterText() const { return m_filterText; }
    void setFilterText(const QString &t);
    QString category() const { return m_category; }
    void setCategory(const QString &c);
    QStringList categories() const { return m_categories; }

    Q_INVOKABLE bool loadCatalog(const QString &path = {});
    Q_INVOKABLE void toggleFavorite(int row);
    Q_INVOKABLE bool isFavorite(int bankMsb, int bankLsb, int program) const;
    Q_INVOKABLE void loadFavorites();
    Q_INVOKABLE void saveFavorites();
    Q_INVOKABLE QString resolveName(int bankMsb, int bankLsb, int program) const;
    Q_INVOKABLE QString resolveCategory(int bankMsb, int bankLsb, int program) const;
    Q_INVOKABLE QVariantMap toneAt(int row) const;

signals:
    void filterChanged();
    void catalogLoaded();

private:
    void rebuildFiltered();
    QString favoriteKey(int msb, int lsb, int pc) const;
    QString favoritesPath() const;

    QVector<ToneEntry> m_all;
    QVector<int> m_filtered; // indices into m_all
    QStringList m_categories;
    QString m_filterText;
    QString m_category = QStringLiteral("All");
    QSet<QString> m_favorites;
};
