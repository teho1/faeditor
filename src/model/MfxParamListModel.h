#pragma once

#include <QAbstractListModel>

class MfxModel;

/** Catalog-driven MFX parameter rows for the Advanced / generic param grid. */
class MfxParamListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        MinRole,
        MaxRole,
        ParamValueRole,
        IndexRole,
        HasEnumRole,
        EnumNamesRole
    };

    explicit MfxParamListModel(MfxModel *mfx, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setValue(int row, int value);

signals:
    void countChanged();

private:
    void syncFromMfx();

    MfxModel *m_mfx = nullptr;
    int m_count = 0;
};
