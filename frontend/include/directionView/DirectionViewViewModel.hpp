#pragma once

#include <QObject>

class DirectionViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool autoReturnPending READ autoReturnPending NOTIFY autoReturnPendingChanged)
    Q_PROPERTY(bool idleReturnPending READ idleReturnPending NOTIFY idleReturnPendingChanged)
    Q_PROPERTY(QString mapId READ mapId NOTIFY mapIdChanged)

public:
    explicit DirectionViewViewModel(QObject* parent = nullptr);
    ~DirectionViewViewModel() override;

    static DirectionViewViewModel* instance();

    bool isActive() const;
    bool autoReturnPending() const;
    bool idleReturnPending() const;
    QString mapId() const { return m_mapId; }
    Q_INVOKABLE void setAutoReturnPending(bool value);
    Q_INVOKABLE void setIdleReturnPending(bool value);
    Q_INVOKABLE void requestMainView();
    Q_INVOKABLE void requestRunningView();
    Q_INVOKABLE void startNavigation(int cpId);

signals:
    void isActiveChanged();
    void autoReturnPendingChanged();
    void idleReturnPendingChanged();
    void mapIdChanged();

private slots:
    void onStateMachineChanged();
    void onMapIdChanged(const QString& mapId);
    void onRobotStateChanged(const QString& state);
    void onCheckpointChanged(int cpId);

private:
    void checkIdleReturnStatus();

    bool m_isActive = false;
    bool m_autoReturnPending = false;
    bool m_idleReturnPending = false;
    QString m_mapId = "unknown";
    QString m_robotState = "Unknown";
    int m_currentCheckpoint = -1;
};
