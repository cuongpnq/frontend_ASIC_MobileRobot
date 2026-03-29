#pragma once

#include <QObject>

class DirectionViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)

public:
    explicit DirectionViewViewModel(QObject* parent = nullptr);
    ~DirectionViewViewModel() override = default;

    bool isActive() const;
    Q_INVOKABLE void requestMainView();

signals:
    void isActiveChanged();

private slots:
    void onStateMachineChanged();

private:
    bool m_isActive = false;
};
