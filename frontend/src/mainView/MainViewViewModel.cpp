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

void MainViewViewModel::requestControlCenterView() {
    AppStateMachine::instance().goToControlCenter();
}

void MainViewViewModel::requestDirectionView() {
    AppStateMachine::instance().goToDirection();
}

void MainViewViewModel::requestSettingsView() {
    AppStateMachine::instance().goToSettings();
}

void MainViewViewModel::requestChatView() {
    AppStateMachine::instance().goToChatView();
}

void MainViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "MainView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}
