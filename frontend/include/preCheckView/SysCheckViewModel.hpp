#pragma once

#include <QObject>
#include <QVariantList>
#include <QProcess>
#include <QString>

/**
 * SysCheckViewModel — runs sys_check.sh and streams results to QML.
 *
 * Two modes of operation:
 *
 *   1. BOOT MODE  (startup overlay, main.qml)
 *      Triggered automatically at app start.  The overlay stays visible
 *      until the user taps "Continue" → dismissStartupCheck().
 *      Pass --no-syscheck on the command line to bypass the boot overlay
 *      entirely (useful for native / dev builds without a ROS environment).
 *
 *   2. MANUAL MODE  (ControlCenter panel → open() → SysCheckView.qml overlay)
 *      Operator can re-run the check at any time.  Has a Back button.
 */
class SysCheckViewModel : public QObject {
    Q_OBJECT

    // ── Boot overlay control ──────────────────────────────────────
    // false  → boot overlay is shown (blocks app until dismissed)
    // true   → boot overlay unloaded; normal app flow continues
    Q_PROPERTY(bool isStartupCheckDone READ isStartupCheckDone NOTIFY isStartupCheckDoneChanged)

    // ── Manual overlay control (ControlCenter) ────────────────────
    Q_PROPERTY(bool isVisible READ isVisible NOTIFY isVisibleChanged)

    // ── Which mode we are currently in ───────────────────────────
    // true  → boot mode (no Back button, has Continue button)
    // false → manual mode (has Back button, no Continue button)
    Q_PROPERTY(bool isBootMode READ isBootMode NOTIFY isModeChanged)

    // ── Run state ─────────────────────────────────────────────────
    // "idle" | "running" | "ready" | "warnings" | "failed"
    Q_PROPERTY(QString  status   READ status   NOTIFY statusChanged)
    Q_PROPERTY(bool     isRunning READ isRunning NOTIFY statusChanged)

    // ── Structured results ────────────────────────────────────────
    Q_PROPERTY(QVariantList results READ results NOTIFY resultsChanged)

    // ── Raw terminal log ──────────────────────────────────────────
    Q_PROPERTY(QString rawLog READ rawLog NOTIFY rawLogChanged)

    // ── Counters ─────────────────────────────────────────────────
    Q_PROPERTY(int passCount READ passCount NOTIFY countersChanged)
    Q_PROPERTY(int warnCount READ warnCount NOTIFY countersChanged)
    Q_PROPERTY(int failCount READ failCount NOTIFY countersChanged)

    // ── Navigation process (run_nav.sh) ──────────────────────────────
    Q_PROPERTY(bool    isNavRunning READ isNavRunning NOTIFY isNavRunningChanged)
    Q_PROPERTY(QString navStatus    READ navStatus    NOTIFY navStatusChanged)
    Q_PROPERTY(QString navFloor     READ navFloor     NOTIFY navStatusChanged)

public:
    explicit SysCheckViewModel(QObject *parent = nullptr);
    ~SysCheckViewModel() override;

    // Getters
    bool         isStartupCheckDone() const;
    bool         isVisible()          const;
    bool         isBootMode()         const;
    QString      status()             const;
    bool         isRunning()          const;
    QVariantList results()            const;
    QString      rawLog()             const;
    int          passCount()          const;
    int          warnCount()          const;
    int          failCount()          const;
    bool         isNavRunning()       const;
    QString      navStatus()          const;
    QString      navFloor()           const;

    // ── QML-callable ─────────────────────────────────────────────

    /** Boot overlay: user taps "Continue" after seeing the result. */
    Q_INVOKABLE void dismissStartupCheck();

    /** ControlCenter: open manual overlay. */
    Q_INVOKABLE void open();
    /** ControlCenter: close manual overlay (also cancels any running check). */
    Q_INVOKABLE void close();

    /**
     * Run the system check script.
     * @param floor       Floor identifier passed to sys_check.sh (default "e6")
     * @param skipBuild   Append --skip-build (default true)
     * @param checkTopics Append --check-topics (default false)
     */
    Q_INVOKABLE void runSysCheck(const QString &floor = QStringLiteral("e6"),
                                 bool skipBuild   = true,
                                 bool checkTopics = false);
    Q_INVOKABLE void cancelSysCheck();
    Q_INVOKABLE void launchNavigation(const QString& floor = QStringLiteral("e6"));
    Q_INVOKABLE void stopNavigation();

signals:
    void isStartupCheckDoneChanged();
    void isVisibleChanged();
    void isModeChanged();
    void statusChanged();
    void resultsChanged();
    void rawLogChanged();
    void countersChanged();
    void isNavRunningChanged();
    void navStatusChanged();

private slots:
    void onReadyRead();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onNavFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void autoStartBootCheck();
    void parseLine(const QString &line);
    void setStatus(const QString &s);
    void resetState();
    static QString scriptPath();
    static QString navScriptPath();
    void startNavProcess();
    void setNavStatus(const QString& s);

    bool         m_isStartupCheckDone = false;
    bool         m_isVisible          = false;
    bool         m_isBootMode         = false;

    QString      m_status    = QStringLiteral("idle");
    QVariantList m_results;
    QString      m_rawLog;
    int          m_passCount = 0;
    int          m_warnCount = 0;
    int          m_failCount = 0;

    QProcess    *m_process      = nullptr;

    // Navigation process state
    QProcess    *m_navProcess   = nullptr;
    bool         m_isNavRunning = false;
    QString      m_navStatus    = QStringLiteral("idle");
    QString      m_currentFloor = QStringLiteral("e6");
};

extern SysCheckViewModel* g_sysCheckViewModel;

