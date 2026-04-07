import QtQuick 2.12
import QtQuick.VirtualKeyboard 2.1
import QtQuick.VirtualKeyboard.Settings 2.1

InputPanel {
    id: root
    
    property bool keyboardVisible: false
    property bool hasVirtualKeyboard: true

    width: parent.width
    z: 2147483647 
    y: keyboardVisible ? parent.height - height : parent.height
    visible: hasVirtualKeyboard
    
    Component.onCompleted: {
        VirtualKeyboardSettings.styleName = "default"
    }
}
