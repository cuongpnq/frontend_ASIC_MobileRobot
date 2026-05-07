#include "directionView/DirectionViewViewModel.hpp"
#include "application/AppStateMachine.hpp"
#include "application/ROSManager.hpp"
#include "application/NavigationModule.hpp"
#include <QDebug>

static DirectionViewViewModel* s_instance = nullptr;

DirectionViewViewModel* DirectionViewViewModel::instance() {
    return s_instance;
}

DirectionViewViewModel::DirectionViewViewModel(QObject* parent) 
    : QObject(parent), m_isActive(false)
{
    s_instance = this;
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &DirectionViewViewModel::onStateMachineChanged);
    
    auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
    if (navModule) {
        m_mapId = navModule->mapId();
        connect(navModule.get(), &NavigationModule::mapIdChanged, this, &DirectionViewViewModel::onMapIdChanged);
    }
}

DirectionViewViewModel::~DirectionViewViewModel() {
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

bool DirectionViewViewModel::isActive() const {
    return m_isActive;
}

bool DirectionViewViewModel::autoReturnPending() const {
    return m_autoReturnPending;
}

bool DirectionViewViewModel::idleReturnPending() const {
    return m_idleReturnPending;
}

void DirectionViewViewModel::setAutoReturnPending(bool value) {
    if (m_autoReturnPending != value) {
        m_autoReturnPending = value;
        emit autoReturnPendingChanged();
    }
}

void DirectionViewViewModel::setIdleReturnPending(bool value) {
    if (m_idleReturnPending != value) {
        m_idleReturnPending = value;
        emit idleReturnPendingChanged();
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

void DirectionViewViewModel::onMapIdChanged(const QString& mapId) {
    if (m_mapId != mapId) {
        m_mapId = mapId;
        emit mapIdChanged();
    }
}
