#include "application/GuiApplication.hpp"

int main(int argc, char *argv[])
{
    GuiApplication app(argc, argv);
    return app.exec();
}
