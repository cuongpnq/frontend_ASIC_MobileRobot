#include "application/GuiApplication.hpp"
#include "application/AppStateMachine.hpp"
#include "mainView/MainViewViewModel.hpp"
#include "containerBar/ContainerBarViewModel.hpp"
#include "runningView/RunningViewViewModel.hpp"
#include "modeSwitch/ModeSwitchViewModel.hpp"
#include "controlCenterView/ControlCenterViewViewModel.hpp"
#include "directionView/DirectionViewViewModel.hpp"
#include "settingsView/SettingsViewViewModel.hpp"
#include "diagnosticsView/DiagnosticsViewViewModel.hpp"
#include "mapPanelView/MapPanelViewViewModel.hpp"
#include "chatView/ChatViewViewModel.hpp"
#include "wifiSettingView/WifiSettingViewViewModel.hpp"
#include "wifiManager/WifiManager.hpp"
#include <QQmlContext>
#include <QQmlEngine>

GuiApplication::GuiApplication(int &argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    app = std::make_unique<QGuiApplication>(argc, argv);
    engine = std::make_unique<QQmlApplicationEngine>();

#ifdef HAS_VIRTUAL_KEYBOARD
    engine->rootContext()->setContextProperty("HAS_VIRTUAL_KEYBOARD", true);
#else
    engine->rootContext()->setContextProperty("HAS_VIRTUAL_KEYBOARD", false);
#endif

    qmlRegisterSingletonType<ModeSwitchViewModel>("com.asic.mobilerobot.viewmodels", 1, 0, "ModeSwitchViewModel",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return new ModeSwitchViewModel();
        });

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

    qmlRegisterSingletonType<ControlCenterViewViewModel>("com.asic.mobilerobot.viewmodels", 1, 0, "ControlCenterViewViewModel",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return new ControlCenterViewViewModel();
        });

    qmlRegisterSingletonType<DirectionViewViewModel>("com.asic.mobilerobot.viewmodels", 1, 0, "DirectionViewViewModel",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return new DirectionViewViewModel();
        });

    qmlRegisterSingletonType<SettingsViewViewModel>("com.asic.mobilerobot.viewmodels", 1, 0, "SettingsViewViewModel",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return new SettingsViewViewModel();
        });

    qmlRegisterSingletonType<DiagnosticsViewViewModel>("com.asic.mobilerobot.viewmodels", 1, 0, "DiagnosticsViewViewModel",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return new DiagnosticsViewViewModel();
        });

    qmlRegisterSingletonType<MapPanelViewViewModel>("com.asic.mobilerobot.viewmodels", 1, 0, "MapPanelViewViewModel",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return new MapPanelViewViewModel();
        });

    qmlRegisterSingletonType<ChatViewViewModel>("com.asic.mobilerobot.viewmodels", 1, 0, "ChatViewViewModel",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return new ChatViewViewModel();
        });

    qmlRegisterSingletonType<WifiSettingViewViewModel>("com.asic.mobilerobot.viewmodels", 1, 0, "WifiSettingViewViewModel",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return new WifiSettingViewViewModel();
        });

    qmlRegisterSingletonType<WifiManager>("com.asic.mobilerobot.viewmodels", 1, 0, "WifiManager",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return new WifiManager();
        });

    qmlRegisterSingletonType<AppStateMachine>("com.asic.mobilerobot.viewmodels", 1, 0, "AppStateMachine",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            auto instance = &AppStateMachine::instance();
            QQmlEngine::setObjectOwnership(instance, QQmlEngine::CppOwnership);
            return instance;
        });

    // Initialize the backend state machine

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
