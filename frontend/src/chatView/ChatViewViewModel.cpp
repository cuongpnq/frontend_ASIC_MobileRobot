#include "chatView/ChatViewViewModel.hpp"
#include "application/AppStateMachine.hpp"
#include <QGuiApplication>
#include <QInputMethod>

ChatViewViewModel::ChatViewViewModel(QObject* parent) : QObject(parent) {
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged, this, &ChatViewViewModel::onStateMachineChanged);
    
    auto inputMethod = QGuiApplication::inputMethod();
    if (inputMethod) {
        connect(inputMethod, &QInputMethod::visibleChanged, this, &ChatViewViewModel::onInputMethodVisibleChanged);
    }
}

bool ChatViewViewModel::isActive() const {
    return m_isActive;
}

bool ChatViewViewModel::keyboardVisible() const {
    return QGuiApplication::inputMethod()->isVisible();
}

bool ChatViewViewModel::hasVirtualKeyboard() const {
#ifdef HAS_VIRTUAL_KEYBOARD
    return true;
#else
    return false;
#endif
}

void ChatViewViewModel::requestMainView() {
    dismissKeyboard();
    AppStateMachine::instance().returnToMain();
}

void ChatViewViewModel::dismissKeyboard() {
    auto inputMethod = QGuiApplication::inputMethod();
    if (inputMethod) {
        inputMethod->hide();
    }
}

void ChatViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "ChatView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}

void ChatViewViewModel::onInputMethodVisibleChanged() {
    bool visible = QGuiApplication::inputMethod()->isVisible();
    if (m_keyboardVisible != visible) {
        m_keyboardVisible = visible;
        emit keyboardVisibleChanged();
        if (visible) {
            emit requestScrollToBottom();
        }
    }
}
