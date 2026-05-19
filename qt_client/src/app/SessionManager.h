#pragma once

#include <QObject>
#include <QString>

#include "models/User.h"

class SessionManager : public QObject
{
    Q_OBJECT

public:
    explicit SessionManager(QObject *parent = nullptr);

    void setBackendUrl(const QString &url);
    QString backendUrl() const;

    void setAccessToken(const QString &token);
    QString accessToken() const;

    void setCurrentUser(const User &user);
    User currentUser() const;

    bool isAuthenticated() const;
    void clear();

signals:
    void sessionChanged();

private:
    QString m_backendUrl;
    QString m_accessToken;
    User m_currentUser;
};
