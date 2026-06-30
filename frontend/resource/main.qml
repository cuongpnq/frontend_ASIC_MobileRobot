import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import com.asic.mobilerobot.viewmodels 1.0

ApplicationWindow {
    id: window
    visible: true
    flags: Qt.FramelessWindowHint
    visibility: Window.FullScreen
    width: 1920
    height: 1200

    property bool isStandby: true

    Timer {
        id: standbyTimer
        interval: SettingsViewViewModel.sleepTimeoutMs
        running: SettingsViewViewModel.sleepTimeout > 0
                 && !window.isStandby
                 && AppStateMachine.currentState !== "RunningView"
                 && !PresentationViewViewModel.isFileViewerOpen
                 && !PresentationViewViewModel.isMobileImportActive
        repeat: false
        onTriggered: {
            if (AppStateMachine.currentState !== "RunningView") {
                window.isStandby = true
            }
        }
    }

    Connections {
        target: UserInteraction
        onInteracted: {
            standbyTimer.restart();
        }
    }

    // Skip standby after boot: go directly to MainView when the boot overlay
    // auto-dismisses (or user taps Continue early).
    Connections {
        target: SysCheckViewModel
        onIsStartupCheckDoneChanged: {
            if (SysCheckViewModel.isStartupCheckDone) {
                window.isStandby = false
            }
        }
    }

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
                case "PresentationView":  return "PresentationView.qml"
                case "ChatView":          return "ChatView.qml"
                case "WiFiSettingView":   return "WiFiSettingView.qml"
                default:                  return "MainView.qml"
            }
        }
    }

    // Global Virtual Keyboard
    // Placing it here ensures it's above all views and dialogs
    CustomKeyboard {
        id: globalKeyboard
        keyboardVisible: Qt.inputMethod.visible
        hasVirtualKeyboard: HAS_VIRTUAL_KEYBOARD
    }

    // Standby View Overlay
    StandbyView {
        id: standbyView
        anchors.fill: parent
        visible: window.isStandby
        z: 100 // High z-index to cover everything
    }
}
