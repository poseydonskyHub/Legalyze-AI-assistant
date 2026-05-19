#include "widgets/DocumentsPage.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

DocumentsPage::DocumentsPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(14);

    auto *title = new QLabel("Документы пользователя", this);
    title->setStyleSheet("font-size:24px; font-weight:700; color:#eef4fb;");

    auto *hint = new QLabel("Загружай договоры, претензии, HTML-страницы и скриншоты документов. После загрузки они попадут в анализ и локальный RAG.", this);
    hint->setWordWrap(true);
    hint->setStyleSheet("color:#9fb0c3; font-size:14px;");

    auto *actions = new QHBoxLayout();
    m_uploadButton = new QPushButton("Загрузить документ", this);
    m_refreshButton = new QPushButton("Обновить список", this);

    const QString buttonStyle =
        "QPushButton { background:#223246; color:#eef4fb; border:1px solid rgba(255,255,255,0.10); border-radius:14px; padding:12px 16px; font-weight:600; }"
        "QPushButton:hover { background:#2a3f58; }";
    m_uploadButton->setCursor(Qt::PointingHandCursor);
    m_refreshButton->setCursor(Qt::PointingHandCursor);
    m_uploadButton->setStyleSheet(buttonStyle);
    m_refreshButton->setStyleSheet(buttonStyle);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"Файл", "Тип", "RAG", "Создан"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setStyleSheet(
        "QTableWidget { background:#101b28; color:#eef4fb; border:1px solid rgba(255,255,255,0.08); border-radius:18px; gridline-color:rgba(255,255,255,0.06); }"
        "QHeaderView::section { background:#172536; color:#d7e3f0; border:none; padding:10px; font-weight:700; }");

    actions->addWidget(m_uploadButton);
    actions->addWidget(m_refreshButton);
    actions->addStretch();

    layout->addWidget(title);
    layout->addWidget(hint);
    layout->addLayout(actions);
    layout->addWidget(m_table);

    connect(m_uploadButton, &QPushButton::clicked, this, &DocumentsPage::uploadRequested);
    connect(m_refreshButton, &QPushButton::clicked, this, &DocumentsPage::refreshRequested);
}

void DocumentsPage::clearTable()
{
    m_table->setRowCount(0);
}

QTableWidget *DocumentsPage::table() const
{
    return m_table;
}
