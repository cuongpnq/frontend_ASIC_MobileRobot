import QtQuick 2.12
import QtQuick.Controls 2.12
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

    Rectangle { anchors.fill: parent; color: "#F5F4EF"; z: -1
        Image { anchors.horizontalCenter: parent.horizontalCenter; y: 120; width: 1300; height: 1105; source: "images/UIT_logo.png"; opacity: 0.15 }
    }

    Text {
        id: settingsText
        anchors.top: containerBar.bottom
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.topMargin: 70
        text: "Settings"
        font.pixelSize: 96; font.bold: true; font.family: "Inter"; color: "#000000"
    }

    Column {
        anchors.top: settingsText.bottom
        anchors.left: parent.left
        anchors.leftMargin: 200
        anchors.topMargin: 70
        spacing: 20

        // ── WiFi ──────────────────────────────────────────────────────────
        Rectangle {
            width: 1500; height: 100; color: "#c9d9d9d9"; radius: 25
            Image { id: wifiIcon; source: "images/wifi_icon.png"; anchors.left: parent.left; anchors.leftMargin: 70; anchors.verticalCenter: parent.verticalCenter }
            Text { anchors.left: wifiIcon.right; anchors.leftMargin: 55; anchors.verticalCenter: parent.verticalCenter; text: "WiFi"; font.pixelSize: 64; font.family: "Inter"; color: "#000000" }
            Image { source: "images/forward_button.png"; anchors.right: parent.right; anchors.rightMargin: 70; anchors.verticalCenter: parent.verticalCenter }
            MouseArea { anchors.fill: parent; onClicked: SettingsViewViewModel.requestWifiSettingsView() }
        }

        // ── User Mode ─────────────────────────────────────────────────────
        Rectangle {
            width: 1500; height: 100; color: "#c9d9d9d9"; radius: 25

            Row {
                anchors.left: parent.left; anchors.leftMargin: 70
                anchors.verticalCenter: parent.verticalCenter
                spacing: 30

                Text { text: "User Mode"; font.pixelSize: 48; font.family: "Inter"; color: "#000000"; anchors.verticalCenter: parent.verticalCenter }

                Rectangle {
                    height: 52; width: badgeLabel.implicitWidth + 36; radius: 26
                    anchors.verticalCenter: parent.verticalCenter
                    color: UserManager.roleBadgeColor
                    Text { id: badgeLabel; anchors.centerIn: parent; text: UserManager.roleName; font.pixelSize: 28; font.bold: true; font.family: "Inter"; color: "white" }
                }
            }

            Row {
                anchors.right: parent.right; anchors.rightMargin: 40
                anchors.verticalCenter: parent.verticalCenter
                spacing: 20

                // Logout — only when elevated
                Rectangle {
                    visible: UserManager.currentRole !== UserManager.User
                    width: 160; height: 60; radius: 15
                    color: logoutArea.pressed ? "#dc2626" : "#ef4444"
                    Text { anchors.centerIn: parent; text: "Logout"; font.pixelSize: 28; font.bold: true; font.family: "Inter"; color: "white" }
                    MouseArea { id: logoutArea; anchors.fill: parent; onClicked: UserManager.logout() }
                }

                // Switch Role button
                Rectangle {
                    width: 230; height: 60; radius: 15
                    color: switchArea.pressed ? "#1d4ed8" : "#2563eb"
                    Text { anchors.centerIn: parent; text: "Switch Mode"; font.pixelSize: 28; font.bold: true; font.family: "Inter"; color: "white" }
                    MouseArea {
                        id: switchArea; anchors.fill: parent
                        onClicked: {
                            UserManager.resetLoginResult()
                            loginDialog.step = 1   // go back to role-selection step
                            loginDialog.open()
                        }
                    }
                }
            }
        }
    }

    // ── Two-Step Login Dialog ─────────────────────────────────────────────
    // Using Item instead of Popup to avoid modal event grabbing that blocks
    // the virtual keyboard. Popup.modal intercepts touch events at QWindow
    // level regardless of z-order, preventing InputPanel from receiving input.
    Item {
        id: loginDialog
        anchors.fill: parent
        z: 50
        visible: false
        focus: visible

        property int step: 1

        function open() {
            step = 1
            passwordField.text = ""
            UserManager.resetLoginResult()
            visible = true
        }

        function close() {
            passwordField.focus = false
            Qt.inputMethod.hide()
            visible = false
            UserManager.resetLoginResult()
        }

        Keys.onEscapePressed: loginDialog.close()

        // Dim backdrop — closes dialog on tap; keyboard (z:MAX) absorbs its own taps first
        Rectangle {
            anchors.fill: parent
            color: "#80000000"
            MouseArea { anchors.fill: parent; onClicked: loginDialog.close() }
        }

        // Dialog card — slides up when the virtual keyboard is visible
        Rectangle {
            id: dialogCard
            width: 720
            height: loginDialog.step === 1 ? 340 : 440
            radius: 28; color: "white"; border.color: "#e2e8f0"; border.width: 2

            anchors.horizontalCenter: parent.horizontalCenter
            y: {
                var kbH = Qt.inputMethod.visible ? Qt.inputMethod.keyboardRectangle.height : 0
                return Math.max(20, (parent.height - kbH - height) / 2)
            }
            Behavior on y { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }

            // ── ← Back button (top-left, step 2 only) ────────────────────
            Rectangle {
                width: 48; height: 48; radius: 24
                color: backBtnArea.pressed ? "#e2e8f0" : "transparent"
                anchors.top: parent.top; anchors.left: parent.left
                anchors.topMargin: 12; anchors.leftMargin: 12
                z: 10
                visible: loginDialog.step === 2
                Text { anchors.centerIn: parent; text: "←"; font.pixelSize: 28; color: "#94a3b8" }
                MouseArea {
                    id: backBtnArea; anchors.fill: parent
                    onClicked: { loginDialog.step = 1; UserManager.resetLoginResult() }
                }
            }

            // ── × Close button (top-right) ────────────────────────────
            Rectangle {
                id: closeBtn
                width: 48; height: 48; radius: 24
                color: closeBtnArea.pressed ? "#e2e8f0" : "transparent"
                anchors.top: parent.top; anchors.right: parent.right
                anchors.topMargin: 12; anchors.rightMargin: 12
                z: 10
                Text { anchors.centerIn: parent; text: "✕"; font.pixelSize: 28; color: "#94a3b8" }
                MouseArea { id: closeBtnArea; anchors.fill: parent; onClicked: loginDialog.close() }
            }

            // ── Step 1: Role selection ────────────────────────────────────────
            Column {
                anchors.centerIn: parent
                spacing: 28
                width: parent.width - 80
                visible: loginDialog.step === 1

                Text { anchors.horizontalCenter: parent.horizontalCenter; text: "Select Mode"; font.pixelSize: 40; font.bold: true; font.family: "Inter"; color: "#1e293b" }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 24

                    Rectangle {
                        width: 280; height: 72; radius: 16
                        color: adminBtn.pressed ? "#92400e" : "#d97706"
                        Text { anchors.centerIn: parent; text: "Administrator"; font.pixelSize: 28; font.bold: true; font.family: "Inter"; color: "white" }
                        MouseArea {
                            id: adminBtn; anchors.fill: parent
                            onClicked: {
                                UserManager.selectTargetRole(UserManager.Administrator)
                                passwordField.text = ""
                                loginDialog.step = 2
                            }
                        }
                    }

                    Rectangle {
                        width: 240; height: 72; radius: 16
                        color: devBtn.pressed ? "#4c1d95" : "#7c3aed"
                        Text { anchors.centerIn: parent; text: "Developer"; font.pixelSize: 28; font.bold: true; font.family: "Inter"; color: "white" }
                        MouseArea {
                            id: devBtn; anchors.fill: parent
                            onClicked: {
                                UserManager.selectTargetRole(UserManager.Developer)
                                passwordField.text = ""
                                loginDialog.step = 2
                            }
                        }
                    }
                }
            }

            // ── Step 2: Password entry ────────────────────────────────────────
            Column {
                anchors.centerIn: parent
                spacing: 24
                width: parent.width - 80
                visible: loginDialog.step === 2

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Enter " + UserManager.targetRoleName + " Password"
                    font.pixelSize: 34; font.bold: true; font.family: "Inter"; color: "#1e293b"
                }

                Rectangle {
                    width: parent.width; height: 80; radius: 16
                    color: "#f1f5f9"
                    border.color: passwordField.activeFocus ? "#2563eb" : "#cbd5e1"; border.width: 2
                    TextField {
                        id: passwordField
                        anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 20
                        placeholderText: "Password..."
                        echoMode: TextInput.Password
                        font.pixelSize: 32; font.family: "Inter"
                        background: Rectangle { color: "transparent" }
                        onAccepted: UserManager.attemptLogin(passwordField.text)
                        onTextChanged: { if (UserManager.loginResult === UserManager.Failed) UserManager.resetLoginResult() }
                    }
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: UserManager.loginResult === UserManager.Failed ? "Incorrect password. Please try again." : ""
                    font.pixelSize: 26; font.family: "Inter"; color: "#ef4444"
                    visible: UserManager.loginResult === UserManager.Failed
                }

                Connections {
                    target: UserManager
                    onLoginResultChanged: {
                        if (UserManager.loginResult === UserManager.Success) {
                            loginDialog.close()
                            SettingsViewViewModel.requestMainView()
                        } else if (UserManager.loginResult === UserManager.Failed) {
                            passwordField.text = ""
                            passwordField.forceActiveFocus()
                        }
                    }
                }

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 240; height: 72; radius: 16
                    color: confirmBtn.pressed ? "#1d4ed8" : "#2563eb"
                    Text { anchors.centerIn: parent; text: "Login"; font.pixelSize: 30; font.bold: true; font.family: "Inter"; color: "white" }
                    MouseArea { id: confirmBtn; anchors.fill: parent; onClicked: UserManager.attemptLogin(passwordField.text) }
                }
            }
        }
    }
}
