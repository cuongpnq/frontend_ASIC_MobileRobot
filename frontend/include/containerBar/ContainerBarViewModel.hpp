#pragma once

#include <QObject>
#include <QString>

class ContainerBarViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(int batteryPower READ batteryPower WRITE setBatteryPower NOTIFY batteryPowerChanged)
    Q_PROPERTY(QString currentTime READ currentTime WRITE setCurrentTime NOTIFY currentTimeChanged)

public:
    explicit ContainerBarViewModel(QObject* parent = nullptr);
    ~ContainerBarViewModel() override = default;
    void setBatteryPower(int power);
    const int& batteryPower();
    void setCurrentTime(QString time);
    const QString& currentTime();

public signals:
    void batteryPowerChanged();
    void currentTimeChanged();

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
    int m_power {0};
    QString m_currentTime {"00:00"};
};
