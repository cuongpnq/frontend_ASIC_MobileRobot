#pragma once

#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QMap>
#include <QByteArray>

class SimpleUploadServer : public QTcpServer {
    Q_OBJECT
public:
    explicit SimpleUploadServer(QObject* parent = nullptr);
    bool startServer(quint16 port = 8081);
    void stopServer();

signals:
    void fileUploaded(const QString& filePath);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    // Per-socket receive buffer — accumulates chunks until the full
    // HTTP request (headers + body) has arrived.
    QMap<QTcpSocket*, QByteArray> m_buffers;

    void handleGet(QTcpSocket* socket);
    void handlePost(QTcpSocket* socket, const QByteArray& data);

    // Returns true when the full HTTP request has been received.
    bool isRequestComplete(const QByteArray& data) const;
    // Parse Content-Length from raw request headers.
    qint64 parseContentLength(const QByteArray& data) const;
};
