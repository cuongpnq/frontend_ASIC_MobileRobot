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
    }

    Rectangle {
        anchors.top: containerBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "#F5F4EF"
        z: -1
    }

    ListView {
        id: selectionListView
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: 1200
        height: 600
        spacing: 100
        orientation: ListView.Horizontal
        leftMargin: 50
        rightMargin: 50
        clip: true
        interactive: true

        model: ListModel {
            ListElement { name: "Direction View"; viewType: "direction" }
            ListElement { name: "Control Center"; viewType: "controlCenter" }
            ListElement { name: "Settings View"; viewType: "settings" }
        }

        delegate: Rectangle {
            width: 500
            height: 600
            color: "#F2FEFE"
            border.color: "#2C2C2C"
            border.width: 1
            radius: 35

            Text {
                anchors.centerIn: parent
                text: model.name
                font.pixelSize: 40
                font.bold: true
                color: "#2C2C2C"
                horizontalAlignment: Text.AlignHCenter
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (model.viewType === "direction") {
                        MainViewViewModel.requestDirectionView()
                    } else if (model.viewType === "controlCenter") {
                        MainViewViewModel.requestRunningView()
                    } else if (model.viewType === "settings") {
                        MainViewViewModel.requestSettingsView()
                    }
                }
            }
        }
    }
}
