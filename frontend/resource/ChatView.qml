import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.VirtualKeyboard 2.1
import QtQuick.VirtualKeyboard.Settings 2.1
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root
    anchors.fill: parent

    Component.onCompleted: {
        VirtualKeyboardSettings.styleName = "default"
    }

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onBackClicked: ChatViewViewModel.requestMainView()

        MouseArea {
            anchors.fill: parent
            z: -1 // Behind the back button but on top of bar background
            onClicked: ChatViewViewModel.dismissKeyboard()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#F5F4EF"
        z: -1

        Image {
            x: 220
            y: 110
            source: "images/UIT_logo.png"
            opacity: 0.15
        }

        MouseArea {
            anchors.fill: parent
            onClicked: ChatViewViewModel.dismissKeyboard()
        }
    }

    Flickable {
        id: mainFlickable
        anchors.top: containerBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: inputPanel.top
        contentHeight: contentWrapper.height + inputRow.height
        contentWidth: width
        clip: true
        interactive: contentHeight > height

        Item {
            id: contentWrapper
            width: mainFlickable.width
            height: Math.max(mainFlickable.height, 500)

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    ChatViewViewModel.dismissKeyboard()
                    mouse.accepted = false
                }
                propagateComposedEvents: true
            }

            Rectangle {
                id: chatContainer
                anchors.centerIn: parent
                width: 800
                height: 500
                color: "white"
                radius: 20
                border.color: '#80e0e0e0'
                border.width: 1

                Column {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    Text {
                        text: "AI Chat Assistant"
                        font.pixelSize: 28
                        font.bold: true
                        color: "#2C2C2C"
                    }

                    Rectangle {
                        width: parent.width
                        height: 350
                        color: "#F9F9F9"
                        radius: 10
                        border.color: "#EEEEEE"

                        ListView {
                            id: chatLog
                            anchors.fill: parent
                            anchors.margins: 10
                            model: ListModel {
                                ListElement { sender: "Robot"; message: "Hello! How can I help you today?" }
                                ListElement { sender: "User"; message: "Where are you currently located?" }
                                ListElement { sender: "Robot"; message: "I am in the main hallway, heading towards the charging station." }
                            }
                            delegate: Column {
                                width: chatLog.width - 20
                                spacing: 5
                                Text {
                                    text: model.sender
                                    font.bold: true
                                    font.pixelSize: 14
                                    color: model.sender === "Robot" ? "#007AFF" : "#4CD964"
                                }
                                Text {
                                    text: model.message
                                    width: parent.width
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 16
                                }
                                Item { height: 10; width: 1 }
                            }
                        }
                    }

                    Row {
                        id: inputRow
                        width: parent.width
                        height: 40
                        spacing: 10

                        Rectangle {
                            width: parent.width - 100
                            height: parent.height
                            color: "#F0F0F0"
                            radius: 20
                            TextField {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                anchors.verticalCenter: parent.verticalCenter
                                placeholderText: "Type a message..."
                                font.pixelSize: 16
                                font.family: "Inter"
                                background: Rectangle {
                                    color: "transparent"
                                }
                            }
                        }

                        Rectangle {
                            width: 90
                            height: parent.height
                            color: "#007AFF"
                            radius: 20
                            Text {
                                anchors.centerIn: parent
                                text: "Send"
                                color: "white"
                                font.bold: true
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    InputPanel {
        id: inputPanel
        width: parent.width
        y: ChatViewViewModel.keyboardVisible ? parent.height - inputPanel.height : parent.height
        z: 10000 
        visible: ChatViewViewModel.hasVirtualKeyboard
    }

    Connections {
        target: ChatViewViewModel
        onRequestScrollToBottom: {
            mainFlickable.contentY = Math.max(0, mainFlickable.contentHeight - mainFlickable.height)
        }
    }
}
