#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QString>

/**
 * @brief PerformanceMonitor polls OS-specific APIs for system metrics.
 *
 * On Linux it reads /proc/stat, /proc/meminfo, and /proc/net/dev.
 * Exposes CPU%, RAM%, network throughput, and disk usage as Q_PROPERTYs.
 */
class PerformanceMonitor : public QObject {
    Q_OBJECT

    // ── CPU ──────────────────────────────────────────────────────
    Q_PROPERTY(double cpuUsage READ cpuUsage NOTIFY cpuUsageChanged)

    // ── RAM ──────────────────────────────────────────────────────
    Q_PROPERTY(double ramUsage READ ramUsage NOTIFY ramUsageChanged)
    Q_PROPERTY(double ramUsedMB READ ramUsedMB NOTIFY ramUsageChanged)
    Q_PROPERTY(double ramTotalMB READ ramTotalMB NOTIFY ramUsageChanged)

    // ── Network / Data Usage ────────────────────────────────────
    Q_PROPERTY(double netRxKBps READ netRxKBps NOTIFY netUsageChanged)
    Q_PROPERTY(double netTxKBps READ netTxKBps NOTIFY netUsageChanged)
    Q_PROPERTY(double totalRxMB READ totalRxMB NOTIFY netUsageChanged)
    Q_PROPERTY(double totalTxMB READ totalTxMB NOTIFY netUsageChanged)

    // ── Disk ────────────────────────────────────────────────────
    Q_PROPERTY(double diskUsage READ diskUsage NOTIFY diskUsageChanged)
    Q_PROPERTY(double diskUsedGB READ diskUsedGB NOTIFY diskUsageChanged)
    Q_PROPERTY(double diskTotalGB READ diskTotalGB NOTIFY diskUsageChanged)

    // ── History (for mini-graphs) ───────────────────────────────
    Q_PROPERTY(QVariantList cpuHistory READ cpuHistory NOTIFY cpuHistoryChanged)
    Q_PROPERTY(QVariantList ramHistory READ ramHistory NOTIFY ramHistoryChanged)

public:
    explicit PerformanceMonitor(QObject *parent = nullptr);
    ~PerformanceMonitor() override = default;

    // Getters
    double cpuUsage() const;
    double ramUsage() const;
    double ramUsedMB() const;
    double ramTotalMB() const;
    double netRxKBps() const;
    double netTxKBps() const;
    double totalRxMB() const;
    double totalTxMB() const;
    double diskUsage() const;
    double diskUsedGB() const;
    double diskTotalGB() const;

    QVariantList cpuHistory() const;
    QVariantList ramHistory() const;

signals:
    void cpuUsageChanged();
    void ramUsageChanged();
    void netUsageChanged();
    void diskUsageChanged();
    void cpuHistoryChanged();
    void ramHistoryChanged();

private slots:
    void poll();

private:
    void pollCpu();
    void pollRam();
    void pollNet();
    void pollDisk();
    QString resolveActiveInterface() const;
    bool readInterfaceBytes(const QString &iface, quint64 &rxBytes, quint64 &txBytes) const;

    QTimer m_timer;

    // CPU
    double m_cpuUsage = 0.0;
    quint64 m_prevCpuIdle = 0;
    quint64 m_prevCpuTotal = 0;

    // RAM
    double m_ramUsage = 0.0;
    double m_ramUsedMB = 0.0;
    double m_ramTotalMB = 0.0;

    // Network
    double m_netRxKBps = 0.0;
    double m_netTxKBps = 0.0;
    double m_totalRxMB = 0.0;
    double m_totalTxMB = 0.0;
    quint64 m_prevRxBytes = 0;
    quint64 m_prevTxBytes = 0;
    bool    m_netFirstPoll = true;
    QString m_monitoredNetIface;
    int     m_ifaceRefreshCountdown = 0;

    // Disk
    double m_diskUsage = 0.0;
    double m_diskUsedGB = 0.0;
    double m_diskTotalGB = 0.0;

    // History buffers (last 60 samples ≈ 60 seconds)
    static constexpr int HISTORY_SIZE = 60;
    QVariantList m_cpuHistory;
    QVariantList m_ramHistory;
};
