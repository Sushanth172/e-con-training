#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "application.h"
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;


    engine.rootContext()->setContextProperty("device",&Application::deviceNameModel);
    engine.rootContext()->setContextProperty("format",&Application::formatTypeModel);
    engine.rootContext()->setContextProperty("resolution",&Application::resolutionModel);
    engine.rootContext()->setContextProperty("fps",&Application::fpsModel);

    qmlRegisterType<Application>("qml.components.sample", 1, 0, "Devices");
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}



