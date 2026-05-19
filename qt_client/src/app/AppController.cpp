#include "app/AppController.h"

#include "api/ApiClient.h"
#include "app/SessionManager.h"
#include "models/User.h"
#include "storage/TokenStorage.h"
#include "widgets/LoginPage.h"
#include "widgets/MainWindow.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QProcessEnvironment>
#include <QTextStream>

namespace {
QString backendUrlFromFile()
{
    const QString path = QCoreApplication::applicationDirPath() + "/backend_url.txt";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream stream(&file);
    const QString url = stream.readLine().trimmed();
    if (url.startsWith("http://") || url.startsWith("https://")) {
        return url;
    }
    return {};
}

QString defaultBackendUrl()
{
    const QString fileUrl = backendUrlFromFile();
    if (!fileUrl.isEmpty()) {
        return fileUrl;
    }

    const QString envUrl = QProcessEnvironment::systemEnvironment().value("LEGALYZE_BACKEND_URL").trimmed();
    if (!envUrl.isEmpty()) {
        return envUrl;
    }
    return "http://127.0.0.1:8000";
}

QString replyDetail(QNetworkReply *reply)
{
    const QByteArray raw = reply->readAll();
    const QJsonDocument json = QJsonDocument::fromJson(raw);
    if (json.isObject()) {
        return json.object().value("detail").toString(QString::fromUtf8(raw));
    }
    return QString::fromUtf8(raw);
}

QString friendlyReplyError(QNetworkReply *reply)
{
    switch (reply->error()) {
    case QNetworkReply::ConnectionRefusedError:
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::TimeoutError:
        return QString("Не удаётся подключиться к backend на %1. Проверь, что FastAPI сервер запущен и адрес задан верно.")
            .arg(defaultBackendUrl());
    default:
        break;
    }
    return replyDetail(reply);
}
}

AppController::AppController(QObject *parent)
    : QObject(parent),
      m_sessionManager(new SessionManager(this)),
      m_tokenStorage(new TokenStorage()),
      m_apiClient(new ApiClient(this)),
      m_mainWindow(new MainWindow(m_sessionManager)),
      m_loginPage(new LoginPage())
{
    m_tokenStorage->ensureDeviceId();
    connect(m_loginPage, &LoginPage::loginRequested, this, &AppController::handleLogin);
    connect(m_loginPage, &LoginPage::registerRequested, this, &AppController::handleRegister);
    connect(m_mainWindow, &MainWindow::logoutRequested, this, [this]() {
        m_sessionManager->clear();
        m_tokenStorage->clear();
        showLogin();
    });
}

AppController::~AppController()
{
    delete m_tokenStorage;
    delete m_mainWindow;
    delete m_loginPage;
}

QString AppController::backendUrl() const
{
    const QString saved = m_tokenStorage->loadBackendUrl().trimmed();
    if (!saved.isEmpty()) {
        return saved;
    }
    return defaultBackendUrl();
}

void AppController::start()
{
    tryRestoreSession();
}

void AppController::showLogin()
{
    m_loginPage->setBusy(false);
    m_loginPage->setStatusText("Введите данные для входа.");
    m_mainWindow->hide();
    m_loginPage->show();
    m_loginPage->raise();
    m_loginPage->activateWindow();
}

void AppController::showMainWindow()
{
    m_loginPage->hide();
    m_mainWindow->initialize();
    m_mainWindow->show();
    m_mainWindow->raise();
    m_mainWindow->activateWindow();
}

void AppController::tryRestoreSession()
{
    const QString currentBackendUrl = backendUrl();
    const QString token = m_tokenStorage->loadToken();

    m_sessionManager->setBackendUrl(currentBackendUrl);
    if (token.isEmpty()) {
        showLogin();
        return;
    }

    m_sessionManager->setAccessToken(token);
    validateSession();
}

void AppController::validateSession()
{
    m_apiClient->setBaseUrl(m_sessionManager->backendUrl());
    m_apiClient->setBearerToken(m_sessionManager->accessToken());

    QNetworkReply *reply = m_apiClient->get("/auth/me");
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            m_sessionManager->clear();
            m_tokenStorage->clear();
            showLogin();
            return;
        }

        const QJsonObject object = QJsonDocument::fromJson(reply->readAll()).object();
        User user;
        user.id = object.value("id").toInt();
        user.email = object.value("email").toString();
        user.fullName = object.value("full_name").toString();
        m_sessionManager->setCurrentUser(user);
        showMainWindow();
    });
}

void AppController::handleLogin(const QString &email, const QString &password)
{
    if (email.trimmed().isEmpty() || password.isEmpty()) {
        m_loginPage->setStatusText("Укажи email и пароль.", true);
        return;
    }

    m_loginPage->setBusy(true);
    m_loginPage->setStatusText("Выполняется вход...");

    const QString currentBackendUrl = backendUrl();
    m_sessionManager->setBackendUrl(currentBackendUrl);
    m_apiClient->setBaseUrl(currentBackendUrl);

    QJsonObject payload{
        {"email", email},
        {"password", password},
        {"device_id", m_tokenStorage->ensureDeviceId()},
    };
    QNetworkReply *reply = m_apiClient->post("/auth/login", QJsonDocument(payload));
    connect(reply, &QNetworkReply::finished, this, [this, reply, currentBackendUrl]() {
        reply->deleteLater();
        m_loginPage->setBusy(false);
        if (reply->error() != QNetworkReply::NoError) {
            m_loginPage->setStatusText(friendlyReplyError(reply), true);
            return;
        }
        completeAuthorizedSession(currentBackendUrl, QJsonDocument::fromJson(reply->readAll()).object());
    });
}

void AppController::handleRegister(const QString &email, const QString &fullName, const QString &password)
{
    if (email.trimmed().isEmpty() || fullName.trimmed().isEmpty() || password.isEmpty()) {
        m_loginPage->setStatusText("Для регистрации заполни email, имя и пароль.", true);
        return;
    }

    m_loginPage->setBusy(true);
    m_loginPage->setStatusText("Создаём пользователя...");

    const QString currentBackendUrl = backendUrl();
    m_sessionManager->setBackendUrl(currentBackendUrl);
    m_apiClient->setBaseUrl(currentBackendUrl);

    QJsonObject payload{
        {"email", email},
        {"full_name", fullName},
        {"password", password},
        {"device_id", m_tokenStorage->ensureDeviceId()},
    };
    QNetworkReply *reply = m_apiClient->post("/auth/register", QJsonDocument(payload));
    connect(reply, &QNetworkReply::finished, this, [this, reply, currentBackendUrl]() {
        reply->deleteLater();
        m_loginPage->setBusy(false);
        if (reply->error() != QNetworkReply::NoError) {
            m_loginPage->setStatusText(friendlyReplyError(reply), true);
            return;
        }
        completeAuthorizedSession(currentBackendUrl, QJsonDocument::fromJson(reply->readAll()).object());
    });
}

void AppController::completeAuthorizedSession(const QString &currentBackendUrl, const QJsonObject &authObject)
{
    User user;
    user.id = authObject.value("user_id").toInt();
    user.email = authObject.value("email").toString();
    user.fullName = authObject.value("full_name").toString();

    const QString token = authObject.value("access_token").toString();
    m_sessionManager->setBackendUrl(currentBackendUrl);
    m_sessionManager->setAccessToken(token);
    m_sessionManager->setCurrentUser(user);

    m_tokenStorage->saveBackendUrl(currentBackendUrl);
    m_tokenStorage->saveToken(token);
    showMainWindow();
}
