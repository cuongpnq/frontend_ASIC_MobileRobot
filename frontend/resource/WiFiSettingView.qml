import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.VirtualKeyboard 2.1
import QtQuick.VirtualKeyboard.Settings 2.1
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root
    anchors.fill: parent

    // CustomKeyboard is anchored to the bottom of the parent window.
    // It must be declared LAST (or have the highest z) so it renders above
    // the Dialog's modal overlay and is reachable by touch/mouse events.

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
        // Leave space for the keyboard at the bottom when it is visible.
        anchors.bottom: parent.bottom
        anchors.bottomMargin: WifiSettingViewViewModel.keyboardVisible ? (root.height * 0.4) : 20
        anchors.leftMargin: 200
        anchors.rightMargin: 200
        anchors.topMargin: 40
        spacing: 20

        Behavior on anchors.bottomMargin { NumberAnimation { duration: 200 } }

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
        y: WifiSettingViewViewModel.keyboardVisible ? 30 : (root.height - height) / 2
        width: 780
        height: 420

        // Animate dialog position when keyboard appears
        Behavior on y { NumberAnimation { duration: 200 } }

        property string selectedSsid: ""

        // Ensure password field always gets focus when dialog opens
        onOpened: {
            passwordField.forceActiveFocus()
        }
        onClosed: {
            WifiSettingViewViewModel.dismissKeyboard()
            passwordField.text = ""
        }

        background: Rectangle {
            color: "#ffffff"
            radius: 20
            border.color: "#cccccc"
            border.width: 1
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
                width: 640
                height: 90
                font.pixelSize: 32
                echoMode: TextInput.Password
                placeholderText: "Password..."
                // Make this field the active focus item so the VK responds.
                activeFocusOnPress: true
                background: Rectangle {
                    color: "#f0f0f0"
                    radius: 10
                    border.color: passwordField.activeFocus ? "#007AFF" : "#cccccc"
                }
                // Pressing Return/Enter connects immediately.
                Keys.onReturnPressed: {
                    WifiManager.connectToNetwork(passwordDialog.selectedSsid, passwordField.text)
                    passwordDialog.close()
                }
                Keys.onEnterPressed: {
                    WifiManager.connectToNetwork(passwordDialog.selectedSsid, passwordField.text)
                    passwordDialog.close()
                }
                // Also keep old onAccepted for compatibility.
                onAccepted: {
                    WifiManager.connectToNetwork(passwordDialog.selectedSsid, passwordField.text)
                    passwordDialog.close()
                }
                // Explicitly request focus (triggers virtual keyboard) on press.
                MouseArea {
                    anchors.fill: parent
                    onPressed: {
                        passwordField.forceActiveFocus()
                        mouse.accepted = false  // let the TextField handle the press too
                    }
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
                    onClicked: passwordDialog.close()
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

    // The CustomKeyboard MUST be declared last so it appears above all children,
    // including the Dialog's modal Overlay. z: 99999 ensures event delivery.
    CustomKeyboard {
        id: inputPanel
        keyboardVisible: WifiSettingViewViewModel.keyboardVisible
        hasVirtualKeyboard: WifiSettingViewViewModel.hasVirtualKeyboard
        z: 99999
    }
}