#include "mapPanelView/MapPanelViewViewModel.hpp"
#include "application/AppStateMachine.hpp"

MapPanelViewViewModel::MapPanelViewViewModel(QObject* parent)
    : QObject(parent), m_isActive(false)
{
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &MapPanelViewViewModel::onStateMachineChanged);
}

bool MapPanelViewViewModel::isActive() const {
    return m_isActive;
}

void MapPanelViewViewModel::requestControlCenterView() {
    AppStateMachine::instance().returnToControlCenter();
}

void MapPanelViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "MapPanelView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}
