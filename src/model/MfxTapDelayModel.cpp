#include "model/MfxTapDelayModel.h"
#include "model/MfxModel.h"
#include "model/MfxUiHelpers.h"

#include <algorithm>

namespace {
constexpr int kType3TapPanDelay = 36;
}

MfxTapDelayModel::MfxTapDelayModel(MfxModel *mfx, QObject *parent)
    : QObject(parent)
    , m_mfx(mfx)
{
    if (m_mfx) {
        connect(m_mfx, &MfxModel::mfxChanged, this, &MfxTapDelayModel::syncFromMfx);
        syncFromMfx();
    }
}

bool MfxTapDelayModel::active() const
{
    return m_active;
}

int MfxTapDelayModel::rawParam(int index) const
{
    return m_mfx ? m_mfx->paramValue(index) : 0;
}

void MfxTapDelayModel::writeParam(int index, int value)
{
    if (m_mfx)
        m_mfx->setParamValue(index, value);
}

void MfxTapDelayModel::maybeSeedDemoDefaults()
{
    if (!m_mfx || m_mfx->type() != kType3TapPanDelay)
        return;
    auto *ui = MfxUiHelpers::instance();
    // All-zero nibble params decode to −32768. Real 0 ms encodes as packed 32768 (≥ 0).
    if (rawParam(ui->idxDelayLeft()) >= 0
        || rawParam(ui->idxDelayRight()) >= 0
        || rawParam(ui->idxDelayCenter()) >= 0) {
        return;
    }

    // Readable L/C/R spread so the timeline is obviously interactive on first open.
    m_mfx->setParamValue(ui->idxDelayLeft(), 200);
    m_mfx->setParamValue(ui->idxDelayCenter(), 400);
    m_mfx->setParamValue(ui->idxDelayRight(), 600);
    m_mfx->setParamValue(ui->idxLeftLevel(), 100);
    m_mfx->setParamValue(ui->idxCenterLevel(), 110);
    m_mfx->setParamValue(ui->idxRightLevel(), 100);
    m_mfx->setParamValue(ui->idxCenterFeedback(), 32);
    m_mfx->setParamValue(ui->idxHfDamp(), ui->hfDampBypass());
    m_mfx->setParamValue(ui->idxLowGain(), 0);
    m_mfx->setParamValue(ui->idxHighGain(), 0);
    m_mfx->setParamValue(ui->idxBalance(), 50);
    m_mfx->setParamValue(ui->idxLevel(), 100);
}

void MfxTapDelayModel::syncFromMfx()
{
    if (!m_mfx || m_inSync)
        return;
    m_inSync = true;

    maybeSeedDemoDefaults();

    auto *ui = MfxUiHelpers::instance();

    const bool active = m_mfx->type() == kType3TapPanDelay;
    const int leftTime = ui->sanitizeDelayTime(rawParam(ui->idxDelayLeft()));
    const int rightTime = ui->sanitizeDelayTime(rawParam(ui->idxDelayRight()));
    const int centerTime = ui->sanitizeDelayTime(rawParam(ui->idxDelayCenter()));
    const int leftLevel = ui->sanitizeLevel(rawParam(ui->idxLeftLevel()));
    const int rightLevel = ui->sanitizeLevel(rawParam(ui->idxRightLevel()));
    const int centerLevel = ui->sanitizeLevel(rawParam(ui->idxCenterLevel()));
    const int feedback = ui->sanitizeFeedback(rawParam(ui->idxCenterFeedback()));
    const int hfDamp = ui->sanitizeHfDamp(rawParam(ui->idxHfDamp()));
    const int lowGain = ui->sanitizeGainDb(rawParam(ui->idxLowGain()));
    const int highGain = ui->sanitizeGainDb(rawParam(ui->idxHighGain()));
    const int balance = ui->sanitizeBalance(rawParam(ui->idxBalance()));
    const int level = ui->sanitizeLevel(rawParam(ui->idxLevel()));
    const int chorusSend = m_mfx->chorusSend();
    const int reverbSend = m_mfx->reverbSend();

    bool any = false;
    auto setI = [&](int &dst, int v, void (MfxTapDelayModel::*sig)()) {
        if (dst == v)
            return;
        dst = v;
        emit (this->*sig)();
        any = true;
    };

    if (m_active != active) {
        m_active = active;
        emit activeChanged();
        any = true;
    }
    setI(m_leftTime, leftTime, &MfxTapDelayModel::leftTimeChanged);
    setI(m_rightTime, rightTime, &MfxTapDelayModel::rightTimeChanged);
    setI(m_centerTime, centerTime, &MfxTapDelayModel::centerTimeChanged);
    setI(m_leftLevel, leftLevel, &MfxTapDelayModel::leftLevelChanged);
    setI(m_rightLevel, rightLevel, &MfxTapDelayModel::rightLevelChanged);
    setI(m_centerLevel, centerLevel, &MfxTapDelayModel::centerLevelChanged);
    setI(m_feedback, feedback, &MfxTapDelayModel::feedbackChanged);
    const bool prevBypass = MfxUiHelpers::instance()->isHfDampBypass(m_hfDamp);
    setI(m_hfDamp, hfDamp, &MfxTapDelayModel::hfDampChanged);
    if (prevBypass != ui->isHfDampBypass(m_hfDamp))
        emit hfDampBypassChanged();
    setI(m_lowGain, lowGain, &MfxTapDelayModel::lowGainChanged);
    setI(m_highGain, highGain, &MfxTapDelayModel::highGainChanged);
    setI(m_balance, balance, &MfxTapDelayModel::balanceChanged);
    setI(m_level, level, &MfxTapDelayModel::levelChanged);
    setI(m_chorusSend, chorusSend, &MfxTapDelayModel::chorusSendChanged);
    setI(m_reverbSend, reverbSend, &MfxTapDelayModel::reverbSendChanged);

    if (any)
        emit tapChanged();

    m_inSync = false;
}

int MfxTapDelayModel::leftTime() const { return m_leftTime; }
int MfxTapDelayModel::rightTime() const { return m_rightTime; }
int MfxTapDelayModel::centerTime() const { return m_centerTime; }
int MfxTapDelayModel::leftLevel() const { return m_leftLevel; }
int MfxTapDelayModel::rightLevel() const { return m_rightLevel; }
int MfxTapDelayModel::centerLevel() const { return m_centerLevel; }
int MfxTapDelayModel::feedback() const { return m_feedback; }
int MfxTapDelayModel::hfDamp() const { return m_hfDamp; }
bool MfxTapDelayModel::hfDampBypass() const
{
    return MfxUiHelpers::instance()->isHfDampBypass(m_hfDamp);
}
qreal MfxTapDelayModel::hfDampNorm() const
{
    return MfxUiHelpers::instance()->hfDampNorm(m_hfDamp);
}
int MfxTapDelayModel::lowGain() const { return m_lowGain; }
int MfxTapDelayModel::highGain() const { return m_highGain; }
int MfxTapDelayModel::balance() const { return m_balance; }
int MfxTapDelayModel::level() const { return m_level; }
int MfxTapDelayModel::chorusSend() const { return m_chorusSend; }
int MfxTapDelayModel::reverbSend() const { return m_reverbSend; }

QString MfxTapDelayModel::feedbackLabel() const
{
    return MfxUiHelpers::instance()->formatFeedback(m_feedback);
}
QString MfxTapDelayModel::hfDampLabel() const
{
    return MfxUiHelpers::instance()->formatHfDamp(m_hfDamp);
}
QString MfxTapDelayModel::balanceLabel() const
{
    return MfxUiHelpers::instance()->formatBalance(m_balance);
}

void MfxTapDelayModel::setLeftTime(int v)
{
    writeParam(MfxUiHelpers::instance()->idxDelayLeft(),
               MfxUiHelpers::instance()->clampDelayMs(v));
}
void MfxTapDelayModel::setRightTime(int v)
{
    writeParam(MfxUiHelpers::instance()->idxDelayRight(),
               MfxUiHelpers::instance()->clampDelayMs(v));
}
void MfxTapDelayModel::setCenterTime(int v)
{
    writeParam(MfxUiHelpers::instance()->idxDelayCenter(),
               MfxUiHelpers::instance()->clampDelayMs(v));
}
void MfxTapDelayModel::setLeftLevel(int v)
{
    writeParam(MfxUiHelpers::instance()->idxLeftLevel(),
               MfxUiHelpers::instance()->clampLevel(v));
}
void MfxTapDelayModel::setRightLevel(int v)
{
    writeParam(MfxUiHelpers::instance()->idxRightLevel(),
               MfxUiHelpers::instance()->clampLevel(v));
}
void MfxTapDelayModel::setCenterLevel(int v)
{
    writeParam(MfxUiHelpers::instance()->idxCenterLevel(),
               MfxUiHelpers::instance()->clampLevel(v));
}
void MfxTapDelayModel::setFeedback(int v)
{
    writeParam(MfxUiHelpers::instance()->idxCenterFeedback(),
               MfxUiHelpers::instance()->clampFeedback(v));
}
void MfxTapDelayModel::setHfDamp(int v)
{
    auto *ui = MfxUiHelpers::instance();
    if (ui->isHfDampBypass(v))
        writeParam(ui->idxHfDamp(), ui->hfDampBypass());
    else
        writeParam(ui->idxHfDamp(), ui->clampHfDampHz(v));
}
void MfxTapDelayModel::setHfDampBypass(bool on)
{
    auto *ui = MfxUiHelpers::instance();
    if (on)
        writeParam(ui->idxHfDamp(), ui->hfDampBypass());
    else if (hfDampBypass())
        writeParam(ui->idxHfDamp(), 800);
}
void MfxTapDelayModel::setLowGain(int v)
{
    writeParam(MfxUiHelpers::instance()->idxLowGain(),
               MfxUiHelpers::instance()->clampGainDb(v));
}
void MfxTapDelayModel::setHighGain(int v)
{
    writeParam(MfxUiHelpers::instance()->idxHighGain(),
               MfxUiHelpers::instance()->clampGainDb(v));
}
void MfxTapDelayModel::setBalance(int v)
{
    writeParam(MfxUiHelpers::instance()->idxBalance(),
               MfxUiHelpers::instance()->clampBalance(v));
}
void MfxTapDelayModel::setLevel(int v)
{
    writeParam(MfxUiHelpers::instance()->idxLevel(),
               MfxUiHelpers::instance()->clampLevel(v));
}
void MfxTapDelayModel::setChorusSend(int v)
{
    if (m_mfx)
        m_mfx->setChorusSend(v);
}
void MfxTapDelayModel::setReverbSend(int v)
{
    if (m_mfx)
        m_mfx->setReverbSend(v);
}

int MfxTapDelayModel::timeOf(int tap) const
{
    if (tap == 0) return m_leftTime;
    if (tap == 2) return m_rightTime;
    return m_centerTime;
}

int MfxTapDelayModel::levelOf(int tap) const
{
    if (tap == 0) return m_leftLevel;
    if (tap == 2) return m_rightLevel;
    return m_centerLevel;
}

void MfxTapDelayModel::setTime(int tap, int ms)
{
    if (tap == 0) setLeftTime(ms);
    else if (tap == 2) setRightTime(ms);
    else setCenterTime(ms);
}

void MfxTapDelayModel::setLevelAt(int tap, int level)
{
    if (tap == 0) setLeftLevel(level);
    else if (tap == 2) setRightLevel(level);
    else setCenterLevel(level);
}

int MfxTapDelayModel::spinTimeValue(int tap) const
{
    auto *ui = MfxUiHelpers::instance();
    const int t = timeOf(tap);
    if (ui->isDelayNote(t))
        return ui->delayMsMax();
    return ui->clampDelayMs(t);
}

bool MfxTapDelayModel::isDelayNote(int tap) const
{
    return MfxUiHelpers::instance()->isDelayNote(timeOf(tap));
}

QString MfxTapDelayModel::formatTime(int tap) const
{
    return MfxUiHelpers::instance()->formatDelayTime(timeOf(tap));
}

qreal MfxTapDelayModel::hfDampCurveMagnitude(qreal tNorm) const
{
    return MfxUiHelpers::instance()->hfDampCurveMagnitude(m_hfDamp, tNorm);
}

void MfxTapDelayModel::setHfDampFromNorm(qreal n)
{
    setHfDamp(MfxUiHelpers::instance()->hfDampFromNorm(n));
}

int MfxTapDelayModel::delayTimeForAxis(int v) const
{
    return MfxUiHelpers::instance()->delayTimeForAxis(v);
}

int MfxTapDelayModel::timeFromAxisNorm(qreal nx) const
{
    auto *ui = MfxUiHelpers::instance();
    nx = std::clamp(nx, 0.0, 1.0);
    return ui->clampDelayMs(int(std::lround(nx * ui->delayMsMax())));
}

int MfxTapDelayModel::levelFromAxisNorm(qreal ny) const
{
    ny = std::clamp(ny, 0.0, 1.0);
    return MfxUiHelpers::instance()->clampLevel(int(std::lround(ny * 127.0)));
}
