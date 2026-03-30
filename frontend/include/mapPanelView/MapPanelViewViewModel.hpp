#pragma once

#include <QObject>

class MapPanelViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)

public:
    explicit MapPanelViewViewModel(QObject* parent = nullptr);
    ~MapPanelViewViewModel() override = default;

    bool isActive() const;
    Q_INVOKABLE void requestControlCenterView();

signals:
    void isActiveChanged();

private slots:
    void onStateMachineChanged();

private:
    bool m_isActive = false;
};
