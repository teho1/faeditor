#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

/** Parameter Guide MFX type → named param slots (resources/mfx_params/catalog.json). */
namespace MfxParamCatalog {

inline constexpr int MaxParams = 32;
inline constexpr int TypeCount = 69; // 0 Thru … 68 Vocoder
inline constexpr int DefaultMin = -20000;
inline constexpr int DefaultMax = 20000;

struct ParamSlot {
    QString name;
    int minValue = DefaultMin;
    int maxValue = DefaultMax;
    bool hasRange = false;
    QStringList enumNames; // non-empty → ComboBox in QML
};

/** Load catalog once from Qt resources (idempotent). */
bool ensureLoaded();

/** Named slots for MFX type 0–68. Empty for Thru / unknown. */
QVector<ParamSlot> paramsForType(int typeId);

int paramCountForType(int typeId);
QString paramName(int typeId, int index);
int paramMin(int typeId, int index);
int paramMax(int typeId, int index);
QStringList paramEnumNames(int typeId, int index);
bool paramHasEnum(int typeId, int index);

} // namespace MfxParamCatalog
