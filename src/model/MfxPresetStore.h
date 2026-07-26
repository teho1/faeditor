#pragma once

#include <QAbstractListModel>
#include <QJsonObject>
#include <QVector>

/** Ready-made MFX presets loaded from resources/mfx_presets/presets.json. */
class MfxPresetStore : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        CategoryRole,
        TypeRole,
        PresetRole
    };

    explicit MfxPresetStore(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QJsonObject presetAt(int row) const;
    Q_INVOKABLE bool loadFromResource(const QString &url = QStringLiteral(":/qt/qml/FAEditor/resources/mfx_presets/presets.json"));

private:
    struct Preset {
        QString name;
        QString category;
        QJsonObject object;
    };
    QVector<Preset> m_presets;
};
