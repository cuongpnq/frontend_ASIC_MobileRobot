#pragma once

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QString>

/**
 * UserManager — Singleton managing authentication and role-based access control.
 *
 * Roles:
 *   User          — Default; limited views (Direction, Q&A, Settings, Presentation)
 *   Administrator — Full access to all views
 *   Developer     — Full access + future developer-only features (reserved)
 *
 * Login flow (two-step, driven entirely from C++):
 *   1. QML calls selectTargetRole(role) — sets which role the user wants to switch to
 *   2. QML calls attemptLogin(password)  — validates and applies the role, emits loginResultChanged
 *
 * Auto-logout: reverts to User role after AUTO_LOGOUT_MS ms of inactivity.
 *
 * Future fleet server:
 *   Replace the local m_passwords map with a RemoteAuthProvider.
 *   No QML changes needed.
 */
class UserManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(Role   currentRole     READ currentRole     NOTIFY roleChanged)
    Q_PROPERTY(QString roleName       READ roleName        NOTIFY roleChanged)
    Q_PROPERTY(QString roleBadgeColor READ roleBadgeColor  NOTIFY roleChanged)

    Q_PROPERTY(Role   targetRole      READ targetRole      NOTIFY targetRoleChanged)
    Q_PROPERTY(QString targetRoleName READ targetRoleName  NOTIFY targetRoleChanged)

    Q_PROPERTY(LoginResult loginResult READ loginResult    NOTIFY loginResultChanged)

public:
    static constexpr int AUTO_LOGOUT_MS = 3 * 60 * 1000; // 3 minutes

    enum Role {
        User          = 0,
        Administrator = 1,
        Developer     = 2
    };
    Q_ENUM(Role)

    enum LoginResult {
        Idle    = 0,
        Success = 1,
        Failed  = 2
    };
    Q_ENUM(LoginResult)

    static UserManager& instance();

    Role        currentRole()    const { return m_role; }
    QString     roleName()       const;
    QString     roleBadgeColor() const;

    Role        targetRole()     const { return m_targetRole; }
    QString     targetRoleName() const;

    LoginResult loginResult()    const { return m_loginResult; }

    /** Step 1: QML selects which role the user wants to switch to. */
    Q_INVOKABLE void selectTargetRole(int role);

    /** Step 2: QML submits password. Emits loginResultChanged with Success or Failed. */
    Q_INVOKABLE void attemptLogin(const QString& password);

    /** Reset loginResult back to Idle (call after dialog closes). */
    Q_INVOKABLE void resetLoginResult();

    /** Revert to User role immediately. */
    Q_INVOKABLE void logout();

    /** Called by UserInteractionMonitor to reset the inactivity timer. */
    Q_INVOKABLE void resetInactivityTimer();

signals:
    void roleChanged();
    void targetRoleChanged();
    void loginResultChanged();
    void autoLoggedOut();

private:
    explicit UserManager(QObject* parent = nullptr);
    ~UserManager() override = default;

    void applyRole(Role role);

    Role                m_role        = User;
    Role                m_targetRole  = Administrator;
    LoginResult         m_loginResult = Idle;
    QTimer              m_inactivityTimer;
    QMap<Role, QString> m_passwords;
};
