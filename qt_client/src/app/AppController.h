#pragma once

#include <QObject>
#include <QJsonObject>
#include <QString>

class ApiClient;
class LoginPage;
class MainWindow;
class SessionManager;
class TokenStorage;

class AppController : public QObject
{
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    void start();

private:
    QString backendUrl() const;
    void showLogin();
    void showMainWindow();
    void tryRestoreSession();
    void validateSession();
    void handleLogin(const QString &email, const QString &password);
    void handleRegister(const QString &email, const QString &fullName, const QString &password);
    void completeAuthorizedSession(const QString &backendUrl, const QJsonObject &authObject);

    SessionManager *m_sessionManager = nullptr;
    TokenStorage *m_tokenStorage = nullptr;
    ApiClient *m_apiClient = nullptr;
    MainWindow *m_mainWindow = nullptr;
    LoginPage *m_loginPage = nullptr;
};
