#include "presentationView/SimpleUploadServer.hpp"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QDateTime>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Sends a styled HTML status page to the mobile browser.
// isSuccess=true → green checkmark; isSuccess=false → red cross.
static void sendStatusPage(QTcpSocket* socket, bool isSuccess,
                            const QString& title, const QString& detail,
                            const QString& filename = QString())
{
    QString iconColor  = isSuccess ? "#27ae60" : "#e74c3c";
    QString iconSymbol = isSuccess ? "&#10003;" : "&#10007;"; // ✓ or ✗
    QString bgColor    = isSuccess ? "#f0fdf4" : "#fff5f5";
    QString badgeText  = isSuccess ? "SUCCESS" : "FAILED";

    QString fileRow;
    if (!filename.isEmpty()) {
        fileRow = QString(
            "<div style='margin-top:12px; padding:10px 16px; background:%1; "
            "border-radius:10px; font-size:0.9rem; color:#444; word-break:break-all;'>"
            "<strong>File:</strong> %2</div>")
            .arg(isSuccess ? "#e8f5e9" : "#fce8e8")
            .arg(filename.toHtmlEscaped());
    }

    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss  dd/MM/yyyy");

    QString html = QString(
        "<!DOCTYPE html>"
        "<html><head>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<meta charset='utf-8'>"
        "<title>%1</title>"
        "<style>"
        "*{box-sizing:border-box;margin:0;padding:0}"
        "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;"
        "     background:%2; min-height:100vh; display:flex;"
        "     align-items:center; justify-content:center; padding:24px;}"
        ".card{background:#fff; border-radius:24px;"
        "      box-shadow:0 8px 32px rgba(0,0,0,0.12);"
        "      padding:40px 32px; max-width:380px; width:100%;"
        "      text-align:center;}"
        ".icon{width:80px; height:80px; border-radius:50%;"
        "      background:%3; color:#fff; font-size:2.6rem;"
        "      display:inline-flex; align-items:center; justify-content:center;"
        "      margin-bottom:20px;}"
        ".badge{display:inline-block; padding:4px 14px;"
        "       background:%3; color:#fff; border-radius:20px;"
        "       font-size:0.75rem; font-weight:700; letter-spacing:1px;"
        "       margin-bottom:16px;}"
        "h1{font-size:1.5rem; color:#1a1a1a; margin-bottom:8px;}"
        "p{color:#666; font-size:0.95rem; line-height:1.5;}"
        ".ts{margin-top:24px; font-size:0.78rem; color:#aaa;}"
        "a{display:block; margin-top:24px; text-decoration:none;"
        "  background:#2C2C2C; color:#fff; padding:12px;"
        "  border-radius:12px; font-size:0.95rem; font-weight:600;}"
        "a:active{opacity:0.85;}"
        "</style></head><body>"
        "<div class='card'>"
        "<div class='icon'>%4</div>"
        "<div class='badge'>%5</div>"
        "<h1>%6</h1>"
        "<p>%7</p>"
        "%8"
        "<p class='ts'>%9</p>"
        "<a href='/'>&#8617; Upload another file</a>"
        "</div></body></html>")
        .arg(title)           // %1 <title>
        .arg(bgColor)         // %2 body bg
        .arg(iconColor)       // %3 icon + badge color
        .arg(iconSymbol)      // %4 icon symbol
        .arg(badgeText)       // %5 badge text
        .arg(title)           // %6 h1
        .arg(detail)          // %7 detail paragraph
        .arg(fileRow)         // %8 optional file row
        .arg(timestamp);      // %9 timestamp

    QByteArray body = html.toUtf8();
    int statusCode  = isSuccess ? 200 : 400;
    QString statusMsg = isSuccess ? "OK" : "Bad Request";

    socket->write(QString("HTTP/1.1 %1 %2\r\n").arg(statusCode).arg(statusMsg).toUtf8());
    socket->write("Content-Type: text/html; charset=utf-8\r\n");
    socket->write(QString("Content-Length: %1\r\n").arg(body.size()).toUtf8());
    socket->write("Connection: close\r\n");
    socket->write("\r\n");
    socket->write(body);
    socket->flush();
    socket->disconnectFromHost();
}

// Checks if the full HTTP request has been received by comparing the actual
// body bytes received against the Content-Length declared in the headers.
bool SimpleUploadServer::isRequestComplete(const QByteArray& data) const {
    int headerEnd = data.indexOf("\r\n\r\n");
    if (headerEnd == -1) return false; // headers not yet complete

    qint64 contentLength = parseContentLength(data);
    if (contentLength < 0) {
        // No Content-Length (e.g. GET) — headers-only request is complete
        return true;
    }

    qint64 bodyReceived = data.size() - (headerEnd + 4);
    return bodyReceived >= contentLength;
}

qint64 SimpleUploadServer::parseContentLength(const QByteArray& data) const {
    int clPos = data.indexOf("Content-Length:");
    if (clPos == -1) {
        // Case-insensitive fallback
        clPos = data.toLower().indexOf("content-length:");
    }
    if (clPos == -1) return -1;

    int lineEnd = data.indexOf("\r\n", clPos);
    if (lineEnd == -1) return -1;

    QByteArray valueStr = data.mid(clPos + 15, lineEnd - clPos - 15).trimmed();
    bool ok = false;
    qint64 val = valueStr.toLongLong(&ok);
    return ok ? val : -1;
}

// ---------------------------------------------------------------------------
// Server lifecycle
// ---------------------------------------------------------------------------

SimpleUploadServer::SimpleUploadServer(QObject* parent) : QTcpServer(parent) {
}

bool SimpleUploadServer::startServer(quint16 port) {
    if (!this->listen(QHostAddress::Any, port)) {
        qWarning() << "SimpleUploadServer: Could not start server on port" << port;
        return false;
    }
    qDebug() << "SimpleUploadServer: Started on port" << port;
    return true;
}

void SimpleUploadServer::stopServer() {
    this->close();
    m_buffers.clear();
}

// ---------------------------------------------------------------------------
// Connection handling
// ---------------------------------------------------------------------------

void SimpleUploadServer::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    m_buffers[socket] = QByteArray();
    connect(socket, &QTcpSocket::readyRead,    this, &SimpleUploadServer::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &SimpleUploadServer::onDisconnected);
}

void SimpleUploadServer::onReadyRead() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    // Accumulate all available data into this socket's buffer
    m_buffers[socket] += socket->readAll();
    const QByteArray& request = m_buffers[socket];

    // Wait until we have the complete request before processing
    if (!isRequestComplete(request)) {
        return;
    }

    // Full request received — dispatch and remove buffer
    QByteArray fullRequest = m_buffers.take(socket);

    if (fullRequest.startsWith("GET")) {
        handleGet(socket);
    } else if (fullRequest.startsWith("POST")) {
        handlePost(socket, fullRequest);
    } else {
        socket->write("HTTP/1.1 405 Method Not Allowed\r\n\r\n");
        socket->disconnectFromHost();
    }
}

void SimpleUploadServer::onDisconnected() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        m_buffers.remove(socket);
        socket->deleteLater();
    }
}

// ---------------------------------------------------------------------------
// GET — serve the upload page
// ---------------------------------------------------------------------------

void SimpleUploadServer::handleGet(QTcpSocket* socket) {
    QString html =
        "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<style>"
        "body { font-family: sans-serif; display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; margin: 0; background-color: #F5F4EF; }"
        ".card { background: white; padding: 2rem; border-radius: 20px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); text-align: center; width: 80%; max-width: 400px; }"
        "h1 { color: #2C2C2C; margin-bottom: 1.5rem; }"
        "input[type=file] { margin: 1.5rem 0; }"
        "button { background: #2C2C2C; color: white; border: none; padding: 10px 20px; border-radius: 10px; font-size: 1.1rem; cursor: pointer; width: 100%; }"
        "button:active { transform: scale(0.98); }"
        "</style></head><body>"
        "<div class='card'>"
        "<h1>Robot File Import</h1>"
        "<p>Select a .txt or .pptx file to upload to the robot</p>"
        "<form method='POST' enctype='multipart/form-data'>"
        "<input type='file' name='file' accept='.txt,.pptx' required><br>"
        "<button type='submit'>Upload to Robot</button>"
        "</form>"
        "</div>"
        "</body></html>";

    QByteArray body = html.toUtf8();
    socket->write("HTTP/1.1 200 OK\r\n");
    socket->write("Content-Type: text/html; charset=utf-8\r\n");
    socket->write(QString("Content-Length: %1\r\n").arg(body.size()).toUtf8());
    socket->write("Connection: close\r\n");
    socket->write("\r\n");
    socket->write(body);
    socket->flush();
    socket->disconnectFromHost();
}

// ---------------------------------------------------------------------------
// POST — parse multipart/form-data and save the file
// ---------------------------------------------------------------------------

void SimpleUploadServer::handlePost(QTcpSocket* socket, const QByteArray& data) {
    // ---- locate header/body boundary ----
    int headerEnd = data.indexOf("\r\n\r\n");
    if (headerEnd == -1) {
        socket->write("HTTP/1.1 400 Bad Request\r\n\r\n");
        socket->disconnectFromHost();
        return;
    }

    QByteArray headers = data.left(headerEnd);
    QByteArray body    = data.mid(headerEnd + 4);

    // ---- extract multipart boundary from Content-Type header ----
    QByteArray boundary;
    {
        int ctPos = headers.toLower().indexOf("content-type:");
        if (ctPos != -1) {
            int lineEnd = headers.indexOf("\r\n", ctPos);
            QByteArray ctLine = (lineEnd != -1)
                                ? headers.mid(ctPos, lineEnd - ctPos)
                                : headers.mid(ctPos);
            int bPos = ctLine.toLower().indexOf("boundary=");
            if (bPos != -1) {
                boundary = "--" + ctLine.mid(bPos + 9).trimmed();
            }
        }
    }

    if (boundary.isEmpty()) {
        qWarning() << "SimpleUploadServer: No boundary found in POST";
        sendStatusPage(socket, false,
            "Upload Failed",
            "The request was malformed (no multipart boundary). Please try again.");
        return;
    }

    // ---- find the part that contains a filename ----
    int filenamePos = data.indexOf("filename=\"");
    if (filenamePos == -1) {
        sendStatusPage(socket, false,
            "Upload Failed",
            "No file was included in the request. Please choose a file and try again.");
        return;
    }

    filenamePos += 10; // skip past 'filename="'
    int filenameEnd = data.indexOf("\"", filenamePos);
    if (filenameEnd == -1) {
        sendStatusPage(socket, false,
            "Upload Failed",
            "Could not read the filename from the request. Please try again.");
        return;
    }

    QString filename = QString::fromUtf8(data.mid(filenamePos, filenameEnd - filenamePos));

    // ---- find the start of the file content (double CRLF after part headers) ----
    int contentStart = data.indexOf("\r\n\r\n", filenameEnd);
    if (contentStart == -1) {
        sendStatusPage(socket, false,
            "Upload Failed",
            "Could not locate file content in the request. Please try again.");
        return;
    }
    contentStart += 4;

    // ---- find the closing boundary ----
    QByteArray closingBoundary = "\r\n" + boundary + "--";
    int boundaryEnd = data.lastIndexOf(closingBoundary);
    if (boundaryEnd == -1) {
        // Fallback: try simple boundary without trailing '--'
        boundaryEnd = data.lastIndexOf("\r\n" + boundary);
    }

    if (boundaryEnd <= contentStart) {
        qWarning() << "SimpleUploadServer: Could not locate closing boundary";
        sendStatusPage(socket, false,
            "Upload Failed",
            "The file transfer was incomplete or corrupted. Please try again.");
        return;
    }

    QByteArray fileContent = data.mid(contentStart, boundaryEnd - contentStart);

    // ---- save to temp dir ----
    QString tempPath = QDir::tempPath() + "/" + filename;
    QFile file(tempPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "SimpleUploadServer: Cannot write to" << tempPath;
        sendStatusPage(socket, false,
            "Upload Failed",
            "The robot could not save the file (disk error). Please contact support.",
            filename);
        return;
    }

    file.write(fileContent);
    file.close();
    qDebug() << "SimpleUploadServer: Received file" << filename
             << "(" << fileContent.size() << "bytes) saved to" << tempPath;

    emit fileUploaded(tempPath);

    // ---- send success response ----
    sendStatusPage(socket, true,
        "Upload Successful",
        QString("Your file was received by the robot (%1 KB). You can now close this page.")
            .arg(QString::number(fileContent.size() / 1024.0, 'f', 1)),
        filename);
}
