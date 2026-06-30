import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import com.asic.mobilerobot.viewmodels 1.0
import QtGraphicalEffects 1.12

Item {
    id: root

    ContainerBar {
        id: containerBar
        anchors.top: parent.top
        onBackClicked: ControlCenterViewViewModel.requestMainView()
    }

    Rectangle {
        anchors.fill: parent
        color: "#F5F4EF" // warm beige background
        z: -1

        // Background watermark
        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 120
            width: 1300
            height: 1105
            source: "images/UIT_logo.png"
            opacity: 0.15
        }

        // ── Title ──────────────────────────────────────────────────
        Text {
            id: titleText
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: containerBar.height + 150
            text: "ROBOT CONTROL CENTER"
            font.pixelSize: 96
            font.bold: true
            font.family: "Inter"
            color: "#1a1a1a"
        }

        // ── Mode Switch Pill ────────────────────────────────────────
        ModeSwitch {
            id: modeSwitchContainer
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: titleText.bottom
            anchors.topMargin: 100
        }

        // ── Two Panels Row ────────────────────────────────────────
        Row {
            id: panelsRow
            height: 380
            width: parent.width
            leftPadding: 80
            rightPadding: 80
            anchors.top: modeSwitchContainer.bottom
            anchors.topMargin: 100
            spacing: 40

            // ── Diagnostics Panel ───────────────────────────────────
            Rectangle {
                id: diagnosticsPanel
                width: (parent.width - 80 * 2 - 120) / 4
                height: 320
                radius: 32
                color: "#DAD8D8"
                opacity: 0.8
                Text {
                    id: diagnosticsTitle
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 20
                    text: "Diagnostics"
                    font.pixelSize: 48
                    font.bold: true
                    font.family: "Inter"
                    color: "#000000"
                }

                Column {
                    anchors.top: diagnosticsTitle.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: 30
                    anchors.leftMargin: 20
                    anchors.rightMargin: 20
                    spacing: 14

                    // CPU row
                    Column {
                        width: parent.width
                        spacing: 4
                        Text {
                            text: "CPU: <b>" + DiagnosticsViewViewModel.cpuUsage.toFixed(1) + "%</b>"
                            font.pixelSize: 36
                            font.family: "Inter"
                            color: "#333333"
                            textFormat: Text.RichText
                        }
                        Rectangle {
                            width: parent.width
                            height: 8
                            radius: 4
                            color: "#C5C3C3"
                            Rectangle {
                                width: Math.max(0, parent.width * DiagnosticsViewViewModel.cpuUsage / 100)
                                height: parent.height
                                radius: 4
                                color: DiagnosticsViewViewModel.cpuUsage > 80 ? "#f44336" : DiagnosticsViewViewModel.cpuUsage > 50 ? "#ff9800" : "#4caf50"
                                Behavior on width { NumberAnimation { duration: 300 } }
                            }
                        }
                    }

                    // RAM row
                    Column {
                        width: parent.width
                        spacing: 4
                        Text {
                            text: "RAM: <b>" + DiagnosticsViewViewModel.ramUsage.toFixed(1) + "%</b>  (" + (DiagnosticsViewViewModel.ramUsedMB / 1024).toFixed(1) + "/" + (DiagnosticsViewViewModel.ramTotalMB / 1024).toFixed(1) + " GB)"
                            font.pixelSize: 36
                            font.family: "Inter"
                            color: "#333333"
                            textFormat: Text.RichText
                        }
                        Rectangle {
                            width: parent.width
                            height: 8
                            radius: 4
                            color: "#C5C3C3"
                            Rectangle {
                                width: Math.max(0, parent.width * DiagnosticsViewViewModel.ramUsage / 100)
                                height: parent.height
                                radius: 4
                                color: DiagnosticsViewViewModel.ramUsage > 85 ? "#f44336" : DiagnosticsViewViewModel.ramUsage > 60 ? "#ff9800" : "#2196F3"
                                Behavior on width { NumberAnimation { duration: 300 } }
                            }
                        }
                    }

                    // FPS row
                    Text {
                        text: "FPS: <b>" + DiagnosticsViewViewModel.fps + "</b>"
                        font.pixelSize: 36
                        font.family: "Inter"
                        color: "#333333"
                        textFormat: Text.RichText
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        ControlCenterViewViewModel.requestDiagnosticsView()
                    }
                }
            }

            // ── Map Panel ───────────────────────────────────────────
            Rectangle {
                id: mapPanel
                width: (parent.width - 80 * 2 - 120) / 4
                height: 320
                radius: 32
                color: "#DAD8D8"
                opacity: 0.8

                Text {
                    id: mapTitle
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 20
                    text: "Map"
                    font.pixelSize: 48
                    font.family: "Inter"
                    font.bold: true
                    color: "#000000"
                }

                // Map image placeholder
                Rectangle {
                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: 8
                    width: parent.width - 40
                    height: parent.height - 120
                    radius: 8
                    color: "#D9D9D9"
                    border.color: "#2C2C2C"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "Map"
                        font.family: "Inter"
                        font.pixelSize: 30
                        color: "#000000"
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        ControlCenterViewViewModel.requestMapPanelView()
                    }
                }
            }

            // ── Pre-Flight Check Panel ──────────────────────────────
            Rectangle {
                id: preCheckPanel
                width: (parent.width - 80 * 2 - 120) / 4
                height: 320
                radius: 32
                color: "#DAD8D8"
                opacity: 0.8

                Text {
                    id: preCheckTitle
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 20
                    text: "System Check"
                    font.pixelSize: 48
                    font.bold: true
                    font.family: "Inter"
                    color: "#000000"
                }

                // Status badge — mirrors the last run result
                Rectangle {
                    id: statusBadge
                    anchors.top: preCheckTitle.bottom
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.topMargin: 10
                    width: badgeText.implicitWidth + 24
                    height: 36
                    radius: 18
                    color: {
                        var s = SysCheckViewModel.status
                        if (s === "ready")    return "#0d2e0d"
                        if (s === "warnings") return "#2e1f00"
                        if (s === "failed")   return "#2e0808"
                        return "#1a1a1a"
                    }
                    border.color: {
                        var s = SysCheckViewModel.status
                        if (s === "ready")    return "#4caf50"
                        if (s === "warnings") return "#ff9800"
                        if (s === "failed")   return "#f44336"
                        return "#555555"
                    }
                    border.width: 1
                    Text {
                        id: badgeText
                        anchors.centerIn: parent
                        text: {
                            var s = SysCheckViewModel.status
                            if (s === "ready")    return "✓ READY"
                            if (s === "warnings") return "⚠ WARNINGS"
                            if (s === "failed")   return "✕ FAILED"
                            if (s === "running")  return "● RUNNING"
                            return "NOT RUN"
                        }
                        font.pixelSize: 15
                        font.bold: true
                        font.family: "Inter"
                        color: {
                            var s = SysCheckViewModel.status
                            if (s === "ready")    return "#4caf50"
                            if (s === "warnings") return "#ff9800"
                            if (s === "failed")   return "#f44336"
                            return "#888888"
                        }
                    }
                }

                // Mini counters
                Column {
                    anchors.top: statusBadge.bottom
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.topMargin: 18
                    spacing: 8
                    Row {
                        spacing: 16
                        Text {
                            text: "✓ " + SysCheckViewModel.passCount + " PASS"
                            font.pixelSize: 26; font.family: "Inter"
                            color: "#2e7d32"; font.bold: true
                        }
                        Text {
                            text: "⚠ " + SysCheckViewModel.warnCount + " WARN"
                            font.pixelSize: 26; font.family: "Inter"
                            color: "#e65100"; font.bold: true
                        }
                    }
                    Text {
                        text: "✕ " + SysCheckViewModel.failCount + " FAIL"
                        font.pixelSize: 26; font.family: "Inter"
                        color: SysCheckViewModel.failCount > 0 ? "#b71c1c" : "#555555"
                        font.bold: true
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: ControlCenterViewViewModel.requestSysCheckView()
                }
            }

            // ── Navigation Panel ────────────────────────────────────
            Rectangle {
                id: navPanel
                width: (parent.width - 80 * 2 - 120) / 4
                height: 320
                radius: 32
                color: "#DAD8D8"
                opacity: 0.8

                // Title
                Text {
                    id: navPanelTitle
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 20
                    text: "Navigation"
                    font.pixelSize: 48
                    font.bold: true
                    font.family: "Inter"
                    color: "#000000"
                }

                // Nav status badge
                Rectangle {
                    id: navStatusBadge
                    anchors.top: navPanelTitle.bottom
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.topMargin: 10
                    width: navBadgeTxt.implicitWidth + 24
                    height: 36
                    radius: 18
                    color: SysCheckViewModel.isNavRunning ? "#0a1f2a" : "#1a1a1a"
                    border.color: SysCheckViewModel.isNavRunning ? "#2196F3" : "#555555"
                    border.width: 1
                    Text {
                        id: navBadgeTxt
                        anchors.centerIn: parent
                        text: SysCheckViewModel.isNavRunning ? "▶ RUNNING" : (SysCheckViewModel.navStatus === "stopped" ? "■ STOPPED" : "— IDLE")
                        font.pixelSize: 15
                        font.bold: true
                        font.family: "Inter"
                        color: SysCheckViewModel.isNavRunning ? "#2196F3" : "#888888"
                    }
                }

                // Active floor label
                Text {
                    id: navFloorLabel
                    anchors.top: navStatusBadge.bottom
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.topMargin: 10
                    text: "Floor: <b>" + DirectionViewViewModel.mapId + "</b>"
                    font.pixelSize: 26
                    font.family: "Inter"
                    color: "#333333"
                    textFormat: Text.RichText
                }

                // Map-switch reminder banner
                Rectangle {
                    id: navRestartBanner
                    anchors.top: navFloorLabel.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    anchors.topMargin: 8
                    height: 44
                    radius: 10
                    visible: SysCheckViewModel.navNeedsRestart
                    color: "#2e1f00"
                    border.color: "#ff9800"
                    border.width: 1

                    SequentialAnimation on opacity {
                        running: SysCheckViewModel.navNeedsRestart
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.55; duration: 700; easing.type: Easing.InOutSine }
                        NumberAnimation { to: 1.0;  duration: 700; easing.type: Easing.InOutSine }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "⚠  Map changed — restart nav"
                        font.pixelSize: 18
                        font.bold: true
                        font.family: "Inter"
                        color: "#ff9800"
                    }
                }

                // Start / Stop buttons
                Row {
                    id: navButtonRow
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 20
                    anchors.rightMargin: 20
                    anchors.bottomMargin: 20
                    spacing: 12

                    // Start button — disabled while nav is running
                    Rectangle {
                        id: startNavBtn
                        width: (parent.width - 12) / 2
                        height: 56
                        radius: 14
                        // Highlight with orange border when map changed and nav needs restart
                        color: startNavMa.containsMouse ? "#1565C0" : "#1976D2"
                        opacity: SysCheckViewModel.isNavRunning ? 0.38 : 1.0
                        border.color: SysCheckViewModel.navNeedsRestart ? "#ff9800" : "transparent"
                        border.width: SysCheckViewModel.navNeedsRestart ? 2 : 0
                        Behavior on color { ColorAnimation { duration: 150 } }

                        Text {
                            anchors.centerIn: parent
                            text: "▶  Start"
                            font.pixelSize: 22
                            font.bold: true
                            font.family: "Inter"
                            color: "#FFFFFF"
                        }

                        MouseArea {
                            id: startNavMa
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: SysCheckViewModel.isNavRunning ? Qt.ArrowCursor : Qt.PointingHandCursor
                            enabled: !SysCheckViewModel.isNavRunning
                            onClicked: ControlCenterViewViewModel.requestStartNavigation()
                        }
                    }

                    // Stop button — disabled while nav is not running
                    Rectangle {
                        id: stopNavBtn
                        width: (parent.width - 12) / 2
                        height: 56
                        radius: 14
                        color: stopNavMa.containsMouse ? "#B71C1C" : "#C62828"
                        opacity: SysCheckViewModel.isNavRunning ? 1.0 : 0.38
                        Behavior on color { ColorAnimation { duration: 150 } }

                        Text {
                            anchors.centerIn: parent
                            text: "■  Stop"
                            font.pixelSize: 22
                            font.bold: true
                            font.family: "Inter"
                            color: "#FFFFFF"
                        }

                        MouseArea {
                            id: stopNavMa
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: SysCheckViewModel.isNavRunning ? Qt.PointingHandCursor : Qt.ArrowCursor
                            enabled: SysCheckViewModel.isNavRunning
                            onClicked: ControlCenterViewViewModel.requestStopNavigation()
                        }
                    }
                }
            }
        }
    }

    // ── System Check overlay (ControlCenter manual re-run) ─────────────
    Loader {
        id: sysCheckLoader
        anchors.fill: parent
        active: SysCheckViewModel.isVisible
        source: active ? "SysCheckView.qml" : ""
        z: 200
    }
}
