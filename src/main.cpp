#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>
#include <QFile>
#include <QCoreApplication>

#include "app/AppController.h"
#include "model/PartModel.h"
#include "model/EffectsModel.h"
#include "model/StudioSetModel.h"
#include "model/ToneBrowserModel.h"
#include "model/StudioSetBrowserModel.h"
#include "model/AudioFxModel.h"
#include "model/MfxModel.h"
#include "model/MfxTapDelayModel.h"
#include "model/MfxParamListModel.h"
#include "model/MfxUiHelpers.h"
#include "midi/MidiDeviceModel.h"
#include "project/ProjectStore.h"
#include "undo/UndoController.h"

#include <qqml.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("FAEditor"));
    app.setOrganizationDomain(QStringLiteral("faeditor.local"));
    app.setApplicationName(QStringLiteral("FA Editor"));
    app.setApplicationVersion(QStringLiteral("1.1"));

    // Prefer bundle .icns (Dock/Finder), fall back to embedded PNG.
    QIcon appIcon;
#if defined(Q_OS_MACOS)
    {
        const QString icns = QCoreApplication::applicationDirPath()
                             + QStringLiteral("/../Resources/AppIcon.icns");
        if (QFile::exists(icns))
            appIcon.addFile(icns);
    }
#endif
    if (appIcon.isNull())
        appIcon.addFile(QStringLiteral(":/qt/qml/FAEditor/resources/icons/appicon.png"));
    if (!appIcon.isNull())
        app.setWindowIcon(appIcon);

    // Qt Quick Controls 2: prefer native macOS style when available.
#if defined(Q_OS_MACOS)
    QQuickStyle::setStyle(QStringLiteral("macOS"));
#else
    QQuickStyle::setStyle(QStringLiteral("Fusion"));
#endif

    qmlRegisterUncreatableType<PartModel>("FAEditor", 1, 0, "PartModel",
                                          QStringLiteral("Obtained from StudioSetModel"));
    qmlRegisterUncreatableType<EffectsModel>("FAEditor", 1, 0, "EffectsModel",
                                             QStringLiteral("Obtained from StudioSetModel"));
    qmlRegisterUncreatableType<StudioSetModel>("FAEditor", 1, 0, "StudioSetModel",
                                               QStringLiteral("Obtained from AppController"));
    qmlRegisterUncreatableType<ToneBrowserModel>("FAEditor", 1, 0, "ToneBrowserModel",
                                                 QStringLiteral("Obtained from AppController"));
    qmlRegisterUncreatableType<StudioSetBrowserModel>("FAEditor", 1, 0, "StudioSetBrowserModel",
                                                      QStringLiteral("Obtained from AppController"));
    qmlRegisterUncreatableType<AudioFxModel>("FAEditor", 1, 0, "AudioFxModel",
                                             QStringLiteral("Obtained from AppController"));
    qmlRegisterUncreatableType<MidiDeviceModel>("FAEditor", 1, 0, "MidiDeviceModel",
                                                QStringLiteral("Obtained from AppController"));
    qmlRegisterUncreatableType<ProjectStore>("FAEditor", 1, 0, "ProjectStore",
                                             QStringLiteral("Obtained from AppController"));
    qmlRegisterUncreatableType<UndoController>("FAEditor", 1, 0, "UndoController",
                                               QStringLiteral("Obtained from AppController"));
    qmlRegisterUncreatableType<AppController>("FAEditor", 1, 0, "AppController",
                                              QStringLiteral("Use App context property"));
    qmlRegisterUncreatableType<MfxModel>("FAEditor", 1, 0, "MfxModel",
                                         QStringLiteral("Obtained from TemporaryToneModel"));
    qmlRegisterUncreatableType<MfxTapDelayModel>("FAEditor", 1, 0, "MfxTapDelayModel",
                                                 QStringLiteral("Obtained from MfxModel.tapDelay"));
    qmlRegisterUncreatableType<MfxParamListModel>("FAEditor", 1, 0, "MfxParamListModel",
                                                  QStringLiteral("Obtained from MfxModel.paramList"));
    qmlRegisterSingletonInstance("FAEditor", 1, 0, "MfxUiFamily", MfxUiHelpers::instance());

    AppController controller;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("App"), &controller);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("FAEditor"), QStringLiteral("Main"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
