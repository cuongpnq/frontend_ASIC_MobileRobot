#include "diagnosticsView/DiagnosticsViewViewModel.hpp"
#include "diagnosticsView/PerformanceMonitor.hpp"
#include "application/AppStateMachine.hpp"

#include <QGuiApplication>
#include <QQuickWindow>
#include <QTimer>
#include <QDebug>
#include <QVariantList>

// ═══════════════════════════════════════════════════════════════════
//  Construction
// ═══════════════════════════════════════════════════════════════════

DiagnosticsViewViewModel::DiagnosticsViewViewModel(QObject* parent)
    : QObject(parent), m_isActive(false)
{
    // State machine connection
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &DiagnosticsViewViewModel::onStateMachineChanged);

    // Performance monitor (CPU / RAM / Net / Disk)
    m_perfMonitor = new PerformanceMonitor(this);

    // Forward all PerformanceMonitor signals through this ViewModel
    connect(m_perfMonitor, &PerformanceMonitor::cpuUsageChanged,
            this, &DiagnosticsViewViewModel::cpuUsageChanged);
    connect(m_perfMonitor, &PerformanceMonitor::ramUsageChanged,
            this, &DiagnosticsViewViewModel::ramUsageChanged);
    connect(m_perfMonitor, &PerformanceMonitor::netUsageChanged,
            this, &DiagnosticsViewViewModel::netUsageChanged);
    connect(m_perfMonitor, &PerformanceMonitor::diskUsageChanged,
            this, &DiagnosticsViewViewModel::diskUsageChanged);
    connect(m_perfMonitor, &PerformanceMonitor::cpuHistoryChanged,
            this, &DiagnosticsViewViewModel::cpuHistoryChanged);
    connect(m_perfMonitor, &PerformanceMonitor::ramHistoryChanged,
            this, &DiagnosticsViewViewModel::ramHistoryChanged);

    // Pre-fill FPS history with zeros
    for (int i = 0; i < FPS_HISTORY_SIZE; ++i)
        m_fpsHistory.append(0);

    // FPS counter timer — fires once per second
    m_fpsTimer = new QTimer(this);
    m_fpsTimer->setInterval(1000);
    connect(m_fpsTimer, &QTimer::timeout,
            this, &DiagnosticsViewViewModel::onFpsTimerFired);
    m_fpsTimer->start();

    // Attempt to connect to the render window
    // (may not exist yet if QML hasn't loaded)
    QTimer::singleShot(500, this, &DiagnosticsViewViewModel::tryConnectWindow);
}

// ═══════════════════════════════════════════════════════════════════
//  Window connection for FPS
// ═══════════════════════════════════════════════════════════════════

void DiagnosticsViewViewModel::tryConnectWindow()
{
    if (m_windowConnected)
        return;

    const auto windows = QGuiApplication::allWindows();
    for (auto *w : windows) {
        auto *qw = qobject_cast<QQuickWindow *>(w);
        if (qw) {
            m_window = qw;
            connect(qw, &QQuickWindow::afterRendering,
                    this, &DiagnosticsViewViewModel::onFrameRendered,
                    Qt::DirectConnection);
            m_windowConnected = true;
            qDebug() << "[DiagnosticsVM] Connected to QQuickWindow for FPS tracking";
            return;
        }
    }

    // Retry after another 500 ms if window isn't ready yet
    QTimer::singleShot(500, this, &DiagnosticsViewViewModel::tryConnectWindow);
}

// ═══════════════════════════════════════════════════════════════════
//  Getters
// ═══════════════════════════════════════════════════════════════════

bool DiagnosticsViewViewModel::isActive() const { return m_isActive; }
int  DiagnosticsViewViewModel::fps()      const { return m_fps; }
QVariantList DiagnosticsViewViewModel::fpsHistory() const { return m_fpsHistory; }

// Forwarded from PerformanceMonitor
double DiagnosticsViewViewModel::cpuUsage()    const { return m_perfMonitor->cpuUsage(); }
double DiagnosticsViewViewModel::ramUsage()    const { return m_perfMonitor->ramUsage(); }
double DiagnosticsViewViewModel::ramUsedMB()   const { return m_perfMonitor->ramUsedMB(); }
double DiagnosticsViewViewModel::ramTotalMB()  const { return m_perfMonitor->ramTotalMB(); }
double DiagnosticsViewViewModel::netRxKBps()   const { return m_perfMonitor->netRxKBps(); }
double DiagnosticsViewViewModel::netTxKBps()   const { return m_perfMonitor->netTxKBps(); }
double DiagnosticsViewViewModel::totalRxMB()   const { return m_perfMonitor->totalRxMB(); }
double DiagnosticsViewViewModel::totalTxMB()   const { return m_perfMonitor->totalTxMB(); }
double DiagnosticsViewViewModel::diskUsage()   const { return m_perfMonitor->diskUsage(); }
double DiagnosticsViewViewModel::diskUsedGB()  const { return m_perfMonitor->diskUsedGB(); }
double DiagnosticsViewViewModel::diskTotalGB() const { return m_perfMonitor->diskTotalGB(); }
QVariantList DiagnosticsViewViewModel::cpuHistory() const { return m_perfMonitor->cpuHistory(); }
QVariantList DiagnosticsViewViewModel::ramHistory() const { return m_perfMonitor->ramHistory(); }

// ═══════════════════════════════════════════════════════════════════
//  Navigation
// ═══════════════════════════════════════════════════════════════════

void DiagnosticsViewViewModel::requestControlCenterView() {
    AppStateMachine::instance().returnToControlCenter();
}

// ═══════════════════════════════════════════════════════════════════
//  State machine
// ═══════════════════════════════════════════════════════════════════

void DiagnosticsViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "DiagnosticsView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}

// ═══════════════════════════════════════════════════════════════════
//  FPS tracking
// ═══════════════════════════════════════════════════════════════════

void DiagnosticsViewViewModel::onFrameRendered()
{
    ++m_frameCount;
}

void DiagnosticsViewViewModel::onFpsTimerFired()
{
    int newFps = m_frameCount;
    m_frameCount = 0;

    if (newFps != m_fps) {
        m_fps = newFps;
        emit fpsChanged();
    }

    // Push to history
    m_fpsHistory.removeFirst();
    m_fpsHistory.append(m_fps);
    emit fpsHistoryChanged();
}
