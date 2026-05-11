#pragma once

#include <QObject>
#include <QString>

// Include the Yakindu generated C header
extern "C" {
#include "Statechart.h"
}

class AppStateMachine : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentState READ currentState NOTIFY currentStateChanged)

public:
    static AppStateMachine& instance() {
        static AppStateMachine instance;
        return instance;
    }

    QString currentState() const;

    // Direct C++ methods to trigger state machine events
    Q_INVOKABLE void goToRunning();
    Q_INVOKABLE void returnToMain();
    Q_INVOKABLE void goToControlCenter();
    Q_INVOKABLE void goToDirection();
    Q_INVOKABLE void goToPresentation();
    Q_INVOKABLE void returnToControlCenter();
    Q_INVOKABLE void goToSettings();
    Q_INVOKABLE void goToDiagnostics();
    Q_INVOKABLE void goToMapPanel();
    Q_INVOKABLE void goToChatView();
    Q_INVOKABLE void goToWiFiSettings();
    Q_INVOKABLE void returnToSettings();

signals:
    void currentStateChanged();

private:
    explicit AppStateMachine(QObject* parent = nullptr);
    ~AppStateMachine() override;
    
    void updateState(); // Called internally to sync state to QML property

    Statechart m_statechart;
    QString m_currentState;
};
