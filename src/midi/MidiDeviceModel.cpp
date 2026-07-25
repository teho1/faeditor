#include "midi/MidiDeviceModel.h"
#include "midi/SysexEngine.h"

#include <RtMidi.h>
#include <algorithm>

MidiDeviceModel::MidiDeviceModel(SysexEngine *engine, QObject *parent)
    : QAbstractListModel(parent)
    , m_engine(engine)
{
    m_connectionPoll.setInterval(1500);
    connect(&m_connectionPoll, &QTimer::timeout, this, &MidiDeviceModel::pollConnection);
    refresh();
}

int MidiDeviceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_inputs.size();
}

QVariant MidiDeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_inputs.size())
        return {};
    const auto &p = m_inputs.at(index.row());
    switch (role) {
    case NameRole:
    case Qt::DisplayRole:
        return p.name;
    case IndexRole:
        return p.index;
    case IsDawControlRole:
        return p.isDawControl;
    case LooksLikeFaRole:
        return p.looksLikeFa;
    default:
        return {};
    }
}

QHash<int, QByteArray> MidiDeviceModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {IndexRole, "portIndex"},
        {IsDawControlRole, "isDawControl"},
        {LooksLikeFaRole, "looksLikeFa"}
    };
}

void MidiDeviceModel::setSelectedInput(int v)
{
    if (m_selectedInput == v)
        return;
    m_selectedInput = v;
    emit selectionChanged();
}

void MidiDeviceModel::setSelectedOutput(int v)
{
    if (m_selectedOutput == v)
        return;
    m_selectedOutput = v;
    emit selectionChanged();
}

bool MidiDeviceModel::nameIsDawControl(const QString &name)
{
    const auto n = name.toLower();
    return n.contains(QStringLiteral("daw"))
           || n.contains(QStringLiteral("mackie"))
           || (n.contains(QStringLiteral("ctrl")) && !n.contains(QStringLiteral("controller")));
}

bool MidiDeviceModel::nameLooksLikeFa(const QString &name)
{
    if (nameIsDawControl(name))
        return false;
    const auto n = name.toUpper();
    // Common CoreMIDI labels: "FA-08", "Roland FA-07", "FA08", "FA 06"
    if (n.contains(QStringLiteral("FA-06")) || n.contains(QStringLiteral("FA-07"))
        || n.contains(QStringLiteral("FA-08")) || n.contains(QStringLiteral("FA 06"))
        || n.contains(QStringLiteral("FA 07")) || n.contains(QStringLiteral("FA 08"))
        || n.contains(QStringLiteral("FA06")) || n.contains(QStringLiteral("FA07"))
        || n.contains(QStringLiteral("FA08")))
        return true;
    // Broader: "FA" + model digit, but avoid random "FA" substrings in other devices
    return n.contains(QLatin1String("ROLAND")) && n.contains(QLatin1String("FA"));
}

void MidiDeviceModel::setStatus(const QString &text)
{
    if (m_statusText == text)
        return;
    m_statusText = text;
    emit statusTextChanged();
}

bool MidiDeviceModel::indexInRange(int idx, int count) const
{
    return idx >= 0 && idx < count;
}

bool MidiDeviceModel::selectionValid() const
{
    return indexInRange(m_selectedInput, m_inputs.size())
           && indexInRange(m_selectedOutput, m_outputs.size());
}

void MidiDeviceModel::preferFaSelection(bool force)
{
    const bool inOk = indexInRange(m_selectedInput, m_inputs.size());
    const bool outOk = indexInRange(m_selectedOutput, m_outputs.size());
    const bool inIsFa = inOk && m_inputs.at(m_selectedInput).looksLikeFa;
    const bool outIsFa = outOk && m_outputs.at(m_selectedOutput).looksLikeFa;

    if (force || !inOk || !inIsFa) {
        int found = -1;
        for (const auto &p : m_inputs) {
            if (p.looksLikeFa) {
                found = p.index;
                break;
            }
        }
        if (found >= 0)
            m_selectedInput = found;
        else if (!inOk)
            m_selectedInput = m_inputs.isEmpty() ? -1 : 0;
    }

    if (force || !outOk || !outIsFa) {
        int found = -1;
        for (const auto &p : m_outputs) {
            if (p.looksLikeFa) {
                found = p.index;
                break;
            }
        }
        if (found >= 0)
            m_selectedOutput = found;
        else if (!outOk)
            m_selectedOutput = m_outputs.isEmpty() ? -1 : 0;
    }
    emit selectionChanged();
}

void MidiDeviceModel::refresh()
{
    beginResetModel();
    m_inputs.clear();
    m_outputs.clear();
    try {
        RtMidiIn in(RtMidi::MACOSX_CORE);
        RtMidiOut out(RtMidi::MACOSX_CORE);
        for (unsigned i = 0; i < in.getPortCount(); ++i) {
            MidiPortInfo info;
            info.index = static_cast<int>(i);
            info.name = QString::fromStdString(in.getPortName(i));
            info.isDawControl = nameIsDawControl(info.name);
            info.looksLikeFa = nameLooksLikeFa(info.name);
            m_inputs.push_back(info);
        }
        for (unsigned i = 0; i < out.getPortCount(); ++i) {
            MidiPortInfo info;
            info.index = static_cast<int>(i);
            info.name = QString::fromStdString(out.getPortName(i));
            info.isDawControl = nameIsDawControl(info.name);
            info.looksLikeFa = nameLooksLikeFa(info.name);
            m_outputs.push_back(info);
        }
    } catch (const RtMidiError &e) {
        setStatus(QString::fromStdString(e.getMessage()));
    }
    endResetModel();

    // If we thought we were connected but the engine/ports are gone (FA power-cycled), clear flag.
    if (m_connected && (!m_engine || !m_engine->isOpen() || !connectedPortStillPresent())) {
        stopConnectionPoll();
        if (m_engine && m_engine->isOpen())
            m_engine->closePorts();
        m_connected = false;
        m_connectedName.clear();
        setStatus(QStringLiteral("FA disconnected — cable unplugged or powered off"));
        emit connectedChanged();
    }

    preferFaSelection(false);
    emit portsChanged();
}

QStringList MidiDeviceModel::inputNames() const
{
    QStringList list;
    for (const auto &p : m_inputs)
        list << p.name;
    return list;
}

QStringList MidiDeviceModel::outputNames() const
{
    QStringList list;
    for (const auto &p : m_outputs)
        list << p.name;
    return list;
}

bool MidiDeviceModel::connectSelected()
{
    if (!m_engine)
        return false;

    refresh();
    if (!selectionValid()) {
        setStatus(QStringLiteral("Select MIDI in/out ports"));
        return false;
    }

    // Re-open cleanly (important after FA reboot / CoreMIDI renumber)
    if (m_engine->isOpen())
        m_engine->closePorts();

    QString err;
    if (!m_engine->openPorts(m_selectedInput, m_selectedOutput, &err)) {
        setStatus(err);
        m_connected = false;
        m_connectedName.clear();
        emit connectedChanged();
        return false;
    }

    const QString inName = m_inputs.at(m_selectedInput).name;
    m_connected = true;
    m_connectedName = inName;
    setStatus(QStringLiteral("Connected: %1").arg(inName));
    emit connectedChanged();
    startConnectionPoll();
    probeIdentity();
    return true;
}

bool MidiDeviceModel::autoConnectFa()
{
    refresh();
    preferFaSelection(true);
    if (!selectionValid()
        || !m_inputs.at(m_selectedInput).looksLikeFa
        || !m_outputs.at(m_selectedOutput).looksLikeFa) {
        setStatus(QStringLiteral("No FA MIDI port found (ignoring DAW CTRL)"));
        if (m_connected)
            disconnectDevice();
        return false;
    }
    return connectSelected();
}

void MidiDeviceModel::disconnectDevice()
{
    stopConnectionPoll();
    if (m_engine)
        m_engine->closePorts();
    const bool wasConnected = m_connected;
    m_connected = false;
    m_connectedName.clear();
    setStatus(QStringLiteral("Disconnected"));
    if (wasConnected)
        emit connectedChanged();
}

bool MidiDeviceModel::connectedPortStillPresent() const
{
    if (m_connectedName.isEmpty())
        return false;
    for (const auto &p : m_inputs) {
        if (p.name == m_connectedName)
            return true;
    }
    // CoreMIDI sometimes renames slightly; treat any FA port as still present.
    for (const auto &p : m_inputs) {
        if (p.looksLikeFa)
            return true;
    }
    for (const auto &p : m_outputs) {
        if (p.looksLikeFa)
            return true;
    }
    return false;
}

void MidiDeviceModel::startConnectionPoll()
{
    if (!m_connectionPoll.isActive())
        m_connectionPoll.start();
}

void MidiDeviceModel::stopConnectionPoll()
{
    m_connectionPoll.stop();
}

void MidiDeviceModel::pollConnection()
{
    if (!m_connected)
        return;

    // Enumerate ports without resetting the QML list model every tick.
    QStringList inNames;
    QStringList outNames;
    try {
        RtMidiIn in(RtMidi::MACOSX_CORE);
        RtMidiOut out(RtMidi::MACOSX_CORE);
        for (unsigned i = 0; i < in.getPortCount(); ++i)
            inNames << QString::fromStdString(in.getPortName(i));
        for (unsigned i = 0; i < out.getPortCount(); ++i)
            outNames << QString::fromStdString(out.getPortName(i));
    } catch (const RtMidiError &) {
        stopConnectionPoll();
        if (m_engine)
            m_engine->closePorts();
        m_connected = false;
        m_connectedName.clear();
        setStatus(QStringLiteral("FA disconnected — MIDI error"));
        emit connectedChanged();
        refresh();
        return;
    }

    const bool namePresent = inNames.contains(m_connectedName)
                             || std::any_of(inNames.cbegin(), inNames.cend(),
                                            [](const QString &n) { return nameLooksLikeFa(n); })
                             || std::any_of(outNames.cbegin(), outNames.cend(),
                                            [](const QString &n) { return nameLooksLikeFa(n); });
    const bool healthy = m_engine && m_engine->portsHealthy();

    if (!namePresent || !healthy) {
        stopConnectionPoll();
        if (m_engine)
            m_engine->closePorts();
        m_connected = false;
        m_connectedName.clear();
        setStatus(QStringLiteral("FA disconnected — cable unplugged or powered off"));
        emit connectedChanged();
        refresh();
    }
}

bool MidiDeviceModel::probeIdentity()
{
    if (!m_engine || !m_engine->isOpen())
        return false;
    QString err;
    if (!m_engine->sendIdentityRequest(&err)) {
        setStatus(err);
        return false;
    }
    quint8 dev = 0x10;
    if (m_engine->waitIdentityReply(&dev, 1500)) {
        m_engine->setDeviceId(dev);
        setStatus(QStringLiteral("FA Identity OK (device %1)").arg(dev, 2, 16, QChar('0')));
        return true;
    }
    setStatus(QStringLiteral("Connected (no Identity Reply — using device 0x10)"));
    m_engine->setDeviceId(0x10);
    return false;
}
