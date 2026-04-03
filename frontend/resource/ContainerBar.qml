import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import com.asic.mobilerobot.viewmodels 1.0
import QtGraphicalEffects 1.12

Item {
    id: containerBar
    width: parent.width
    height: 100
    z: 10

    signal backClicked()

    Rectangle {
        id: bgRect
        anchors.fill: parent
        color: "#F8F7F3"
    }

    DropShadow {
        anchors.fill: bgRect
        horizontalOffset: 0
        verticalOffset: 10
        radius: 16
        samples: 32
        color: "#40000000"
        source: bgRect
    }

    Image {
        id: backButton
        source: "qrc:/images/back_button.png"
        width: 60
        height: 60
        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.verticalCenter: parent.verticalCenter
        visible: !MainViewViewModel.isActive

        MouseArea {
            anchors.fill: parent
            onClicked: {
                containerBar.backClicked()
            }
        }
    }
    
    // ── Time (centered) ──
    Text {
        id: timeText
        anchors.centerIn: parent
        text: ContainerBarViewModel.currentTime
        font.pixelSize: 48
        font.bold: true
        font.family: "Inter"
        color: "#000000"
    }

    // ── WiFi + Battery (right side) ──
    Row {
        id: statusRow
        anchors.right: parent.right
        anchors.rightMargin: 40
        anchors.verticalCenter: parent.verticalCenter
        spacing: 24

        // WiFi icon (drawn with Canvas)
        Item {
            id: wifiIcon
            width: 45
            height: 35
            anchors.verticalCenter: parent.verticalCenter

            Canvas {
                anchors.fill: parent
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)

                    var connected = ContainerBarViewModel.wifiConnected
                    var color = connected ? "#000000" : '#757575'

                    ctx.strokeStyle = color
                    ctx.lineWidth = 3
                    ctx.lineCap = "round"

                    // Draw wifi arcs
                    var cx = width / 2
                    var cy = height - 4

                    if (connected) {
                        ctx.beginPath()
                        ctx.arc(cx, cy, 9, Math.PI * 1.25, Math.PI * 1.75, false)
                        ctx.stroke()

                        ctx.beginPath()
                        ctx.arc(cx, cy, 18, Math.PI * 1.25, Math.PI * 1.75, false)
                        ctx.stroke()

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

                // Repaint when wifi status changes
                Connections {
                    target: ContainerBarViewModel
                    onWifiStatusChanged: wifiIcon.children[0].requestPaint()
                }
            }
        }

        // Battery group (Text and Icon)
        Row {
            spacing: 12
            anchors.verticalCenter: parent.verticalCenter

            // Battery percentage text
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: ContainerBarViewModel.batteryLevel + "%"
                font.pixelSize: 48
                font.family: "Inter"
                color: "#000000"
            }

            // Battery icon (drawn with Canvas)
            Item {
                id: batteryIcon
                width: 100
                height: 50
                anchors.verticalCenter: parent.verticalCenter
                
                Canvas {
                    id: batteryCanvas
                    anchors.fill: parent
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)

                        var level = ContainerBarViewModel.batteryLevel
                        var bodyWidth = width - 6
                        var bodyHeight = height
                        var tipWidth = 6
                        var tipHeight = 14

                        // Battery body outline
                        ctx.strokeStyle = "#1a1a1a"
                        ctx.lineWidth = 3
                        ctx.beginPath()
                        ctx.roundedRect(1.5, 1.5, bodyWidth - 3, bodyHeight - 3, 4, 4)
                        ctx.stroke()

                        // Battery tip (positive terminal)
                        ctx.fillStyle = "#1a1a1a"
                        ctx.beginPath()
                        ctx.roundedRect(bodyWidth, (bodyHeight - tipHeight) / 2, tipWidth, tipHeight, 2, 2)
                        ctx.fill()

                        // Battery fill level
                        var fillWidth = (bodyWidth - 4) * (level / 100)
                        var fillColor = "#4caf50"
                        if (level <= 20) fillColor = "#f44336"
                        else if (level <= 50) fillColor = "#ff9800"

                        ctx.fillStyle = fillColor
                        ctx.beginPath()
                        ctx.roundedRect(4, 4, Math.max(0, fillWidth - 5), bodyHeight - 8, 2, 2)
                        ctx.fill()
                    }

                    // Repaint when battery level changes
                    Connections {
                        target: ContainerBarViewModel
                        onBatteryLevelChanged: batteryCanvas.requestPaint()
                    }
                }
            }
        }
    }
}
