#include "chatView/ChatViewViewModel.hpp"
#include "chatView/LlamaInference.hpp"
#include "application/AppStateMachine.hpp"
#include "application/SessionLogger.hpp"
#include <QGuiApplication>
#include <QInputMethod>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

ChatViewViewModel::ChatViewViewModel(QObject* parent) : QObject(parent) {
    connect(&AppStateMachine::instance(), &AppStateMachine::currentStateChanged,
            this, &ChatViewViewModel::onStateMachineChanged);

    auto inputMethod = QGuiApplication::inputMethod();
    if (inputMethod) {
        connect(inputMethod, &QInputMethod::visibleChanged,
                this, &ChatViewViewModel::onInputMethodVisibleChanged);
    }

    // Load the system prompt from knowledge.txt
    loadSystemPrompt();

    m_llama = new LlamaInference(this);
    connect(m_llama, &LlamaInference::isLoadedChanged,  this, &ChatViewViewModel::isLoadedChanged);
    connect(m_llama, &LlamaInference::isLoadingChanged, this, &ChatViewViewModel::isLoadingChanged);
    connect(m_llama, &LlamaInference::tokenGenerated,   this, &ChatViewViewModel::onTokenGenerated);

    // Asynchronously detect Ollama server — avoids blocking UI startup
    QtConcurrent::run([this]() {
        bool loaded = m_llama->loadModel("qwen2.5");
        if (!loaded) {
            qWarning() << "Ollama server not available at startup.";
        }
    });

    connect(&m_inferenceWatcher, &QFutureWatcher<QString>::finished,
            this, &ChatViewViewModel::onInferenceFinished);

    addMessage("ASIC Chatbot", "Hello! I am ASIC Bot, your AI assistant at UIT. How can I help you?");
}

ChatViewViewModel::~ChatViewViewModel() {
    delete m_llama;
}

// ---------------------------------------------------------------------------
// Property getters
// ---------------------------------------------------------------------------

QVariantList ChatViewViewModel::messages()  const { return m_messages; }
bool ChatViewViewModel::isThinking()        const { return m_isThinking; }
bool ChatViewViewModel::isGenerating()      const { return m_isGenerating; }
bool ChatViewViewModel::isLoaded()          const { return m_llama && m_llama->isLoaded(); }
bool ChatViewViewModel::isLoading()         const { return m_llama && m_llama->isLoading(); }
bool ChatViewViewModel::keyboardVisible()   const { return QGuiApplication::inputMethod()->isVisible(); }
bool ChatViewViewModel::hasVirtualKeyboard() const {
#ifdef HAS_VIRTUAL_KEYBOARD
    return true;
#else
    return false;
#endif
}
bool ChatViewViewModel::isActive() const { return m_isActive; }

// ---------------------------------------------------------------------------
// Navigation / keyboard
// ---------------------------------------------------------------------------

void ChatViewViewModel::requestMainView() {
    dismissKeyboard();
    AppStateMachine::instance().returnToMain();
}

void ChatViewViewModel::dismissKeyboard() {
    auto inputMethod = QGuiApplication::inputMethod();
    if (inputMethod) inputMethod->hide();
}

// ---------------------------------------------------------------------------
// Send message  —  pure system-prompt approach, no RAG
// ---------------------------------------------------------------------------

void ChatViewViewModel::sendMessage(const QString& message) {
    if (message.trimmed().isEmpty() || m_isThinking || m_isGenerating) return;

    // 1. Display user message in UI
    addMessage("User", message);

    // 2. Append user turn to the conversation history
    QJsonObject userMsg;
    userMsg["role"]    = "user";
    userMsg["content"] = message;
    m_chatHistory.append(userMsg);

    // 3. Show thinking state
    m_isThinking  = true;
    m_isGenerating = false;
    emit isThinkingChanged();
    emit isGeneratingChanged();

    // 4. Add empty bot bubble for streaming
    addMessage("ASIC Chatbot", "");

    // 5. Send system prompt + recent conversation history to Ollama /api/chat.
    //    Cap history to the last 6 messages (3 user+assistant turns) so that
    //    prefill time stays bounded even in long conversations.
    static constexpr int MAX_HISTORY_MESSAGES = 6;
    QJsonArray historyToSend;
    int start = qMax(0, m_chatHistory.size() - MAX_HISTORY_MESSAGES);
    for (int i = start; i < m_chatHistory.size(); ++i) {
        historyToSend.append(m_chatHistory[i]);
    }

    m_inferenceWatcher.setFuture(m_llama->chat(m_systemPrompt, historyToSend));

    // ── Telemetry: start chat timer and log query ────────────────────────
    m_chatTimer.start();
    m_tokenCount       = 0;
    m_firstToken       = true;
    m_pendingQuestion  = message.trimmed();
    SessionLogger::instance().logEvent(QStringLiteral("chat"),
                                       QStringLiteral("chat"),
                                       QStringLiteral("query_start"),
                                       { { QStringLiteral("question"),       m_pendingQuestion },
                                         { QStringLiteral("history_turns"),  m_chatHistory.size() - 1 } });

    emit requestScrollToBottom();
}

// ---------------------------------------------------------------------------
// Inference callbacks
// ---------------------------------------------------------------------------

void ChatViewViewModel::onTokenGenerated(const QString& token) {
    if (m_messages.isEmpty()) return;

    // Transition from thinking → generating on first token
    if (m_isThinking) {
        m_isThinking  = false;
        m_isGenerating = true;
        emit isThinkingChanged();
        emit isGeneratingChanged();
    }

    // ── Telemetry: capture time-to-first-token ────────────────────────
    if (m_firstToken) {
        m_firstToken = false;
        const double ttftMs = double(m_chatTimer.elapsed());
        SessionLogger::instance().logEvent(QStringLiteral("chat"),
                                           QStringLiteral("chat"),
                                           QStringLiteral("first_token"),
                                           { { QStringLiteral("ttft_ms"), ttftMs } });
    }
    ++m_tokenCount;

    // Stream token into the last bot bubble
    QVariantMap lastMsg = m_messages.last().toMap();
    if (lastMsg["sender"].toString() == "ASIC Chatbot") {
        lastMsg["message"] = lastMsg["message"].toString() + token;
        m_messages[m_messages.size() - 1] = lastMsg;
        emit messagesChanged();
        emit requestScrollToBottom();
    }
}

void ChatViewViewModel::onInferenceFinished() {
    m_isThinking  = false;
    m_isGenerating = false;
    emit isThinkingChanged();
    emit isGeneratingChanged();
    emit requestScrollToBottom();

    // Append the bot's complete reply to conversation history so context is maintained
    QString reply;
    if (!m_messages.isEmpty()) {
        QVariantMap lastMsg = m_messages.last().toMap();
        if (lastMsg["sender"].toString() == "ASIC Chatbot") {
            reply = lastMsg["message"].toString();
            if (!reply.isEmpty() && reply != "[The response was interrupted]") {
                QJsonObject assistantMsg;
                assistantMsg["role"]    = "assistant";
                assistantMsg["content"] = reply;
                m_chatHistory.append(assistantMsg);
            }
        }
    }

    // ── Telemetry: log generation complete stats ────────────────────────
    const double totalMs     = double(m_chatTimer.elapsed());
    const double tokensPerSec = (totalMs > 0 && m_tokenCount > 0)
                                ? double(m_tokenCount) / (totalMs / 1000.0)
                                : 0.0;
    const bool   interrupted = reply.endsWith(QStringLiteral("[The response was interrupted]"));

    SessionLogger::instance().logEvent(QStringLiteral("chat"),
                                       QStringLiteral("chat"),
                                       QStringLiteral("complete"),
                                       { { QStringLiteral("question"),        m_pendingQuestion },
                                         { QStringLiteral("token_count"),     m_tokenCount     },
                                         { QStringLiteral("total_ms"),        totalMs          },
                                         { QStringLiteral("tokens_per_sec"),  tokensPerSec     },
                                         { QStringLiteral("interrupted"),     interrupted      },
                                         { QStringLiteral("answer_len_chars"),reply.length()   } });
}

// ---------------------------------------------------------------------------
// History management
// ---------------------------------------------------------------------------

void ChatViewViewModel::clearHistory() {
    m_messages.clear();
    m_chatHistory = QJsonArray(); // reset conversation context
    addMessage("ASIC Chatbot", "Hello! I am ASIC Bot, your AI assistant at UIT. How can I help you?");
    emit messagesChanged();
    emit requestScrollToBottom();
}

void ChatViewViewModel::refreshModel() {
    QtConcurrent::run([this]() {
        m_llama->loadModel("qwen2.5");
    });
}

void ChatViewViewModel::stopChat() {
    if (m_isThinking || m_isGenerating) {
        qDebug() << "[VM] User requested stop of inference.";
        m_llama->stopInference();

        m_isThinking  = false;
        m_isGenerating = false;
        emit isThinkingChanged();
        emit isGeneratingChanged();

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

// ---------------------------------------------------------------------------
// State machine
// ---------------------------------------------------------------------------

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
        if (visible) emit requestScrollToBottom();
    }
}

// ---------------------------------------------------------------------------
// Message helpers
// ---------------------------------------------------------------------------

void ChatViewViewModel::addMessage(const QString& sender, const QString& message) {
    QVariantMap msg;
    msg["sender"]  = sender;
    msg["message"] = message;
    m_messages.append(msg);
    emit messagesChanged();
}

// ---------------------------------------------------------------------------
// System prompt loader  —  reads knowledge.txt as plain prose
// ---------------------------------------------------------------------------

void ChatViewViewModel::loadSystemPrompt() {
    QString appPath = QCoreApplication::applicationDirPath();

    QStringList candidates;
    candidates << appPath + "/../../frontend/knowledge.txt"
               << appPath + "/../frontend/knowledge.txt"
               << appPath + "/knowledge.txt"
               << "frontend/knowledge.txt"
               << "knowledge.txt";

    for (const QString& path : candidates) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            m_systemPrompt = in.readAll().trimmed();
            file.close();
            qDebug() << "System prompt loaded from:" << path
                     << "(" << m_systemPrompt.length() << "chars)";
            return;
        }
    }

    // Minimal fallback if the file is missing
    qWarning() << "knowledge.txt not found. Using built-in fallback system prompt.";
    m_systemPrompt =
        "You are ASIC Bot, an autonomous mobile robot assistant at the University of Information Technology (UIT), Vietnam. "
        "Answer questions about the university and the robot application helpfully and concisely. "
        "Reply in the same language as the user.";
}
