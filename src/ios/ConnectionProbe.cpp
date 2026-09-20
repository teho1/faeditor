#include "ConnectionProbe.h"
#include "midi/SysexEngine.h"
#include "platform/AutoDetectRolandPlatform.h"
#include <QtConcurrent>
#include <QGuiApplication>
#include <QClipboard>
#include <QDateTime>
#include <QSysInfo>

ConnectionProbe::ConnectionProbe(QObject *parent) : QObject(parent) {
    connect(&m_watcher, &QFutureWatcher<QString>::finished, this, [this] {
        append(m_watcher.result());
        m_busy = false;
        emit busyChanged();
    });
    append("FA Connection Test 0.1 — " + QSysInfo::prettyProductName());
    refresh();
}
ConnectionProbe::~ConnectionProbe() { m_watcher.waitForFinished(); }
void ConnectionProbe::append(const QString &text) {
    m_log += QDateTime::currentDateTime().toString("hh:mm:ss ") + text + "\n";
    if (m_log.size() > 24000) m_log = m_log.right(24000);
    emit logChanged();
}
void ConnectionProbe::copyLog() { QGuiApplication::clipboard()->setText(m_log); }
void ConnectionProbe::refresh() {
    if (m_busy) return;
    SysexEngine engine;
    AutoDetectRolandPlatform platform(&engine);
    QVector<InstrumentPlatform::MidiPort> ins, outs;
    QString error;
    m_inputs.clear(); m_outputs.clear();
    if (!platform.discoverMidiPorts(&ins, &outs, &error)) append("Discovery failed: " + error);
    for (const auto &p : ins) m_inputs << p.name;
    for (const auto &p : outs) m_outputs << p.name;
    emit portsChanged();
    append(QString("Found %1 inputs, %2 outputs.").arg(ins.size()).arg(outs.size()));
    append("Inputs: " + m_inputs.join(" | ") + "\nOutputs: " + m_outputs.join(" | "));
}
void ConnectionProbe::test(int input, int output) {
    if (m_busy) return;
    if (input < 0 || input >= m_inputs.size() || output < 0 || output >= m_outputs.size()) {
        append("Select an input and output first."); return;
    }
    const QString inputName = m_inputs[input], outputName = m_outputs[output];
    m_busy = true; emit busyChanged();
    append("Testing " + inputName + " → " + outputName + " …");
    m_watcher.setFuture(QtConcurrent::run([input, output, inputName, outputName]() -> QString {
        SysexEngine engine;
        AutoDetectRolandPlatform platform(&engine);
        QVector<InstrumentPlatform::MidiPort> ins, outs;
        QString error;
        if (!platform.discoverMidiPorts(&ins, &outs, &error)) return "Discovery failed: " + error;
        if (input >= ins.size() || output >= outs.size() || ins[input].name != inputName || outs[output].name != outputName)
            return "Ports changed. Refresh and select the ports again.";
        if (!platform.openMidiConnection(input, output, &error)) return "Open failed: " + error;
        quint8 deviceId = 0x10;
        if (!platform.detectDevice(&deviceId, 2500, &error))
            return "Ports opened, but identity failed: " + error + "\nCheck USB GENERIC mode, MIDI port selection and cable; then retry.";
        QString result = "Identity OK: " + platform.profile().model
            + QString(" (device 0x%1)\n").arg(deviceId, 2, 16, QChar('0'))
            + "Reply: " + QString::fromLatin1(engine.lastIdentityReply().toHex(' ')) + "\n";
        QString name;
        if (!platform.readTemporaryPerformanceName(&name, &error))
            return result + "SysEx name read FAILED: " + error;
        return result + "PASS — bidirectional SysEx works.\nCurrent set/scene: " + name
            + "\nPorts closed. No instrument settings changed.";
    }));
}
