import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import com.asic.mobilerobot.viewmodels 1.0
import QtGraphicalEffects 1.12

Item {
    id: root

    // Shorthand alias
    property var vm: DiagnosticsViewViewModel

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onBackClicked: DiagnosticsViewViewModel.requestControlCenterView()
    }

    // ── Background (matches app template) ──
    Rectangle {
        anchors.fill: parent
        color: "#F5F4EF"
        z: -1

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 120
            width: 1300
            height: 1105
            source: "images/UIT_logo.png"
            opacity: 0.15
        }
    }

    // ── Scrollable Content Area ──
    Flickable {
        id: contentArea
        anchors.top: containerBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 40
        anchors.topMargin: 20
        contentHeight: mainLayout.height + 40
        clip: true

        Column {
            id: mainLayout
            width: parent.width
            spacing: 30

            // ── Title ──
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "SYSTEM DIAGNOSTICS"
                font.pixelSize: 64
                font.bold: true
                font.family: "Inter"
                color: "#1a1a1a"
            }

            // ══════════════════════════════════════════════════════
            //  ROW 1: CPU  ·  RAM  ·  FPS
            // ══════════════════════════════════════════════════════
            Row {
                width: parent.width
                spacing: 24

                // ── CPU Card ──
                DiagnosticCard {
                    width: (parent.width - 48) / 3
                    height: 400
                    title: "CPU USAGE"
                    icon: "⚙"
                    value: vm.cpuUsage.toFixed(1) + "%"
                    accentColor: getCpuColor(vm.cpuUsage)
                    percentage: vm.cpuUsage
                    historyData: vm.cpuHistory
                    graphColor: getCpuColor(vm.cpuUsage)
                }

                // ── RAM Card ──
                DiagnosticCard {
                    width: (parent.width - 48) / 3
                    height: 400
                    title: "RAM USAGE"
                    icon: "🧠"
                    value: vm.ramUsage.toFixed(1) + "%"
                    subtitle: (vm.ramUsedMB / 1024).toFixed(1) + " / " + (vm.ramTotalMB / 1024).toFixed(1) + " GB"
                    accentColor: getRamColor(vm.ramUsage)
                    percentage: vm.ramUsage
                    historyData: vm.ramHistory
                    graphColor: getRamColor(vm.ramUsage)
                }

                // ── FPS Card ──
                DiagnosticCard {
                    width: (parent.width - 48) / 3
                    height: 400
                    title: "FRAME RATE"
                    icon: "🎞"
                    value: vm.fps + " FPS"
                    accentColor: getFpsColor(vm.fps)
                    percentage: Math.min(100, vm.fps * 100 / 60)
                    historyData: vm.fpsHistory
                    graphColor: getFpsColor(vm.fps)
                    maxGraphValue: 80
                }
            }

            // ══════════════════════════════════════════════════════
            //  ROW 2: Network  ·  Disk
            // ══════════════════════════════════════════════════════
            Row {
                width: parent.width
                spacing: 24

                // ── Network Card ──
                Rectangle {
                    width: (parent.width - 24) / 2
                    height: 280
                    radius: 32
                    color: "#DAD8D8"
                    opacity: 0.8

                    Column {
                        anchors.fill: parent
                        anchors.margins: 28
                        spacing: 16

                        Row {
                            spacing: 12
                            Text { text: "📡"; font.pixelSize: 28; anchors.verticalCenter: parent.verticalCenter }
                            Text {
                                text: "NETWORK"
                                font.pixelSize: 22
                                font.bold: true
                                font.family: "Inter"
                                font.letterSpacing: 2
                                color: "#1a1a1a"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        // Speed indicators
                        Row {
                            spacing: 40
                            width: parent.width

                            Column {
                                spacing: 6
                                Text {
                                    text: "▼ DOWNLOAD"
                                    font.pixelSize: 16
                                    font.family: "Inter"
                                    color: "#2E7DD1"
                                    font.letterSpacing: 1
                                }
                                Text {
                                    text: formatNetSpeed(vm.netRxKBps)
                                    font.pixelSize: 38
                                    font.bold: true
                                    font.family: "Inter"
                                    color: "#1a1a1a"
                                }
                            }

                            Column {
                                spacing: 6
                                Text {
                                    text: "▲ UPLOAD"
                                    font.pixelSize: 16
                                    font.family: "Inter"
                                    color: "#D14B2E"
                                    font.letterSpacing: 1
                                }
                                Text {
                                    text: formatNetSpeed(vm.netTxKBps)
                                    font.pixelSize: 38
                                    font.bold: true
                                    font.family: "Inter"
                                    color: "#1a1a1a"
                                }
                            }
                        }

                        // Separator
                        Rectangle {
                            width: parent.width
                            height: 1
                            color: "#B0AEAE"
                        }

                        // Total data
                        Row {
                            spacing: 40

                            Text {
                                text: "Total ▼  " + vm.totalRxMB.toFixed(1) + " MB"
                                font.pixelSize: 18
                                font.family: "Inter"
                                color: "#555555"
                            }
                            Text {
                                text: "Total ▲  " + vm.totalTxMB.toFixed(1) + " MB"
                                font.pixelSize: 18
                                font.family: "Inter"
                                color: "#555555"
                            }
                        }
                    }
                }

                // ── Disk Card ──
                Rectangle {
                    width: (parent.width - 24) / 2
                    height: 280
                    radius: 32
                    color: "#DAD8D8"
                    opacity: 0.8

                    Column {
                        anchors.fill: parent
                        anchors.margins: 28
                        spacing: 16

                        Row {
                            spacing: 12
                            Text { text: "💾"; font.pixelSize: 28; anchors.verticalCenter: parent.verticalCenter }
                            Text {
                                text: "DISK STORAGE"
                                font.pixelSize: 22
                                font.bold: true
                                font.family: "Inter"
                                font.letterSpacing: 2
                                color: "#1a1a1a"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        // Disk usage ring + info
                        Row {
                            spacing: 40
                            width: parent.width

                            // Circular gauge
                            Item {
                                width: 140; height: 140

                                Canvas {
                                    id: diskCanvas
                                    anchors.fill: parent
                                    property double usage: vm.diskUsage

                                    onPaint: {
                                        var ctx = getContext("2d");
                                        ctx.reset();

                                        var cx = width / 2;
                                        var cy = height / 2;
                                        var r = 58;
                                        var lineW = 12;
                                        var startAngle = -Math.PI / 2;
                                        var endAngle = startAngle + (2 * Math.PI * usage / 100);

                                        // Background arc
                                        ctx.beginPath();
                                        ctx.arc(cx, cy, r, 0, 2 * Math.PI);
                                        ctx.lineWidth = lineW;
                                        ctx.strokeStyle = "#C5C3C3";
                                        ctx.stroke();

                                        // Usage arc
                                        ctx.beginPath();
                                        ctx.arc(cx, cy, r, startAngle, endAngle);
                                        ctx.lineWidth = lineW;
                                        ctx.lineCap = "round";
                                        ctx.strokeStyle = getDiskColor(usage);
                                        ctx.stroke();
                                    }

                                    onUsageChanged: requestPaint()
                                }

                                // Center text
                                Column {
                                    anchors.centerIn: parent
                                    Text {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: vm.diskUsage.toFixed(0) + "%"
                                        font.pixelSize: 28
                                        font.bold: true
                                        font.family: "Inter"
                                        color: "#1a1a1a"
                                    }
                                    Text {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: "USED"
                                        font.pixelSize: 12
                                        font.family: "Inter"
                                        color: "#777777"
                                        font.letterSpacing: 1
                                    }
                                }
                            }

                            Column {
                                spacing: 8
                                anchors.verticalCenter: parent.verticalCenter

                                Text {
                                    text: vm.diskUsedGB.toFixed(1) + " GB used"
                                    font.pixelSize: 24
                                    font.family: "Inter"
                                    color: "#1a1a1a"
                                }
                                Text {
                                    text: vm.diskTotalGB.toFixed(1) + " GB total"
                                    font.pixelSize: 20
                                    font.family: "Inter"
                                    color: "#555555"
                                }
                                Text {
                                    text: (vm.diskTotalGB - vm.diskUsedGB).toFixed(1) + " GB free"
                                    font.pixelSize: 20
                                    font.family: "Inter"
                                    color: "#4caf50"
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // ══════════════════════════════════════════════════════════════
    //  Helper functions
    // ══════════════════════════════════════════════════════════════

    function getCpuColor(usage) {
        if (usage > 80) return "#f44336"
        if (usage > 50) return "#ff9800"
        return "#4caf50"
    }

    function getRamColor(usage) {
        if (usage > 85) return "#f44336"
        if (usage > 60) return "#ff9800"
        return "#2196F3"
    }

    function getFpsColor(fps) {
        if (fps < 15) return "#f44336"
        if (fps < 30) return "#ff9800"
        return "#4caf50"
    }

    function getDiskColor(usage) {
        if (usage > 90) return "#f44336"
        if (usage > 70) return "#ff9800"
        return "#2196F3"
    }

    function formatNetSpeed(kbps) {
        if (kbps > 1024)
            return (kbps / 1024).toFixed(1) + " MB/s"
        return kbps.toFixed(0) + " KB/s"
    }
}
