#pragma once

#include <QObject>

class SimpleUploadServer;

class PresentationViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(QString content READ content NOTIFY contentChanged)
    Q_PROPERTY(QString qrCodeUrl READ qrCodeUrl NOTIFY qrCodeUrlChanged)
    Q_PROPERTY(bool isMobileImportActive READ isMobileImportActive NOTIFY isMobileImportActiveChanged)
    Q_PROPERTY(QString uploadStatus READ uploadStatus NOTIFY uploadStatusChanged)

public:
    explicit PresentationViewViewModel(QObject* parent = nullptr);
    ~PresentationViewViewModel() override;

    bool isActive() const;
    QString content() const;
    QString qrCodeUrl() const;
    bool isMobileImportActive() const;
    QString uploadStatus() const;

    Q_INVOKABLE void requestMainView();
    Q_INVOKABLE void startMobileImport();
    Q_INVOKABLE void stopMobileImport();
    Q_INVOKABLE void loadFromFile(const QString& filePath);

signals:
    void isActiveChanged();
    void contentChanged();
    void qrCodeUrlChanged();
    void isMobileImportActiveChanged();
    void uploadStatusChanged();

private slots:
    void onStateMachineChanged();
    void loadContent();
    void onFileUploaded(const QString& filePath);

private:
    bool m_isActive = false;
    QString m_content;
    QString m_qrCodeUrl;
    bool m_isMobileImportActive = false;
    QString m_uploadStatus;
    SimpleUploadServer* m_server = nullptr;
};
