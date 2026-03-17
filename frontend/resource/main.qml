import QtQuick 2.12
import QtQuick.Controls 2.12

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: qsTr("Mobile Robot Application")

    MainView {
        id: mainViewComponent
        anchors.fill: parent
        visible: true
    }

    RunningView {
        id: runningViewComponent
        anchors.fill: parent
        visible: false
    }
}
