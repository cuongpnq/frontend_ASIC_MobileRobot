#ifndef CHATVIEWVIEWMODEL_HPP
#define CHATVIEWVIEWMODEL_HPP

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QFutureWatcher>
#include <QVector>
#include <QSet>
#include "LlamaInference.hpp"

struct KnowledgeEntry {
    QStringList patterns;
    QVector<QSet<QString>> patternWordSets;
    QString response;
};

class ChatViewViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList messages READ messages NOTIFY messagesChanged)
    Q_PROPERTY(bool isThinking READ isThinking NOTIFY isThinkingChanged)
    Q_PROPERTY(bool isGenerating READ isGenerating NOTIFY isGeneratingChanged)
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY isLoadedChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool hasVirtualKeyboard READ hasVirtualKeyboard NOTIFY hasVirtualKeyboardChanged)
    Q_PROPERTY(bool keyboardVisible READ keyboardVisible NOTIFY keyboardVisibleChanged)
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)

public:
    explicit ChatViewViewModel(QObject* parent = nullptr);
    ~ChatViewViewModel();

    QVariantList messages() const;
    bool isThinking() const;
    bool isGenerating() const;
    bool isLoaded() const;
    bool isLoading() const;
    bool hasVirtualKeyboard() const;
    bool keyboardVisible() const;
    bool isActive() const;

    Q_INVOKABLE void sendMessage(const QString& message);
    Q_INVOKABLE void clearHistory();
    Q_INVOKABLE void refreshModel();
    Q_INVOKABLE void stopChat();
    Q_INVOKABLE void requestMainView();
    Q_INVOKABLE void dismissKeyboard();

signals:
    void messagesChanged();
    void isThinkingChanged();
    void isGeneratingChanged();
    void isLoadedChanged();
    void isLoadingChanged();
    void requestScrollToBottom();
    void hasVirtualKeyboardChanged();
    void keyboardVisibleChanged();
    void isActiveChanged();

private slots:
    void onStateMachineChanged();
    void onInputMethodVisibleChanged();
    void onTokenGenerated(const QString& token);
    void onInferenceFinished();

private:
    void addMessage(const QString& sender, const QString& message);
    void loadKnowledgeBase();
    float calculateSimilarity(const QSet<QString>& set1, const QSet<QString>& set2, const QString& s1, const QString& s2);

    LlamaInference* m_llama;
    QVariantList m_messages;
    bool m_isThinking = false;
    bool m_isGenerating = false;
    QFutureWatcher<QString> m_inferenceWatcher;
    QVector<KnowledgeEntry> m_knowledgeBase;
    bool m_hasVirtualKeyboard = true;
    bool m_keyboardVisible = false;
    bool m_isActive = false;
};

#endif // CHATVIEWVIEWMODEL_HPP
