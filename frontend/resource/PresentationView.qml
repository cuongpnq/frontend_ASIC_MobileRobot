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

    Text {
        id: titleText
        anchors.top: containerBar.bottom
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.topMargin: 70
        text: "Presentation"
        font.pixelSize: 96
        font.bold: true
        font.family: "Inter"
        color: "#000000"
    }

    // --- Action Buttons ---
    Row {
        id: actionButtons
        anchors.top: titleText.bottom
        anchors.left: parent.left
        anchors.leftMargin: 200
        anchors.topMargin: 30
        spacing: 20

        Rectangle {
            width: 220
            height: 60
            radius: 25
            color: "#c9d9d9d9"

            Text {
                anchors.centerIn: parent
                text: "Import"
                font.family: "Inter"
                font.bold: true
                font.pixelSize: 32
                color: "black"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: PresentationViewViewModel.startMobileImport()
            }
        }

        // Refresh Button
        Rectangle {
            width: 80
            height: 60
            radius: 25
            color: "#c9d9d9d9"

            Image {
                source: "images/refresh_icon.png"
                width: 30
                height: 30
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                anchors.fill: parent
                onClicked: PresentationViewViewModel.refreshFileList()
            }
        }
    }

    // Delete All Button (right-aligned)
    Rectangle {
        id: deleteAllBtn
        anchors.top: titleText.bottom
        anchors.right: parent.right
        anchors.topMargin: 30
        anchors.rightMargin: 200
        width: 200
        height: 60
        radius: 25
        color: deleteAllArea.pressed ? "#cc5555" : "#ff9999"

        Text {
            anchors.centerIn: parent
            text: "Delete All"
            font.family: "Inter"
            font.bold: true
            font.pixelSize: 32
            color: "black"
        }

        MouseArea {
            id: deleteAllArea
            anchors.fill: parent
            onClicked: PresentationViewViewModel.deleteAllFiles()
        }
    }

    // --- File List (styled like WiFi list) ---
    ListView {
        id: fileListView
        anchors.top: actionButtons.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 30
        anchors.leftMargin: 200
        anchors.rightMargin: 200
        anchors.bottomMargin: 40
        model: PresentationViewViewModel.fileList
        clip: true
        spacing: 20
        interactive: true

        delegate: Rectangle {
            width: fileListView.width
            height: 110
            color: "#c9d9d9d9"
            radius: 25

            // Tap to open file (lower z, delete button wins over its area)
            MouseArea {
                anchors.fill: parent
                onClicked: PresentationViewViewModel.openFile(modelData)
            }

            // File icon
            Text {
                id: fileIcon
                anchors.left: parent.left
                anchors.leftMargin: 30
                anchors.verticalCenter: parent.verticalCenter
                text: {
                    if (modelData.endsWith(".pptx") || modelData.endsWith(".ppt"))
                        return "📊"
                    return "📄"
                }
                font.pixelSize: 42
            }

            // File name
            Text {
                id: fileNameText
                anchors.left: fileIcon.right
                anchors.leftMargin: 20
                anchors.right: deleteBtn.left
                anchors.rightMargin: 20
                anchors.verticalCenter: parent.verticalCenter
                text: modelData
                font.pixelSize: 48
                font.family: "Inter"
                color: "black"
                elide: Text.ElideMiddle
            }

            // Delete button
            Rectangle {
                id: deleteBtn
                anchors.right: parent.right
                anchors.rightMargin: 30
                anchors.verticalCenter: parent.verticalCenter
                width: 70
                height: 70
                radius: 35
                color: deleteArea.pressed ? "#cc5555" : "#ff9999"

                Image {
                    anchors.centerIn: parent
                    source: "images/delete_icon.png"
                    width: 32
                    height: 32
                    fillMode: Image.PreserveAspectFit
                }

                MouseArea {
                    id: deleteArea
                    anchors.fill: parent
                    onClicked: PresentationViewViewModel.deleteFile(modelData)
                }
            }
        }

        // Empty state placeholder
        Text {
            anchors.centerIn: parent
            text: "No files uploaded yet.\nTap \"Import\" to add files."
            font.pixelSize: 32
            font.family: "Inter"
            color: "#AAAAAA"
            horizontalAlignment: Text.AlignHCenter
            visible: fileListView.count === 0
        }
    }

    // =========================================================================
    // File Viewer Popup (fullscreen overlay)
    // =========================================================================
    Rectangle {
        id: fileViewerOverlay
        anchors.fill: parent
        color: "#CC000000"
        visible: PresentationViewViewModel.isFileViewerOpen
        z: 100

        MouseArea { anchors.fill: parent } // block clicks through

        Rectangle {
            id: viewerCard
            anchors.centerIn: parent
            width: parent.width - 100
            height: parent.height - 100
            color: "white"
            radius: 25
            border.color: "#2C2C2C"
            border.width: 2

            // Header bar
            Rectangle {
                id: viewerHeader
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 80
                color: "#2C2C2C"
                radius: 25

                // Square off bottom corners
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 25
                    color: parent.color
                }

                Text {
                    anchors.centerIn: parent
                    text: PresentationViewViewModel.viewerFileName
                    font.pixelSize: 32
                    font.family: "Inter"
                    font.bold: true
                    color: "white"
                    elide: Text.ElideMiddle
                    width: parent.width - 200
                    horizontalAlignment: Text.AlignHCenter
                }

                // Close button
                Rectangle {
                    anchors.right: parent.right
                    anchors.rightMargin: 15
                    anchors.verticalCenter: parent.verticalCenter
                    width: 55
                    height: 55
                    radius: 28
                    color: closeArea.pressed ? "#555555" : "#444444"

                    Text {
                        anchors.centerIn: parent
                        text: "✕"
                        font.pixelSize: 30
                        color: "white"
                    }

                    MouseArea {
                        id: closeArea
                        anchors.fill: parent
                        onClicked: PresentationViewViewModel.closeFileViewer()
                    }
                }
            }

            // --- Content area ---
            Item {
                id: viewerBody
                anchors.top: viewerHeader.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 10

                // ── Converting indicator ─────────────────────
                Column {
                    anchors.centerIn: parent
                    spacing: 20
                    visible: PresentationViewViewModel.isConverting

                    BusyIndicator {
                        anchors.horizontalCenter: parent.horizontalCenter
                        running: PresentationViewViewModel.isConverting
                        width: 80
                        height: 80
                    }

                    Text {
                        text: "Converting presentation..."
                        font.pixelSize: 28
                        font.family: "Inter"
                        color: "#666666"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                // ── Error / fallback text ────────────────────
                Text {
                    anchors.centerIn: parent
                    text: PresentationViewViewModel.viewerContent
                    font.pixelSize: 28
                    font.family: "Inter"
                    color: "#cc0000"
                    wrapMode: Text.WordWrap
                    width: parent.width - 80
                    horizontalAlignment: Text.AlignHCenter
                    visible: !PresentationViewViewModel.isConverting
                             && PresentationViewViewModel.isSlideMode
                             && PresentationViewViewModel.viewerContent !== ""
                }

                // ── Text file viewer ─────────────────────────
                Flickable {
                    anchors.fill: parent
                    anchors.margins: 30
                    contentWidth: txtContent.width
                    contentHeight: txtContent.height
                    clip: true
                    visible: !PresentationViewViewModel.isSlideMode

                    Text {
                        id: txtContent
                        width: viewerBody.width - 60
                        text: PresentationViewViewModel.viewerContent
                        font.pixelSize: 30
                        font.family: "Inter"
                        color: "#333333"
                        wrapMode: Text.WordWrap
                    }

                    ScrollBar.vertical: ScrollBar { active: true }
                }

                // ── Slide viewer ─────────────────────────────
                Item {
                    anchors.fill: parent
                    visible: PresentationViewViewModel.isSlideMode
                             && !PresentationViewViewModel.isConverting
                             && PresentationViewViewModel.totalSlides > 0

                    Image {
                        id: slideImage
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: slideNav.top
                        anchors.margins: 10
                        source: PresentationViewViewModel.currentSlideImage
                        fillMode: Image.PreserveAspectFit
                        cache: false
                    }

                    // Navigation bar
                    Row {
                        id: slideNav
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 10
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 40

                        // Previous
                        Rectangle {
                            width: 160
                            height: 60
                            radius: 25
                            color: PresentationViewViewModel.currentSlideIndex > 0
                                   ? (prevArea.pressed ? "#888888" : "#c9d9d9d9")
                                   : "#e8e8e8"

                            Text {
                                anchors.centerIn: parent
                                text: "◀  Prev"
                                font.pixelSize: 28
                                font.family: "Inter"
                                font.bold: true
                                color: PresentationViewViewModel.currentSlideIndex > 0 ? "black" : "#aaaaaa"
                            }

                            MouseArea {
                                id: prevArea
                                anchors.fill: parent
                                enabled: PresentationViewViewModel.currentSlideIndex > 0
                                onClicked: PresentationViewViewModel.prevSlide()
                            }
                        }

                        // Slide counter
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: (PresentationViewViewModel.currentSlideIndex + 1) + " / " + PresentationViewViewModel.totalSlides
                            font.pixelSize: 32
                            font.family: "Inter"
                            font.bold: true
                            color: "#333333"
                        }

                        // Next
                        Rectangle {
                            width: 160
                            height: 60
                            radius: 25
                            color: PresentationViewViewModel.currentSlideIndex < PresentationViewViewModel.totalSlides - 1
                                   ? (nextArea.pressed ? "#888888" : "#c9d9d9d9")
                                   : "#e8e8e8"

                            Text {
                                anchors.centerIn: parent
                                text: "Next  ▶"
                                font.pixelSize: 28
                                font.family: "Inter"
                                font.bold: true
                                color: PresentationViewViewModel.currentSlideIndex < PresentationViewViewModel.totalSlides - 1 ? "black" : "#aaaaaa"
                            }

                            MouseArea {
                                id: nextArea
                                anchors.fill: parent
                                enabled: PresentationViewViewModel.currentSlideIndex < PresentationViewViewModel.totalSlides - 1
                                onClicked: PresentationViewViewModel.nextSlide()
                            }
                        }
                    }
                }
            }
        }
    }

    // =========================================================================
    // Mobile Import Popup
    // =========================================================================
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
