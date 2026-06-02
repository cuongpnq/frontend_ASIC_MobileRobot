#pragma once

#include <QObject>
#include <QSettings>

class SettingsViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(int sleepTimeout READ sleepTimeout WRITE setSleepTimeout NOTIFY sleepTimeoutChanged)
    Q_PROPERTY(int sleepTimeoutMs READ sleepTimeoutMs NOTIFY sleepTimeoutChanged)

public:
    explicit SettingsViewViewModel(QObject* parent = nullptr);
    ~SettingsViewViewModel() override = default;

    bool isActive() const;

    // Sleep timeout in minutes. 0 = never sleep. Valid presets: 0,1,2,3,5,10,15
    int sleepTimeout() const;
    void setSleepTimeout(int minutes);

    // Convenience: timeout in milliseconds for the QML Timer
    int sleepTimeoutMs() const;

    Q_INVOKABLE void requestMainView();
    Q_INVOKABLE void requestWifiSettingsView();

signals:
    void isActiveChanged();
    void sleepTimeoutChanged();

private slots:
    void onStateMachineChanged();

private:
    bool m_isActive = false;
    int  m_sleepTimeout = 1; // minutes, default 1 min
};
