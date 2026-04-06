#ifndef LLAMAINFERENCE_HPP
#define LLAMAINFERENCE_HPP

#include <QObject>
#include <QStringList>
#include <QFuture>
#include <QtConcurrent>

// Use Qt Network for separate process communication
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>

class LlamaInference : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY isLoadedChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

public:
    explicit LlamaInference(QObject* parent = nullptr);
    ~LlamaInference();

    bool isLoaded() const { return m_isLoaded; }
    bool isLoading() const { return m_isLoading; }

    bool loadModel(const QString& modelPath);
    QFuture<QString> generateResponse(const QString& prompt);
    void stopInference();

signals:
    void isLoadedChanged();
    void isLoadingChanged();
    void tokenGenerated(const QString& token);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_networkManager = nullptr;
    QNetworkReply* m_currentReply = nullptr;
    bool m_isLoaded = false;
    bool m_isLoading = false;
};

#endif // LLAMAINFERENCE_HPP
