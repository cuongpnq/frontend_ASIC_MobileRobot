#include "presentationView/PresentationViewViewModel.hpp"
#include "application/AppStateMachine.hpp"
#include <QFile>
#include <QTextStream>
#include <QDebug>

PresentationViewViewModel::PresentationViewViewModel(QObject* parent) 
    : QObject(parent), m_isActive(false)
{
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &PresentationViewViewModel::onStateMachineChanged);
}

bool PresentationViewViewModel::isActive() const {
    return m_isActive;
}

QString PresentationViewViewModel::content() const {
    return m_content;
}

void PresentationViewViewModel::requestMainView() {
    AppStateMachine::instance().returnToMain();
}

void PresentationViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "PresentationView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
        if (m_isActive) {
            loadContent();
        }
    }
}

void PresentationViewViewModel::loadContent() {
    QFile file("frontend/presentation.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open presentation.txt";
        m_content = "Failed to load presentation content.";
    } else {
        QTextStream in(&file);
        m_content = in.readAll();
        file.close();
    }
    emit contentChanged();
}
