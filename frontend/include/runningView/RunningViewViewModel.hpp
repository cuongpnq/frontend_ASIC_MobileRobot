#pragma once

#include <QObject>

class RunningViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)

public:
    explicit RunningViewViewModel(QObject* parent = nullptr);
    ~RunningViewViewModel() override = default;

    bool isActive() const;
    Q_INVOKABLE void requestControlCenterView();
    Q_INVOKABLE void requestDirectionView();

signals:
    void isActiveChanged();

private slots:
    void onStateMachineChanged();

private:
    bool m_isActive = false;
};
