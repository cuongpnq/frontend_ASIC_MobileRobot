#include "application/GuiApplication.hpp"
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

    GuiApplication app(argc, argv);
    return app.exec();
}
