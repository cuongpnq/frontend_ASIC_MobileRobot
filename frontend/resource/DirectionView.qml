import QtQuick 2.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onBackClicked: DirectionViewViewModel.requestMainView()
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
    
    Rectangle {
        id: mapBackground
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: containerBar.bottom
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.margins: 50
        radius: 32
        color: '#80dad8d8'
    }

    Text {
        x: mapBackground.x+35
        y: mapBackground.y+25
        text: "Map"
        font.pixelSize: 48
        font.bold: true
        color: '#000000'
    }

    Image {
        id: mapButton
        anchors.bottom: mapBackground.bottom
        anchors.right: mapBackground.right
        anchors.bottomMargin: 25
        anchors.rightMargin: 25
        width: 100
        height: 100
        source: "qrc:/images/map_button.png"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: {}
        }
    }
    // Back button logic is in ContainerBar, but we could add a local one if needed.
    // For now, it's blank as requested.
}
