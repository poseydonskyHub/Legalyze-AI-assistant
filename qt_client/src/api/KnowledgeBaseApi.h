#pragma once

#include <QObject>

class ApiClient;

class KnowledgeBaseApi : public QObject
{
    Q_OBJECT

public:
    explicit KnowledgeBaseApi(ApiClient *client, QObject *parent = nullptr);

private:
    ApiClient *m_client = nullptr;
};
