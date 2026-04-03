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
        z: -1

        // Background watermark
        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 120
            width: 1300
            height: 1105
            source: "images/UIT_logo.png"
            opacity: 0.15
        }

        // ── Title ──────────────────────────────────────────────────
        Text {
            id: titleText
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: containerBar.height + 150
            text: "ROBOT CONTROL CENTER"
            font.pixelSize: 96
            font.bold: true
            font.family: "Inter"
            color: "#1a1a1a"
        }

        // ── Mode Switch Pill ────────────────────────────────────────
        ModeSwitch {
            id: modeSwitchContainer
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: titleText.bottom
            anchors.topMargin: 100
        }

        // ── Three Panels Row ────────────────────────────────────────
        Row {
            id: panelsRow
            height: 380
            width: parent.width
            leftPadding: 80
            rightPadding: 80
            anchors.top: modeSwitchContainer.bottom
            anchors.topMargin: 100
            spacing: 100

            // ── Diagnostics Panel ───────────────────────────────────
            Rectangle {
                id: diagnosticsPanel
                width: (parent.width - 80 * 2 - 100 * 2) / 3
                height: 320
                radius: 32
                color: "#DAD8D8"
                opacity: 0.8
                Text {
                    id: diagnosticsTitle
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 20
                    text: "Diagnostics"
                    font.pixelSize: 48
                    font.bold: true
                    font.family: "Inter"
                    color: "#000000"
                }

                Column {
                    anchors.top: diagnosticsTitle.bottom
                    anchors.left: parent.left
                    anchors.topMargin: 50
                    anchors.leftMargin: 20
                    spacing: 8

                    Text {
                        text: "Battery Health: <b>Good</b>"
                        font.pixelSize: 40
                        font.family: "Inter"
                        color: "#333333"
                        textFormat: Text.RichText
                    }

                    Text {
                        text: "Motors: <b>Stable</b>"
                        font.pixelSize: 40
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
                width: (parent.width - 80 * 2 - 100 * 2) / 3
                height: 320
                radius: 32
                color: "#DAD8D8"
                opacity: 0.8

                Column {
                    anchors.centerIn: parent
                    spacing: 40

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "CURRENT STATUS:"
                        font.pixelSize: 48
                        font.family: "Inter"
                        font.bold: true
                        color: "#594A4A"
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: ModeSwitchViewModel.isTakeControl ? "Controlling..." : "Exploring..."
                        font.pixelSize: 46
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
                width: (parent.width - 80 * 2 - 100 * 2) / 3
                height: 320
                radius: 32
                color: "#DAD8D8"
                opacity: 0.8

                Text {
                    id: mapTitle
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 20
                    text: "Map"
                    font.pixelSize: 48
                    font.family: "Inter"
                    font.bold: true
                    color: "#000000"
                }

                // Map image placeholder
                Rectangle {
                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: 8
                    width: parent.width - 40
                    height: parent.height - 120
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
