#include "runningView/RunningViewViewModel.hpp"
#include "application/AppStateMachine.hpp"
#include "application/ROSManager.hpp"
#include "application/NavigationModule.hpp"
#include "application/SessionLogger.hpp"
#include "directionView/DirectionViewViewModel.hpp"
#include <QDebug>

RunningViewViewModel::RunningViewViewModel(QObject* parent) 
    : QObject(parent), m_isActive(false)
{
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &RunningViewViewModel::onStateMachineChanged);
    
    auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
    if (navModule) {
        m_robotState = navModule->robotState();
        m_statusMessage = navModule->statusMessage();
        m_currentCheckpoint = navModule->currentCheckpoint();
        m_currentCheckpointName = navModule->getCheckpointName(m_currentCheckpoint);

        connect(navModule.get(), &NavigationModule::robotStateChanged, this, &RunningViewViewModel::onRobotStateChanged);
        connect(navModule.get(), &NavigationModule::statusMessageReceived, this, &RunningViewViewModel::onStatusMessageReceived);
        connect(navModule.get(), &NavigationModule::currentCheckpointChanged, this, &RunningViewViewModel::onCheckpointChanged);
    }

    m_idleTimer = new QTimer(this);
    connect(m_idleTimer, &QTimer::timeout, this, &RunningViewViewModel::onIdleTimerTimeout);
    m_idleTimer->start(1000);
}

bool RunningViewViewModel::isActive() const {
    return m_isActive;
}

void RunningViewViewModel::requestControlCenterView() {
    AppStateMachine::instance().returnToControlCenter();
}

void RunningViewViewModel::requestDirectionView() {
    // Some state machine transitions require returning to a parent state first
    AppStateMachine::instance().returnToControlCenter();
    AppStateMachine::instance().goToDirection();
}

void RunningViewViewModel::resetToDirectionView() {
    qDebug() << "RunningViewViewModel: Resetting and returning to DirectionView";
    SessionLogger::instance().logInteractionEnd(
        QStringLiteral("reset_direction"), QStringLiteral("Reset button pressed"));
    SessionLogger::instance().logEvent(QStringLiteral("running_view"),
                                       QStringLiteral("navigation"),
                                       QStringLiteral("reset_requested"));

    auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
    if (navModule) {
        navModule->sendReset();
    }

    // Set auto-return pending on the DirectionView
    if (auto dirView = DirectionViewViewModel::instance()) {
        dirView->setAutoReturnPending(true);
    }

    requestDirectionView();
}

void RunningViewViewModel::stopRobot() {
    SessionLogger::instance().logInteractionEnd(
        QStringLiteral("emergency_stop"), QStringLiteral("Stop button pressed"));
    auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
    if (navModule) {
        navModule->sendEmergencyStop(true);
    }
    SessionLogger::instance().logEvent(QStringLiteral("running_view"),
                                       QStringLiteral("navigation"),
                                       QStringLiteral("emergency_stop"),
                                       { { QStringLiteral("active"), true } });
}

void RunningViewViewModel::resumeRobot() {
    SessionLogger::instance().logInteractionEnd(
        QStringLiteral("emergency_resume"), QStringLiteral("Continue button pressed"));
    auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
    if (navModule) {
        navModule->sendEmergencyStop(false);
    }
    SessionLogger::instance().logEvent(QStringLiteral("running_view"),
                                       QStringLiteral("navigation"),
                                       QStringLiteral("emergency_stop"),
                                       { { QStringLiteral("active"), false } });
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
        const QString previousState = m_robotState;
        m_robotState = state;
        emit robotStateChanged();

        // ── Telemetry ──────────────────────────────────────────
        SessionLogger::instance().logEvent(QStringLiteral("running_view"),
                                           QStringLiteral("navigation"),
                                           QStringLiteral("state_changed"),
                                           { { QStringLiteral("from"), previousState },
                                             { QStringLiteral("to"),   m_robotState   } });

        // We intentionally do not auto-return to DirectionView here.
        // The user can manually return via the 'Direction View' button in RunningView
        // or the system will return Home via the backend's timeout.

        // Reset countdown if not in a waiting state
        if (m_robotState != "IDLE" && m_robotState != "AT_CHECKPOINT" && m_robotState != "WAITING_RESET") {
            m_idleCountdown = 15;
            emit idleCountdownChanged();
        }
    }
}

void RunningViewViewModel::onStatusMessageReceived(const QString& message) {
    if (m_statusMessage != message) {
        m_statusMessage = message;
        emit statusMessageChanged();

        // ── Telemetry ──────────────────────────────────────────
        SessionLogger::instance().logEvent(QStringLiteral("running_view"),
                                           QStringLiteral("navigation"),
                                           QStringLiteral("status"),
                                           { { QStringLiteral("message"), m_statusMessage } });

        // Parse countdown from status message if present
        // Format: "[AT_CP_TIMER] 10s remaining..." or "[RESET_TIMER] 25s remaining..."
        static QRegExp rx("(\\d+)s remaining");
        if (rx.indexIn(message) != -1) {
            int countdown = rx.cap(1).toInt();
            if (m_idleCountdown != countdown) {
                m_idleCountdown = countdown;
                emit idleCountdownChanged();
            }
        }
    }
}

void RunningViewViewModel::onCheckpointChanged(int cpId) {
    if (m_currentCheckpoint != cpId) {
        m_currentCheckpoint = cpId;
        emit currentCheckpointChanged();
        
        auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
        if (navModule) {
            m_currentCheckpointName = navModule->getCheckpointName(m_currentCheckpoint);
            emit currentCheckpointNameChanged();
        }

        // ── Telemetry ──────────────────────────────────────────
        SessionLogger::instance().logEvent(QStringLiteral("running_view"),
                                           QStringLiteral("navigation"),
                                           QStringLiteral("checkpoint_arrived"),
                                           { { QStringLiteral("id"),   m_currentCheckpoint      },
                                             { QStringLiteral("name"), m_currentCheckpointName  } });
    }
}

void RunningViewViewModel::onIdleTimerTimeout() {
    if (!m_isActive) return;

    if (m_robotState == "IDLE" || m_robotState == "AT_CHECKPOINT" || m_robotState == "WAITING_RESET") {
        if (m_currentCheckpoint != 0 && m_idleCountdown > 0) {
            m_idleCountdown--;
            emit idleCountdownChanged();

            if (m_idleCountdown <= 0) {
                auto navModule = ROSManager::instance().getModule<NavigationModule>("NavigationModule");
                if (navModule) {
                    navModule->navigateToCheckpoint(0);
                }
            }
        }
    }
}
