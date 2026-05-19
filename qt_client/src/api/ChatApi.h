#pragma once

#include <QObject>

class ApiClient;

class ChatApi : public QObject
{
    Q_OBJECT

public:
    explicit ChatApi(ApiClient *client, QObject *parent = nullptr);

private:
    ApiClient *m_client = nullptr;
};
