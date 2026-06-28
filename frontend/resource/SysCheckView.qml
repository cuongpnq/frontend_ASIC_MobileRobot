import QtQuick 2.12
import QtQuick.Controls 2.12
import com.asic.mobilerobot.viewmodels 1.0

Item {
    id: root
    anchors.fill: parent

    Component.onCompleted: {
        if (SysCheckViewModel.isBootMode && SysCheckViewModel.status === "idle")
            SysCheckViewModel.runSysCheck("a1", true, false)
    }

    // ══════════════════════════════════════════════════════════════
    //  BOOT SPLASH (simple, centered)
    // ══════════════════════════════════════════════════════════════
    Item {
        anchors.fill: parent
        visible: SysCheckViewModel.isBootMode

        // Background (matching light theme style)
        Rectangle { anchors.fill: parent; color: "#F5F4EF" }

        // ── Center content ──────────────────────────────────────
        Column {
            anchors.centerIn: parent
            spacing: 40

            // Logo
            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "images/UIT_logo.png"
                width: 220; height: 220
                fillMode: Image.PreserveAspectFit
                opacity: 0.9
            }

            // Title
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "ASIC Mobile Robot"
                font.pixelSize: 72
                font.bold: true
                font.family: "Inter"
                color: "#1a1a1a"
                font.letterSpacing: 2
            }

            // Subtitle
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Initializing system…"
                font.pixelSize: 26
                font.family: "Inter"
                color: "#555555"
                font.letterSpacing: 1
            }
        }

        // ── Spinner at bottom ───────────────────────────────────
        Item {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 120
            width: 80; height: 80

            Canvas {
                id: bootSpinner
                anchors.fill: parent
                property real angle: 0
                onPaint: {
                    var ctx = getContext("2d"); ctx.reset()
                    var cx = width/2, cy = height/2, r = 30
                    ctx.beginPath(); ctx.arc(cx,cy,r,0,2*Math.PI)
                    ctx.lineWidth=5; ctx.strokeStyle="#DAD8D8"; ctx.stroke()
                    ctx.beginPath()
                    ctx.arc(cx,cy,r, angle*Math.PI/180, (angle+240)*Math.PI/180)
                    ctx.lineWidth=5; ctx.lineCap="round"
                    ctx.strokeStyle="#2196F3"; ctx.stroke()
                }
                NumberAnimation on angle {
                    from:0; to:360; duration:900
                    loops:Animation.Infinite
                    running: SysCheckViewModel.isBootMode
                }
                onAngleChanged: requestPaint()
            }

            // Replace spinner with verdict icon when done
            Canvas {
                id: bootVerdict
                anchors.fill: parent
                visible: !SysCheckViewModel.isRunning && SysCheckViewModel.status !== "idle"
                property real p: 0
                onPaint: {
                    var ctx = getContext("2d"); ctx.reset()
                    var cx=width/2, cy=height/2, r=30
                    var failed = SysCheckViewModel.status === "failed"
                    var col = failed ? "#f44336" : "#4caf50"
                    ctx.beginPath(); ctx.arc(cx,cy,r,0,2*Math.PI)
                    ctx.lineWidth=5; ctx.strokeStyle=col; ctx.stroke()
                    ctx.lineWidth=5; ctx.lineCap="round"; ctx.strokeStyle=col
                    if (!failed) {
                        var p1=Math.min(1,p/0.4), p2=Math.max(0,(p-0.4)/0.6)
                        var x0=cx-14,y0=cy+2,x1=cx-3,y1=cy+13,x2=cx+14,y2=cy-9
                        ctx.beginPath(); ctx.moveTo(x0,y0); ctx.lineTo(x0+(x1-x0)*p1,y0+(y1-y0)*p1)
                        if(p1>=1&&p2>0){ctx.moveTo(x1,y1);ctx.lineTo(x1+(x2-x1)*p2,y1+(y2-y1)*p2)}
                        ctx.stroke()
                    } else {
                        var q1=Math.min(1,p/0.5),q2=Math.max(0,(p-0.5)/0.5),m=14
                        ctx.beginPath();ctx.moveTo(cx-m,cy-m);ctx.lineTo(cx-m+2*m*q1,cy-m+2*m*q1);ctx.stroke()
                        if(q2>0){ctx.beginPath();ctx.moveTo(cx+m,cy-m);ctx.lineTo(cx+m-2*m*q2,cy-m+2*m*q2);ctx.stroke()}
                    }
                }
                onPChanged: requestPaint()
                NumberAnimation on p { id:va; from:0;to:1;duration:600;easing.type:Easing.OutCubic }
                Connections {
                    target: SysCheckViewModel
                    onStatusChanged: { var s=SysCheckViewModel.status; if(s!=="idle"&&s!=="running"){bootVerdict.p=0;va.restart()} }
                }
            }
        }

        // ── Status text below spinner ───────────────────────────
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 70
            text: {
                var s = SysCheckViewModel.status
                if (s === "running")  return "Running checks… (" + SysCheckViewModel.passCount + " pass / " + SysCheckViewModel.failCount + " fail)"
                if (s === "ready")    return "✓  All checks passed"
                if (s === "warnings") return "⚠  Checks done with warnings"
                if (s === "failed")   return "✕  " + SysCheckViewModel.failCount + " checks failed"
                return ""
            }
            font.pixelSize: 22; font.family: "Inter"
            color: {
                var s = SysCheckViewModel.status
                if (s === "ready") return "#4caf50"
                if (s === "warnings") return "#ff9800"
                if (s === "failed") return "#f44336"
                return "#555555"
            }
        }

        // ── Navigation started indicator ────────────────────────
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 42
            visible: SysCheckViewModel.isNavRunning
            text: "▶  Navigation started — FLOOR: " + SysCheckViewModel.navFloor
            font.pixelSize: 20; font.family: "Inter"
            color: "#4caf50"
        }

        // ── Continue button (bottom-center, active when done) ───
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            width: contLbl.implicitWidth + 60; height: 64; radius: 32
            visible: !SysCheckViewModel.isRunning && SysCheckViewModel.status !== "idle"
            color: SysCheckViewModel.status === "failed"
                   ? (contMa2.containsMouse ? "#d32f2f" : "#f44336")
                   : (contMa2.containsMouse ? "#388e3c" : "#4caf50")
            border.color: SysCheckViewModel.status === "failed" ? "#f44336" : "#4caf50"
            border.width: 1
            Behavior on color { ColorAnimation { duration: 150 } }
            Text {
                id: contLbl; anchors.centerIn: parent
                text: SysCheckViewModel.status === "failed" ? "Continue anyway  →" : "Continue  →"
                font.pixelSize: 26; font.bold: true; font.family: "Inter"; color: "#FFFFFF"
            }
            MouseArea { id: contMa2; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                onClicked: SysCheckViewModel.dismissStartupCheck() }
        }

        // ── Developer skip (bottom-right corner) ───────────────
        Text {
            anchors.right: parent.right; anchors.bottom: parent.bottom
            anchors.rightMargin: 40; anchors.bottomMargin: 30
            text: "Developer Access"
            font.pixelSize: 18; font.family: "Inter"; color: "#777777"
            MouseArea {
                anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                onClicked: {
                    UserManager.selectTargetRole(UserManager.Developer)
                    devPasswordField.text = ""
                    UserManager.resetLoginResult()
                    devDialog.visible = true
                }
            }
        }

        // ── Developer password dialog ───────────────────────────
        Item {
            id: devDialog
            anchors.fill: parent
            visible: false
            z: 10

            Rectangle { anchors.fill: parent; color: "#80000000"
                MouseArea { anchors.fill: parent; onClicked: { devPasswordField.focus = false; Qt.inputMethod.hide(); devDialog.visible = false } } }

            Rectangle {
                id: dialogBox
                anchors.horizontalCenter: parent.horizontalCenter
                width: 640; height: 340; radius: 24
                color: "#FFFFFF"; border.color: "#DAD8D8"; border.width: 1

                y: {
                    var kbH = Qt.inputMethod.visible ? Qt.inputMethod.keyboardRectangle.height : 0
                    return Math.max(20, (parent.height - kbH - height) / 2)
                }
                Behavior on y { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }

                Column {
                    anchors.centerIn: parent; spacing: 28; width: parent.width - 80

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Developer Access"
                        font.pixelSize: 36; font.bold: true; font.family: "Inter"; color: "#1a1a1a"
                    }

                    Rectangle {
                        width: parent.width; height: 72; radius: 14
                        color: "#F5F4EF"
                        border.color: devPasswordField.activeFocus ? "#2196F3" : "#DAD8D8"
                        border.width: 2
                        TextField {
                            id: devPasswordField
                            anchors.fill: parent; anchors.margins: 16
                            placeholderText: "Enter Developer password"
                            echoMode: TextInput.Password
                            font.pixelSize: 28; font.family: "Inter"; color: "#1a1a1a"
                            background: Rectangle { color: "transparent" }
                            onAccepted: UserManager.attemptLogin(devPasswordField.text)
                            onTextChanged: if (UserManager.loginResult === UserManager.Failed) UserManager.resetLoginResult()
                        }
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Incorrect password"
                        font.pixelSize: 22; font.family: "Inter"; color: "#f44336"
                        visible: UserManager.loginResult === UserManager.Failed
                    }

                    Connections {
                        target: UserManager
                        onLoginResultChanged: {
                            if (UserManager.loginResult === UserManager.Success) {
                                devPasswordField.focus = false
                                Qt.inputMethod.hide()
                                devDialog.visible = false
                                SysCheckViewModel.dismissStartupCheck()
                            } else if (UserManager.loginResult === UserManager.Failed) {
                                devPasswordField.text = ""
                                devPasswordField.forceActiveFocus()
                            }
                        }
                    }

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter; spacing: 20
                        Rectangle {
                            width: 160; height: 60; radius: 14; color: cancelMa.pressed ? "#E1DFD9" : "#ECEAE4"
                            border.color: "#DAD8D8"; border.width: 1
                            Text { anchors.centerIn: parent; text: "Cancel"; font.pixelSize: 24; font.family: "Inter"; color: "#555555" }
                            MouseArea { id: cancelMa; anchors.fill: parent; onClicked: { devPasswordField.focus = false; Qt.inputMethod.hide(); devDialog.visible = false } }
                        }
                        Rectangle {
                            width: 200; height: 60; radius: 14; color: loginMa.pressed ? "#1976D2" : "#2196F3"
                            Text { anchors.centerIn: parent; text: "Login"; font.pixelSize: 24; font.bold: true; font.family: "Inter"; color: "#FFFFFF" }
                            MouseArea { id: loginMa; anchors.fill: parent; onClicked: UserManager.attemptLogin(devPasswordField.text) }
                        }
                    }
                }
            }
        }
    }

    // ══════════════════════════════════════════════════════════════
    //  MANUAL MODE (ControlCenter re-run) — detail panels
    // ══════════════════════════════════════════════════════════════
    Item {
        anchors.fill: parent
        visible: !SysCheckViewModel.isBootMode

        Rectangle { anchors.fill: parent; color: "#0D0D0D" }

        // Header
        Rectangle {
            id: hdr; anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right
            height: 100; color: "#141414"

            Rectangle {
                anchors.left: parent.left; anchors.leftMargin: 32; anchors.verticalCenter: parent.verticalCenter
                width: 120; height: 60; radius: 12; color: bmA.containsMouse ? "#2a2a2a" : "#1e1e1e"
                border.color: "#333"; border.width: 1
                Behavior on color { ColorAnimation { duration: 150 } }
                Row { anchors.centerIn: parent; spacing: 8
                    Text { text: "←"; font.pixelSize: 26; color: "#e0e0e0"; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "Back"; font.pixelSize: 20; font.family: "Inter"; color: "#e0e0e0"; anchors.verticalCenter: parent.verticalCenter }
                }
                MouseArea { id: bmA; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: SysCheckViewModel.close() }
            }

            Column {
                anchors.centerIn: parent; spacing: 2
                Text { anchors.horizontalCenter: parent.horizontalCenter; text: "SYSTEM CHECK"; font.pixelSize: 36; font.bold: true; font.family: "Inter"; font.letterSpacing: 4; color: "#fff" }
                Text { anchors.horizontalCenter: parent.horizontalCenter; text: "Mobile Robot ROS 2 Environment Verification"; font.pixelSize: 16; font.family: "Inter"; color: "#888"; font.letterSpacing: 1 }
            }

            Rectangle {
                anchors.right: parent.right; anchors.rightMargin: 32; anchors.verticalCenter: parent.verticalCenter
                width: 160; height: 60; radius: 12
                enabled: !SysCheckViewModel.isRunning; opacity: enabled ? 1.0 : 0.4
                color: rrA.containsMouse ? "#2e7d32" : "#1b5e20"; border.color: "#4caf50"; border.width: 1
                Behavior on color { ColorAnimation { duration: 150 } }
                Text { anchors.centerIn: parent; text: SysCheckViewModel.isRunning ? "⏳  Running…" : "▶  Re-run"; font.pixelSize: 18; font.family: "Inter"; color: "#e0e0e0" }
                MouseArea { id: rrA; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: SysCheckViewModel.runSysCheck("a1",true,false) }
            }
        }

        // Two-column content
        Row {
            anchors.top: hdr.bottom; anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: foot.top
            anchors.margins: 24; anchors.topMargin: 16; spacing: 20

            // Left: results list
            Rectangle {
                id: lp; width: parent.width * 0.52; height: parent.height
                radius: 16; color: "#111"; border.color: "#222"; border.width: 1

                Rectangle { id: lh; anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right; height: 52; radius: 16; color: "#1a1a1a"
                    Rectangle { anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.right: parent.right; height: 16; color: "#1a1a1a" }
                    Row { anchors.left: parent.left; anchors.leftMargin: 20; anchors.verticalCenter: parent.verticalCenter; spacing: 10
                        Text { text: "📋"; font.pixelSize: 20 }
                        Text { text: "CHECK RESULTS"; font.pixelSize: 16; font.bold: true; font.family: "Inter"; font.letterSpacing: 2; color: "#aaa" }
                    }
                    Text { anchors.right: parent.right; anchors.rightMargin: 20; anchors.verticalCenter: parent.verticalCenter; text: SysCheckViewModel.results.length+" items"; font.pixelSize: 14; font.family: "Inter"; color: "#555" }
                }

                // Spinner
                Item { anchors.top: lh.bottom; anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                    visible: SysCheckViewModel.isRunning && SysCheckViewModel.results.length === 0
                    Column { anchors.centerIn: parent; spacing: 24
                        Canvas { id: bspin; width: 120; height: 120; anchors.horizontalCenter: parent.horizontalCenter; property real angle: 0
                            onPaint: { var ctx=getContext("2d");ctx.reset();var cx=width/2,cy=height/2,r=48;ctx.beginPath();ctx.arc(cx,cy,r,0,2*Math.PI);ctx.lineWidth=8;ctx.strokeStyle="#1f1f1f";ctx.stroke();ctx.beginPath();ctx.arc(cx,cy,r,angle*Math.PI/180,(angle+240)*Math.PI/180);ctx.lineWidth=8;ctx.lineCap="round";ctx.strokeStyle="#4caf50";ctx.stroke() }
                            NumberAnimation on angle { from:0;to:360;duration:900;loops:Animation.Infinite;running:SysCheckViewModel.isRunning }
                            onAngleChanged: requestPaint()
                        }
                        Text { anchors.horizontalCenter: parent.horizontalCenter; text: "Running system checks…"; font.pixelSize: 22; font.family: "Inter"; color: "#666" }
                    }
                }

                // Verdict
                Item { id: va2; anchors.top: lh.bottom; anchors.left: parent.left; anchors.right: parent.right; height: 180
                    visible: !SysCheckViewModel.isRunning && SysCheckViewModel.status !== "idle" && SysCheckViewModel.results.length > 0
                    Row { anchors.centerIn: parent; spacing: 20
                        Canvas { id: vi; width: 80; height: 80; anchors.verticalCenter: parent.verticalCenter; property real dp: 0
                            onPaint: { var ctx=getContext("2d");ctx.reset();var cx=width/2,cy=height/2,r=32,failed=SysCheckViewModel.status==="failed",col=failed?"#f44336":(SysCheckViewModel.status==="warnings"?"#ff9800":"#4caf50");ctx.beginPath();ctx.arc(cx,cy,r,0,2*Math.PI);ctx.lineWidth=5;ctx.strokeStyle=col;ctx.stroke();ctx.lineWidth=5;ctx.lineCap="round";ctx.lineJoin="round";ctx.strokeStyle=col;var p=dp;if(!failed){var p1=Math.min(1,p/0.4),p2=Math.max(0,(p-0.4)/0.6),x0=cx-16,y0=cy+2,x1=cx-4,y1=cy+14,x2=cx+16,y2=cy-10;ctx.beginPath();ctx.moveTo(x0,y0);ctx.lineTo(x0+(x1-x0)*p1,y0+(y1-y0)*p1);if(p1>=1&&p2>0){ctx.moveTo(x1,y1);ctx.lineTo(x1+(x2-x1)*p2,y1+(y2-y1)*p2)}ctx.stroke()}else{var q1=Math.min(1,p/0.5),q2=Math.max(0,(p-0.5)/0.5),m=18;ctx.beginPath();ctx.moveTo(cx-m,cy-m);ctx.lineTo(cx-m+2*m*q1,cy-m+2*m*q1);ctx.stroke();if(q2>0){ctx.beginPath();ctx.moveTo(cx+m,cy-m);ctx.lineTo(cx+m-2*m*q2,cy-m+2*m*q2);ctx.stroke()}} }
                            onDpChanged: requestPaint()
                            NumberAnimation on dp { id: va2anim; from:0;to:1;duration:600;easing.type:Easing.OutCubic;running:va2.visible }
                            Connections { target: SysCheckViewModel; onStatusChanged: { var s=SysCheckViewModel.status;if(s!=="idle"&&s!=="running"){vi.dp=0;va2anim.restart()} } }
                        }
                        Column { spacing: 4; anchors.verticalCenter: parent.verticalCenter
                            Text {
                                text: {
                                    var s = SysCheckViewModel.status
                                    if (s === "ready")    return "READY"
                                    if (s === "warnings") return "READY WITH WARNINGS"
                                    if (s === "failed")   return "NOT READY"
                                    return ""
                                }
                                font.pixelSize: 28; font.bold: true; font.family: "Inter"; font.letterSpacing: 2
                                color: {
                                    var s = SysCheckViewModel.status
                                    if (s === "ready")    return "#4caf50"
                                    if (s === "warnings") return "#ff9800"
                                    return "#f44336"
                                }
                            }
                            Text {
                                text: {
                                    var s = SysCheckViewModel.status
                                    if (s === "ready")   return "Safe to run autonomous navigation"
                                    if (s === "warnings") return "Review WARN items"
                                    if (s === "failed")  return "Fix FAIL items before proceeding"
                                    return ""
                                }
                                font.pixelSize: 16; font.family: "Inter"; color: "#777"
                            }
                        }
                    }
                }

                ListView {
                    anchors.top: va2.visible ? va2.bottom : lh.bottom; anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                    anchors.margins: 12; clip: true; model: SysCheckViewModel.results; spacing: 6
                    onCountChanged: positionViewAtEnd()
                    delegate: Rectangle {
                        width: parent ? parent.width : 0; height: 46; radius: 8
                        color:{var t=modelData["tag"];if(t==="PASS")return"#0a1f0a";if(t==="WARN")return"#1f1500";if(t==="FAIL")return"#1f0505";return"#111"}
                        Row { anchors.left: parent.left; anchors.leftMargin: 12; anchors.verticalCenter: parent.verticalCenter; spacing: 10
                            Rectangle { width: tl.implicitWidth+16; height: 28; radius: 6; color: Qt.rgba(parseInt(modelData["color"].substring(1,3),16)/255,parseInt(modelData["color"].substring(3,5),16)/255,parseInt(modelData["color"].substring(5,7),16)/255,0.18); border.color: modelData["color"]; border.width: 1
                                Text { id: tl; anchors.centerIn: parent; text: modelData["tag"]; font.pixelSize: 12; font.bold: true; font.family: "Inter"; font.letterSpacing: 1; color: modelData["color"] }
                            }
                            Text { width: (parent.parent ? parent.parent.width : 400)-160; anchors.verticalCenter: parent.verticalCenter; text: modelData["message"]; font.pixelSize: 16; font.family: "Inter"; color: "#ccc"; elide: Text.ElideRight }
                        }
                    }
                }
            }

            // Right: raw log
            Rectangle {
                width: parent.width - lp.width - parent.spacing; height: parent.height
                radius: 16; color: "#0a0a0a"; border.color: "#1a1a1a"; border.width: 1
                Rectangle { id: rh; anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right; height: 52; radius: 16; color: "#111"
                    Rectangle { anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.right: parent.right; height: 16; color: "#111" }
                    Row { anchors.left: parent.left; anchors.leftMargin: 20; anchors.verticalCenter: parent.verticalCenter; spacing: 10
                        Text { text: "🖥"; font.pixelSize: 20 }
                        Text { text: "TERMINAL OUTPUT"; font.pixelSize: 16; font.bold: true; font.family: "Inter"; font.letterSpacing: 2; color: "#aaa" }
                    }
                    Rectangle { anchors.right: parent.right; anchors.rightMargin: 20; anchors.verticalCenter: parent.verticalCenter; width: 10; height: 10; radius: 5; color: "#4caf50"; visible: SysCheckViewModel.isRunning
                        SequentialAnimation on opacity { running: SysCheckViewModel.isRunning; loops: Animation.Infinite; NumberAnimation { to: 0.1; duration: 500 } NumberAnimation { to: 1.0; duration: 500 } }
                    }
                }
                Flickable {
                    anchors.top: rh.bottom; anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                    anchors.margins: 12; clip: true; contentHeight: rawTxt.implicitHeight; contentWidth: width
                    onContentHeightChanged: { if (SysCheckViewModel.isRunning) contentY = Math.max(0, contentHeight - height) }
                    Text { id: rawTxt; width: parent.width; text: SysCheckViewModel.rawLog; font.pixelSize: 14; font.family: "Monospace"; color: "#b0b0b0"; wrapMode: Text.WrapAtWordBoundaryOrAnywhere; lineHeight: 1.4 }
                }
            }
        }

        // Footer summary
        Rectangle {
            id: foot; anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.right: parent.right
            height: 80; color: "#0f0f0f"
            Row { anchors.centerIn: parent; spacing: 60
                Row {
                    spacing: 10; anchors.verticalCenter: parent.verticalCenter
                    Rectangle { width:12; height:12; radius:6; color:"#4caf50"; anchors.verticalCenter:parent.verticalCenter }
                    Text { text:"PASS  "+SysCheckViewModel.passCount; font.pixelSize:22; font.bold:true; font.family:"Inter"; color:"#4caf50"; anchors.verticalCenter:parent.verticalCenter }
                }
                Rectangle { width:1; height:40; color:"#2a2a2a"; anchors.verticalCenter:parent.verticalCenter }
                Row {
                    spacing: 10; anchors.verticalCenter: parent.verticalCenter
                    Rectangle { width:12; height:12; radius:6; color:"#ff9800"; anchors.verticalCenter:parent.verticalCenter }
                    Text { text:"WARN  "+SysCheckViewModel.warnCount; font.pixelSize:22; font.bold:true; font.family:"Inter"; color:"#ff9800"; anchors.verticalCenter:parent.verticalCenter }
                }
                Rectangle { width:1; height:40; color:"#2a2a2a"; anchors.verticalCenter:parent.verticalCenter }
                Row {
                    spacing: 10; anchors.verticalCenter: parent.verticalCenter
                    Rectangle { width:12; height:12; radius:6; color:"#f44336"; anchors.verticalCenter:parent.verticalCenter }
                    Text { text:"FAIL  "+SysCheckViewModel.failCount; font.pixelSize:22; font.bold:true; font.family:"Inter"; color:"#f44336"; anchors.verticalCenter:parent.verticalCenter }
                }
            }
        }
    }
}
