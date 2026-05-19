#include "widgets/LoginPage.h"

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
QLabel *makeFieldLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setStyleSheet("color:#c8d4df; font-size:14px; font-weight:600;");
    return label;
}
}

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Legalyze - Вход");
    resize(760, 620);
    setStyleSheet("background:#0b1219;");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 32, 32, 32);

    auto *card = new QFrame(this);
    card->setMaximumWidth(460);
    card->setStyleSheet("QFrame { background:#111b27; border:1px solid rgba(255,255,255,0.08); border-radius:28px; }");

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 28, 28, 28);
    cardLayout->setSpacing(14);

    auto *badge = new QLabel("Demo access", this);
    badge->setStyleSheet("color:#1b1308; background:#f4a340; border-radius:12px; padding:8px 12px; font-weight:700;");

    auto *title = new QLabel("Вход в Legalyze", this);
    title->setStyleSheet("font-size:30px; font-weight:800; color:#eef4fb;");

    auto *subtitle = new QLabel(
        "Авторизуйся, чтобы работать с чатом, загружать документы и использовать демо-режим на 5 запросов в день.",
        this);
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("color:#9fb0c3; font-size:14px;");

    const QString inputStyle =
        "QLineEdit { background:#172536; color:#eef4fb; border:1px solid rgba(255,255,255,0.10); border-radius:14px; padding:12px 14px; }"
        "QLineEdit:focus { border:1px solid #f4a340; }";

    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setPlaceholderText("name@example.com");
    m_emailEdit->setStyleSheet(inputStyle);

    m_fullNameEdit = new QLineEdit(this);
    m_fullNameEdit->setPlaceholderText("Имя для регистрации");
    m_fullNameEdit->setStyleSheet(inputStyle);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Пароль");
    m_passwordEdit->setStyleSheet(inputStyle);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setMinimumHeight(24);
    m_statusLabel->setStyleSheet("color:#9fb0c3;");

    m_loginButton = new QPushButton("Войти", this);
    m_registerButton = new QPushButton("Создать аккаунт", this);
    m_loginButton->setCursor(Qt::PointingHandCursor);
    m_registerButton->setCursor(Qt::PointingHandCursor);

    m_loginButton->setStyleSheet(
        "QPushButton { background:#f4a340; color:#1b1308; border:none; border-radius:14px; padding:14px 18px; font-weight:700; }"
        "QPushButton:hover { background:#f6bc66; }");
    m_registerButton->setStyleSheet(
        "QPushButton { background:#223246; color:#eef4fb; border:1px solid rgba(255,255,255,0.10); border-radius:14px; padding:14px 18px; font-weight:600; }"
        "QPushButton:hover { background:#293b51; }");

    cardLayout->addWidget(badge, 0, Qt::AlignLeft);
    cardLayout->addWidget(title);
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(makeFieldLabel("Email", this));
    cardLayout->addWidget(m_emailEdit);
    cardLayout->addWidget(makeFieldLabel("Имя", this));
    cardLayout->addWidget(m_fullNameEdit);
    cardLayout->addWidget(makeFieldLabel("Пароль", this));
    cardLayout->addWidget(m_passwordEdit);
    cardLayout->addWidget(m_statusLabel);
    cardLayout->addWidget(m_loginButton);
    cardLayout->addWidget(m_registerButton);

    layout->addStretch();
    layout->addWidget(card, 0, Qt::AlignCenter);
    layout->addStretch();

    connect(m_loginButton, &QPushButton::clicked, this, &LoginPage::submitLogin);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginPage::submitRegister);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginPage::submitLogin);
}

void LoginPage::submitLogin()
{
    emit loginRequested(m_emailEdit->text().trimmed(), m_passwordEdit->text());
}

void LoginPage::submitRegister()
{
    emit registerRequested(
        m_emailEdit->text().trimmed(),
        m_fullNameEdit->text().trimmed(),
        m_passwordEdit->text());
}

void LoginPage::setBusy(bool busy)
{
    m_emailEdit->setEnabled(!busy);
    m_fullNameEdit->setEnabled(!busy);
    m_passwordEdit->setEnabled(!busy);
    m_loginButton->setEnabled(!busy);
    m_registerButton->setEnabled(!busy);
}

void LoginPage::setStatusText(const QString &text, bool isError)
{
    m_statusLabel->setText(text);
    m_statusLabel->setStyleSheet(isError ? "color:#ff9b8e;" : "color:#9fd5aa;");
}
