#include "application/AppStateMachine.hpp"
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

void AppStateMachine::updateState()
{
    QString newState = m_currentState;

    if (statechart_is_state_active(&m_statechart, Statechart_frontend_app_MainView)) {
        newState = "MainView";
    } else if (statechart_is_state_active(&m_statechart, Statechart_frontend_app_RunningView)) {
        newState = "RunningView";
    }

    if (m_currentState != newState) {
        m_currentState = newState;
        emit currentStateChanged();
    }
}
