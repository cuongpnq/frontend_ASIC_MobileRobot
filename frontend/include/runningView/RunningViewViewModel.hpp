#pragma once

#include <QObject>
#include <QTimer>

class RunningViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(QString robotState READ robotState NOTIFY robotStateChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int currentCheckpoint READ currentCheckpoint NOTIFY currentCheckpointChanged)
    Q_PROPERTY(QString currentCheckpointName READ currentCheckpointName NOTIFY currentCheckpointNameChanged)
    Q_PROPERTY(int idleCountdown READ idleCountdown NOTIFY idleCountdownChanged)

public:
    explicit RunningViewViewModel(QObject* parent = nullptr);
    ~RunningViewViewModel() override = default;

    bool isActive() const;
    QString robotState() const { return m_robotState; }
    QString statusMessage() const { return m_statusMessage; }
    int currentCheckpoint() const { return m_currentCheckpoint; }
    QString currentCheckpointName() const { return m_currentCheckpointName; }
    int idleCountdown() const { return m_idleCountdown; }

    Q_INVOKABLE void requestControlCenterView();
    Q_INVOKABLE void requestDirectionView();
    Q_INVOKABLE void resetToDirectionView();
    Q_INVOKABLE void stopRobot();
    Q_INVOKABLE void resumeRobot();

signals:
    void isActiveChanged();
    void robotStateChanged();
    void statusMessageChanged();
    void currentCheckpointChanged();
    void currentCheckpointNameChanged();
    void idleCountdownChanged();

private slots:
    void onStateMachineChanged();
    void onRobotStateChanged(const QString& state);
    void onStatusMessageReceived(const QString& message);
    void onCheckpointChanged(int cpId);
    void onIdleTimerTimeout();

private:
    bool m_isActive = false;
    QString m_robotState = "IDLE";
    QString m_statusMessage = "Ready";
    int m_currentCheckpoint = -1;
    QString m_currentCheckpointName = "Unknown";
    int m_idleCountdown = 15;
    QTimer* m_idleTimer = nullptr;
};
