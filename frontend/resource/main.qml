import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: qsTr("Mobile Robot Application")

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
