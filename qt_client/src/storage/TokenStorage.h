#pragma once

#include <QString>

class TokenStorage
{
public:
    QString loadToken() const;
    void saveToken(const QString &token) const;
    QString loadBackendUrl() const;
    void saveBackendUrl(const QString &url) const;
    QString ensureDeviceId() const;
    void clear() const;
};
