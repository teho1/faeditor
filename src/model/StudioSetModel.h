#pragma once

#include "model/PartModel.h"
#include "model/EffectsModel.h"

#include <QAbstractListModel>
#include <QVector>
#include <QJsonObject>
#include <functional>

class SysexEngine;
class UndoController;

class StudioSetModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(int selectedPart READ selectedPart WRITE setSelectedPart NOTIFY selectedPartChanged)
    Q_PROPERTY(PartModel *selectedPartModel READ selectedPartModel NOTIFY selectedPartChanged)
    Q_PROPERTY(EffectsModel *effects READ effects CONSTANT)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)
    Q_PROPERTY(int soloPart READ soloPart WRITE setSoloPart NOTIFY soloPartChanged)

public:
    enum Roles {
        PartRole = Qt::UserRole + 1,
        PartNumberRole,
        ToneNameRole,
        LevelRole,
        PanRole,
        MuteRole,
        SoloRole,
        BankMsbRole,
        BankLsbRole,
        ProgramRole,
        ChannelRole,
        ChorusSendRole,
        ReverbSendRole,
        OctaveRole,
        KeyLowRole,
        KeyHighRole,
        KeyboardSwitchRole,
        PartSwitchRole,
        OutputAssignRole
    };

    explicit StudioSetModel(SysexEngine *engine, UndoController *undo, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QString name() const { return m_name; }
    void setName(const QString &n);
    int selectedPart() const { return m_selectedPart; }
    void setSelectedPart(int v);
    PartModel *selectedPartModel() const;
    EffectsModel *effects() const { return m_effects; }
    bool busy() const { return m_busy; }
    QString lastError() const { return m_lastError; }
    bool dirty() const { return m_dirty; }
    int soloPart() const { return m_soloPart; }
    void setSoloPart(int v);

    PartModel *partAt(int index) const;
    Q_INVOKABLE PartModel *part(int index) const { return partAt(index); }

    Q_INVOKABLE bool pullFromDevice();
    Q_INVOKABLE bool pushToDevice();
    Q_INVOKABLE void markClean();

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &obj);

    void setToneNameResolver(const std::function<QString(int, int, int)> &fn);

signals:
    void nameChanged();
    void selectedPartChanged();
    void busyChanged();
    void lastErrorChanged();
    void dirtyChanged();
    void soloPartChanged();
    void studioSetLoaded();
    void autosaveRequested();

private slots:
    void onPartEdited(int partIndex, const QString &param, int value);
    void onEffectsEdited(const QString &section, const QString &param, int value);

private:
    void setBusy(bool v);
    void setError(const QString &e);
    void setDirty(bool v);
    void writePartParam(int partIndex, const QString &param, int value);
    void writeEffectParam(const QString &section, const QString &param, int value);
    void refreshToneNames();
    void notifyPartRow(int row);

    SysexEngine *m_engine = nullptr;
    UndoController *m_undo = nullptr;
    QVector<PartModel *> m_parts;
    EffectsModel *m_effects = nullptr;
    QString m_name = QStringLiteral("INIT STUDIOSET");
    int m_selectedPart = 0;
    int m_soloPart = 0; // 0 = off, 1-16
    bool m_busy = false;
    bool m_dirty = false;
    QString m_lastError;
    std::function<QString(int, int, int)> m_toneResolver;
    bool m_suppressUndo = false;
};
