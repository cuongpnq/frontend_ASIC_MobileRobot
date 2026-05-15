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
        m_robotState = navModule->robotState();
        m_currentCheckpoint = navModule->currentCheckpoint();
        connect(navModule.get(), &NavigationModule::mapIdChanged, this, &DirectionViewViewModel::onMapIdChanged);
        connect(navModule.get(), &NavigationModule::robotStateChanged, this, &DirectionViewViewModel::onRobotStateChanged);
        connect(navModule.get(), &NavigationModule::currentCheckpointChanged, this, &DirectionViewViewModel::onCheckpointChanged);
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

QString DirectionViewViewModel::mapImage() const {
    auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
    if (navModule) {
        return navModule->getMapImage();
    }
    return "images/a_maplocation.png";
}

QVariantList DirectionViewViewModel::locations() const {
    auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
    if (navModule) {
        return navModule->getLocations();
    }
    return QVariantList();
}

void DirectionViewViewModel::onRobotStateChanged(const QString& state) {
    if (m_robotState != state) {
        m_robotState = state;
        
        if (m_robotState == "IDLE") {
            // Requirement: If robot in IDLE state, the view is always DirectionView
            if (AppStateMachine::instance().currentState() != "DirectionView") {
                qDebug() << "DirectionViewViewModel: Robot is IDLE, forcing DirectionView transition.";
                AppStateMachine::instance().goToDirection();
            }
        }
        
        checkIdleReturnStatus();
    }
}

void DirectionViewViewModel::onCheckpointChanged(int cpId) {
    if (m_currentCheckpoint != cpId) {
        m_currentCheckpoint = cpId;
        checkIdleReturnStatus();
    }
}

void DirectionViewViewModel::checkIdleReturnStatus() {
    // Only proceed if this view is active
    if (!m_isActive) return;

    // Condition: Robot is IDLE and NOT at Home (cpId 0)
    if (m_robotState == "IDLE" && m_currentCheckpoint != 0 && m_currentCheckpoint != -1) {
        // Trigger the 15s countdown in QML
        if (!m_idleReturnPending) {
            qDebug() << "DirectionViewViewModel: Robot is IDLE away from home, starting 15s timeout.";
            setIdleReturnPending(true);
        }
    } else {
        // If robot starts moving or reaches home, cancel the pending return
        if (m_idleReturnPending) {
            setIdleReturnPending(false);
        }
    }
}

void DirectionViewViewModel::setMapId(const QString& mapId) {
    auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
    if (navModule) {
        navModule->setMapId(mapId);
    }
}

QStringList DirectionViewViewModel::availableMaps() const {
    auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
    if (navModule) {
        return navModule->availableMaps();
    }
    return QStringList();
}
