import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root
    
    // Add a signal we can emit
    signal changeViewRequested(string viewName)
    
    Rectangle {
        anchors.fill: parent
        color: "#d0ffd0"
        
        Text {
            anchors.centerIn: parent
            text: "RUNNING VIEW DASHBOARD"
            font.pixelSize: 24
            font.bold: true
        }

        Button {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: 40
            text: "Return to Main View"
            onClicked: {
                // Request state machine event from the correct view model
                RunningViewViewModel.requestMainView()
            }
        }
    }
}
