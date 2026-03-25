import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import com.asic.mobilerobot.viewmodels 1.0

Rectangle {
    id: containerBar
    width: parent.width
    height: 36
    color: "#F8F7F3"

    // ── Time (centered) ──
    Text {
        id: timeText
        anchors.centerIn: parent
        text: ContainerBarViewModel.currentTime
        font.pixelSize: 14
        font.bold: true
        font.family: "monospace"
        color: "#1a1a1a"
    }

    // ── WiFi + Battery (right side) ──
    Row {
        id: statusRow
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        spacing: 14

        // WiFi icon (drawn with Canvas)
        Item {
            id: wifiIcon
            width: 18
            height: 18
            anchors.verticalCenter: parent.verticalCenter

            Canvas {
                anchors.fill: parent
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)

                    var connected = ContainerBarViewModel.wifiConnected
                    var color = connected ? "#4caf50" : "#f44336"

                    ctx.strokeStyle = color
                    ctx.lineWidth = 1.8
                    ctx.lineCap = "round"

                    // Draw wifi arcs (3 arcs from bottom center)
                    var cx = width / 2
                    var cy = height

                    // Arc 1 (smallest)
                    if (connected) {
                        ctx.beginPath()
                        ctx.arc(cx, cy, 4, Math.PI * 1.25, Math.PI * 1.75, false)
                        ctx.stroke()

                        // Arc 2 (medium)
                        ctx.beginPath()
                        ctx.arc(cx, cy, 8, Math.PI * 1.25, Math.PI * 1.75, false)
                        ctx.stroke()

                        // Arc 3 (largest)
                        ctx.beginPath()
                        ctx.arc(cx, cy, 12, Math.PI * 1.25, Math.PI * 1.75, false)
                        ctx.stroke()
                    }

                    // Center dot
                    ctx.fillStyle = color
                    ctx.beginPath()
                    ctx.arc(cx, cy - 1, 1.8, 0, Math.PI * 2)
                    ctx.fill()
                }

                // Repaint when wifi status changes
                Connections {
                    target: ContainerBarViewModel
                    onWifiStatusChanged: wifiIcon.children[0].requestPaint()
                }
            }
        }

        // Battery icon (drawn with Canvas)
        Item {
            id: batteryIcon
            width: 28
            height: 14
            anchors.verticalCenter: parent.verticalCenter

            Canvas {
                anchors.fill: parent
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)

                    var level = ContainerBarViewModel.batteryLevel
                    var bodyWidth = width - 4
                    var bodyHeight = height
                    var tipWidth = 3
                    var tipHeight = 6

                    // Battery body outline
                    ctx.strokeStyle = "#1a1a1a"
                    ctx.lineWidth = 1.5
                    ctx.beginPath()
                    ctx.roundedRect(0.5, 0.5, bodyWidth - 1, bodyHeight - 1, 2, 2)
                    ctx.stroke()

                    // Battery tip (positive terminal)
                    ctx.fillStyle = "#1a1a1a"
                    ctx.beginPath()
                    ctx.roundedRect(bodyWidth, (bodyHeight - tipHeight) / 2, tipWidth, tipHeight, 1, 1)
                    ctx.fill()

                    // Battery fill level
                    var fillWidth = (bodyWidth - 4) * (level / 100)
                    var fillColor = "#4caf50"
                    if (level <= 20) fillColor = "#f44336"
                    else if (level <= 50) fillColor = "#ff9800"

                    ctx.fillStyle = fillColor
                    ctx.beginPath()
                    ctx.roundedRect(2, 2, fillWidth, bodyHeight - 4, 1, 1)
                    ctx.fill()
                }

                // Repaint when battery level changes
                Connections {
                    target: ContainerBarViewModel
                    onBatteryLevelChanged: batteryIcon.children[0].requestPaint()
                }
            }

            // Battery percentage text
            Text {
                anchors.left: parent.right
                anchors.leftMargin: 4
                anchors.verticalCenter: parent.verticalCenter
                text: ContainerBarViewModel.batteryLevel + "%"
                font.pixelSize: 11
                color: "#1a1a1a"
            }
        }
    }
}
