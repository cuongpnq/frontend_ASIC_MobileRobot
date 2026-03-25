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
        Rectangle {
            id: modeSwitchContainer
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 320
            width: 840
            height: 180
            radius: 99
            color: "#33D8FFFF"
            border.color: "#c0b8b0"
            border.width: 1.5

            // ── Sliding track (highlight) — moves left or right ───
            Rectangle {
                id: switchTrack
                width: 465
                height: 140
                radius: 99
                anchors.verticalCenter: parent.verticalCenter
                x: MainViewViewModel.isTakeControl ? parent.width - 465 - 20 : 20
                color: "#FFFFFF"
                border.color: "#4caf50"
                border.width: 2

                Behavior on x {
                    NumberAnimation {
                        duration: 220
                        easing.type: Easing.InOutCubic
                    }
                }
            }
            DropShadow {
                anchors.fill: switchTrack
                source: switchTrack
                horizontalOffset: 0
                verticalOffset: 2
                radius: 24
                samples: 32
                spread: 0.16
                color: "#32A323"
            }

            // "Self Discovery" label — left side
            Text {
                id: selfDiscoveryLabel
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 30
                text: "Self Discovery"
                font.pixelSize: 36
                font.family: "Inter"
                font.bold: true
                color: !MainViewViewModel.isTakeControl ? "#2d7d32" : "#6E6E6E"
                z: 2

                Behavior on color { ColorAnimation { duration: 200 } }
            }

            // "Take Control" label — right side
            Text {
                id: takeControlLabel
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 30
                text: "Take Control"
                font.pixelSize: 36
                font.family: "Inter"
                font.bold: true
                color: MainViewViewModel.isTakeControl ? "#2d7d32" : "#6E6E6E"
                z: 2

                Behavior on color { ColorAnimation { duration: 200 } }
            }

            // ── Fixed green knob — always centered ─────────────────
            Rectangle {
                id: switchKnob
                width: 110
                height: 110
                radius: 55
                anchors.centerIn: parent
                color: "#4caf50"
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.5; color: "#4caf50" }
                    GradientStop { position: !MainViewViewModel.isTakeControl? 1.0 : 0.0; color: "#4EC490" }
                }
                z: 3
            }

            // Click anywhere on the pill to toggle
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    MainViewViewModel.setControlMode(!MainViewViewModel.isTakeControl)
                }
            }
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
                        text: MainViewViewModel.isTakeControl ? "Manual Control" : "Exploring..."
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
