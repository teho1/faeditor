#pragma once

#include <QAbstractListModel>
#include <QByteArray>

class InstrumentPlatform;
class StudioSetModel;

class SvdImportModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString sourceName READ sourceName NOTIFY sourceChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(int toneCount READ rowCount NOTIFY sourceChanged)

public:
    enum Role { NameRole = Qt::UserRole + 1, SlotRole, CategoryRole };

    explicit SvdImportModel(InstrumentPlatform *platform, StudioSetModel *studioSet,
                            QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString sourceName() const { return m_sourceName; }
    QString lastError() const { return m_lastError; }

    Q_INVOKABLE bool loadFile(const QUrl &url);
    Q_INVOKABLE bool pushTone(int row);

    static bool decodeSnAcoustic(const QByteArray &packed, QByteArray *common,
                                 QByteArray *mfx, QString *error = nullptr);

signals:
    void sourceChanged();
    void lastErrorChanged();
    void tonePushed(QString name, int partNumber);

private:
    struct Tone { QString name; QByteArray packed; };
    void setError(const QString &error);

    InstrumentPlatform *m_platform = nullptr;
    StudioSetModel *m_studioSet = nullptr;
    QVector<Tone> m_tones;
    QString m_sourceName;
    QString m_lastError;
};
