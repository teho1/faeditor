#include "undo/UndoController.h"
#include "model/StudioSetModel.h"
#include "model/PartModel.h"
#include "model/EffectsModel.h"

UndoController::UndoController(QObject *parent)
    : QObject(parent)
{
    connect(&m_stack, &QUndoStack::indexChanged, this, &UndoController::changed);
    connect(&m_stack, &QUndoStack::cleanChanged, this, &UndoController::changed);
}

bool UndoController::canUndo() const { return m_stack.canUndo(); }
bool UndoController::canRedo() const { return m_stack.canRedo(); }
QString UndoController::undoText() const { return m_stack.undoText(); }
QString UndoController::redoText() const { return m_stack.redoText(); }

void UndoController::undo() { m_stack.undo(); }
void UndoController::redo() { m_stack.redo(); }

void UndoController::clear()
{
    m_stack.clear();
    m_lastPartValues.clear();
    m_lastEffectValues.clear();
    emit changed();
}

void UndoController::pushPartChange(StudioSetModel *model, int partIndex, const QString &param, int newValue)
{
    const QString key = QStringLiteral("%1:%2").arg(partIndex).arg(param);
    if (!m_lastPartValues.contains(key)) {
        m_lastPartValues.insert(key, newValue);
        return;
    }
    const int oldValue = m_lastPartValues.value(key);
    if (oldValue == newValue)
        return;
    m_stack.push(new PartUndoCommand(model, partIndex, param, oldValue, newValue));
    m_lastPartValues.insert(key, newValue);
}

void UndoController::pushEffectChange(StudioSetModel *model, const QString &section, const QString &param, int newValue)
{
    const QString key = section + QLatin1Char(':') + param;
    if (!m_lastEffectValues.contains(key)) {
        m_lastEffectValues.insert(key, newValue);
        return;
    }
    const int oldValue = m_lastEffectValues.value(key);
    if (oldValue == newValue)
        return;
    m_stack.push(new EffectUndoCommand(model, section, param, oldValue, newValue));
    m_lastEffectValues.insert(key, newValue);
}

static void setPartParam(PartModel *p, const QString &param, int value)
{
    if (!p)
        return;
    if (param == QLatin1String("receiveChannel"))
        p->setReceiveChannel(value);
    else if (param == QLatin1String("partSwitch"))
        p->setPartSwitch(value != 0);
    else if (param == QLatin1String("bankMsb"))
        p->setBankMsb(value);
    else if (param == QLatin1String("bankLsb"))
        p->setBankLsb(value);
    else if (param == QLatin1String("program"))
        p->setProgram(value);
    else if (param == QLatin1String("level"))
        p->setLevel(value);
    else if (param == QLatin1String("pan"))
        p->setPan(value);
    else if (param == QLatin1String("coarseTune"))
        p->setCoarseTune(value);
    else if (param == QLatin1String("octaveShift"))
        p->setOctaveShift(value);
    else if (param == QLatin1String("velocityLow"))
        p->setVelocityLow(value);
    else if (param == QLatin1String("velocityHigh"))
        p->setVelocityHigh(value);
    else if (param == QLatin1String("mute"))
        p->setMute(value != 0);
    else if (param == QLatin1String("chorusSend"))
        p->setChorusSend(value);
    else if (param == QLatin1String("reverbSend"))
        p->setReverbSend(value);
    else if (param == QLatin1String("outputAssign"))
        p->setOutputAssign(value);
    else if (param == QLatin1String("keyLow"))
        p->setKeyLow(value);
    else if (param == QLatin1String("keyHigh"))
        p->setKeyHigh(value);
    else if (param == QLatin1String("keyboardSwitch"))
        p->setKeyboardSwitch(value != 0);
}

PartUndoCommand::PartUndoCommand(StudioSetModel *model, int partIndex, QString param, int oldValue, int newValue)
    : QUndoCommand(QStringLiteral("Part %1 %2").arg(partIndex + 1).arg(param))
    , m_model(model)
    , m_partIndex(partIndex)
    , m_param(std::move(param))
    , m_oldValue(oldValue)
    , m_newValue(newValue)
{
}

void PartUndoCommand::apply(int value)
{
    auto *p = m_model->partAt(m_partIndex);
    if (!p)
        return;
    setPartParam(p, m_param, value);
}

void PartUndoCommand::undo() { apply(m_oldValue); }
void PartUndoCommand::redo() { apply(m_newValue); }

EffectUndoCommand::EffectUndoCommand(StudioSetModel *model, QString section, QString param, int oldValue, int newValue)
    : QUndoCommand(section + QLatin1Char(' ') + param)
    , m_model(model)
    , m_section(std::move(section))
    , m_param(std::move(param))
    , m_oldValue(oldValue)
    , m_newValue(newValue)
{
}

void EffectUndoCommand::apply(int value)
{
    auto *fx = m_model->effects();
    if (!fx)
        return;
    if (m_section == QLatin1String("chorus")) {
        if (m_param == QLatin1String("type"))
            fx->setChorusType(value);
        else if (m_param == QLatin1String("level"))
            fx->setChorusLevel(value);
    } else if (m_section == QLatin1String("reverb")) {
        if (m_param == QLatin1String("type"))
            fx->setReverbType(value);
        else if (m_param == QLatin1String("level"))
            fx->setReverbLevel(value);
    } else if (m_section == QLatin1String("masterComp")) {
        if (m_param == QLatin1String("switch"))
            fx->setMasterCompSwitch(value != 0);
        else if (m_param == QLatin1String("attack"))
            fx->setMasterCompAttack(value);
        else if (m_param == QLatin1String("release"))
            fx->setMasterCompRelease(value);
        else if (m_param == QLatin1String("threshold"))
            fx->setMasterCompThreshold(value);
        else if (m_param == QLatin1String("ratio"))
            fx->setMasterCompRatio(value);
        else if (m_param == QLatin1String("gain"))
            fx->setMasterCompGain(value);
    }
}

void EffectUndoCommand::undo() { apply(m_oldValue); }
void EffectUndoCommand::redo() { apply(m_newValue); }
