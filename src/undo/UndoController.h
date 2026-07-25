#pragma once

#include <QObject>
#include <QUndoStack>
#include <QUndoCommand>
#include <QHash>

class StudioSetModel;

class UndoController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY changed)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY changed)
    Q_PROPERTY(QString undoText READ undoText NOTIFY changed)
    Q_PROPERTY(QString redoText READ redoText NOTIFY changed)

public:
    explicit UndoController(QObject *parent = nullptr);

    bool canUndo() const;
    bool canRedo() const;
    QString undoText() const;
    QString redoText() const;

    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    void clear();

    void pushPartChange(StudioSetModel *model, int partIndex, const QString &param, int newValue);
    void pushEffectChange(StudioSetModel *model, const QString &section, const QString &param, int newValue);

    QUndoStack *stack() { return &m_stack; }

signals:
    void changed();

private:
    QUndoStack m_stack;
    // track last known values for undo (simple approach: store old inside command at push time)
    QHash<QString, int> m_lastPartValues;
    QHash<QString, int> m_lastEffectValues;
};

class PartUndoCommand : public QUndoCommand
{
public:
    PartUndoCommand(StudioSetModel *model, int partIndex, QString param, int oldValue, int newValue);
    void undo() override;
    void redo() override;

private:
    StudioSetModel *m_model = nullptr;
    int m_partIndex = 0;
    QString m_param;
    int m_oldValue = 0;
    int m_newValue = 0;
    void apply(int value);
};

class EffectUndoCommand : public QUndoCommand
{
public:
    EffectUndoCommand(StudioSetModel *model, QString section, QString param, int oldValue, int newValue);
    void undo() override;
    void redo() override;

private:
    StudioSetModel *m_model = nullptr;
    QString m_section;
    QString m_param;
    int m_oldValue = 0;
    int m_newValue = 0;
    void apply(int value);
};
