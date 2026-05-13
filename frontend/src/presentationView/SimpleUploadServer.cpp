#include "presentationView/SimpleUploadServer.hpp"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

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
}

void SimpleUploadServer::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, &SimpleUploadServer::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &SimpleUploadServer::onDisconnected);
}

void SimpleUploadServer::onReadyRead() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray request = socket->readAll();
    if (request.startsWith("GET")) {
        handleGet(socket);
    } else if (request.startsWith("POST")) {
        handlePost(socket, request);
    }
}

void SimpleUploadServer::onDisconnected() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        socket->deleteLater();
    }
}

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
        "<p>Select a .txt file to display on the robot</p>"
        "<form method='POST' enctype='multipart/form-data'>"
        "<input type='file' name='file' accept='.txt' required><br>"
        "<button type='submit'>Upload to Robot</button>"
        "</form>"
        "</div>"
        "</body></html>";

    socket->write("HTTP/1.1 200 OK\r\n");
    socket->write("Content-Type: text/html\r\n");
    socket->write(QString("Content-Length: %1\r\n").arg(html.length()).toUtf8());
    socket->write("\r\n");
    socket->write(html.toUtf8());
    socket->flush();
    socket->disconnectFromHost();
}

void SimpleUploadServer::handlePost(QTcpSocket* socket, const QByteArray& data) {
    // Very basic multipart parsing
    // In a real scenario, we should use a proper HTTP parser
    int headerEnd = data.indexOf("\r\n\r\n");
    if (headerEnd == -1) return;

    QByteArray body = data.mid(headerEnd + 4);
    
    // Find filename and content
    // Look for "filename=\"...\""
    int filenamePos = data.indexOf("filename=\"");
    if (filenamePos != -1) {
        filenamePos += 10;
        int filenameEnd = data.indexOf("\"", filenamePos);
        QString filename = data.mid(filenamePos, filenameEnd - filenamePos);
        
        // Find double newline after filename header which marks start of content
        int contentStart = data.indexOf("\r\n\r\n", filenameEnd);
        if (contentStart != -1) {
            contentStart += 4;
            // The boundary is at the end. Find the last boundary.
            int boundaryEnd = data.lastIndexOf("\r\n--");
            if (boundaryEnd != -1 && boundaryEnd > contentStart) {
                QByteArray fileContent = data.mid(contentStart, boundaryEnd - contentStart);
                
                QString tempPath = QDir::tempPath() + "/" + filename;
                QFile file(tempPath);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(fileContent);
                    file.close();
                    qDebug() << "SimpleUploadServer: Received file" << filename << "saved to" << tempPath;
                    
                    emit fileUploaded(tempPath);

                    QString response = "<html><body style='font-family:sans-serif; text-align:center; padding-top:100px;'><h1>Success!</h1><p>File uploaded to robot.</p></body></html>";
                    socket->write("HTTP/1.1 200 OK\r\n");
                    socket->write("Content-Type: text/html\r\n");
                    socket->write(QString("Content-Length: %1\r\n").arg(response.length()).toUtf8());
                    socket->write("\r\n");
                    socket->write(response.toUtf8());
                    socket->flush();
                    socket->disconnectFromHost();
                    return;
                }
            }
        }
    }

    socket->write("HTTP/1.1 400 Bad Request\r\n\r\n");
    socket->disconnectFromHost();
}
