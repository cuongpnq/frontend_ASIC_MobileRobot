import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root
    anchors.fill: parent

    // ── All available view cards ────────────────────────────────────────────
    // minRole: minimum role required to see this card.
    // To add a new card, just append here — no other QML changes needed.
    readonly property var allCards: [
        { name: "Direction View",  viewType: "direction",     icon: "images/direction.png",      minRole: UserManager.User },
        { name: "Presentation",    viewType: "presentation",  icon: "images/presentation.png",   minRole: UserManager.User },
        { name: "Q&A",             viewType: "chatView",      icon: "images/chatbot.png",        minRole: UserManager.User },
        { name: "Control Center",  viewType: "controlCenter", icon: "images/control_center.png", minRole: UserManager.Administrator },
        { name: "Settings View",   viewType: "settings",      icon: "images/settings.png",       minRole: UserManager.User }
    ]

    function filteredCards() {
        var result = []
        for (var i = 0; i < allCards.length; i++) {
            if (UserManager.currentRole >= allCards[i].minRole)
                result.push(allCards[i])
        }
        return result
    }

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onPowerClicked: powerPopup.open()
    }

    // ── Auto-logout notification toast ─────────────────────────────────────
    Rectangle {
        id: autoLogoutToast
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 40
        width: toastText.implicitWidth + 48
        height: 56
        radius: 28
        color: "#1e293b"
        opacity: 0
        z: 100
        visible: opacity > 0

        Text {
            id: toastText
            anchors.centerIn: parent
            text: "Session expired \u2014 reverted to User mode"
            font.pixelSize: 22
            font.family: "Inter"
            color: "white"
        }

        SequentialAnimation {
            id: toastAnim
            NumberAnimation { target: autoLogoutToast; property: "opacity"; to: 1; duration: 300 }
            PauseAnimation  { duration: 3000 }
            NumberAnimation { target: autoLogoutToast; property: "opacity"; to: 0; duration: 400 }
        }
    }

    Connections {
        target: UserManager
        onAutoLoggedOut: toastAnim.start()
    }

    // --- Power Menu Popup ---
    Popup {
        id: powerPopup
        x: 30
        y: 115
        width: 240
        height: 150
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Connections {
            target: window
            onIsStandbyChanged: { if (window.isStandby) powerPopup.close() }
        }

        background: Rectangle { border.color: "#22000000"; border.width: 4; radius: 24 }

        Column {
            anchors.fill: parent
            spacing: 8

            Rectangle {
                width: parent.width; height: 60; radius: 12
                color: powerOffMouse.pressed ? "#e0e0e0" : "#f0f0f0"
                border.color: "#bbbbbb"; border.width: 1
                Text { anchors.centerIn: parent; text: "Power Off"; font.pixelSize: 22; font.family: "Inter"; color: "#ff0000"; font.bold: true }
                MouseArea { id: powerOffMouse; anchors.fill: parent; hoverEnabled: true; onClicked: Qt.quit() }
            }

            Rectangle {
                width: parent.width; height: 60; radius: 12
                color: sleepMouse.pressed ? "#e0e0e0" : "#f0f0f0"
                border.color: "#bbbbbb"; border.width: 1
                Text { anchors.centerIn: parent; text: "Sleep"; font.pixelSize: 22; font.family: "Inter"; color: "#978800"; font.bold: true }
                MouseArea {
                    id: sleepMouse; anchors.fill: parent; hoverEnabled: true
                    onClicked: { window.isStandby = true; powerPopup.close() }
                }
            }
        }
    }

    Rectangle {
        anchors.fill: parent; color: "#F5F4EF"; z: -1
        Image { anchors.horizontalCenter: parent.horizontalCenter; y: 120; width: 1300; height: 1105; source: "images/UIT_logo.png"; opacity: 0.15 }
    }

    Rectangle {
        id: selectionListViewReg
        width: parent.width
        height: parent.height - containerBar.height
        anchors.top: containerBar.bottom
        color: "transparent"

        Connections { target: UserManager; onRoleChanged: cardRepeater.rebuildModel() }

        ListView {
            id: selectionListView
            anchors.centerIn: parent
            width: parent.width - 160
            height: 600
            spacing: 100
            orientation: ListView.Horizontal
            interactive: true
            clip: true
            model: ListModel { id: cardModel }
            Component.onCompleted: cardRepeater.rebuildModel()

            Item {
                id: cardRepeater
                function rebuildModel() {
                    cardModel.clear()
                    var cards = root.filteredCards()
                    for (var i = 0; i < cards.length; i++) cardModel.append(cards[i])
                }
            }

            delegate: Rectangle {
                width: 500; height: 600
                color: "#F2FEFE"; border.color: "#2C2C2C"; border.width: 1; radius: 35
                Image { source: model.icon; anchors.fill: parent; fillMode: Image.PreserveAspectFit }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (model.viewType === "direction")         MainViewViewModel.requestDirectionView()
                        else if (model.viewType === "presentation") MainViewViewModel.requestPresentationView()
                        else if (model.viewType === "chatView")     MainViewViewModel.requestChatView()
                        else if (model.viewType === "controlCenter")MainViewViewModel.requestControlCenterView()
                        else if (model.viewType === "settings")     MainViewViewModel.requestSettingsView()
                    }
                }
            }
        }
    }
}
