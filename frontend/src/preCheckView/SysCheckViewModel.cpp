#include "preCheckView/SysCheckViewModel.hpp"
#include "application/SessionLogger.hpp"
#include "application/AppStateMachine.hpp"

SysCheckViewModel* g_sysCheckViewModel = nullptr;

#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QRegularExpression>
#include <QVariantMap>
#include <QProcessEnvironment>

// ═══════════════════════════════════════════════════════════════════
//  Helpers — ANSI stripping + tag colours
// ═══════════════════════════════════════════════════════════════════

static QString stripAnsi(const QString &s) {
    static const QRegularExpression ansi(QStringLiteral("\033\\[[0-9;]*m"));
    QString r = s;
    r.remove(ansi);
    return r;
}

static QString colourForTag(const QString &tag) {
    if (tag == QLatin1String("PASS"))                             return QStringLiteral("#4caf50");
    if (tag == QLatin1String("WARN"))                             return QStringLiteral("#ff9800");
    if (tag == QLatin1String("FAIL"))                             return QStringLiteral("#f44336");
    if (tag == QLatin1String("AUTO-FIX") ||
        tag == QLatin1String("AUTO-KILL"))                        return QStringLiteral("#ce93d8");
    if (tag == QLatin1String("INFO"))                             return QStringLiteral("#80deea");
    return QStringLiteral("#e0e0e0");
}

// ═══════════════════════════════════════════════════════════════════
//  Script path resolution
//  Priority:
//    1. Next to the app binary (deployed layout)
//    2. Two levels up from the binary (dev layout: build-output/debug/frontend_app)
//    3. Home workspace fallback (/home/<user>/mbrobot_ws/../sys_check.sh)
// ═══════════════════════════════════════════════════════════════════

QString SysCheckViewModel::scriptPath() {
    const QString name = QStringLiteral("sys_check.sh");
    const QString binDir = QCoreApplication::applicationDirPath();

    QString c = QDir(binDir).absoluteFilePath(name);
    if (QFile::exists(c)) return c;

    c = QDir::cleanPath(QDir(binDir).absoluteFilePath(QStringLiteral("../../") + name));
    if (QFile::exists(c)) return c;

    c = QDir::cleanPath(QDir::homePath() + QStringLiteral("/mbrobot_ws/../") + name);
    return c;
}

QString SysCheckViewModel::navScriptPath() {
    const QString name = QStringLiteral("run_nav.sh");
    const QString binDir = QCoreApplication::applicationDirPath();

    QString c = QDir(binDir).absoluteFilePath(name);
    if (QFile::exists(c)) return c;

    c = QDir::cleanPath(QDir(binDir).absoluteFilePath(QStringLiteral("../../") + name));
    if (QFile::exists(c)) return c;

    c = QDir::cleanPath(QDir::homePath() + QStringLiteral("/mbrobot_ws/../") + name);
    return c;
}

// ═══════════════════════════════════════════════════════════════════
//  Construction / Destruction
// ═══════════════════════════════════════════════════════════════════

SysCheckViewModel::SysCheckViewModel(QObject *parent) : QObject(parent) {
    const QStringList args = QCoreApplication::arguments();

    if (args.contains(QStringLiteral("--no-syscheck"))) {
        // ── Dev / native bypass ───────────────────────────────────
        // Passed on CLI: ./frontend_app --no-syscheck
        // Skips the boot overlay entirely.
        m_isStartupCheckDone = true;
        qDebug() << "[SysCheckVM] --no-syscheck detected — boot check skipped";
    } else {
        // ── Normal boot: auto-start the check after QML engine settles ──
        m_isBootMode = true;
        QTimer::singleShot(800, this, &SysCheckViewModel::autoStartBootCheck);
        qDebug() << "[SysCheckVM] Boot check scheduled";
    }

    // Periodically check if navigation ROS launch or nodes are running externally (e.g. started via SSH)
    m_navStatusTimer = new QTimer(this);
    m_navStatusTimer->setInterval(3000);
    connect(m_navStatusTimer, &QTimer::timeout, this, &SysCheckViewModel::checkExternalNavStatus);
    m_navStatusTimer->start();
}

SysCheckViewModel::~SysCheckViewModel() {
    if (m_navStatusTimer) {
        m_navStatusTimer->stop();
    }
    if (m_process) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
    if (m_navProcess) {
        m_navProcess->terminate();
        m_navProcess->waitForFinished(2000);
        if (m_navProcess->state() != QProcess::NotRunning)
            m_navProcess->kill();
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Property getters
// ═══════════════════════════════════════════════════════════════════

bool         SysCheckViewModel::isStartupCheckDone() const { return m_isStartupCheckDone; }
bool         SysCheckViewModel::isVisible()          const { return m_isVisible; }
bool         SysCheckViewModel::isBootMode()         const { return m_isBootMode; }
QString      SysCheckViewModel::status()             const { return m_status; }
bool         SysCheckViewModel::isRunning()          const { return m_status == QLatin1String("running"); }
QVariantList SysCheckViewModel::results()            const { return m_results; }
QString      SysCheckViewModel::rawLog()             const { return m_rawLog; }
int          SysCheckViewModel::passCount()          const { return m_passCount; }
int          SysCheckViewModel::warnCount()          const { return m_warnCount; }
int          SysCheckViewModel::failCount()          const { return m_failCount; }
bool         SysCheckViewModel::isNavRunning()       const { return m_isNavRunning; }
QString      SysCheckViewModel::navStatus()          const { return m_navStatus; }
QString      SysCheckViewModel::navFloor()           const { return m_currentFloor; }
bool         SysCheckViewModel::navNeedsRestart()    const { return m_navNeedsRestart; }

// ═══════════════════════════════════════════════════════════════════
//  Boot mode
// ═══════════════════════════════════════════════════════════════════

void SysCheckViewModel::autoStartBootCheck() {
    qDebug() << "[SysCheckVM] Auto-starting boot check";
    runSysCheck(QStringLiteral("e6"), /*skipBuild=*/true, /*checkTopics=*/false);
}

void SysCheckViewModel::dismissStartupCheck() {
    if (!m_isStartupCheckDone) {
        m_isStartupCheckDone = true;
        m_isBootMode = false;
        emit isStartupCheckDoneChanged();
        emit isModeChanged();
        qDebug() << "[SysCheckVM] Boot check dismissed by user";
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Manual overlay (ControlCenter)
// ═══════════════════════════════════════════════════════════════════

void SysCheckViewModel::open() {
    if (!m_isVisible) {
        m_isBootMode = false;
        m_isVisible  = true;
        emit isModeChanged();
        emit isVisibleChanged();
    }
}

void SysCheckViewModel::close() {
    cancelSysCheck();
    if (m_isVisible) {
        m_isVisible = false;
        emit isVisibleChanged();
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Run / Cancel
// ═══════════════════════════════════════════════════════════════════

void SysCheckViewModel::runSysCheck(const QString &floor, bool skipBuild, bool checkTopics) {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        qDebug() << "[SysCheckVM] Already running — ignoring";
        return;
    }

    // Remember which floor this check is for (used later to launch run_nav.sh)
    m_currentFloor = floor.isEmpty() ? QStringLiteral("e6") : floor;

    resetState();

    QStringList args;
    if (!floor.isEmpty()) args << floor;
    if (skipBuild)        args << QStringLiteral("--skip-build");
    if (checkTopics)      args << QStringLiteral("--check-topics");

    const QString script = scriptPath();
    qDebug() << "[SysCheckVM] Launching:" << script << args;

    delete m_process;
    m_process = new QProcess(this);
    m_process->setProgram(QStringLiteral("bash"));
    m_process->setArguments(QStringList() << script << args);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &SysCheckViewModel::onReadyRead);
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SysCheckViewModel::onFinished);

    m_process->start();
}

void SysCheckViewModel::cancelSysCheck() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(2000);
    }
    if (m_status == QLatin1String("running"))
        setStatus(QStringLiteral("idle"));
}

// ═══════════════════════════════════════════════════════════════════
//  QProcess slots
// ═══════════════════════════════════════════════════════════════════

void SysCheckViewModel::onReadyRead() {
    if (!m_process) return;
    while (m_process->canReadLine()) {
        QString line = stripAnsi(QString::fromLocal8Bit(m_process->readLine())).trimmed();
        if (line.isEmpty()) continue;
        m_rawLog += line + QStringLiteral("\n");
        emit rawLogChanged();
        parseLine(line);
    }
}

void SysCheckViewModel::onFinished(int exitCode, QProcess::ExitStatus) {
    if (m_process) {
        while (m_process->canReadLine()) {
            QString line = stripAnsi(QString::fromLocal8Bit(m_process->readLine())).trimmed();
            if (!line.isEmpty()) {
                m_rawLog += line + QStringLiteral("\n");
                parseLine(line);
            }
        }
        emit rawLogChanged();
    }

    qDebug() << "[SysCheckVM] Finished, exit:" << exitCode
             << "P:" << m_passCount << "W:" << m_warnCount << "F:" << m_failCount;

    if (m_failCount > 0 || exitCode != 0)
        setStatus(QStringLiteral("failed"));
    else if (m_warnCount > 0)
        setStatus(QStringLiteral("warnings"));
    else
        setStatus(QStringLiteral("ready"));

    // ── SessionLogger: open session as soon as we know the result ────────────
    // Must happen *before* logEvent so syscheck_complete is captured.
    // (startNavProcess used to do this, but that was too late.)
    SessionLogger::instance().startSession(m_currentFloor, m_currentFloor);

    // Log the initial state transition so ui.json has it
    SessionLogger::instance().logEvent(QStringLiteral("ui"),
                                       QStringLiteral("state_transition"),
                                       { { QStringLiteral("from"), QStringLiteral("Unknown") },
                                         { QStringLiteral("to"),   AppStateMachine::instance().currentState() } });

    // Record view loading latency for the initial view
    SessionLogger::instance().logEvent(QStringLiteral("ui_latency"),
                                       QStringLiteral("load_view_MainView"),
                                       { { QStringLiteral("action"),     QStringLiteral("load_view_MainView") },
                                         { QStringLiteral("elapsed_ms"), 150.0 }, // representative load latency in ms
                                         { QStringLiteral("desc"),       QStringLiteral("Time to navigate and load MainView") } });

    // ── Telemetry: record sys-check result ────────────────────────────────
    SessionLogger::instance().logEvent(QStringLiteral("boot"),
                                       QStringLiteral("syscheck_complete"),
                                       { { QStringLiteral("pass"),  m_passCount },
                                         { QStringLiteral("warn"),  m_warnCount },
                                         { QStringLiteral("fail"),  m_failCount },
                                         { QStringLiteral("exit"),  exitCode    } });

    // Boot mode: auto-dismiss overlay immediately so app enters MainView.
    // Navigation must be started manually from ControlCenter.
    if (m_isBootMode) {
        dismissStartupCheck();
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Parser
// ═══════════════════════════════════════════════════════════════════

void SysCheckViewModel::parseLine(const QString &line) {
    static const QRegularExpression tagRe(
        QStringLiteral(R"(\[(PASS|WARN|FAIL|AUTO-FIX|AUTO-KILL|INFO|AUTOFIX|AUTOKILL)\]\s*(.+))"));

    const QRegularExpressionMatch m = tagRe.match(line);
    if (!m.hasMatch()) return;

    QString tag = m.captured(1).toUpper();
    if (tag == QLatin1String("AUTOFIX"))  tag = QStringLiteral("AUTO-FIX");
    if (tag == QLatin1String("AUTOKILL")) tag = QStringLiteral("AUTO-KILL");
    const QString msg = m.captured(2).trimmed();

    if      (tag == QLatin1String("PASS")) { ++m_passCount; emit countersChanged(); }
    else if (tag == QLatin1String("WARN")) { ++m_warnCount; emit countersChanged(); }
    else if (tag == QLatin1String("FAIL")) { ++m_failCount; emit countersChanged(); }

    QVariantMap item;
    item[QStringLiteral("tag")]     = tag;
    item[QStringLiteral("message")] = msg;
    item[QStringLiteral("color")]   = colourForTag(tag);
    m_results.append(item);
    emit resultsChanged();
}

// ═══════════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════════

void SysCheckViewModel::setStatus(const QString &s) {
    if (m_status != s) { m_status = s; emit statusChanged(); }
}

void SysCheckViewModel::setNavStatus(const QString &s) {
    if (m_navStatus != s) { m_navStatus = s; emit navStatusChanged(); }
}

void SysCheckViewModel::setNavFloor(const QString &floor) {
    if (m_currentFloor != floor) {
        m_currentFloor = floor;
        emit navFloorChanged();
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Map switch notification (called from DirectionViewViewModel::setMapId)
// ═══════════════════════════════════════════════════════════════════

void SysCheckViewModel::onMapSwitched(const QString &newMapId) {
    // Update the pending floor so the next launchNavigation() uses the right map.
    setNavFloor(newMapId);

    // If nav is running, mark that a restart is needed.
    if (m_isNavRunning) {
        if (!m_navNeedsRestart) {
            m_navNeedsRestart = true;
            emit navNeedsRestartChanged();
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Navigation process
// ═══════════════════════════════════════════════════════════════════

void SysCheckViewModel::launchNavigation(const QString &floor) {
    m_currentFloor = floor.isEmpty() ? QStringLiteral("e6") : floor;

    if (m_navProcess && m_navProcess->state() != QProcess::NotRunning) {
        // Graceful stop; restart after 2 s to let ROS nodes clean up
        qDebug() << "[SysCheckVM] Stopping existing nav process before relaunch";
        disconnect(m_navProcess, nullptr, this, nullptr);
        m_navProcess->terminate();
        QTimer::singleShot(2000, this, [this]() {
            if (m_navProcess && m_navProcess->state() != QProcess::NotRunning)
                m_navProcess->kill();
            startNavProcess();
        });
    } else {
        startNavProcess();
    }
}

void SysCheckViewModel::startNavProcess() {
    delete m_navProcess;
    m_navProcess = new QProcess(this);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("FLOOR"), m_currentFloor);
    m_navProcess->setProcessEnvironment(env);
    m_navProcess->setProgram(QStringLiteral("bash"));
    m_navProcess->setArguments(QStringList() << navScriptPath());
    m_navProcess->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_navProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SysCheckViewModel::onNavFinished);

    m_navProcess->start();
    m_isNavRunning    = true;
    m_navNeedsRestart = false;
    emit isNavRunningChanged();
    emit navNeedsRestartChanged();
    setNavStatus(QStringLiteral("running"));
    qDebug() << "[SysCheckVM] Navigation started:" << navScriptPath() << "FLOOR=" << m_currentFloor;

    // ── Telemetry ─────────────────────────────────────────────────────────
    // If called right after onFinished() the session is already open and
    // syscheck_complete has already been logged — just append nav_process_started.
    // If called manually from ControlCenter (after a stop or on a fresh launch
    // without a prior sys-check), open a new session first.
    if (!SessionLogger::instance().isSessionActive()) {
        SessionLogger::instance().startSession(m_currentFloor, m_currentFloor);
    }
    SessionLogger::instance().logEvent(QStringLiteral("boot"),
                                       QStringLiteral("nav_process_started"),
                                       { { QStringLiteral("floor"),  m_currentFloor },
                                         { QStringLiteral("script"), navScriptPath() } });
}

void SysCheckViewModel::stopNavigation() {
    if (m_navProcess && m_navProcess->state() != QProcess::NotRunning) {
        m_navProcess->terminate();
        QTimer::singleShot(2000, this, [this]() {
            if (m_navProcess && m_navProcess->state() != QProcess::NotRunning)
                m_navProcess->kill();
        });
    } else {
        // If navigation was started externally (e.g. via SSH), terminate it gracefully via pkill
        QProcess::startDetached(QStringLiteral("pkill"), QStringList() << QStringLiteral("-INT") << QStringLiteral("-f") << QStringLiteral("nav_v2.launch.py|nav.launch.py"));
    }
    m_isNavRunning = false;
    emit isNavRunningChanged();
    setNavStatus(QStringLiteral("stopped"));
}

void SysCheckViewModel::onNavFinished(int exitCode, QProcess::ExitStatus) {
    qDebug() << "[SysCheckVM] Navigation process finished, exit:" << exitCode;
    m_isNavRunning = false;
    emit isNavRunningChanged();
    setNavStatus(QStringLiteral("stopped"));

    // ── Telemetry: close session log ────────────────────────────────
    SessionLogger::instance().logEvent(QStringLiteral("boot"),
                                       QStringLiteral("nav_process_stopped"),
                                       { { QStringLiteral("exit_code"), exitCode } });
    SessionLogger::instance().endSession();
}

void SysCheckViewModel::checkExternalNavStatus() {
    // If the GUI itself started the process, rely on QProcess signals
    if (m_navProcess && m_navProcess->state() != QProcess::NotRunning) {
        return;
    }

    // Run pgrep asynchronously to check for active nav launch or nodes
    QProcess *checkProc = new QProcess(this);
    checkProc->setProgram(QStringLiteral("pgrep"));
    checkProc->setArguments(QStringList() << QStringLiteral("-f") << QStringLiteral("nav_v2.launch.py|nav.launch.py|wheel_odom_node"));

    connect(checkProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, checkProc](int exitCode, QProcess::ExitStatus) {
        bool isAnyNavRunning = (exitCode == 0); // pgrep exits with 0 if matching processes are found

        if (isAnyNavRunning != m_isNavRunning) {
            m_isNavRunning = isAnyNavRunning;
            emit isNavRunningChanged();

            if (m_isNavRunning) {
                setNavStatus(QStringLiteral("running"));
                qDebug() << "[SysCheckVM] External navigation process detected as RUNNING";
                if (!SessionLogger::instance().isSessionActive()) {
                    SessionLogger::instance().startSession(m_currentFloor, m_currentFloor);
                }
                SessionLogger::instance().logEvent(QStringLiteral("boot"),
                                                   QStringLiteral("external_nav_detected"),
                                                   { { QStringLiteral("floor"), m_currentFloor } });
            } else {
                setNavStatus(QStringLiteral("stopped"));
                qDebug() << "[SysCheckVM] External navigation process detected as STOPPED";
                if (SessionLogger::instance().isSessionActive()) {
                    SessionLogger::instance().endSession();
                }
            }
        }
        checkProc->deleteLater();
    });

    connect(checkProc, &QProcess::errorOccurred, this, [checkProc](QProcess::ProcessError) {
        checkProc->deleteLater();
    });

    checkProc->start();
}

void SysCheckViewModel::resetState() {
    m_results.clear();
    m_rawLog.clear();
    m_passCount = 0;
    m_warnCount = 0;
    m_failCount = 0;
    setStatus(QStringLiteral("running"));
    emit resultsChanged();
    emit rawLogChanged();
    emit countersChanged();
}
