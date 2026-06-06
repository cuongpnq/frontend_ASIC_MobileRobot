#include "preCheckView/SysCheckViewModel.hpp"

#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QRegularExpression>
#include <QVariantMap>

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
}

SysCheckViewModel::~SysCheckViewModel() {
    if (m_process) {
        m_process->kill();
        m_process->waitForFinished(1000);
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

// ═══════════════════════════════════════════════════════════════════
//  Boot mode
// ═══════════════════════════════════════════════════════════════════

void SysCheckViewModel::autoStartBootCheck() {
    qDebug() << "[SysCheckVM] Auto-starting boot check";
    runSysCheck(QStringLiteral("a1"), /*skipBuild=*/true, /*checkTopics=*/false);
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
