#include "directionView/DirectionViewViewModel.hpp"
#include "application/AppStateMachine.hpp"
#include "application/ROSManager.hpp"
#include "application/NavigationModule.hpp"

DirectionViewViewModel::DirectionViewViewModel(QObject* parent) 
    : QObject(parent), m_isActive(false)
{
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &DirectionViewViewModel::onStateMachineChanged);
}

bool DirectionViewViewModel::isActive() const {
    return m_isActive;
}

bool DirectionViewViewModel::autoReturnPending() const {
    return m_autoReturnPending;
}

void DirectionViewViewModel::setAutoReturnPending(bool value) {
    if (m_autoReturnPending != value) {
        m_autoReturnPending = value;
        emit autoReturnPendingChanged();
    }
}

void DirectionViewViewModel::requestMainView() {
    AppStateMachine::instance().returnToMain();
}

void DirectionViewViewModel::requestRunningView() {
    AppStateMachine::instance().goToRunning();
}

void DirectionViewViewModel::startNavigation(int cpId) {
    auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
    if (navModule) {
        navModule->navigateToCheckpoint(cpId);
    }
}

void DirectionViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "DirectionView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}
