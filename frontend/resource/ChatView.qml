import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.VirtualKeyboard 2.1
import QtQuick.VirtualKeyboard.Settings 2.1
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root
    anchors.fill: parent

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onBackClicked: ChatViewViewModel.requestMainView()

        MouseArea {
            anchors.fill: parent
            z: -1
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

    Rectangle {
        id: chatContainer
        anchors.top: containerBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: sampleQuestionsRow.visible ? sampleQuestionsRow.top : inputBarBackground.top
        anchors.margins: 20
        anchors.bottomMargin: sampleQuestionsRow.visible ? 10 : 20
        color: "white"
        radius: 20
        border.color: '#80e0e0e0'
        border.width: 1
        clip: true

        Rectangle {
            id: titleBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 20
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            height: 40
            color: "transparent"
            z: 5
            
            Text {
                text: "AI Chat Assistant"
                font.pixelSize: 28
                font.bold: true
                color: "#2C2C2C"
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 10
            }

            Rectangle {
                width: 100
                height: 35
                color: (ChatViewViewModel.isThinking || ChatViewViewModel.isGenerating) ? '#ff0000' : '#00fafafa'
                radius: 10
                border.color: (ChatViewViewModel.isThinking || ChatViewViewModel.isGenerating) ? '#ff0000' : '#920000'
                border.width: 1
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 10
                Text {
                    anchors.centerIn: parent
                    text: (ChatViewViewModel.isThinking || ChatViewViewModel.isGenerating) ? "Stop" : "Clear"
                    color: (ChatViewViewModel.isThinking || ChatViewViewModel.isGenerating) ? "white" : '#920000'
                    font.bold: true
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (ChatViewViewModel.isThinking || ChatViewViewModel.isGenerating)
                            ChatViewViewModel.stopChat()
                        else
                            ChatViewViewModel.clearHistory()
                    }
                }
            }
        }

        ListView {
            id: mainFlickable
            anchors.top: titleBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.topMargin: 15
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            anchors.bottomMargin: 20
            clip: true
            model: ChatViewViewModel.messages
            spacing: 15

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
                z: -1
                onClicked: {
                    messageField.focus = false
                    ChatViewViewModel.dismissKeyboard()
                }
            }

            footer: Item {
                width: mainFlickable.width
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
                width: mainFlickable.width - 20
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
                    color: "#2C2C2C"
                }
                Item { height: 10; width: 1 }
            }
        }
    }
    
    // ── Sample Questions ──────────────────────────────────────────
    Row {
        id: sampleQuestionsRow
        anchors.bottom: inputBarBackground.top
        anchors.left: inputBarBackground.left
        anchors.bottomMargin: 15
        spacing: 15
        visible: ChatViewViewModel.isLoaded && !ChatViewViewModel.isThinking && !ChatViewViewModel.isGenerating && !Qt.inputMethod.visible

        Repeater {
            model: [
                "What is the established date of UIT?",
                "Who is Dean of Computer Engineering department?",
                "Tell me about UIT",
                "Tell me about faculty of Computer Engineering"
            ]
            delegate: Rectangle {
                height: 50
                width: questionText.implicitWidth + 40
                radius: 25
                color: '#00ffffff'
                border.color: sampleArea.pressed ? "#007AFF" : "#E0E0E0"
                border.width: 1

                Text {
                    id: questionText
                    anchors.centerIn: parent
                    text: modelData
                    font.pixelSize: 16
                    font.family: "Inter"
                    color: "#007AFF"
                }

                MouseArea {
                    id: sampleArea
                    anchors.fill: parent
                    onClicked: ChatViewViewModel.sendMessage(modelData)
                }
            }
        }
    }

    Rectangle {
        id: inputBarBackground
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Qt.inputMethod.visible ? Qt.inputMethod.keyboardRectangle.height : 20
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
                    placeholderText: !ChatViewViewModel.isLoaded ? (ChatViewViewModel.isLoading ? "Loading model..." : "Model not ready") : ((ChatViewViewModel.isThinking || ChatViewViewModel.isGenerating) ? "AI is busy..." : "Type a message...")
                    font.pixelSize: 18
                    font.family: "Inter"
                    enabled: ChatViewViewModel.isLoaded && !ChatViewViewModel.isThinking && !ChatViewViewModel.isGenerating
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
                color: (ChatViewViewModel.isThinking || ChatViewViewModel.isGenerating || !ChatViewViewModel.isLoaded) ? "#CCCCCC" : "#007AFF"
                radius: 20
                enabled: ChatViewViewModel.isLoaded && !ChatViewViewModel.isThinking && !ChatViewViewModel.isGenerating
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

    Connections {
        target: ChatViewViewModel
        onRequestScrollToBottom: {
            mainFlickable.positionViewAtEnd()
            mainFlickable.contentY = Math.max(0, mainFlickable.contentHeight - mainFlickable.height)
        }
    }
}
