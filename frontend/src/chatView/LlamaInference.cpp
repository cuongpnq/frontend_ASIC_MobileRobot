#include "chatView/LlamaInference.hpp"
#include <QDebug>

LlamaInference::LlamaInference(QObject* parent) : QObject(parent), m_networkManager(new QNetworkAccessManager(this)) {
}

LlamaInference::~LlamaInference() {
}

// ---------------------------------------------------------------------------
// Model / server detection
// ---------------------------------------------------------------------------

bool LlamaInference::loadModel(const QString& modelPath) {
    m_isLoading = true;
    emit isLoadingChanged();

    // Resolve model name from path string (backward compatibility with GUI config)
    if (modelPath.contains("0.5b", Qt::CaseInsensitive)) {
        m_modelName = "qwen2.5:0.5b";
    } else if (modelPath.contains("qwen", Qt::CaseInsensitive)) {
        m_modelName = "qwen2.5:0.5b"; // Default to 0.5b for faster inference on Jetson
    } else if (modelPath.contains("llama", Qt::CaseInsensitive)) {
        m_modelName = "llama3.1";
    } else {
        m_modelName = "qwen2.5:0.5b"; // Default: fastest model for Jetson Xavier
    }

    // Ping Ollama to verify it is running
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QNetworkRequest request(QUrl("http://localhost:11434/"));
    QNetworkReply* reply = manager->get(request);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        m_isLoaded = true;
        qDebug() << "Ollama server detected on localhost:11434. Model:" << m_modelName;
    } else {
        m_isLoaded = false;
        qWarning() << "Ollama server not found on localhost:11434. Error:" << reply->errorString();
    }

    m_isLoading = false;
    emit isLoadingChanged();
    emit isLoadedChanged();

    reply->deleteLater();
    manager->deleteLater();
    return m_isLoaded;
}

// ---------------------------------------------------------------------------
// Chat — uses /api/chat with full message history
// ---------------------------------------------------------------------------

QFuture<QString> LlamaInference::chat(const QString& systemPrompt, const QJsonArray& messages) {
    return QtConcurrent::run([this, systemPrompt, messages]() -> QString {

        // Build the messages array for /api/chat:
        //   [ { role: "system", content: <systemPrompt> },
        //     { role: "user",   content: "..." },
        //     { role: "assistant", content: "..." },
        //     ... ]
        QJsonArray chatMessages;

        // System message always comes first
        if (!systemPrompt.isEmpty()) {
            QJsonObject sysMsg;
            sysMsg["role"]    = "system";
            sysMsg["content"] = systemPrompt;
            chatMessages.append(sysMsg);
        }

        // Append the conversation history passed in
        for (const QJsonValue& v : messages) {
            chatMessages.append(v);
        }

        QJsonObject body;
        body["model"]    = m_modelName;
        body["messages"] = chatMessages;
        body["stream"]   = true;

        // Generation options — tuned for low-latency on Jetson Xavier
        QJsonObject options;
        options["temperature"]    = 0.3;
        options["top_k"]          = 20;
        options["top_p"]          = 0.85;
        options["repeat_penalty"] = 1.1;
        options["num_predict"]    = 350;   // enough for a full list-style answer
        options["num_ctx"]        = 2048;

        // Only stop on role-reversal patterns — do NOT use triple-newline,
        // it truncates numbered list answers mid-way.
        QJsonArray stop;
        stop.append("\nUser:");
        stop.append("\nHuman:");
        options["stop"] = stop;

        body["options"] = options;

        QNetworkAccessManager manager;
        QNetworkRequest request(QUrl("http://localhost:11434/api/chat"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply* reply = manager.post(request, QJsonDocument(body).toJson());
        m_currentReply = reply;

        QString fullContent;
        QEventLoop loop;

        // Stream tokens: each line from /api/chat looks like:
        // { "message": { "role": "assistant", "content": "<token>" }, "done": false }
        connect(reply, &QNetworkReply::readyRead, [this, reply, &fullContent]() {
            while (reply->canReadLine()) {
                QByteArray line = reply->readLine().trimmed();
                if (line.isEmpty()) continue;

                QJsonDocument doc = QJsonDocument::fromJson(line);
                if (doc.isNull()) continue;

                QJsonObject obj = doc.object();
                QString token = obj.value("message").toObject().value("content").toString();

                if (!token.isEmpty()) {
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

        if (reply->error() != QNetworkReply::NoError &&
            reply->error() != QNetworkReply::OperationCanceledError) {
            reply->deleteLater();
            return QString("Error: Ollama server communication failed.");
        }

        reply->deleteLater();
        return fullContent.trimmed();
    });
}

// ---------------------------------------------------------------------------
// Stop inference
// ---------------------------------------------------------------------------

void LlamaInference::stopInference() {
    if (m_currentReply && m_currentReply->isRunning()) {
        qDebug() << "[INF] Stopping AI inference...";
        m_currentReply->abort();
        m_currentReply = nullptr;
    }
}

void LlamaInference::onReplyFinished(QNetworkReply* reply) {
    Q_UNUSED(reply)
}
