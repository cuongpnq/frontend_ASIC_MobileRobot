#pragma once

#include <QObject>

class WifiSettingViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool keyboardVisible READ keyboardVisible NOTIFY keyboardVisibleChanged)
    Q_PROPERTY(bool hasVirtualKeyboard READ hasVirtualKeyboard CONSTANT)

public:
    explicit WifiSettingViewViewModel(QObject* parent = nullptr);
    ~WifiSettingViewViewModel() override = default;

    bool isActive() const;
    bool keyboardVisible() const;
    bool hasVirtualKeyboard() const;
    
    Q_INVOKABLE void requestSettingsView();
    Q_INVOKABLE void dismissKeyboard();

signals:
    void isActiveChanged();
    void keyboardVisibleChanged();

private slots:
    void onStateMachineChanged();
    void onInputMethodVisibleChanged();

private:
    bool m_isActive = false;
    bool m_keyboardVisible = false;
};
