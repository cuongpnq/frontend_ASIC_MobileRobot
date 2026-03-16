#include "mainView/MainViewViewModel.hpp"
#include "application/AppStateMachine.hpp"

MainViewViewModel::MainViewViewModel(QObject* parent) 
    : QObject(parent)
{
}

void MainViewViewModel::requestRunningView() {
    AppStateMachine::instance().goToRunning();
}
