#include "containerBar/ContainerBarViewModel.hpp"
#include <QTimer>

ContainerBarViewModel::ContainerBarViewModel(QObject* parent) 
    : QObject(parent),
      m_wifiConnected(true),
      m_batteryLevel(85)
{
    QTimer currentTime;
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

const int &ContainerBarViewModel::batteryPower()
{
    return m_power;
}

void ContainerBarViewModel::setCurrentTime(QString time)
{

}

const QString &ContainerBarViewModel::currentTime()
{

}
