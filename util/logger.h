#pragma once

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QTextStream>

namespace QLog {
void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    // 1. 获取应用程序同级目录下的 logs 文件夹
    // 注意：在 main 函数执行前，QCoreApplication::instance() 可能为空
    // 但 applicationDirPath() 依然能通过系统接口拿到路径
    static QString logPath = QCoreApplication::applicationDirPath() + "/logs";

    static bool dirChecked = false;
    if(!dirChecked) {
        QDir logDir(logPath);
        if(!logDir.exists()) {
            logDir.mkpath(".");
        }
        dirChecked = true;
    }

    // 2. 文件名：logs/2026-03-03.log
    QString fileName =
        QString("%1/%2.log").arg(logPath, QDateTime::currentDateTime().toString("yyyy-MM-dd"));

    QFile file(fileName);
    // 使用 Text 模式处理换行符，Append 模式追加内容
    if(!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    // 3. 映射日志等级
    QString levelText;
    switch(type) {
    case QtDebugMsg:
        levelText = "DEBUG";
        break;
    case QtInfoMsg:
        levelText = "INFO ";
        break;
    case QtWarningMsg:
        levelText = "WARN ";
        break;
    case QtCriticalMsg:
        levelText = "ERROR";
        break;
    case QtFatalMsg:
        levelText = "FATAL";
        break;
    }

    // 4. 写入文件
    QTextStream out(&file);
    // 自动识别 context 中的信息（由 CMake 中的 -DQT_MESSAGELOGCONTEXT 提供）
    QString fileLine =
        context.file ? QString("%1:%2").arg(context.file).arg(context.line) : "Unknown";

    out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz ") << levelText << " ["
        << fileLine << "] " << msg << "\n";

    // 刷新缓冲区并输出到 IDE 控制台
    out.flush();
    fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
}
}
