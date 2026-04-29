import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root

    property string selectedLocation: "Home"
    property string pendingLocation: ""
    property int autoHomeSeconds: 30

    // ── Auto-home countdown timer ─────────────────────────────────────
    Timer {
        id: autoHomeTimer
        interval: 1000
        repeat: true
        running: false
        onTriggered: {
            if (autoHomeSeconds <= 1) {
                autoHomeTimer.stop()
                selectedLocation = "Home"
                DirectionViewViewModel.requestRunningView()
            } else {
                autoHomeSeconds -= 1
            }
        }
    }

    // Watch DirectionViewViewModel.isActive to start/stop the timer
    Connections {
        target: DirectionViewViewModel
        onIsActiveChanged: {
            if (DirectionViewViewModel.isActive && DirectionViewViewModel.autoReturnPending) {
                DirectionViewViewModel.setAutoReturnPending(false)
                autoHomeSeconds = 30
                autoHomeTimer.start()
            } else {
                autoHomeTimer.stop()
            }
        }
    }

    ListModel {
        id: locationModel
        ListElement { name: "Meeting Room (E1.1)"; cpId: 0 }
        ListElement { name: "Elevator"; cpId: 1 }
        ListElement { name: "CELUiT's Office"; cpId: 3 }
    }

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onBackClicked: DirectionViewViewModel.requestMainView()
    }

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
    
    Rectangle {
        id: mapBackground
        anchors.top: containerBar.bottom
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.margins: 50
        radius: 32
        color: '#80dad8d8'
    }

    Text {
        x: mapBackground.x+35
        y: mapBackground.y+25
        text: "Map"
        font.pixelSize: 48
        font.bold: true
        color: '#000000'
    }

    // ── Auto-home countdown hint ──────────────────────────────────────
    Text {
        id: autoHomeHint
        anchors.horizontalCenter: mapBackground.horizontalCenter
        anchors.bottom: mapBackground.bottom
        anchors.bottomMargin: 28
        text: "Select new location — returning Home in " + autoHomeSeconds + " s…"
        font.pixelSize: 22
        font.family: "Inter"
        color: '#2C2C2C'
        visible: autoHomeTimer.running

        SequentialAnimation on opacity {
            running: autoHomeTimer.running
            loops: Animation.Infinite
            NumberAnimation { to: 0.45; duration: 600; easing.type: Easing.InOutSine }
            NumberAnimation { to: 0.90; duration: 600; easing.type: Easing.InOutSine }
        }
    }

    Image {
        id: mapButton
        anchors.bottom: mapBackground.bottom
        anchors.right: mapBackground.right
        anchors.bottomMargin: 25
        anchors.rightMargin: 25
        width: 100
        height: 100
        source: "qrc:/images/map_button.png"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: {
                locationPopup.open()
            }
        }

        Popup {
            id: locationPopup
            x: -width - 20
            y: (parent.height - height) / 2
            width: 300
            height: Math.min(locationModel.count * 60 + 20, 400)
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
            
            background: Rectangle {
                color: "#ffffff"
                radius: 15
                border.color: "#cccccc"
                border.width: 1
            }

            ListView {
                anchors.fill: parent
                anchors.margins: 10
                model: locationModel
                clip: true
                delegate: Item {
                    width: parent.width
                    height: 60
                    
                    Rectangle {
                        anchors.fill: parent
                        color: mouseArea.pressed ? "#e0e0e0" : "transparent"
                        radius: 10
                    }
                    
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: model.name
                        font.pixelSize: 24
                        color: "#333333"
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        onClicked: {
                            if (model.name === selectedLocation) {
                                locationPopup.close()
                                sameLocationPopup.open()
                            } else {
                                pendingLocation = model.name
                                locationPopup.close()
                                confirmPopup.open()
                            }
                        }
                    }
                }
            }
        }
    }

    // Error popup when trying to select the current location
    Popup {
        id: sameLocationPopup
        anchors.centerIn: parent
        width: parent.width * 0.5
        height: 200
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#ffffff"
            radius: 20
            border.color: "#cccccc"
            border.width: 1

            Rectangle {
                anchors.fill: parent
                anchors.margins: -4
                color: "transparent"
                border.color: "#22000000"
                border.width: 4
                radius: 24
                z: -1
            }
        }

        Column {
            anchors.centerIn: parent
            spacing: 24
            width: parent.width - 48

            Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                text: "Your current destination is " + selectedLocation
                font.pixelSize: 26
                font.bold: true
                color: "#CC3333"
            }

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                width: 150
                height: 52
                radius: 12
                color: okArea.pressed ? "#e0e0e0" : "#f0f0f0"
                border.color: "#bbbbbb"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "OK"
                    font.pixelSize: 20
                    color: "#444444"
                }

                MouseArea {
                    id: okArea
                    anchors.fill: parent
                    onClicked: {
                        sameLocationPopup.close()
                    }
                }
            }
        }
    }

    // Confirmation popup
    Popup {
        id: confirmPopup
        anchors.centerIn: parent
        width: parent.width * 0.5
        height: 240
        modal: true
        closePolicy: Popup.CloseOnEscape

        background: Rectangle {
            color: "#ffffff"
            radius: 20
            border.color: "#cccccc"
            border.width: 1

            // Subtle drop shadow effect via a slightly larger behind rectangle
            Rectangle {
                anchors.fill: parent
                anchors.margins: -4
                color: "transparent"
                border.color: "#22000000"
                border.width: 4
                radius: 24
                z: -1
            }
        }

        Column {
            anchors.centerIn: parent
            spacing: 24
            width: parent.width - 48

            // Title
            Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                text: "Do you want Robot arrive to '" + pendingLocation + "'?"
                font.pixelSize: 26
                font.bold: true
                color: "#1a1a1a"
            }

            // Buttons row
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 20

                // Cancel button
                Rectangle {
                    width: 150
                    height: 52
                    radius: 12
                    color: cancelArea.pressed ? "#e0e0e0" : "#f0f0f0"
                    border.color: "#bbbbbb"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "No"
                        font.pixelSize: 20
                        color: "#444444"
                    }

                    MouseArea {
                        id: cancelArea
                        anchors.fill: parent
                        onClicked: {
                            confirmPopup.close()
                            pendingLocation = ""
                        }
                    }
                }

                // Confirm button
                Rectangle {
                    width: 150
                    height: 52
                    radius: 12
                    color: confirmArea.pressed ? "#1565c0" : "#1976d2"

                    Text {
                        anchors.centerIn: parent
                        text: "Yes"
                        font.pixelSize: 20
                        font.bold: true
                        color: "#ffffff"
                    }

                    MouseArea {
                            id: confirmArea
                            anchors.fill: parent
                            onClicked: {
                                autoHomeTimer.stop()
                                selectedLocation = pendingLocation
                                confirmPopup.close()
                                pendingLocation = ""
                                DirectionViewViewModel.startNavigation(model.cpId)
                                DirectionViewViewModel.requestRunningView()
                            }
                        }
                }
            }
        }
    }

    // Back button logic is in ContainerBar, but we could add a local one if needed.
    // For now, it's blank as requested.
}
