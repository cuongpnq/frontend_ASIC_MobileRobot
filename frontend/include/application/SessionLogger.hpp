#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QJsonArray>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QHash>
#include <QFile>
#include <QMutex>

/**
 * @brief Singleton that records a structured JSON telemetry log for each robot
 *        test scenario.
 *
 *  Session lifecycle
 *  ─────────────────
 *  startSession()  – called automatically when run_nav.sh is launched
 *  endSession()    – called automatically when the nav process finishes
 *
 *  Event categories
 *  ────────────────
 *  "boot"        – sys-check summary, nav process start/stop
 *  "navigation"  – robot state changes, checkpoint arrivals, commands
 *  "ui"          – state machine view transitions
 *  "ui_latency"  – QML → C++ handler round-trip time
 *  "perf"        – CPU/RAM/Net/FPS samples (1 s interval)
 *  "chat"        – chatbot query_start, first_token (TTFT), complete (tokens/s)
 *  "error"       – Qt warning / critical messages
 *
 *  UI latency measurement
 *  ──────────────────────
 *  QML calls    : SessionLogger.logInteractionStart("action_id")  (Q_INVOKABLE)
 *  C++ handler  : SessionLogger::instance().logInteractionEnd("action_id")
 *  The elapsed_ms is computed and stored in the JSON entry.
 *
 *  Log file location
 *  ─────────────────
 *  ~/.local/share/frontend_app/logs/session_<UTC>_<floor>.json
 *  Falls back to <appDir>/logs/ if the above is not writable.
 */
class SessionLogger : public QObject {
    Q_OBJECT
public:
    static SessionLogger& instance();

    // ── Session lifecycle ─────────────────────────────────────────────
    void startSession(const QString& floor, const QString& mapId);
    void endSession();
    bool isSessionActive() const { return m_active; }

    // ── General event ─────────────────────────────────────────────────
    void logEvent(const QString& category,
                  const QString& event,
                  const QVariantMap& payload = {});

    // ── Performance sample (called every 1 s from DiagnosticsVM) ──────
    void logPerformanceSample(double cpu, double ram,
                              double netRxKBps, double netTxKBps,
                              int fps);

    // ── UI latency measurement ────────────────────────────────────────
    /**
     * Call from QML right before invoking a ViewModel method.
     * @param actionId  Unique label matching the paired logInteractionEnd call.
     */
    Q_INVOKABLE void logInteractionStart(const QString& actionId);

    /**
     * Call from C++ or QML. Records elapsed_ms since logInteractionStart.
     */
    Q_INVOKABLE void logInteractionEnd(const QString& actionId, const QString& description = {});

private:
    explicit SessionLogger(QObject* parent = nullptr);
    ~SessionLogger() override;

    void appendEvent(const QString& category, const QJsonObject& entry);
    void flushToDisk(const QString& category);
    QString resolveLogDir() const;
    QString buildFilePath(const QString& category) const;

    bool         m_active     = false;
    QString      m_floor;
    QString      m_mapId;
    QString      m_sessionTimestamp;
    QHash<QString, QJsonArray> m_categoryEvents; // category -> event array
    QJsonObject  m_sessionMeta;

    // Session-level summary counters
    int          m_checkpointCount  = 0;
    int          m_emergencyStops   = 0;
    int          m_resets           = 0;
    QStringList  m_visitedCps;

    // UI latency tracking
    QHash<QString, qint64>  m_interactionStartNs; // actionId → start ns
    QElapsedTimer            m_elapsedTimer;       // monotonic clock

    // Last known system performance metrics
    double       m_lastCpuPct       = 0.0;
    double       m_lastRamPct       = 0.0;

    mutable QMutex m_mutex;
};
