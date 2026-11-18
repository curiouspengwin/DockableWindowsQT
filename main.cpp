#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "tabmodel.h"
#include "tabfilterproxy.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);

    // Register C++ types for QML (Qt 5.15 syntax)
    qmlRegisterType<TabModel>("BrowserBackend", 1, 0, "TabModel");
    qmlRegisterType<TabFilterProxy>("BrowserBackend", 1, 0, "TabFilterProxy");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return QGuiApplication::exec();
}
