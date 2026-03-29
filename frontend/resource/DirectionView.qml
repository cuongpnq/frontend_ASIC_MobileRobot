import QtQuick 2.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root
    anchors.fill: parent

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onBackClicked: DirectionViewViewModel.requestMainView()
    }

    Rectangle {
        anchors.top: containerBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "#F5F4EF"
        z: -1
    }

    Text {
        anchors.centerIn: parent
        text: "Direction View (Blank)"
        font.pixelSize: 40
        color: "#2C2C2C"
    }

    // Back button logic is in ContainerBar, but we could add a local one if needed.
    // For now, it's blank as requested.
}
