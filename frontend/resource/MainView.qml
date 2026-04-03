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
            anchors.horizontalCenter: parent.horizontalCenter
            y: 120
            width: 1300
            height: 1105
            source: "images/UIT_logo.png"
            opacity: 0.15
        }
    }

    Rectangle{
        id: selectionListViewReg
        width: parent.width
        height: parent.height - containerBar.height
        anchors.top: containerBar.bottom
        color: "transparent"
        
        ListView {
            id: selectionListView
            anchors.centerIn: parent
            width: parent.width - 160
            height: 600
            spacing: 100
            orientation: ListView.Horizontal
            interactive: true

            model: ListModel {
                ListElement { name: "Direction View"; viewType: "direction"; icon: "images/direction.png" }
                ListElement { name: "Q&A"; viewType: "chatView"; icon: "images/chatbot.png" }
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
                        } else if (model.viewType === "chatView") {
                            MainViewViewModel.requestChatView()
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
}
