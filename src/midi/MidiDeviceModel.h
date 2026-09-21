#pragma once

#include "platform/InstrumentPlatform.h"

#include <QAbstractListModel>
#include <QStringList>
#include <QTimer>
#include <memory>

using MidiPortInfo = InstrumentPlatform::MidiPort;

class MidiDeviceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString connectedName READ connectedName NOTIFY connectedChanged)
    Q_PROPERTY(int inputPortCount READ inputPortCount NOTIFY portsChanged)
    Q_PROPERTY(int outputPortCount READ outputPortCount NOTIFY portsChanged)
    Q_PROPERTY(QStringList inputPortNames READ inputNames NOTIFY portsChanged)
    Q_PROPERTY(QStringList outputPortNames READ outputNames NOTIFY portsChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(int selectedInput READ selectedInput WRITE setSelectedInput NOTIFY selectionChanged)
    Q_PROPERTY(int selectedOutput READ selectedOutput WRITE setSelectedOutput NOTIFY selectionChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        IndexRole,
        IsDawControlRole,
        LooksLikeFaRole
    };

    explicit MidiDeviceModel(InstrumentPlatform *platform, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool connected() const { return m_connected; }
    QString connectedName() const { return m_connectedName; }
    int inputPortCount() const { return m_inputs.size(); }
    int outputPortCount() const { return m_outputs.size(); }
    QString statusText() const { return m_statusText; }
    int selectedInput() const { return m_selectedInput; }
    int selectedOutput() const { return m_selectedOutput; }
    void setSelectedInput(int v);
    void setSelectedOutput(int v);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QStringList inputNames() const;
    Q_INVOKABLE QStringList outputNames() const;
    Q_INVOKABLE bool connectSelected();
    Q_INVOKABLE bool autoConnectFa();
    Q_INVOKABLE bool autoConnectInstrument() { return autoConnectFa(); }
    Q_INVOKABLE void disconnectDevice();
    /** Show MIDI as connected without opening ports (store screenshots). */
    void setPreviewConnected(const QString &name);
    Q_INVOKABLE bool probeIdentity();
    /** True if current selection still refers to valid ports. */
    Q_INVOKABLE bool selectionValid() const;

signals:
    void connectedChanged();
    void portsChanged();
    void statusTextChanged();
    void selectionChanged();

private:
    void setStatus(const QString &text);
    void preferFaSelection(bool force = false);
    bool indexInRange(int idx, int count) const;
    void startConnectionPoll();
    void stopConnectionPoll();
    bool connectedPortStillPresent() const;
    bool connectWithCurrentSelection(bool resetHost);

    InstrumentPlatform *m_platform = nullptr;
    QVector<MidiPortInfo> m_inputs;
    QVector<MidiPortInfo> m_outputs;
    bool m_connected = false;
    bool m_previewConnected = false;
    QString m_connectedName;
    QString m_statusText = QStringLiteral("Disconnected");
    int m_selectedInput = -1;
    int m_selectedOutput = -1;
    QTimer m_connectionPoll;
    int m_unhealthyPolls = 0;
    qint64 m_pollGraceUntilMs = 0;

private slots:
    void pollConnection();
};
