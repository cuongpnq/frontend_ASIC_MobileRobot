#pragma once

#include <QObject>

class PresentationViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(QString content READ content NOTIFY contentChanged)

public:
    explicit PresentationViewViewModel(QObject* parent = nullptr);
    ~PresentationViewViewModel() override = default;

    bool isActive() const;
    QString content() const;
    Q_INVOKABLE void requestMainView();

signals:
    void isActiveChanged();
    void contentChanged();

private slots:
    void onStateMachineChanged();
    void loadContent();

private:
    bool m_isActive = false;
    QString m_content;
};
