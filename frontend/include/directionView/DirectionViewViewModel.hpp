#pragma once

#include <QObject>

class DirectionViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool autoReturnPending READ autoReturnPending NOTIFY autoReturnPendingChanged)

public:
    explicit DirectionViewViewModel(QObject* parent = nullptr);
    ~DirectionViewViewModel() override = default;

    bool isActive() const;
    bool autoReturnPending() const;
    Q_INVOKABLE void setAutoReturnPending(bool value);
    Q_INVOKABLE void requestMainView();
    Q_INVOKABLE void requestRunningView();

signals:
    void isActiveChanged();
    void autoReturnPendingChanged();

private slots:
    void onStateMachineChanged();

private:
    bool m_isActive = false;
    bool m_autoReturnPending = false;
};
