#pragma once

#include <QObject>

class ContainerBarViewModel : public QObject {
    Q_OBJECT

public:
    explicit ContainerBarViewModel(QObject* parent = nullptr);
    ~ContainerBarViewModel() override = default;

private:
};
