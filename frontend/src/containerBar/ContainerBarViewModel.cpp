#include "containerBar/ContainerBarViewModel.hpp"
#include <QDateTime>

ContainerBarViewModel::ContainerBarViewModel(QObject* parent) 
    : QObject(parent),
      m_wifiConnected(true),
      m_batteryLevel(85)
{
    // Initialize with current time
    updateTime();

    // Update time every second
    connect(&m_timer, &QTimer::timeout, this, &ContainerBarViewModel::updateTime);
    m_timer.start(1000);
}

QString ContainerBarViewModel::currentTime() const {
    return m_currentTime;
}

bool ContainerBarViewModel::wifiConnected() const {
    return m_wifiConnected;
}

int ContainerBarViewModel::batteryLevel() const {
    return m_batteryLevel;
}

void ContainerBarViewModel::updateTime() {
    QString newTime = QDateTime::currentDateTime().toString("hh:mm:ss");
    if (m_currentTime != newTime) {
        m_currentTime = newTime;
        emit currentTimeChanged();
    }
}
