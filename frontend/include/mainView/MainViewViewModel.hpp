#pragma once

#include <QObject>

class MainViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)

public:
    explicit MainViewViewModel(QObject* parent = nullptr);
    ~MainViewViewModel() override = default;

    bool isActive() const;
    Q_INVOKABLE void requestRunningView();

signals:
    void isActiveChanged();

private slots:
    void onStateMachineChanged();

private:
    bool m_isActive = true;
};
