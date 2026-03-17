import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: "#e0e0e0"

        Text {
            anchors.centerIn: parent
            text: "MAIN VIEW DASHBOARD"
            font.pixelSize: 24
            font.bold: true
        }

        Image {
            id: uitLogo
            source: "qrc:/images/UIT_logo.png"
            width: 150
            height: 150
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 20
            fillMode: Image.PreserveAspectFit
        }

        Button {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: 40
            text: "Go to Running View"
            onClicked: {
                MainViewViewModel.requestRunningView()
            }
        }
    }
}
