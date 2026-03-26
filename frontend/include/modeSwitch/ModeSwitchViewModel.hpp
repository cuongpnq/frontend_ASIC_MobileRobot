#pragma once

#include <QObject>

class ModeSwitchViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isTakeControl READ isTakeControl WRITE setControlMode NOTIFY controlModeChanged)

public:
    explicit ModeSwitchViewModel(QObject* parent = nullptr);
    ~ModeSwitchViewModel() override = default;

    bool isTakeControl() const;
    Q_INVOKABLE void setControlMode(bool takeControl);

signals:
    void controlModeChanged();

private:
    bool m_isTakeControl = false;
};
