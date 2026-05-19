#pragma once

#include <QObject>

class ApiClient;

class DocumentApi : public QObject
{
    Q_OBJECT

public:
    explicit DocumentApi(ApiClient *client, QObject *parent = nullptr);

private:
    ApiClient *m_client = nullptr;
};
