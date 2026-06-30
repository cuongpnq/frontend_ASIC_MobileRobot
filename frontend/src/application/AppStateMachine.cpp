#include "application/AppStateMachine.hpp"
#include "application/SessionLogger.hpp"
#include <QDebug>

AppStateMachine::AppStateMachine(QObject* parent) 
    : QObject(parent), m_currentState("Unknown")
{
    // Initialize and start the Yakindu state machine
    statechart_init(&m_statechart);
    statechart_enter(&m_statechart);

    // Initial state check
    updateState();
}

AppStateMachine::~AppStateMachine()
{
    statechart_exit(&m_statechart);
}

QString AppStateMachine::currentState() const
{
    return m_currentState;
}

void AppStateMachine::goToRunning()
{
    statechart_raise_goToRunning(&m_statechart);
    updateState();
}

void AppStateMachine::returnToMain()
{
    statechart_raise_returnToMain(&m_statechart);
    updateState();
}

void AppStateMachine::goToControlCenter()
{
    statechart_raise_goToControlCenter(&m_statechart);
    updateState();
}

void AppStateMachine::goToDirection()
{
    statechart_raise_goToDirection(&m_statechart);
    updateState();
}

void AppStateMachine::goToPresentation()
{
    statechart_raise_goToPresentation(&m_statechart);
    updateState();
}

void AppStateMachine::returnToControlCenter()
{
    statechart_raise_returnToControlCenter(&m_statechart);
    updateState();
}

void AppStateMachine::goToSettings()
{
    statechart_raise_goToSettings(&m_statechart);
    updateState();
}

void AppStateMachine::goToDiagnostics()
{
    statechart_raise_goToDiagnostics(&m_statechart);
    updateState();
}

void AppStateMachine::goToMapPanel()
{
    statechart_raise_goToMapPanel(&m_statechart);
    updateState();
}

void AppStateMachine::goToChatView()
{
    statechart_raise_goToChatView(&m_statechart);
    updateState();
}

void AppStateMachine::goToWiFiSettings()
{
    statechart_raise_goToWiFiSettings(&m_statechart);
    updateState();
}

void AppStateMachine::returnToSettings()
{
    statechart_raise_returnToSettings(&m_statechart);
    updateState();
}

void AppStateMachine::updateState()
{
    QString newState = m_currentState;

    if (statechart_is_state_active(&m_statechart, Statechart_frontend_app_MainView)) {
        newState = "MainView";
    } else if (statechart_is_state_active(&m_statechart, Statechart_frontend_app_ControlCenterView)) {
        newState = "ControlCenterView";
    } else if (statechart_is_state_active(&m_statechart, Statechart_frontend_app_DirectionView)) {
        newState = "DirectionView";
    } else if (statechart_is_state_active(&m_statechart, Statechart_frontend_app_RunningView)) {
        newState = "RunningView";
    } else if (statechart_is_state_active(&m_statechart, Statechart_frontend_app_SettingsView)) {
        newState = "SettingsView";
    } else if (statechart_is_state_active(&m_statechart, Statechart_frontend_app_DiagnosticsView)) {
        newState = "DiagnosticsView";
    } else if (statechart_is_state_active(&m_statechart, Statechart_frontend_app_MapPanelView)) {
        newState = "MapPanelView";
    } else if (statechart_is_state_active(&m_statechart, Statechart_frontend_app_ChatView)) {
        newState = "ChatView";
    } else if (statechart_is_state_active(&m_statechart, Statechart_frontend_app_WiFiSettingView)) {
        newState = "WiFiSettingView";
    } else if (statechart_is_state_active(&m_statechart, Statechart_frontend_app_PresentationView)) {
        newState = "PresentationView";
    }

    if (m_currentState != newState) {
        const QString previousState = m_currentState;
        m_currentState = newState;
        emit currentStateChanged();

        // ── Telemetry: record UI view transition ────────────────────────
        SessionLogger::instance().logEvent(QStringLiteral("ui"),
                                           QStringLiteral("state_transition"),
                                           { { QStringLiteral("from"), previousState },
                                             { QStringLiteral("to"),   m_currentState } });
    }
}
