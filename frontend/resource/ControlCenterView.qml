import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import com.asic.mobilerobot.viewmodels 1.0
import QtGraphicalEffects 1.12

Item {
    id: root

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        onBackClicked: ControlCenterViewViewModel.requestMainView()
    }

    Rectangle {
        anchors.fill: parent
        color: "#F5F4EF" // warm beige background

        // Background watermark
        Image {
            x: 220
            y: 110
            source: "images/UIT_logo.png"
            opacity: 0.15
        }

        // ── Title ──────────────────────────────────────────────────
        Text {
            id: titleText
            anchors.horizontalCenter: parent.horizontalCenter
            y: 160
            text: "ROBOT CONTROL CENTER"
            font.pixelSize: 50
            font.bold: true
            font.family: "Inter"
            color: "#1a1a1a"
        }

        // ── Mode Switch Pill ────────────────────────────────────────
        ModeSwitch {
            id: modeSwitchContainer
            anchors.horizontalCenter: parent.horizontalCenter
            y: 320
        }

        // ── Three Panels Row ────────────────────────────────────────
        Row {
            id: panelsRow
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 50
            y: 580
            spacing: 45

            // ── Diagnostics Panel ───────────────────────────────────
            Rectangle {
                id: diagnosticsPanel
                width: 381
                height: 202
                radius: 32
                color: "#DAD8D8"
                opacity: 0.8
                Text {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 20
                    text: "Diagnostics"
                    font.pixelSize: 30
                    font.bold: true
                    font.family: "Inter"
                    color: "#000000"
                }

                Column {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.topMargin: 85
                    anchors.leftMargin: 20
                    spacing: 8

                    Text {
                        text: "Battery Health: <b>Good</b>"
                        font.pixelSize: 30
                        font.family: "Inter"
                        color: "#333333"
                        textFormat: Text.RichText
                    }

                    Text {
                        text: "Motors: <b>Stable</b>"
                        font.pixelSize: 30
                        font.family: "Inter"
                        color: "#333333"
                        textFormat: Text.RichText
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        ControlCenterViewViewModel.requestDiagnosticsView()
                    }
                }
            }

            // ── Current Status Panel ────────────────────────────────
            Rectangle {
                id: statusPanel
                width: 450
                height: 202
                radius: 32
                color: "#DAD8D8"
                opacity: 0.8

                Column {
                    anchors.centerIn: parent
                    spacing: 20

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "CURRENT STATUS:"
                        font.pixelSize: 30
                        font.family: "Inter"
                        font.bold: true
                        font.letterSpacing: 1
                        color: "#594A4A"
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: ModeSwitchViewModel.isTakeControl ? "Controlling..." : "Exploring..."
                        font.pixelSize: 40
                        font.family: "Inter"
                        font.bold: true
                        color: ModeSwitchViewModel.isTakeControl ? "#007339" : "#107DB3"

                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }

                    Text {
                        text: "Tap to go to running view"
                        font.pixelSize: 25
                        font.family: "Inter"
                        color: "#2C2C2C"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        ControlCenterViewViewModel.requestRunningView()
                    }
                }
            }

            // ── Map Panel ───────────────────────────────────────────
            Rectangle {
                id: mapPanel
                width: 400
                height: 292
                radius: 32
                color: "#DAD8D8"
                opacity: 0.8

                Text {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 20
                    text: "Map"
                    font.pixelSize: 30
                    font.family: "Inter"
                    font.bold: true
                    color: "#000000"
                }

                // Map image placeholder
                Rectangle {
                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: 8
                    width: 300
                    height: 160
                    radius: 8
                    color: "#D9D9D9"
                    border.color: "#2C2C2C"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "Map"
                        font.family: "Inter"
                        font.pixelSize: 30
                        color: "#000000"
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        ControlCenterViewViewModel.requestMapPanelView()
                    }
                }
            }
        }
    }
}
