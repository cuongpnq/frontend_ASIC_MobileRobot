#include "runningView/RunningViewViewModel.hpp"
#include "application/AppStateMachine.hpp"
#include "application/ROSManager.hpp"
#include "application/NavigationModule.hpp"
#include <QDebug>

RunningViewViewModel::RunningViewViewModel(QObject* parent) 
    : QObject(parent), m_isActive(false)
{
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &RunningViewViewModel::onStateMachineChanged);
    
    auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
    if (navModule) {
        connect(navModule.get(), &NavigationModule::robotStateChanged, this, &RunningViewViewModel::onRobotStateChanged);
        connect(navModule.get(), &NavigationModule::statusMessageReceived, this, &RunningViewViewModel::onStatusMessageReceived);
        connect(navModule.get(), &NavigationModule::currentCheckpointChanged, this, &RunningViewViewModel::onCheckpointChanged);
    }
}

bool RunningViewViewModel::isActive() const {
    return m_isActive;
}

void RunningViewViewModel::requestControlCenterView() {
    AppStateMachine::instance().returnToControlCenter();
}

void RunningViewViewModel::requestDirectionView() {
    AppStateMachine::instance().returnToControlCenter();
}

void RunningViewViewModel::stopRobot() {
    auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
    if (navModule) {
        navModule->sendEmergencyStop(true);
    }
}

void RunningViewViewModel::resumeRobot() {
    auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
    if (navModule) {
        navModule->sendEmergencyStop(false);
    }
}

void RunningViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "RunningView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}

void RunningViewViewModel::onRobotStateChanged(const QString& state) {
    if (m_robotState != state) {
        m_robotState = state;
        emit robotStateChanged();
    }
}

void RunningViewViewModel::onStatusMessageReceived(const QString& message) {
    if (m_statusMessage != message) {
        m_statusMessage = message;
        emit statusMessageChanged();
    }
}

void RunningViewViewModel::onCheckpointChanged(int cpId) {
    if (m_currentCheckpoint != cpId) {
        m_currentCheckpoint = cpId;
        emit currentCheckpointChanged();
    }
}
