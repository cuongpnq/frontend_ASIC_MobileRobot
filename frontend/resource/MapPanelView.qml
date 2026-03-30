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
        onBackClicked: MapPanelViewViewModel.requestControlCenterView()
    }

    Rectangle {
        anchors.fill: parent
        color: "#F5F4EF"
        z: -1

        Image {
            x: 220
            y: 110
            source: "images/UIT_logo.png"
            width: 1000
            height: 1000
            fillMode: Image.PreserveAspectFit
            opacity: 0.15
        }
    }

    Text {
        anchors.centerIn: parent
        text: "MAP PANEL VIEW"
        font.pixelSize: 24
        font.bold: true
        color: "#2C2C2C"
    }
}
