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
            x: 220
            y: 110
            source: "images/UIT_logo.png"
            opacity: 0.15
        }
    }
    
    Rectangle {
        id: mapBackground
        x: 50
        y: 120
        width: 1150
        height: 770
        radius: 32
        color: '#80dad8d8'
    }

    Text {
        x: mapBackground.x+35
        y: mapBackground.y+25
        text: "Map"
        font.pixelSize: 30
        font.bold: true
        color: '#000000'
    }

    Image {
        id: mapButton
        x: mapBackground.x + 1030
        y: mapBackground.y + 650
        width: 100
        height: 100
        source: "qrc:/images/map_button.png"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: {}
        }
    }

    Image {
        id: chatboxButton
        x: 1250
        y: 750
        width: 150
        height: 150
        source: "qrc:/images/chatbox_button.png"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: {}
        }
    }

    // Back button logic is in ContainerBar, but we could add a local one if needed.
    // For now, it's blank as requested.
}
