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
    m_categorySessions.clear();

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

    // Initialize all known feature log files so they exist from session start
    const QStringList features = {
        QStringLiteral("boot"),
        QStringLiteral("direction_view"),
        QStringLiteral("running_view"),
        QStringLiteral("chat"),
        QStringLiteral("control_center"),
        QStringLiteral("ui"),
        QStringLiteral("error")
    };

    for (const auto& feat : features) {
        const QString filePath = buildFilePath(feat);
        QJsonArray existingEvents;
        QJsonArray existingSessions;

        QFile f(filePath);
        if (f.exists() && f.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
            f.close();
            if (doc.isObject()) {
                QJsonObject root = doc.object();
                if (root.contains(QStringLiteral("events"))) {
                    existingEvents = root.value(QStringLiteral("events")).toArray();
                }
                if (root.contains(QStringLiteral("sessions"))) {
                    existingSessions = root.value(QStringLiteral("sessions")).toArray();
                } else if (root.contains(QStringLiteral("session"))) {
                    existingSessions.append(root.value(QStringLiteral("session")).toObject());
                }
            }
        }

        m_categoryEvents[feat] = existingEvents;

        // Append current session meta to the loaded sessions
        existingSessions.append(m_sessionMeta);
        m_categorySessions[feat] = existingSessions;

        flushToDisk(feat);
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

    // Update the current session entry in all feature's session array
    for (auto it = m_categorySessions.begin(); it != m_categorySessions.end(); ++it) {
        QJsonArray sessions = it.value();
        if (!sessions.isEmpty()) {
            sessions.replace(sessions.size() - 1, m_sessionMeta);
            *it = sessions;
        }
    }

    for (auto it = m_categoryEvents.begin(); it != m_categoryEvents.end(); ++it) {
        flushToDisk(it.key());
    }
    qDebug() << "[SessionLogger] Session ended for timestamp" << m_sessionTimestamp;
}

// ═══════════════════════════════════════════════════════════════════
//  General event
// ═══════════════════════════════════════════════════════════════════

// Feature-aware overload: feature drives the filename
void SessionLogger::logEvent(const QString& feature,
                              const QString& category,
                              const QString& event,
                              const QVariantMap& payload)
{
    QMutexLocker lock(&m_mutex);

    if (!m_active)
        return;

    QJsonObject entry{
        { QStringLiteral("t"),        QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs) },
        { QStringLiteral("feature"),  feature  },
        { QStringLiteral("cat"),      category },
        { QStringLiteral("event"),    event    },
        { QStringLiteral("cpu_pct"),  qRound(m_lastCpuPct * 10) / 10.0 },
        { QStringLiteral("ram_pct"),  qRound(m_lastRamPct * 10) / 10.0 }
    };

    for (auto it = payload.cbegin(); it != payload.cend(); ++it)
        entry[it.key()] = QJsonValue::fromVariant(it.value());

    appendEvent(feature, entry);

    // Update summary counters (same logic, category-driven)
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

// Backward-compatible overload: category used as both category and feature
void SessionLogger::logEvent(const QString& category,
                              const QString& event,
                              const QVariantMap& payload)
{
    logEvent(category, category, event, payload);
}

// ═══════════════════════════════════════════════════════════════════
//  Performance sample
// ═══════════════════════════════════════════════════════════════════

void SessionLogger::logPerformanceSample(double cpu, double ram,
                                          double netRxKBps, double netTxKBps,
                                          int fps)
{
    QMutexLocker lock(&m_mutex);

    m_lastCpuPct = cpu;
    m_lastRamPct = ram;

    if (!m_active)
        return;

    QJsonObject entry{
        { QStringLiteral("t"),           QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs) },
        { QStringLiteral("feature"),      QStringLiteral("control_center") },
        { QStringLiteral("cat"),          QStringLiteral("perf") },
        { QStringLiteral("event"),        QStringLiteral("perf_sample") },
        { QStringLiteral("cpu_pct"),      qRound(cpu  * 10) / 10.0 },
        { QStringLiteral("ram_pct"),      qRound(ram  * 10) / 10.0 },
        { QStringLiteral("net_rx_kbps"),  qRound(netRxKBps * 10) / 10.0 },
        { QStringLiteral("net_tx_kbps"),  qRound(netTxKBps * 10) / 10.0 },
        { QStringLiteral("fps"),          fps }
    };

    appendEvent(QStringLiteral("control_center"), entry);
}

// ═══════════════════════════════════════════════════════════════════
//  UI latency
// ═══════════════════════════════════════════════════════════════════

void SessionLogger::logInteractionStart(const QString& actionId, const QString& feature)
{
    QMutexLocker lock(&m_mutex);

    if (!m_active)
        return;

    m_interactionStartNs[actionId] = { m_elapsedTimer.nsecsElapsed(), feature };
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
    const qint64 elapsedNs = endNs - it->startNs;
    const double elapsedMs = double(elapsedNs) / 1'000'000.0;
    const QString feature  = it->feature.isEmpty() ? QStringLiteral("ui") : it->feature;
    m_interactionStartNs.erase(it);

    QJsonObject entry{
        { QStringLiteral("t"),          QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs) },
        { QStringLiteral("feature"),    feature },
        { QStringLiteral("cat"),        QStringLiteral("ui_latency") },
        { QStringLiteral("event"),      QStringLiteral("interaction") },
        { QStringLiteral("action"),     actionId },
        { QStringLiteral("elapsed_ms"), elapsedMs },
        { QStringLiteral("cpu_pct"),    qRound(m_lastCpuPct * 10) / 10.0 },
        { QStringLiteral("ram_pct"),    qRound(m_lastRamPct * 10) / 10.0 }
    };
    if (!description.isEmpty())
        entry[QStringLiteral("desc")] = description;

    appendEvent(feature, entry);

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

    // Flush immediately to guarantee real-time logging and crash resilience
    flushToDisk(category);
}

void SessionLogger::flushToDisk(const QString& category)
{
    // Caller must already hold m_mutex (or called from endSession / startSession)
    const QString filePath = buildFilePath(category);
    if (filePath.isEmpty())
        return;

    QJsonObject root;
    root[QStringLiteral("session")]  = m_sessionMeta; // backward compatibility
    root[QStringLiteral("sessions")] = m_categorySessions.value(category);
    root[QStringLiteral("events")]   = m_categoryEvents.value(category);

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
    const QString name = QStringLiteral("%1.json").arg(category);
    return resolveLogDir() + QStringLiteral("/") + name;
}
