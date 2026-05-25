#include "chatView/ChatViewViewModel.hpp"
#include "chatView/LlamaInference.hpp"
#include "application/AppStateMachine.hpp"
#include <QGuiApplication>
#include <QInputMethod>
#include <QTimer>
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
    addMessage("ASIC Chatbot", "Hello! I am your AI assistant. How can I help you?");
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

bool ChatViewViewModel::isGenerating() const {
    return m_isGenerating;
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
    if (message.trimmed().isEmpty() || m_isThinking || m_isGenerating) return;

    addMessage("User", message);
    
    m_isThinking = true;
    m_isGenerating = false;
    emit isThinkingChanged();
    emit isGeneratingChanged();

    // 1. RAG: Search knowledge base for context
    QString context;
    float bestScore = 0.0f;
    QString lowerMsg = message.toLower();
    QStringList queryWords = lowerMsg.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    QSet<QString> querySet = QSet<QString>::fromList(queryWords);
    
    for (const KnowledgeEntry& entry : m_knowledgeBase) {
        float entryBestScore = 0;
        // Check keywords
        for (int i = 0; i < entry.patterns.size(); ++i) {
            float score = calculateSimilarity(entry.patternWordSets[i], querySet, entry.patterns[i], message.toLower());
            if (score > entryBestScore) entryBestScore = score;
        }
        
        // Scan response text for extra relevance (helps with finding specific names)
        float textMatchScore = 0;
        int matchCount = 0;
        QString responseLower = entry.response.toLower();
        for (const QString& qWord : queryWords) {
            if (qWord.length() > 3 && responseLower.contains(qWord)) {
                matchCount++;
            }
        }
        if (!queryWords.isEmpty()) {
            textMatchScore = (float)matchCount / queryWords.size();
            // Weight text matches slightly less than keyword matches to avoid false positives
            if (textMatchScore * 0.8f > entryBestScore) entryBestScore = textMatchScore * 0.8f;
        }

        if (entryBestScore > bestScore || (entryBestScore == bestScore && bestScore > 0)) {
            bestScore = entryBestScore;
            context = entry.response;
        }
    }

    qDebug() << "RAG: Best similarity score =" << bestScore;
    if (bestScore > 0) {
        qDebug() << "RAG: Matched context =" << context;
    }

    if (bestScore >= 0.8f) {
        qDebug() << "RAG FAST PATH: score" << bestScore << ">= 0.8, typing response instantly without thinking bubble.";
        
        m_isThinking = false;
        m_isGenerating = true;
        emit isThinkingChanged();
        emit isGeneratingChanged();
        
        // Add an empty message bubble for the typing animation
        addMessage("ASIC Chatbot", "");
        
        // Fast typing timer: appends 2 characters every 15ms for a rapid, sleek appearance
        QTimer* typingTimer = new QTimer(this);
        typingTimer->setInterval(15);
        
        connect(typingTimer, &QTimer::timeout, this, [this, typingTimer, context, idx = 0]() mutable {
            if (idx < context.length()) {
                int charsToAppend = qMin(2, context.length() - idx);
                QString chunk = context.mid(idx, charsToAppend);
                idx += charsToAppend;
                
                if (!m_messages.isEmpty()) {
                    QVariantMap lastMsg = m_messages.last().toMap();
                    if (lastMsg["sender"].toString() == "ASIC Chatbot") {
                        lastMsg["message"] = lastMsg["message"].toString() + chunk;
                        m_messages[m_messages.size() - 1] = lastMsg;
                        emit messagesChanged();
                        emit requestScrollToBottom();
                    }
                }
            } else {
                typingTimer->stop();
                typingTimer->deleteLater();
                
                m_isGenerating = false;
                emit isGeneratingChanged();
                emit requestScrollToBottom();
            }
        });
        
        typingTimer->start();
        return;
    }

    // 2. Augment the prompt using standard chat template
    //    System prompt compressed to ~40 tokens to reduce prefill time.
    QString augmentedPrompt;
    if (bestScore > 0.25f) {
        augmentedPrompt = QString(
            "<|system|>\n"
            "You are UIT Assistant (ASIC Bot), an autonomous mobile navigation and direction robot at the Faculty of Computer Engineering, UIT. "
            "Answer concisely using the provided context. Reply in English.\n"
            "Context: %1<|end|>\n"
            "<|user|>\n%2<|end|>\n"
            "<|assistant|>\n"
        ).arg(context).arg(message);
        qDebug() << "RAG: Using compact augmented prompt with context.";
    } else {
        augmentedPrompt = QString(
            "<|system|>\n"
            "You are UIT Assistant (ASIC Bot), an autonomous mobile navigation and direction robot. Answer concisely in English.<|end|>\n"
            "<|user|>\n%1<|end|>\n"
            "<|assistant|>\n"
        ).arg(message);
        qDebug() << "RAG: Similarity low (" << bestScore << "), using minimal template.";
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
    m_isGenerating = false;
    emit isThinkingChanged();
    emit isGeneratingChanged();
    emit requestScrollToBottom();
}

void ChatViewViewModel::onTokenGenerated(const QString& token) {
    if (m_messages.isEmpty()) return;
    
    // Transition from thinking to generating on the first token
    if (m_isThinking) {
        m_isThinking = false;
        m_isGenerating = true;
        emit isThinkingChanged();
        emit isGeneratingChanged();
    }
    
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
    addMessage("ASIC Chatbot", "Hello! I am your AI assistant. How can I help you?");
    emit messagesChanged();
    emit requestScrollToBottom();
}

void ChatViewViewModel::refreshModel() {
    m_llama->loadModel("qwen2.5");
}

void ChatViewViewModel::stopChat() {
    if (m_isThinking || m_isGenerating) {
        qDebug() << "[VM] User requested stop of inference.";
        m_llama->stopInference();
        
        m_isThinking = false;
        m_isGenerating = false;
        emit isThinkingChanged();
        emit isGeneratingChanged();
        
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
    QString absolutePath = QCoreApplication::applicationDirPath() + "/../../frontend/knowledge.txt";
    QFile file(absolutePath);
    
    if (!file.exists()) {
        QString appPath = QCoreApplication::applicationDirPath();
        QStringList possiblePaths;
        possiblePaths << "knowledge.txt"
                      << appPath + "/knowledge.txt"
                      << appPath + "/../frontend/knowledge.txt"
                      << "../frontend/knowledge.txt";

        for (const QString& path : possiblePaths) {
            file.setFileName(path);
            if (file.exists()) break;
        }
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Knowledge base loaded from:" << file.fileName();
        QTextStream in(&file);
        QRegExp wordSplitRule("\\W+");
        while (!in.atEnd()) {
            QString line = in.readLine();
            int colonIdx = line.indexOf(':');
            if (colonIdx > 0) {
                KnowledgeEntry entry;
                QString patternsPart = line.left(colonIdx);
                QString responsePart = line.mid(colonIdx + 1);
                
                entry.patterns = patternsPart.trimmed().toLower().split(",");
                for (QString& p : entry.patterns) {
                    p = p.trimmed();
                    entry.patternWordSets.append(QSet<QString>::fromList(p.split(wordSplitRule, QString::SkipEmptyParts)));
                }
                entry.response = responsePart.trimmed();
                entry.response.replace("\\n", "\n");
                m_knowledgeBase.append(entry);
            }
        }
        file.close();
    }

    // Default fallback knowledge
    if (m_knowledgeBase.isEmpty()) {
        qWarning() << "Knowledge base file not found even after searching. Using fallbacks.";
        
        KnowledgeEntry e1;
        e1.patterns << "hello" << "hi";
        e1.patternWordSets << QSet<QString>({"hello"}) << QSet<QString>({"hi"});
        e1.response = "I am the ASIC Lab assistant.";
        m_knowledgeBase.append(e1);

        KnowledgeEntry e2;
        e2.patterns << "location" << "where";
        e2.patternWordSets << QSet<QString>({"location"}) << QSet<QString>({"where"});
        e2.response = "The laboratory is in the H1 building.";
        m_knowledgeBase.append(e2);
    }
}

float ChatViewViewModel::calculateSimilarity(const QSet<QString>& set1, const QSet<QString>& set2, const QString& s1, const QString& s2) {
    if (set1.isEmpty() || set2.isEmpty()) return 0.0f;

    // Fast path: Exact sequence match
    if (s2.contains(s1)) {
        return 1.0f;
    }

    int intersection = 0;
    for (const QString& word : set1) {
        if (set2.contains(word)) intersection++;
    }
    
    // Use overlap coefficient relative to the pattern size.
    // This makes the match robust against "noise" words in the user query.
    return static_cast<float>(intersection) / set1.size();
}
