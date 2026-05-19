#include "app/SessionManager.h"

#include <QCoreApplication>
#include <QFile>
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
}

SessionManager::SessionManager(QObject *parent)
    : QObject(parent),
      m_backendUrl(defaultBackendUrl())
{
}

void SessionManager::setBackendUrl(const QString &url)
{
    m_backendUrl = url;
    emit sessionChanged();
}

QString SessionManager::backendUrl() const
{
    return m_backendUrl;
}

void SessionManager::setAccessToken(const QString &token)
{
    m_accessToken = token;
    emit sessionChanged();
}

QString SessionManager::accessToken() const
{
    return m_accessToken;
}

void SessionManager::setCurrentUser(const User &user)
{
    m_currentUser = user;
    emit sessionChanged();
}

User SessionManager::currentUser() const
{
    return m_currentUser;
}

bool SessionManager::isAuthenticated() const
{
    return !m_accessToken.isEmpty();
}

void SessionManager::clear()
{
    m_accessToken.clear();
    m_currentUser = User{};
    emit sessionChanged();
}
