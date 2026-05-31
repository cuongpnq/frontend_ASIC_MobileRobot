#include "presentationView/PresentationViewViewModel.hpp"
#include "presentationView/SimpleUploadServer.hpp"
#include "application/AppStateMachine.hpp"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QNetworkInterface>
#include <QUrl>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QProcess>
#include <QDateTime>

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
    cleanupSlideImages();
}

// ---------------------------------------------------------------------------
// Static helper – resolves the fileupload directory with multiple fallbacks.
// ---------------------------------------------------------------------------
QString PresentationViewViewModel::fileUploadDir() {
    QString appPath = QCoreApplication::applicationDirPath();
    QStringList candidates;
    candidates << appPath + "/../../frontend/resource/fileupload"
               << appPath + "/../frontend/resource/fileupload"
               << appPath + "/resource/fileupload"
               << "frontend/resource/fileupload";

    for (const QString& p : candidates) {
        QDir d(p);
        if (d.exists()) {
            return d.absolutePath();
        }
    }

    // Fallback: create using first candidate
    QDir d(candidates.first());
    d.mkpath(".");
    return d.absolutePath();
}

// ---------------------------------------------------------------------------
// Property getters
// ---------------------------------------------------------------------------
bool PresentationViewViewModel::isActive() const { return m_isActive; }
QString PresentationViewViewModel::content() const { return m_content; }
QString PresentationViewViewModel::qrCodeUrl() const { return m_qrCodeUrl; }
bool PresentationViewViewModel::isMobileImportActive() const { return m_isMobileImportActive; }
QString PresentationViewViewModel::uploadStatus() const { return m_uploadStatus; }
QStringList PresentationViewViewModel::fileList() const { return m_fileList; }
bool PresentationViewViewModel::isFileViewerOpen() const { return m_isFileViewerOpen; }
QString PresentationViewViewModel::viewerFileName() const { return m_viewerFileName; }
QString PresentationViewViewModel::viewerContent() const { return m_viewerContent; }
bool PresentationViewViewModel::isSlideMode() const { return m_isSlideMode; }
bool PresentationViewViewModel::isConverting() const { return m_isConverting; }
int PresentationViewViewModel::currentSlideIndex() const { return m_currentSlideIndex; }
int PresentationViewViewModel::totalSlides() const { return m_slideImages.size(); }

QString PresentationViewViewModel::currentSlideImage() const {
    if (m_currentSlideIndex >= 0 && m_currentSlideIndex < m_slideImages.size()) {
        return m_slideImages.at(m_currentSlideIndex);
    }
    return QString();
}

// ---------------------------------------------------------------------------
// Navigation
// ---------------------------------------------------------------------------
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

    if (m_server->isListening()) {
        m_isMobileImportActive = true;
        emit isMobileImportActiveChanged();
        return;
    }

    if (m_server->startServer(8081)) {
        QString uploadUrl = QString("http://%1:8081").arg(localIp);
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
            if (m_isFileViewerOpen) closeFileViewer();
        }
    }
}

void PresentationViewViewModel::loadContent() {
    m_content = "";
    emit contentChanged();
    refreshFileList();
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
    QString destDir = fileUploadDir();
    QDir().mkpath(destDir);

    QFileInfo info(filePath);
    QString destPath = destDir + "/" + info.fileName();

    if (QFile::exists(destPath)) {
        QFile::remove(destPath);
    }

    if (QFile::copy(filePath, destPath)) {
        qDebug() << "Copied uploaded file to" << destPath;
    } else {
        qWarning() << "Failed to copy uploaded file to" << destPath;
    }

    m_uploadStatus = "File received: " + info.fileName();
    emit uploadStatusChanged();
    refreshFileList();
    
    QTimer::singleShot(2000, this, [this]() {
        stopMobileImport();
    });
}

void PresentationViewViewModel::refreshFileList() {
    QString dir = fileUploadDir();
    QDir d(dir);
    d.mkpath(".");

    QStringList filters;
    filters << "*.txt" << "*.pptx" << "*.ppt";
    QStringList files = d.entryList(filters, QDir::Files, QDir::Name | QDir::IgnoreCase);

    if (m_fileList != files) {
        m_fileList = files;
        emit fileListChanged();
    }
}

void PresentationViewViewModel::deleteFile(const QString& fileName) {
    QString dir = fileUploadDir();
    QString fullPath = dir + "/" + fileName;

    if (QFile::exists(fullPath)) {
        if (QFile::remove(fullPath)) {
            qDebug() << "Deleted file:" << fullPath;
        } else {
            qWarning() << "Failed to delete file:" << fullPath;
        }
    }
    refreshFileList();
}

void PresentationViewViewModel::deleteAllFiles() {
    QString dir = fileUploadDir();
    QDir d(dir);
    QStringList filters;
    filters << "*.txt" << "*.pptx" << "*.ppt";
    QStringList files = d.entryList(filters, QDir::Files);

    for (const QString& f : files) {
        QString fullPath = dir + "/" + f;
        if (!QFile::remove(fullPath)) {
            qWarning() << "Failed to delete file:" << fullPath;
        }
    }
    qDebug() << "Deleted all presentation files from" << dir;
    refreshFileList();
}

// ---------------------------------------------------------------------------
// File Viewer
// ---------------------------------------------------------------------------
void PresentationViewViewModel::openFile(const QString& fileName) {
    QString dir = fileUploadDir();
    QString fullPath = dir + "/" + fileName;

    m_viewerFileName = fileName;
    emit viewerFileNameChanged();

    if (fileName.endsWith(".txt", Qt::CaseInsensitive)) {
        // --- Text file ---
        QFile file(fullPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            m_viewerContent = in.readAll();
            file.close();
        } else {
            m_viewerContent = "Failed to open file.";
        }
        m_isSlideMode = false;
        m_isConverting = false;
        m_isFileViewerOpen = true;
        emit viewerContentChanged();
        emit isSlideModeChanged();
        emit isConvertingChanged();
        emit isFileViewerOpenChanged();
    } else if (fileName.endsWith(".pptx", Qt::CaseInsensitive) ||
               fileName.endsWith(".ppt", Qt::CaseInsensitive)) {
        // --- Presentation file ---
        m_isSlideMode = true;
        m_isConverting = true;
        m_isFileViewerOpen = true;
        m_viewerContent = "";
        m_slideImages.clear();
        m_currentSlideIndex = 0;

        emit isSlideModeChanged();
        emit isConvertingChanged();
        emit isFileViewerOpenChanged();
        emit viewerContentChanged();
        emit currentSlideIndexChanged();
        emit totalSlidesChanged();
        emit currentSlideImageChanged();

        convertPptxToImages(fullPath);
    }
}

void PresentationViewViewModel::closeFileViewer() {
    m_isFileViewerOpen = false;
    m_viewerContent = "";
    m_viewerFileName = "";
    m_isSlideMode = false;
    m_isConverting = false;
    m_slideImages.clear();
    m_currentSlideIndex = 0;

    emit isFileViewerOpenChanged();
    emit viewerContentChanged();
    emit viewerFileNameChanged();
    emit isSlideModeChanged();
    emit isConvertingChanged();
    emit currentSlideIndexChanged();
    emit totalSlidesChanged();
    emit currentSlideImageChanged();
}

void PresentationViewViewModel::nextSlide() {
    if (m_currentSlideIndex < m_slideImages.size() - 1) {
        m_currentSlideIndex++;
        emit currentSlideIndexChanged();
        emit currentSlideImageChanged();
    }
}

void PresentationViewViewModel::prevSlide() {
    if (m_currentSlideIndex > 0) {
        m_currentSlideIndex--;
        emit currentSlideIndexChanged();
        emit currentSlideImageChanged();
    }
}

// ---------------------------------------------------------------------------
// PPTX → PDF → PNG conversion (async, chained QProcesses)
// ---------------------------------------------------------------------------
void PresentationViewViewModel::convertPptxToImages(const QString& filePath) {
    cleanupSlideImages();

    m_slideTempDir = QDir::tempPath() + "/presentation_slides_"
                   + QString::number(QDateTime::currentMSecsSinceEpoch());
    QDir().mkpath(m_slideTempDir);

    // Step 1: libreoffice --headless --convert-to pdf
    QProcess* lo = new QProcess(this);
    QString tmpDir = m_slideTempDir;            // capture for lambda
    QString baseName = QFileInfo(filePath).completeBaseName();

    connect(lo, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, lo, tmpDir, baseName](int exitCode, QProcess::ExitStatus) {
        lo->deleteLater();
        QString pdfPath = tmpDir + "/" + baseName + ".pdf";

        if (exitCode != 0 || !QFile::exists(pdfPath)) {
            m_isConverting = false;
            m_viewerContent = "Conversion failed.\nInstall LibreOffice:\n  sudo apt install libreoffice-impress";
            emit isConvertingChanged();
            emit viewerContentChanged();
            return;
        }

        // Step 2: pdftoppm -png -r 200 <pdf> <prefix>
        QProcess* ppm = new QProcess(this);
        QString prefix = tmpDir + "/slide";
        connect(ppm, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, ppm](int code, QProcess::ExitStatus) {
            ppm->deleteLater();
            if (code != 0) {
                m_isConverting = false;
                m_viewerContent = "Image conversion failed.\nInstall poppler-utils:\n  sudo apt install poppler-utils";
                emit isConvertingChanged();
                emit viewerContentChanged();
                return;
            }
            collectSlideImages();
        });
        connect(ppm, &QProcess::errorOccurred, this, [this, ppm](QProcess::ProcessError) {
            ppm->deleteLater();
            m_isConverting = false;
            m_viewerContent = "pdftoppm not found.\n  sudo apt install poppler-utils";
            emit isConvertingChanged();
            emit viewerContentChanged();
        });
        ppm->start("pdftoppm", {"-png", "-r", "200", pdfPath, prefix});
    });

    connect(lo, &QProcess::errorOccurred, this, [this, lo](QProcess::ProcessError) {
        lo->deleteLater();
        m_isConverting = false;
        m_viewerContent = "LibreOffice not found.\n  sudo apt install libreoffice-impress";
        emit isConvertingChanged();
        emit viewerContentChanged();
    });

    lo->start("libreoffice", {"--headless", "--convert-to", "pdf",
                              "--outdir", tmpDir, filePath});
}

void PresentationViewViewModel::collectSlideImages() {
    QDir dir(m_slideTempDir);
    QStringList filters;
    filters << "slide-*.png";
    QStringList files = dir.entryList(filters, QDir::Files, QDir::Name);

    m_slideImages.clear();
    for (const QString& f : files) {
        m_slideImages << "file://" + dir.absoluteFilePath(f);
    }

    m_isConverting = false;
    if (m_slideImages.isEmpty()) {
        m_viewerContent = "No slides could be extracted.";
        emit viewerContentChanged();
    } else {
        m_currentSlideIndex = 0;
        m_viewerContent = "";
        emit viewerContentChanged();
    }
    emit isConvertingChanged();
    emit currentSlideIndexChanged();
    emit totalSlidesChanged();
    emit currentSlideImageChanged();
}

void PresentationViewViewModel::cleanupSlideImages() {
    if (!m_slideTempDir.isEmpty()) {
        QDir dir(m_slideTempDir);
        if (dir.exists()) {
            dir.removeRecursively();
        }
        m_slideTempDir.clear();
    }
    m_slideImages.clear();
}
