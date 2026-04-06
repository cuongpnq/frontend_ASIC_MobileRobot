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
        onBackClicked: SettingsViewViewModel.requestMainView()
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

    Text {
        id: settingsText
        anchors.top: containerBar.bottom
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.topMargin: 70
        text: "Settings"
        font.pixelSize: 96
        font.bold: true
        font.family: "Inter"
        color: "#000000"
    }

    // ── Settings Options ──
    Column {
        anchors.top: settingsText.bottom
        anchors.left: parent.left
        anchors.leftMargin: 200
        anchors.topMargin: 70
        spacing:20

        Rectangle {
            width: 1500
            height: 100
            color: '#c9d9d9d9'
            radius: 25
            
            Image {
                id: wifiIcon
                source: "images/wifi_icon.png"
                anchors.left: parent.left
                anchors.leftMargin: 70
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                anchors.left: wifiIcon.right
                anchors.leftMargin: 55
                anchors.verticalCenter: parent.verticalCenter
                text: "WiFi"
                font.pixelSize: 64
                font.family: "Inter"
                color: "#000000"
            }

            Image{
                source: "images/forward_button.png"
                anchors.right: parent.right
                anchors.rightMargin: 70
                anchors.verticalCenter: parent.verticalCenter
            }
            
            MouseArea{
                anchors.fill: parent
                onClicked:{
                    SettingsViewViewModel.requestWifiSettingsView()
                }
            }
        }
        Rectangle {
            width: 1500
            height: 100
            color: '#c9d9d9d9'
            radius: 25
            
            // Image {
            //     id: wifiIcon
            //     source: "images/wifi_icon.png"
            //     anchors.left: parent.left
            //     anchors.leftMargin: 70
            //     anchors.verticalCenter: parent.verticalCenter
            // }

            // Text {
            //     anchors.left: wifiIcon.right
            //     anchors.leftMargin: 55
            //     anchors.verticalCenter: parent.verticalCenter
            //     text: "WiFi"
            //     font.pixelSize: 64
            //     font.family: "Inter"
            //     color: "#000000"
            // }

            // Image{
            //     source: "images/forward_button.png"
            //     anchors.right: parent.right
            //     anchors.rightMargin: 70
            //     anchors.verticalCenter: parent.verticalCenter
            // }
            
            // MouseArea{
            //     anchors.fill: parent
            //     onClicked:{
            //         // SettingsViewViewModel.requestWifiSettingsView()
            //     }
            // }
        }
    }    
}
