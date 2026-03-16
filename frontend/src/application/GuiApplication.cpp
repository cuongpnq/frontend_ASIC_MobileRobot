#include "application/GuiApplication.hpp"
#include "application/AppStateMachine.hpp"
#include "mainView/MainViewViewModel.hpp"
#include "containerBar/ContainerBarViewModel.hpp"
#include "runningView/RunningViewViewModel.hpp"
#include <QQmlContext>
#include <QQmlEngine>

GuiApplication::GuiApplication(int &argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    app = std::make_unique<QGuiApplication>(argc, argv);
    engine = std::make_unique<QQmlApplicationEngine>();

    qmlRegisterSingletonType<MainViewViewModel>("com.asic.mobilerobot.viewmodels", 1, 0, "MainViewViewModel",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return new MainViewViewModel();
        });

    qmlRegisterSingletonType<ContainerBarViewModel>("com.asic.mobilerobot.viewmodels", 1, 0, "ContainerBarViewModel",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return new ContainerBarViewModel();
        });

    qmlRegisterSingletonType<RunningViewViewModel>("com.asic.mobilerobot.viewmodels", 1, 0, "RunningViewViewModel",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return new RunningViewViewModel();
        });

    // Provide the AppStateMachine singleton to QML as a context property to listen to state
    engine->rootContext()->setContextProperty("AppStateMachine", &AppStateMachine::instance());

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    
    QObject::connect(engine.get(), &QQmlApplicationEngine::objectCreated,
                     app.get(), [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    
    engine->load(url);
}

int GuiApplication::exec()
{
    return app->exec();
}
