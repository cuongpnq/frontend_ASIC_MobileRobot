#include "modeSwitch/ModeSwitchViewModel.hpp"
#include "application/AppStateMachine.hpp"

ModeSwitchViewModel::ModeSwitchViewModel(QObject* parent) 
    : QObject(parent), m_isTakeControl(false)
{
}

bool ModeSwitchViewModel::isTakeControl() const {
    return m_isTakeControl;
}

void ModeSwitchViewModel::setControlMode(bool takeControl) {
    if (m_isTakeControl != takeControl) {
        m_isTakeControl = takeControl;
        emit controlModeChanged();
    }
}
