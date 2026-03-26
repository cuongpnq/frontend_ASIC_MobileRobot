#include "containerBar/ContainerBarViewModel.hpp"
#include <QTimer>

ContainerBarViewModel::ContainerBarViewModel(QObject* parent) 
    : QObject(parent),
      m_wifiConnected(true),
      m_batteryLevel(85)
{
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
}
