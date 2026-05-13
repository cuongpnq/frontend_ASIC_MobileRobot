#pragma once

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <memory>
#include <QObject>
#include <QEvent>

class UserInteractionMonitor : public QObject {
    Q_OBJECT
public:
    explicit UserInteractionMonitor(QObject *parent = nullptr) : QObject(parent) {}

signals:
    void interacted();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::KeyPress ||
            event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::TouchBegin ||
            event->type() == QEvent::Wheel) {
            emit interacted();
        }
        return QObject::eventFilter(obj, event);
    }
};

class GuiApplication {
public:
    GuiApplication(int &argc, char **argv);
    ~GuiApplication() = default;

    int exec();

private:
    std::unique_ptr<QGuiApplication> app;
    std::unique_ptr<QQmlApplicationEngine> engine;
};
