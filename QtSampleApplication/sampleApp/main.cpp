#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "sample.h"
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;


    engine.rootContext()->setContextProperty("device",&Sample::deviceNameModel);
    engine.rootContext()->setContextProperty("format",&Sample::formatModel);
    engine.rootContext()->setContextProperty("resolution",&Sample::resolutionModel);
    engine.rootContext()->setContextProperty("fps",&Sample::fpsModel);

    qmlRegisterType<Sample>("qml.components.sample", 1, 0, "Devices");
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    //    Sample s;
    //    s.callFunc();

    return app.exec();
}



