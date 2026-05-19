#include "api/KnowledgeBaseApi.h"

KnowledgeBaseApi::KnowledgeBaseApi(ApiClient *client, QObject *parent)
    : QObject(parent),
      m_client(client)
{
}
