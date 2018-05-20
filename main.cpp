#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "butlerdconnection.h"

int main(int argc, char *argv[])
{
    qmlRegisterSingletonType<ButlerdConnection>("Qt.itch.butlerd", 1, 0, "ButlerdConnection", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)

        ButlerdConnection *conn = new ButlerdConnection();
        return conn;
    });

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
