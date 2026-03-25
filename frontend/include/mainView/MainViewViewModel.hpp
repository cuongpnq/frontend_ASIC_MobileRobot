#pragma once

#include <QObject>

class MainViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool isTakeControl READ isTakeControl NOTIFY controlModeChanged)

public:
    explicit MainViewViewModel(QObject* parent = nullptr);
    ~MainViewViewModel() override = default;

    bool isActive() const;
    bool isTakeControl() const;
    Q_INVOKABLE void requestRunningView();
    Q_INVOKABLE void setControlMode(bool takeControl);

signals:
    void isActiveChanged();
    void controlModeChanged();

private slots:
    void onStateMachineChanged();

private:
    bool m_isActive = true;
    bool m_isTakeControl = false;
};
