#include "model/MfxParamCatalog.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>

#include <algorithm>

namespace MfxParamCatalog {
namespace {

QMutex g_mutex;
bool g_loaded = false;
QVector<QVector<ParamSlot>> g_byType; // index = type id

bool openCatalogFile(QFile &f)
{
    const QStringList alts = {
        QStringLiteral(":/qt/qml/FAEditor/resources/mfx_params/catalog.json"),
        QStringLiteral(":/FAEditor/resources/mfx_params/catalog.json"),
        QStringLiteral(":/resources/mfx_params/catalog.json"),
    };
    for (const auto &path : alts) {
        f.setFileName(path);
        if (f.open(QIODevice::ReadOnly))
            return true;
    }
    return false;
}

QStringList enumFromJson(const QJsonObject &o)
{
    QStringList out;
    const QJsonValue ev = o.contains(QStringLiteral("enum"))
                              ? o.value(QStringLiteral("enum"))
                              : o.value(QStringLiteral("values"));
    if (!ev.isArray())
        return out;
    const QJsonArray arr = ev.toArray();
    out.reserve(arr.size());
    for (const auto &v : arr)
        out.append(v.toString());
    return out;
}

ParamSlot slotFromJson(const QJsonObject &o, int fallbackIndex)
{
    ParamSlot s;
    s.name = o.value(QStringLiteral("name")).toString();
    if (s.name.isEmpty())
        s.name = QStringLiteral("Parameter %1").arg(fallbackIndex + 1);
    if (o.contains(QStringLiteral("min")) && o.contains(QStringLiteral("max"))) {
        s.minValue = o.value(QStringLiteral("min")).toInt(DefaultMin);
        s.maxValue = o.value(QStringLiteral("max")).toInt(DefaultMax);
        if (s.minValue > s.maxValue)
            std::swap(s.minValue, s.maxValue);
        s.hasRange = true;
    } else {
        s.minValue = DefaultMin;
        s.maxValue = DefaultMax;
        s.hasRange = false;
    }
    s.enumNames = enumFromJson(o);
    if (!s.enumNames.isEmpty() && s.hasRange) {
        // Keep enum length consistent with inclusive [min,max] when possible.
        const int span = s.maxValue - s.minValue + 1;
        if (span > 0 && s.enumNames.size() > span)
            s.enumNames = s.enumNames.mid(0, span);
    }
    return s;
}

void loadLocked()
{
    g_byType.clear();
    g_byType.resize(TypeCount);

    QFile f;
    if (!openCatalogFile(f)) {
        g_loaded = true;
        return;
    }

    const auto doc = QJsonDocument::fromJson(f.readAll());
    const QJsonObject root = doc.isObject() ? doc.object() : QJsonObject();
    const QJsonObject types = root.value(QStringLiteral("types")).toObject();

    for (auto it = types.begin(); it != types.end(); ++it) {
        bool ok = false;
        const int typeId = it.key().toInt(&ok);
        if (!ok || typeId < 0 || typeId >= TypeCount)
            continue;
        const QJsonObject typeObj = it.value().toObject();
        const QJsonArray arr = typeObj.value(QStringLiteral("params")).toArray();
        QVector<ParamSlot> entries;
        entries.reserve(std::min<int>(int(arr.size()), MaxParams));
        for (int i = 0; i < arr.size() && i < MaxParams; ++i)
            entries.push_back(slotFromJson(arr.at(i).toObject(), i));
        g_byType[typeId] = entries;
    }
    g_loaded = true;
}

} // namespace

bool ensureLoaded()
{
    QMutexLocker lock(&g_mutex);
    if (!g_loaded)
        loadLocked();
    return g_loaded;
}

QVector<ParamSlot> paramsForType(int typeId)
{
    ensureLoaded();
    QMutexLocker lock(&g_mutex);
    if (typeId < 0 || typeId >= g_byType.size())
        return {};
    return g_byType.at(typeId);
}

int paramCountForType(int typeId)
{
    return paramsForType(typeId).size();
}

QString paramName(int typeId, int index)
{
    const auto entries = paramsForType(typeId);
    if (index >= 0 && index < entries.size())
        return entries.at(index).name;
    if (index >= 0 && index < MaxParams)
        return QStringLiteral("Parameter %1").arg(index + 1);
    return {};
}

int paramMin(int typeId, int index)
{
    const auto entries = paramsForType(typeId);
    if (index >= 0 && index < entries.size())
        return entries.at(index).minValue;
    return DefaultMin;
}

int paramMax(int typeId, int index)
{
    const auto entries = paramsForType(typeId);
    if (index >= 0 && index < entries.size())
        return entries.at(index).maxValue;
    return DefaultMax;
}

QStringList paramEnumNames(int typeId, int index)
{
    const auto entries = paramsForType(typeId);
    if (index >= 0 && index < entries.size())
        return entries.at(index).enumNames;
    return {};
}

bool paramHasEnum(int typeId, int index)
{
    return !paramEnumNames(typeId, index).isEmpty();
}

} // namespace MfxParamCatalog
