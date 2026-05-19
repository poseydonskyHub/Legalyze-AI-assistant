#include "widgets/SettingsPage.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(16);

    auto *title = new QLabel("Настройки и доступ", this);
    title->setStyleSheet("font-size:24px; font-weight:700; color:#eef4fb;");

    auto *hint = new QLabel(
        "Приложение работает в демо-режиме. Здесь можно проверить подключение к backend, текущий аккаунт и перейти к поддержке проекта.",
        this);
    hint->setWordWrap(true);
    hint->setStyleSheet("color:#9fb0c3; font-size:14px;");

    m_userLabel = new QLabel(this);
    m_userLabel->setWordWrap(true);
    m_userLabel->setStyleSheet("color:#d7e3f0; background:#172536; border:1px solid rgba(255,255,255,0.08); border-radius:14px; padding:14px;");

    m_demoLabel = new QLabel(this);
    m_demoLabel->setWordWrap(true);
    m_demoLabel->setStyleSheet("color:#ffe1b0; background:rgba(244,163,64,0.12); border:1px solid rgba(244,163,64,0.25); border-radius:14px; padding:14px;");

    auto *supportTitle = new QLabel("Поддержать проект", this);
    supportTitle->setStyleSheet("font-size:18px; font-weight:700; color:#eef4fb;");

    auto *supportHint = new QLabel(
        "Если приложение оказалось полезным, можно поддержать развитие проекта через DonationAlerts. Ссылка откроется во внешнем браузере.",
        this);
    supportHint->setWordWrap(true);
    supportHint->setStyleSheet("color:#9fb0c3; font-size:14px; background:#172536; border:1px solid rgba(255,255,255,0.08); border-radius:14px; padding:14px;");

    m_supportButton = new QPushButton("Поддержать проект", this);
    m_supportButton->setCursor(Qt::PointingHandCursor);
    m_supportButton->setStyleSheet(
        "QPushButton { background:#f4a340; color:#1b1308; border:none; border-radius:14px; padding:14px 18px; font-weight:700; }"
        "QPushButton:hover { background:#f6bc66; }");

    m_logoutButton = new QPushButton("Выйти из аккаунта", this);
    m_logoutButton->setCursor(Qt::PointingHandCursor);
    m_logoutButton->setStyleSheet(
        "QPushButton { background:#223246; color:#eef4fb; border:1px solid rgba(255,255,255,0.10); border-radius:14px; padding:14px 18px; font-weight:600; }"
        "QPushButton:hover { background:#293b51; }");

    layout->addWidget(title);
    layout->addWidget(hint);
    layout->addWidget(m_userLabel);
    layout->addWidget(m_demoLabel);
    layout->addWidget(supportTitle);
    layout->addWidget(supportHint);
    layout->addWidget(m_supportButton);
    layout->addWidget(m_logoutButton);
    layout->addStretch();

    connect(m_supportButton, &QPushButton::clicked, this, &SettingsPage::supportRequested);
    connect(m_logoutButton, &QPushButton::clicked, this, &SettingsPage::logoutRequested);
}

void SettingsPage::setBackendUrl(const QString &url)
{
    Q_UNUSED(url);
}

void SettingsPage::setUserInfo(const QString &text)
{
    m_userLabel->setText(QString("Аккаунт<br>%1").arg(text.toHtmlEscaped().replace("\n", "<br>")));
}

void SettingsPage::setDemoInfo(const QString &text)
{
    m_demoLabel->setText(text.toHtmlEscaped().replace("\n", "<br>"));
}
