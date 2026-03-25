#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

class ContainerBarViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentTime READ currentTime NOTIFY currentTimeChanged)
    Q_PROPERTY(bool wifiConnected READ wifiConnected NOTIFY wifiStatusChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)

public:
    explicit ContainerBarViewModel(QObject* parent = nullptr);
    ~ContainerBarViewModel() override = default;

    QString currentTime() const;
    bool wifiConnected() const;
    int batteryLevel() const;

signals:
    void currentTimeChanged();
    void wifiStatusChanged();
    void batteryLevelChanged();

private slots:
    void updateTime();

private:
    QString m_currentTime;
    bool m_wifiConnected;
    int m_batteryLevel;
    QTimer m_timer;
};
