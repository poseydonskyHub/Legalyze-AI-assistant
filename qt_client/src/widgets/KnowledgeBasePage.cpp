#include "widgets/KnowledgeBasePage.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

KnowledgeBasePage::KnowledgeBasePage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(14);

    auto *title = new QLabel("База официальных актов", this);
    title->setStyleSheet("font-size:24px; font-weight:700; color:#eef4fb;");

    auto *hint = new QLabel("Добавляй ссылки с pravo.gov.ru или publication.pravo.gov.ru, чтобы ответы могли ссылаться на официальные источники.", this);
    hint->setWordWrap(true);
    hint->setStyleSheet("color:#9fb0c3; font-size:14px;");

    auto *controls = new QHBoxLayout();
    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText("https://publication.pravo.gov.ru/document/...");
    m_urlEdit->setStyleSheet("QLineEdit { background:#172536; color:#eef4fb; border:1px solid rgba(255,255,255,0.10); border-radius:14px; padding:12px 14px; }");

    m_maxDocuments = new QSpinBox(this);
    m_maxDocuments->setRange(1, 20);
    m_maxDocuments->setValue(5);
    m_maxDocuments->setStyleSheet("QSpinBox { background:#172536; color:#eef4fb; border:1px solid rgba(255,255,255,0.10); border-radius:14px; padding:10px 12px; }");

    const QString buttonStyle =
        "QPushButton { background:#223246; color:#eef4fb; border:1px solid rgba(255,255,255,0.10); border-radius:14px; padding:12px 16px; font-weight:600; }"
        "QPushButton:hover { background:#2a3f58; }";

    m_ingestButton = new QPushButton("Добавить акт", this);
    m_refreshButton = new QPushButton("Обновить список", this);
    m_ingestButton->setCursor(Qt::PointingHandCursor);
    m_refreshButton->setCursor(Qt::PointingHandCursor);
    m_ingestButton->setStyleSheet(buttonStyle);
    m_refreshButton->setStyleSheet(buttonStyle);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"Название", "Источник", "RAG", "Создан"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setStyleSheet(
        "QTableWidget { background:#101b28; color:#eef4fb; border:1px solid rgba(255,255,255,0.08); border-radius:18px; gridline-color:rgba(255,255,255,0.06); }"
        "QHeaderView::section { background:#172536; color:#d7e3f0; border:none; padding:10px; font-weight:700; }");

    controls->addWidget(m_urlEdit, 1);
    controls->addWidget(m_maxDocuments);
    controls->addWidget(m_ingestButton);
    controls->addWidget(m_refreshButton);

    layout->addWidget(title);
    layout->addWidget(hint);
    layout->addLayout(controls);
    layout->addWidget(m_table);

    connect(m_ingestButton, &QPushButton::clicked, this, &KnowledgeBasePage::ingestRequested);
    connect(m_refreshButton, &QPushButton::clicked, this, &KnowledgeBasePage::refreshRequested);
}

QString KnowledgeBasePage::url() const
{
    return m_urlEdit->text().trimmed();
}

int KnowledgeBasePage::maxDocuments() const
{
    return m_maxDocuments->value();
}

void KnowledgeBasePage::clearTable()
{
    m_table->setRowCount(0);
}

QTableWidget *KnowledgeBasePage::table() const
{
    return m_table;
}
