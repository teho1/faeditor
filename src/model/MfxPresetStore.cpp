#include "model/MfxPresetStore.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

MfxPresetStore::MfxPresetStore(QObject *parent)
    : QAbstractListModel(parent)
{
    loadFromResource();
}

int MfxPresetStore::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_presets.size();
}

QVariant MfxPresetStore::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_presets.size())
        return {};
    const auto &p = m_presets.at(index.row());
    switch (role) {
    case NameRole:
        return p.name;
    case CategoryRole:
        return p.category;
    case TypeRole:
        return p.object.value(QStringLiteral("type")).toInt();
    case PresetRole:
        return p.object;
    default:
        return {};
    }
}

QHash<int, QByteArray> MfxPresetStore::roleNames() const
{
    return {
        {NameRole, "name"},
        {CategoryRole, "category"},
        {TypeRole, "mfxType"},
        {PresetRole, "preset"}
    };
}

QJsonObject MfxPresetStore::presetAt(int row) const
{
    if (row < 0 || row >= m_presets.size())
        return {};
    return m_presets.at(row).object;
}

bool MfxPresetStore::loadFromResource(const QString &url)
{
    QFile f(url);
    // qt_add_qml_module may prefix differently; try a few paths.
    if (!f.exists()) {
        const QStringList alts = {
            url,
            QStringLiteral(":/FAEditor/resources/mfx_presets/presets.json"),
            QStringLiteral(":/resources/mfx_presets/presets.json"),
            QStringLiteral(":/qt/qml/FAEditor/resources/mfx_presets/presets.json")
        };
        bool opened = false;
        for (const auto &path : alts) {
            f.setFileName(path);
            if (f.open(QIODevice::ReadOnly)) {
                opened = true;
                break;
            }
        }
        if (!opened)
            return false;
    } else if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }

    const auto doc = QJsonDocument::fromJson(f.readAll());
    const auto arr = doc.isArray() ? doc.array()
                                   : doc.object().value(QStringLiteral("presets")).toArray();
    beginResetModel();
    m_presets.clear();
    for (const auto &v : arr) {
        const auto o = v.toObject();
        Preset p;
        p.name = o.value(QStringLiteral("name")).toString();
        p.category = o.value(QStringLiteral("category")).toString();
        p.object = o;
        if (!p.name.isEmpty())
            m_presets.push_back(p);
    }
    endResetModel();
    return !m_presets.isEmpty();
}
