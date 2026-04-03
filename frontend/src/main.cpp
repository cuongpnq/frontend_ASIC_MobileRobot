#include "application/GuiApplication.hpp"

int main(int argc, char *argv[])
{
#ifdef HAS_VIRTUAL_KEYBOARD
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    qputenv("QT_VIRTUALKEYBOARD_STYLE", QByteArray("retro"));
#endif

    GuiApplication app(argc, argv);
    return app.exec();
}
