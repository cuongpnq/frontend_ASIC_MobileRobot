import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root
    anchors.fill: parent

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onPowerClicked: powerPopup.open()
    }

    // --- Power Menu Popup ---
    Popup {
        id: powerPopup
        x: 30
        y: 115
        width: 240
        height: 150
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        // Automatically close when standby is triggered (e.g. by timer or button)
        Connections {
            target: window
            onIsStandbyChanged: {
                if (window.isStandby) powerPopup.close()
            }
        }

        background: Rectangle {
            border.color: "#22000000"
            border.width: 4
            radius: 24
        }

        Column {
            anchors.fill: parent
            spacing: 8

            // Power Off Row
            Rectangle {
                width: parent.width
                height: 60
                radius: 12
                color: powerOffMouse.pressed ? "#e0e0e0" : "#f0f0f0"
                border.color: "#bbbbbb"
                border.width: 1

                Row {
                    anchors.centerIn: parent
                    spacing: 12
                    Text { 
                        text: "Power Off"
                        font.pixelSize: 22
                        font.family: "Inter"
                        color: '#ff0000' 
                        font.bold: true
                    }
                }

                MouseArea {
                    id: powerOffMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: Qt.quit()
                }
            }

            // Sleep Row
            Rectangle {
                width: parent.width
                height: 60
                radius: 12
                color: sleepMouse.pressed ? "#e0e0e0" : "#f0f0f0"
                border.color: "#bbbbbb"
                border.width: 1

                Row {
                    anchors.centerIn: parent
                    spacing: 12
                    Text { 
                        text: "Sleep"
                        font.pixelSize: 22
                        font.family: "Inter"
                        color: '#978800' 
                        font.bold: true
                    }
                }

                MouseArea {
                    id: sleepMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        window.isStandby = true
                        powerPopup.close()
                    }
                }
            }
        }
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
                ListElement { name: "Presentation"; viewType: "presentation"; icon: "images/presentation.png" }
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
                        } else if (model.viewType === "presentation") {
                            MainViewViewModel.requestPresentationView()
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
