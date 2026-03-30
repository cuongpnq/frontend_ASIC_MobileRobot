#include "diagnosticsView/DiagnosticsViewViewModel.hpp"
#include "application/AppStateMachine.hpp"

DiagnosticsViewViewModel::DiagnosticsViewViewModel(QObject* parent)
    : QObject(parent), m_isActive(false)
{
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &DiagnosticsViewViewModel::onStateMachineChanged);
}

bool DiagnosticsViewViewModel::isActive() const {
    return m_isActive;
}

void DiagnosticsViewViewModel::requestControlCenterView() {
    AppStateMachine::instance().returnToControlCenter();
}

void DiagnosticsViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "DiagnosticsView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}
