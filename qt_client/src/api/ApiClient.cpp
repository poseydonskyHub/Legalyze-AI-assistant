#include "api/ApiClient.h"

#include <QCoreApplication>
#include <QFile>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
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

ApiClient::ApiClient(QObject *parent)
    : QObject(parent),
      m_manager(new QNetworkAccessManager(this)),
      m_baseUrl(defaultBackendUrl())
{
}

void ApiClient::setBaseUrl(const QString &baseUrl)
{
    m_baseUrl = baseUrl;
}

QString ApiClient::baseUrl() const
{
    return m_baseUrl;
}

void ApiClient::setBearerToken(const QString &token)
{
    m_bearerToken = token;
}

QString ApiClient::bearerToken() const
{
    return m_bearerToken;
}

QNetworkReply *ApiClient::get(const QString &path)
{
    QNetworkRequest request = makeRequest(path);
    return m_manager->get(request);
}

QNetworkReply *ApiClient::post(const QString &path, const QJsonDocument &payload)
{
    QNetworkRequest request = makeRequest(path);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return m_manager->post(request, payload.toJson(QJsonDocument::Compact));
}

QNetworkReply *ApiClient::patch(const QString &path, const QJsonDocument &payload)
{
    QNetworkRequest request = makeRequest(path);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return m_manager->sendCustomRequest(request, "PATCH", payload.toJson(QJsonDocument::Compact));
}

QNetworkReply *ApiClient::deleteResource(const QString &path)
{
    QNetworkRequest request = makeRequest(path);
    return m_manager->deleteResource(request);
}

QNetworkReply *ApiClient::postMultipart(const QString &path, QHttpMultiPart *multipart)
{
    QNetworkRequest request = makeRequest(path);
    QNetworkReply *reply = m_manager->post(request, multipart);
    multipart->setParent(reply);
    return reply;
}

QNetworkRequest ApiClient::makeRequest(const QString &path) const
{
    QNetworkRequest request(buildUrl(path));
    applyHeaders(request);
    return request;
}

QUrl ApiClient::buildUrl(const QString &path) const
{
    return QUrl(m_baseUrl + path);
}

void ApiClient::applyHeaders(QNetworkRequest &request) const
{
    if (!m_bearerToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_bearerToken).toUtf8());
    }
    request.setRawHeader("Accept", "application/json");
}
