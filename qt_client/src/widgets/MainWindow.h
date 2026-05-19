#pragma once

#include <QJsonArray>
#include <QMainWindow>

#include "models/User.h"

class ApiClient;
class ChatPage;
class DocumentsPage;
class KnowledgeBasePage;
class QListWidget;
class QPushButton;
class QStackedWidget;
class QTableWidgetItem;
class SessionManager;
class SettingsPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(SessionManager *sessionManager, QWidget *parent = nullptr);

    void initialize();

signals:
    void logoutRequested();

private:
    void buildUi();
    void connectUi();
    void applySession();
    void loadAppInfo();
    void loadUsage();
    void loadConversations();
    void loadConversationMessages(int conversationId);
    void loadDocuments();
    void loadKnowledgeBase();
    void sendCurrentMessage();
    void startNewConversation();
    void uploadDocument();
    void ingestKnowledgeBase();
    void logout();
    void openSupportLink();
    QString formatCitationsHtml(const QJsonArray &citations, double confidence, const QString &sourceValidation, const QString &refusalReason) const;

    SessionManager *m_sessionManager = nullptr;
    ApiClient *m_apiClient = nullptr;

    QListWidget *m_pagesList = nullptr;
    QPushButton *m_newChatButton = nullptr;
    QPushButton *m_supportSidebarButton = nullptr;
    QListWidget *m_conversationsList = nullptr;
    QStackedWidget *m_pages = nullptr;
    ChatPage *m_chatPage = nullptr;
    DocumentsPage *m_documentsPage = nullptr;
    KnowledgeBasePage *m_knowledgeBasePage = nullptr;
    SettingsPage *m_settingsPage = nullptr;

    int m_currentConversationId = -1;
    QString m_supportUrl = "https://dalink.to/legalyze";
};
