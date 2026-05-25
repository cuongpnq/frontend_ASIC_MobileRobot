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
    
    // Parse model name from incoming path (backward compatibility with GUI config references)
    if (modelPath.contains("qwen", Qt::CaseInsensitive)) {
        m_modelName = "qwen2.5";
    } else if (modelPath.contains("llama", Qt::CaseInsensitive)) {
        m_modelName = "llama3.1";
    } else {
        m_modelName = "qwen2.5"; // Default optimized model for Jetson Xavier
    }
    
    // Create local manager for thread-safety (loading occurs in separate thread)
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    // Ollama runs on port 11434 by default. GET http://localhost:11434/ returns 200 OK "Ollama is running"
    QNetworkRequest request(QUrl("http://localhost:11434/"));
    QNetworkReply* reply = manager->get(request);
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        m_isLoaded = true;
        qDebug() << "Ollama Server detected on localhost:11434. Target model:" << m_modelName;
    } else {
        m_isLoaded = false;
        qWarning() << "Ollama Server not found on localhost:11434. Error:" << reply->errorString();
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
        json["model"] = m_modelName;
        json["stream"] = true;

        // Parse template structure into standard Ollama system instructions and user prompt
        QString systemPrompt = "";
        QString userPrompt = prompt;

        if (prompt.contains("<|system|>")) {
            int sysStart = prompt.indexOf("<|system|>") + 10;
            int sysEnd = prompt.indexOf("<|end|>", sysStart);
            if (sysEnd == -1) sysEnd = prompt.indexOf("<|endoftext|>", sysStart);
            if (sysEnd != -1) {
                systemPrompt = prompt.mid(sysStart, sysEnd - sysStart).trimmed();
                
                int userStart = prompt.indexOf("<|user|>\n", sysEnd);
                if (userStart != -1) {
                    userStart += 9;
                    int userEnd = prompt.indexOf("<|end|>", userStart);
                    if (userEnd == -1) userEnd = prompt.indexOf("<|endoftext|>", userStart);
                    if (userEnd != -1) {
                        userPrompt = prompt.mid(userStart, userEnd - userStart).trimmed();
                    } else {
                        userPrompt = prompt.mid(userStart).trimmed();
                        if (userPrompt.endsWith("<|assistant|>")) {
                            userPrompt.chop(13);
                            userPrompt = userPrompt.trimmed();
                        }
                    }
                }
            }
        }

        if (!systemPrompt.isEmpty()) {
            json["system"] = systemPrompt;
        }
        json["prompt"] = userPrompt;

        // Wrap inference options for Ollama
        QJsonObject options;
        options["num_predict"] = 128; // Cap output at 128 tokens for low-latency FAQ

        QJsonArray stopTokens;
        stopTokens.append("<|end|>");
        stopTokens.append("<|endoftext|>");
        stopTokens.append("</s>");
        stopTokens.append("\n\n\n");
        options["stop"] = stopTokens;

        options["temperature"] = 0.2;
        options["top_k"] = 20;
        options["top_p"] = 0.8;
        options["repeat_penalty"] = 1.3;

        json["options"] = options;
        
        QNetworkRequest request(QUrl("http://localhost:11434/api/generate"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply* reply = manager.post(request, QJsonDocument(json).toJson());
        
        // Register current reply for stopping
        m_currentReply = reply;

        QString fullContent = "";
        QEventLoop loop;
        
        connect(reply, &QNetworkReply::readyRead, [this, reply, &fullContent]() {
            while (reply->canReadLine()) {
                QByteArray line = reply->readLine().trimmed();
                if (line.isEmpty()) continue;
                QJsonDocument doc = QJsonDocument::fromJson(line);
                if (!doc.isNull()) {
                    QString token = doc.object().value("response").toString();
                    fullContent += token;
                    QMetaObject::invokeMethod(this, [this, token]() {
                        emit tokenGenerated(token);
                    }, Qt::QueuedConnection);
                }
            }
        });

        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        m_currentReply = nullptr;

        QString finalResult = fullContent;
        if (reply->error() != QNetworkReply::NoError && reply->error() != QNetworkReply::OperationCanceledError) {
            finalResult = "Error: Ollama Server communication failed.";
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
