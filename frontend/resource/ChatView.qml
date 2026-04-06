import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
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
            anchors.horizontalCenter: parent.horizontalCenter
            y: 120
            width: 1300
            height: 1105
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
        anchors.bottom: inputBarBackground.top
        anchors.margins: 20
        contentHeight: chatContainer.height
        contentWidth: width
        clip: true
        interactive: contentHeight > height

        Rectangle {
            id: chatContainer
            width: parent.width
            height: Math.max(mainFlickable.height, 700)
            color: "white"
            radius: 20
            border.color: '#80e0e0e0'
            border.width: 1

            Column {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 15

                Rectangle {
                    width: parent.width
                    height: 40
                    color: "transparent"
                    
                    Text {
                        text: "AI Chat Assistant"
                        font.pixelSize: 28
                        font.bold: true
                        color: "#2C2C2C"
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                    }

                    Item { Layout.fillWidth: true; width: 1 } // Spacer

                    Rectangle {
                        width: 100
                        height: 35
                        color: ChatViewViewModel.isThinking ? "#007AFF" : '#00fafafa'
                        radius: 10
                        border.color: ChatViewViewModel.isThinking ? "#007AFF" : '#920000'
                        border.width: 1
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        Text {
                            anchors.centerIn: parent
                            text: ChatViewViewModel.isThinking ? "Stop" : "Clear"
                            color: ChatViewViewModel.isThinking ? "white" : '#920000'
                            font.bold: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (ChatViewViewModel.isThinking)
                                    ChatViewViewModel.stopChat()
                                else
                                    ChatViewViewModel.clearHistory()
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: parent.height - 100 // Leave space for title
                    color: "#F9F9F9"
                    radius: 10
                    border.color: "#EEEEEE"

                    // Loading Indicator
                    Rectangle {
                        anchors.fill: parent
                        color: "#CCFFFFFF"
                        visible: ChatViewViewModel.isLoading
                        z: 10
                        Column {
                            anchors.centerIn: parent
                            spacing: 20
                            BusyIndicator {
                                anchors.horizontalCenter: parent.horizontalCenter
                                running: ChatViewViewModel.isLoading
                            }
                            Text {
                                text: "Optimizing AI Model for Jetson GPU...\n(Repacking tensors)"
                                horizontalAlignment: Text.AlignHCenter
                                font.pixelSize: 18
                                color: "#555555"
                            }
                        }
                    }

                    // Dismiss keyboard when clicking outside input
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            messageField.focus = false
                            ChatViewViewModel.dismissKeyboard()
                            mouse.accepted = false
                        }
                    }

                    ListView {
                        id: chatLog
                        anchors.fill: parent
                        anchors.margins: 10
                        model: ChatViewViewModel.messages
                        footer: Item {
                            width: chatLog.width
                            height: 40
                            visible: ChatViewViewModel.isThinking
                            Text {
                                anchors.centerIn: parent
                                text: "AI is thinking..."
                                font.italic: true
                                color: "#888888"
                            }
                        }
                        delegate: Column {
                            width: chatLog.width - 20
                            spacing: 5
                            Text {
                                text: modelData.sender
                                font.bold: true
                                font.pixelSize: 14
                                color: modelData.sender === "ASIC Chatbot" ? "#007AFF" : "#4CD964"
                            }
                            Text {
                                text: modelData.message
                                width: parent.width
                                wrapMode: Text.Wrap
                                font.pixelSize: 16
                            }
                            Item { height: 10; width: 1 }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: inputBarBackground
        anchors.bottom: inputPanel.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 40
        height: 80
        color: "white"
        border.color: "#EEEEEE"
        border.width: 1
        radius: 10

        Row {
            id: inputRow
            anchors.fill: parent
            anchors.margins: 15
            spacing: 10

            Rectangle {
                width: parent.width - 110
                height: parent.height
                color: "#F0F0F0"
                radius: 20
                TextField {
                    id: messageField
                    anchors.fill: parent
                    anchors.leftMargin: 15
                    anchors.rightMargin: 15
                    anchors.verticalCenter: parent.verticalCenter
                    placeholderText: !ChatViewViewModel.isLoaded ? (ChatViewViewModel.isLoading ? "Loading model..." : "Model not ready") : (ChatViewViewModel.isThinking ? "AI is thinking..." : "Type a message...")
                    font.pixelSize: 18
                    font.family: "Inter"
                    enabled: ChatViewViewModel.isLoaded && !ChatViewViewModel.isThinking
                    inputMethodHints: Qt.ImhNoPredictiveText
                    background: Rectangle {
                        color: "transparent"
                    }
                    onAccepted: {
                        ChatViewViewModel.sendMessage(messageField.text)
                        messageField.text = ""
                        messageField.focus = false 
                        ChatViewViewModel.dismissKeyboard()
                    }
                }
            }

            Rectangle {
                width: 90
                height: parent.height
                color: (ChatViewViewModel.isThinking || !ChatViewViewModel.isLoaded) ? "#CCCCCC" : "#007AFF"
                radius: 20
                enabled: ChatViewViewModel.isLoaded && !ChatViewViewModel.isThinking
                Text {
                    anchors.centerIn: parent
                    text: "Send"
                    color: "white"
                    font.bold: true
                }
                MouseArea {
                    anchors.fill: parent
                    enabled: parent.enabled
                    onClicked: {
                        ChatViewViewModel.sendMessage(messageField.text)
                        messageField.text = ""
                        messageField.focus = false
                        ChatViewViewModel.dismissKeyboard()
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
            // Jump to end of list
            chatLog.positionViewAtEnd()
            // And ensure flicker is at bottom
            mainFlickable.contentY = Math.max(0, mainFlickable.contentHeight - mainFlickable.height)
        }
    }
}
