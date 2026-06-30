#include "application/SessionLogger.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QMutexLocker>
#include <QDebug>
#include <QStandardPaths>

// ═══════════════════════════════════════════════════════════════════
//  Singleton
// ═══════════════════════════════════════════════════════════════════

SessionLogger& SessionLogger::instance()
{
    static SessionLogger inst;
    return inst;
}

SessionLogger::SessionLogger(QObject* parent)
    : QObject(parent)
{
    m_elapsedTimer.start();
}

SessionLogger::~SessionLogger()
{
    if (m_active)
        endSession();
}

// ═══════════════════════════════════════════════════════════════════
//  Session lifecycle
// ═══════════════════════════════════════════════════════════════════

void SessionLogger::startSession(const QString& floor, const QString& mapId)
{
    QMutexLocker lock(&m_mutex);

    if (m_active) {
        // Close the previous session first (e.g. map switch / relaunch)
        lock.unlock();
        endSession();
        lock.relock();
    }

    m_floor  = floor.isEmpty()  ? QStringLiteral("unknown") : floor;
    m_mapId  = mapId.isEmpty()  ? m_floor : mapId;

    m_categoryEvents.clear();

    m_checkpointCount = 0;
    m_emergencyStops  = 0;
    m_resets          = 0;
    m_visitedCps.clear();
    m_interactionStartNs.clear();

    m_sessionTimestamp = QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd_HHmmss"));
    m_active   = true;

    const QString startTs = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);

    m_sessionMeta = QJsonObject{
        { QStringLiteral("start"),  startTs },
        { QStringLiteral("floor"),  m_floor  },
        { QStringLiteral("mapId"),  m_mapId  }
    };

    qDebug() << "[SessionLogger] Session started with timestamp" << m_sessionTimestamp;

    // Initialize all known feature categories so their empty log files exist with session headers
    const QStringList categories = {
        QStringLiteral("boot"),
        QStringLiteral("navigation"),
        QStringLiteral("ui"),
        QStringLiteral("ui_latency"),
        QStringLiteral("perf"),
        QStringLiteral("chat"),
        QStringLiteral("error")
    };
    for (const auto& cat : categories) {
        m_categoryEvents[cat] = QJsonArray();
        flushToDisk(cat);
    }
}

void SessionLogger::endSession()
{
    QMutexLocker lock(&m_mutex);

    if (!m_active)
        return;

    m_active = false;

    const QString endTs    = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);

    // Build summary from counters
    QJsonArray cpArray;
    for (const auto& cp : m_visitedCps)
        cpArray.append(cp);

    QJsonObject summary{
        { QStringLiteral("end"),                  endTs },
        { QStringLiteral("checkpoints_visited"),  cpArray },
        { QStringLiteral("emergency_stops"),      m_emergencyStops },
        { QStringLiteral("resets"),               m_resets }
    };

    // Inject summary and flush
    // (m_sessionMeta already has start/floor/mapId)
    m_sessionMeta[QStringLiteral("summary")] = summary;

    for (auto it = m_categoryEvents.begin(); it != m_categoryEvents.end(); ++it) {
        flushToDisk(it.key());
    }
    qDebug() << "[SessionLogger] Session ended for timestamp" << m_sessionTimestamp;
}

// ═══════════════════════════════════════════════════════════════════
//  General event
// ═══════════════════════════════════════════════════════════════════

void SessionLogger::logEvent(const QString& category,
                              const QString& event,
                              const QVariantMap& payload)
{
    QMutexLocker lock(&m_mutex);

    if (!m_active)
        return;

    QJsonObject entry{
        { QStringLiteral("t"),     QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs) },
        { QStringLiteral("cat"),   category },
        { QStringLiteral("event"), event }
    };

    for (auto it = payload.cbegin(); it != payload.cend(); ++it)
        entry[it.key()] = QJsonValue::fromVariant(it.value());

    appendEvent(category, entry);

    // Update summary counters
    if (category == QLatin1String("navigation")) {
        if (event == QLatin1String("checkpoint_arrived")) {
            ++m_checkpointCount;
            const QString cpLabel = payload.value(QStringLiteral("name"),
                                                   payload.value(QStringLiteral("id"))).toString();
            if (!cpLabel.isEmpty())
                m_visitedCps.append(cpLabel);
        } else if (event == QLatin1String("emergency_stop")) {
            if (payload.value(QStringLiteral("active")).toBool())
                ++m_emergencyStops;
        } else if (event == QLatin1String("reset_requested")) {
            ++m_resets;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Performance sample
// ═══════════════════════════════════════════════════════════════════

void SessionLogger::logPerformanceSample(double cpu, double ram,
                                          double netRxKBps, double netTxKBps,
                                          int fps)
{
    QMutexLocker lock(&m_mutex);

    if (!m_active)
        return;

    QJsonObject entry{
        { QStringLiteral("t"),        QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs) },
        { QStringLiteral("cat"),      QStringLiteral("perf") },
        { QStringLiteral("cpu_pct"),  qRound(cpu  * 10) / 10.0 },
        { QStringLiteral("ram_pct"),  qRound(ram  * 10) / 10.0 },
        { QStringLiteral("net_rx_kbps"), qRound(netRxKBps * 10) / 10.0 },
        { QStringLiteral("net_tx_kbps"), qRound(netTxKBps * 10) / 10.0 },
        { QStringLiteral("fps"),      fps }
    };

    appendEvent(QStringLiteral("perf"), entry);
}

// ═══════════════════════════════════════════════════════════════════
//  UI latency
// ═══════════════════════════════════════════════════════════════════

void SessionLogger::logInteractionStart(const QString& actionId)
{
    QMutexLocker lock(&m_mutex);

    if (!m_active)
        return;

    // Record high-resolution monotonic timestamp (nanoseconds)
    m_interactionStartNs[actionId] = m_elapsedTimer.nsecsElapsed();
}

void SessionLogger::logInteractionEnd(const QString& actionId,
                                       const QString& description)
{
    QMutexLocker lock(&m_mutex);

    if (!m_active)
        return;

    auto it = m_interactionStartNs.find(actionId);
    if (it == m_interactionStartNs.end()) {
        qWarning() << "[SessionLogger] logInteractionEnd: no matching start for" << actionId;
        return;
    }

    const qint64 endNs     = m_elapsedTimer.nsecsElapsed();
    const qint64 elapsedNs = endNs - it.value();
    const double elapsedMs = double(elapsedNs) / 1'000'000.0;
    m_interactionStartNs.erase(it);

    QJsonObject entry{
        { QStringLiteral("t"),           QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs) },
        { QStringLiteral("cat"),         QStringLiteral("ui_latency") },
        { QStringLiteral("action"),      actionId },
        { QStringLiteral("elapsed_ms"),  elapsedMs }
    };
    if (!description.isEmpty())
        entry[QStringLiteral("desc")] = description;

    appendEvent(QStringLiteral("ui_latency"), entry);

    qDebug() << "[SessionLogger] UI latency [" << actionId << "] ="
             << QString::number(elapsedMs, 'f', 2) << "ms";
}

// ═══════════════════════════════════════════════════════════════════
//  Internal helpers
// ═══════════════════════════════════════════════════════════════════

void SessionLogger::appendEvent(const QString& category, const QJsonObject& entry)
{
    // Caller must already hold m_mutex
    m_categoryEvents[category].append(entry);

    // Flush every 50 events to protect against crash data loss
    if (m_categoryEvents[category].size() % 50 == 0)
        flushToDisk(category);
}

void SessionLogger::flushToDisk(const QString& category)
{
    // Caller must already hold m_mutex (or called from endSession / startSession)
    const QString filePath = buildFilePath(category);
    if (filePath.isEmpty())
        return;

    QJsonObject root;
    root[QStringLiteral("session")] = m_sessionMeta;
    root[QStringLiteral("events")]  = m_categoryEvents.value(category);

    QJsonDocument doc(root);

    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "[SessionLogger] Cannot write log file:" << filePath << f.errorString();
        return;
    }
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
}

QString SessionLogger::resolveLogDir() const
{
    // Prefer XDG data location: ~/.local/share/<AppName>/logs
    const QString xdgDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                           + QStringLiteral("/logs");
    if (QDir().mkpath(xdgDir))
        return xdgDir;

    // Fallback: next to the binary
    const QString binDir = QCoreApplication::applicationDirPath() + QStringLiteral("/logs");
    QDir().mkpath(binDir);
    return binDir;
}

QString SessionLogger::buildFilePath(const QString& category) const
{
    const QString name = QStringLiteral("session_%1_%2_%3.json").arg(m_sessionTimestamp, m_floor, category);
    return resolveLogDir() + QStringLiteral("/") + name;
}
