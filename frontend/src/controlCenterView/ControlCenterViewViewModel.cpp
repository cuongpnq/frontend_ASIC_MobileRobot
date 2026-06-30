#include "controlCenterView/ControlCenterViewViewModel.hpp"
#include "application/AppStateMachine.hpp"

ControlCenterViewViewModel::ControlCenterViewViewModel(QObject* parent) 
    : QObject(parent), m_isActive(false)
{
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &ControlCenterViewViewModel::onStateMachineChanged);
}

bool ControlCenterViewViewModel::isActive() const {
    return m_isActive;
}

void ControlCenterViewViewModel::requestRunningView() {
    AppStateMachine::instance().goToRunning();
}

void ControlCenterViewViewModel::requestMainView() {
    AppStateMachine::instance().returnToMain();
}

void ControlCenterViewViewModel::requestMapPanelView() {
    AppStateMachine::instance().goToMapPanel();
}

void ControlCenterViewViewModel::requestDiagnosticsView() {
    AppStateMachine::instance().goToDiagnostics();
}

void ControlCenterViewViewModel::requestSysCheckView() {
    extern SysCheckViewModel* g_sysCheckViewModel;
    if (g_sysCheckViewModel) g_sysCheckViewModel->open();
}

void ControlCenterViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "ControlCenterView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}
