import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root

    // Mapping ROS state to local navStatus
    property string navStatus: {
        var state = RunningViewViewModel.robotState
        if (state === "NAVIGATING" || state === "PRE_ROTATING" || state === "COMPUTING_PATH" || state === "RETURNING_HOME")
            return "navigating"
        if (state === "WAITING_RESET")
            return "waiting"
        return "stopped"
    }

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onBackClicked: RunningViewViewModel.requestControlCenterView()
    }

    // Background
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

    // ── Central Status Card ─────────────────────────────────────────
    Rectangle {
        id: statusCard
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 40
        width: 700
        height: 380
        radius: 36
        color: "#FFFFFF"
        opacity: 0.92

        // Subtle border
        border.color: navStatus === "navigating" ? "#107DB3" : "#CC3333"
        border.width: 3

        Behavior on border.color { ColorAnimation { duration: 250 } }

        // Left accent stripe
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.topMargin: 16
            anchors.bottomMargin: 16
            width: 8
            radius: 4
            color: navStatus === "navigating" ? "#107DB3" : "#CC3333"
            Behavior on color { ColorAnimation { duration: 250 } }
        }

        Column {
            anchors.centerIn: parent
            spacing: 32
            width: parent.width - 80

            // ── Status Text ───────────────────────────────────────
            Column {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 6

                Text {
                    id: statusLabel
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: RunningViewViewModel.statusMessage
                    font.pixelSize: 30
                    font.family: "Inter"
                    font.bold: false
                    color: "#555555"
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                    width: parent.width
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 14

                    Text {
                        id: statusText
                        text: RunningViewViewModel.robotState
                        font.pixelSize: 64
                        font.family: "Inter"
                        font.bold: true
                        color: navStatus === "navigating" ? "#107DB3" : "#CC3333"

                        Behavior on color { ColorAnimation { duration: 250 } }
                    }

                    // 3 staggered pulsing dots when navigating
                    Row {
                        visible: navStatus === "navigating"
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        Repeater {
                            model: 3
                            Rectangle {
                                width: 14
                                height: 14
                                radius: 7
                                color: "#107DB3"
                                opacity: 0.2

                                SequentialAnimation on opacity {
                                    running: navStatus === "navigating"
                                    loops: Animation.Infinite
                                    PauseAnimation   { duration: index * 200 }
                                    NumberAnimation  { to: 1.0; duration: 400; easing.type: Easing.InOutSine }
                                    NumberAnimation  { to: 0.2; duration: 400; easing.type: Easing.InOutSine }
                                    PauseAnimation   { duration: (2 - index) * 200 }
                                }
                            }
                        }
                    }
                }
            }

            // ── Current Location ──────────────────────────────────────
            Column {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 4
                visible: RunningViewViewModel.currentCheckpointName !== "Unknown"

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Current Location"
                    font.pixelSize: 20
                    font.family: "Inter"
                    color: "#888888"
                    font.capitalization: Font.AllUppercase
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: RunningViewViewModel.currentCheckpointName
                    font.pixelSize: 36
                    font.family: "Inter"
                    font.bold: true
                    color: "#333333"
                }
            }

            // ── Auto-home countdown hint ───────────────────────────────
            Text {
                id: autoHomeHint
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Returning Home in " + RunningViewViewModel.idleCountdown + "s…"
                font.pixelSize: 22
                font.family: "Inter"
                color: "#CC3333"
                font.italic: true
                visible: (RunningViewViewModel.robotState === "IDLE" || RunningViewViewModel.robotState === "AT_CHECKPOINT" || RunningViewViewModel.robotState === "WAITING_RESET") && RunningViewViewModel.currentCheckpoint !== 0

                SequentialAnimation on opacity {
                    running: autoHomeHint.visible
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.4; duration: 800; easing.type: Easing.InOutSine }
                    NumberAnimation { to: 1.0; duration: 800; easing.type: Easing.InOutSine }
                }
            }

            // ── Button Row ────────────────────────────────────────
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 24

                // STOP button (only when navigating)
                Rectangle {
                    visible: navStatus === "navigating" || RunningViewViewModel.robotState === "NAVIGATING"
                    width: 200
                    height: 66
                    radius: 16
                    color: stopArea.pressed ? "#AA2222" : "#CC3333"

                    Behavior on color { ColorAnimation { duration: 150 } }

                    Text {
                        anchors.centerIn: parent
                        text: "■  Stop"
                        font.pixelSize: 28
                        font.family: "Inter"
                        font.bold: true
                        color: "#FFFFFF"
                    }

                    MouseArea {
                        id: stopArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: RunningViewViewModel.stopRobot()
                    }
                }

                // RESET button (only when stopped or idle)
                Rectangle {
                    visible: navStatus === "stopped" || RunningViewViewModel.robotState === "IDLE" || RunningViewViewModel.robotState === "AT_CHECKPOINT"
                    width: 200
                    height: 66
                    radius: 16
                    color: resetArea.pressed ? "#888888" : "#AAAAAA"

                    Behavior on color { ColorAnimation { duration: 150 } }

                    Text {
                        anchors.centerIn: parent
                        text: "↺  Reset"
                        font.pixelSize: 28
                        font.family: "Inter"
                        font.bold: true
                        color: "#FFFFFF"
                    }

                    MouseArea {
                        id: resetArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            DirectionViewViewModel.setAutoReturnPending(true)
                            RunningViewViewModel.requestDirectionView()
                        }
                    }
                }

                // CONTINUE button (only when emergency stop)
                Rectangle {
                    visible: RunningViewViewModel.robotState === "EMERGENCY_STOP"
                    width: 200
                    height: 66
                    radius: 16
                    color: continueArea.pressed ? "#0D6199" : "#107DB3"

                    Behavior on color { ColorAnimation { duration: 150 } }

                    Text {
                        anchors.centerIn: parent
                        text: "▶  Resume"
                        font.pixelSize: 28
                        font.family: "Inter"
                        font.bold: true
                        color: "#FFFFFF"
                    }

                    MouseArea {
                        id: continueArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: RunningViewViewModel.resumeRobot()
                    }
                }
            }
        }
    }
}
