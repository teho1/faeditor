#pragma once

#include "midi/AddressMap.h"

#include <QObject>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>
#include <memory>
#include <atomic>

class RtMidiIn;
class RtMidiOut;

class SysexEngine : public QObject
{
    Q_OBJECT
public:
    explicit SysexEngine(QObject *parent = nullptr);
    ~SysexEngine() override;

    bool openPorts(int inIndex, int outIndex, QString *error = nullptr);
    void closePorts();
    bool isOpen() const;

    void setDeviceId(quint8 id);
    quint8 deviceId() const { return m_deviceId; }

    /** Blocking RQ1 with timeout; returns DT1 payload (data only) on success. */
    bool read(const roland::Address &address, int size, QByteArray *outData, QString *error = nullptr,
              int timeoutMs = 3000);

    /** Write DT1, splitting into <=256 byte packets with inter-packet delay. */
    bool write(const roland::Address &address, const QByteArray &data, QString *error = nullptr);

    /** Single-byte (or short) parameter write. */
    bool writeParam(const roland::Address &address, const QByteArray &data, QString *error = nullptr);

    bool sendIdentityRequest(QString *error = nullptr);
    /** Wait for Identity Reply; returns true if Roland FA signature seen. */
    bool waitIdentityReply(quint8 *deviceIdOut, int timeoutMs = 2000);

    /** Send raw MIDI (e.g. note on/off for tone preview). */
    bool sendMessage(const QByteArray &message, QString *error = nullptr);

signals:
    void sysexReceived(const QByteArray &message);
    void errorOccurred(const QString &message);

private:
    void onMidiMessage(double deltaTime, std::vector<unsigned char> *message);
    static void rtMidiCallback(double deltaTime, std::vector<unsigned char> *message, void *userData);

    QByteArray buildRq1(const roland::Address &address, int size) const;
    QByteArray buildDt1(const roland::Address &address, const QByteArray &data) const;
    bool sendRaw(const QByteArray &msg, QString *error);

    std::unique_ptr<RtMidiIn> m_in;
    std::unique_ptr<RtMidiOut> m_out;
    quint8 m_deviceId = roland::kDefaultDeviceId;

    mutable QMutex m_mutex;
    QWaitCondition m_cond;
    QByteArray m_pendingDt1Data;
    roland::Address m_pendingAddress{};
    bool m_gotDt1 = false;
    bool m_gotIdentity = false;
    quint8 m_identityDeviceId = roland::kDefaultDeviceId;
    std::atomic<bool> m_open{false};
};
