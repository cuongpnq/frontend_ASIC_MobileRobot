import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12

Window {
    visible: true
    width: 800
    height: 600
    title: qsTr("Mobile Robot Application")

    MainView {
        id: mainViewComponent
        anchors.fill: parent
        visible: true // Default starting view
    }

    RunningView {
        id: runningViewComponent
        anchors.fill: parent
        visible: false
    }

    // State machine controlled dynamically by Yakindu AppStateMachine
    state: AppStateMachine.currentState
    
    states: [
        State {
            name: "MainView"
            PropertyChanges { target: mainViewComponent; visible: true }
            PropertyChanges { target: runningViewComponent; visible: false }
        },
        State {
            name: "RunningView"
            PropertyChanges { target: mainViewComponent; visible: false }
            PropertyChanges { target: runningViewComponent; visible: true }
        }
    ]
}
