#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "ConnectionProbe.h"
int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("FA Connection Test");
    app.setOrganizationName("Righthere");
    QQuickStyle::setStyle("Fusion");
    ConnectionProbe probe;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("Probe", &probe);
    engine.loadFromModule("FAConnectionTest", "ConnectionTest");
    if (engine.rootObjects().isEmpty()) return 1;
    return app.exec();
}
