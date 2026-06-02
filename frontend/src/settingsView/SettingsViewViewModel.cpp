#include "settingsView/SettingsViewViewModel.hpp"
#include "application/AppStateMachine.hpp"
#include <QSettings>
#include <QDebug>

static constexpr int MIN_SLEEP_MINUTES = 0;  // 0 = never
static constexpr int MAX_SLEEP_MINUTES = 15;
static const char*   SETTINGS_KEY      = "robot/sleepTimeoutMinutes";

SettingsViewViewModel::SettingsViewViewModel(QObject* parent)
    : QObject(parent), m_isActive(false)
{
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &SettingsViewViewModel::onStateMachineChanged);

    // Load persisted value (falls back to 1 minute if not set)
    QSettings settings;
    m_sleepTimeout = settings.value(SETTINGS_KEY, 1).toInt();
    m_sleepTimeout = qBound(MIN_SLEEP_MINUTES, m_sleepTimeout, MAX_SLEEP_MINUTES);
}

bool SettingsViewViewModel::isActive() const {
    return m_isActive;
}

int SettingsViewViewModel::sleepTimeout() const {
    return m_sleepTimeout;
}

void SettingsViewViewModel::setSleepTimeout(int minutes) {
    // Accept only valid preset values (0 = never, or a positive minute count)
    int clamped = qBound(MIN_SLEEP_MINUTES, minutes, MAX_SLEEP_MINUTES);
    if (m_sleepTimeout == clamped) return;
    m_sleepTimeout = clamped;

    QSettings settings;
    settings.setValue(SETTINGS_KEY, m_sleepTimeout);

    qDebug() << "SettingsViewViewModel: Sleep timeout set to" << m_sleepTimeout << "minutes";
    emit sleepTimeoutChanged();
}

int SettingsViewViewModel::sleepTimeoutMs() const {
    if (m_sleepTimeout == 0) return INT_MAX;  // never triggers
    return m_sleepTimeout * 60 * 1000;
}

void SettingsViewViewModel::requestMainView() {
    AppStateMachine::instance().returnToMain();
}

void SettingsViewViewModel::requestWifiSettingsView() {
    AppStateMachine::instance().goToWiFiSettings();
}

void SettingsViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "SettingsView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}
