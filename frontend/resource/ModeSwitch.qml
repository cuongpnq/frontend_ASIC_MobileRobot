import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0
import QtGraphicalEffects 1.12

Item {
    id: modeSwitchContainer
    width: 840
    height: 180

    DropShadow {
        anchors.fill: bgRect
        horizontalOffset: 9
        verticalOffset: 14
        radius: 25.6
        samples: 32
        color: "#40000000"
        source: bgRect
    }

    Rectangle {
        id: bgRect
        anchors.fill: parent
        radius: 99
        color: "#F2FEFE"
        border.color: "#c0b8b0"
        border.width: 1.5
    }

    // ── Sliding track (highlight) — moves left or right ───
    Rectangle {
        id: switchTrack
        width: 465
        height: 140
        radius: 99
        anchors.verticalCenter: parent.verticalCenter
        x: ModeSwitchViewModel.isTakeControl ? parent.width - 465 - 20 : 20
        color: "#FFFFFF"
        border.color: "#4caf50"
        border.width: 2

        Behavior on x {
            NumberAnimation {
                id: trackAnimation
                duration: 200
                easing.type: Easing.InOutCubic
            }
        }
    }
    DropShadow {
        id: switchTrackShadow
        visible: !trackAnimation.running
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
        color: !ModeSwitchViewModel.isTakeControl ? "#2d7d32" : "#6E6E6E"
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
        color: ModeSwitchViewModel.isTakeControl ? "#2d7d32" : "#6E6E6E"
        z: 2

        Behavior on color { ColorAnimation { duration: 200 } }
    }

    // ── Fixed green knob — always centered ─────────────────
    Rectangle {
        id: switchKnob
        width: 100
        height: 100
        radius: 55
        border.color: "#4EC490"
        border.width: 2
        anchors.centerIn: parent
        color: "#4caf50"
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.5; color: "#4caf50" }
            GradientStop { position: !ModeSwitchViewModel.isTakeControl? 1.0 : 0.0; color: "#4EC490" }
        }
        z: 3
    }
    DropShadow {
        visible: !trackAnimation.running
        anchors.fill: switchKnob
        source: switchKnob
        horizontalOffset: 4
        verticalOffset: 4
        radius: 20
        samples: 32
        color: "#4059BA94"
    }
    DropShadow {
        visible: !trackAnimation.running
        anchors.fill: switchKnob
        source: switchKnob
        horizontalOffset: -4
        verticalOffset: -4
        radius: 20
        samples: 32
        color: "#4059BA94"
    }

    // Click anywhere on the pill to toggle
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            ModeSwitchViewModel.setControlMode(!ModeSwitchViewModel.isTakeControl)
        }
    }
}
