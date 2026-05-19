#include "widgets/MainWindow.h"

#include "api/ApiClient.h"
#include "app/SessionManager.h"
#include "widgets/ChatPage.h"
#include "widgets/DocumentsPage.h"
#include "widgets/KnowledgeBasePage.h"
#include "widgets/SettingsPage.h"

#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHttpMultiPart>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

namespace {
QString replyDetail(QNetworkReply *reply)
{
    const QByteArray raw = reply->readAll();
    const QJsonDocument json = QJsonDocument::fromJson(raw);
    if (json.isObject()) {
        return json.object().value("detail").toString(QString::fromUtf8(raw));
    }
    return QString::fromUtf8(raw);
}
}

MainWindow::MainWindow(SessionManager *sessionManager, QWidget *parent)
    : QMainWindow(parent),
      m_sessionManager(sessionManager),
      m_apiClient(new ApiClient(this))
{
    buildUi();
    connectUi();
    applySession();
}

void MainWindow::initialize()
{
    applySession();
    loadAppInfo();
    loadUsage();
    loadConversations();
    loadDocuments();
    loadKnowledgeBase();
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *splitter = new QSplitter(central);
    splitter->setChildrenCollapsible(false);

    auto *sidebar = new QWidget(splitter);
    sidebar->setMinimumWidth(290);
    sidebar->setMaximumWidth(360);
    sidebar->setStyleSheet("background:#111b27; border-right:1px solid rgba(255,255,255,0.06);");

    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(18, 18, 18, 18);
    sidebarLayout->setSpacing(14);

    auto *brand = new QLabel("Legalyze", sidebar);
    brand->setStyleSheet("font-size:28px; font-weight:800; color:#eef4fb;");

    auto *brandHint = new QLabel("Демо-версия с локальным RAG, документами и официальными актами.", sidebar);
    brandHint->setWordWrap(true);
    brandHint->setStyleSheet("color:#9fb0c3; font-size:13px;");

    auto *pagesLabel = new QLabel("Разделы", sidebar);
    pagesLabel->setStyleSheet("font-size:13px; font-weight:700; color:#8fa6bc; text-transform:uppercase;");

    m_pagesList = new QListWidget(sidebar);
    m_pagesList->addItems({"Чат", "Документы", "База актов", "Настройки"});
    m_pagesList->setStyleSheet(
        "QListWidget { background:#172536; color:#eef4fb; border:1px solid rgba(255,255,255,0.08); border-radius:18px; padding:8px; }"
        "QListWidget::item { padding:12px 14px; border-radius:12px; }"
        "QListWidget::item:selected { background:#f4a340; color:#1b1308; font-weight:700; }");

    auto *conversationsLabel = new QLabel("Диалоги", sidebar);
    conversationsLabel->setStyleSheet("font-size:13px; font-weight:700; color:#8fa6bc; text-transform:uppercase;");

    m_newChatButton = new QPushButton("Новый диалог", sidebar);
    m_newChatButton->setCursor(Qt::PointingHandCursor);
    m_newChatButton->setStyleSheet(
        "QPushButton { background:#f4a340; color:#1b1308; border:none; border-radius:14px; padding:12px 16px; font-weight:700; }"
        "QPushButton:hover { background:#f6bc66; }");

    m_conversationsList = new QListWidget(sidebar);
    m_conversationsList->setStyleSheet(
        "QListWidget { background:#172536; color:#eef4fb; border:1px solid rgba(255,255,255,0.08); border-radius:18px; padding:8px; }"
        "QListWidget::item { padding:10px 12px; border-radius:12px; }"
        "QListWidget::item:selected { background:#24364b; }");

    auto *supportLabel = new QLabel("Поддержка проекта", sidebar);
    supportLabel->setStyleSheet("font-size:13px; font-weight:700; color:#8fa6bc; text-transform:uppercase;");

    m_supportSidebarButton = new QPushButton("DonationAlerts", sidebar);
    m_supportSidebarButton->setCursor(Qt::PointingHandCursor);
    m_supportSidebarButton->setStyleSheet(
        "QPushButton { background:#24364b; color:#eef4fb; border:1px solid rgba(244,163,64,0.22); border-radius:14px; padding:12px 16px; font-weight:700; text-align:left; }"
        "QPushButton:hover { background:#2d445d; border-color:rgba(244,163,64,0.45); }");

    sidebarLayout->addWidget(brand);
    sidebarLayout->addWidget(brandHint);
    sidebarLayout->addSpacing(4);
    sidebarLayout->addWidget(pagesLabel);
    sidebarLayout->addWidget(m_pagesList);
    sidebarLayout->addSpacing(6);
    sidebarLayout->addWidget(conversationsLabel);
    sidebarLayout->addWidget(m_newChatButton);
    sidebarLayout->addWidget(m_conversationsList, 1);
    sidebarLayout->addWidget(supportLabel);
    sidebarLayout->addWidget(m_supportSidebarButton);

    m_pages = new QStackedWidget(splitter);
    m_pages->setStyleSheet("background:#0e1620;");
    m_chatPage = new ChatPage(splitter);
    m_documentsPage = new DocumentsPage(splitter);
    m_knowledgeBasePage = new KnowledgeBasePage(splitter);
    m_settingsPage = new SettingsPage(splitter);

    m_pages->addWidget(m_chatPage);
    m_pages->addWidget(m_documentsPage);
    m_pages->addWidget(m_knowledgeBasePage);
    m_pages->addWidget(m_settingsPage);

    splitter->addWidget(sidebar);
    splitter->addWidget(m_pages);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    layout->addWidget(splitter);
    setCentralWidget(central);
    setStyleSheet("QMainWindow { background:#0b1219; }");
    resize(1720, 1040);
    setMinimumSize(1480, 900);
    setWindowTitle("Legalyze");
    statusBar()->setStyleSheet("QStatusBar { background:#111b27; color:#d7e3f0; border-top:1px solid rgba(255,255,255,0.06); }");
    statusBar()->showMessage("Готово к работе");
    m_pagesList->setCurrentRow(0);
}

void MainWindow::connectUi()
{
    connect(m_pagesList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0) {
            m_pages->setCurrentIndex(row);
        }
    });

    connect(m_newChatButton, &QPushButton::clicked, this, &MainWindow::startNewConversation);
    connect(m_conversationsList, &QListWidget::currentItemChanged, this, [this](QListWidgetItem *current) {
        if (current == nullptr) {
            return;
        }
        const int conversationId = current->data(Qt::UserRole).toInt();
        m_currentConversationId = conversationId;
        loadConversationMessages(conversationId);
    });

    connect(m_chatPage, &ChatPage::sendRequested, this, &MainWindow::sendCurrentMessage);
    connect(m_documentsPage, &DocumentsPage::uploadRequested, this, &MainWindow::uploadDocument);
    connect(m_documentsPage, &DocumentsPage::refreshRequested, this, &MainWindow::loadDocuments);
    connect(m_knowledgeBasePage, &KnowledgeBasePage::ingestRequested, this, &MainWindow::ingestKnowledgeBase);
    connect(m_knowledgeBasePage, &KnowledgeBasePage::refreshRequested, this, &MainWindow::loadKnowledgeBase);
    connect(m_settingsPage, &SettingsPage::logoutRequested, this, &MainWindow::logout);
    connect(m_settingsPage, &SettingsPage::supportRequested, this, &MainWindow::openSupportLink);
    connect(m_supportSidebarButton, &QPushButton::clicked, this, &MainWindow::openSupportLink);
}

void MainWindow::applySession()
{
    if (m_sessionManager == nullptr) {
        return;
    }

    m_apiClient->setBaseUrl(m_sessionManager->backendUrl());
    m_apiClient->setBearerToken(m_sessionManager->accessToken());
    m_settingsPage->setBackendUrl(m_sessionManager->backendUrl());

    const User user = m_sessionManager->currentUser();
    m_settingsPage->setUserInfo(QString("%1\n%2").arg(user.fullName, user.email));
}

void MainWindow::loadAppInfo()
{
    QNetworkReply *reply = m_apiClient->get("/app-info");
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            statusBar()->showMessage("Не удалось получить информацию о приложении");
            return;
        }

        const QJsonObject object = QJsonDocument::fromJson(reply->readAll()).object();
        const QString title = QString("%1 v%2").arg(object.value("app_name").toString(), object.value("api_version").toString());
        setWindowTitle(title);

        const QString donationUrl = object.value("donation_url").toString();
        if (!donationUrl.isEmpty()) {
            m_supportUrl = donationUrl;
        }
    });
}

void MainWindow::loadUsage()
{
    QNetworkReply *reply = m_apiClient->get("/v1/usage");
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            m_chatPage->setUsageInfo("Демо: лимит пока недоступен");
            m_settingsPage->setDemoInfo("Не удалось получить текущий дневной лимит.");
            return;
        }

        const QJsonObject object = QJsonDocument::fromJson(reply->readAll()).object();
        const int remaining = object.value("remaining_today").toInt();
        const int limit = object.value("daily_limit").toInt();
        const int used = object.value("used_today").toInt();
        const QString resetDate = object.value("reset_date").toString();

        m_chatPage->setUsageInfo(QString("Демо: осталось %1 из %2 запросов").arg(remaining).arg(limit));
        m_settingsPage->setDemoInfo(
            QString("Демо-режим активен\nИспользовано сегодня: %1 из %2\nОсталось: %3\nДата обновления лимита: %4")
                .arg(used)
                .arg(limit)
                .arg(remaining)
                .arg(resetDate));
    });
}

void MainWindow::loadConversations()
{
    QNetworkReply *reply = m_apiClient->get("/v1/conversations");
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            statusBar()->showMessage("Не удалось загрузить диалоги");
            return;
        }

        const QJsonArray items = QJsonDocument::fromJson(reply->readAll()).array();
        m_conversationsList->clear();
        for (const QJsonValue &value : items) {
            const QJsonObject object = value.toObject();
            auto *item = new QListWidgetItem(object.value("title").toString(), m_conversationsList);
            item->setData(Qt::UserRole, object.value("id").toInt());
        }
    });
}

void MainWindow::loadConversationMessages(int conversationId)
{
    QNetworkReply *reply = m_apiClient->get(QString("/v1/conversations/%1/messages").arg(conversationId));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            statusBar()->showMessage("Не удалось загрузить сообщения");
            return;
        }

        m_chatPage->clearConversationView();
        const QJsonArray items = QJsonDocument::fromJson(reply->readAll()).array();
        for (const QJsonValue &value : items) {
            const QJsonObject object = value.toObject();
            const QString role = object.value("role").toString();
            const QString content = object.value("content").toString();
            if (role == "user") {
                m_chatPage->appendUserMessage(content);
            } else {
                m_chatPage->appendAssistantMessage(content);
            }
        }
    });
}

void MainWindow::loadDocuments()
{
    QNetworkReply *reply = m_apiClient->get("/v1/documents");
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            statusBar()->showMessage("Не удалось загрузить документы");
            return;
        }

        const QJsonArray items = QJsonDocument::fromJson(reply->readAll()).array();
        QTableWidget *table = m_documentsPage->table();
        table->setRowCount(0);
        for (const QJsonValue &value : items) {
            const QJsonObject object = value.toObject();
            const int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(object.value("filename").toString()));
            table->setItem(row, 1, new QTableWidgetItem(object.value("extension").toString()));
            table->setItem(row, 2, new QTableWidgetItem(object.value("indexed_in_rag").toString()));
            table->setItem(row, 3, new QTableWidgetItem(object.value("created_at").toString()));
        }
    });
}

void MainWindow::loadKnowledgeBase()
{
    QNetworkReply *reply = m_apiClient->get("/v1/knowledge-base");
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            statusBar()->showMessage("Не удалось загрузить базу официальных актов");
            return;
        }

        const QJsonArray items = QJsonDocument::fromJson(reply->readAll()).array();
        QTableWidget *table = m_knowledgeBasePage->table();
        table->setRowCount(0);
        for (const QJsonValue &value : items) {
            const QJsonObject object = value.toObject();
            const int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(object.value("title").toString()));
            table->setItem(row, 1, new QTableWidgetItem(object.value("source_url").toString()));
            table->setItem(row, 2, new QTableWidgetItem(object.value("indexed_in_rag").toString()));
            table->setItem(row, 3, new QTableWidgetItem(object.value("created_at").toString()));
        }
    });
}

void MainWindow::sendCurrentMessage()
{
    const QString text = m_chatPage->messageText();
    if (text.isEmpty()) {
        return;
    }

    const QString mode = m_chatPage->mode();
    m_chatPage->appendUserMessage(text);
    m_chatPage->clearInput();
    m_chatPage->setBusy(true);
    statusBar()->showMessage("Отправка запроса...");

    QJsonObject payload;
    if (m_currentConversationId > 0) {
        payload.insert("conversation_id", m_currentConversationId);
    } else {
        payload.insert("conversation_id", QJsonValue::Null);
        payload.insert("title", text.left(60));
    }

    QNetworkReply *reply = nullptr;
    if (mode == "plain") {
        payload.insert("message", text);
        reply = m_apiClient->post("/v1/chat", QJsonDocument(payload));
    } else {
        payload.insert("question", text);
        payload.insert("top_k", 5);
        payload.insert("search_scope", mode);
        reply = m_apiClient->post("/v1/rag/query", QJsonDocument(payload));
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply, mode]() {
        reply->deleteLater();
        m_chatPage->setBusy(false);

        if (reply->error() != QNetworkReply::NoError) {
            const QString detail = replyDetail(reply);
            QMessageBox::warning(this, "Ошибка запроса", detail);
            statusBar()->showMessage("Ошибка запроса");
            loadUsage();
            return;
        }

        const QJsonObject object = QJsonDocument::fromJson(reply->readAll()).object();
        m_currentConversationId = object.value("conversation_id").toInt();

        if (mode == "plain") {
            m_chatPage->appendAssistantMessage(object.value("answer").toString());
            m_chatPage->setCitationsHtml("<p><i>Для обычного чата дополнительные citations не требуются.</i></p>");
        } else {
            m_chatPage->appendAssistantMessage(object.value("answer").toString());
            const QJsonArray citations = object.value("citations").toArray();
            const double confidence = object.value("confidence_score").toDouble();
            const QString sourceValidation = object.value("source_validation").toString();
            const QString refusalReason = object.value("refusal_reason").toString();
            m_chatPage->setCitationsHtml(formatCitationsHtml(citations, confidence, sourceValidation, refusalReason));
        }

        loadConversations();
        loadUsage();
        statusBar()->showMessage("Ответ получен");
    });
}

void MainWindow::startNewConversation()
{
    m_currentConversationId = -1;
    m_conversationsList->clearSelection();
    m_chatPage->clearConversationView();
    m_chatPage->appendStatus("Новый диалог начат. Можешь задать первый вопрос.");
}

void MainWindow::uploadDocument()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Выберите документ",
        QString(),
        "Documents (*.pdf *.docx *.html *.htm *.png *.jpg *.jpeg)");
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть выбранный файл.");
        return;
    }

    auto *multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(QFileInfo(filePath).fileName())));
    filePart.setBody(file.readAll());
    multipart->append(filePart);

    QHttpPart typePart;
    typePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"analysis_type\""));
    typePart.setBody("general");
    multipart->append(typePart);

    statusBar()->showMessage("Загрузка документа...");
    QNetworkReply *reply = m_apiClient->postMultipart("/v1/documents/upload", multipart);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::warning(this, "Ошибка загрузки", replyDetail(reply));
            return;
        }

        loadDocuments();
        statusBar()->showMessage("Документ загружен");
    });
}

void MainWindow::ingestKnowledgeBase()
{
    if (m_knowledgeBasePage->url().isEmpty()) {
        QMessageBox::information(this, "Нужна ссылка", "Введи ссылку на pravo.gov.ru или publication.pravo.gov.ru.");
        return;
    }

    QJsonObject payload;
    payload.insert("url", m_knowledgeBasePage->url());
    payload.insert("max_documents", m_knowledgeBasePage->maxDocuments());

    statusBar()->showMessage("Добавление официального акта...");
    QNetworkReply *reply = m_apiClient->post("/v1/knowledge-base/ingest", QJsonDocument(payload));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::warning(this, "Ошибка загрузки акта", replyDetail(reply));
            return;
        }

        loadKnowledgeBase();
        statusBar()->showMessage("Акт добавлен в базу");
    });
}

void MainWindow::logout()
{
    QNetworkReply *reply = m_apiClient->post("/auth/logout", QJsonDocument(QJsonObject{}));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        emit logoutRequested();
    });
}

void MainWindow::openSupportLink()
{
    QDesktopServices::openUrl(QUrl(m_supportUrl));
}

QString MainWindow::formatCitationsHtml(
    const QJsonArray &citations,
    double confidence,
    const QString &sourceValidation,
    const QString &refusalReason) const
{
    QString html;
    html += QString("<div style='padding:10px 0;'><b>Confidence:</b> %1</div>").arg(QString::number(confidence, 'f', 2));
    html += QString("<div style='padding:0 0 12px;'><b>Проверка источников:</b> %1</div>").arg(sourceValidation.toHtmlEscaped());
    if (!refusalReason.isEmpty()) {
        html += QString("<div style='padding:0 0 12px; color:#ffcf8d;'><b>Причина отказа:</b> %1</div>").arg(refusalReason.toHtmlEscaped());
    }

    for (const QJsonValue &value : citations) {
        const QJsonObject object = value.toObject();
        html += "<hr style='border:0; border-top:1px solid rgba(255,255,255,0.12);'>";
        html += QString("<p><b>%1</b></p>").arg(object.value("title").toString().toHtmlEscaped());
        if (!object.value("article_label").toString().isEmpty()) {
            html += QString("<p>%1</p>").arg(object.value("article_label").toString().toHtmlEscaped());
        }
        if (!object.value("article_url").toString().isEmpty()) {
            const QString link = object.value("article_url").toString();
            html += QString("<p><a href=\"%1\">Открыть статью</a></p>").arg(link.toHtmlEscaped());
        } else if (!object.value("source_url").toString().isEmpty()) {
            const QString link = object.value("source_url").toString();
            html += QString("<p><a href=\"%1\">Открыть источник</a></p>").arg(link.toHtmlEscaped());
        }
        html += QString("<p>%1</p>").arg(object.value("quote").toString().toHtmlEscaped());
    }

    if (citations.isEmpty()) {
        html += "<p><i>Цитаты отсутствуют.</i></p>";
    }

    return html;
}
