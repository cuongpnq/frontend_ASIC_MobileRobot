#pragma once

#include <QObject>
#include <QStringList>

class SimpleUploadServer;
class QProcess;

class PresentationViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(QString content READ content NOTIFY contentChanged)
    Q_PROPERTY(QString qrCodeUrl READ qrCodeUrl NOTIFY qrCodeUrlChanged)
    Q_PROPERTY(bool isMobileImportActive READ isMobileImportActive NOTIFY isMobileImportActiveChanged)
    Q_PROPERTY(QString uploadStatus READ uploadStatus NOTIFY uploadStatusChanged)
    Q_PROPERTY(QStringList fileList READ fileList NOTIFY fileListChanged)
    // File viewer
    Q_PROPERTY(bool isFileViewerOpen READ isFileViewerOpen NOTIFY isFileViewerOpenChanged)
    Q_PROPERTY(QString viewerFileName READ viewerFileName NOTIFY viewerFileNameChanged)
    Q_PROPERTY(QString viewerContent READ viewerContent NOTIFY viewerContentChanged)
    Q_PROPERTY(bool isSlideMode READ isSlideMode NOTIFY isSlideModeChanged)
    Q_PROPERTY(bool isConverting READ isConverting NOTIFY isConvertingChanged)
    Q_PROPERTY(int currentSlideIndex READ currentSlideIndex NOTIFY currentSlideIndexChanged)
    Q_PROPERTY(int totalSlides READ totalSlides NOTIFY totalSlidesChanged)
    Q_PROPERTY(QString currentSlideImage READ currentSlideImage NOTIFY currentSlideImageChanged)

public:
    explicit PresentationViewViewModel(QObject* parent = nullptr);
    ~PresentationViewViewModel() override;

    bool isActive() const;
    QString content() const;
    QString qrCodeUrl() const;
    bool isMobileImportActive() const;
    QString uploadStatus() const;
    QStringList fileList() const;

    bool isFileViewerOpen() const;
    QString viewerFileName() const;
    QString viewerContent() const;
    bool isSlideMode() const;
    bool isConverting() const;
    int currentSlideIndex() const;
    int totalSlides() const;
    QString currentSlideImage() const;

    Q_INVOKABLE void requestMainView();
    Q_INVOKABLE void startMobileImport();
    Q_INVOKABLE void stopMobileImport();
    Q_INVOKABLE void loadFromFile(const QString& filePath);
    Q_INVOKABLE void deleteFile(const QString& fileName);
    Q_INVOKABLE void deleteAllFiles();
    Q_INVOKABLE void refreshFileList();
    Q_INVOKABLE void openFile(const QString& fileName);
    Q_INVOKABLE void closeFileViewer();
    Q_INVOKABLE void nextSlide();
    Q_INVOKABLE void prevSlide();

    static QString fileUploadDir();

signals:
    void isActiveChanged();
    void contentChanged();
    void qrCodeUrlChanged();
    void isMobileImportActiveChanged();
    void uploadStatusChanged();
    void fileListChanged();
    void isFileViewerOpenChanged();
    void viewerFileNameChanged();
    void viewerContentChanged();
    void isSlideModeChanged();
    void isConvertingChanged();
    void currentSlideIndexChanged();
    void totalSlidesChanged();
    void currentSlideImageChanged();

private slots:
    void onStateMachineChanged();
    void loadContent();
    void onFileUploaded(const QString& filePath);

private:
    bool convertToPdf(const QString& inputPath, const QString& outputDir, QString* outPdfPath, QString* outError = nullptr);
    void convertPdfToImages(const QString& pdfPath);
    void collectSlideImages();
    void cleanupSlideImages();

    bool m_isActive = false;
    QString m_content;
    QString m_qrCodeUrl;
    bool m_isMobileImportActive = false;
    QString m_uploadStatus;
    SimpleUploadServer* m_server = nullptr;
    QStringList m_fileList;

    // File viewer state
    bool m_isFileViewerOpen = false;
    QString m_viewerFileName;
    QString m_viewerContent;
    bool m_isSlideMode = false;
    bool m_isConverting = false;
    int m_currentSlideIndex = 0;
    QStringList m_slideImages;
    QString m_slideTempDir;
};
