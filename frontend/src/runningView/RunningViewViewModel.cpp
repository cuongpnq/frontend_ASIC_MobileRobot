#include "runningView/RunningViewViewModel.hpp"
#include "application/AppStateMachine.hpp"

RunningViewViewModel::RunningViewViewModel(QObject* parent) 
    : QObject(parent)
{
}

void RunningViewViewModel::requestMainView() {
    AppStateMachine::instance().returnToMain();
}
