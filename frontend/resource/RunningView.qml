import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root

    // "navigating" or "stopped"
    property string navStatus: "navigating"

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
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Current status:"
                    font.pixelSize: 30
                    font.family: "Inter"
                    font.bold: false
                    color: "#555555"
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 14

                    Text {
                        id: statusText
                        text: navStatus === "navigating" ? "Navigating" : "Stopped"
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

            // ── Button Row ────────────────────────────────────────
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 24

                // STOP button (only when navigating)
                Rectangle {
                    visible: navStatus === "navigating"
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
                        onClicked: navStatus = "stopped"
                    }
                }

                // RESET button (only when stopped)
                Rectangle {
                    visible: navStatus === "stopped"
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

                // CONTINUE button (only when stopped)
                Rectangle {
                    visible: navStatus === "stopped"
                    width: 200
                    height: 66
                    radius: 16
                    color: continueArea.pressed ? "#0D6199" : "#107DB3"

                    Behavior on color { ColorAnimation { duration: 150 } }

                    Text {
                        anchors.centerIn: parent
                        text: "▶  Continue"
                        font.pixelSize: 28
                        font.family: "Inter"
                        font.bold: true
                        color: "#FFFFFF"
                    }

                    MouseArea {
                        id: continueArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: navStatus = "navigating"
                    }
                }
            }
        }
    }

    // Reset status to "navigating" every time this view becomes active
    Connections {
        target: RunningViewViewModel
        onIsActiveChanged: {
            if (RunningViewViewModel.isActive) {
                navStatus = "navigating"
            }
        }
    }
}
