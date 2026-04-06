#pragma once

#include <QObject>

class SettingsViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)

public:
    explicit SettingsViewViewModel(QObject* parent = nullptr);
    ~SettingsViewViewModel() override = default;

    bool isActive() const;
    Q_INVOKABLE void requestMainView();
    Q_INVOKABLE void requestWifiSettingsView();

signals:
    void isActiveChanged();

private slots:
    void onStateMachineChanged();

private:
    bool m_isActive = false;
};
