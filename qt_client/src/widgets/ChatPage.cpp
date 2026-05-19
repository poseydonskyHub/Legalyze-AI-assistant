#include "widgets/ChatPage.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTextBrowser>
#include <QVBoxLayout>

ChatPage::ChatPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(14);

    auto *header = new QHBoxLayout();
    auto *titleWrap = new QVBoxLayout();
    auto *title = new QLabel("Юридический ассистент", this);
    title->setStyleSheet("font-size:26px; font-weight:700; color:#eef4fb;");

    auto *subtitle = new QLabel("Пиши вопрос, загружай документы и переключай режим поиска по своим файлам или официальным актам.", this);
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("color:#9fb0c3; font-size:14px;");

    m_usageLabel = new QLabel("Демо: загрузка лимита...", this);
    m_usageLabel->setStyleSheet(
        "color:#ffe1b0; background:rgba(244,163,64,0.12); border:1px solid rgba(244,163,64,0.25); border-radius:14px; padding:10px 14px; font-weight:600;");

    titleWrap->addWidget(title);
    titleWrap->addWidget(subtitle);
    header->addLayout(titleWrap, 1);
    header->addWidget(m_usageLabel);

    auto *splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);

    auto *left = new QWidget(splitter);
    auto *leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(12);

    auto *controls = new QHBoxLayout();
    auto *modeLabel = new QLabel("Режим:", this);
    modeLabel->setStyleSheet("color:#c6d3e0; font-weight:600;");

    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItem("Обычный чат", "plain");
    m_modeCombo->addItem("RAG: мои документы", "user");
    m_modeCombo->addItem("RAG: официальные акты", "official");
    m_modeCombo->addItem("RAG: все источники", "all");
    m_modeCombo->setStyleSheet(
        "QComboBox { background:#172536; color:#eef4fb; border:1px solid rgba(255,255,255,0.10); border-radius:12px; padding:10px 14px; min-width:240px; }");

    controls->addWidget(modeLabel);
    controls->addWidget(m_modeCombo, 1);

    m_messagesView = new QTextBrowser(this);
    m_messagesView->setOpenExternalLinks(true);
    m_messagesView->setStyleSheet(
        "QTextBrowser { background:#101b28; color:#eef4fb; border:1px solid rgba(255,255,255,0.08); border-radius:20px; padding:16px; }");

    m_inputEdit = new QPlainTextEdit(this);
    m_inputEdit->setPlaceholderText("Введите ваш вопрос по документу, договору или статье...");
    m_inputEdit->setMaximumHeight(170);
    m_inputEdit->setStyleSheet(
        "QPlainTextEdit { background:#172536; color:#eef4fb; border:1px solid rgba(255,255,255,0.10); border-radius:16px; padding:14px; }");

    m_sendButton = new QPushButton("Отправить запрос", this);
    m_sendButton->setCursor(Qt::PointingHandCursor);
    m_sendButton->setStyleSheet(
        "QPushButton { background:#f4a340; color:#1b1308; border:none; border-radius:14px; padding:14px 20px; font-weight:700; }"
        "QPushButton:hover { background:#f6bc66; }"
        "QPushButton:disabled { background:#7f6640; color:#dbc6a2; }");

    leftLayout->addLayout(controls);
    leftLayout->addWidget(m_messagesView, 1);
    leftLayout->addWidget(m_inputEdit);
    leftLayout->addWidget(m_sendButton);

    auto *right = new QWidget(splitter);
    auto *rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(12);

    auto *rightTitle = new QLabel("Цитаты и источники", this);
    rightTitle->setStyleSheet("font-size:18px; font-weight:700; color:#eef4fb;");

    auto *rightHint = new QLabel("Здесь появятся ссылки на статьи, источники и confidence score ответа.", this);
    rightHint->setWordWrap(true);
    rightHint->setStyleSheet("color:#9fb0c3; font-size:13px;");

    m_citationsView = new QTextBrowser(this);
    m_citationsView->setOpenExternalLinks(true);
    m_citationsView->setStyleSheet(
        "QTextBrowser { background:#101b28; color:#eef4fb; border:1px solid rgba(255,255,255,0.08); border-radius:20px; padding:16px; }");

    rightLayout->addWidget(rightTitle);
    rightLayout->addWidget(rightHint);
    rightLayout->addWidget(m_citationsView, 1);

    splitter->addWidget(left);
    splitter->addWidget(right);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    layout->addLayout(header);
    layout->addWidget(splitter, 1);

    connect(m_sendButton, &QPushButton::clicked, this, &ChatPage::sendRequested);
}

QString ChatPage::messageText() const
{
    return m_inputEdit->toPlainText().trimmed();
}

QString ChatPage::mode() const
{
    return m_modeCombo->currentData().toString();
}

void ChatPage::setUsageInfo(const QString &text)
{
    m_usageLabel->setText(text);
}

void ChatPage::clearInput()
{
    m_inputEdit->clear();
}

void ChatPage::appendStatus(const QString &text)
{
    m_messagesView->append(QString("<div style='margin:8px 0; color:#9fb0c3;'><i>%1</i></div>").arg(text.toHtmlEscaped()));
}

void ChatPage::appendUserMessage(const QString &text)
{
    m_messagesView->append(
        QString("<div style='margin:14px 0; padding:14px 16px; border-radius:16px; background:#1d3044;'>"
                "<div style='font-weight:700; margin-bottom:8px;'>Вы</div><div>%1</div></div>")
            .arg(text.toHtmlEscaped().replace("\n", "<br>")));
}

void ChatPage::appendAssistantMessage(const QString &text)
{
    m_messagesView->append(
        QString("<div style='margin:14px 0; padding:14px 16px; border-radius:16px; background:#162231; border:1px solid rgba(255,255,255,0.06);'>"
                "<div style='font-weight:700; margin-bottom:8px; color:#ffd59b;'>Ассистент</div><div>%1</div></div>")
            .arg(text.toHtmlEscaped().replace("\n", "<br>")));
}

void ChatPage::setCitationsHtml(const QString &html)
{
    m_citationsView->setHtml(html);
}

void ChatPage::clearConversationView()
{
    m_messagesView->clear();
    m_citationsView->clear();
}

void ChatPage::setBusy(bool busy)
{
    m_modeCombo->setEnabled(!busy);
    m_inputEdit->setEnabled(!busy);
    m_sendButton->setEnabled(!busy);
}
