#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QVector>

/** Roland FA Sound List waveform names (SN-S PCM + PCM Synth INT-A/B) + filterable browser model. */
class WaveformCatalog : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool loaded READ loaded NOTIFY catalogChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(int snCount READ snCount NOTIFY catalogChanged)
    Q_PROPERTY(int pcmIntACount READ pcmIntACount NOTIFY catalogChanged)
    Q_PROPERTY(int pcmIntBCount READ pcmIntBCount NOTIFY catalogChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    /** Browser bank: "sn" | "intA" | "intB" | "srx". */
    Q_PROPERTY(QString bank READ bank WRITE setBank NOTIFY filterChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterChanged)
    Q_PROPERTY(bool bankHasNames READ bankHasNames NOTIFY filterChanged)
    Q_PROPERTY(QString bankDisplayName READ bankDisplayName NOTIFY filterChanged)
    Q_PROPERTY(int filteredCount READ filteredCount NOTIFY filterChanged)

public:
    enum Roles {
        NumberRole = Qt::UserRole + 1,
        NameRole
    };

    explicit WaveformCatalog(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool loaded() const { return m_loaded; }
    bool busy() const { return m_busy; }
    int snCount() const { return m_sn.size(); }
    int pcmIntACount() const { return m_pcmIntA.size(); }
    int pcmIntBCount() const { return m_pcmIntB.size(); }
    QString statusText() const { return m_statusText; }
    QString lastError() const { return m_lastError; }

    QString bank() const { return m_bank; }
    void setBank(const QString &b);
    QString filterText() const { return m_filterText; }
    void setFilterText(const QString &t);
    bool bankHasNames() const;
    QString bankDisplayName() const;
    int filteredCount() const { return m_filtered.size(); }

    /** Load bundled Sound List waveforms (default resource path). */
    Q_INVOKABLE bool loadCatalog(const QString &path = {});
    /** Explicit user action — same as loadCatalog from the bundled resource. */
    Q_INVOKABLE bool fetchWaveformNames();

    /** Raw name for SN-S PCM wave number (1-based); empty if unknown. 0 → empty. */
    Q_INVOKABLE QString resolveSnName(int waveNumber) const;
    /** Raw name for PCM Synth INT-A/B; empty for SRX/Ex/unknown. */
    Q_INVOKABLE QString resolvePcmName(int waveGroupType, int waveGroupId, int waveNumber) const;

    /** Display helper: "OFF", name, or fallback "#n". */
    Q_INVOKABLE QString displaySn(int waveNumber) const;
    Q_INVOKABLE QString displayPcm(int waveGroupType, int waveGroupId, int waveNumber) const;
    /** "INT-A" / "INT-B" / "SRX" / "INT" / "—". */
    Q_INVOKABLE QString pcmBankLabel(int waveGroupType, int waveGroupId) const;

    /** Filtered-list row for wave number, or -1 if not visible. */
    Q_INVOKABLE int indexOfWave(int waveNumber) const;
    Q_INVOKABLE QVariantMap waveAt(int row) const;

signals:
    void catalogChanged();
    void busyChanged();
    void statusTextChanged();
    void lastErrorChanged();
    void filterChanged();

private:
    struct WaveEntry {
        int number = 0;
        QString name;
    };

    enum class PcmBank { Unknown, IntA, IntB, Srx };

    PcmBank pcmBank(int waveGroupType, int waveGroupId) const;
    void setStatus(const QString &s);
    void setError(const QString &e);
    void setBusy(bool v);
    void rebuildSortedTables();
    void rebuildFiltered();
    const QVector<WaveEntry> *activeTable() const;
    static QVector<WaveEntry> sortedEntries(const QHash<int, QString> &table);

    bool m_loaded = false;
    bool m_busy = false;
    QString m_statusText;
    QString m_lastError;
    QHash<int, QString> m_sn;
    QHash<int, QString> m_pcmIntA;
    QHash<int, QString> m_pcmIntB;

    QVector<WaveEntry> m_snList;
    QVector<WaveEntry> m_pcmIntAList;
    QVector<WaveEntry> m_pcmIntBList;
    QVector<WaveEntry> m_filtered;

    QString m_bank = QStringLiteral("sn");
    QString m_filterText;
};
