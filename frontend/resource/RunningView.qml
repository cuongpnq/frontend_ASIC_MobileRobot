import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onBackClicked: RunningViewViewModel.requestControlCenterView()
    }

    Rectangle {
        anchors.fill: parent
        color: "#F5F4EF"
        z: -1

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 120
            width: 1300
            height: 1105
            source: "images/UIT_logo.png"
            opacity: 0.15
        }
    }

    Text {
        anchors.centerIn: parent
        text: "RUNNING VIEW DASHBOARD"
        font.pixelSize: 24
        font.bold: true
        color: "#2C2C2C"
    }
}
