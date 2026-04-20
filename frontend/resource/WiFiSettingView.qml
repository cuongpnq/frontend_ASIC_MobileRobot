import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.VirtualKeyboard 2.1
import QtQuick.VirtualKeyboard.Settings 2.1
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root
    anchors.fill: parent

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onBackClicked: WifiSettingViewViewModel.requestSettingsView()
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

        MouseArea {
            anchors.fill: parent
            onClicked: WifiSettingViewViewModel.dismissKeyboard()
        }
    }

    Text {
        id: settingsText
        anchors.top: containerBar.bottom
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.topMargin: 70
        text: "WiFi Settings"
        font.pixelSize: 96
        font.bold: true
        font.family: "Inter"
        color: "#000000"
    }

    // Main content area
    ColumnLayout {
        id: contentColumn
        anchors.top: settingsText.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Qt.inputMethod.visible ? Qt.inputMethod.keyboardRectangle.height : 20
        anchors.leftMargin: 200
        anchors.rightMargin: 200
        anchors.topMargin: 40
        spacing: 20

        Rectangle {
            width: 220
            height: 60
            radius: 25
            color: WifiManager.busy ? "#a9a9a9" : "#c9d9d9d9"
            Layout.preferredWidth: 220
            Layout.preferredHeight: 60

            Text {
                anchors.centerIn: parent
                text: WifiManager.busy ? "Scanning..." : "Scan"
                font.family: "Inter"
                font.bold: true
                font.pixelSize: 32
                color: "black"
            }

            MouseArea {
                anchors.fill: parent
                enabled: !WifiManager.busy
                onClicked: WifiManager.scanNetworks()
            }
        }

        ListView {
            id: wifiList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: WifiManager
            clip: true
            spacing: 20
            interactive: true

            delegate: Rectangle {
                width: wifiList.width
                height: 110
                color: "#c9d9d9d9"
                radius: 25

                Text {
                    id: networkText
                    anchors.left: parent.left
                    anchors.leftMargin: 55
                    anchors.verticalCenter: parent.verticalCenter
                    text: ssid + (secure ? " 🔒" : "")
                    font.pixelSize: 48
                    font.family: "Inter"
                    color: "black"
                }

                Text {
                    anchors.left: networkText.right
                    anchors.leftMargin: 40
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Signal: " + strength + "%"
                    font.pixelSize: 32
                    font.family: "Inter"
                    color: "#666666"
                }

                Rectangle {
                    anchors.right: parent.right
                    anchors.rightMargin: 70
                    anchors.verticalCenter: parent.verticalCenter
                    width: 250
                    height: 70
                    radius: 25
                    color: (WifiManager.connectedSsid === ssid) ? "#ff9999" : "#a9c9c9"

                    Text {
                        anchors.centerIn: parent
                        text: (WifiManager.connectedSsid === ssid) ? "Disconnect" : "Connect"
                        font.family: "Inter"
                        font.pixelSize: 36
                        color: "black"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (WifiManager.connectedSsid === ssid) {
                                WifiManager.disconnectCurrent()
                            } else {
                                if (secure) {
                                    passwordDialog.selectedSsid = ssid
                                    passwordDialog.open()
                                } else {
                                    WifiManager.connectToNetwork(ssid, "")
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: passwordDialog
        modal: true
        focus: true
        x: (root.width - width) / 2
        y: WifiSettingViewViewModel.keyboardVisible ? 50 : (root.height - height) / 2
        width: 700
        height: 400
        
        property string selectedSsid: ""

        background: Rectangle {
            color: "#ffffff"
            radius: 20
            border.color: "#cccccc"
            border.width: 1

            MouseArea {
                anchors.fill: parent
                onClicked: WifiSettingViewViewModel.dismissKeyboard()
                z: -1
            }
        }

        Column {
            anchors.centerIn: parent
            spacing: 30

            Text {
                text: "Enter password for " + passwordDialog.selectedSsid
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: true
                color: "#000000"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            TextField {
                id: passwordField
                width: 600
                height: 80
                font.pixelSize: 32
                anchors.horizontalCenter: parent.horizontalCenter
                echoMode: TextInput.Password
                placeholderText: "Password..."
                background: Rectangle {
                    color: "#f0f0f0"
                    radius: 10
                    border.color: passwordField.focus ? "#007AFF" : "#cccccc"
                }
                onAccepted: {
                    WifiManager.connectToNetwork(passwordDialog.selectedSsid, passwordField.text)
                    passwordDialog.close()
                    passwordField.text = ""
                    WifiSettingViewViewModel.dismissKeyboard()
                }
            }

            Row {
                spacing: 30
                anchors.horizontalCenter: parent.horizontalCenter

                Button {
                    text: "Cancel"
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 28
                        color: "black"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    width: 200
                    height: 70
                    onClicked: {
                        passwordDialog.close()
                        passwordField.text = ""
                        WifiSettingViewViewModel.dismissKeyboard()
                    }
                }

                Button {
                    text: "Connect"
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 28
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: "#007AFF"
                        radius: 10
                    }
                    width: 200
                    height: 70
                    onClicked: {
                        WifiManager.connectToNetwork(passwordDialog.selectedSsid, passwordField.text)
                        passwordDialog.close()
                        passwordField.text = ""
                        WifiSettingViewViewModel.dismissKeyboard()
                    }
                }
            }
        }
    }

    Connections {
        target: WifiManager
        function onConnectionSucceeded(ssid) { console.log("Connected to", ssid) }
        function onConnectionFailed(ssid, reason) { console.log("Connect failed:", ssid, reason) }
    }
}