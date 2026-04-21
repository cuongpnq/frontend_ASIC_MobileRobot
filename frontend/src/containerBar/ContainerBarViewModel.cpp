#include "containerBar/ContainerBarViewModel.hpp"
#include <QTimer>
#include <QDateTime>

ContainerBarViewModel::ContainerBarViewModel(QObject* parent) 
    : QObject(parent),
      m_wifiConnected(true),
      m_batteryLevel(85)
{
    // Sync time immediately, then update every second
    updateTime();

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ContainerBarViewModel::updateTime);
    timer->start(1000);
}

void ContainerBarViewModel::setBatteryPower(int power)
{
    if(m_power == power)
    {
        return;
    }
    m_power = power;
    Q_EMIT batteryPowerChanged();
}

const int &ContainerBarViewModel::batteryPower() const
{
    return m_power;
}

void ContainerBarViewModel::setCurrentTime(const QString& time)
{
    if (m_currentTime == time) {
        return;
    }
    m_currentTime = time;
    Q_EMIT currentTimeChanged();
}

const QString &ContainerBarViewModel::currentTime() const
{
    return m_currentTime;
}

bool ContainerBarViewModel::wifiConnected() const
{
    return m_wifiConnected;
}

int ContainerBarViewModel::batteryLevel() const
{
    return m_batteryLevel;
}

void ContainerBarViewModel::updateTime()
{
    // Format: HH:mm  (24-hour clock, matching the existing QML font size)
    setCurrentTime(QDateTime::currentDateTime().toString("HH:mm"));
}
