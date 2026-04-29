#pragma once

#include <QObject>

class RunningViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(QString robotState READ robotState NOTIFY robotStateChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int currentCheckpoint READ currentCheckpoint NOTIFY currentCheckpointChanged)

public:
    explicit RunningViewViewModel(QObject* parent = nullptr);
    ~RunningViewViewModel() override = default;

    bool isActive() const;
    QString robotState() const { return m_robotState; }
    QString statusMessage() const { return m_statusMessage; }
    int currentCheckpoint() const { return m_currentCheckpoint; }

    Q_INVOKABLE void requestControlCenterView();
    Q_INVOKABLE void requestDirectionView();
    Q_INVOKABLE void stopRobot();
    Q_INVOKABLE void resumeRobot();

signals:
    void isActiveChanged();
    void robotStateChanged();
    void statusMessageChanged();
    void currentCheckpointChanged();

private slots:
    void onStateMachineChanged();
    void onRobotStateChanged(const QString& state);
    void onStatusMessageReceived(const QString& message);
    void onCheckpointChanged(int cpId);

private:
    bool m_isActive = false;
    QString m_robotState = "IDLE";
    QString m_statusMessage = "Ready";
    int m_currentCheckpoint = -1;
};
