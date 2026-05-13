#include "presentationView/PresentationViewViewModel.hpp"
#include "presentationView/SimpleUploadServer.hpp"
#include "application/AppStateMachine.hpp"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QNetworkInterface>
#include <QUrl>
#include <QTimer>

PresentationViewViewModel::PresentationViewViewModel(QObject* parent) 
    : QObject(parent), m_isActive(false), m_server(new SimpleUploadServer(this))
{
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &PresentationViewViewModel::onStateMachineChanged);
    
    connect(m_server, &SimpleUploadServer::fileUploaded,
            this, &PresentationViewViewModel::onFileUploaded);
}

PresentationViewViewModel::~PresentationViewViewModel() {
    if (m_server) {
        m_server->stopServer();
    }
}

bool PresentationViewViewModel::isActive() const {
    return m_isActive;
}

QString PresentationViewViewModel::content() const {
    return m_content;
}

QString PresentationViewViewModel::qrCodeUrl() const {
    return m_qrCodeUrl;
}

bool PresentationViewViewModel::isMobileImportActive() const {
    return m_isMobileImportActive;
}

QString PresentationViewViewModel::uploadStatus() const {
    return m_uploadStatus;
}

void PresentationViewViewModel::requestMainView() {
    AppStateMachine::instance().returnToMain();
}

void PresentationViewViewModel::startMobileImport() {
    QString localIp;
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            localIp = address.toString();
            break;
        }
    }

    if (localIp.isEmpty()) {
        m_uploadStatus = "Error: No network connection detected.";
        emit uploadStatusChanged();
        return;
    }

    if (m_server->startServer(8081)) {
        QString uploadUrl = QString("http://%1:8081").arg(localIp);
        // Use qrserver.com API to generate QR code image URL
        m_qrCodeUrl = QString("https://api.qrserver.com/v1/create-qr-code/?size=400x400&data=%1").arg(uploadUrl);
        m_isMobileImportActive = true;
        m_uploadStatus = "Waiting for mobile upload...";
        
        emit qrCodeUrlChanged();
        emit isMobileImportActiveChanged();
        emit uploadStatusChanged();
    } else {
        m_uploadStatus = "Error: Could not start upload server.";
        emit uploadStatusChanged();
    }
}

void PresentationViewViewModel::stopMobileImport() {
    m_server->stopServer();
    m_isMobileImportActive = false;
    emit isMobileImportActiveChanged();
}

void PresentationViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "PresentationView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
        if (m_isActive) {
            loadContent();
        } else {
            stopMobileImport();
        }
    }
}

void PresentationViewViewModel::loadContent() {
    loadFromFile("frontend/presentation.txt");
}

void PresentationViewViewModel::loadFromFile(const QString& filePath) {
    QString localPath = filePath;
    if (filePath.startsWith("file://")) {
        localPath = QUrl(filePath).toLocalFile();
    }

    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open" << localPath;
        if (m_content.isEmpty()) {
            m_content = "Failed to load presentation content.";
        }
    } else {
        QTextStream in(&file);
        m_content = in.readAll();
        file.close();
    }
    emit contentChanged();
}

void PresentationViewViewModel::onFileUploaded(const QString& filePath) {
    m_uploadStatus = "File received! Loading...";
    emit uploadStatusChanged();
    
    loadFromFile(filePath);
    
    // Auto-close mobile import popup after success
    QTimer::singleShot(2000, this, [this]() {
        stopMobileImport();
    });
}
