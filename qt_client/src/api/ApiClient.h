#pragma once

#include <QObject>
#include <QUrl>

class QHttpMultiPart;
class QJsonDocument;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

class ApiClient : public QObject
{
    Q_OBJECT

public:
    explicit ApiClient(QObject *parent = nullptr);

    void setBaseUrl(const QString &baseUrl);
    QString baseUrl() const;

    void setBearerToken(const QString &token);
    QString bearerToken() const;

    QNetworkReply *get(const QString &path);
    QNetworkReply *post(const QString &path, const QJsonDocument &payload);
    QNetworkReply *patch(const QString &path, const QJsonDocument &payload);
    QNetworkReply *deleteResource(const QString &path);
    QNetworkReply *postMultipart(const QString &path, QHttpMultiPart *multipart);

    QNetworkRequest makeRequest(const QString &path) const;

private:
    QUrl buildUrl(const QString &path) const;
    void applyHeaders(QNetworkRequest &request) const;

    QNetworkAccessManager *m_manager = nullptr;
    QString m_baseUrl;
    QString m_bearerToken;
};
