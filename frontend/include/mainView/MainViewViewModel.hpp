#pragma once

#include <QObject>

class MainViewViewModel : public QObject {
    Q_OBJECT

public:
    explicit MainViewViewModel(QObject* parent = nullptr);
    ~MainViewViewModel() override = default;

    Q_INVOKABLE void requestRunningView();

private:
};
