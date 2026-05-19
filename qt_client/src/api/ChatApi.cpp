#include "api/ChatApi.h"

ChatApi::ChatApi(ApiClient *client, QObject *parent)
    : QObject(parent),
      m_client(client)
{
}
