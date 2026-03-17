import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: "#d0ffd0"

        Text {
            anchors.centerIn: parent
            text: "RUNNING VIEW DASHBOARD"
            font.pixelSize: 24
            font.bold: true
        }

        Image {
            id: backButton
            source: "qrc:/images/back_button.png"
            width: 40
            height: 40
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 20

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.log("Back button clicked")
                    RunningViewViewModel.requestMainView()
                }
            }
        }
    }
}
