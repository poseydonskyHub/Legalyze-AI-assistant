#pragma once

#include <QWidget>

class QComboBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QTextBrowser;

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);

    QString messageText() const;
    QString mode() const;
    void setUsageInfo(const QString &text);
    void clearInput();
    void appendStatus(const QString &text);
    void appendUserMessage(const QString &text);
    void appendAssistantMessage(const QString &text);
    void setCitationsHtml(const QString &html);
    void clearConversationView();
    void setBusy(bool busy);

signals:
    void sendRequested();

private:
    QTextBrowser *m_messagesView = nullptr;
    QTextBrowser *m_citationsView = nullptr;
    QComboBox *m_modeCombo = nullptr;
    QPlainTextEdit *m_inputEdit = nullptr;
    QPushButton *m_sendButton = nullptr;
    QLabel *m_usageLabel = nullptr;
};
