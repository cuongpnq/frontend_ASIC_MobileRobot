#include "runningView/RunningViewViewModel.hpp"
#include "application/AppStateMachine.hpp"

RunningViewViewModel::RunningViewViewModel(QObject* parent) 
    : QObject(parent), m_isActive(false)
{
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &RunningViewViewModel::onStateMachineChanged);
}

bool RunningViewViewModel::isActive() const {
    return m_isActive;
}

void RunningViewViewModel::requestMainView() {
    AppStateMachine::instance().returnToMain();
}

void RunningViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "RunningView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}
