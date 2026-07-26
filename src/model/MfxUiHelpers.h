#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

/**
 * Shared MFX UI constants, family routing, clamps, and formatters.
 * Registered as QML singleton MfxUiFamily (replaces the former QML JS singleton).
 *
 * Family map (types 0–68): see resources/mfx_params/families.json
 *   generic · eq · filter · modulation · pipeline · slicer · rotary ·
 *   amp · compressor · delay · multiTapDelay
 */
class MfxUiHelpers : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString familyParamGrid READ familyParamGrid CONSTANT)
    Q_PROPERTY(QString familyGeneric READ familyGeneric CONSTANT)
    Q_PROPERTY(QString familyMultiTapDelay READ familyMultiTapDelay CONSTANT)
    Q_PROPERTY(QString familyDelay READ familyDelay CONSTANT)
    Q_PROPERTY(QString familyEq READ familyEq CONSTANT)
    Q_PROPERTY(QString familyFilter READ familyFilter CONSTANT)
    Q_PROPERTY(QString familyModulation READ familyModulation CONSTANT)
    Q_PROPERTY(QString familyRotary READ familyRotary CONSTANT)
    Q_PROPERTY(QString familyCompressor READ familyCompressor CONSTANT)
    Q_PROPERTY(QString familyAmp READ familyAmp CONSTANT)
    Q_PROPERTY(QString familyPipeline READ familyPipeline CONSTANT)
    Q_PROPERTY(QString familySlicer READ familySlicer CONSTANT)

    Q_PROPERTY(int idxDelayLeft READ idxDelayLeft CONSTANT)
    Q_PROPERTY(int idxDelayRight READ idxDelayRight CONSTANT)
    Q_PROPERTY(int idxDelayCenter READ idxDelayCenter CONSTANT)
    Q_PROPERTY(int idxCenterFeedback READ idxCenterFeedback CONSTANT)
    Q_PROPERTY(int idxHfDamp READ idxHfDamp CONSTANT)
    Q_PROPERTY(int idxLeftLevel READ idxLeftLevel CONSTANT)
    Q_PROPERTY(int idxRightLevel READ idxRightLevel CONSTANT)
    Q_PROPERTY(int idxCenterLevel READ idxCenterLevel CONSTANT)
    Q_PROPERTY(int idxLowGain READ idxLowGain CONSTANT)
    Q_PROPERTY(int idxHighGain READ idxHighGain CONSTANT)
    Q_PROPERTY(int idxBalance READ idxBalance CONSTANT)
    Q_PROPERTY(int idxLevel READ idxLevel CONSTANT)

    Q_PROPERTY(int delayMsMax READ delayMsMax CONSTANT)
    Q_PROPERTY(int hfDampMinHz READ hfDampMinHz CONSTANT)
    Q_PROPERTY(int hfDampMaxHz READ hfDampMaxHz CONSTANT)
    Q_PROPERTY(int hfDampBypass READ hfDampBypass CONSTANT)
    Q_PROPERTY(QStringList delayNoteNames READ delayNoteNames CONSTANT)

public:
    explicit MfxUiHelpers(QObject *parent = nullptr);

    static MfxUiHelpers *instance();

    QString familyParamGrid() const { return QStringLiteral("paramGrid"); }
    QString familyGeneric() const { return QStringLiteral("generic"); }
    QString familyMultiTapDelay() const { return QStringLiteral("multiTapDelay"); }
    QString familyDelay() const { return QStringLiteral("delay"); }
    QString familyEq() const { return QStringLiteral("eq"); }
    QString familyFilter() const { return QStringLiteral("filter"); }
    QString familyModulation() const { return QStringLiteral("modulation"); }
    QString familyRotary() const { return QStringLiteral("rotary"); }
    QString familyCompressor() const { return QStringLiteral("compressor"); }
    QString familyAmp() const { return QStringLiteral("amp"); }
    QString familyPipeline() const { return QStringLiteral("pipeline"); }
    QString familySlicer() const { return QStringLiteral("slicer"); }

    int idxDelayLeft() const { return 0; }
    int idxDelayRight() const { return 1; }
    int idxDelayCenter() const { return 2; }
    int idxCenterFeedback() const { return 3; }
    int idxHfDamp() const { return 4; }
    int idxLeftLevel() const { return 5; }
    int idxRightLevel() const { return 6; }
    int idxCenterLevel() const { return 7; }
    int idxLowGain() const { return 8; }
    int idxHighGain() const { return 9; }
    int idxBalance() const { return 10; }
    int idxLevel() const { return 11; }

    int delayMsMax() const { return 2600; }
    int hfDampMinHz() const { return 200; }
    int hfDampMaxHz() const { return 8000; }
    int hfDampBypass() const { return 8001; }
    QStringList delayNoteNames() const;

    Q_INVOKABLE QString familyForType(int typeId) const;
    Q_INVOKABLE bool usesVisualTemplate(int typeId) const;

    Q_INVOKABLE bool isDelayNote(int v) const;
    Q_INVOKABLE int delayNoteIndex(int v) const;
    Q_INVOKABLE int delayTimeForAxis(int v) const;
    Q_INVOKABLE int clampDelayMs(int v) const;
    Q_INVOKABLE QString formatDelayTime(int v) const;

    Q_INVOKABLE bool isHfDampBypass(int v) const;
    Q_INVOKABLE int clampHfDampHz(int v) const;
    Q_INVOKABLE qreal hfDampNorm(int v) const;
    Q_INVOKABLE int hfDampFromNorm(qreal n) const;
    Q_INVOKABLE qreal hfDampCurveMagnitude(int hfDamp, qreal tNorm) const;
    Q_INVOKABLE QString formatHfDamp(int v) const;

    Q_INVOKABLE int clampFeedback(int v) const;
    Q_INVOKABLE QString formatFeedback(int v) const;
    Q_INVOKABLE QString formatBalance(int v) const;
    Q_INVOKABLE QString formatDb(int v) const;
    Q_INVOKABLE QString formatLevel(int v) const;
    Q_INVOKABLE int clampLevel(int v) const;
    Q_INVOKABLE int clampGainDb(int v) const;
    Q_INVOKABLE int clampBalance(int v) const;

    /** Map uninitialized nibble decode (−32768 from all-zero raw) into UI ranges. */
    Q_INVOKABLE int sanitizeDelayTime(int v) const;
    Q_INVOKABLE int sanitizeLevel(int v) const;
    Q_INVOKABLE int sanitizeFeedback(int v) const;
    Q_INVOKABLE int sanitizeHfDamp(int v) const;
    Q_INVOKABLE int sanitizeGainDb(int v) const;
    Q_INVOKABLE int sanitizeBalance(int v) const;
};
