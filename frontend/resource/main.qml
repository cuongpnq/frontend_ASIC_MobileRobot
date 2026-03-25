import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0

ApplicationWindow {
    visible: true
    width: 1440
    height: 960
    title: qsTr("Mobile Robot Application")

    // Container Bar at the top (always visible)
    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    // Content area below the container bar
    Item {
        id: contentArea
        anchors.top: containerBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        MainView {
            id: mainViewComponent
            anchors.fill: parent
            visible: MainViewViewModel.isActive
        }

        RunningView {
            id: runningViewComponent
            anchors.fill: parent
            visible: RunningViewViewModel.isActive
        }
    }
}
