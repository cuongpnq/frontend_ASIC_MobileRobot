#pragma once

#include <QObject>
#include <QVariantList>

class PerformanceMonitor;
class QQuickWindow;
class QTimer;

class DiagnosticsViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)

    // FPS tracking
    Q_PROPERTY(int fps READ fps NOTIFY fpsChanged)
    Q_PROPERTY(QVariantList fpsHistory READ fpsHistory NOTIFY fpsHistoryChanged)

    // ── Forwarded from PerformanceMonitor ────────────────────────
    Q_PROPERTY(double cpuUsage READ cpuUsage NOTIFY cpuUsageChanged)
    Q_PROPERTY(double ramUsage READ ramUsage NOTIFY ramUsageChanged)
    Q_PROPERTY(double ramUsedMB READ ramUsedMB NOTIFY ramUsageChanged)
    Q_PROPERTY(double ramTotalMB READ ramTotalMB NOTIFY ramUsageChanged)

    Q_PROPERTY(double netRxKBps READ netRxKBps NOTIFY netUsageChanged)
    Q_PROPERTY(double netTxKBps READ netTxKBps NOTIFY netUsageChanged)
    Q_PROPERTY(double totalRxMB READ totalRxMB NOTIFY netUsageChanged)
    Q_PROPERTY(double totalTxMB READ totalTxMB NOTIFY netUsageChanged)

    Q_PROPERTY(double diskUsage READ diskUsage NOTIFY diskUsageChanged)
    Q_PROPERTY(double diskUsedGB READ diskUsedGB NOTIFY diskUsageChanged)
    Q_PROPERTY(double diskTotalGB READ diskTotalGB NOTIFY diskUsageChanged)

    Q_PROPERTY(QVariantList cpuHistory READ cpuHistory NOTIFY cpuHistoryChanged)
    Q_PROPERTY(QVariantList ramHistory READ ramHistory NOTIFY ramHistoryChanged)

public:
    explicit DiagnosticsViewViewModel(QObject* parent = nullptr);
    ~DiagnosticsViewViewModel() override = default;

    bool isActive() const;
    int  fps() const;
    QVariantList fpsHistory() const;

    // Forwarded getters
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

    Q_INVOKABLE void requestControlCenterView();

signals:
    void isActiveChanged();
    void fpsChanged();
    void fpsHistoryChanged();
    void cpuUsageChanged();
    void ramUsageChanged();
    void netUsageChanged();
    void diskUsageChanged();
    void cpuHistoryChanged();
    void ramHistoryChanged();

private slots:
    void onStateMachineChanged();
    void onFrameRendered();
    void onFpsTimerFired();

private:
    void tryConnectWindow();

    bool m_isActive = false;

    // FPS
    int           m_fps = 0;
    int           m_frameCount = 0;
    QTimer*       m_fpsTimer = nullptr;
    QQuickWindow* m_window = nullptr;
    bool          m_windowConnected = false;

    // History (60 samples)
    static constexpr int FPS_HISTORY_SIZE = 60;
    QVariantList m_fpsHistory;

    PerformanceMonitor* m_perfMonitor = nullptr;
};
