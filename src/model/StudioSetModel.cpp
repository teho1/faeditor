#include "model/StudioSetModel.h"
#include "midi/SysexEngine.h"
#include "midi/AddressMap.h"
#include "undo/UndoController.h"

#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>

StudioSetModel::StudioSetModel(SysexEngine *engine, UndoController *undo, QObject *parent)
    : QAbstractListModel(parent)
    , m_engine(engine)
    , m_undo(undo)
    , m_effects(new EffectsModel(this))
{
    for (int i = 0; i < 16; ++i) {
        auto *p = new PartModel(i, this);
        connect(p, &PartModel::parameterEdited, this, &StudioSetModel::onPartEdited);
        connect(p, &PartModel::partChanged, this, [this, i]() { notifyPartRow(i); });
        m_parts.push_back(p);
    }
    connect(m_effects, &EffectsModel::parameterEdited, this, &StudioSetModel::onEffectsEdited);
}

int StudioSetModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_parts.size();
}

QHash<int, QByteArray> StudioSetModel::roleNames() const
{
    return {
        {PartRole, "part"},
        {PartNumberRole, "partNumber"},
        {ToneNameRole, "toneName"},
        {LevelRole, "level"},
        {PanRole, "pan"},
        {MuteRole, "mute"},
        {SoloRole, "solo"},
        {BankMsbRole, "bankMsb"},
        {BankLsbRole, "bankLsb"},
        {ProgramRole, "program"},
        {ChannelRole, "channel"},
        {ChorusSendRole, "chorusSend"},
        {ReverbSendRole, "reverbSend"},
        {OctaveRole, "octave"},
        {KeyLowRole, "keyLow"},
        {KeyHighRole, "keyHigh"},
        {KeyboardSwitchRole, "keyboardSwitch"},
        {PartSwitchRole, "partSwitch"},
        {OutputAssignRole, "outputAssign"}
    };
}

Qt::ItemFlags StudioSetModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant StudioSetModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_parts.size())
        return {};
    auto *p = m_parts.at(index.row());
    switch (role) {
    case PartRole:
        return QVariant::fromValue(p);
    case PartNumberRole:
        return p->partNumber();
    case ToneNameRole:
        return p->toneName();
    case LevelRole:
        return p->level();
    case PanRole:
        return p->pan();
    case MuteRole:
        return p->mute();
    case SoloRole:
        return p->solo();
    case BankMsbRole:
        return p->bankMsb();
    case BankLsbRole:
        return p->bankLsb();
    case ProgramRole:
        return p->program();
    case ChannelRole:
        return p->receiveChannel() + 1;
    case ChorusSendRole:
        return p->chorusSend();
    case ReverbSendRole:
        return p->reverbSend();
    case OctaveRole:
        return p->octaveShift() - 64;
    case KeyLowRole:
        return p->keyLow();
    case KeyHighRole:
        return p->keyHigh();
    case KeyboardSwitchRole:
        return p->keyboardSwitch();
    case PartSwitchRole:
        return p->partSwitch();
    case OutputAssignRole:
        return p->outputAssign();
    default:
        return {};
    }
}

bool StudioSetModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    auto *p = m_parts.at(index.row());
    switch (role) {
    case LevelRole:
        p->setLevel(value.toInt());
        return true;
    case PanRole:
        p->setPan(value.toInt());
        return true;
    case MuteRole:
        p->setMute(value.toBool());
        return true;
    case SoloRole:
        p->setSolo(value.toBool());
        if (value.toBool())
            setSoloPart(p->partNumber());
        else if (m_soloPart == p->partNumber())
            setSoloPart(0);
        return true;
    default:
        return false;
    }
}

void StudioSetModel::notifyPartRow(int row)
{
    if (row < 0 || row >= m_parts.size())
        return;
    const auto idx = index(row, 0);
    emit dataChanged(idx, idx);
}

PartModel *StudioSetModel::partAt(int index) const
{
    if (index < 0 || index >= m_parts.size())
        return nullptr;
    return m_parts.at(index);
}

PartModel *StudioSetModel::selectedPartModel() const
{
    return partAt(m_selectedPart);
}

void StudioSetModel::setName(const QString &n)
{
    auto trimmed = n.left(16);
    if (m_name == trimmed)
        return;
    m_name = trimmed;
    emit nameChanged();
    setDirty(true);
    if (m_engine && m_engine->isOpen()) {
        QByteArray data(16, ' ');
        const QByteArray utf = m_name.toLatin1();
        for (int i = 0; i < utf.size() && i < 16; ++i)
            data[i] = utf[i];
        m_engine->writeParam(roland::addr::commonParam(0), data);
    }
    emit autosaveRequested();
}

void StudioSetModel::setSelectedPart(int v)
{
    v = std::clamp(v, 0, 15);
    if (m_selectedPart == v)
        return;
    m_selectedPart = v;
    emit selectedPartChanged();
}

void StudioSetModel::setSoloPart(int v)
{
    v = std::clamp(v, 0, 16);
    if (m_soloPart == v)
        return;
    m_soloPart = v;
    for (int i = 0; i < m_parts.size(); ++i) {
        m_parts[i]->setFromDevice(true);
        m_parts[i]->setSolo(m_soloPart == i + 1);
        m_parts[i]->setFromDevice(false);
        notifyPartRow(i);
    }
    emit soloPartChanged();
    if (m_engine && m_engine->isOpen()) {
        QByteArray d(1, static_cast<char>(m_soloPart));
        m_engine->writeParam(roland::addr::commonParam(roland::commonOff::SoloPart), d);
    }
    setDirty(true);
    emit autosaveRequested();
}

void StudioSetModel::setBusy(bool v)
{
    if (m_busy == v)
        return;
    m_busy = v;
    emit busyChanged();
}

void StudioSetModel::setError(const QString &e)
{
    m_lastError = e;
    emit lastErrorChanged();
}

void StudioSetModel::setDirty(bool v)
{
    if (m_dirty == v)
        return;
    m_dirty = v;
    emit dirtyChanged();
}

void StudioSetModel::markClean()
{
    setDirty(false);
}

void StudioSetModel::setToneNameResolver(const std::function<QString(int, int, int)> &fn)
{
    m_toneResolver = fn;
    refreshToneNames();
}

void StudioSetModel::refreshToneNames()
{
    if (!m_toneResolver)
        return;
    for (auto *p : m_parts) {
        p->setToneName(m_toneResolver(p->bankMsb(), p->bankLsb(), p->program()));
    }
}

bool StudioSetModel::pullFromDevice()
{
    if (!m_engine || !m_engine->isOpen()) {
        setError(QStringLiteral("Not connected"));
        return false;
    }
    setBusy(true);
    setError({});
    m_suppressUndo = true;

    QString err;
    QByteArray common;
    if (!m_engine->read(roland::addr::kStudioSetCommon, 0x40, &common, &err)) {
        setError(err);
        setBusy(false);
        m_suppressUndo = false;
        return false;
    }
    if (common.size() >= 16) {
        m_name = QString::fromLatin1(common.constData(), 16).trimmed();
        emit nameChanged();
    }
    if (common.size() > roland::commonOff::SoloPart)
        setSoloPart(static_cast<quint8>(common[roland::commonOff::SoloPart]));

    for (int i = 0; i < 16; ++i) {
        QByteArray partData;
        if (!m_engine->read(roland::addr::part(i), roland::partOff::PartSize, &partData, &err)) {
            setError(QStringLiteral("Part %1: %2").arg(i + 1).arg(err));
            setBusy(false);
            m_suppressUndo = false;
            return false;
        }
        m_parts[i]->loadFromPartBytes(partData);

        QByteArray zoneData;
        if (m_engine->read(roland::addr::zone(i), roland::zoneOff::ZoneSize, &zoneData, &err))
            m_parts[i]->loadFromZoneBytes(zoneData);
    }

    QByteArray chorus, reverb, mcomp;
    if (m_engine->read(roland::addr::kStudioSetChorus, 0x20, &chorus, &err))
        m_effects->loadChorus(chorus);
    if (m_engine->read(roland::addr::kStudioSetReverb, 0x20, &reverb, &err))
        m_effects->loadReverb(reverb);
    if (m_engine->read(roland::addr::kStudioSetMasterComp, 0x20, &mcomp, &err))
        m_effects->loadMasterComp(mcomp);

    refreshToneNames();
    beginResetModel();
    endResetModel();
    m_suppressUndo = false;
    setBusy(false);
    setDirty(false);
    if (m_undo)
        m_undo->clear();
    emit studioSetLoaded();
    return true;
}

bool StudioSetModel::pushToDevice()
{
    if (!m_engine || !m_engine->isOpen()) {
        setError(QStringLiteral("Not connected"));
        return false;
    }
    setBusy(true);
    QString err;

    QByteArray nameData(16, ' ');
    const auto latin = m_name.toLatin1();
    for (int i = 0; i < latin.size() && i < 16; ++i)
        nameData[i] = latin[i];
    if (!m_engine->write(roland::addr::kStudioSetCommon, nameData, &err)) {
        setError(err);
        setBusy(false);
        return false;
    }

    for (int i = 0; i < 16; ++i) {
        if (!m_engine->write(roland::addr::part(i), m_parts[i]->toPartBytes(), &err)) {
            setError(err);
            setBusy(false);
            return false;
        }
        if (!m_engine->write(roland::addr::zone(i), m_parts[i]->toZoneBytes(), &err)) {
            setError(err);
            setBusy(false);
            return false;
        }
    }

    m_engine->write(roland::addr::kStudioSetChorus, m_effects->chorusBytes(), &err);
    m_engine->write(roland::addr::kStudioSetReverb, m_effects->reverbBytes(), &err);
    m_engine->write(roland::addr::kStudioSetMasterComp, m_effects->masterCompBytes(), &err);

    setBusy(false);
    setDirty(false);
    return true;
}

void StudioSetModel::onPartEdited(int partIndex, const QString &param, int value)
{
    if (m_suppressUndo)
        return;
    if (m_undo) {
        auto *p = partAt(partIndex);
        Q_UNUSED(p);
        m_undo->pushPartChange(this, partIndex, param, value);
    }
    writePartParam(partIndex, param, value);
    setDirty(true);
    emit autosaveRequested();
    notifyPartRow(partIndex);
}

void StudioSetModel::onEffectsEdited(const QString &section, const QString &param, int value)
{
    if (m_suppressUndo)
        return;
    if (m_undo)
        m_undo->pushEffectChange(this, section, param, value);
    writeEffectParam(section, param, value);
    setDirty(true);
    emit autosaveRequested();
}

void StudioSetModel::writePartParam(int partIndex, const QString &param, int value)
{
    if (!m_engine || !m_engine->isOpen())
        return;
    using namespace roland;
    Address a{};
    QByteArray d(1, static_cast<char>(value & 0x7F));
    if (param == QLatin1String("receiveChannel"))
        a = addr::partParam(partIndex, partOff::ReceiveChannel);
    else if (param == QLatin1String("partSwitch"))
        a = addr::partParam(partIndex, partOff::PartSwitch);
    else if (param == QLatin1String("bankMsb"))
        a = addr::partParam(partIndex, partOff::ToneBankMsb);
    else if (param == QLatin1String("bankLsb"))
        a = addr::partParam(partIndex, partOff::ToneBankLsb);
    else if (param == QLatin1String("program"))
        a = addr::partParam(partIndex, partOff::ToneProgram);
    else if (param == QLatin1String("level"))
        a = addr::partParam(partIndex, partOff::PartLevel);
    else if (param == QLatin1String("pan"))
        a = addr::partParam(partIndex, partOff::PartPan);
    else if (param == QLatin1String("coarseTune"))
        a = addr::partParam(partIndex, partOff::PartCoarseTune);
    else if (param == QLatin1String("octaveShift"))
        a = addr::partParam(partIndex, partOff::PartOctaveShift);
    else if (param == QLatin1String("velocityLow"))
        a = addr::partParam(partIndex, partOff::VelocityRangeLower);
    else if (param == QLatin1String("velocityHigh"))
        a = addr::partParam(partIndex, partOff::VelocityRangeUpper);
    else if (param == QLatin1String("mute"))
        a = addr::partParam(partIndex, partOff::MuteSwitch);
    else if (param == QLatin1String("chorusSend"))
        a = addr::partParam(partIndex, partOff::ChorusSend);
    else if (param == QLatin1String("reverbSend"))
        a = addr::partParam(partIndex, partOff::ReverbSend);
    else if (param == QLatin1String("outputAssign"))
        a = addr::partParam(partIndex, partOff::OutputAssign);
    else if (param == QLatin1String("keyLow"))
        a = addr::zoneParam(partIndex, zoneOff::KeyRangeLower);
    else if (param == QLatin1String("keyHigh"))
        a = addr::zoneParam(partIndex, zoneOff::KeyRangeUpper);
    else if (param == QLatin1String("keyboardSwitch"))
        a = addr::zoneParam(partIndex, zoneOff::KeyboardSwitch);
    else if (param == QLatin1String("solo")) {
        setSoloPart(value ? partIndex + 1 : 0);
        return;
    } else
        return;

    m_engine->writeParam(a, d);
    if (param == QLatin1String("bankMsb") || param == QLatin1String("bankLsb")
        || param == QLatin1String("program"))
        refreshToneNames();
}

void StudioSetModel::writeEffectParam(const QString &section, const QString &param, int value)
{
    if (!m_engine || !m_engine->isOpen())
        return;
    using namespace roland;
    Address a{};
    QByteArray d(1, static_cast<char>(value & 0x7F));
    if (section == QLatin1String("chorus")) {
        // MIDI Imple Studio Set Chorus: 00 Switch, 01 Type, 02 Level
        if (param == QLatin1String("type"))
            a = addr::chorusParam(0x01);
        else if (param == QLatin1String("level"))
            a = addr::chorusParam(0x02);
        else
            return;
    } else if (section == QLatin1String("reverb")) {
        if (param == QLatin1String("type"))
            a = addr::reverbParam(0x01);
        else if (param == QLatin1String("level"))
            a = addr::reverbParam(0x02);
        else
            return;
    } else if (section == QLatin1String("masterComp")) {
        if (param == QLatin1String("switch"))
            a = addr::masterCompParam(0x00);
        else if (param == QLatin1String("attack"))
            a = addr::masterCompParam(0x01);
        else if (param == QLatin1String("release"))
            a = addr::masterCompParam(0x02);
        else if (param == QLatin1String("threshold"))
            a = addr::masterCompParam(0x03);
        else if (param == QLatin1String("ratio"))
            a = addr::masterCompParam(0x04);
        else if (param == QLatin1String("gain"))
            a = addr::masterCompParam(0x05);
        else
            return;
    } else
        return;
    m_engine->writeParam(a, d);
}

QJsonObject StudioSetModel::toJson() const
{
    QJsonObject root;
    root.insert(QStringLiteral("name"), m_name);
    root.insert(QStringLiteral("soloPart"), m_soloPart);
    QJsonArray parts;
    for (auto *p : m_parts) {
        QJsonObject o;
        o.insert(QStringLiteral("receiveChannel"), p->receiveChannel());
        o.insert(QStringLiteral("partSwitch"), p->partSwitch());
        o.insert(QStringLiteral("bankMsb"), p->bankMsb());
        o.insert(QStringLiteral("bankLsb"), p->bankLsb());
        o.insert(QStringLiteral("program"), p->program());
        o.insert(QStringLiteral("level"), p->level());
        o.insert(QStringLiteral("pan"), p->pan());
        o.insert(QStringLiteral("coarseTune"), p->coarseTune());
        o.insert(QStringLiteral("octaveShift"), p->octaveShift());
        o.insert(QStringLiteral("velocityLow"), p->velocityLow());
        o.insert(QStringLiteral("velocityHigh"), p->velocityHigh());
        o.insert(QStringLiteral("mute"), p->mute());
        o.insert(QStringLiteral("chorusSend"), p->chorusSend());
        o.insert(QStringLiteral("reverbSend"), p->reverbSend());
        o.insert(QStringLiteral("outputAssign"), p->outputAssign());
        o.insert(QStringLiteral("keyLow"), p->keyLow());
        o.insert(QStringLiteral("keyHigh"), p->keyHigh());
        o.insert(QStringLiteral("keyboardSwitch"), p->keyboardSwitch());
        o.insert(QStringLiteral("toneName"), p->toneName());
        parts.append(o);
    }
    root.insert(QStringLiteral("parts"), parts);
    QJsonObject fx;
    fx.insert(QStringLiteral("chorusType"), m_effects->chorusType());
    fx.insert(QStringLiteral("chorusLevel"), m_effects->chorusLevel());
    fx.insert(QStringLiteral("reverbType"), m_effects->reverbType());
    fx.insert(QStringLiteral("reverbLevel"), m_effects->reverbLevel());
    fx.insert(QStringLiteral("masterCompSwitch"), m_effects->masterCompSwitch());
    fx.insert(QStringLiteral("masterCompAttack"), m_effects->masterCompAttack());
    fx.insert(QStringLiteral("masterCompRelease"), m_effects->masterCompRelease());
    fx.insert(QStringLiteral("masterCompThreshold"), m_effects->masterCompThreshold());
    fx.insert(QStringLiteral("masterCompRatio"), m_effects->masterCompRatio());
    fx.insert(QStringLiteral("masterCompGain"), m_effects->masterCompGain());
    root.insert(QStringLiteral("effects"), fx);
    return root;
}

bool StudioSetModel::fromJson(const QJsonObject &obj)
{
    m_suppressUndo = true;
    setName(obj.value(QStringLiteral("name")).toString(QStringLiteral("INIT STUDIOSET")));
    setSoloPart(obj.value(QStringLiteral("soloPart")).toInt(0));
    const auto parts = obj.value(QStringLiteral("parts")).toArray();
    for (int i = 0; i < 16 && i < parts.size(); ++i) {
        const auto o = parts.at(i).toObject();
        auto *p = m_parts[i];
        p->setFromDevice(true);
        p->setReceiveChannel(o.value(QStringLiteral("receiveChannel")).toInt(i));
        p->setPartSwitch(o.value(QStringLiteral("partSwitch")).toBool(true));
        p->setBankMsb(o.value(QStringLiteral("bankMsb")).toInt(87));
        p->setBankLsb(o.value(QStringLiteral("bankLsb")).toInt(64));
        p->setProgram(o.value(QStringLiteral("program")).toInt(0));
        p->setLevel(o.value(QStringLiteral("level")).toInt(100));
        p->setPan(o.value(QStringLiteral("pan")).toInt(64));
        p->setCoarseTune(o.value(QStringLiteral("coarseTune")).toInt(64));
        p->setOctaveShift(o.value(QStringLiteral("octaveShift")).toInt(64));
        p->setVelocityLow(o.value(QStringLiteral("velocityLow")).toInt(1));
        p->setVelocityHigh(o.value(QStringLiteral("velocityHigh")).toInt(127));
        p->setMute(o.value(QStringLiteral("mute")).toBool(false));
        p->setChorusSend(o.value(QStringLiteral("chorusSend")).toInt(0));
        p->setReverbSend(o.value(QStringLiteral("reverbSend")).toInt(0));
        p->setOutputAssign(o.value(QStringLiteral("outputAssign")).toInt(0));
        p->setKeyLow(o.value(QStringLiteral("keyLow")).toInt(0));
        p->setKeyHigh(o.value(QStringLiteral("keyHigh")).toInt(127));
        p->setKeyboardSwitch(o.value(QStringLiteral("keyboardSwitch")).toBool(true));
        p->setToneName(o.value(QStringLiteral("toneName")).toString());
        p->setFromDevice(false);
    }
    const auto fx = obj.value(QStringLiteral("effects")).toObject();
    m_effects->setFromDevice(true);
    m_effects->setChorusType(fx.value(QStringLiteral("chorusType")).toInt(0));
    m_effects->setChorusLevel(fx.value(QStringLiteral("chorusLevel")).toInt(0));
    m_effects->setReverbType(fx.value(QStringLiteral("reverbType")).toInt(0));
    m_effects->setReverbLevel(fx.value(QStringLiteral("reverbLevel")).toInt(0));
    m_effects->setMasterCompSwitch(fx.value(QStringLiteral("masterCompSwitch")).toBool(false));
    m_effects->setMasterCompAttack(fx.value(QStringLiteral("masterCompAttack")).toInt(0));
    m_effects->setMasterCompRelease(fx.value(QStringLiteral("masterCompRelease")).toInt(0));
    m_effects->setMasterCompThreshold(fx.value(QStringLiteral("masterCompThreshold")).toInt(0));
    m_effects->setMasterCompRatio(fx.value(QStringLiteral("masterCompRatio")).toInt(0));
    m_effects->setMasterCompGain(fx.value(QStringLiteral("masterCompGain")).toInt(0));
    m_effects->setFromDevice(false);
    refreshToneNames();
    beginResetModel();
    endResetModel();
    m_suppressUndo = false;
    setDirty(false);
    emit studioSetLoaded();
    return true;
}
