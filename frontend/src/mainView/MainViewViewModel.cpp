#include "mainView/MainViewViewModel.hpp"
#include "application/AppStateMachine.hpp"

MainViewViewModel::MainViewViewModel(QObject* parent) 
    : QObject(parent), m_isActive(true)
{
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &MainViewViewModel::onStateMachineChanged);
}

bool MainViewViewModel::isActive() const {
    return m_isActive;
}

bool MainViewViewModel::isTakeControl() const {
    return m_isTakeControl;
}

void MainViewViewModel::setControlMode(bool takeControl) {
    if (m_isTakeControl != takeControl) {
        m_isTakeControl = takeControl;
        emit controlModeChanged();
    }
}

void MainViewViewModel::requestRunningView() {
    AppStateMachine::instance().goToRunning();
}

void MainViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "MainView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}
