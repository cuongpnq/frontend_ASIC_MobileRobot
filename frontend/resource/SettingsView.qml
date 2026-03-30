import QtQuick 2.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root
    anchors.fill: parent

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onBackClicked: SettingsViewViewModel.requestMainView()
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

    MouseArea {
        anchors.top: containerBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            MainViewViewModel.requestRunningView()
        }
    }

    Text {
        anchors.centerIn: parent
        text: "Settings View (Blank)"
        font.pixelSize: 40
        color: "#2C2C2C"
    }
}
