import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0

ApplicationWindow {
    visible: true
    width: 1440
    height: 960
    title: qsTr("Mobile Robot Application")


    // Dynamic View Loader (State-Driven)
    Loader {
        id: viewLoader
        anchors.fill: parent
        source: {
            switch (AppStateMachine.currentState) {
                case "MainView":          return "MainView.qml"
                case "ControlCenterView": return "ControlCenterView.qml"
                case "RunningView":       return "RunningView.qml"
                case "DirectionView":     return "DirectionView.qml"
                case "SettingsView":      return "SettingsView.qml"
                case "MapPanelView":      return "MapPanelView.qml"
                case "DiagnosticsView":   return "DiagnosticsView.qml"
                default:                  return "MainView.qml"
            }
        }
    }
}
