#include "application/UserManager.hpp"
#include <QDebug>

// ── Password table ────────────────────────────────────────────────────────────
// To integrate with a fleet management server in the future:
//   1. Add a RemoteAuthProvider member.
//   2. Replace the m_passwords.value() lookup in attemptLogin() with an async API call.
//   3. All QML bindings remain unchanged.
// ─────────────────────────────────────────────────────────────────────────────
static const QMap<UserManager::Role, QString> DEFAULT_PASSWORDS = {
    { UserManager::Administrator, QStringLiteral("admin")     },
    { UserManager::Developer,     QStringLiteral("developer") },
};

UserManager& UserManager::instance() {
    static UserManager inst;
    return inst;
}

UserManager::UserManager(QObject* parent)
    : QObject(parent)
    , m_role(User)
    , m_targetRole(Administrator)
    , m_loginResult(Idle)
    , m_passwords(DEFAULT_PASSWORDS)
{
    m_inactivityTimer.setInterval(AUTO_LOGOUT_MS);
    m_inactivityTimer.setSingleShot(true);
    connect(&m_inactivityTimer, &QTimer::timeout, this, [this]() {
        if (m_role != User) {
            qDebug() << "[UserManager] Auto-logout after"
                     << AUTO_LOGOUT_MS / 1000 << "s inactivity. Reverting to User.";
            applyRole(User);
            emit autoLoggedOut();
        }
    });
}

// ── Properties ────────────────────────────────────────────────────────────────

QString UserManager::roleName() const {
    switch (m_role) {
        case Administrator: return QStringLiteral("Administrator");
        case Developer:     return QStringLiteral("Developer");
        default:            return QStringLiteral("User");
    }
}

QString UserManager::roleBadgeColor() const {
    switch (m_role) {
        case Administrator: return QStringLiteral("#d97706"); // amber
        case Developer:     return QStringLiteral("#7c3aed"); // violet
        default:            return QStringLiteral("#2563eb"); // blue
    }
}

QString UserManager::targetRoleName() const {
    switch (m_targetRole) {
        case Administrator: return QStringLiteral("Administrator");
        case Developer:     return QStringLiteral("Developer");
        default:            return QStringLiteral("User");
    }
}

// ── Two-step login API ────────────────────────────────────────────────────────

void UserManager::selectTargetRole(int role) {
    auto r = static_cast<Role>(role);
    if (m_targetRole != r) {
        m_targetRole = r;
        emit targetRoleChanged();
    }
    // Reset any previous result when the user picks a new role
    if (m_loginResult != Idle) {
        m_loginResult = Idle;
        emit loginResultChanged();
    }
}

void UserManager::attemptLogin(const QString& password) {
    if (m_passwords.value(m_targetRole) == password) {
        qDebug() << "[UserManager] Login OK → role:" << targetRoleName();
        applyRole(m_targetRole);
        m_loginResult = Success;
    } else {
        qDebug() << "[UserManager] Login FAILED for role:" << targetRoleName();
        m_loginResult = Failed;
    }
    emit loginResultChanged();
}

void UserManager::resetLoginResult() {
    if (m_loginResult != Idle) {
        m_loginResult = Idle;
        emit loginResultChanged();
    }
}

void UserManager::logout() {
    qDebug() << "[UserManager] Manual logout → User.";
    applyRole(User);
}

void UserManager::resetInactivityTimer() {
    if (m_role != User) {
        m_inactivityTimer.start();
    }
}

// ── Internal helpers ──────────────────────────────────────────────────────────

void UserManager::applyRole(Role role) {
    if (m_role == role) return;
    m_role = role;
    (m_role == User) ? m_inactivityTimer.stop() : m_inactivityTimer.start();
    emit roleChanged();
}
