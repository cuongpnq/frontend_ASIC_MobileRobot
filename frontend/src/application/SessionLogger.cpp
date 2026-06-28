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

    m_events.~QJsonArray();
    new (&m_events) QJsonArray();

    m_checkpointCount = 0;
    m_emergencyStops  = 0;
    m_resets          = 0;
    m_visitedCps.clear();
    m_interactionStartNs.clear();

    m_filePath = buildFilePath(m_floor);
    m_active   = true;

    const QString startTs = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);

    m_sessionMeta = QJsonObject{
        { QStringLiteral("start"),  startTs },
        { QStringLiteral("floor"),  m_floor  },
        { QStringLiteral("mapId"),  m_mapId  }
    };

    qDebug() << "[SessionLogger] Session started →" << m_filePath;

    // Write file header immediately so the file exists even if the app crashes
    flushToDisk();
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

    flushToDisk();
    qDebug() << "[SessionLogger] Session ended →" << m_filePath;
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

    appendEvent(entry);

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

    appendEvent(entry);
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

    appendEvent(entry);

    qDebug() << "[SessionLogger] UI latency [" << actionId << "] ="
             << QString::number(elapsedMs, 'f', 2) << "ms";
}

// ═══════════════════════════════════════════════════════════════════
//  Internal helpers
// ═══════════════════════════════════════════════════════════════════

void SessionLogger::appendEvent(const QJsonObject& entry)
{
    // Caller must already hold m_mutex
    m_events.append(entry);

    // Flush every 50 events to protect against crash data loss
    if (m_events.size() % 50 == 0)
        flushToDisk();
}

void SessionLogger::flushToDisk()
{
    // Caller must already hold m_mutex (or called from endSession / startSession)
    if (m_filePath.isEmpty())
        return;

    QJsonObject root;
    root[QStringLiteral("session")] = m_sessionMeta;
    root[QStringLiteral("events")]  = m_events;

    QJsonDocument doc(root);

    QFile f(m_filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "[SessionLogger] Cannot write log file:" << m_filePath << f.errorString();
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

QString SessionLogger::buildFilePath(const QString& floor) const
{
    const QString ts = QDateTime::currentDateTimeUtc()
                           .toString(QStringLiteral("yyyyMMdd_HHmmss"));
    const QString name = QStringLiteral("session_%1_%2.json").arg(ts, floor);
    return resolveLogDir() + QStringLiteral("/") + name;
}
