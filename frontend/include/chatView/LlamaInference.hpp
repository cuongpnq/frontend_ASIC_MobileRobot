#ifndef LLAMAINFERENCE_HPP
#define LLAMAINFERENCE_HPP

#include <QObject>
#include <QStringList>
#include <QFuture>
#include <QtConcurrent>

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

    // Send a full message history to /api/chat (maintains conversation context).
    // systemPrompt  — injected as the very first "system" message
    // messages      — array of {role, content} objects for the conversation history
    QFuture<QString> chat(const QString& systemPrompt, const QJsonArray& messages);

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
    QString m_modelName = "qwen2.5";
};

#endif // LLAMAINFERENCE_HPP
