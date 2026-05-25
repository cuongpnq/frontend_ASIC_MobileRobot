import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root

    property string selectedLocation: "Home"
    property string pendingLocation: ""
    property int pendingCpId: -1
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
                DirectionViewViewModel.startNavigation(0)
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
            if (DirectionViewViewModel.isActive) {
                if (DirectionViewViewModel.autoReturnPending) {
                    DirectionViewViewModel.setAutoReturnPending(false)
                    autoHomeSeconds = 30
                    autoHomeTimer.start()
                } else if (DirectionViewViewModel.idleReturnPending) {
                    DirectionViewViewModel.setIdleReturnPending(false)
                    autoHomeSeconds = 15
                    autoHomeTimer.start()
                }
            } else {
                autoHomeTimer.stop()
            }
        }
        
        onMapIdChanged: {
            updateLocationModel()
        }
    }

    Component.onCompleted: {
        updateLocationModel()
    }

    function updateLocationModel() {
        locationModel.clear()
        var locs = DirectionViewViewModel.locations
        if (locs.length === 0) {
            locationModel.append({ "name": "Waiting for Map...", "room": "Waiting for Map...", "cpId": -1, "pctX": 0.0, "pctY": 0.0 })
        } else {
            for (var i = 0; i < locs.length; i++) {
                locationModel.append(locs[i])
            }
        }
    }

    ListModel {
        id: locationModel
        // Dynamically populated in updateLocationModel()
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

        Flickable {
            id: mapFlickable
            anchors.fill: parent
            anchors.margins: 20
            clip: true
            contentWidth: mapContainer.width
            contentHeight: mapContainer.height
            boundsBehavior: Flickable.StopAtBounds

            PinchArea {
                anchors.fill: parent
                z: -1 // Behind content
                pinch.target: null
                pinch.minimumScale: 1.0
                pinch.maximumScale: 5.0
                
                onPinchUpdated: {
                    var oldScale = mapContainer.zoomScale
                    mapContainer.zoomScale = Math.min(5.0, Math.max(1.0, mapContainer.zoomScale * pinch.scale / pinch.previousScale))
                    
                    var zoomFactor = mapContainer.zoomScale / oldScale
                    mapFlickable.contentX = (mapFlickable.contentX + pinch.center.x) * zoomFactor - pinch.center.x
                    mapFlickable.contentY = (mapFlickable.contentY + pinch.center.y) * zoomFactor - pinch.center.y
                }
            }

            MouseArea {
                anchors.fill: parent
                z: -1 // Behind content
                acceptedButtons: Qt.NoButton
                propagateComposedEvents: true
                onWheel: {
                    var oldScale = mapContainer.zoomScale
                    var zoomStep = wheel.angleDelta.y > 0 ? 1.1 : 0.9
                    mapContainer.zoomScale = Math.min(5.0, Math.max(1.0, mapContainer.zoomScale * zoomStep))
                    
                    var zoomFactor = mapContainer.zoomScale / oldScale
                    mapFlickable.contentX = (mapFlickable.contentX + wheel.x) * zoomFactor - wheel.x
                    mapFlickable.contentY = (mapFlickable.contentY + wheel.y) * zoomFactor - wheel.y
                }
            }

            Item {
                id: mapContainer
                width: mapFlickable.width * zoomScale
                height: mapFlickable.height * zoomScale
                
                property double zoomScale: 1.0
                onZoomScaleChanged: {
                    if (zoomScale <= 1.0) {
                        mapFlickable.contentX = 0
                        mapFlickable.contentY = 0
                    }
                }
                
                Image {
                    id: mapImage
                    anchors.fill: parent
                    source: DirectionViewViewModel.mapImage
                    fillMode: Image.PreserveAspectFit
                    
                    onStatusChanged: {
                        if (status === Image.Ready) {
                            mapContainer.zoomScale = 1.0
                        }
                    }

                    // MouseArea {
                    //     anchors.fill: parent
                    //     onClicked: {
                    //         var pctX = (mouse.x - (mapImage.width - mapImage.paintedWidth) / 2) / mapImage.paintedWidth
                    //         var pctY = (mouse.y - (mapImage.height - mapImage.paintedHeight) / 2) / mapImage.paintedHeight
                    //         console.log("Map Clicked - ID: " + DirectionViewViewModel.mapId + " pctX: " + pctX.toFixed(4) + ", pctY: " + pctY.toFixed(4))
                    //     }
                    // }

                    Repeater {
                        model: locationModel
                        delegate: Item {
                            z: 100 // High z-index to ensure clickability
                            // Coordinate mapping using parent.width/height (mapContainer/mapImage)
                            x: (mapImage.width - mapImage.paintedWidth) / 2 + mapImage.paintedWidth * model.pctX - width / 2
                            y: (mapImage.height - mapImage.paintedHeight) / 2 + mapImage.paintedHeight * model.pctY - height / 2
                            width: DirectionViewViewModel.mapId === "a1" ? 30 : 80
                            height: DirectionViewViewModel.mapId === "a1" ? 40 : 80
                            visible: model.pctX > 0.0 && model.pctY > 0.0

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (model.room === selectedLocation) {
                                        sameLocationPopup.open()
                                    } else {
                                        pendingLocation = model.room
                                        pendingCpId = model.cpId
                                        confirmPopup.open()
                                    }
                                }
                            }
                        }
                    }
                }
            }


        }
    }

    Text {
        id: mapText
        x: mapBackground.x+35
        y: mapBackground.y+25
        text: "Map"
        font.pixelSize: 48
        font.bold: true
        color: '#000000'
        z: 10
    }

    Rectangle {
        id: switchButton
        anchors.top: mapBackground.top
        anchors.right: mapBackground.right
        anchors.topMargin: 25
        anchors.rightMargin: 35
        width: 140
        height: 50
        color: "#3498db"
        radius: 8
        z: 10
        visible: DirectionViewViewModel.availableMaps.length > 1 && UserManager.currentRole > UserManager.User

        Text {
            anchors.centerIn: parent
            text: "Switch"
            color: "white"
            font.pixelSize: 24
            font.bold: true
        }

        MouseArea {
            anchors.fill: parent
            onClicked: mapSwitchPopup.open()
        }
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

    Text {
        id: listRoomText
        anchors.bottom: listRoom.top
        anchors.left: listRoom.left
        anchors.margins: 10
        visible: DirectionViewViewModel.mapId === "a1"
        text: "List room"
        font.pixelSize: 24
        font.bold: true
        font.family: "Inter"
        color: "#2C2C2C"
    }
    Rectangle {
        id: listRoom
        width: mapBackground.width/3.5
        height: 200
        anchors.top: mapText.bottom
        anchors.left: mapText.left
        anchors.topMargin: listRoomText.height + 30
        color: '#b9efefef'
        visible: DirectionViewViewModel.mapId === "a1"
        radius: 25
        
        ListView {
            anchors.fill: parent
            anchors.margins: 10
            model: locationModel
            clip: true
            delegate: Item {
                width: parent.width
                height: 60
                
                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 15
                    anchors.verticalCenter: parent.verticalCenter
                    text: model.name
                    font.pixelSize: 20
                    font.family: "Inter"
                    color: "#2C2C2C"
                }
            }
            
            ScrollIndicator.vertical: ScrollIndicator {
                anchors.right: parent.right
                anchors.margins: 2
            }
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

            // Automatically close when standby is triggered
            Connections {
                target: window
                onIsStandbyChanged: {
                    if (window.isStandby) locationPopup.close()
                }
            }
            
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
                        text: model.room
                        font.pixelSize: 24
                        color: "#333333"
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        onClicked: {
                            if (model.room === selectedLocation) {
                                locationPopup.close()
                                sameLocationPopup.open()
                            } else {
                                pendingLocation = model.room
                                pendingCpId = model.cpId
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

        // Automatically close when standby is triggered
        Connections {
            target: window
            onIsStandbyChanged: {
                if (window.isStandby) sameLocationPopup.close()
            }
        }

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

        // Automatically close when standby is triggered
        Connections {
            target: window
            onIsStandbyChanged: {
                if (window.isStandby) confirmPopup.close()
            }
        }

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
                            pendingCpId = -1
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
                                DirectionViewViewModel.startNavigation(pendingCpId)
                                pendingCpId = -1
                                DirectionViewViewModel.requestRunningView()
                            }
                        }
                }
            }
        }
    }

    Popup {
        id: mapSwitchPopup
        anchors.centerIn: parent
        width: 400
        height: Math.min(DirectionViewViewModel.availableMaps.length * 80 + 100, 600)
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#f8f9fa"
            radius: 20
            border.color: "#dee2e6"
            border.width: 2
        }

        Column {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20

            Text {
                text: "Select Map"
                font.pixelSize: 32
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
            }

            ListView {
                width: parent.width
                height: parent.height - 80
                model: DirectionViewViewModel.availableMaps
                clip: true
                spacing: 10

                delegate: Rectangle {
                    width: parent.width
                    height: 70
                    color: modelData === DirectionViewViewModel.mapId ? "#e9ecef" : "white"
                    radius: 10
                    border.color: modelData === DirectionViewViewModel.mapId ? "#3498db" : "#ced4da"
                    border.width: modelData === DirectionViewViewModel.mapId ? 2 : 1

                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        font.pixelSize: 26
                        font.bold: modelData === DirectionViewViewModel.mapId
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            DirectionViewViewModel.setMapId(modelData)
                            mapSwitchPopup.close()
                        }
                    }
                }
            }
        }
    }
}
