#include "chatView/ChatViewViewModel.hpp"
#include "chatView/LlamaInference.hpp"
#include "application/AppStateMachine.hpp"
#include <QGuiApplication>
#include <QInputMethod>
#include <QFile>
#include <QTextStream>
#include <QVariantMap>
#include <QStringList>
#include <QSet>
#include <QRegExp>
#include <QtGlobal>

ChatViewViewModel::ChatViewViewModel(QObject* parent) : QObject(parent) {
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged, this, &ChatViewViewModel::onStateMachineChanged);
    
    auto inputMethod = QGuiApplication::inputMethod();
    if (inputMethod) {
        connect(inputMethod, &QInputMethod::visibleChanged, this, &ChatViewViewModel::onInputMethodVisibleChanged);
    }

    loadKnowledgeBase();
    
    m_llama = new LlamaInference(this);
    m_hasVirtualKeyboard = true; 
    m_keyboardVisible = false;
    connect(m_llama, &LlamaInference::isLoadedChanged, this, &ChatViewViewModel::isLoadedChanged);
    connect(m_llama, &LlamaInference::isLoadingChanged, this, &ChatViewViewModel::isLoadingChanged);
    connect(m_llama, &LlamaInference::tokenGenerated, this, &ChatViewViewModel::onTokenGenerated);
    
    // Asynchronously load the model to avoid blocking UI startup
    QtConcurrent::run([this]() {
        bool loaded = m_llama->loadModel("frontend/models/phi-3-mini.gguf");
        if (!loaded) loaded = m_llama->loadModel("models/phi-3-mini.gguf");
        if (!loaded) loaded = m_llama->loadModel("../frontend/models/phi-3-mini.gguf");
        
        if (!loaded) {
            qWarning() << "Failed to load llama model from any expected path.";
        }
    });
    
    connect(&m_inferenceWatcher, &QFutureWatcher<QString>::finished, this, &ChatViewViewModel::onInferenceFinished);

    // Initial welcome message
    addMessage("ASIC Chatbot", "Hello! I am your AI assistant running locally on Jetson. How can I help you?");
}

ChatViewViewModel::~ChatViewViewModel() {
    delete m_llama;
}

bool ChatViewViewModel::isActive() const {
    return m_isActive;
}

bool ChatViewViewModel::isLoaded() const {
    return m_llama && m_llama->isLoaded();
}

bool ChatViewViewModel::isLoading() const {
    return m_llama && m_llama->isLoading();
}

bool ChatViewViewModel::keyboardVisible() const {
    return QGuiApplication::inputMethod()->isVisible();
}

bool ChatViewViewModel::hasVirtualKeyboard() const {
#ifdef HAS_VIRTUAL_KEYBOARD
    return true;
#else
    return false;
#endif
}

QVariantList ChatViewViewModel::messages() const {
    return m_messages;
}

bool ChatViewViewModel::isThinking() const {
    return m_isThinking;
}

void ChatViewViewModel::requestMainView() {
    dismissKeyboard();
    AppStateMachine::instance().returnToMain();
}

void ChatViewViewModel::dismissKeyboard() {
    auto inputMethod = QGuiApplication::inputMethod();
    if (inputMethod) {
        inputMethod->hide();
    }
}

void ChatViewViewModel::sendMessage(const QString& message) {
    if (message.trimmed().isEmpty() || m_isThinking) return;

    addMessage("User", message);
    
    m_isThinking = true;
    emit isThinkingChanged();

    // 1. RAG: Search knowledge base for context
    QString context;
    float bestScore = 0.0f;
    for (const auto& entry : m_knowledgeBase) {
        for (const auto& pattern : entry.patterns) {
            float score = calculateSimilarity(message.toLower(), pattern);
            if (score > bestScore) {
                bestScore = score;
                context = entry.response;
            }
        }
    }

    qDebug() << "RAG: Best similarity score =" << bestScore;
    if (bestScore > 0) {
        qDebug() << "RAG: Matched context =" << context;
    }

    // 2. Augment the prompt using the Phi-3 chat template
    QString augmentedPrompt;
    if (bestScore > 0.3f) {
        augmentedPrompt = QString("<|system|>\nYou are a robot assistant. Use the provided context to answer the user's question. If the answer is not in the context, say you don't know.\nContext: %1<|end|>\n<|user|>\n%2<|end|>\n<|assistant|>\n").arg(context).arg(message);
        qDebug() << "RAG: Using Phi-3 augmented prompt with context.";
    } else {
        augmentedPrompt = QString("<|user|>\n%1<|end|>\n<|assistant|>\n").arg(message);
        qDebug() << "RAG: Similarity low (" << bestScore << "), using default template.";
    }
    
    qDebug() << "Final Prompt sent to LLM:\n" << augmentedPrompt;

    // 3. Start asynchronous inference
    // Add an empty message placeholder for streaming
    addMessage("ASIC Chatbot", "");
    m_inferenceWatcher.setFuture(m_llama->generateResponse(augmentedPrompt));

    emit requestScrollToBottom();
}

void ChatViewViewModel::onInferenceFinished() {
    // Inference Finished (QFuture done)
    m_isThinking = false;
    emit isThinkingChanged();
    emit requestScrollToBottom();
}

void ChatViewViewModel::onTokenGenerated(const QString& token) {
    if (m_messages.isEmpty()) return;
    
    // Append token to the last message (must be from "ASIC Chatbot")
    QVariantMap lastMsg = m_messages.last().toMap();
    if (lastMsg["sender"].toString() == "ASIC Chatbot") {
        lastMsg["message"] = lastMsg["message"].toString() + token;
        m_messages[m_messages.size() - 1] = lastMsg;
        emit messagesChanged();
        emit requestScrollToBottom();
    }
}

void ChatViewViewModel::clearHistory() {
    m_messages.clear();
    addMessage("ASIC Chatbot", "Hello! I am your AI assistant running locally on Jetson. How can I help you?");
    emit messagesChanged();
    emit requestScrollToBottom();
}

void ChatViewViewModel::refreshModel() {
    m_llama->loadModel("phi-3-mini.gguf");
}

void ChatViewViewModel::stopChat() {
    if (m_isThinking) {
        qDebug() << "[VM] User requested stop of inference.";
        m_llama->stopInference();
        
        // Append interruption notice to the last message if it's from the bot
        if (!m_messages.isEmpty()) {
            QVariantMap lastMsg = m_messages.last().toMap();
            if (lastMsg["sender"].toString() == "ASIC Chatbot") {
                lastMsg["message"] = lastMsg["message"].toString() + "\n\n[The response was interrupted]";
                m_messages[m_messages.size() - 1] = lastMsg;
                emit messagesChanged();
                emit requestScrollToBottom();
            }
        }
    }
}

void ChatViewViewModel::onStateMachineChanged() {
    bool active = (AppStateMachine::instance().currentState() == "ChatView");
    if (m_isActive != active) {
        m_isActive = active;
        emit isActiveChanged();
    }
}

void ChatViewViewModel::onInputMethodVisibleChanged() {
    bool visible = QGuiApplication::inputMethod()->isVisible();
    if (m_keyboardVisible != visible) {
        m_keyboardVisible = visible;
        emit keyboardVisibleChanged();
        if (visible) {
            emit requestScrollToBottom();
        }
    }
}

void ChatViewViewModel::addMessage(const QString& sender, const QString& message) {
    QVariantMap msg;
    msg["sender"] = sender;
    msg["message"] = message;
    m_messages.append(msg);
    emit messagesChanged();
}

void ChatViewViewModel::loadKnowledgeBase() {
    m_knowledgeBase.clear();
    
    // Use absolute path for development on Jetson Xavier
    QString absolutePath = "/home/kien/Development/frontend_ASIC_MobileRobot/frontend/knowledge.txt";
    QFile file(absolutePath);
    
    if (!file.exists()) {
        QString appPath = QCoreApplication::applicationDirPath();
        QStringList possiblePaths = {
            "knowledge.txt",
            appPath + "/knowledge.txt",
            appPath + "/../frontend/knowledge.txt",
            "../frontend/knowledge.txt"
        };

        for (const QString& path : possiblePaths) {
            file.setFileName(path);
            if (file.exists()) break;
        }
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Knowledge base loaded from:" << file.fileName();
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(":");
            if (parts.size() >= 2) {
                KnowledgeEntry entry;
                entry.patterns = parts[0].trimmed().toLower().split(",");
                for (QString& p : entry.patterns) p = p.trimmed();
                entry.response = parts[1].trimmed();
                m_knowledgeBase.append(entry);
            }
        }
        file.close();
    }

    // Default fallback knowledge
    if (m_knowledgeBase.isEmpty()) {
        qWarning() << "Knowledge base file not found even after searching. Using fallbacks.";
        m_knowledgeBase.append({{"hello", "hi"}, "I am the ASIC Lab assistant."});
        m_knowledgeBase.append({{"location", "where"}, "The laboratory is in the H1 building."});
    }
}

float ChatViewViewModel::calculateSimilarity(const QString& s1, const QString& s2) {
    // Keyword match logic (Robot-specific optimization)
    if (s1.contains(s2, Qt::CaseInsensitive) || s2.contains(s1, Qt::CaseInsensitive)) {
        return 1.0f;
    }

    QStringList words1 = s1.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    QStringList words2 = s2.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    if (words1.isEmpty() || words2.isEmpty()) return 0.0f;
    QSet<QString> set1 = QSet<QString>::fromList(words1);
    QSet<QString> set2 = QSet<QString>::fromList(words2);
    int intersection = 0;
    for (const QString& word : set1) if (set2.contains(word)) intersection++;
    return static_cast<float>(intersection) / (set1.size() + set2.size() - intersection);
}
