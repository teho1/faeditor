#include "model/MfxUiHelpers.h"

#include <algorithm>
#include <cmath>

MfxUiHelpers::MfxUiHelpers(QObject *parent)
    : QObject(parent)
{
}

MfxUiHelpers *MfxUiHelpers::instance()
{
    static MfxUiHelpers s;
    return &s;
}

QStringList MfxUiHelpers::delayNoteNames() const
{
    return {
        QStringLiteral("32nd"), QStringLiteral("16th"), QStringLiteral("8th T"),
        QStringLiteral("16th."), QStringLiteral("8th"), QStringLiteral("4th T"),
        QStringLiteral("8th."), QStringLiteral("4th"), QStringLiteral("2nd T"),
        QStringLiteral("4th."), QStringLiteral("2nd"), QStringLiteral("2nd."),
        QStringLiteral("1/1")
    };
}

QString MfxUiHelpers::familyForType(int typeId) const
{
    // Complete map for MFX types 0–68. Documented in resources/mfx_params/families.json
    switch (typeId) {
    case 0: // Thru
    case 43: // Pitch Shifter
    case 44: // 2Voice Pitch Shifter
    case 68: // Vocoder
        return familyGeneric();

    case 1: // Equalizer
    case 2: // Spectrum
    case 3: // Low Boost
        return familyEq();

    case 4: // Step Filter
    case 5: // Enhancer
    case 6: // Auto Wah
        return familyFilter();

    case 7: // Humanizer
    case 9: // Phaser 1
    case 10: // Phaser 2
    case 11: // Phaser 3
    case 12: // Step Phaser
    case 13: // Multi Stage Phaser
    case 14: // Infinite Phaser
    case 16: // Tremolo
    case 17: // Auto Pan
    case 22: // Chorus
    case 23: // Flanger
    case 24: // Step Flanger
    case 25: // Hexa-Chorus
    case 26: // Tremolo Chorus
    case 27: // Space-D
        return familyModulation();

    case 15: // Ring Modulator
    case 41: // LOFI Compress
    case 42: // Bit Crusher
    case 45: case 46: case 47: case 48: case 49: case 50:
    case 51: case 52: case 53: case 54: case 55: case 56:
    case 57: case 58: case 59: case 60: case 61: case 62:
    case 63: case 64: case 65: case 66: case 67:
        return familyPipeline();

    case 18: // Slicer
        return familySlicer();

    case 19: case 20: case 21: // Rotary 1–3
        return familyRotary();

    case 8: // Speaker Simulator
    case 28: // Overdrive
    case 29: // Distortion
    case 30: // Guitar Amp Simulator
        return familyAmp();

    case 31: // Compressor
    case 32: // Limiter
    case 33: // Gate
        return familyCompressor();

    case 36: // 3Tap Pan Delay — rich timeline
        return familyMultiTapDelay();

    case 34: // Delay
    case 35: // Modulation Delay
    case 37: // 4Tap Pan Delay
    case 38: // Multi Tap Delay
    case 39: // Reverse Delay
    case 40: // Time Ctrl Delay
        return familyDelay();

    default:
        return familyGeneric();
    }
}

bool MfxUiHelpers::usesVisualTemplate(int typeId) const
{
    const QString f = familyForType(typeId);
    return f != familyParamGrid() && f != familyGeneric();
}

bool MfxUiHelpers::isDelayNote(int v) const
{
    return v > delayMsMax();
}

int MfxUiHelpers::delayNoteIndex(int v) const
{
    const int n = delayNoteNames().size();
    return std::clamp(v - (delayMsMax() + 1), 0, n - 1);
}

int MfxUiHelpers::delayTimeForAxis(int v) const
{
    if (v <= delayMsMax())
        return std::max(0, v);
    return delayMsMax();
}

int MfxUiHelpers::clampDelayMs(int v) const
{
    return std::clamp(v, 0, delayMsMax());
}

QString MfxUiHelpers::formatDelayTime(int v) const
{
    if (isDelayNote(v)) {
        const auto names = delayNoteNames();
        return names.at(delayNoteIndex(v)) + QStringLiteral(" note");
    }
    return QString::number(std::max(0, v)) + QStringLiteral(" ms");
}

bool MfxUiHelpers::isHfDampBypass(int v) const
{
    return v < hfDampMinHz() || v > hfDampMaxHz();
}

int MfxUiHelpers::clampHfDampHz(int v) const
{
    return std::clamp(v, hfDampMinHz(), hfDampMaxHz());
}

qreal MfxUiHelpers::hfDampNorm(int v) const
{
    if (isHfDampBypass(v))
        return 1.0;
    const qreal lo = std::log(qreal(hfDampMinHz()));
    const qreal hi = std::log(qreal(hfDampMaxHz()));
    const qreal clamped = std::clamp(v, hfDampMinHz(), hfDampMaxHz());
    return (std::log(clamped) - lo) / (hi - lo);
}

int MfxUiHelpers::hfDampFromNorm(qreal n) const
{
    n = std::clamp(n, 0.0, 1.0);
    if (n > 0.97)
        return hfDampBypass();
    const qreal lo = std::log(qreal(hfDampMinHz()));
    const qreal hi = std::log(qreal(hfDampMaxHz()));
    const qreal hz = std::exp(lo + n * (hi - lo));
    return clampHfDampHz(int(std::lround(hz)));
}

qreal MfxUiHelpers::hfDampCurveMagnitude(int hfDamp, qreal tNorm) const
{
    tNorm = std::clamp(tNorm, 0.0, 1.0);
    if (isHfDampBypass(hfDamp))
        return 0.9;
    const qreal cutN = hfDampNorm(hfDamp);
    if (tNorm <= cutN)
        return 0.9;
    const qreal dn = (tNorm - cutN) / std::max(0.05, 1.0 - cutN);
    return 0.9 / (1.0 + std::pow(dn * 4.5, 2.2));
}

QString MfxUiHelpers::formatHfDamp(int v) const
{
    if (isHfDampBypass(v))
        return QStringLiteral("BYPASS");
    return QString::number(v) + QStringLiteral(" Hz");
}

int MfxUiHelpers::clampFeedback(int v) const
{
    return std::clamp(v, -98, 98);
}

QString MfxUiHelpers::formatFeedback(int v) const
{
    const int n = clampFeedback(v);
    return (n > 0 ? QStringLiteral("+") : QString()) + QString::number(n) + QStringLiteral(" %");
}

QString MfxUiHelpers::formatBalance(int v) const
{
    const int w = clampBalance(v);
    return QStringLiteral("D%1:%2W").arg(100 - w).arg(w);
}

QString MfxUiHelpers::formatDb(int v) const
{
    const int n = clampGainDb(v);
    return (n > 0 ? QStringLiteral("+") : QString()) + QString::number(n) + QStringLiteral(" dB");
}

QString MfxUiHelpers::formatLevel(int v) const
{
    return QString::number(clampLevel(v));
}

int MfxUiHelpers::clampLevel(int v) const
{
    return std::clamp(v, 0, 127);
}

int MfxUiHelpers::clampGainDb(int v) const
{
    return std::clamp(v, -15, 15);
}

int MfxUiHelpers::clampBalance(int v) const
{
    return std::clamp(v, 0, 100);
}

int MfxUiHelpers::sanitizeDelayTime(int v) const
{
    if (v < 0)
        return 0;
    if (v > 2613)
        return 0;
    return v;
}

int MfxUiHelpers::sanitizeLevel(int v) const
{
    if (v < 0 || v > 127)
        return 0;
    return v;
}

int MfxUiHelpers::sanitizeFeedback(int v) const
{
    if (v < -98 || v > 98)
        return 0;
    return v;
}

int MfxUiHelpers::sanitizeHfDamp(int v) const
{
    if (v >= hfDampMinHz() && v <= hfDampBypass())
        return v;
    return hfDampBypass();
}

int MfxUiHelpers::sanitizeGainDb(int v) const
{
    if (v < -15 || v > 15)
        return 0;
    return v;
}

int MfxUiHelpers::sanitizeBalance(int v) const
{
    if (v < 0 || v > 100)
        return 50;
    return v;
}
