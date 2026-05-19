#include "api/AuthApi.h"

AuthApi::AuthApi(ApiClient *client, QObject *parent)
    : QObject(parent),
      m_client(client)
{
}
