import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root
    anchors.fill: parent

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onBackClicked: PresentationViewViewModel.requestMainView()
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

    // --- Action Buttons ---
    Row {
        id: actionButtons
        anchors.top: containerBar.bottom
        anchors.right: parent.right
        anchors.topMargin: 20
        anchors.rightMargin: 100
        spacing: 20

        // Mobile Import Button
        Button {
            id: mobileImportBtn
            text: "Import"
            font.pixelSize: 24
            contentItem: Text {
                text: mobileImportBtn.text
                font: mobileImportBtn.font
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                implicitWidth: 300
                implicitHeight: 60
                color: mobileImportBtn.pressed ? '#505050' : '#888888'
                radius: 10
            }
            onClicked: PresentationViewViewModel.startMobileImport()
        }
    }

    Rectangle {
        id: contentBox
        anchors.top: actionButtons.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 20
        anchors.leftMargin: 100
        anchors.rightMargin: 100
        anchors.bottomMargin: 100
        color: "#FFFFFF"
        radius: 35
        border.color: "#2C2C2C"
        border.width: 1

        Flickable {
            id: flickable
            anchors.fill: parent
            anchors.margins: 40
            contentWidth: textContent.width
            contentHeight: textContent.height
            clip: true

            Text {
                id: textContent
                width: flickable.width
                text: PresentationViewViewModel.content
                font.pixelSize: 32
                font.family: "Inter"
                color: "#333333"
                wrapMode: Text.WordWrap
            }

            Text {
                id: placeholderText
                anchors.centerIn: parent
                text: "Please import text file"
                font.pixelSize: 32
                font.family: "Inter"
                color: "#AAAAAA"
                visible: textContent.text === ""
            }

            ScrollBar.vertical: ScrollBar {
                active: true
            }
        }
    }

    // --- Mobile Import Popup ---
    Popup {
        id: mobileImportPopup
        anchors.centerIn: parent
        width: 600
        height: 700
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        visible: PresentationViewViewModel.isMobileImportActive
        onClosed: PresentationViewViewModel.stopMobileImport()

        // Automatically close when standby is triggered
        Connections {
            target: window
            onIsStandbyChanged: {
                if (window.isStandby) mobileImportPopup.close()
            }
        }

        background: Rectangle {
            color: "white"
            radius: 25
            border.color: "#2C2C2C"
            border.width: 2
        }

        Column {
            anchors.centerIn: parent
            spacing: 30
            width: parent.width * 0.8

            Text {
                text: "Scan to Upload File"
                font.pixelSize: 36
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Image {
                id: qrCodeImage
                width: 400
                height: 400
                source: PresentationViewViewModel.qrCodeUrl
                anchors.horizontalCenter: parent.horizontalCenter
                fillMode: Image.PreserveAspectFit
                
                // Add a placeholder while loading
                Rectangle {
                    anchors.fill: parent
                    color: "#f0f0f0"
                    visible: qrCodeImage.status !== Image.Ready
                    Text {
                        anchors.centerIn: parent
                        text: "Generating QR..."
                    }
                }
            }

            Text {
                text: PresentationViewViewModel.uploadStatus
                font.pixelSize: 24
                color: "#666666"
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Button {
                text: "Cancel"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: mobileImportPopup.close()
            }
        }
    }
}
