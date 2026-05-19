#include "storage/TokenStorage.h"

#include <QCoreApplication>
#include <QFile>
#include <QProcessEnvironment>
#include <QSettings>
#include <QTextStream>
#include <QUuid>

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

QString TokenStorage::loadToken() const
{
    QSettings settings;
    return settings.value("auth/token").toString();
}

void TokenStorage::saveToken(const QString &token) const
{
    QSettings settings;
    settings.setValue("auth/token", token);
}

QString TokenStorage::loadBackendUrl() const
{
    QSettings settings;
    QString url = settings.value("app/backend_url", defaultBackendUrl()).toString().trimmed();
    if (url.isEmpty() || url == "http://localhost:8000" || url == "http://127.0.0.1:8000" || url.contains("trycloudflare.com")) {
        url = defaultBackendUrl();
        settings.setValue("app/backend_url", url);
    }
    return url;
}

void TokenStorage::saveBackendUrl(const QString &url) const
{
    QSettings settings;
    settings.setValue("app/backend_url", url);
}

QString TokenStorage::ensureDeviceId() const
{
    QSettings settings;
    QString deviceId = settings.value("app/device_id").toString().trimmed();
    if (deviceId.isEmpty()) {
        deviceId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        settings.setValue("app/device_id", deviceId);
    }
    return deviceId;
}

void TokenStorage::clear() const
{
    QSettings settings;
    settings.remove("auth/token");
}
