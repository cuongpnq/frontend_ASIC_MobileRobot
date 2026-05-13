import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root
    anchors.fill: parent

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onBackClicked: PresentationViewViewModel.requestMainView()
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
        id: contentBox
        anchors.top: containerBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 100
        color: "#FFFFFF"
        radius: 35
        border.color: "#2C2C2C"
        border.width: 1

        Flickable {
            id: flickable
            anchors.fill: parent
            anchors.margins: 40
            contentWidth: textContent.width
            contentHeight: textContent.height
            clip: true

            Text {
                id: textContent
                width: flickable.width
                text: PresentationViewViewModel.content
                font.pixelSize: 32
                font.family: "Inter"
                color: "#333333"
                wrapMode: Text.WordWrap
            }

            ScrollBar.vertical: ScrollBar {
                active: true
            }
        }
    }
}
