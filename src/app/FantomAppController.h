#pragma once

#include "platform/RolandFantomPlatform.h"

#include <QObject>
#include <QString>

// Product shell controller only. Device operations remain unavailable until a
// verified Fantom protocol adapter is implemented.
class FantomAppController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString productName READ productName CONSTANT)
    Q_PROPERTY(QString deviceProfile READ deviceProfile CONSTANT)
    Q_PROPERTY(QString implementationStatus READ implementationStatus CONSTANT)
    Q_PROPERTY(bool deviceEditingAvailable READ deviceEditingAvailable CONSTANT)
    Q_PROPERTY(bool localLibraryAvailable READ localLibraryAvailable CONSTANT)

public:
    explicit FantomAppController(QObject *parent = nullptr) : QObject(parent) {}

    QString productName() const { return QStringLiteral("Fantom Editor"); }
    QString deviceProfile() const { return m_platform.profile().model; }
    QString implementationStatus() const
    {
        return QStringLiteral("Device-specific Fantom editing is not implemented yet. No MIDI or SysEx is sent.");
    }
    bool deviceEditingAvailable() const
    {
        return m_platform.supportsWorkspace(InstrumentPlatform::Workspace::ToneEditing, true);
    }
    bool localLibraryAvailable() const
    {
        return m_platform.supportsWorkspace(InstrumentPlatform::Workspace::Library, true);
    }

private:
    RolandFantomPlatform m_platform;
};
