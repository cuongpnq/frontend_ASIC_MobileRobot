#pragma once

#include <QObject>

class ControlCenterViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)

public:
    explicit ControlCenterViewViewModel(QObject* parent = nullptr);
    ~ControlCenterViewViewModel() override = default;

    bool isActive() const;
    Q_INVOKABLE void requestRunningView();
    Q_INVOKABLE void requestMainView();
    Q_INVOKABLE void requestMapPanelView();
    Q_INVOKABLE void requestDiagnosticsView();

signals:
    void isActiveChanged();

private slots:
    void onStateMachineChanged();

private:
    bool m_isActive = false;
};
