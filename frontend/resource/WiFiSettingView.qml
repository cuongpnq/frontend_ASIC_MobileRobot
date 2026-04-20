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

                Item {
                    id: wifiStrengthIcon
                    anchors.left: networkText.right
                    anchors.leftMargin: 40
                    anchors.verticalCenter: parent.verticalCenter
                    width: 54
                    height: 42

                    Canvas {
                        id: wifiStrengthCanvas
                        anchors.fill: parent
                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)

                            var sig = strength   // 0-100 from model
                            var color = "#444444"

                            ctx.strokeStyle = color
                            ctx.lineWidth = 3.5
                            ctx.lineCap = "round"

                            var cx = width / 2
                            var cy = height - 4

                            // Smallest arc — always shown
                            ctx.beginPath()
                            ctx.arc(cx, cy, 9, Math.PI * 1.25, Math.PI * 1.75, false)
                            ctx.stroke()

                            // Medium arc — shown when signal > 33 %
                            if (sig > 33) {
                                ctx.beginPath()
                                ctx.arc(cx, cy, 18, Math.PI * 1.25, Math.PI * 1.75, false)
                                ctx.stroke()
                            }

                            // Large arc — shown when signal > 66 %
                            if (sig > 66) {
                                ctx.beginPath()
                                ctx.arc(cx, cy, 27, Math.PI * 1.25, Math.PI * 1.75, false)
                                ctx.stroke()
                            }

                            // Center dot
                            ctx.fillStyle = color
                            ctx.beginPath()
                            ctx.arc(cx, cy, 3.5, 0, Math.PI * 2)
                            ctx.fill()
                        }

                        Component.onCompleted: requestPaint()
                    }
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
                                showToast("Disconnecting from '" + ssid + "'...", "info")
                                WifiManager.disconnectCurrent()
                            } else {
                                if (secure) {
                                    passwordDialog.selectedSsid = ssid
                                    passwordDialog.open()
                                } else {
                                    showToast("Connecting to '" + ssid + "'...", "info")
                                    WifiManager.connectToNetwork(ssid, "")
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Dim overlay — sits BELOW the keyboard (z:99999) so keyboard events pass through.
    // Tapping the dim area dismisses the password popup.
    Rectangle {
        id: dimOverlay
        anchors.fill: parent
        color: "#80000000"
        visible: passwordDialog.visible
        z: 9000

        MouseArea {
            anchors.fill: parent
            onClicked: passwordDialog.close()
        }
    }

    // Non-modal Popup — avoids the window-level Overlay that blocks the virtual keyboard.
    Popup {
        id: passwordDialog
        // non-modal: no Qt-level Overlay is created, so keyboard events reach CustomKeyboard
        modal: false
        focus: false          // let TextField own the focus
        closePolicy: Popup.NoAutoClose   // we close manually
        x: (root.width - width) / 2
        y: WifiSettingViewViewModel.keyboardVisible ? 30 : (root.height - height) / 2
        width: 780
        height: 420
        z: 9001               // above dim overlay, below keyboard

        // Animate position when keyboard appears/disappears
        Behavior on y { NumberAnimation { duration: 200 } }

        property string selectedSsid: ""

        // Small delay so the Popup enter animation settles before we grab focus.
        Timer {
            id: focusTimer
            interval: 150
            repeat: false
            onTriggered: passwordField.forceActiveFocus()
        }

        onOpened: focusTimer.restart()
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
                anchors.horizontalCenter: parent.horizontalCenter
                echoMode: TextInput.Password
                placeholderText: "Password..."
                activeFocusOnPress: true
                background: Rectangle {
                    color: "#f0f0f0"
                    radius: 10
                    border.color: passwordField.activeFocus ? "#007AFF" : "#cccccc"
                }
                Keys.onReturnPressed: {
                    showToast("Connecting to '" + passwordDialog.selectedSsid + "'...", "info")
                    WifiManager.connectToNetwork(passwordDialog.selectedSsid, passwordField.text)
                    passwordDialog.close()
                }
                Keys.onEnterPressed: {
                    showToast("Connecting to '" + passwordDialog.selectedSsid + "'...", "info")
                    WifiManager.connectToNetwork(passwordDialog.selectedSsid, passwordField.text)
                    passwordDialog.close()
                }
                onAccepted: {
                    showToast("Connecting to '" + passwordDialog.selectedSsid + "'...", "info")
                    WifiManager.connectToNetwork(passwordDialog.selectedSsid, passwordField.text)
                    passwordDialog.close()
                }
                // Tap on field: (re-)grab focus to keep keyboard visible
                MouseArea {
                    anchors.fill: parent
                    onPressed: {
                        passwordField.forceActiveFocus()
                        mouse.accepted = false
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
                        showToast("Connecting to '" + passwordDialog.selectedSsid + "'...", "info")
                        WifiManager.connectToNetwork(passwordDialog.selectedSsid, passwordField.text)
                        passwordDialog.close()
                    }
                }
            }
        }
    }


    // ── Toast notification ────────────────────────────────────────────────────
    // Usage: call showToast("message", "success" | "error" | "info")
    function showToast(message, type) {
        toastMessage.text = message
        if (type === "success") {
            toastBg.color = "#2ECC71"       // green
            toastIcon.text = "✔"
        } else if (type === "error") {
            toastBg.color = "#E74C3C"       // red
            toastIcon.text = "✖"
        } else {
            toastBg.color = "#5B93C5"       // blue-info
            toastIcon.text = "ℹ"
        }
        toastItem.opacity = 1
        toastTimer.restart()
    }

    Item {
        id: toastItem
        anchors.top: parent.top
        anchors.topMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter
        width: toastRow.implicitWidth + 60
        height: 90
        opacity: 0
        z: 199998          // just below the keyboard

        Behavior on opacity { NumberAnimation { duration: 220 } }

        Rectangle {
            id: toastBg
            anchors.fill: parent
            radius: 45
            color: "#2ECC71"

            // soft drop-shadow via layering
            layer.enabled: true
        }

        Row {
            id: toastRow
            anchors.centerIn: parent
            spacing: 18

            Text {
                id: toastIcon
                text: "✔"
                font.pixelSize: 40
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                id: toastMessage
                text: ""
                font.pixelSize: 34
                font.family: "Inter"
                font.bold: true
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Timer {
            id: toastTimer
            interval: 3000
            repeat: false
            onTriggered: toastItem.opacity = 0
        }
    }

    // ── WiFiManager event connections ─────────────────────────────────────────
    Connections {
        target: WifiManager

        function onConnectionSucceeded(ssid) {
            console.log("Connected to", ssid)
            showToast("Connected to '" + ssid + "'", "success")
        }

        function onConnectionFailed(ssid, reason) {
            console.log("Connect failed:", ssid, reason)
            // Detect wrong-password clue from NM error string
            var isPwdWrong = reason.indexOf("secrets") !== -1
                          || reason.indexOf("password") !== -1
                          || reason.indexOf("psk") !== -1
                          || reason.indexOf("Invalid") !== -1
            if (isPwdWrong) {
                showToast("Wrong password for '" + ssid + "'", "error")
            } else if (ssid === "") {
                showToast("Disconnected", "info")
            } else {
                showToast("Failed: " + reason, "error")
            }
        }

        function onConnectedSsidChanged() {
            // When ssid becomes empty it means we just disconnected successfully.
            if (WifiManager.connectedSsid === "") {
                showToast("Wi-Fi disconnected", "info")
            }
        }
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