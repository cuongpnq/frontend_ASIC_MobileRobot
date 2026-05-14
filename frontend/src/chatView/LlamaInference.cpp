#include "chatView/LlamaInference.hpp"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

LlamaInference::LlamaInference(QObject* parent) : QObject(parent), m_networkManager(new QNetworkAccessManager(this)) {
}

LlamaInference::~LlamaInference() {
}

bool LlamaInference::loadModel(const QString& modelPath) {
    m_isLoading = true;
    emit isLoadingChanged();
    
    // Create local manager for thread-safety (loading occurs in separate thread)
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QNetworkRequest request(QUrl("http://localhost:8080/health"));
    QNetworkReply* reply = manager->get(request);
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        m_isLoaded = true;
        qDebug() << "AI Server detected and connected on localhost:8080";
    } else {
        m_isLoaded = false;
        qWarning() << "AI Server not found. Error:" << reply->errorString();
    }
    
    m_isLoading = false;
    emit isLoadingChanged();
    emit isLoadedChanged();
    
    reply->deleteLater();
    manager->deleteLater();
    return m_isLoaded;
}

QFuture<QString> LlamaInference::generateResponse(const QString& prompt) {
    return QtConcurrent::run([this, prompt]() -> QString {
        QNetworkAccessManager manager; // Locally created for thread-safe concurrent usage
        QJsonObject json;
        json["prompt"] = prompt;

        // ── Optimized generation parameters for Jetson ──
        // Cap output at 128 tokens — FAQ answers rarely exceed 50 tokens.
        // The stop tokens will terminate generation *before* hitting this limit.
        json["n_predict"] = 128;

        // Stop tokens: instruct the server to stop as soon as the model
        // emits any of these markers, preventing wasted generation.
        QJsonArray stopTokens;
        stopTokens.append("<|end|>");
        stopTokens.append("<|endoftext|>");
        stopTokens.append("</s>");
        stopTokens.append("\n\n\n");
        json["stop"] = stopTokens;

        // Low temperature for factual, deterministic answers (FAQ use-case)
        json["temperature"] = 0.2;
        json["top_k"] = 20;
        json["top_p"] = 0.8;
        json["repeat_penalty"] = 1.3;

        // Stream for progressive UI updates
        json["stream"] = true;
        
        QNetworkRequest request(QUrl("http://localhost:8080/completion"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply* reply = manager.post(request, QJsonDocument(json).toJson());
        
        // Register current reply for stopping
        m_currentReply = reply;

        QString fullContent = "";
        QEventLoop loop;
        
        connect(reply, &QNetworkReply::readyRead, [this, reply, &fullContent]() {
            while (reply->canReadLine()) {
                QByteArray line = reply->readLine();
                if (line.startsWith("data: ")) {
                    QByteArray jsonData = line.mid(6);
                    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
                    if (!doc.isNull()) {
                        QString token = doc.object().value("content").toString();
                        fullContent += token;
                        QMetaObject::invokeMethod(this, [this, token]() {
                            emit tokenGenerated(token);
                        }, Qt::QueuedConnection);
                    }
                }
            }
        });

        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        m_currentReply = nullptr;

        QString finalResult = fullContent;
        if (reply->error() != QNetworkReply::NoError && reply->error() != QNetworkReply::OperationCanceledError) {
            finalResult = "Error: AI Server communication failed.";
        }
        
        reply->deleteLater();
        return finalResult.trimmed();
    });
}

void LlamaInference::stopInference() {
    if (m_currentReply && m_currentReply->isRunning()) {
        qDebug() << "[INF] Stopping AI inference...";
        m_currentReply->abort();
        m_currentReply = nullptr;
    }
}

void LlamaInference::onReplyFinished(QNetworkReply* reply) {
}
