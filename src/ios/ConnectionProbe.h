#pragma once
#include <QObject>
#include <QStringList>
#include <QFutureWatcher>

class ConnectionProbe : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList inputs READ inputs NOTIFY portsChanged)
    Q_PROPERTY(QStringList outputs READ outputs NOTIFY portsChanged)
    Q_PROPERTY(QString log READ log NOTIFY logChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
public:
    explicit ConnectionProbe(QObject *parent = nullptr);
    ~ConnectionProbe() override;
    QStringList inputs() const { return m_inputs; }
    QStringList outputs() const { return m_outputs; }
    QString log() const { return m_log; }
    bool busy() const { return m_busy; }
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void test(int input, int output);
    Q_INVOKABLE void copyLog();
signals:
    void portsChanged();
    void logChanged();
    void busyChanged();
private:
    void append(const QString &text);
    QStringList m_inputs, m_outputs;
    QString m_log;
    bool m_busy = false;
    QFutureWatcher<QString> m_watcher;
};
