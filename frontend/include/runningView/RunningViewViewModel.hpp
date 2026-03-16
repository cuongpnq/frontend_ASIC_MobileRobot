#pragma once

#include <QObject>

class RunningViewViewModel : public QObject {
    Q_OBJECT

public:
    explicit RunningViewViewModel(QObject* parent = nullptr);
    ~RunningViewViewModel() override = default;

    Q_INVOKABLE void requestMainView();

private:
};
