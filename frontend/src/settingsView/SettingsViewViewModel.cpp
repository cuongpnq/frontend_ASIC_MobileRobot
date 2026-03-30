#include "settingsView/SettingsViewViewModel.hpp"
#include "application/AppStateMachine.hpp"

SettingsViewViewModel::SettingsViewViewModel(QObject* parent) 
    : QObject(parent), m_isActive(false)
{
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &SettingsViewViewModel::onStateMachineChanged);
}

bool SettingsViewViewModel::isActive() const {
    return m_isActive;
}

void SettingsViewViewModel::requestMainView() {
    AppStateMachine::instance().returnToMain();
}

void SettingsViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "SettingsView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}
