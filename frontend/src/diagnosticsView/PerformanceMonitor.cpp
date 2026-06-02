#include "diagnosticsView/PerformanceMonitor.hpp"

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QStorageInfo>
#include <QDebug>

// ═══════════════════════════════════════════════════════════════════
//  Construction
// ═══════════════════════════════════════════════════════════════════

PerformanceMonitor::PerformanceMonitor(QObject *parent)
    : QObject(parent)
{
    // Pre-fill history with zeros so the graph always has data points
    for (int i = 0; i < HISTORY_SIZE; ++i) {
        m_cpuHistory.append(0.0);
        m_ramHistory.append(0.0);
    }

    connect(&m_timer, &QTimer::timeout, this, &PerformanceMonitor::poll);
    m_timer.start(1000); // poll every 1 second

    // Initial poll so we don't start with blank values
    poll();
}

// ═══════════════════════════════════════════════════════════════════
//  Getters
// ═══════════════════════════════════════════════════════════════════

double PerformanceMonitor::cpuUsage()   const { return m_cpuUsage; }
double PerformanceMonitor::ramUsage()   const { return m_ramUsage; }
double PerformanceMonitor::ramUsedMB()  const { return m_ramUsedMB; }
double PerformanceMonitor::ramTotalMB() const { return m_ramTotalMB; }
double PerformanceMonitor::netRxKBps()  const { return m_netRxKBps; }
double PerformanceMonitor::netTxKBps()  const { return m_netTxKBps; }
double PerformanceMonitor::totalRxMB()  const { return m_totalRxMB; }
double PerformanceMonitor::totalTxMB()  const { return m_totalTxMB; }
double PerformanceMonitor::diskUsage()  const { return m_diskUsage; }
double PerformanceMonitor::diskUsedGB() const { return m_diskUsedGB; }
double PerformanceMonitor::diskTotalGB()const { return m_diskTotalGB; }

QVariantList PerformanceMonitor::cpuHistory() const { return m_cpuHistory; }
QVariantList PerformanceMonitor::ramHistory() const { return m_ramHistory; }

// ═══════════════════════════════════════════════════════════════════
//  Main poll slot
// ═══════════════════════════════════════════════════════════════════

void PerformanceMonitor::poll()
{
    pollCpu();
    pollRam();
    pollNet();
    pollDisk();
}

// ═══════════════════════════════════════════════════════════════════
//  CPU — /proc/stat
// ═══════════════════════════════════════════════════════════════════

void PerformanceMonitor::pollCpu()
{
#ifdef Q_OS_LINUX
    QFile f("/proc/stat");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    // First line: "cpu  user nice system idle iowait irq softirq steal ..."
    QString line = QTextStream(&f).readLine();
    f.close();

    QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    if (parts.size() < 5)
        return;

    // Sum all jiffies except the label "cpu"
    quint64 idle  = parts[4].toULongLong();          // idle
    if (parts.size() > 5) idle += parts[5].toULongLong(); // iowait

    quint64 total = 0;
    for (int i = 1; i < parts.size(); ++i)
        total += parts[i].toULongLong();

    quint64 diffIdle  = idle  - m_prevCpuIdle;
    quint64 diffTotal = total - m_prevCpuTotal;

    m_prevCpuIdle  = idle;
    m_prevCpuTotal = total;

    if (diffTotal > 0) {
        m_cpuUsage = 100.0 * (1.0 - double(diffIdle) / double(diffTotal));
        emit cpuUsageChanged();
    }

    // Push into history ring buffer
    m_cpuHistory.removeFirst();
    m_cpuHistory.append(m_cpuUsage);
    emit cpuHistoryChanged();
#endif
}

// ═══════════════════════════════════════════════════════════════════
//  RAM — /proc/meminfo
// ═══════════════════════════════════════════════════════════════════

void PerformanceMonitor::pollRam()
{
#ifdef Q_OS_LINUX
    QFile f("/proc/meminfo");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "[PerfMon] RAM: cannot open /proc/meminfo";
        return;
    }

    quint64 totalKB = 0, availableKB = 0;

    QTextStream in(&f);
    static int debugCount = 0;
    while (true) {
        QString line = in.readLine();
        if (line.isNull())
            break;

        // Use section(':') to get the part after the colon, then trim and extract number
        if (line.startsWith("MemTotal:")) {
            QString valueStr = line.section(':', 1).trimmed().split(' ').first();
            totalKB = valueStr.toULongLong();
            if (debugCount < 2)
                qDebug() << "[PerfMon] MemTotal raw:" << line << "-> parsed:" << totalKB;
        } else if (line.startsWith("MemAvailable:")) {
            QString valueStr = line.section(':', 1).trimmed().split(' ').first();
            availableKB = valueStr.toULongLong();
            if (debugCount < 2)
                qDebug() << "[PerfMon] MemAvailable raw:" << line << "-> parsed:" << availableKB;
        }
    }
    f.close();
    debugCount++;

    if (totalKB == 0) {
        return;
    }

    quint64 usedKB = (totalKB > availableKB) ? (totalKB - availableKB) : 0;
    double usage   = 100.0 * double(usedKB) / double(totalKB);
    double usedMB  = double(usedKB)  / 1024.0;
    double totalMB = double(totalKB) / 1024.0;

    m_ramUsage   = usage;
    m_ramUsedMB  = usedMB;
    m_ramTotalMB = totalMB;
    emit ramUsageChanged();

    m_ramHistory.removeFirst();
    m_ramHistory.append(m_ramUsage);
    emit ramHistoryChanged();
#endif
}

// ═══════════════════════════════════════════════════════════════════
//  Network — /proc/net/dev  (all interfaces combined)
// ═══════════════════════════════════════════════════════════════════

void PerformanceMonitor::pollNet()
{
#ifdef Q_OS_LINUX
    QFile f("/proc/net/dev");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    quint64 rxTotal = 0, txTotal = 0;
    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.contains(':'))
            continue;

        QString iface = line.section(':', 0, 0).trimmed();
        // Skip loopback
        if (iface == "lo")
            continue;

        QStringList cols = line.section(':', 1).trimmed().split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if (cols.size() < 9)
            continue;

        rxTotal += cols[0].toULongLong();   // bytes received
        txTotal += cols[8].toULongLong();   // bytes transmitted
    }
    f.close();

    if (m_netFirstPoll) {
        m_prevRxBytes  = rxTotal;
        m_prevTxBytes  = txTotal;
        m_netFirstPoll = false;
        return;
    }

    quint64 diffRx = rxTotal - m_prevRxBytes;
    quint64 diffTx = txTotal - m_prevTxBytes;
    m_prevRxBytes = rxTotal;
    m_prevTxBytes = txTotal;

    m_netRxKBps = double(diffRx) / 1024.0;  // per second (timer = 1 s)
    m_netTxKBps = double(diffTx) / 1024.0;
    m_totalRxMB = double(rxTotal) / (1024.0 * 1024.0);
    m_totalTxMB = double(txTotal) / (1024.0 * 1024.0);

    emit netUsageChanged();
#endif
}

// ═══════════════════════════════════════════════════════════════════
//  Disk — QStorageInfo (cross-platform)
// ═══════════════════════════════════════════════════════════════════

void PerformanceMonitor::pollDisk()
{
    QStorageInfo storage = QStorageInfo::root();
    double totalGB = double(storage.bytesTotal()) / (1024.0 * 1024.0 * 1024.0);
    double freeGB  = double(storage.bytesAvailable()) / (1024.0 * 1024.0 * 1024.0);
    double usedGB  = totalGB - freeGB;
    double usage   = (totalGB > 0) ? (100.0 * usedGB / totalGB) : 0.0;

    if (qAbs(usage - m_diskUsage) > 0.1) {
        m_diskUsage   = usage;
        m_diskUsedGB  = usedGB;
        m_diskTotalGB = totalGB;
        emit diskUsageChanged();
    }
}
