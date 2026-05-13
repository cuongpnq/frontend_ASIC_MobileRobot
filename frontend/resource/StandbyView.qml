import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: root
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: "#2C2C2C"

        Image {
            id: robotEyes
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height / 3.0
            source: "images/robot_eyes_open.png"
            width: 1000
            fillMode: Image.PreserveAspectFit
        }

        Timer {
            id: blinkTimer
            interval: 10000
            running: root.visible
            repeat: true
            onTriggered: {
                robotEyes.source = "images/robot_eyes_closed.png"
                closeTimer.start()
            }
        }

        Timer {
            id: closeTimer
            interval: 150
            running: false
            repeat: false
            onTriggered: {
                robotEyes.source = "images/robot_eyes_open.png"
            }
        }

        Text {
            id: timeText
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            color: "#FFFFFF"
            font.pixelSize: 80
            font.bold: true
            text: Qt.formatDateTime(new Date(), "hh:mm ap")
        }

        Text {
            id: standbyText
            anchors.bottom: timeText.top
            anchors.bottomMargin: 100
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Tap anywhere to wake"
            color: "#FFFFFF"
            font.pixelSize: 50
            horizontalAlignment: Text.AlignHCenter
        }


        Timer {
            interval: 1000
            running: root.visible
            repeat: true
            onTriggered: timeText.text = Qt.formatDateTime(new Date(), "hh:mm ap")
        }

        // Block clicks and use onClicked to wake up
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (window.isStandby) {
                    window.isStandby = false;
                }
            }
        }
    }
}
