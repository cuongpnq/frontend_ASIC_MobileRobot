#include "application/GuiApplication.hpp"
#include "application/SessionLogger.hpp"
#include <signal.h>
#include <unistd.h>
#include <QDebug>

// Unix signal handler for Ctrl+C
void signalHandler(int sig) {
    qDebug() << "Signal received:" << sig << ". Quitting...";
    QCoreApplication::quit();
}

int main(int argc, char *argv[])
{
#ifdef HAS_VIRTUAL_KEYBOARD
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    qputenv("QT_VIRTUALKEYBOARD_STYLE", QByteArray("retro"));
#endif

    // Register signals
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Capture Qt warnings/criticals into the session log
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
        // Always print to stderr as well
        const char* severity = "DEBUG";
        if      (type == QtWarningMsg)  severity = "WARN";
        else if (type == QtCriticalMsg) severity = "CRITICAL";
        else if (type == QtFatalMsg)    severity = "FATAL";
        fprintf(stderr, "[%s] %s\n", severity, msg.toLocal8Bit().constData());

        // Only forward warnings and above to the session log
        if (type >= QtWarningMsg) {
            SessionLogger::instance().logEvent(
                QStringLiteral("error"),
                QStringLiteral("qt_message"),
                { { QStringLiteral("severity"), QString::fromLatin1(severity) },
                  { QStringLiteral("msg"),      msg } });
        }

        if (type == QtFatalMsg)
            abort();
    });

    GuiApplication app(argc, argv);
    return app.exec();
}
