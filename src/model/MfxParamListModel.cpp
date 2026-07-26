#include "model/MfxParamListModel.h"
#include "model/MfxModel.h"

#include <algorithm>

MfxParamListModel::MfxParamListModel(MfxModel *mfx, QObject *parent)
    : QAbstractListModel(parent)
    , m_mfx(mfx)
{
    if (m_mfx) {
        connect(m_mfx, &MfxModel::mfxChanged, this, &MfxParamListModel::syncFromMfx);
        m_count = m_mfx->paramCount();
    }
}

int MfxParamListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_count;
}

QVariant MfxParamListModel::data(const QModelIndex &index, int role) const
{
    if (!m_mfx || !index.isValid() || index.row() < 0 || index.row() >= m_count)
        return {};
    const int i = index.row();
    switch (role) {
    case NameRole:
        return m_mfx->paramName(i);
    case MinRole:
        return m_mfx->paramMin(i);
    case MaxRole:
        return m_mfx->paramMax(i);
    case ParamValueRole: {
        const int v = m_mfx->paramValue(i);
        const int lo = m_mfx->paramMin(i);
        const int hi = m_mfx->paramMax(i);
        // Uninitialized nibble decode is −32768; clamping to min (−15 dB etc.)
        // makes SpinBoxes look “stuck”. Treat out-of-range as a neutral default.
        if (v < lo || v > hi)
            return (lo <= 0 && hi >= 0) ? 0 : lo;
        return v;
    }
    case IndexRole:
        return i;
    case HasEnumRole:
        return m_mfx->paramHasEnum(i);
    case EnumNamesRole:
        return m_mfx->paramEnumNames(i);
    default:
        return {};
    }
}

QHash<int, QByteArray> MfxParamListModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {MinRole, "min"},
        {MaxRole, "max"},
        {ParamValueRole, "paramValue"},
        {IndexRole, "paramIndex"},
        {HasEnumRole, "hasEnum"},
        {EnumNamesRole, "enumNames"}
    };
}

void MfxParamListModel::setValue(int row, int value)
{
    if (!m_mfx || row < 0 || row >= m_count)
        return;
    m_mfx->setParamValue(row, value);
}

void MfxParamListModel::syncFromMfx()
{
    if (!m_mfx)
        return;
    const int n = m_mfx->paramCount();
    if (n != m_count) {
        beginResetModel();
        m_count = n;
        endResetModel();
        emit countChanged();
        return;
    }
    if (m_count <= 0)
        return;
    emit dataChanged(index(0), index(m_count - 1),
                     {ParamValueRole, MinRole, MaxRole, NameRole, HasEnumRole, EnumNamesRole});
}
