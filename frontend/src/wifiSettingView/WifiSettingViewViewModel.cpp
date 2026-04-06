#include "wifiSettingView/WifiSettingViewViewModel.hpp"
#include "application/AppStateMachine.hpp"
#include <QGuiApplication>
#include <QInputMethod>

WifiSettingViewViewModel::WifiSettingViewViewModel(QObject* parent) 
    : QObject(parent), m_isActive(false), m_keyboardVisible(false)
{
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &WifiSettingViewViewModel::onStateMachineChanged);
            
    auto inputMethod = QGuiApplication::inputMethod();
    if (inputMethod) {
        connect(inputMethod, &QInputMethod::visibleChanged, this, &WifiSettingViewViewModel::onInputMethodVisibleChanged);
    }
}

bool WifiSettingViewViewModel::isActive() const {
    return m_isActive;
}

void WifiSettingViewViewModel::requestSettingsView() {
    dismissKeyboard();
    AppStateMachine::instance().returnToSettings();
}

void WifiSettingViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "WiFiSettingView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}

bool WifiSettingViewViewModel::keyboardVisible() const {
    return QGuiApplication::inputMethod()->isVisible();
}

bool WifiSettingViewViewModel::hasVirtualKeyboard() const {
#ifdef HAS_VIRTUAL_KEYBOARD
    return true;
#else
    return false;
#endif
}

void WifiSettingViewViewModel::dismissKeyboard() {
    auto inputMethod = QGuiApplication::inputMethod();
    if (inputMethod) {
        inputMethod->hide();
    }
}

void WifiSettingViewViewModel::onInputMethodVisibleChanged() {
    bool visible = QGuiApplication::inputMethod()->isVisible();
    if (m_keyboardVisible != visible) {
        m_keyboardVisible = visible;
        emit keyboardVisibleChanged();
    }
}
