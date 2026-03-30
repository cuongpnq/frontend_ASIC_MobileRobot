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
            ListElement { name: "Direction View"; viewType: "direction"; icon: "images/direction.png" }
            ListElement { name: "Control Center"; viewType: "controlCenter"; icon: "images/control_center.png" }
            ListElement { name: "Settings View"; viewType: "settings"; icon: "images/settings.png" }
        }

        delegate: Rectangle {
            width: 500
            height: 600
            color: "#F2FEFE"
            border.color: "#2C2C2C"
            border.width: 1
            radius: 35

            Image {
                source: model.icon
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (model.viewType === "direction") {
                        MainViewViewModel.requestDirectionView()
                    } else if (model.viewType === "controlCenter") {
                        MainViewViewModel.requestControlCenterView()
                    } else if (model.viewType === "settings") {
                        MainViewViewModel.requestSettingsView()
                    }
                }
            }
        }
    }
}
