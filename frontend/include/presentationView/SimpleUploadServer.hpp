#pragma once

#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

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
    void handleGet(QTcpSocket* socket);
    void handlePost(QTcpSocket* socket, const QByteArray& data);
};
