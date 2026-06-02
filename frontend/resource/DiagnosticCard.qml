import QtQuick 2.12
import QtGraphicalEffects 1.12

Rectangle {
    id: card
    radius: 32
    color: "#DAD8D8"
    opacity: 0.8

    property string title: ""
    property string icon: ""
    property string value: ""
    property string subtitle: ""
    property color accentColor: "#4caf50"
    property double percentage: 0
    property var historyData: []
    property color graphColor: "#4caf50"
    property double maxGraphValue: 100

    Column {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 12

        // Header
        Row {
            spacing: 10
            Text {
                text: card.icon
                font.pixelSize: 24
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                text: card.title
                font.pixelSize: 18
                font.bold: true
                font.family: "Inter"
                font.letterSpacing: 2
                color: "#1a1a1a"
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // Value
        Text {
            text: card.value
            font.pixelSize: 48
            font.bold: true
            font.family: "Inter"
            color: card.accentColor
        }

        // Subtitle
        Text {
            text: card.subtitle
            font.pixelSize: 16
            font.family: "Inter"
            color: "#555555"
            visible: card.subtitle.length > 0
        }

        // Progress bar
        Item {
            width: parent.width
            height: 10

            Rectangle {
                width: parent.width
                height: 10
                radius: 5
                color: "#C5C3C3"
            }

            Rectangle {
                width: Math.max(0, Math.min(parent.width, parent.width * card.percentage / 100))
                height: 10
                radius: 5
                color: card.accentColor

                Behavior on width {
                    NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                }
            }
        }

        // Spacer
        Item { width: 1; height: 4 }

        // Mini sparkline graph
        Canvas {
            id: sparkline
            width: parent.width
            height: 100

            property var data: card.historyData
            property color lineColor: card.graphColor
            property double maxVal: card.maxGraphValue

            onPaint: {
                var ctx = getContext("2d");
                ctx.reset();

                if (!data || data.length === 0) return;

                var w = width;
                var h = height;
                var padding = 4;
                var drawH = h - padding * 2;
                var drawW = w - padding * 2;
                var step = drawW / (data.length - 1);

                // Fill gradient
                var grd = ctx.createLinearGradient(0, padding, 0, h);
                grd.addColorStop(0, Qt.rgba(
                    lineColor.r, lineColor.g, lineColor.b, 0.35));
                grd.addColorStop(1, Qt.rgba(
                    lineColor.r, lineColor.g, lineColor.b, 0.05));

                ctx.beginPath();
                ctx.moveTo(padding, h);

                for (var i = 0; i < data.length; i++) {
                    var val = Math.min(data[i], maxVal);
                    var x = padding + i * step;
                    var y = padding + drawH * (1 - val / maxVal);
                    ctx.lineTo(x, y);
                }
                ctx.lineTo(padding + (data.length - 1) * step, h);
                ctx.closePath();
                ctx.fillStyle = grd;
                ctx.fill();

                // Line
                ctx.beginPath();
                for (var j = 0; j < data.length; j++) {
                    var valL = Math.min(data[j], maxVal);
                    var xL = padding + j * step;
                    var yL = padding + drawH * (1 - valL / maxVal);
                    if (j === 0) ctx.moveTo(xL, yL);
                    else ctx.lineTo(xL, yL);
                }
                ctx.lineWidth = 2;
                ctx.strokeStyle = lineColor;
                ctx.stroke();

                // Current value dot
                if (data.length > 0) {
                    var lastVal = Math.min(data[data.length - 1], maxVal);
                    var dotX = padding + (data.length - 1) * step;
                    var dotY = padding + drawH * (1 - lastVal / maxVal);
                    ctx.beginPath();
                    ctx.arc(dotX, dotY, 4, 0, 2 * Math.PI);
                    ctx.fillStyle = lineColor;
                    ctx.fill();

                    // Soft glow
                    ctx.beginPath();
                    ctx.arc(dotX, dotY, 8, 0, 2 * Math.PI);
                    ctx.fillStyle = Qt.rgba(lineColor.r, lineColor.g, lineColor.b, 0.25);
                    ctx.fill();
                }
            }

            onDataChanged: requestPaint()
            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
        }
    }
}
