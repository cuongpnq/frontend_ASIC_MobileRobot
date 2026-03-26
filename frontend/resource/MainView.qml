import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import com.asic.mobilerobot.viewmodels 1.0
import QtGraphicalEffects 1.12

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: "#F5F4EF" // warm beige background

        // ── Title ──────────────────────────────────────────────────
        Text {
            id: titleText
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 160
            text: "ROBOT CONTROL CENTER"
            font.pixelSize: 50
            font.bold: true
            font.family: "Inter"
            color: "#1a1a1a"
        }

        // ── Mode Switch Pill ────────────────────────────────────────
        ModeSwitch {
            id: modeSwitchContainer
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 320
        }

        // ── Three Panels Row ────────────────────────────────────────
        Row {
            id: panelsRow
            anchors.top: modeSwitchContainer.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 32
            anchors.leftMargin: 32
            anchors.rightMargin: 32
            spacing: 16

            // ── Diagnostics Panel ───────────────────────────────────
            Rectangle {
                id: diagnosticsPanel
                width: (panelsRow.width - 2 * panelsRow.spacing) / 3
                height: 160
                radius: 12
                color: "#ddd8d0"

                Column {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 16
                    spacing: 8

                    Text {
                        text: "Diagnostics"
                        font.pixelSize: 15
                        font.bold: true
                        color: "#1a1a1a"
                    }

                    Text {
                        text: "Battery Health: <b>Good</b>"
                        font.pixelSize: 13
                        color: "#333333"
                        textFormat: Text.RichText
                    }

                    Text {
                        text: "Motors: <b>Stable</b>"
                        font.pixelSize: 13
                        color: "#333333"
                        textFormat: Text.RichText
                    }
                }

                // Settings button
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.margins: 12
                    width: settingsBtnRow.width + 20
                    height: 32
                    radius: 16
                    color: "#c8c0b8"
                    border.color: "#aaa098"
                    border.width: 1

                    Row {
                        id: settingsBtnRow
                        anchors.centerIn: parent
                        spacing: 6

                        Text {
                            text: "⚙"
                            font.pixelSize: 14
                            color: "#333333"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "Settings"
                            font.pixelSize: 13
                            color: "#333333"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                    }
                }
            }

            // ── Current Status Panel ────────────────────────────────
            Rectangle {
                id: statusPanel
                width: (panelsRow.width - 2 * panelsRow.spacing) / 3
                height: 160
                radius: 12
                color: "#ddd8d0"

                Column {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "CURRENT STATUS:"
                        font.pixelSize: 13
                        font.bold: true
                        font.letterSpacing: 1
                        color: "#555555"
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: ModeSwitchViewModel.isTakeControl ? "Manual Control" : "Exploring..."
                        font.pixelSize: 20
                        font.bold: true
                        color: "#1a73e8"

                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }
                }
            }

            // ── Map Panel ───────────────────────────────────────────
            Rectangle {
                id: mapPanel
                width: (panelsRow.width - 2 * panelsRow.spacing) / 3
                height: 160
                radius: 12
                color: "#ddd8d0"

                Text {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 12
                    text: "Map"
                    font.pixelSize: 15
                    font.bold: true
                    color: "#1a1a1a"
                }

                // Map image placeholder
                Rectangle {
                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: 8
                    width: parent.width - 32
                    height: parent.height - 52
                    radius: 8
                    color: "#c8c0b8"
                    border.color: "#aaa098"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "Image"
                        font.pixelSize: 13
                        color: "#666666"
                    }
                }
            }
        }
    }
}
