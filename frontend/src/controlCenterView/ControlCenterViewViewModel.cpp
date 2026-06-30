#include "controlCenterView/ControlCenterViewViewModel.hpp"
#include "application/AppStateMachine.hpp"
#include "preCheckView/SysCheckViewModel.hpp"
#include "directionView/DirectionViewViewModel.hpp"

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
    if (g_sysCheckViewModel) g_sysCheckViewModel->open();
}

void ControlCenterViewViewModel::requestStartNavigation() {
    if (!g_sysCheckViewModel) return;
    // Use the floor currently selected in DirectionView.
    // DirectionViewViewModel::instance() is set in its constructor.
    const QString floor = DirectionViewViewModel::instance()
                          ? DirectionViewViewModel::instance()->mapId()
                          : QStringLiteral("e6");
    g_sysCheckViewModel->launchNavigation(floor);
}

void ControlCenterViewViewModel::requestStopNavigation() {
    if (g_sysCheckViewModel) g_sysCheckViewModel->stopNavigation();
}

void ControlCenterViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "ControlCenterView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}
