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
    void goToRunning();
    void returnToMain();
    void goToControlCenter();
    void goToDirection();
    void goToSettings();
    void returnToControlCenter();

signals:
    void currentStateChanged();

private:
    explicit AppStateMachine(QObject* parent = nullptr);
    ~AppStateMachine() override;
    
    void updateState(); // Called internally to sync state to QML property

    Statechart m_statechart;
    QString m_currentState;
};
