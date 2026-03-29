#include "directionView/DirectionViewViewModel.hpp"
#include "application/AppStateMachine.hpp"

DirectionViewViewModel::DirectionViewViewModel(QObject* parent) 
    : QObject(parent), m_isActive(false)
{
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &DirectionViewViewModel::onStateMachineChanged);
}

bool DirectionViewViewModel::isActive() const {
    return m_isActive;
}

void DirectionViewViewModel::requestMainView() {
    AppStateMachine::instance().returnToMain();
}

void DirectionViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "DirectionView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}
