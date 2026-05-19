#pragma once

#include <QWidget>

class QLabel;
class QPushButton;

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

    void setBackendUrl(const QString &url);
    void setUserInfo(const QString &text);
    void setDemoInfo(const QString &text);

signals:
    void logoutRequested();
    void supportRequested();

private:
    QLabel *m_backendLabel = nullptr;
    QLabel *m_userLabel = nullptr;
    QLabel *m_demoLabel = nullptr;
    QPushButton *m_supportButton = nullptr;
    QPushButton *m_logoutButton = nullptr;
};
