#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

class LoginPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);

    void setBusy(bool busy);
    void setStatusText(const QString &text, bool isError = false);

signals:
    void loginRequested(const QString &email, const QString &password);
    void registerRequested(const QString &email, const QString &fullName, const QString &password);

private:
    void submitLogin();
    void submitRegister();

    QLineEdit *m_emailEdit = nullptr;
    QLineEdit *m_fullNameEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    QLabel *m_statusLabel = nullptr;
    QPushButton *m_loginButton = nullptr;
    QPushButton *m_registerButton = nullptr;
};
