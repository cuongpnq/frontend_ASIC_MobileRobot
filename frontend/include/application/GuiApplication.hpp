#pragma once

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <memory>

class GuiApplication {
public:
    GuiApplication(int &argc, char **argv);
    ~GuiApplication() = default;

    int exec();

private:
    std::unique_ptr<QGuiApplication> app;
    std::unique_ptr<QQmlApplicationEngine> engine;
};
