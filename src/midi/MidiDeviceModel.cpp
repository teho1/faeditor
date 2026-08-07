#include "midi/MidiDeviceModel.h"
#include <algorithm>

MidiDeviceModel::MidiDeviceModel(InstrumentPlatform *platform, QObject *parent)
    : QAbstractListModel(parent)
    , m_platform(platform)
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
    QString error;
    if (!m_platform || !m_platform->supportsWorkspace(InstrumentPlatform::Workspace::MidiConnection))
        setStatus(QStringLiteral("MIDI connection is not supported by this instrument"));
    else if (!m_platform->discoverMidiPorts(&m_inputs,&m_outputs,&error)) setStatus(error);
    endResetModel();

    // If we thought we were connected but the engine/ports are gone (FA power-cycled), clear flag.
    if (m_connected && (!m_platform || !m_platform->isConnected() || !connectedPortStillPresent())) {
        stopConnectionPoll();
        if (m_platform && m_platform->isConnected()) m_platform->closeMidiConnection();
        m_connected = false;
        m_connectedName.clear();
        setStatus(QStringLiteral("Instrument disconnected — cable unplugged or powered off"));
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
    if (!m_platform || !m_platform->supportsWorkspace(InstrumentPlatform::Workspace::MidiConnection, true)) {
        setStatus(QStringLiteral("MIDI connection is not supported by this instrument"));
        return false;
    }

    refresh();
    if (!selectionValid()) {
        setStatus(QStringLiteral("Select MIDI in/out ports"));
        return false;
    }

    // Re-open cleanly (important after FA reboot / CoreMIDI renumber)
    if (m_platform->isConnected()) m_platform->closeMidiConnection();

    QString err;
    if (!m_platform->openMidiConnection(m_selectedInput, m_selectedOutput, &err)) {
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
    if (!m_platform || !m_platform->supportsWorkspace(InstrumentPlatform::Workspace::MidiConnection, true)) {
        setStatus(QStringLiteral("MIDI connection is not supported by this instrument"));
        return false;
    }
    refresh();
    preferFaSelection(true);
    if (!selectionValid()
        || !m_inputs.at(m_selectedInput).looksLikeFa
        || !m_outputs.at(m_selectedOutput).looksLikeFa) {
        setStatus(QStringLiteral("No matching instrument MIDI port found (ignoring DAW CTRL)"));
        if (m_connected)
            disconnectDevice();
        return false;
    }
    return connectSelected();
}

void MidiDeviceModel::disconnectDevice()
{
    stopConnectionPoll();
    if (m_platform) m_platform->closeMidiConnection();
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

    QVector<MidiPortInfo> inputs,outputs; QString discoveryError;
    if (!m_platform || !m_platform->discoverMidiPorts(&inputs,&outputs,&discoveryError)) {
        stopConnectionPoll();
        if (m_platform) m_platform->closeMidiConnection();
        m_connected = false;
        m_connectedName.clear();
        setStatus(QStringLiteral("Instrument disconnected — MIDI error"));
        emit connectedChanged();
        refresh();
        return;
    }

    const bool namePresent = std::any_of(inputs.cbegin(),inputs.cend(),[this](const auto&p){return p.name==m_connectedName||p.looksLikeFa;})
                             || std::any_of(outputs.cbegin(),outputs.cend(),[](const auto&p){return p.looksLikeFa;});
    const bool healthy = m_platform && m_platform->midiConnectionHealthy();

    if (!namePresent || !healthy) {
        stopConnectionPoll();
        if (m_platform) m_platform->closeMidiConnection();
        m_connected = false;
        m_connectedName.clear();
        setStatus(QStringLiteral("Instrument disconnected — cable unplugged or powered off"));
        emit connectedChanged();
        refresh();
    }
}

bool MidiDeviceModel::probeIdentity()
{
    if (!m_platform || !m_platform->supportsWorkspace(InstrumentPlatform::Workspace::DeviceIdentity)) {
        setStatus(QStringLiteral("Device identity is not supported by this instrument"));
        return false;
    }
    if (!m_platform || !m_platform->isConnected())
        return false;
    QString err;
    quint8 dev = 0x10;
    if (m_platform->detectDevice(&dev,1500,&err)) {
        setStatus(QStringLiteral("%1 Identity OK (device %2)").arg(m_platform->profile().model).arg(dev, 2, 16, QChar('0')));
        return true;
    }
    if (!err.isEmpty()) {
        setStatus(err);
        return false;
    }
    setStatus(QStringLiteral("Connected (no Identity Reply — using device 0x10)"));
    return false;
}
