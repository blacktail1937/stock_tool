#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "util/logger.h"

#ifdef _WIN32
    #include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // 设置控制台输出为 UTF-8
    SetConsoleOutputCP(CP_UTF8);
#endif

    qInstallMessageHandler(QLog::messageHandler);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(EXIT_FAILURE); },
        Qt::QueuedConnection);
    engine.loadFromModule("stock_tool", "Main");

    ::_exit(app.exec());
}
