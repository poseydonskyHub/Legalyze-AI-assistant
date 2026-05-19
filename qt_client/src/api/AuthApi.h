#pragma once

#include <QObject>

class ApiClient;

class AuthApi : public QObject
{
    Q_OBJECT

public:
    explicit AuthApi(ApiClient *client, QObject *parent = nullptr);

private:
    ApiClient *m_client = nullptr;
};
