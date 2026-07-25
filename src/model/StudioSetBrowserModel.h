#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QTimer>

class SysexEngine;
class StudioSetModel;

struct StudioSetSlot {
    enum class Kind { User, Preset };
    Kind kind = Kind::User;
    int number = 1; // 1-based User 1..512 or Preset 1..69
    QString name;   // empty until scanned / loaded
    int bankMsb = 85;
    int bankLsb = 0;
    int program = 0; // 0-based PC
};

class StudioSetBrowserModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterChanged)
    Q_PROPERTY(QString groupFilter READ groupFilter WRITE setGroupFilter NOTIFY filterChanged)
    Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentChanged)
    Q_PROPERTY(QString currentLabel READ currentLabel NOTIFY currentChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(int scanProgress READ scanProgress NOTIFY scanningChanged)
    Q_PROPERTY(int scanTotal READ scanTotal NOTIFY scanningChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)

public:
    enum Roles {
        LabelRole = Qt::UserRole + 1,
        NameRole,
        KindRole,
        NumberRole,
        SlotIdRole,
        HasNameRole
    };

    explicit StudioSetBrowserModel(SysexEngine *engine, StudioSetModel *studioSet,
                                   QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString filterText() const { return m_filterText; }
    void setFilterText(const QString &t);
    QString groupFilter() const { return m_groupFilter; }
    void setGroupFilter(const QString &g);
    int currentRow() const { return m_currentRow; }
    void setCurrentRow(int row);
    QString currentLabel() const;
    bool scanning() const { return m_scanning; }
    int scanProgress() const { return m_scanProgress; }
    int scanTotal() const { return m_scanTotal; }
    QString statusText() const { return m_statusText; }

    Q_INVOKABLE bool recallRow(int row, bool pullAfter = true);
    Q_INVOKABLE bool recallCurrent(bool pullAfter = true);
    Q_INVOKABLE void startScanNames(bool userOnly = false);
    Q_INVOKABLE void cancelScan();
    Q_INVOKABLE QVariantMap slotAt(int row) const;

signals:
    void filterChanged();
    void currentChanged();
    void scanningChanged();
    void statusTextChanged();
    void recalled(int row);

private:
    void rebuildFiltered();
    void setStatus(const QString &s);
    void buildSlots();
    bool writeSetupSelect(const StudioSetSlot &slot, QString *error);
    bool readTemporaryName(QString *outName, QString *error);
    void scanStep();
    int absoluteIndexFromFiltered(int row) const;

    SysexEngine *m_engine = nullptr;
    StudioSetModel *m_studioSet = nullptr;
    QVector<StudioSetSlot> m_all;
    QVector<int> m_filtered;
    QString m_filterText;
    QString m_groupFilter = QStringLiteral("All"); // All | User | Preset
    int m_currentRow = -1;
    bool m_scanning = false;
    int m_scanProgress = 0;
    int m_scanTotal = 0;
    int m_scanIndex = 0;
    QVector<int> m_scanQueue;
    QString m_statusText;
    QTimer m_scanTimer;
};
